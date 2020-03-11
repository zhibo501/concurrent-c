#ifdef __cplusplus
extern "C" {
#endif

#ifndef __RCU_HEADER__
#define __RCU_HEADER__

#ifdef __CONS_PATCH__
#    define  _PUB_
#    define  _LOCAL_
#else
#    define  _PUB_     __attribute__ ((visibility("default")))
#    define  _LOCAL_   __attribute__ ((visibility("hidden")))
#endif


#define SHARE_DATA_MAX_TRY      (128)
#define SHARE_DATA_MAX_SHADOW   (64)

typedef void* (*usr_clone)(const void *usr_data);
typedef void  (*usr_free)(void *usr_data);

typedef int   (*share_read_hook)(const void* usr_data, void* option);
typedef int   (*share_write_hook)(void* usr_data, void* option);

_PUB_       void* new_share_data(int shadow_max, usr_clone clone, usr_free free, void* usr_data);
_PUB_         int share_read(void* self, share_read_hook read_hook, void* option);
_PUB_         int share_write(void* self, share_write_hook write_hook, void* option);
_PUB_ const char* share_get_ver();

#endif

#ifdef __cplusplus
}
#endif /* end of __cplusplus */
