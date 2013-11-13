#ifndef _PLAYERCORE_PLAYERINTERFACE_H_
#define _PLAYERCORE_PLAYERINTERFACE_H_

#include "PlayerCommon.h"
#include "MediaInfo.h"

//////////////////////////////////////////////////////////////////////////
class IPlayControl
{
public:
    virtual PlayerResult OpenMedia(MediaInfo* url) = 0;
    virtual PlayerResult CloseMedia() = 0;
    virtual PlayerResult Play() = 0;
    virtual PlayerResult Pause() = 0;
    virtual PlayerResult Stop() = 0;
    virtual PlayerResult Abort() = 0;
    virtual PlayerResult GetPlayState(PlayerState* state) = 0;
    virtual PlayerResult GetDuration(long* duration) = 0;   // ms
    virtual PlayerResult GetCurrentPlayPos(long* current_play_pos) = 0; // ms
    virtual PlayerResult SetPlayPos(long pos_to_play) = 0; // ms
};

//////////////////////////////////////////////////////////////////////////
class IVideoControl
{
public:
    virtual PlayerResult SetVideoWindow(void* video_window) = 0;
    virtual PlayerResult SetVideoPosition(int ) = 0;
    virtual PlayerResult GetVideoSize(int* w, int* h) = 0;
    virtual PlayerResult SetColorControl(int brightness, int contrast, int hue, int staturation) = 0;
    virtual PlayerResult LoadExternalSubtitle(const char* subtitle_path) = 0;
    virtual PlayerResult GrabCurrentVideoFrame(const char* save_file_name) = 0;
};

//////////////////////////////////////////////////////////////////////////
class IAudioControl
{
public:
    virtual PlayerResult GetMute(bool* mute) = 0;
    virtual PlayerResult SetMute(bool mute) = 0;
    virtual PlayerResult GetVolume(int* volume) = 0;
    virtual PlayerResult SetVolume(int volume) = 0;
};


#endif