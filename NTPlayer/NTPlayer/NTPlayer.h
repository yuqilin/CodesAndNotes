#ifndef __NTPLAYER_H__
#define __NTPLAYER_H__

#ifdef NTPLAYER_EXPORTS
#define NTPLAYER_API __declspec(dllexport)
#else
#define NTPLAYER_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

NTPLAYER_API int ntplayer_init();
NTPLAYER_API int ntplayer_uninit();
NTPLAYER_API int ntplayer_get_codecs_info_string(const char** info);



#ifdef __cplusplus
}
#endif

#endif
