#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "rcu.h"

#define SHARE_DATA_MAGIC_NUM     (0x20200308)

///////////////////////////////////////////////////////////////////////////////
// ref data
///////////////////////////////////////////////////////////////////////////////

/// ref nums:
///     0 :     empty
///     1 :     default
///     n + 1 : n attached
typedef struct {
	int32_t  ref;
	void    *data;
} ref_data;

static inline
void __ref_data_init(ref_data* this, void* usr_data) {
	assert(this != NULL);
	this->ref = 0;
	this->data = usr_data;
}

static inline
int __ref_data_is_empty(ref_data* this) {
	assert(this != NULL);
	return (__sync_or_and_fetch(&(this->ref), 0) <= 0);
}

static inline
void __ref_data_attach(ref_data* this) {
	assert(this != NULL);
	__sync_add_and_fetch(&(this->ref), 1);
}

static inline
int __ref_data_try_attach(ref_data* this) {
	assert(this != NULL);
	int32_t ref = __sync_or_and_fetch(&(this->ref), 0);
	if (0 == ref) {
		return -1;
	}

	// maybe some one change ref to 0 or another
	if (__sync_bool_compare_and_swap(&(this->ref), ref, ref + 1)) {
		return 0;
	} else {
		return -1;
	}
}

static inline
void __ref_data_detach(ref_data* this) {
	assert(this != NULL);
	__sync_sub_and_fetch(&(this->ref), 1);
}

static inline
void* __ref_data_get_usr(ref_data* this) {
	assert(this != NULL);
	return this->data;
}

static inline
void __ref_data_set_usr(ref_data* this, void* usr_data) {
	assert(this != NULL);
	this->data = usr_data;
}

static inline
void __ref_data_free_usr(ref_data* this, usr_free free_hook) {
	assert(this != NULL && free_hook != NULL);
	if (this->data != NULL) {
		free_hook(this->data);
		this->data = NULL;
	}
}


///////////////////////////////////////////////////////////////////////////////
// share data
///////////////////////////////////////////////////////////////////////////////

typedef struct {
	// static section
	int32_t          magic;
	int32_t          max;
	usr_clone        clone;
	usr_free         free;
	// dyn section
	pthread_rwlock_t rwlock;
	int32_t          resv;
	int32_t          cur;
	ref_data         rdata[0];
} share_data;

void* new_share_data(int shadow_max, usr_clone clone, usr_free free, void* usr_data)
{
	if ((NULL == usr_data) || (NULL == clone) || (NULL == free)) {
		return NULL;
	}

	if (shadow_max < 2) {
		shadow_max = 2;
	} else if (shadow_max > SHARE_DATA_MAX_SHADOW) {
		shadow_max = SHARE_DATA_MAX_SHADOW;
	}

	int size = sizeof(share_data) + (shadow_max * sizeof(ref_data));
	share_data* p = (share_data*)malloc(size);
	if (NULL == p) {
		return p;
	}

	memset(p, 0, size);
	p->magic = SHARE_DATA_MAGIC_NUM;
	p->max = shadow_max;
	p->clone = clone;
	p->free = free;

	if (0 != pthread_rwlock_init(&(p->rwlock), NULL)) {
		// TODO show error info
		free(p);
		return NULL;
	}
	p->cur = 0;
	__ref_data_init(&(p->rdata[0]), usr_data);
	return p;
}

void share_debug_show(void* this)
{
	if (NULL == this) {
		return;
	}

	share_data* p = (share_data*)this;
	printf("====== share_data : magic = 0x%x | max = %d | cur = %d | clone = %p | free = %p\n",
		p->magic, p->max, p->cur, p->clone, p->free);
	for (int32_t i = 0; i < p->max && i < SHARE_DATA_MAX_SHADOW; i++) {
		printf("====== share_data : rdata[%d] = (%d , %p)\n",
			i, p->rdata[i].ref, p->rdata[i].data);
	}
}

static inline
int __share_check(share_data* this) {
	return (SHARE_DATA_MAGIC_NUM == this->magic);
}

static inline
int32_t __share_data_find_next(share_data* this, int32_t exclude) {
	int32_t try = 0;
	while (try++ < SHARE_DATA_MAX_TRY) {
		for (int32_t i = 0; i < this->max && i < SHARE_DATA_MAX_SHADOW; i++) {
			if ((i != exclude) && __ref_data_is_empty(&(this->rdata[i]))) {
				return i;
			}
		}

		usleep(10);
	}

	share_debug_show(this);
	return -1;
}

static inline
int __share_read(share_data* this, share_read_hook read_hook, void* option)
{
	assert((this != NULL) && (read_hook != NULL));
	int i = 0;
	ref_data *rd = NULL;

	while (i++ < SHARE_DATA_MAX_TRY) {
		rd = &(this->rdata[__sync_fetch_and_or(&(this->cur), 0)]);
		if (0 == __ref_data_try_attach(rd)) {
			break;
		}
	}
	if (i >= SHARE_DATA_MAX_TRY) {
		share_debug_show(this);
		return -1;
	}

	int ret = read_hook(__ref_data_get_usr(rd), option);
	__ref_data_detach(rd);
	return ret;
}

int share_read(void* this, share_read_hook read_hook, void* option)
{
	if ((this != NULL) && (read_hook != NULL)
		&& __share_check(this)) {
		return __share_read((share_data*)this, read_hook, option);
	}
	return -1;
}

int __share_write(share_data* this, share_write_hook write_hook, void* option)
{
	// get cur rdata
	uint32_t  cur_i  = __sync_fetch_and_or(&(this->cur), 0);
	ref_data *cur_rd = &(this->rdata[cur_i]);
	// find next empty rdata which is attached by no one.
	// if all rdata is attached, select another and wait
	int32_t   next_i = __share_data_find_next(this, cur_i);
	if (next_i < 0) {
		return -1;
	}
	ref_data *next_rd = &(this->rdata[next_i]);

	// copy cur rdata to next rdata
	__ref_data_free_usr(next_rd, this->free);
	__ref_data_set_usr(next_rd, this->clone(__ref_data_get_usr(cur_rd)));
	// call write hook with the next rdata
	int ret = write_hook(__ref_data_get_usr(next_rd), option);
	if (ret != 0) {
		return ret;
	}

	// default attach to next rdata
	__ref_data_attach(next_rd);
	// current index switch to next rdata
	__sync_lock_test_and_set(&(this->cur), next_i);

	// detach cur rdata
	__ref_data_detach(cur_rd);
	return 0;
}

static inline
int __share_wlock(share_data* this) {
	assert(this != NULL);
	return pthread_rwlock_wrlock(&(this->rwlock));
}

static inline
int __share_unlock(share_data* this) {
	assert(this != NULL);
	return pthread_rwlock_unlock(&(this->rwlock));
}

int share_write(void* this, share_write_hook write_hook, void* option)
{
	int ret = -1;
	if (NULL == this || NULL == write_hook) {
		return ret;
	}

	if (__share_check(this)) {
		__share_wlock((share_data*)this);
		ret = __share_write((share_data*)this, write_hook, option);
		__share_unlock((share_data*)this);
	}
	return ret;
}
