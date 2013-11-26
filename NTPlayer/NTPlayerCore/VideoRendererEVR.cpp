#include "stdafx.h"
#include "MediaInfo.h"
#include "VideoRendererEVR.h"

VideoRendererEVR::VideoRendererEVR(HRESULT& hr)
 : BaseVideoRenderer(kVideoRenderEVR)
{
    hr = Create();
}

VideoRendererEVR::~VideoRendererEVR()
{

}

HRESULT VideoRendererEVR::CreateRenderer(IBaseFilter** ppBF)
{
    CheckPointer(ppBF, E_POINTER);

    *ppBF = m_pIBaseFilter;

    if (*ppBF)
        (*ppBF)->AddRef();

    return S_OK;
}

HRESULT VideoRendererEVR::SetVideoWindow(HWND hVideoWindow)
{
    HRESULT hr = E_FAIL;
    if (m_pMFVDC)
    {
        hr = m_pMFVDC->SetVideoWindow(hVideoWindow);

        if (SUCCEEDED(hr))
        {
            MFVideoAspectRatioMode theMode = MFVideoARMode_None;
            hr = m_pMFVDC->SetAspectRatioMode(theMode);
        }
    }
    return hr;
}

HRESULT VideoRendererEVR::SetVideoPosition(LPRECT prcDisplay)
{
    HRESULT hr = E_FAIL;
    if (m_pMFVDC)
    {
        hr = m_pMFVDC->SetVideoPosition(NULL, prcDisplay);
    }
    return hr;
}

HRESULT VideoRendererEVR::RepaintVideo()
{
    return S_OK;
}

HRESULT VideoRendererEVR::Create()
{
    HRESULT hr = S_OK;

    CComPtr<IBaseFilter> pBF;
    if (SUCCEEDED(hr = pBF.CoCreateInstance(CLSID_EnhancedVideoRenderer)))
    {
        m_pIBaseFilter = pBF;
    }

    if (CComQIPtr<IMFGetService, &__uuidof(IMFGetService)> pMFGS = pBF)
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
    else
    {
        hr = E_NOINTERFACE;
    }

    return hr;
}

HRESULT VideoRendererEVR::GetVideoSize(VideoSize* pVideoSize)
{
    HRESULT hr = E_FAIL;

    if (m_pMFVDC)
    {
        SIZE szVideo, szARVideo;
        if (SUCCEEDED(hr = m_pMFVDC->GetNativeVideoSize(&szVideo, &szARVideo)))
        {
            int gcd = GCD(szARVideo.cx, szARVideo.cy);
            if (gcd > 1) {
                szARVideo.cx /= gcd;
                szARVideo.cy /= gcd;
            }

            if (pVideoSize)
            {
                pVideoSize->w = szVideo.cx;
                pVideoSize->h = szVideo.cy;
                pVideoSize->arx = szARVideo.cx;
                pVideoSize->ary = szARVideo.cy;
            }
        }
    }

    return hr;
}