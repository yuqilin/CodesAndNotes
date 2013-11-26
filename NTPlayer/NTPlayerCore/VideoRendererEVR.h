
#pragma once

#include <evr.h>
#include "d3d9types.h"
#include "d3d9.h"
#include <evr9.h>
#include "BaseVideoRenderer.h"

class VideoRendererEVR : public BaseVideoRenderer
{
public:
    VideoRendererEVR(HRESULT& hr);
    ~VideoRendererEVR();

    virtual HRESULT CreateRenderer(IBaseFilter** ppBF);

    virtual HRESULT SetVideoWindow(HWND hVideoWindow);
    virtual HRESULT SetVideoPosition(LPRECT prcDisplay);
    virtual HRESULT RepaintVideo();
    virtual HRESULT GetVideoSize(VideoSize* pVideoSize);

protected:
    HRESULT Create();
protected:
    CComPtr<IMFVideoDisplayControl> m_pMFVDC;
    CComPtr<IMFVideoMixerBitmap>    m_pMFMB;
    CComPtr<IMFVideoProcessor>      m_pMFVP;
};