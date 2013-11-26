#include "stdafx.h"
#include "../filters/renderer/VideoRenderers/AllocatorCommon.h"

#include "VideoRendererEVRCP.h"

VideoRendererEVRCP::VideoRendererEVRCP(HRESULT& hr, HWND hVideoWindow, bool bFullScreen)
: BaseVideoRenderer(kVideoRenderEVRCP)
{
    CComPtr<ISubPicAllocatorPresenter> pCAP;
    hr = CreateEVR(CLSID_EVRAllocatorPresenter, hVideoWindow, bFullScreen, &pCAP);
    if (SUCCEEDED(hr))
    {
        m_pCAP = pCAP;

        m_hVideoWindow = hVideoWindow;

        CComPtr<IUnknown> pRenderer;
        if (SUCCEEDED(hr = m_pCAP->CreateRenderer(&pRenderer)))
        {
            m_pIBaseFilter = pRenderer;//CComQIPtr<IBaseFilter>(pRenderer).Detach();
            if (!m_pIBaseFilter)
            {
                hr = E_NOINTERFACE;
            }
        }

        m_pCAP2 = m_pCAP;
    }

    CComQIPtr<IMFGetService, &__uuidof(IMFGetService)> pMFGS;
    if (SUCCEEDED(hr) && (pMFGS = m_pIBaseFilter))
    {
        if (FAILED(hr = pMFGS->GetService(MR_VIDEO_RENDER_SERVICE, IID_IMFVideoDisplayControl, (void**)&m_pMFVDC)))
        {
            player_log(kLogLevelError, _T("IMFGetService, IID_IMFVideoDisplayControl failed, hr = 0x%08x"), hr);
        }

        if (FAILED(hr = pMFGS->GetService(MR_VIDEO_MIXER_SERVICE, IID_IMFVideoMixerBitmap, (void**)&m_pMFMB)))
        {
            player_log(kLogLevelError, _T("IMFGetService, IID_IMFVideoMixerBitmap failed, hr = 0x%08x"), hr);
        }

        if (FAILED(hr = pMFGS->GetService(MR_VIDEO_MIXER_SERVICE, IID_IMFVideoProcessor, (void**)&m_pMFVP)))
        {
            player_log(kLogLevelError, _T("IMFGetService, IID_IMFVideoProcessor failed, hr = 0x%08x"), hr);
        }
    }
}

VideoRendererEVRCP::~VideoRendererEVRCP()
{

}

HRESULT VideoRendererEVRCP::CreateRenderer(IBaseFilter** ppBF)
{
    HRESULT hr = S_OK;
    *ppBF = m_pIBaseFilter;

    if (*ppBF)
        (*ppBF)->AddRef();

    return hr;
}

HRESULT VideoRendererEVRCP::SetVideoWindow(HWND hVideoWindow)
{
    HRESULT hr = S_OK;
    if (m_pMFVDC)
    {
        hr = m_pMFVDC->SetVideoWindow(hVideoWindow);

//         if (SUCCEEDED(hr))
//         {
//             MFVideoAspectRatioMode theMode = MFVideoARMode_None;
//             hr = m_pMFVDC->SetAspectRatioMode(theMode);
//         }
    }

//     if (m_pCAP)
//         m_pCAP->SetPixelShader(NULL, NULL);
//     if (m_pCAP2)
//         m_pCAP2->SetPixelShader2(NULL, NULL, true);

    return hr;
}

HRESULT VideoRendererEVRCP::SetVideoPosition(LPRECT prcDisplay)
{
    HRESULT hr = S_OK;


    RECT wr, vr;
    GetClientRect(m_hVideoWindow, &wr);

    vr = *prcDisplay;

    if (m_pCAP)
    {
        m_pCAP->SetPosition(wr, vr);
        int nAngleX = 0, nAngleY = 0, nAngleZ = 0;
        Vector v(Vector::DegToRad(nAngleX), Vector::DegToRad(nAngleY), Vector::DegToRad(nAngleZ));
        m_pCAP->SetVideoAngle(v);
    }


    return hr;
}

HRESULT VideoRendererEVRCP::RepaintVideo()
{
    if (m_pCAP)
    {
        m_pCAP->Paint(false);
    }

    return S_OK;
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