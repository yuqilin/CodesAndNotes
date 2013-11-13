
#include "DirectShowGraph.h"

DirectShowGraph::DirectShowGraph()
{

}

DirectShowGraph::~DirectShowGraph()
{

}

PlayerResult DirectShowGraph::OpenMedia(MediaInfo* media_info)
{
    PlayerResult res = kNoError;
    media_info_ = media_info;
    return res;
}

PlayerResult DirectShowGraph::CloseMedia()
{
    PlayerResult res = kNoError;
    media_info_ = 0;
    return res;
}

PlayerResult DirectShowGraph::Play()
{
    PlayerResult res = kNoError;
    return res;
}
PlayerResult DirectShowGraph::Pause()
{
    PlayerResult res = kNoError;
    return res;
}

PlayerResult DirectShowGraph::Stop()
{
    PlayerResult res = kNoError;
    return res;
}

PlayerResult DirectShowGraph::Abort()
{
    PlayerResult res = kNoError;
    return res;
}

PlayerResult DirectShowGraph::GetPlayState(PlayerState* state)
{
    PlayerResult res = kNoError;
    return res;
}
PlayerResult DirectShowGraph::GetDuration(long* duration)
{
    PlayerResult res = kNoError;
    return res;
}

PlayerResult DirectShowGraph::GetCurrentPlayPos(long* current_play_pos)
{
    PlayerResult res = kNoError;
    return res;
}

PlayerResult DirectShowGraph::SetPlayPos(long pos_to_play)
{
    PlayerResult res = kNoError;
    return res;
}

bool DirectShowGraph::IsPlaying()
{
    bool is_playing = false;

    return is_playing;
}
// IVideoControl
PlayerResult DirectShowGraph::SetVideoWindow(void* video_window)
{
    PlayerResult res = kNoError;
    return res;
}

PlayerResult DirectShowGraph::SetVideoPosition(int )
{
    PlayerResult res = kNoError;
    return res;
}

PlayerResult DirectShowGraph::GetVideoSize(int* w, int* h)
{
    PlayerResult res = kNoError;
    return res;
}

PlayerResult DirectShowGraph::SetColorControl(int brightness, int contrast, int hue, int staturation)
{
    PlayerResult res = kNoError;
    return res;
}

PlayerResult DirectShowGraph::LoadExternalSubtitle(const char* subtitle_path)
{
    PlayerResult res = kNoError;
    return res;
}

PlayerResult DirectShowGraph::GrabCurrentVideoFrame(const char* save_file_name)
{
    PlayerResult res = kNoError;
    return res;
}

// IAudioControl
PlayerResult DirectShowGraph::GetMute(bool* mute)
{
    PlayerResult res = kNoError;
    return res;
}

PlayerResult DirectShowGraph::SetMute(bool mute)
{
    PlayerResult res = kNoError;
    return res;
}

PlayerResult DirectShowGraph::GetVolume(int* volume)
{
    PlayerResult res = kNoError;
    return res;
}

PlayerResult DirectShowGraph::SetVolume(int volume)
{
    PlayerResult res = kNoError;
    return res;
}
