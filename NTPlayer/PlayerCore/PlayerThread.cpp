#include "PlayerThread.h"
#include "PlayerCore.h"

PlayerThread::PlayerThread(PlayerCore* player)
: player_(player)
{

}

PlayerThread::~PlayerThread()
{
    player_ = 0;
}

DWORD PlayerThread::ThreadProc()
{
    for (;;)
    {

        DWORD dwRequest = GetRequest();

        switch (dwRequest)
        {
        case kCmdOpen:
            OnCmdOpen();
            break;

        case kCmdClose:
            OnCmdClose();
            break;

        case kCmdPlay:
            OnCmdPlay();
            break;

        case kCmdPause:
            OnCmdPause();
            break;

        case kCmdStop:
            OnCmdStop();
            break;

        case kCmdExit:
            OnCmdExit();
            return 0;
        }
    }

    return 0;
}

PlayerResult PlayerThread::PostCmd(int cmd, void* param)
{
    PlayerResult res = kNoError;

    param_ = param;

    CallWorker(cmd);

    return res;
}

PlayerResult PlayerThread::Terminate()
{
    PlayerResult res = kNoError;

    ::TerminateThread(m_hThread, (DWORD)-1);

    return res;
}

void PlayerThread::OnCmdOpen()
{
    std::string url;
    if (param_)
    {
        url = *(std::string*)param_;
    }
    Reply(S_OK);
    if (player_)
    {
        PlayerResult res = player_->OpenMedia(url.c_str());
    }
}

void PlayerThread::OnCmdClose()
{
    url_.clear();
    player_->CloseMedia();
    Reply(S_OK);
}

void PlayerThread::OnCmdPlay()
{
    if (player_)
        player_->Play();
    Reply(S_OK);
}

void PlayerThread::OnCmdPause()
{
    if (player_)
        player_->Pause();
    Reply(S_OK);
}

void PlayerThread::OnCmdStop()
{
    Reply(S_OK);

    if (player_)
        player_->Stop();
}

void PlayerThread::OnCmdExit()
{
    Reply(S_OK);
//     CAMEvent* evt = (CAMEvent*)param_;
// 
// //     if (player_)
// //         player_->CloseMedia();
// 
//     if (evt)
//         evt->Set();
}
