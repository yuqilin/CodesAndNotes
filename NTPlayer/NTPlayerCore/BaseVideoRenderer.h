#pragma once


#include "MediaInfo.h"
#include "PlayerSettings.h"

class BaseVideoRenderer
{
public:
    BaseVideoRenderer(VideoRenderMode mode) 
        : m_VideoRenderMode(mode)
    {}

    virtual ~BaseVideoRenderer()
    {}

    VideoRenderMode GetVideoRenderMode() {
        return m_VideoRenderMode;
    }

    virtual HRESULT CreateRenderer(IBaseFilter** ppBF) = 0;

    virtual HRESULT SetVideoWindow(HWND hVideoWindow) = 0;
    virtual HRESULT SetVideoPosition(LPRECT prcDisplay) = 0;
    virtual HRESULT RepaintVideo() = 0;
    virtual HRESULT GetVideoSize(VideoSize* pVideoSize) = 0;

protected:

protected:
    VideoRenderMode m_VideoRenderMode;
    CComPtr<IBaseFilter> m_pIBaseFilter;
};