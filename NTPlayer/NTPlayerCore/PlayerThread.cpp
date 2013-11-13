#include "stdafx.h"
#include <string>
#include "PlayerThread.h"
#include "PlayerCore.h"

static LPCTSTR PlayerMsgString(UINT uMsg);

PlayerThread::PlayerThread(PlayerCore* player)
: m_pPlayer(player)
{

}

PlayerThread::~PlayerThread()
{
    m_pPlayer = NULL;
}

BOOL PlayerThread::Terminate(DWORD dwExitCode)
{
    BOOL bRet = ::TerminateThread(GetThreadHandle(), dwExitCode);
    if (!bRet)
    {
        player_log(kLogLevelTrace, _T("PlayerThread::Terminate failed, ThreadID=%d, LastError=0x%08x"),
            GetThreadId(), ::GetLastError());
    }
    return bRet;
}


LRESULT PlayerThread::ThreadMessageProc(
    UINT uMsg, DWORD dwFlags, LPVOID lpParam, CAMEvent *pEvent)
{
    LRESULT lResult = 0;
    switch (uMsg)
    {
    case kMsgOpen:
        lResult = OnOpen(uMsg, dwFlags, lpParam, pEvent);
        break;

    case kMsgClose:
        lResult = OnClose(uMsg, dwFlags, lpParam, pEvent);
        break;

    case kMsgPlay:
        lResult = OnPlay(uMsg, dwFlags, lpParam, pEvent);
        break;

    case kMsgPause:
        lResult = OnPause(uMsg, dwFlags, lpParam, pEvent);
        break;

    case kMsgStop:
        lResult = OnStop(uMsg, dwFlags, lpParam, pEvent);
        break;

    case kMsgExit:
        lResult = OnExit(uMsg, dwFlags, lpParam, pEvent);
        break;
    }
    return lResult;
}

LRESULT PlayerThread::OnOpen(UINT uMsg, DWORD dwFlags, LPVOID lpParam, CAMEvent *pEvent)
{
    player_log(kLogLevelTrace, _T("PlayerThread::OnOpen"));

    if (m_pPlayer)
    {
        CAutoPtr<CString> strUrl((CString*)lpParam);
        m_pPlayer->DoOpen(strUrl);
    }
    return 0;
}

LRESULT PlayerThread::OnClose(UINT uMsg, DWORD dwFlags, LPVOID lpParam, CAMEvent *pEvent)
{
    player_log(kLogLevelTrace, _T("PlayerThread::OnClose"));

    return 0;
}

LRESULT PlayerThread::OnPlay(UINT uMsg, DWORD dwFlags, LPVOID lpParam, CAMEvent *pEvent)
{
    player_log(kLogLevelTrace, _T("PlayerThread::OnPlay"));

    if (m_pPlayer)
    {
        m_pPlayer->DoPlay();
    }
    return 0;
}

LRESULT PlayerThread::OnPause(UINT uMsg, DWORD dwFlags, LPVOID lpParam, CAMEvent *pEvent)
{
    player_log(kLogLevelTrace, _T("PlayerThread::OnPause"));
    return 0;
}

LRESULT PlayerThread::OnStop(UINT uMsg, DWORD dwFlags, LPVOID lpParam, CAMEvent *pEvent)
{
    player_log(kLogLevelTrace, _T("PlayerThread::OnStop"));
    return 0;
}

LRESULT PlayerThread::OnExit(UINT uMsg, DWORD dwFlags, LPVOID lpParam, CAMEvent *pEvent)
{
    player_log(kLogLevelTrace, _T("PlayerThread::OnExit"));
    if (pEvent)
    {
        pEvent->Set();
    }
    return -1;
}


static LPCTSTR PlayerMsgString(UINT uMsg)
{
    switch (uMsg)
    {
    case PlayerThread::kMsgOpen:
        return _T("MsgOpen");
    case PlayerThread::kMsgClose:
        return _T("MsgClose");
    case PlayerThread::kMsgPlay:
        return _T("MsgPlay");
    case PlayerThread::kMsgPause:
        return _T("MsgPause");
    case PlayerThread::kMsgStop:
        return _T("MsgStop");
    case PlayerThread::kMsgExit:
        return _T("MsgExit");
    }

    return _T("MsgUnknown");
}