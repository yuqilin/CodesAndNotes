#include "stdafx.h"
#include <atlpath.h>
#include "DirectShowGraph.h"
#include "PlayerCore.h"
#include "PlayerBaseStream.h"
#include "PlayerAsyncReader.h"
#include "VideoRendererEVR.h"
#include "VideoRendererEVRCP.h"
#include "../filters/renderer/VideoRenderers/AllocatorCommon.h"


#define _100NS_UNITS_TO_MILLISECONDS(refTime) \
    ((LONG)(refTime / 10000))

#define sClsid_VsFilter         _T("{93A22E7A-5091-45EF-BA61-6DA26156A5D0}")
#define sClsid_AudioSwitcher    _T("{18C16B08-6497-420E-AD14-22D21C2CEAB7}")

DirectShowGraph::DirectShowGraph(PlayerCore* pPlayer, HRESULT& hr)
: BaseGraph(pPlayer)
, m_pVideoRenderer(NULL)
, m_bAborted(FALSE)
{
    hr = m_pIGraphBuilder.CoCreateInstance(CLSID_FilterGraph);
    //hr = LoadExternalObject(_T("C:\\Windows\\System32\\quartz.dll"), CLSID_FilterGraph, IID_IGraphBuilder, (void**)&m_pIGraphBuilder);
}

DirectShowGraph::~DirectShowGraph()
{
    CloseMedia();
}

HRESULT DirectShowGraph::OpenMedia(MediaInfo* pMediaInfo)
{
    CheckPointer(pMediaInfo, E_POINTER);

    m_pMediaInfo = pMediaInfo;

    if (m_bAborted)
    {
        player_log(kLogLevelTrace, _T("DirectShowGraph::OpenMedia, open aborted before Core_OnRenderPrepare"));
        return E_ABORT;
    }

    HRESULT hr = Core_OnRenderPrepare();

    if (FAILED(hr))
    {
        return hr;
    }

    if (m_bAborted)
    {
        player_log(kLogLevelTrace, _T("DirectShowGraph::OpenMedia, open aborted before Core_Render"));
        return E_ABORT;
    }

    hr = Core_Render();

    if (FAILED(hr))
    {
        return hr;
    }

    if (m_bAborted)
    {
        player_log(kLogLevelTrace, _T("DirectShowGraph::OpenMedia, open aborted before Core_OnRenderComplete"));
        return E_ABORT;
    }

    hr = Core_OnRenderComplete();

    return hr;
}

HRESULT DirectShowGraph::CloseMedia()
{
    HRESULT hr = S_OK;
    
    Stop();

    SAFE_DELETE(m_pVideoRenderer);

    m_bAborted = FALSE;

    RemoveAllFilter();

    return hr;
}

HRESULT DirectShowGraph::Play()
{
    HRESULT hr = S_OK;

    if (m_pIMediaControl)
    {
        hr = m_pIMediaControl->Run();
    }

    return hr;
}

HRESULT DirectShowGraph::Pause()
{
    HRESULT hr = S_OK;

    if (m_pIMediaControl)
    {
        hr = m_pIMediaControl->Pause();
    }

    return hr;
}

HRESULT DirectShowGraph::Stop()
{
    HRESULT hr = S_OK;

    if (m_pIMediaControl)
    {
        hr = m_pIMediaControl->Stop();
    }

    return hr;
}

HRESULT DirectShowGraph::Abort()
{
    HRESULT hr = S_OK;

    m_bAborted = TRUE;

    return hr;
}

HRESULT DirectShowGraph::GetDuration(long* pnDuration)
{
    HRESULT hr = S_OK;

    if (m_pMediaInfo && pnDuration)
    {
        *pnDuration = m_pMediaInfo->GetDuration();
    }

    return hr;
}

HRESULT DirectShowGraph::GetCurrentPlayPos(long* pnCurPlayPos)
{
    HRESULT hr = E_FAIL;

    if (m_GraphMutex.tryLock())
    {
        if (m_pIMediaSeeking)
        {
            LONGLONG llCurrent = 0, llStop = 0;
            hr = m_pIMediaSeeking->GetPositions(&llCurrent, &llStop);
            if (SUCCEEDED(hr))
            {
                if (pnCurPlayPos)
                    *pnCurPlayPos = _100NS_UNITS_TO_MILLISECONDS(llCurrent);
//                 player_log(kLogLevelTrace, _T("DirectShowGraph::GetCurrentPlayPos, llCurrent = %I64d(%s)"),
//                     llCurrent, Millisecs2CString(_100NS_UNITS_TO_MILLISECONDS(llCurrent)));
            }
            else
            {
                player_log(kLogLevelTrace, _T("DirectShowGraph::GetCurrentPlayPos, GetPositions failed hr = 0x%08x"), hr);
            }
        }
    }
    else
    {
        player_log(kLogLevelTrace, _T("DirectShowGraph::GetCurrentPlayPos, graph is busy"));
    }

    return hr;
}

HRESULT DirectShowGraph::SetPlayPos(long pos_to_play)
{
    HRESULT hr = S_OK;
    return hr;
}

bool DirectShowGraph::IsPlaying()
{
    bool is_playing = false;

    return is_playing;
}
// IVideoControl
HRESULT DirectShowGraph::SetVideoWindow(void* video_window)
{
    HRESULT hr = S_OK;

    if (m_pVideoRenderer)
    {
        hr = m_pVideoRenderer->SetVideoWindow((HWND)video_window);
    }

    return hr;
}

HRESULT DirectShowGraph::SetVideoPosition(LPRECT lpRect)
{
    HRESULT hr = S_OK;

    if (m_pVideoRenderer)
    {
        hr = m_pVideoRenderer->SetVideoPosition(lpRect);
    }

    return hr;
}

HRESULT DirectShowGraph::GetVideoSize(VideoSize* pVideoSize)
{
    HRESULT hr = S_OK;

    if (m_pVideoRenderer)
    {
        hr = m_pVideoRenderer->GetVideoSize(pVideoSize);
        if (SUCCEEDED(hr))
        {
            m_pMediaInfo->SetVideoSize(pVideoSize);
        }
    }

    return hr;
}

HRESULT DirectShowGraph::SetColorControl(int brightness, int contrast, int hue, int staturation)
{
    HRESULT hr = S_OK;
    return hr;
}

HRESULT DirectShowGraph::LoadExternalSubtitle(const char* subtitle_path)
{
    HRESULT hr = S_OK;
    return hr;
}

HRESULT DirectShowGraph::GrabCurrentVideoFrame(const char* save_file_name)
{
    HRESULT hr = S_OK;
    return hr;
}

// IAudioControl
HRESULT DirectShowGraph::GetMute(bool* mute)
{
    HRESULT hr = S_OK;
    return hr;
}

HRESULT DirectShowGraph::SetMute(bool mute)
{
    HRESULT hr = S_OK;
    return hr;
}

HRESULT DirectShowGraph::GetVolume(int* volume)
{
    HRESULT hr = S_OK;
    return hr;
}

HRESULT DirectShowGraph::SetVolume(int volume)
{
    HRESULT hr = S_OK;
    return hr;
}

/*
HRESULT DirectShowGraph::RenderFile(LPCWSTR lpwcsUrl)
{
    player_log(kLogLevelTrace, _T("DirectShowGraph::RenderFile"));

    CheckPointer(lpwcsUrl, E_POINTER);

    HRESULT hr;
    HRESULT hrRFS = S_OK;


    // OnPreloadFilters
    {
        CodecsInfo* pInfo = PlayerCore::GetPlayerCodecs().FindCodecsInfo(sClsid_VsFilter, kCodecsTypeVideoEffect);
        if (pInfo)
        {
            CInterfaceList<IUnknown, &IID_IUnknown> pUnks;
            HRESULT hrVsfilte = E_FAIL;
            if (SUCCEEDED(hrVsfilte = PlayerCore::GetPlayerCodecs().CreateCodecsObject(pInfo, &m_pVSFilter, pUnks)))
            {
                if (SUCCEEDED(hrVsfilte = AddFilter(m_pVSFilter, pInfo->name)))
                {
                    player_log(kLogLevelTrace, _T("Preload VsFilter OK"));
                }
            }
            if (FAILED(hrVsfilte))
            {
                player_log(kLogLevelTrace, _T("Preload VsFilter FAIL"));
            }
        }
        pInfo = PlayerCore::GetPlayerCodecs().FindCodecsInfo(sClsid_AudioSwitcher, kCodecsTypeAudioEffect);
        if (pInfo)
        {
            CInterfaceList<IUnknown, &IID_IUnknown> pUnks;
            HRESULT hrAudioSwitcher = E_FAIL;
            if (SUCCEEDED(hrAudioSwitcher = PlayerCore::GetPlayerCodecs().CreateCodecsObject(pInfo, &m_pAudioSwitcher, pUnks)))
            {
                if (SUCCEEDED(hrAudioSwitcher = AddFilter(m_pAudioSwitcher, pInfo->name)))
                {
                    player_log(kLogLevelTrace, _T("Preload AudioSwitcher OK"));
                }
            }
            if (FAILED(hrAudioSwitcher))
            {
                player_log(kLogLevelTrace, _T("Preload AudioSwitcher FAIL"));
            }
        }
    }


    CodecsList fl;
    if (FAILED(hr = EnumSourceFilters(lpwcsUrl, fl)))
    {
        return hr;
    }

    hr = VFW_E_CANNOT_RENDER;

    POSITION pos = fl.GetHeadPosition();
    while (pos)
    {
        CComPtr<IBaseFilter> pBF;
        CodecsInfo* info = fl.GetNext(pos);

        if (info &&
            SUCCEEDED(hr = AddSourceFilter(info, lpwcsUrl, info->name, &pBF)))
        {
            if (SUCCEEDED(hr = ConnectFilter(pBF, NULL)))
            {
                return hr;
            }

            NukeDownstream(pBF);
            RemoveFilter(pBF);
        }
    }

    return hr;
}
//*/

HRESULT DirectShowGraph::Core_Render()
{
    player_log(kLogLevelTrace, _T("DirectShowGraph::Core_Render"));
    
    HRESULT hr = E_FAIL;
    CodecsListEx fl;
    if (FAILED(hr = Core_EnumSourceFilters(fl)))
    {
        return hr;
    }

    if (m_bAborted)
    {
        player_log(kLogLevelTrace, _T("DirectShowGraph::Core_Render, open aborted before EnumSourceFilters"));
        return E_ABORT;
    }

    hr = VFW_E_CANNOT_RENDER;

    POSITION pos = fl.GetHeadPosition();
    while (pos)
    {
        CodecsInfo* info = fl.GetNext(pos);
        if (!info)
            continue;
        CComPtr<IBaseFilter> pBF;
        if (SUCCEEDED(hr = Core_AddSourceFilter(info, &pBF)))
        {
            if (m_bAborted)
            {
                player_log(kLogLevelTrace, _T("DirectShowGraph::Core_Render, open aborted before Core_RenderFilter"));
                return E_ABORT;
            }

            if (SUCCEEDED(hr = Core_RenderFilter(pBF)))
            {
                return hr;
            }

            NukeDownstream(pBF);
            RemoveFilter(pBF);
        }
    }

    return hr;
}

HRESULT DirectShowGraph::Core_OnRenderPrepare()
{
    // OnPreloadFilters
    {
        CodecsInfo* pInfo = PlayerCore::GetPlayerCodecs().FindCodecsInfo(sClsid_VsFilter, kCodecsTypeVideoEffect);
        if (pInfo)
        {
            CInterfaceList<IUnknown, &IID_IUnknown> pUnks;
            HRESULT hrVsfilter = Core_AddFilter(pInfo, &m_pVSFilter, pUnks);
            if (FAILED(hrVsfilter))
            {
                player_log(kLogLevelTrace, _T("Preload VsFilter FAIL"));
            }
        }
        pInfo = PlayerCore::GetPlayerCodecs().FindCodecsInfo(sClsid_AudioSwitcher, kCodecsTypeAudioEffect);
        if (pInfo)
        {
            CInterfaceList<IUnknown, &IID_IUnknown> pUnks;
            HRESULT hrAudioSwitcher = Core_AddFilter(pInfo, &m_pAudioSwitcher, pUnks);
            if (FAILED(hrAudioSwitcher))
            {
                player_log(kLogLevelTrace, _T("Preload AudioSwitcher FAIL"));
            }
        }
    }

    return S_OK;
}

HRESULT DirectShowGraph::Core_OnRenderComplete()
{
    player_log(kLogLevelTrace, _T("DirectShowGraph::Core_OnRenderComplete, Used filter list:"));

    HRESULT hr = S_OK;
    CLSID clsid;
    int nCount = 0;

    CLSID clsidVideoRenderer;

    BeginEnumFilters(m_pIGraphBuilder, pEF, pBF)
    {
        nCount++;
        hr = pBF->GetClassID(&clsid);
        if(SUCCEEDED(hr))
        {
            player_log(kLogLevelTrace, _T("Filter %d, %s, %s"), nCount, GetFilterName(pBF), CStringFromGUID(clsid));
        }
    }
    EndEnumFilters;
    //DumpGraph(m_pIGraphBuilder, 0);

    m_pIMediaControl = m_pIGraphBuilder;
    m_pIMediaEventEx = m_pIGraphBuilder;
    m_pIVideoWindow = m_pIGraphBuilder;
    m_pIBasicVideo = m_pIGraphBuilder;
    m_pIBasicAudio = m_pIGraphBuilder;
    m_pIMediaSeeking = m_pIGraphBuilder;

    //hr = m_pIGraphBuilder->QueryInterface(IID_PPV_ARGS(&m_pIVideoWindow));

    if (!m_pIMediaControl ||
        !m_pIMediaEventEx ||
        !m_pIVideoWindow ||
        !m_pIBasicVideo ||
        !m_pIBasicAudio ||
        !m_pIMediaSeeking)
    {
        return E_NOINTERFACE;
    }

    HWND hVideoWnd = m_pPlayer->GetVideoWindow();
    RECT rcDisplay = m_pPlayer->GetDisplayRect();

    if (m_pVideoRenderer)
    {
        hr = m_pVideoRenderer->SetVideoWindow(hVideoWnd);

        hr = m_pVideoRenderer->SetVideoPosition(&rcDisplay);

        VideoSize video_size;
        hr = m_pVideoRenderer->GetVideoSize(&video_size);
        if (SUCCEEDED(hr))
        {
            m_pMediaInfo->SetVideoSize(&video_size);
        }
    }

    hr = m_pIMediaSeeking->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);

    LONGLONG llDuration = 0;
    hr = m_pIMediaSeeking->GetDuration(&llDuration);
    if (SUCCEEDED(hr))
    {
        LONG lDuration = _100NS_UNITS_TO_MILLISECONDS(llDuration);
        player_log(kLogLevelTrace, _T("DirectShowGraph::Core_OnRenderComplete, GetDuration = %d(%s)"),
            lDuration, Millisecs2CString(lDuration));
        m_pMediaInfo->SetDuration(lDuration);
    }

    return hr;
}

//*/

BOOL DirectShowGraph::Core_CheckProtocol(CodecsInfo* info)
{
    CString strProtocol(m_pMediaInfo->GetProtocol());
    if (strProtocol.CompareNoCase(_T("file")) == 0 &&
        info->protocols.IsEmpty())
    {
        return TRUE;
    }
    if (info->protocols.Find(strProtocol))
        return TRUE;
    return FALSE;
}

BOOL DirectShowGraph::Core_CheckBytes(CodecsInfo* info)
{
    if (info->checkbytes.IsEmpty())
        return TRUE;

    POSITION pos = info->checkbytes.GetHeadPosition();
    while (pos)
    {
        CheckByteItem& item = info->checkbytes.GetNext(pos);
        item.matched = false;
        if (!CheckBytes(m_pPlayer->GetStream()->GetHeader(), item.checkbyte))
            continue;
        item.matched = true;
        player_log(kLogLevelTrace, _T("Core_CheckBytes, '%s' matched, subtype = %s"), info->name, item.subtype);
        return TRUE;
    }
    return FALSE;
}

BOOL DirectShowGraph::Core_CheckExtension(CodecsInfo* info)
{
    CString strExt(m_pMediaInfo->GetExtension());

    if (!strExt.IsEmpty() && 
        info->extensions.Find(strExt))
    {
        return TRUE;
    }

    return FALSE;
}

BOOL DirectShowGraph::Core_OnSpecialParse(CodecsInfo** info)
{
    return FALSE;
}

HRESULT DirectShowGraph::Core_EnumSourceFilters(CodecsListEx& fl)
{
    fl.RemoveAll();

    CodecsInfoList& sources = PlayerCore::GetPlayerCodecs().GetSources();


    CodecsInfo* info = NULL;
    POSITION pos = sources.GetHeadPosition();
    bool bCheckbytes = false;
    while (pos)
    {
        info = sources.GetNext(pos);

        if (!info)
            continue;
        if (!info->enable)
            continue;

        // check protocol & checkbytes
        if (!Core_CheckProtocol(info))
            continue;

        if (Core_CheckBytes(info) && !info->checkbytes.IsEmpty())
        {
            bCheckbytes = true;
            fl.Insert(info, 0, 0);
        }
        else if (Core_CheckExtension(info))
        {
            fl.Insert(info, 1, 0);
        }
    }

    // special format parse
     if (!bCheckbytes)
     {
         CodecsInfo* info = NULL;
         ///CString subtype;
         if (Core_OnSpecialParse(&info))
         {
             fl.Insert(info, 0, 1);
         }
     }

    return S_OK;
}

BOOL DirectShowGraph::Core_CanAddFilter(IPin* pPinOut, CodecsInfo* info)
{
    GUID clsid = GUIDFromCString(info->clsid);
    for (CComPtr<IBaseFilter> pBFUS = GetFilterFromPin(pPinOut); pBFUS; pBFUS = GetUpStreamFilter(pBFUS))
    {
        if (/*clsid != CLSID_Proxy && */GetCLSID(pBFUS) == clsid)
        {
            player_log(kLogLevelTrace, _T("Core_CanAddFilter, can NOT add filter '%s'"), info->name);
            return FALSE;
        }
    }
    return TRUE;
}

HRESULT DirectShowGraph::Core_AddFilter(CodecsInfo* info,
                                        IBaseFilter** ppBF,
                                        CInterfaceList<IUnknown, &IID_IUnknown>& pUnks)
{
    HRESULT hr = E_FAIL;

    void* pParam = NULL;
    hr = OnCreateFilterPrepare(info, &pParam);
    if (FAILED(hr))
    {
        return hr;
    }

    CComPtr<IBaseFilter> pBF;
    hr = PlayerCore::GetPlayerCodecs().CreateCodecsObject(info, &pBF, pUnks, pParam);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = OnCreateFilterCompelete(info, pBF);
    if (FAILED(hr))
    {
        return hr;
    }

    if (FAILED(hr = AddFilter(pBF, info->name)))
    {
        return hr;
    }

    *ppBF = pBF.Detach();

    return S_OK;
}

HRESULT DirectShowGraph::AddFilter(IBaseFilter* pBF, LPCWSTR pName)
{
    HRESULT hr = E_FAIL;
    if (FAILED(hr = m_pIGraphBuilder->AddFilter(pBF, pName)))
    {
        player_log(kLogLevelTrace, _T("IGraphBuilder->AddFilter %s failed, hr = 0x%08X"), 
            pName, hr);
        return hr;
    }

    player_log(kLogLevelTrace, _T("Core_AddFilter, %s added succeeded"), pName);
    return S_OK;
}

HRESULT DirectShowGraph::Core_FileSourceFilterLoad(IBaseFilter* pBF, LPCWSTR pSubtype)
{
    HRESULT hr = E_FAIL;

    IFileSourceFilter* pFSF = NULL;
    hr = pBF->QueryInterface(IID_IFileSourceFilter, (void**)&pFSF);
    if (FAILED(hr) || !pFSF)
    {
        return E_NOINTERFACE;
    }

    LPCTSTR pUrl = m_pMediaInfo->GetUrl();
    if(_tcslen(pSubtype) > 0)
    {
        player_log(kLogLevelTrace, _T("Call IFileSource::Load as MEDIATYPE_Stream, %s"), pSubtype);

        AM_MEDIA_TYPE mt;
        memset(&mt, 0, sizeof(mt));
        mt.majortype = MEDIATYPE_Stream;
        ::CLSIDFromString((LPOLESTR)pSubtype, &mt.subtype);

        __try
        {
            hr = pFSF->Load(pUrl, &mt);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            hr = DISP_E_EXCEPTION;
            player_log(kLogLevelError, _T("pFileSource->Load raise exception"));
        }
    }
    else
    {
        player_log(kLogLevelTrace, _T("Call IFileSource::Load as MEDIATYPE_Stream,MEDIASUBTYPE_NULL"));
        hr = pFSF->Load(pUrl, NULL);
    }

    pFSF->Release();


    return hr;
}

HRESULT DirectShowGraph::Core_AddSourceFilter(CodecsInfo* info, IBaseFilter** ppBF)
{
    HRESULT hr = E_FAIL;

    CComPtr<IBaseFilter> pBF;
    CInterfaceList<IUnknown, &IID_IUnknown> pUnks;
    if (FAILED(hr = Core_AddFilter(info, &pBF, pUnks)))
    {
        return hr;
    }

    CString strSubtype;
    POSITION pos = info->checkbytes.GetHeadPosition();
    while (pos)
    {
        CheckByteItem& chkbytes = info->checkbytes.GetNext(pos);
        if (chkbytes.matched)
        {
            strSubtype = chkbytes.subtype;
            break;
        }
    }
    if (FAILED(hr = Core_FileSourceFilterLoad(pBF, strSubtype)))
    {
        return hr;
    }

    *ppBF = pBF.Detach();

    m_pUnks.AddTailList(&pUnks);

    return S_OK;
}


HRESULT DirectShowGraph::Core_ConnectFilterDirect(IPin* pPinOut, IBaseFilter* pBF, const AM_MEDIA_TYPE* pmt)
{
    CheckPointer(pPinOut, E_POINTER);
    CheckPointer(pBF, E_POINTER);

    if (S_OK != IsPinDirection(pPinOut, PINDIR_OUTPUT))
    {
        return VFW_E_INVALID_DIRECTION;
    }

    BeginEnumPins(pBF, pEP, pPin)
    {
        if (S_OK == IsPinDirection(pPin, PINDIR_INPUT)
            && S_OK != IsPinConnected(pPin))
        {
            HRESULT hr = m_pIGraphBuilder->ConnectDirect(pPinOut, pPin, pmt);
            if (SUCCEEDED(hr))
            {
                return hr;
            }
        }
    }
    EndEnumPins;

    return VFW_E_CANNOT_CONNECT;
}

HRESULT DirectShowGraph::Core_RenderPin(IPin* pPinOut)
{
    HRESULT hr;

    if (S_OK != IsPinDirection(pPinOut, PINDIR_OUTPUT))
    {
        return VFW_E_INVALID_DIRECTION;
    }

    if (S_OK == IsPinConnected(pPinOut))
    {
        return VFW_E_ALREADY_CONNECTED;
    }

    bool fDeadEnd = true;

    // 1. Try filters in the graph
    {
        player_log(kLogLevelTrace, _T("Core_RenderPin, Trying filters in the graph"));

        CInterfaceList<IBaseFilter> pBFs;

        BeginEnumFilters(m_pIGraphBuilder, pEF, pBF)
        {
            if (GetFilterFromPin(pPinOut) == pBF)
            {
                continue;
            }

            pBFs.AddTail(pBF);
        }
        EndEnumFilters;

        POSITION pos = pBFs.GetHeadPosition();
        while (pos)
        {
            IBaseFilter* pBF = pBFs.GetNext(pos);

            CString strFilterName = GetFilterName(pBF);
            player_log(kLogLevelTrace, _T("Trying connect to %s"), strFilterName);

            if (SUCCEEDED(hr = Core_ConnectFilterDirect(pPinOut, pBF, NULL)))
            {
                player_log(kLogLevelTrace, _T("Core_ConnectFilterDirect OK"));

                if (!IsStreamEnd(pBF))
                {
                    fDeadEnd = false;
                }

                player_log(kLogLevelTrace, _T("==> Continue to render '%s'"), strFilterName);
                if (SUCCEEDED(hr = Core_RenderFilter(pBF)))
                {
                    return hr;
                }
            }
            else
            {
                player_log(kLogLevelTrace, _T("Core_ConnectFilterDirect FAIL"));
            }

            EXECUTE_ASSERT(Disconnect(pPinOut));
        }
    }

    // 2. Look up filters in the <codecs>
    {
        player_log(kLogLevelTrace, _T("Core_RenderPin, Trying filters in <codecs>"));

        CodecsListEx fl;
        CAtlArray<GUID> types;
        ExtractMediaTypes(pPinOut, types);

        CodecsInfoList& transforms = PlayerCore::GetPlayerCodecs().GetTransforms();
        POSITION pos = transforms.GetHeadPosition();
        while (pos)
        {
            int custom_priority = 0;
            CodecsInfo* info = transforms.GetNext(pos);
            if (info->enable && info->CheckTypes(types, false))
            {
                if (info->clsid.CompareNoCase(sClsid_VsFilter) == 0 ||
                    info->clsid.CompareNoCase(sClsid_AudioSwitcher) == 0)
                {
                    continue;
                }

                if (info->type == kCodecsTypeVideoRenderer)
                {
                    if (info->clsid.CompareNoCase(PlayerCore::GetPlayerSettings().m_SelVideoRendererClsid) == 0)
                    {
                        custom_priority = 1;
                    }
                }

                fl.Insert(info, 0, info->CheckTypes(types, true), custom_priority);
            }
        }

        pos = fl.GetHeadPosition();
        while (pos)
        {
            CodecsInfo* info = fl.GetNext(pos);
            player_log(kLogLevelTrace, _T("Connecting '%s'"), info->name);

            if (!Core_CanAddFilter(pPinOut, info))
            {
                continue;
            }

            CComPtr<IBaseFilter> pBF;
            CInterfaceList<IUnknown, &IID_IUnknown> pUnks;
            if (FAILED(hr = Core_AddFilter(info, &pBF, pUnks)))
            {
                pUnks.RemoveAll();
                pBF.Release();
                continue;
            }

            hr = Core_ConnectFilterDirect(pPinOut, pBF, NULL);
            if (SUCCEEDED(hr))
            {
                player_log(kLogLevelTrace, _T("Core_ConnectFilterDirect '%s' => '%s' OK"), 
                    GetFilterName(GetFilterFromPin(pPinOut)), info->name);

                if (!IsStreamEnd(pBF))
                {
                    fDeadEnd = false;
                }
//                 else // ???
//                 {
//                     player_log(kLogLevelTrace, _T("Render routine reach to stream end, '%s'"), info->name);
//                     return hr;
//                 }

                player_log(kLogLevelTrace, _T("==> Continue to render '%s'"), info->name);
                //Sleep(3000);
                hr = Core_RenderFilter(pBF);

                if (SUCCEEDED(hr))
                {
                    m_pUnks.AddTailList(&pUnks);

                    hr = OnRenderFilterEnd(info, pBF);

                    return hr;
                }                
            }
            else
            {
                player_log(kLogLevelTrace, _T("Core_ConnectFilterDirect to '%s' FAIL"), info->name);
            }

            EXECUTE_ASSERT(SUCCEEDED(RemoveFilter(pBF)));
            pUnks.RemoveAll();
            pBF.Release();
        }
    }

    return VFW_E_CANNOT_RENDER;
}

HRESULT DirectShowGraph::Core_RenderFilter(IBaseFilter* pBF)
{
    int nRendered = 0, nTotal = 0;

    BeginEnumPins(pBF, pEP, pPin)
    {
        BOOL bOutput = (S_OK == IsPinDirection(pPin, PINDIR_OUTPUT));
        BOOL bConnected = (S_OK == IsPinConnected(pPin));
        if (bOutput && !bConnected)
        {
            HRESULT hr = Core_RenderPin(pPin);
            if (SUCCEEDED(hr))
            {
                nRendered++;
            }
            nTotal++;
        }
    }
    EndEnumPins;

    HRESULT hr = VFW_E_CANNOT_RENDER;
    if (nRendered == nTotal)
    {
        hr = nRendered > 0 ? S_OK : S_FALSE;
    }
    else
    {
        hr = nRendered > 0 ? VFW_S_PARTIAL_RENDER : VFW_E_CANNOT_RENDER;
    }

    return hr;
}


HRESULT DirectShowGraph::EnumSourceFilters(LPCWSTR lpcwstrFileName, CodecsList& fl)
{
    // TODO: use overrides

    CheckPointer(lpcwstrFileName, E_POINTER);

    fl.RemoveAll();

    CStringW fn = CStringW(lpcwstrFileName).TrimLeft();
    CStringW protocol = fn.Left(fn.Find(':') + 1).TrimRight(':').MakeLower();
    CStringW ext = CPathW(fn).GetExtension().MakeLower();

    HANDLE hFile = INVALID_HANDLE_VALUE;

    if (protocol.GetLength() <= 1 || protocol == L"file")
    {
        hFile = CreateFile(CString(fn), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);

        if (hFile == INVALID_HANDLE_VALUE)
        {
            return VFW_E_NOT_FOUND;
        }
    }

    CodecsInfoList& sources = PlayerCore::GetPlayerCodecs().GetSources();
    CodecsInfo* info = NULL;
    POSITION pos = sources.GetHeadPosition();
    while (pos)
    {
        info = sources.GetNext(pos);

        if (hFile == INVALID_HANDLE_VALUE)
        {
            // internal / protocol
            if (info->protocols.Find(CString(protocol)))
            {
                fl.Insert(info, 0, false, false);
            }
        }
        else
        {
            // internal / check bytes
            POSITION pos2 = info->checkbytes.GetHeadPosition();
            while (pos2)
            {
                CheckByteItem& item = info->checkbytes.GetNext(pos2);
                if (CheckBytes(hFile, item.checkbyte))
                {
                    fl.Insert(info, 1, false, false);
                    break;
                }
            }
        }

        if (!ext.IsEmpty())
        {
            // internal / file extension

            if (info->extensions.Find(CString(ext)))
            {
                fl.Insert(info, 2, false, false);
            }
        }

        {
            // internal / the rest

            if (info->protocols.IsEmpty() &&
                info->checkbytes.IsEmpty() &&
                info->extensions.IsEmpty())
            {
                fl.Insert(info, 3, false, false);
            }
        }

    }

    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
    }

    return S_OK;
}

HRESULT DirectShowGraph::AddSourceFilter(CodecsInfo* info, LPCWSTR lpcwstrFileName, LPCWSTR lpcwstrFilterName, IBaseFilter** ppBF)
{
    player_log(kLogLevelTrace, _T("DirectShowGraph::AddSourceFilter, trying '%s'"), info->name);

    CheckPointer(lpcwstrFileName, E_POINTER);
    CheckPointer(ppBF, E_POINTER);

    //ASSERT(*ppBF == NULL);

    HRESULT hr;

    CComPtr<IBaseFilter> pBF;
    CInterfaceList<IUnknown, &IID_IUnknown> pUnks;
    if (FAILED(hr = PlayerCore::GetPlayerCodecs().CreateCodecsObject(info, &pBF, pUnks)))
    {
        return hr;
    }

    CComQIPtr<IFileSourceFilter> pFSF = pBF;
    if (!pFSF) {
        return E_NOINTERFACE;
    }

    if (FAILED(hr = AddFilter(pBF, lpcwstrFilterName))) {
        return hr;
    }

    AM_MEDIA_TYPE mt;
    memset(&mt, 0, sizeof(mt));
    mt.majortype = MEDIATYPE_Stream;

    POSITION pos = info->checkbytes.GetHeadPosition();
    while (pos)
    {
        CheckByteItem& item = info->checkbytes.GetNext(pos);
        if (1/*item.matched*/)
        {
            player_log(kLogLevelTrace, _T("find matched (checkbytes,subtype) (%s|%s)"),
                item.checkbyte, item.subtype);
            GUIDFromCString(item.subtype, mt.subtype);
            break;
        }
    }

    // sometimes looping with AviSynth
    if (FAILED(hr = pFSF->Load(lpcwstrFileName, &mt))) {
        player_log(kLogLevelTrace, _T("IFileSourceFilter Load failed, hr = 0x%08X"),
            hr);
        RemoveFilter(pBF);
        return hr;
    }

    *ppBF = pBF.Detach();

    m_pUnks.AddTailList(&pUnks);

    return S_OK;
}

HRESULT DirectShowGraph::RemoveFilter(IBaseFilter* pFilter)
{
    if (!m_pIGraphBuilder) {
        return E_UNEXPECTED;
    }

    CAutoLock cAutoLock(&m_Lock);

    return m_pIGraphBuilder->RemoveFilter(pFilter);
}

// HRESULT DirectShowGraph::AddFilter(IBaseFilter* pFilter, LPCWSTR pName)
// {
//     if (!m_pIGraphBuilder) {
//         return E_UNEXPECTED;
//     }
// 
//     CAutoLock cAutoLock(&m_Lock);
// 
//     HRESULT hr;
// 
//     if (FAILED(hr = m_pIGraphBuilder->AddFilter(pFilter, pName))) {
//         player_log(kLogLevelTrace, _T("DirectShowGraph::AddFilter, IGraphBuilder AddFilter %s failed, hr = 0x%08X"), 
//             pName, hr);
//         return hr;
//     }
// 
//     player_log(kLogLevelTrace, _T("DirectShowGraph::AddFilter, %s added succeeded"), 
//         pName);
// 
//     return hr;
// }

HRESULT DirectShowGraph::ConnectFilter(IBaseFilter* pBF, IPin* pPinIn)
{
    CAutoLock cAutoLock(&m_Lock);

    CheckPointer(pBF, E_POINTER);

    if (pPinIn && S_OK != IsPinDirection(pPinIn, PINDIR_INPUT)) {
        return VFW_E_INVALID_DIRECTION;
    }

    int nTotal = 0, nRendered = 0;


    BeginEnumPins(pBF, pEP, pPin) {
        if (S_OK == IsPinDirection(pPin, PINDIR_OUTPUT)
            && S_OK != IsPinConnected(pPin)
            /*&& !((s.iDSVideoRendererType != VIDRNDT_DS_EVR_CUSTOM && s.iDSVideoRendererType != VIDRNDT_DS_EVR && s.iDSVideoRendererType != VIDRNDT_DS_SYNC) && GetPinName(pPin)[0] == '~')*/
            )
        {

                CLSID clsid;
                pBF->GetClassID(&clsid);

                //m_streampath.Append(pBF, pPin);

                HRESULT hr = Connect(pPin, pPinIn);

                if (SUCCEEDED(hr)) {
//                     for (ptrdiff_t i = m_deadends.GetCount() - 1; i >= 0; i--) {
//                         if (m_deadends[i]->Compare(m_streampath)) {
//                             m_deadends.RemoveAt(i);
//                         }
//                     }
                    nRendered++;
                }

                nTotal++;

                //m_streampath.RemoveTail();

                if (SUCCEEDED(hr) && pPinIn) {
                    return S_OK;
                }
        }
    }
    EndEnumPins;

    return
        nRendered == nTotal ? (nRendered > 0 ? S_OK : S_FALSE) :
        nRendered > 0 ? VFW_S_PARTIAL_RENDER :
        VFW_E_CANNOT_RENDER;
}

HRESULT DirectShowGraph::ConnectFilter(IPin* pPinOut, IBaseFilter* pBF)
{
    CAutoLock cAutoLock(&m_Lock);

    CheckPointer(pPinOut, E_POINTER);
    CheckPointer(pBF, E_POINTER);

    if (S_OK != IsPinDirection(pPinOut, PINDIR_OUTPUT)) {
        return VFW_E_INVALID_DIRECTION;
    }

    BeginEnumPins(pBF, pEP, pPin) {
        if (S_OK == IsPinDirection(pPin, PINDIR_INPUT)
            && S_OK != IsPinConnected(pPin)
            /*&& !((s.iDSVideoRendererType != VIDRNDT_DS_EVR_CUSTOM && s.iDSVideoRendererType != VIDRNDT_DS_EVR && s.iDSVideoRendererType != VIDRNDT_DS_SYNC) && GetPinName(pPin)[0] == '~')*/
            )
        {
                HRESULT hr = Connect(pPinOut, pPin);
                if (SUCCEEDED(hr)) {
                    return hr;
                }
        }
    }
    EndEnumPins;

    return VFW_E_CANNOT_CONNECT;
}

HRESULT DirectShowGraph::Connect(IPin* pPinOut, IPin* pPinIn)
{
    return Connect(pPinOut, pPinIn, true);
}

HRESULT DirectShowGraph::Connect(IPin* pPinOut, IPin* pPinIn, bool bContinueRender)
{
    CAutoLock cAutoLock(&m_Lock);

    CheckPointer(pPinOut, E_POINTER);

    HRESULT hr;

    if (S_OK != IsPinDirection(pPinOut, PINDIR_OUTPUT)
        || pPinIn && S_OK != IsPinDirection(pPinIn, PINDIR_INPUT)) {
            return VFW_E_INVALID_DIRECTION;
    }

    if (S_OK == IsPinConnected(pPinOut)
        || pPinIn && S_OK == IsPinConnected(pPinIn)) {
            return VFW_E_ALREADY_CONNECTED;
    }

    bool fDeadEnd = true;

    if (pPinIn)
    {
        // 1. Try a direct connection between the filters, with no intermediate filters
        if (SUCCEEDED(hr = ConnectDirect(pPinOut, pPinIn, NULL)))
        {
            return hr;
        }
    }

    // 1. Try filters in the graph
    {
        player_log(kLogLevelTrace, _T("Trying filters in the graph"));

        CInterfaceList<IBaseFilter> pBFs;

        BeginEnumFilters(m_pIGraphBuilder, pEF, pBF)
        {
            if (pPinIn && GetFilterFromPin(pPinIn) == pBF
                || GetFilterFromPin(pPinOut) == pBF)
            {
                continue;
            }

            // HACK: ffdshow - audio capture filter
            if (GetCLSID(pPinOut) == GUIDFromCString(_T("{04FE9017-F873-410E-871E-AB91661A4EF7}"))
                && GetCLSID(pBF) == GUIDFromCString(_T("{E30629D2-27E5-11CE-875D-00608CB78066}")))
            {
                continue;
            }

            pBFs.AddTail(pBF);
        }
        EndEnumFilters;

        POSITION pos = pBFs.GetHeadPosition();
        while (pos)
        {
            IBaseFilter* pBF = pBFs.GetNext(pos);

            CString strFilterName = GetFilterName(pBF);

            player_log(kLogLevelTrace, _T("Trying connecting %s"), strFilterName);

            if (SUCCEEDED(hr = ConnectFilterDirect(pPinOut, pBF, NULL)))
            {
                player_log(kLogLevelTrace, _T("ConnectFilterDirect OK"));

                if (!IsStreamEnd(pBF))
                {
                    fDeadEnd = false;
                }

                player_log(kLogLevelTrace, _T("==> Continue to render '%s'"), strFilterName);

                if (SUCCEEDED(hr = ConnectFilter(pBF, pPinIn)))
                {
                    return hr;
                }
            }
            else
            {
                player_log(kLogLevelTrace, _T("ConnectFilterDirect FAIL"));
            }

            EXECUTE_ASSERT(Disconnect(pPinOut));
        }
    }

    // 2. Look up filters in the <codecs>
    {
        player_log(kLogLevelTrace, _T("Trying filters in <codecs>"));

        CodecsList fl;

        CAtlArray<GUID> types;
        ExtractMediaTypes(pPinOut, types);

        CodecsInfoList& transforms = PlayerCore::GetPlayerCodecs().GetTransforms();
        POSITION pos = transforms.GetHeadPosition();
        while (pos)
        {
            CodecsInfo* info = transforms.GetNext(pos);
            if (info->enable && info->CheckTypes(types, false))
            {
                fl.Insert(info, 0, info->CheckTypes(types, true), false);
            }
        }

        pos = fl.GetHeadPosition();
        while (pos)
        {
            CodecsInfo* info = fl.GetNext(pos);

            // Checks if madVR is already in the graph to avoid two instances at the same time
//             CComPtr<IBaseFilter> pBFmadVR;
//             FindFilterByName(_T("madVR Renderer"), &pBFmadVR);
//             if (pBFmadVR && (pFGF->GetName() == _T("madVR Renderer")))
//             {
//                 continue;
//             }

//             if (pMadVRAllocatorPresenter && (pFGF->GetCLSID() == CLSID_madVR))
//             {
//                 // the pure madVR filter was selected (without the allocator presenter)
//                 // subtitles, OSD etc don't work correctly without the allocator presenter
//                 // so we prefer the allocator presenter over the pure filter
//                 pFGF = pMadVRAllocatorPresenter;
//             }

            // Checks if vsfilter/audioswitcher is already in the graph to avoid two instances at the same time
            {
                if (info->clsid.CompareNoCase(sClsid_VsFilter) == 0 &&
                    SUCCEEDED(FindFilterByClsid(sClsid_VsFilter, NULL)))
                {
                    player_log(kLogLevelTrace, _T("VsFilter already in graph, skip it"));
                    continue;
                }
                if (info->clsid.CompareNoCase(sClsid_AudioSwitcher) == 0 &&
                    SUCCEEDED(FindFilterByClsid(sClsid_AudioSwitcher, NULL)))
                {
                    player_log(kLogLevelTrace, _T("AudioSwitcher already in graph, skip it"));
                    continue;
                }
            }

            //TRACE(_T("FGM: Connecting '%s'\n"), pFGF->GetName());
            player_log(kLogLevelTrace, _T("Connecting '%s'"), info->name);

            CComPtr<IBaseFilter> pBF;
            CInterfaceList<IUnknown, &IID_IUnknown> pUnks;
            if (FAILED(PlayerCore::GetPlayerCodecs().CreateCodecsObject(info, &pBF, pUnks)))
            {
                //TRACE(_T("     --> Filter creation failed\n"));
                continue;
            }

            if (FAILED(hr = AddFilter(pBF, info->name)))
            {
                //TRACE(_T("     --> Adding the filter failed\n"));
                pUnks.RemoveAll();
                pBF.Release();
                continue;
            }

            hr = ConnectFilterDirect(pPinOut, pBF, NULL);
            /*
            if (FAILED(hr))
            {
            if (types.GetCount() >= 2 && types[0] == MEDIATYPE_Stream && types[1] != GUID_NULL)
            {
            CMediaType mt;

            mt.majortype = types[0];
            mt.subtype = types[1];
            mt.formattype = FORMAT_None;
            if (FAILED(hr)) hr = ConnectFilterDirect(pPinOut, pBF, &mt);

            mt.formattype = GUID_NULL;
            if (FAILED(hr)) hr = ConnectFilterDirect(pPinOut, pBF, &mt);
            }
            }
            */
            if (SUCCEEDED(hr))
            {
                player_log(kLogLevelTrace, _T("Connect Filter '%s' => '%s' OK"), 
                    GetFilterName(GetFilterFromPin(pPinOut)), info->name);

                if (!IsStreamEnd(pBF))
                {
                    fDeadEnd = false;
                }

                if (bContinueRender)
                {
                    player_log(kLogLevelTrace, _T("==> Continue to render '%s'"), info->name);
                    hr = ConnectFilter(pBF, pPinIn);
                }

                if (SUCCEEDED(hr))
                {
                    m_pUnks.AddTailList(&pUnks);
                    return hr;
                }
                
            }
            else
            {
                player_log(kLogLevelTrace, _T("ConnectFilterDirect to '%s' FAIL"), info->name);
            }

            EXECUTE_ASSERT(SUCCEEDED(RemoveFilter(pBF)));
            //TRACE(_T("     --> Failed to connect\n"));
            pUnks.RemoveAll();
            pBF.Release();
        }
    }

//     if (fDeadEnd)
//     {
//         CAutoPtr<CStreamDeadEnd> psde(new CStreamDeadEnd());
//         psde->AddTailList(&m_streampath);
//         int skip = 0;
//         BeginEnumMediaTypes(pPinOut, pEM, pmt)
//         {
//             if (pmt->majortype == MEDIATYPE_Stream && pmt->subtype == MEDIASUBTYPE_NULL)
//             {
//                 skip++;
//             }
//             psde->mts.AddTail(CMediaType(*pmt));
//         }
//         EndEnumMediaTypes(pmt);
//         if (skip < (int)psde->mts.GetCount())
//         {
//             m_deadends.Add(psde);
//         }
//     }

    return pPinIn ? VFW_E_CANNOT_CONNECT : VFW_E_CANNOT_RENDER;
}

HRESULT DirectShowGraph::ConnectFilterDirect(IPin* pPinOut, IBaseFilter* pBF, const AM_MEDIA_TYPE* pmt)
{
    CAutoLock cAutoLock(&m_Lock);

    CheckPointer(pPinOut, E_POINTER);
    CheckPointer(pBF, E_POINTER);

    if (S_OK != IsPinDirection(pPinOut, PINDIR_OUTPUT)) {
        return VFW_E_INVALID_DIRECTION;
    }

    BeginEnumPins(pBF, pEP, pPin) {
        if (S_OK == IsPinDirection(pPin, PINDIR_INPUT)
            && S_OK != IsPinConnected(pPin)
            /*&& !((s.iDSVideoRendererType != VIDRNDT_DS_EVR_CUSTOM && s.iDSVideoRendererType != VIDRNDT_DS_EVR && s.iDSVideoRendererType != VIDRNDT_DS_SYNC) && GetPinName(pPin)[0] == '~'))*/
        )
        {
                HRESULT hr = ConnectDirect(pPinOut, pPin, pmt);
                if (SUCCEEDED(hr)) {
                    return hr;
                }
        }
    }
    EndEnumPins;

    return VFW_E_CANNOT_CONNECT;
}

HRESULT DirectShowGraph::ConnectDirect(IPin* pPinOut, IPin* pPinIn, const AM_MEDIA_TYPE* pmt)
{
    if (!m_pIGraphBuilder) {
        return E_UNEXPECTED;
    }

    CAutoLock cAutoLock(&m_Lock);

    CComPtr<IBaseFilter> pBF = GetFilterFromPin(pPinIn);
    CLSID clsid = GetCLSID(pBF);

    // TODO: GetUpStreamFilter goes up on the first input pin only
    for (CComPtr<IBaseFilter> pBFUS = GetFilterFromPin(pPinOut); pBFUS; pBFUS = GetUpStreamFilter(pBFUS))
    {
        if (pBFUS == pBF)
        {
            return VFW_E_CIRCULAR_GRAPH;
        }
        if (/*clsid != CLSID_Proxy &&*/ GetCLSID(pBFUS) == clsid)
        {
            return VFW_E_CANNOT_CONNECT;
        }
    }

    return m_pIGraphBuilder->ConnectDirect(pPinOut, pPinIn, pmt);
}

HRESULT DirectShowGraph::NukeDownstream(IUnknown* pUnk)
{
    CAutoLock cAutoLock(&m_Lock);

    if (CComQIPtr<IBaseFilter> pBF = pUnk) {
        BeginEnumPins(pBF, pEP, pPin) {
            NukeDownstream(pPin);
        }
        EndEnumPins;
    } else if (CComQIPtr<IPin> pPin = pUnk) {
        CComPtr<IPin> pPinTo;
        if (S_OK == IsPinDirection(pPin, PINDIR_OUTPUT)
            && SUCCEEDED(pPin->ConnectedTo(&pPinTo)) && pPinTo) {
                if (CComPtr<IBaseFilter> pBF = GetFilterFromPin(pPinTo)) {
                    NukeDownstream(pBF);
                    Disconnect(pPinTo);
                    Disconnect(pPin);
                    RemoveFilter(pBF);
                }
        }
    } else {
        return E_INVALIDARG;
    }

    return S_OK;
}

HRESULT DirectShowGraph::IsPinDirection(IPin* pPin, PIN_DIRECTION dir1)
{
    CAutoLock cAutoLock(&m_Lock);

    CheckPointer(pPin, E_POINTER);

    PIN_DIRECTION dir2;
    if (FAILED(pPin->QueryDirection(&dir2)))
    {
        return E_FAIL;
    }

    return dir1 == dir2 ? S_OK : S_FALSE;
}

HRESULT DirectShowGraph::IsPinConnected(IPin* pPin)
{
    CAutoLock cAutoLock(&m_Lock);

    CheckPointer(pPin, E_POINTER);

    CComPtr<IPin> pPinTo;
    return SUCCEEDED(pPin->ConnectedTo(&pPinTo)) && pPinTo ? S_OK : S_FALSE;
}

HRESULT DirectShowGraph::Disconnect(IPin* ppin)
{
    if (!m_pIGraphBuilder) {
        return E_UNEXPECTED;
    }

    CAutoLock cAutoLock(&m_Lock);

    return m_pIGraphBuilder->Disconnect(ppin);
}

bool DirectShowGraph::CheckBytes(MEMORY_DATA* header, CString chkbytes)
{
    CheckPointer(header, false);

    CAtlList<CString> sl;
    Explode(chkbytes, sl, ',');

    if (sl.GetCount() < 4)
    {
        player_log(kLogLevelError,  _T("Explode chkbytes = %s got error"), chkbytes);
        return false;
    }

    ASSERT(!(sl.GetCount() & 3));

    while (sl.GetCount() >= 4)
    {
        CString offsetstr = sl.RemoveHead();
        CString cbstr = sl.RemoveHead();
        CString maskstr = sl.RemoveHead();
        CString valstr = sl.RemoveHead();

        long cb = _ttol(cbstr);

        if (offsetstr.IsEmpty() || cbstr.IsEmpty()
            || valstr.IsEmpty() || (valstr.GetLength() & 1)
            || cb * 2 != valstr.GetLength())
        {
            player_log(kLogLevelError,  _T("parse chckbytes got error"));
            return false;
        }

        __int64 offset = _ttoi64(offsetstr);
        if (offset < 0 || offset + cb > header->size)
        {
            player_log(kLogLevelError,  _T("checkbytes offset = %I64d, cb = %d, out of header size = %d"),
                offset, cb, header->size);
            return false;
        }

        // LAME
        while (maskstr.GetLength() < valstr.GetLength())
        {
            maskstr += 'F';
        }

        CAtlArray<BYTE> mask, val;
        CStringToBin(maskstr, mask);
        CStringToBin(valstr, val);

        for (int i = 0, j = offset; i < val.GetCount() && j < header->size; i++, j++)
        {
            if ((header->data[j] & mask[i]) != val[i])
            {
                return false;
            }
        }
    }
    return sl.IsEmpty();
}

bool DirectShowGraph::CheckBytes(HANDLE hFile, CString chkbytes)
{
    CAtlList<CString> sl;
    Explode(chkbytes, sl, ',');

    if (sl.GetCount() < 4) {
        return false;
    }

    ASSERT(!(sl.GetCount() & 3));

    LARGE_INTEGER size = {0, 0};
    GetFileSizeEx(hFile, &size);

    while (sl.GetCount() >= 4) {
        CString offsetstr = sl.RemoveHead();
        CString cbstr = sl.RemoveHead();
        CString maskstr = sl.RemoveHead();
        CString valstr = sl.RemoveHead();

        long cb = _ttol(cbstr);

        if (offsetstr.IsEmpty() || cbstr.IsEmpty()
            || valstr.IsEmpty() || (valstr.GetLength() & 1)
            || cb * 2 != valstr.GetLength()) {
                return false;
        }

        LARGE_INTEGER offset;
        offset.QuadPart = _ttoi64(offsetstr);
        if (offset.QuadPart < 0) {
            offset.QuadPart = size.QuadPart - offset.QuadPart;
        }
        SetFilePointerEx(hFile, offset, &offset, FILE_BEGIN);

        // LAME
        while (maskstr.GetLength() < valstr.GetLength()) {
            maskstr += 'F';
        }

        CAtlArray<BYTE> mask, val;
        CStringToBin(maskstr, mask);
        CStringToBin(valstr, val);

        for (size_t i = 0; i < val.GetCount(); i++) {
            BYTE b;
            DWORD r;
            if (!ReadFile(hFile, &b, 1, &r, NULL) || (b & mask[i]) != val[i]) {
                return false;
            }
        }
    }

    return sl.IsEmpty();
}

HRESULT DirectShowGraph::FindFilterByClsid(LPCTSTR pClsid, IBaseFilter** ppBF)
{
    HRESULT hr = E_FAIL;

    GUID Clsid2Find = GUIDFromCString(pClsid);
    GUID clsid; 

    BeginEnumFilters(m_pIGraphBuilder, pEF, pBF)
    {
        if (SUCCEEDED(pBF->GetClassID(&clsid)))
        {
            if (Clsid2Find == clsid)
            {
                if (ppBF)
                {
                    *ppBF = pBF;
                }
                return S_OK;
            }
        }
    }
    EndEnumFilters;

    if (ppBF)
    {
        *ppBF = NULL;
    }
    
    return hr;
}

HRESULT DirectShowGraph::FindInterface(REFIID iid, void** ppv, BOOL bRemove)
{
    CheckPointer(ppv, E_POINTER);

    for (POSITION pos = m_pUnks.GetHeadPosition(); pos; m_pUnks.GetNext(pos))
    {
        if (SUCCEEDED(m_pUnks.GetAt(pos)->QueryInterface(iid, ppv)))
        {
            if (bRemove)
            {
                m_pUnks.RemoveAt(pos);
            }
            return S_OK;
        }
    }
    return E_NOINTERFACE;
}

HRESULT DirectShowGraph::OnCreateFilterPrepare(CodecsInfo* pInfo, void** pParam)
{
    GUID clsid = GUIDFromCString(pInfo->clsid);
    
    if (clsid == __uuidof(PlayerAsyncReader))
    {
        *pParam = (void*)m_pPlayer->GetStream();
    }

//     if (pInfo->type == kCodecsTypeVideoRenderer)
//     {
//         *pParam = (void*)m_pPlayer;
//     }

    if (clsid == CLSID_EVRAllocatorPresenter)
    {
        HWND hVideoWindow = m_pPlayer->GetVideoWindow();
        bool bFullScreen = false;
        HRESULT hr;
        m_pVideoRenderer = new VideoRendererEVRCP(hr, hVideoWindow, bFullScreen);

        *pParam = m_pVideoRenderer;
    }
    
    return S_OK;
}

HRESULT DirectShowGraph::OnCreateFilterCompelete(CodecsInfo* pInfo, IBaseFilter* pBF)
{
    HRESULT hr = S_OK;

//     if (CComQIPtr<IEVRFilterConfig> pConfig = pBF)
//     {
//         HRESULT hrConfig = pConfig->SetNumberOfStreams(3);
//         if (FAILED(hrConfig))
//         {
//             player_log(kLogLevelError, _T("IEVRFilterConfig->SetNumberOfStreams(3) FAIL, hr = 0x%08x"), hrConfig);
//         }
//     }

    return hr;
}

HRESULT DirectShowGraph::OnRenderFilterEnd(CodecsInfo* pInfo, IBaseFilter* pBF)
{
    HRESULT hr = S_OK;

    if (pInfo->type == kCodecsTypeVideoRenderer)
    {
        GUID clsid = GUIDFromCString(pInfo->clsid);

        //SAFE_DELETE(m_pVideoRenderer);

        if (clsid == CLSID_EnhancedVideoRenderer)
        {
            m_pVideoRenderer = new VideoRendererEVR(hr, pBF);
        }
//         else if (clsid == CLSID_EVRAllocatorPresenter)
//         {
//             HWND hVideoWindow = m_pPlayer->GetVideoWindow();
//             bool bFullScreen = false;
//             m_pVideoRenderer = new VideoRendererEVRCP(hr, hVideoWindow, bFullScreen, pBF);
//         }
//         else if (clsid == CLSID_VideoMixingRenderer9)
//         {
//             m_pVideoRenderer = new VideoRendererVMR9(pBF, &hr);
//         }
//         else if (clsid == CLSID_VideoMixingRenderer)
//         {
//             m_pVideoRenderer = new VideoRendererVMR7(pBF, &hr);
//         }
    }

    return hr;
}

HRESULT DirectShowGraph::RemoveAllFilter()
{
    HRESULT hr = S_OK;

    BeginEnumFilters(m_pIGraphBuilder, pEF, pBF)
    {
        HRESULT hrRemove = RemoveFilter(pBF);
        pEF->Reset();
    }
    EndEnumFilters;

    return hr;
}