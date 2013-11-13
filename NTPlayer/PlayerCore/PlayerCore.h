#ifndef _PLAYERCORE_H_
#define _PLAYERCORE_H_

#include "PlayerCommon.h"
#include "PlayerThread.h"
#include "PlayerSettings.h"
#include "PlayerCodecs.h"
#include "Mutex.h"
#include "MediaInfo.h"
#include "BaseStream.h"
#include "BaseGraph.h"

//
class PlayerCore
{
    friend class PlayerThread;
public:
    PlayerCore();
    ~PlayerCore();

    PlayerResult Create();
    PlayerResult Destroy();

    // play control
    PlayerResult Open(const char* url);
    PlayerResult Close();
    PlayerResult Play();
    PlayerResult Pause();
    PlayerResult Stop();
    PlayerResult Abort();
    PlayerResult GetPlayState(PlayerState* state);
    PlayerResult GetDuration(long* duration);
    PlayerResult GetCurrentPlayPos(long* current_play_pos);
    PlayerResult SetPlayPos(long pos_to_play);
    bool         IsPlaying();

    // video display
    PlayerResult SetVideoWindow(HWND video_window);
    PlayerResult SetVideoPosition(int );
    PlayerResult GetVideoSize(int* w, int* h);
    PlayerResult SetColorControl(int brightness, int contrast, int hue, int staturation);

    // subtitle
    PlayerResult LoadExternalSubtitle(const char* subtitle_path);

    // grab image
    PlayerResult GrabCurrentVideoFrame(const char* save_file_name);

    // audio
    PlayerResult GetMute(bool* mute);
    PlayerResult SetMute(bool mute);
    PlayerResult GetVolume(int* volume);
    PlayerResult SetVolume(int volume);

protected:

    PlayerResult CreatePlayerThread();
    PlayerResult DestroyPlayerThread();

    PlayerResult OpenMedia(const char* url);
    PlayerResult CloseMedia();

    void SetPlayerState(PlayerState state);

protected:
    PlayerThread* thread_;
    MediaInfo* media_info_;
    BaseGraph* graph_;
    BaseStream* stream_;

    PlayerState state_;
    FastMutex   state_mutex_;

private:
    bool created_;

    bool opening_aborted_;


public:
    static PlayerSettings& GetPlayerSettings();
    static PlayerCodecs& GetPlayerCodecs();
};

#endif