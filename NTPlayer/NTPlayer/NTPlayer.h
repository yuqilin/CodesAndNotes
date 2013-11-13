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

NTPLAYER_API long ntplayer_init();
NTPLAYER_API long ntplayer_uninit();
NTPLAYER_API long ntplayer_get_codecs_info_string(const char** info);
NTPLAYER_API long ntplayer_open(const char* url);
NTPLAYER_API long ntplayer_close();



#ifdef __cplusplus
}
#endif

#endif
