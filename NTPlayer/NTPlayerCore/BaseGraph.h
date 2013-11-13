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
    BaseGraph() : m_pMediaInfo(NULL) {}
    virtual ~BaseGraph() {}

protected:
    virtual HRESULT PrepareRenderFile() { return S_OK; }

protected:
    MediaInfo* m_pMediaInfo;
};

#endif