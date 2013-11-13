#ifndef __NTPLAYERCORE_H__
#define __NTPLAYERCORE_H__

#ifdef NTPLAYERCORE_EXPORTS
#define NTPLAYERCORE_API __declspec(dllexport)
#else
#define NTPLAYERCORE_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    NTPLAYERCORE_API long ntplayer_init();
    NTPLAYERCORE_API long ntplayer_uninit();
    NTPLAYERCORE_API long ntplayer_get_codecs_info_string(const char** info);
    NTPLAYERCORE_API long ntplayer_open(const char* url);
    NTPLAYERCORE_API long ntplayer_close();
    NTPLAYERCORE_API long ntplayer_play();
    NTPLAYERCORE_API long ntplayer_pause();
    NTPLAYERCORE_API long ntplayer_stop();
    NTPLAYERCORE_API long ntplayer_get_duration(long* duration);    // ms
    NTPLAYERCORE_API long ntplayer_set_play_pos(long pos_to_play);  // ms
    NTPLAYERCORE_API long ntplayer_get_current_play_pos(long* current_play_pos);    // ms



#ifdef __cplusplus
}
#endif

#endif
