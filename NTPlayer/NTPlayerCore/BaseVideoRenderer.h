#pragma once


#include "MediaInfo.h"
#include "PlayerSettings.h"

class BaseVideoRenderer
{
public:
    BaseVideoRenderer(VideoRenderMode mode, IBaseFilter* pBF) 
        : m_VideoRenderMode(mode)
        , m_pIBaseFilter(pBF)
    {}

    virtual ~BaseVideoRenderer(){}

    VideoRenderMode GetVideoRenderMode() {
        return m_VideoRenderMode;
    }

    virtual HRESULT SetVideoWindow(HWND hVideoWindow) = 0;
    virtual HRESULT SetVideoPosition(LPRECT prcDisplay) = 0;
    virtual HRESULT GetVideoSize(VideoSize* pVideoSize) = 0;

protected:

protected:
    VideoRenderMode m_VideoRenderMode;
    CComPtr<IBaseFilter> m_pIBaseFilter;
};