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
    BaseGraph(PlayerCore* pPlayer)
        : m_pPlayer(pPlayer)
        , m_pMediaInfo(NULL)
    {}
    virtual ~BaseGraph() {}

protected:

protected:
    MediaInfo* m_pMediaInfo;
    PlayerCore* m_pPlayer;
};

#endif