#pragma once

class PlayerCore;

class PlayerThread : public CMsgThread
{
    PlayerThread(const PlayerThread &refThread);
    PlayerThread &operator=(const PlayerThread &refThread);
public:
    enum {
        kMsgUnknown = -1,
        kMsgOpen,
        kMsgClose,
        kMsgPlay,
        kMsgPause,
        kMsgStop,
        kMsgSeek,
        kMsgExit
    };

    PlayerThread(PlayerCore* player);
    ~PlayerThread();

    virtual LRESULT ThreadMessageProc(
        UINT uMsg, DWORD dwFlags, LPVOID lpParam, CAMEvent *pEvent);

    BOOL Terminate(DWORD dwExitCode);

    BOOL IsRunning();

protected:
    LRESULT OnOpen(UINT uMsg, DWORD dwFlags, LPVOID lpParam, CAMEvent *pEvent);
    LRESULT OnClose(UINT uMsg, DWORD dwFlags, LPVOID lpParam, CAMEvent *pEvent);
    LRESULT OnPlay(UINT uMsg, DWORD dwFlags, LPVOID lpParam, CAMEvent *pEvent);
    LRESULT OnPause(UINT uMsg, DWORD dwFlags, LPVOID lpParam, CAMEvent *pEvent);
    LRESULT OnStop(UINT uMsg, DWORD dwFlags, LPVOID lpParam, CAMEvent *pEvent);
    LRESULT OnSeek(UINT uMsg, DWORD dwFlags, LPVOID lpParam, CAMEvent *pEvent);
    LRESULT OnExit(UINT uMsg, DWORD dwFlags, LPVOID lpParam, CAMEvent *pEvent);

protected:
    PlayerCore* m_pPlayer;
};
