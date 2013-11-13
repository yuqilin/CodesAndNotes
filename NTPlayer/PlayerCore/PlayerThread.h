#ifndef _PLAYERCORE_WORKERTHREAD_H_
#define _PLAYERCORE_WORKERTHREAD_H_

#include <string>
#include <streams.h>
#include "PlayerCommon.h"

class PlayerCore;

class PlayerThread : public CAMThread
{
public:
    enum { kCmdOpen, kCmdClose, kCmdPlay, kCmdPause, kCmdStop, kCmdExit };

    PlayerThread(PlayerCore* player);
    ~PlayerThread();

    PlayerResult PostCmd(int cmd, void* param);

    PlayerResult Terminate();

protected:
    virtual DWORD ThreadProc();

    void OnCmdOpen();
    void OnCmdClose();
    void OnCmdPlay();
    void OnCmdPause();
    void OnCmdStop();
    void OnCmdExit();

protected:
    PlayerCore* player_;
    std::string url_;
    void* param_;
};



#endif