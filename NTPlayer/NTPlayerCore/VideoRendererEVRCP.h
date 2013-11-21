#pragma once

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
    virtual HRESULT GetVideoSize(VideoSize* pVideoSize);

protected:

protected:
    CComPtr<ISubPicAllocatorPresenter> m_pCAP;

    HWND m_hVideoWindow;

};