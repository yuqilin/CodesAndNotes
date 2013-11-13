#ifndef _PLAYERCORE_BASEGRAPH_H_
#define _PLAYERCORE_BASEGRAPH_H_

#include "PlayerInterface.h"

class MediaInfo;

class BaseGraph
    : public IPlayControl
    , public IVideoControl
    , public IAudioControl
{
public:
    BaseGraph() : media_info_(0) {}
    virtual ~BaseGraph() {}

protected:
    MediaInfo* media_info_;
};

#endif