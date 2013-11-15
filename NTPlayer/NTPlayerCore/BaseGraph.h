#ifndef _PLAYERCORE_BASEGRAPH_H_
#define _PLAYERCORE_BASEGRAPH_H_

#include "PlayerInterface.h"

class MediaInfo;

class PlayerCore;

class BaseGraph
    : public IPlayControl
    , public IVideoControl
    , public IAudioControl
{
public:
    BaseGraph(PlayerCore* pPlayer)
        : m_pPlayer(pPlayer)
        , m_pMediaInfo(NULL)
        , m_hVideoWnd(NULL)
    {}
    virtual ~BaseGraph() {}

protected:
    MediaInfo* m_pMediaInfo;
    PlayerCore* m_pPlayer;
    HWND    m_hVideoWnd;
};

#endif