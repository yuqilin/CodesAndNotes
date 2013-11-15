#pragma once

#include "Mutex.h"
#include "PlayerSettings.h"
#include "PlayerCodecs.h"


class MediaInfo;

class PlayerBaseStream;

class BaseGraph;

//////////////////////////////////////////////////////////////////////////
class PlayerCore
{
public:
    PlayerCore();
    ~PlayerCore();

    HRESULT Create();
    HRESULT Destroy();

    // play control
    HRESULT Open(const TCHAR* url);
    HRESULT Close();
    HRESULT Play();
    HRESULT Pause();
    HRESULT Stop();
    HRESULT Abort();
    HRESULT GetPlayState(PlayerState* state);
    HRESULT GetDuration(long* duration);
    HRESULT GetCurrentPlayPos(long* current_play_pos);
    HRESULT SetPlayPos(long pos_to_play);
    //bool    IsPlaying();

    // video display
    HRESULT SetVideoWindow(HWND video_window);
    HRESULT SetVideoPosition(int );
    HRESULT GetVideoSize(int* w, int* h);
    HRESULT SetColorControl(int brightness, int contrast, int hue, int staturation);

    // subtitle
    HRESULT LoadExternalSubtitle(const char* subtitle_path);

    // grab image
    HRESULT GrabCurrentVideoFrame(const char* save_file_name);

    // audio
    HRESULT GetMute(bool* mute);
    HRESULT SetMute(bool mute);
    HRESULT GetVolume(int* volume);
    HRESULT SetVolume(int volume);

    static void SetModuleInstance(void* instance) {
        instance_ = instance;
    }
    static void* GetModuleInstance() {
        return instance_;
    }

    PlayerBaseStream* GetStream() {
        return m_pStream;
    }

//     static void SetLogCallback(player_log_callback log) {
//         log_ = log;
//     }
// 
//     static player_log_callback GetLogCallback() {
//         return log_;
//     }

protected:
    HRESULT CreatePlayerThread();
    HRESULT DestroyPlayerThread();

    HRESULT DoOpen(CAutoPtr<CString> strUrl);
    HRESULT DoClose();
    HRESULT DoPlay();
    HRESULT DoPause();
    HRESULT DoStop();
    HRESULT DoAbort();

    void SetPlayerState(PlayerState state);

protected:
    BaseGraph* m_PlayerGraph;
    PlayerBaseStream* m_pStream;

    PlayerState state_;
    FastMutex   state_mutex_;

    static void* instance_;

    PlayerThread* m_PlayerThread;
    MediaInfo* m_MediaInfo;


private:
    bool m_fCreated;

    bool opening_aborted_;


public:
    static PlayerSettings& GetPlayerSettings();
    static PlayerCodecs& GetPlayerCodecs();
};

