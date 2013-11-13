#ifndef _PLAYERCORE_DIRECTSHOWGRAPH_H_
#define _PLAYERCORE_DIRECTSHOWGRAPH_H_

#include "PlayerCommon.h"
#include "BaseGraph.h"

class DirectShowGraph : public BaseGraph
{
public:
    DirectShowGraph();
    ~DirectShowGraph();

    // IPlayControl
    PlayerResult OpenMedia(MediaInfo* media_info);
    PlayerResult CloseMedia();
    PlayerResult Play();
    PlayerResult Pause();
    PlayerResult Stop();
    PlayerResult Abort();
    PlayerResult GetPlayState(PlayerState* state);
    PlayerResult GetDuration(long* duration);
    PlayerResult GetCurrentPlayPos(long* current_play_pos);
    PlayerResult SetPlayPos(long pos_to_play);
    bool         IsPlaying();

    // IVideoControl
    PlayerResult SetVideoWindow(void* video_window);
    PlayerResult SetVideoPosition(int );
    PlayerResult GetVideoSize(int* w, int* h);
    PlayerResult SetColorControl(int brightness, int contrast, int hue, int staturation);
    PlayerResult LoadExternalSubtitle(const char* subtitle_path);
    PlayerResult GrabCurrentVideoFrame(const char* save_file_name);

    // IAudioControl
    PlayerResult GetMute(bool* mute);
    PlayerResult SetMute(bool mute);
    PlayerResult GetVolume(int* volume);
    PlayerResult SetVolume(int volume);

};

#endif