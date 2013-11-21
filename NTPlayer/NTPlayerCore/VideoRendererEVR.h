
#pragma once

#include <evr.h>
#include "d3d9types.h"
#include "d3d9.h"
#include <evr9.h>
#include "BaseVideoRenderer.h"

class VideoRendererEVR : public BaseVideoRenderer
{
public:
    VideoRendererEVR(HRESULT& hr,
                     IBaseFilter* pBF);
    ~VideoRendererEVR();

    HRESULT SetVideoWindow(HWND hVideoWindow);
    HRESULT SetVideoPosition(LPRECT prcDisplay);
    HRESULT GetVideoSize(VideoSize* pVideoSize);

protected:
    HRESULT Create(IBaseFilter* pBF);
protected:
    CComPtr<IMFVideoDisplayControl> m_pMFVDC;
    CComPtr<IMFVideoMixerBitmap>    m_pMFMB;
    CComPtr<IMFVideoProcessor>      m_pMFVP;
};