#pragma once

#include <evr.h>
#include "d3d9types.h"
#include "d3d9.h"
#include <evr9.h>
#include "../SubPic/ISubPic.h"
#include "BaseVideoRenderer.h"

class VideoRendererEVRCP : public BaseVideoRenderer
{
public:
    VideoRendererEVRCP(HRESULT& hr, HWND hVideoWindow, bool bFullScreen);
    ~VideoRendererEVRCP();

    HRESULT CreateRenderer(IBaseFilter** ppBF);

    virtual HRESULT SetVideoWindow(HWND hVideoWindow);
    virtual HRESULT SetVideoPosition(LPRECT prcDisplay);
    virtual HRESULT RepaintVideo();
    virtual HRESULT GetVideoSize(VideoSize* pVideoSize);

protected:

protected:
    CComPtr<ISubPicAllocatorPresenter> m_pCAP;
    CComQIPtr<ISubPicAllocatorPresenter2> m_pCAP2;

    CComPtr<IMFVideoDisplayControl> m_pMFVDC;
    CComPtr<IMFVideoMixerBitmap>    m_pMFMB;
    CComPtr<IMFVideoProcessor>      m_pMFVP;

    HWND m_hVideoWindow;

};