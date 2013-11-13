
#include <assert.h>
#include "PlayerCore.h"
#include "SingletonHolder.h"
#include "DirectShowGraph.h"

//////////////////////////////////////////////////////////////////////////
static SingletonHolder<PlayerSettings> s_settings;
static SingletonHolder<PlayerCodecs> s_codecs;

//////////////////////////////////////////////////////////////////////////
PlayerCore::PlayerCore()
: graph_(0)
, stream_(0)
, media_info_(0)
, thread_(0)
{

}

PlayerCore::~PlayerCore()
{
    Destroy();
}

PlayerResult PlayerCore::Create()
{
    PlayerResult res = kNoError;

    if (!created_)
    {
        assert(thread_ == 0);
        assert(media_info_ == 0);
        assert(graph_ == 0);
        assert(stream_ == 0);

        // load default settings
        //GetPlayerSettings();
        // load codecs
        //GetPlayerCodecs();

//         thread_ = new PlayerThread(this);
// 
//         BOOL bCreated = FALSE;
//         if (thread_)
//         {
//             bCreated = thread_->Create();
//         }
// 
//         if (bCreated)
//         {
//             created_ = true;
//         }
        created_ = true;
    }

    if (!created_)
    {
        res = kErrorUnknown;
    }

    return res;
}

PlayerResult PlayerCore::Destroy()
{
    PlayerResult res = kNoError;

    if (created_)
    {
//         if (thread_)
//         {
//             CAMEvent evt;
//             thread_->PostCmd(PlayerThread::kCmdExit, (void*)&evt);
//             if (!evt.Wait(3000))
//             {
//                 thread_->Terminate();
//             }
//             SAFE_DELETE(thread_);
//         }

        SAFE_DELETE(graph_);
        SAFE_DELETE(stream_);
        SAFE_DELETE(media_info_);

        created_ = false;
    }

    return res;
}

PlayerResult PlayerCore::CreatePlayerThread()
{
    assert(thread_ == 0);

    thread_ = new PlayerThread(this);
    thread_->Create();

    return kNoError;
}

PlayerResult PlayerCore::DestroyPlayerThread()
{
    if (thread_)
    {
        CAMMsgEvent evt;
        thread_->PostCmd(PlayerThread::kCmdExit, (void*)&evt);
        evt.WaitMsg();
        SAFE_DELETE(thread_);
    }

    return kNoError;
}

PlayerResult PlayerCore::Open(const char* url)
{
    PlayerResult res = kErrorUnknown;

    if (state_ != kPlayerStateNothingSpecial)
    {
        Close();
    }

    media_info_ = new MediaInfo(url);
    if (!media_info_)
    {
        return kErrorOutOfMemory;
    }

//     if (thread_)
//     {
//         res = thread_->PostCmd(PlayerThread::kCmdOpen, &media_info_->url_);
// 
//         // opening ...
//         // wait msg
//     }

    res = OpenMedia(url);

    return res;
}

PlayerResult PlayerCore::Close()
{
    PlayerResult res = kErrorUnknown;

//    int time_waited = 0;
//    while (state_ == kPlayerStateOpening)
//    {
//        opening_aborted_ = true;
//
//        if (graph_)
//            graph_->Abort();
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

    //SetPlayerState(kPlayerStateClosing);

//     if (thread_)
//     {
//         CAMEvent evt;
//         thread_->PostCmd(PlayerThread::kCmdClose, (void*)&evt);
//         evt.Wait();
//     }

    return res;
}

PlayerResult PlayerCore::Play()
{
    PlayerResult res = kErrorUnknown;

//     if (thread_)
//     {
//         thread_->PostCmd(PlayerThread::kCmdPlay, NULL);
//     }

    if (graph_)
    {
        graph_->Play();
    }

    return res;
}

PlayerResult PlayerCore::Pause()
{
    PlayerResult res = kErrorUnknown;

    if (graph_)
    {
        res = graph_->Pause();
    }
    else
    {

    }

    return res;
}

PlayerResult PlayerCore::Stop()
{
    PlayerResult res = kErrorUnknown;

    if (graph_)
    {
        res = graph_->Stop();
    }
    else
    {

    }

    return res;
}

PlayerResult PlayerCore::Abort()
{
    PlayerResult res = kErrorUnknown;

    if (graph_)
    {
        res = graph_->Abort();
    }
    else
    {

    }

    return res;
}

PlayerResult PlayerCore::GetPlayState(PlayerState* state)
{
    PlayerResult res = kErrorUnknown;

    if (graph_)
    {
        res = graph_->GetPlayState(state);
    }
    else
    {

    }

    return res;
}

PlayerResult PlayerCore::GetDuration(long* duration)
{
    PlayerResult res = kErrorUnknown;

    if (graph_)
    {
        res = graph_->GetDuration(duration);
    }
    else
    {

    }

    return res;
}

PlayerResult PlayerCore::GetCurrentPlayPos(long* current_play_pos)
{
    PlayerResult res = kErrorUnknown;

    if (graph_)
    {
        res = graph_->GetCurrentPlayPos(current_play_pos);
    }
    else
    {

    }

    return res;
}

PlayerResult PlayerCore::SetPlayPos(long pos_to_play)
{
    PlayerResult res = kErrorUnknown;

    if (graph_)
    {
        res = graph_->SetPlayPos(pos_to_play);
    }
    else
    {

    }

    return res;
}

PlayerResult PlayerCore::OpenMedia(const char* url)
{
    PlayerResult res = kErrorUnknown;

    SetPlayerState(kPlayerStateOpening);

    graph_ = new DirectShowGraph;

    if (graph_)
    {
        res = graph_->OpenMedia(media_info_);
    }

    if (res == kNoError)
    {
        // open succeeded
    }

    return res;
}

PlayerResult PlayerCore::CloseMedia()
{
    PlayerResult res = kNoError;

    SAFE_DELETE(media_info_);
    SAFE_DELETE(graph_);
    SAFE_DELETE(stream_);

    
    return res;
}

void PlayerCore::SetPlayerState(PlayerState state)
{
    FastMutex::ScopedLock lock(state_mutex_); 
    state_ = state;
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
