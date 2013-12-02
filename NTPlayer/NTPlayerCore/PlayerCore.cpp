#include "stdafx.h"
#include <assert.h>
#include "PlayerCore.h"
#include "PlayerThread.h"
#include "ViewWindow.h"
#include "SingletonHolder.h"
#include "DirectShowGraph.h"
#include "PlayerFileStream.h"
#include "PlayerQvodStream.h"
#include "HookApi.h"

//////////////////////////////////////////////////////////////////////////
const TCHAR* PlayerStateString(ntplayer_state state);

static DWORD s_dwStart = 0;
static DWORD s_dwStop = 0;


//////////////////////////////////////////////////////////////////////////
PlayerCore::PlayerCore()
: m_pPlayerThread(NULL)
, m_pMediaInfo(NULL)
, m_pPlayerGraph(NULL)
, m_pStream(NULL)
, m_PlayerState(kPlayerStateNothingSpecial)
, m_hVideoWindow(NULL)
, m_pfnNotifyUI(NULL)
, m_pUser(NULL)
, m_bCreated(FALSE)
, m_bOpeningAborted(FALSE)
, m_lCurrentPlayPos(0)
{
    memset(&m_rcDisplay, 0, sizeof(RECT));
}

PlayerCore::~PlayerCore()
{
    Destroy();
}

HRESULT PlayerCore::Create(ntplayer_notify_to_ui notify_func, void* pUser)
{
    HRESULT hr = NOERROR;

    if (!m_bCreated)
    {
        m_pfnNotifyUI = notify_func;
        m_pUser = pUser;

        //assert(thread_ == 0);
        assert(m_pMediaInfo == NULL);
        assert(m_pPlayerGraph == 0);
        assert(m_pStream == 0);

        // Remove the working directory from the search path to work around the DLL preloading vulnerability
        SetDllDirectory(_T(""));

        // load default settings        
        hr = GetPlayerSettings().LoadSettings();
        if (FAILED(hr))
        {
            player_log(kLogLevelTrace, _T("PlayerCore::Create, Load PlayerSettings failed, hr = 0x%08X"), hr);
        }

        // load codecs
        hr = GetPlayerCodecs().LoadCodecs();
        if (FAILED(hr))
        {
            player_log(kLogLevelError, _T("PlayerCore::Create, Load PlayerCodecs failed, hr = 0x%08X"), hr);
        }

        hr = CreatePlayerThread();
        if (FAILED(hr))
        {
            player_log(kLogLevelError, _T("PlayerCore::Create, CreatePlayerThread failed, hr = 0x%08X"), hr);
        }

//         hr = CreateViewWindow();
//         if (FAILED(hr))
//         {
//             player_log(kLogLevelError, _T("PlayerCore::Create, CreateViewWindow failed, hr = 0x%08X"), hr);
//         }

        if (SUCCEEDED(hr))
            m_bCreated = true;
    }

    if (!m_bCreated)
    {
        hr = E_FAIL;
    }

    return hr;
}

HRESULT PlayerCore::Destroy()
{
    HRESULT hr = NOERROR;

    if (m_bCreated)
    {
        Close();

        DestroyPlayerThread();

        //DestroyViewWindow();

        SAFE_DELETE(m_pPlayerGraph);
        SAFE_DELETE(m_pStream);
        SAFE_DELETE(m_pMediaInfo);

        GetPlayerCodecs().FreeCodecs();

        m_pfnNotifyUI = NULL;
        m_pUser = NULL;
        m_bCreated = false;

        //UnloadUnusedExternalObjects();
        UnloadExternalObjects();

        SetDllDirectory(NULL);
    }

    return hr;
}

// HRESULT PlayerCore::CreateViewWindow()
// {
//     assert(m_pViewWindow == NULL);
//     assert(m_hVideoWindow != NULL);
// 
//     if (m_hVideoWindow == NULL)
//     {
//         return E_FAIL;
//     }
// 
//     m_pViewWindow = new ViewWindow;
//     if (!m_pViewWindow)
//     {
//         return E_OUTOFMEMORY;
//     }
// 
//     HWND hWnd = m_pViewWindow->Create(m_hVideoWindow, &m_rcDisplay);
//     if (hWnd == NULL)
//     {
//         return E_FAIL;
//     }
// 
//     return S_OK;
// }
// 
// HRESULT PlayerCore::DestroyViewWindow()
// {
//     HRESULT hr = S_OK;
// 
//     if (m_pViewWindow)
//     {
//         delete m_pViewWindow;
//     }
// 
//     return hr;
// }

HRESULT PlayerCore::CreatePlayerThread()
{
    assert(m_pPlayerThread == NULL);

    if (m_pPlayerThread)
        return S_FALSE;

    m_pPlayerThread = new PlayerThread(this);
    if (m_pPlayerThread == NULL)
    {
        return E_OUTOFMEMORY;
    }

    HookAPI();

    if (!m_pPlayerThread->CreateThread())
    {
        player_log(kLogLevelFatal, _T("PlayerCore::CreatePlayerThread, create thread failed"));
        return E_FAIL;
    }

    return S_OK;
}

HRESULT PlayerCore::DestroyPlayerThread()
{
    if (!m_pPlayerThread)
        return S_FALSE;

    if (m_pPlayerThread)
    {
        CAMMsgEvent evt;
        m_pPlayerThread->PutThreadMsg(PlayerThread::kMsgExit, 0, NULL, &evt);
        if (!evt.Wait(2000))
        {
            player_log(kLogLevelTrace, _T("PlayerCore::DestroyPlayerThread, wait timeout, try to terminate thread"));
            m_pPlayerThread->Terminate((DWORD)-1);
        }

        UnHookAPI();
    }
    SAFE_DELETE(m_pPlayerThread);

    return NOERROR;
}

HRESULT PlayerCore::Open(LPCTSTR pUrl, LPCTSTR pDownloadSavePath)
{
    s_dwStart = timeGetTime();

    HRESULT hr = E_FAIL;

    m_strDownloadSavePath = pDownloadSavePath;

    do {
        if (pUrl == NULL ||
            lstrlen(pUrl) <= 0)
        {
            break;
        }

        if (!m_pPlayerThread || !m_pPlayerThread->IsRunning())
        {
            player_log(kLogLevelFatal, _T("PlayerCore::Open, player thread not running"));
            break;
        }

        if (m_PlayerState != kPlayerStateNothingSpecial)
        {
            Close();
        }

        CAutoPtr<CString> autoUrl(new CString(pUrl));

        SetPlayerState(kPlayerStateOpening);

        m_pPlayerThread->PutThreadMsg(PlayerThread::kMsgOpen, 0, (LPVOID)autoUrl.Detach());

        hr = S_OK;

    } while (0);

    if (FAILED(hr))
    {
        OnOpenResult(hr);
    }

    return hr;
}

HRESULT PlayerCore::Close()
{
    if (kPlayerStateNothingSpecial == m_PlayerState ||
        kPlayerStateClosing == m_PlayerState)
    {
        player_log(kLogLevelTrace, _T("PlayerCore::Close, do nothing (%s)"), PlayerStateString(m_PlayerState));
        return S_OK;
    }

    HRESULT hr = S_OK;
    BOOL bTerminated = FALSE;
    int time_waited = 0;

    if (m_pStream)
        m_pStream->Abort();

    if (m_pPlayerGraph)
        m_pPlayerGraph->Abort();

    while (kPlayerStateOpening == m_PlayerState)
    {
        m_bOpeningAborted = TRUE;

        if (time_waited > 2000)
        {
            DestroyPlayerThread();
            break;
        }

        Sleep(50);

        time_waited += 50;
    }

    if (!m_pPlayerThread)
    {
        CreatePlayerThread();
    }

    m_bOpeningAborted = FALSE;

    SetPlayerState(kPlayerStateClosing);
    //m_strDownloadSavePath.Empty();

    if (m_pPlayerThread)
    {
        CAMMsgEvent evt;
        m_pPlayerThread->PutThreadMsg(PlayerThread::kMsgClose, 0, NULL, &evt);
        evt.WaitMsg();
    }

    return hr;
}

HRESULT PlayerCore::Play()
{
    if (kPlayerStateNothingSpecial ==  m_PlayerState ||
        kPlayerStateClosing == m_PlayerState ||
        kPlayerStatePlaying == m_PlayerState)
    {
        player_log(kLogLevelTrace, _T("PlayerCore::Play, do nothing (%s)"), PlayerStateString(m_PlayerState));
        return S_OK;
    }

    HRESULT hr = E_FAIL;

    if (!m_pPlayerThread || !m_pPlayerThread->IsRunning())
    {
        player_log(kLogLevelFatal, _T("PlayerCore::Play, player thread not running"));
        return E_FAIL;
    }

    m_pPlayerThread->PutThreadMsg(PlayerThread::kMsgPlay, 0, NULL);

    return S_OK;
}

HRESULT PlayerCore::Pause()
{
    if (kPlayerStateNothingSpecial ==  m_PlayerState ||
        kPlayerStateClosing == m_PlayerState ||
        kPlayerStatePaused == m_PlayerState)
    {
        player_log(kLogLevelTrace, _T("PlayerCore::Pause, do nothing (%s)"), PlayerStateString(m_PlayerState));
        return S_OK;
    }

    HRESULT hr = E_FAIL;

    if (!m_pPlayerThread || !m_pPlayerThread->IsRunning())
    {
        player_log(kLogLevelFatal, _T("PlayerCore::Pause, player thread not running"));
        return E_FAIL;
    }

    m_pPlayerThread->PutThreadMsg(PlayerThread::kMsgPause, 0, NULL);

    return S_OK;
}

HRESULT PlayerCore::Stop()
{
    HRESULT hr = E_FAIL;

    if (kPlayerStateNothingSpecial == m_PlayerState ||
        kPlayerStateClosing == m_PlayerState)
    {
        player_log(kLogLevelTrace, _T("PlayerCore::Stop, do nothing (%s)"), PlayerStateString(m_PlayerState));
        return S_OK;
    }

//     if (m_pStream)
//         m_pStream->Abort();

    if (m_pPlayerGraph)
        m_pPlayerGraph->Abort();

    if (!m_pPlayerThread || !m_pPlayerThread->IsRunning())
    {
        player_log(kLogLevelFatal, _T("PlayerCore::Stop, player thread not running"));
        return E_FAIL;
    }

    m_pPlayerThread->PutThreadMsg(PlayerThread::kMsgStop, 0, NULL);

    return S_OK;
}

// HRESULT PlayerCore::Abort()
// {
//     HRESULT hr = E_FAIL;
// 
//     m_pPlayerThread->PutThreadMsg(PlayerThread::kMsgStop, 0, NULL);
// 
//     return hr;
// }

ntplayer_state PlayerCore::GetPlayerState()
{
    //FastMutex::ScopedLock lock(m_PlayerStateMutex);
    return m_PlayerState;
}

HRESULT PlayerCore::GetDuration(LONG* pnDuration)
{
    HRESULT hr = S_OK;

    LONG nDuration = 0;
    if (m_pMediaInfo)
    {
        nDuration = m_pMediaInfo->GetDuration();
    }
    if (pnDuration)
        *pnDuration = nDuration;

    //player_log(kLogLevelTrace, _T("PlayerCore::GetDuration = %d"), nDuration);
    return hr;
}

HRESULT PlayerCore::GetCurrentPlayPos(LONG *pnCurPlayPos)
{
    HRESULT hr = S_OK;

    if (m_pPlayerGraph)
    {
        LONG lCurPlayPos = 0;
        hr = m_pPlayerGraph->GetCurrentPlayPos(&lCurPlayPos);
        if (SUCCEEDED(hr))
        {
            m_lCurrentPlayPos = lCurPlayPos;
        }
    }

    if (pnCurPlayPos)
        *pnCurPlayPos = m_lCurrentPlayPos;

//     player_log(kLogLevelTrace, _T("PlayerCore::GetCurrentPlayPos, current play pos = %d(%s)"), 
//         m_lCurrentPlayPos, Millisecs2CString(m_lCurrentPlayPos));

    return hr;
}

HRESULT PlayerCore::SetPlayPos(long pos_to_play)
{
    HRESULT hr = E_FAIL;

    if (!m_pPlayerThread || !m_pPlayerThread->IsRunning())
    {
        player_log(kLogLevelFatal, _T("PlayerCore::Stop, player thread not running"));
        return E_FAIL;
    }

    m_pPlayerThread->PutThreadMsg(PlayerThread::kMsgSeek, 0, (LPVOID)pos_to_play, NULL);

    return hr;
}

HRESULT PlayerCore::DoOpen(CAutoPtr<CString> strUrl)
{
    player_log(kLogLevelTrace, _T("PlayerCore::DoOpen"));

    HRESULT hr = E_FAIL;

    if (m_bOpeningAborted)
    {
        player_log(kLogLevelTrace, _T("PlayerCore::DoOpen, open aborted before create media info"));
        return E_ABORT;
    }

    // Create MediaInfo
    m_pMediaInfo = new MediaInfo(strUrl->GetBuffer(), hr);
    if (FAILED(hr) || !m_pMediaInfo)
    {
        player_log(kLogLevelError, _T("Create MediaInfo FAIL, hr = 0x%08x"), hr);
        return hr;
    }

    if (m_bOpeningAborted)
    {
        player_log(kLogLevelTrace, _T("PlayerCore::DoOpen, open aborted before create stream"));
        return E_ABORT;
    }

    // Create Stream
    MediaProtocol protocol = ProtocolFromString(m_pMediaInfo->GetProtocol());
    if (protocol == kProtocolFile)
    {
        m_pStream = new PlayerFileStream;

    }
    else if (protocol == kProtocolQvod)
    {
        m_pStream = new PlayerQvodStream;
        if (m_pStream)
        {
            ((PlayerQvodStream*)m_pStream)->SetDownloadSavePath(m_strDownloadSavePath);
        }
    }
    else
    {
        return E_FAIL;
    }
    
    if (FAILED(hr = m_pStream->Open(m_pMediaInfo->GetUrl())))
    {
        return hr;
    }

    if (m_bOpeningAborted)
    {
        player_log(kLogLevelTrace, _T("PlayerCore::DoOpen, open aborted before create graph builder"));
        return E_ABORT;
    }

    // Create Graph
    m_pPlayerGraph = new DirectShowGraph(this, hr);
    if (FAILED(hr) || !m_pPlayerGraph)
    {
        player_log(kLogLevelTrace, _T("Create DirectShowGraph FAIL, hr = 0x%08x"), hr);
        return hr;
    }

    if (m_bOpeningAborted)
    {
        player_log(kLogLevelTrace, _T("PlayerCore::DoOpen, open aborted before graph OpenMedia"));
        return E_ABORT;
    }

    {
        //PlayerCore::GetPlayerCodecs().ChangeCurrentDirectory();
        hr = m_pPlayerGraph->OpenMedia(m_pMediaInfo);
        //PlayerCore::GetPlayerCodecs().ResotreCurrentDirectory();
    }

    if (SUCCEEDED(hr))
    {
    }
    return hr;
}

HRESULT PlayerCore::DoClose()
{
    player_log(kLogLevelTrace, _T("PlayerCore::DoClose"));
    HRESULT hr = NOERROR;

    SAFE_DELETE(m_pPlayerGraph);
    SAFE_DELETE(m_pStream);
    SAFE_DELETE(m_pMediaInfo);

    m_lCurrentPlayPos = 0;
    m_strDownloadSavePath.Empty();
    
    SetPlayerState(kPlayerStateNothingSpecial);

    if (::IsWindow(m_hVideoWindow))
        ::InvalidateRect(m_hVideoWindow, NULL, TRUE);

    return hr;
}

HRESULT PlayerCore::DoPlay()
{
    player_log(kLogLevelTrace, _T("PlayerCore::DoPlay"));
    HRESULT hr = S_OK;
    
    if (kPlayerStatePlaying == GetPlayerState())
    {
        player_log(kLogLevelTrace, _T("PlayerCore::DoPlay, already playing"));
        return S_OK;
    }

    if (m_pPlayerGraph)
    {
        hr = m_pPlayerGraph->Play();
        if (SUCCEEDED(hr))
        {
            SetPlayerState(kPlayerStatePlaying);
        }
    }
    return hr;
}

HRESULT PlayerCore::DoPause()
{
    player_log(kLogLevelTrace, _T("PlayerCore::DoPause"));
    HRESULT hr = S_OK;

    if (kPlayerStatePaused == GetPlayerState())
    {
        player_log(kLogLevelTrace, _T("PlayerCore::DoPlay, already paused"));
        return S_OK;
    }

    if (m_pPlayerGraph)
    {
        hr = m_pPlayerGraph->Pause();
        if (SUCCEEDED(hr))
        {
            SetPlayerState(kPlayerStatePaused);
        }
    }
    return hr;
}

HRESULT PlayerCore::DoStop()
{
    player_log(kLogLevelTrace, _T("PlayerCore::DoStop"));

    HRESULT hr = S_OK;
    if (m_pPlayerGraph)
    {
        hr = m_pPlayerGraph->Stop();
        if (SUCCEEDED(hr))
        {
            SetPlayerState(kPlayerStateStopped);
        }
    }
    return hr;
}

HRESULT PlayerCore::DoSeek(long pos)
{
    HRESULT hr = S_OK;

    player_log(kLogLevelTrace, _T("PlayerCore::DoSeek = %d"), pos);

    if (m_pPlayerGraph)
    {
        hr = m_pPlayerGraph->SetPlayPos(pos);
    }

    return hr;
}

HRESULT PlayerCore::DoAbort()
{
    player_log(kLogLevelTrace, _T("PlayerCore::DoAbort"));
    HRESULT hr = S_OK;
    if (m_pPlayerGraph)
    {
        hr = m_pPlayerGraph->Abort();
    }
    return hr;
}

void PlayerCore::OnOpenResult(HRESULT hr)
{
    int msg = kPlayerNotifyOpenFailed;
    if (SUCCEEDED(hr))
    {
        msg = kPlayerNotifyOpenSucceeded;
        
        s_dwStop = timeGetTime();
        player_log(kLogLevelTrace, _T("OpenMedia Succeeded, cost time = %d ms"), s_dwStop - s_dwStart);
    }
//     else if (hr == E_ABORT)
//     {
//         SetPlayerState(kPlayerStateClosing);
//     }
    else
    {
        SetPlayerState(kPlayerStateNothingSpecial);
    }

    if (m_pfnNotifyUI)
    {
        m_pfnNotifyUI(m_pUser, msg, NULL);
    }
}

void PlayerCore::SetPlayerState(ntplayer_state state)
{
    //FastMutex::ScopedLock lock(m_PlayerStateMutex);
    if (m_PlayerState != state)
    {
        m_PlayerState = state;
        m_pfnNotifyUI(m_pUser, kPlayerNotifyPlayerStateChanged, (void*)&m_PlayerState);
    }
    player_log(kLogLevelTrace, _T("PlayerCore::SetPlayerState = %s"), PlayerStateString(state));
}

HRESULT PlayerCore::SetVideoWindow(HWND hVideoWindow)
{
    m_hVideoWindow = hVideoWindow;

    DWORD dwStyles = 0;
    dwStyles = ::GetWindowLong(hVideoWindow, GWL_STYLE);
    dwStyles |= (WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
    LONG lRet = ::SetWindowLong(hVideoWindow, GWL_STYLE, dwStyles);
    DWORD dwLastError = 0;
    if (lRet == 0)
    {
         dwLastError = ::GetLastError();
    }

    return S_OK;
}

HRESULT PlayerCore::SetVideoPosition(LPRECT prcDisplay, BOOL bUpdate)
{
    CheckPointer(prcDisplay, E_POINTER);
    m_rcDisplay = *prcDisplay;

    HRESULT hr = S_OK;
    if (bUpdate)
    {
        if (m_pPlayerGraph)
        {
            hr = m_pPlayerGraph->SetVideoPosition(&m_rcDisplay);
        }
    }
    return hr;
}

HRESULT PlayerCore::GetVideoSize(VideoSize* pVideoSize)
{
    if (m_pMediaInfo)
    {
        VideoSize* pvs = m_pMediaInfo->GetVideoSize();
        if (pvs && pvs->valid() && pVideoSize)
        {
            *pVideoSize = *pvs;
            return S_OK;
        }
    }

    if (m_pPlayerGraph)
    {
        if (SUCCEEDED(m_pPlayerGraph->GetVideoSize(pVideoSize)))
        {
            return S_OK;
        }
    }

    return E_FAIL;
}

HRESULT PlayerCore::SetColorControl(int brightness, int contrast, int hue, int staturation)
{
    return E_NOTIMPL;
}

//////////////////////////////////////////////////////////////////////////
PlayerSettings& PlayerCore::GetPlayerSettings()
{
    static SingletonHolder<PlayerSettings> s_settings;
    return *s_settings.get();
}

PlayerCodecs& PlayerCore::GetPlayerCodecs()
{
    static SingletonHolder<PlayerCodecs> s_codecs;
    return *s_codecs.get();
}



const TCHAR* PlayerStateString(ntplayer_state state)
{
    switch(state)
    {
    case kPlayerStateNothingSpecial:
        return _T("NothingSpecial");
    case kPlayerStateOpening:
        return _T("Opening");
    case kPlayerStatePlaying:
        return _T("Playing");
    case kPlayerStatePaused:
        return _T("Paused");
    case kPlayerStateStopped:
        return _T("Stopped");
    case kPlayerStateClosing:
        return _T("Closing");
    }
    return _T("Unknown");
}

