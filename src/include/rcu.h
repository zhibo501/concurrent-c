#ifdef __cplusplus
extern "C" {
#endif

#ifndef __RCU_HEADER__
#define __RCU_HEADER__

#define SHARE_DATA_MAX_TRY      (128)
#define SHARE_DATA_MAX_SHADOW   (256)

typedef void* (*usr_clone)(const void *usr_data);
typedef void  (*usr_free)(void *usr_data);

typedef int   (*share_read_hook)(const void* usr_data, void* option);
typedef int   (*share_write_hook)(void* usr_data, void* option);

void* new_share_data(int shadow_max, usr_clone clone, usr_free free, void* usr_data);
int   share_read(void* this, share_read_hook read_hook, void* option);
int   share_write(void* this, share_write_hook write_hook, void* option);

#endif

#ifdef __cplusplus
}
#endif /* end of __cplusplus */
