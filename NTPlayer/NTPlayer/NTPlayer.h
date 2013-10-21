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

NTPLAYER_API HRESULT ntplayer_init();
NTPLAYER_API HRESULT ntplayer_uninit();
NTPLAYER_API HRESULT ntplayer_get_codecs_info_string(const char** info);
NTPLAYER_API HRESULT ntplayer_open_url(const char* url);



#ifdef __cplusplus
}
#endif

#endif
