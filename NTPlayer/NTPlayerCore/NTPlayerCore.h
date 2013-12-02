#ifndef __NTPLAYERCORE_H__
#define __NTPLAYERCORE_H__

#ifdef NTPLAYERCORE_EXPORTS
#define NTPLAYERCORE_API __declspec(dllexport)
#else
#define NTPLAYERCORE_API __declspec(dllimport)
#pragma comment(lib, "ntplayercore.lib")
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    enum {
        kPlayerNotifyOpenSucceeded = 1,
        kPlayerNotifyOpenFailed,
        kPlayerNotifyPlayerStateChanged,
    };

    typedef enum {
        kPlayerStateNothingSpecial,
        kPlayerStateOpening,
        //kPlayerStateBuffering,
        kPlayerStatePlaying,
        kPlayerStatePaused,
        kPlayerStateStopped,
        //kPlayerStateEnded,
        kPlayerStateClosing,
        kPlayerStateError,
    } ntplayer_state;

    typedef void (*ntplayer_notify_to_ui)(void* pUser, int msg, void* pParam);

    NTPLAYERCORE_API long ntplayer_init(ntplayer_notify_to_ui notify_func, void* pUser);
    NTPLAYERCORE_API long ntplayer_uninit();
    NTPLAYERCORE_API long ntplayer_get_codecs_info_string(const char** info);
    NTPLAYERCORE_API long ntplayer_open(const char* url, const char* download_save_path = NULL);
    NTPLAYERCORE_API long ntplayer_close();
    NTPLAYERCORE_API long ntplayer_play();
    NTPLAYERCORE_API long ntplayer_pause();
    NTPLAYERCORE_API long ntplayer_stop();
    NTPLAYERCORE_API long ntplayer_get_duration(long* duration);    // ms
    NTPLAYERCORE_API long ntplayer_set_play_pos(long pos_to_play);  // ms
    NTPLAYERCORE_API long ntplayer_get_current_play_pos(long* current_play_pos);    // ms
    NTPLAYERCORE_API long ntplayer_set_video_display(void* video_window, void* display_rect, bool is_full_screen);
    NTPLAYERCORE_API long ntplayer_update_video_display(void* display_rect, bool is_full_screen);
    NTPLAYERCORE_API ntplayer_state ntplayer_get_player_state();





#ifdef __cplusplus
}
#endif

#endif
