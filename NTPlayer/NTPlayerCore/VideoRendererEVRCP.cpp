#include "stdafx.h"
#include "../filters/renderer/VideoRenderers/AllocatorCommon.h"

#include "VideoRendererEVRCP.h"

VideoRendererEVRCP::VideoRendererEVRCP(HRESULT& hr, HWND hVideoWindow, bool bFullScreen)
: BaseVideoRenderer(kVideoRenderEVRCP, NULL)
{
    hr = CreateEVR(CLSID_EVRAllocatorPresenter, hVideoWindow, bFullScreen, &m_pCAP);
    if (SUCCEEDED(hr))
    {
        m_hVideoWindow = hVideoWindow;
    }
}

VideoRendererEVRCP::~VideoRendererEVRCP()
{

}

HRESULT VideoRendererEVRCP::CreateRenderer(IBaseFilter** ppBF)
{
    HRESULT hr;
    CComPtr<IUnknown> pRenderer;
    if (SUCCEEDED(hr = m_pCAP->CreateRenderer(&pRenderer)))
    {
        *ppBF = CComQIPtr<IBaseFilter>(pRenderer).Detach();
    }

    return hr;
}

HRESULT VideoRendererEVRCP::SetVideoWindow(HWND hVideoWindow)
{
    HRESULT hr = S_OK;



    return hr;
}

HRESULT VideoRendererEVRCP::SetVideoPosition(LPRECT prcDisplay)
{
    HRESULT hr = S_OK;


    RECT wr, vr;
    GetClientRect(m_hVideoWindow, &wr);

    vr = *prcDisplay;

    m_pCAP->SetPosition(wr, vr);


    return hr;
}

HRESULT VideoRendererEVRCP::GetVideoSize(VideoSize* pVideoSize)
{
    HRESULT hr = S_OK;

    CSize wh(0, 0), arxy(0, 0);

    wh = m_pCAP->GetVideoSize(false);
    arxy = m_pCAP->GetVideoSize(true);

    int gcd = GCD(arxy.cx, arxy.cy);
    if (gcd > 1) {
        arxy.cx /= gcd;
        arxy.cy /= gcd;
    }

    pVideoSize->w = wh.cx;
    pVideoSize->h = wh.cy;
    pVideoSize->arx = arxy.cx;
    pVideoSize->ary = arxy.cy;


    return hr;
}