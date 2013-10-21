#ifndef __NTPLAYER_H__
#define __NTPLAYER_H__

#ifdef NTPLAYER_EXPORTS
#define NTPLAYER_API __declspec(dllexport)
#else
#define NTPLAYER_API __declspec(dllimport)
#endif

typedef long NTPLAYER_RESULT;

#ifdef __cplusplus
extern "C"
{
#endif

NTPLAYER_API NTPLAYER_RESULT ntplayer_init();
NTPLAYER_API NTPLAYER_RESULT ntplayer_uninit();
NTPLAYER_API NTPLAYER_RESULT ntplayer_get_codecs_info_string(const char** info);
NTPLAYER_API NTPLAYER_RESULT ntplayer_open(const char* url);
NTPLAYER_API NTPLAYER_RESULT ntplayer_close();



#ifdef __cplusplus
}
#endif

#endif
