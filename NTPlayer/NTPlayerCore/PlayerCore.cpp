#include "stdafx.h"
#include <assert.h>
#include "PlayerCore.h"
#include "SingletonHolder.h"
#include "DirectShowGraph.h"
#include "PlayerFileStream.h"
#include "PlayerQvodStream.h"

//////////////////////////////////////////////////////////////////////////
static SingletonHolder<PlayerSettings> s_settings;
static SingletonHolder<PlayerCodecs> s_codecs;

void* PlayerCore::instance_ = NULL;

const TCHAR* PlayerStateString(PlayerState state);

//static CXTimer g_XTimer;
static DWORD s_dwStart = 0;
static DWORD s_dwStop = 0;

//////////////////////////////////////////////////////////////////////////
PlayerCore::PlayerCore()
: m_PlayerGraph(0)
, m_pStream(0)
, m_fCreated(false)
, state_(kPlayerStateNothingSpecial)
, m_PlayerThread(NULL)
, m_MediaInfo(NULL)
{

}

PlayerCore::~PlayerCore()
{
    Destroy();
}

HRESULT PlayerCore::Create()
{
    HRESULT hr = NOERROR;

    if (!m_fCreated)
    {
        //assert(thread_ == 0);
        assert(m_MediaInfo == NULL);
        assert(m_PlayerGraph == 0);
        assert(m_pStream == 0);

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
            player_log(kLogLevelTrace, _T("PlayerCore::Create, Load PlayerCodecs failed, hr = 0x%08X"), hr);
        }

        hr = CreatePlayerThread();
        if (FAILED(hr))
        {
            player_log(kLogLevelTrace, _T("PlayerCore::Create, CreatePlayerThread failed, hr = 0x%08X"), hr);
        }

        if (SUCCEEDED(hr))
            m_fCreated = true;
    }

    if (!m_fCreated)
    {
        hr = E_FAIL;
    }

    return hr;
}

HRESULT PlayerCore::Destroy()
{
    HRESULT res = NOERROR;

    if (m_fCreated)
    {
        DestroyPlayerThread();

        SAFE_DELETE(m_PlayerGraph);
        SAFE_DELETE(m_pStream);
        SAFE_DELETE(m_MediaInfo);

        GetPlayerCodecs().FreeCodecs();

        m_fCreated = false;
    }

    return res;
}

HRESULT PlayerCore::CreatePlayerThread()
{
   assert(m_PlayerThread == NULL);

    m_PlayerThread = new PlayerThread(this);
    if (m_PlayerThread == NULL)
    {
        return E_OUTOFMEMORY;
    }
    
    if (!m_PlayerThread->CreateThread())
    {
        return E_FAIL;
    }

    return S_OK;
}

HRESULT PlayerCore::DestroyPlayerThread()
{
    if (m_PlayerThread)
    {
        CAMMsgEvent evt;
        m_PlayerThread->PutThreadMsg(PlayerThread::kMsgExit, 0, NULL, &evt);
        if (!evt.Wait(2000))
        {
            player_log(kLogLevelTrace, _T("PlayerCore::DestroyPlayerThread, wait timeout, try to terminate thread"));
            m_PlayerThread->Terminate((DWORD)-1);
        }
        else
        {
            player_log(kLogLevelTrace, _T("PlayerCore::DestroyPlayerThread, wait it finish"));
        }
    }
    SAFE_DELETE(m_PlayerThread);

    return NOERROR;
}

HRESULT PlayerCore::Open(const TCHAR* url)
{
    HRESULT res = E_FAIL;

    //g_XTimer.Start();
    s_dwStart = timeGetTime();

    if (state_ != kPlayerStateNothingSpecial)
    {
        Close();
    }

    CAutoPtr<CString> strUrl(new CString(url));

    SetPlayerState(kPlayerStateOpening);

    m_PlayerThread->PutThreadMsg(PlayerThread::kMsgOpen, 0, (LPVOID)strUrl.Detach());

    // opening ...
    // wait msg

    return res;
}

HRESULT PlayerCore::Close()
{
    HRESULT res = E_FAIL;

//    int time_waited = 0;
//    while (state_ == kPlayerStateOpening)
//    {
//        opening_aborted_ = true;
//
//        if (m_PlayerGraph)
//            m_PlayerGraph->Abort();
//
//        if (time_waited > 3000)
//        {
//            if (thread_)
//            {
//                thread_->Terminate();
//                thread_->Create();
//            }
////             else
////             {
////                 thread_ = new PlayerThread(this);
////                 thread_->Create();
////             }
//
//            break;
//        }
//
//        Sleep(50);
//
//        time_waited += 50;
//    }

    opening_aborted_ = false;

    SetPlayerState(kPlayerStateClosing);

    CAMMsgEvent evt;
    m_PlayerThread->PutThreadMsg(PlayerThread::kMsgClose, 0, NULL, &evt);
    evt.WaitMsg();
//     else
//     {
//         CloseMedia();
//     }

    SetPlayerState(kPlayerStateNothingSpecial);

    return res;
}

HRESULT PlayerCore::Play()
{
    HRESULT res = E_FAIL;

    m_PlayerThread->PutThreadMsg(PlayerThread::kMsgPlay, 0, NULL);

    return res;
}

HRESULT PlayerCore::Pause()
{
    HRESULT res = E_FAIL;

    m_PlayerThread->PutThreadMsg(PlayerThread::kMsgPause, 0, NULL);

    return res;
}

HRESULT PlayerCore::Stop()
{
    HRESULT res = E_FAIL;

    m_PlayerThread->PutThreadMsg(PlayerThread::kMsgStop, 0, NULL);

    return res;
}

HRESULT PlayerCore::Abort()
{
    HRESULT res = E_FAIL;

    if (m_PlayerGraph)
    {
        res = m_PlayerGraph->Abort();
    }
    else
    {

    }

    return res;
}

HRESULT PlayerCore::GetPlayState(PlayerState* state)
{
    HRESULT res = E_FAIL;

    if (m_PlayerGraph)
    {
        res = m_PlayerGraph->GetPlayState(state);
    }
    else
    {

    }

    return res;
}

HRESULT PlayerCore::GetDuration(long* duration)
{
    HRESULT res = E_FAIL;

    if (m_PlayerGraph)
    {
        res = m_PlayerGraph->GetDuration(duration);
    }
    else
    {

    }

    return res;
}

HRESULT PlayerCore::GetCurrentPlayPos(long* current_play_pos)
{
    HRESULT res = E_FAIL;

    if (m_PlayerGraph)
    {
        res = m_PlayerGraph->GetCurrentPlayPos(current_play_pos);
    }
    else
    {

    }

    return res;
}

HRESULT PlayerCore::SetPlayPos(long pos_to_play)
{
    HRESULT res = E_FAIL;

    if (m_PlayerGraph)
    {
        res = m_PlayerGraph->SetPlayPos(pos_to_play);
    }
    else
    {

    }

    return res;
}

HRESULT PlayerCore::DoOpen(CAutoPtr<CString> strUrl)
{
    player_log(kLogLevelTrace, _T("PlayerCore::DoOpen"));

    HRESULT hr = E_FAIL;

    // Create MediaInfo
    m_MediaInfo = new MediaInfo(strUrl->GetBuffer(), hr);

    if (FAILED(hr) || !m_MediaInfo)
    {
        player_log(kLogLevelError, _T("Create MediaInfo FAIL, hr = 0x%08x"), hr);
        return hr;
    }

    // Create Stream
    MediaProtocol protocol = m_MediaInfo->GetProtocol();
    if (protocol == kProtocolFile)
    {
        m_pStream = new PlayerFileStream;

    }
    else if (protocol == kProtocolQvod)
    {
        m_pStream = new PlayerQvodStream;
    }
    
    if (FAILED(hr = m_pStream->Open(m_MediaInfo->GetUrl())))
    {
        return hr;
    }

    // Create Graph
    m_PlayerGraph = new DirectShowGraph(hr);
    if (FAILED(hr) || !m_PlayerGraph)
    {
        player_log(kLogLevelTrace, _T("Create DirectShowGraph FAIL, hr = 0x%08x"), hr);
        return hr;
    }

    if (m_PlayerGraph)
    {
        hr = m_PlayerGraph->OpenMedia(m_MediaInfo);
    }

    if (SUCCEEDED(hr))
    {
        // open succeeded

        s_dwStop = timeGetTime();

        player_log(kLogLevelTrace, _T("OpenMedia Succeeded, cost time = %d ms"), s_dwStop - s_dwStart);

    }
    else
    {
        // open failed
        SetPlayerState(kPlayerStateNothingSpecial);
    }

    return hr;
}

HRESULT PlayerCore::DoClose()
{
    player_log(kLogLevelTrace, _T("PlayerCore::DoClose"));
    HRESULT res = NOERROR;

    SAFE_DELETE(m_PlayerGraph);
    SAFE_DELETE(m_pStream);
    SAFE_DELETE(m_MediaInfo);

    
    return res;
}

HRESULT PlayerCore::DoPlay()
{
    player_log(kLogLevelTrace, _T("PlayerCore::DoPlay"));
    HRESULT hr = S_OK;
    if (m_PlayerGraph)
    {
        hr = m_PlayerGraph->Play();
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
    if (m_PlayerGraph)
    {
        hr = m_PlayerGraph->Pause();
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
    if (m_PlayerGraph)
    {
        hr = m_PlayerGraph->Stop();
        if (SUCCEEDED(hr))
        {
            SetPlayerState(kPlayerStateStopped);
        }
    }
    return hr;
}

HRESULT PlayerCore::DoAbort()
{
    player_log(kLogLevelTrace, _T("PlayerCore::DoAbort"));
    HRESULT hr = S_OK;
    if (m_PlayerGraph)
    {
        hr = m_PlayerGraph->Abort();
    }
    return hr;
}

void PlayerCore::SetPlayerState(PlayerState state)
{
    FastMutex::ScopedLock lock(state_mutex_); 
    state_ = state;
    player_log(kLogLevelTrace, _T("PlayerCore::SetPlayerState = %s"), PlayerStateString(state));
}

//////////////////////////////////////////////////////////////////////////
PlayerSettings& PlayerCore::GetPlayerSettings()
{
    return *s_settings.get();
}

PlayerCodecs& PlayerCore::GetPlayerCodecs()
{
    return *s_codecs.get();
}



const TCHAR* PlayerStateString(PlayerState state)
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