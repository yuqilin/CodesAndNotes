#pragma once

#include "Mutex.h"
#include "PlayerSettings.h"
#include "PlayerCodecs.h"
#include "NTPlayerCore.h"

struct VideoSize;

class MediaInfo;

class PlayerBaseStream;

class PlayerThread;

class BaseGraph;

class ViewWindow;

//////////////////////////////////////////////////////////////////////////
enum MediaLoadState_t {
    kMediaLoadStateClosed,
    kMediaLoadStateLoading,
    kMediaLoadStateLoaded,
    kMediaLoadStateClosing,
};

struct OpenMediaData{
    CString strUrl;
    CString strDownloadSavePath;
};


//////////////////////////////////////////////////////////////////////////
class PlayerCore
{
    friend class PlayerThread;
    friend class DirectShowGraph;
    friend class PlayerCodecs;
public:
    PlayerCore();
    ~PlayerCore();

    HRESULT Create(ntplayer_notify_to_ui notify_func, void* pUser);
    HRESULT Destroy();

    // play control
    HRESULT Open(LPCTSTR pUrl, LPCTSTR pDownloadSavePath = NULL);
    HRESULT Close();
    HRESULT Play();
    HRESULT Pause();
    HRESULT Stop();
    //HRESULT Abort();
    ntplayer_state GetPlayerState();
    HRESULT    GetDuration(LONG* pnDuration);
    HRESULT    GetCurrentPlayPos(LONG* pnCurPlayPos);
    HRESULT SetPlayPos(LONG nPlayPos);
    //bool    IsPlaying();

    // video display
    HRESULT SetVideoWindow(HWND hVideoWindow);
    HRESULT SetVideoPosition(LPRECT lprcDisplay, BOOL bUpdate);
    HRESULT GetVideoSize(VideoSize* pVideoSize);
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


    /*
     *	static
     */
    static PlayerSettings& GetPlayerSettings();
    static PlayerCodecs& GetPlayerCodecs();

//     static void SetModuleInstance(void* instance) {
//         instance_ = instance;
//     }
//     static void* GetModuleInstance() {
//         return instance_;
//     }

//     static void SetLogCallback(player_log_callback log) {
//         log_ = log;
//     }
//     static player_log_callback GetLogCallback() {
//         return log_;
//     }

protected:
    HRESULT CreatePlayerThread();
    HRESULT DestroyPlayerThread();

    HRESULT DoOpen(CAutoPtr<OpenMediaData> pOMD);
    HRESULT DoClose();
    HRESULT DoPlay();
    HRESULT DoPause();
    HRESULT DoStop();
    HRESULT DoAbort();
    HRESULT DoSeek(long pos);

    void SetPlayerState(ntplayer_state state);

    PlayerBaseStream* GetStream() {
        return m_pStream;
    }
    HWND GetVideoWindow() {
        return m_hVideoWindow;
    }
    const RECT& GetDisplayRect() {
        return m_rcDisplay;
    }

    void OnOpenResult(HRESULT hr);

//     HRESULT CreateViewWindow();
//     HRESULT DestroyViewWindow();

protected:
    PlayerThread*           m_pPlayerThread;

    MediaInfo*              m_pMediaInfo;
    PlayerBaseStream*       m_pStream;
    BaseGraph*              m_pPlayerGraph;

    volatile ntplayer_state          m_PlayerState;
    FastMutex               m_PlayerStateMutex;

    HWND                    m_hVideoWindow;
    RECT                    m_rcDisplay;

    ntplayer_notify_to_ui   m_pfnNotifyUI;
    void*                   m_pUser;

    LONG                    m_lCurrentPlayPos;

    //CString                 m_strDownloadSavePath;

private:
    BOOL                    m_bCreated;
    volatile BOOL           m_bOpeningAborted;
};

