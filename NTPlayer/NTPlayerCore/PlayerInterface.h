#pragma once

#include "MediaInfo.h"

//////////////////////////////////////////////////////////////////////////
class IPlayControl
{
public:
    virtual HRESULT OpenMedia(MediaInfo* url) = 0;
    virtual HRESULT CloseMedia() = 0;
    virtual HRESULT Play() = 0;
    virtual HRESULT Pause() = 0;
    virtual HRESULT Stop() = 0;
    virtual HRESULT Abort() = 0;
    virtual HRESULT GetPlayState(PlayerState* state) = 0;
    virtual HRESULT GetDuration(long* duration) = 0;   // ms
    virtual HRESULT GetCurrentPlayPos(long* current_play_pos) = 0; // ms
    virtual HRESULT SetPlayPos(long pos_to_play) = 0; // ms
};

//////////////////////////////////////////////////////////////////////////
class IVideoControl
{
public:
    virtual HRESULT SetVideoWindow(void* video_window) = 0;
    virtual HRESULT SetVideoPosition(int ) = 0;
    virtual HRESULT GetVideoSize(int* w, int* h) = 0;
    virtual HRESULT SetColorControl(int brightness, int contrast, int hue, int staturation) = 0;
    virtual HRESULT LoadExternalSubtitle(const char* subtitle_path) = 0;
    virtual HRESULT GrabCurrentVideoFrame(const char* save_file_name) = 0;
};

//////////////////////////////////////////////////////////////////////////
class IAudioControl
{
public:
    virtual HRESULT GetMute(bool* mute) = 0;
    virtual HRESULT SetMute(bool mute) = 0;
    virtual HRESULT GetVolume(int* volume) = 0;
    virtual HRESULT SetVolume(int volume) = 0;
};
