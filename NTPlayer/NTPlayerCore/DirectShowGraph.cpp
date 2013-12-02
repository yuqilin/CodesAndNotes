#include "stdafx.h"
#include <atlpath.h>
#include "DirectShowGraph.h"
#include "PlayerCore.h"
#include "PlayerBaseStream.h"
#include "PlayerAsyncReader.h"
#include "VideoRendererEVR.h"
#include "VideoRendererEVRCP.h"
#include "../filters/renderer/VideoRenderers/AllocatorCommon.h"
#include "TextPassThruFilter.h"
#include "moreuuids.h"
#include "../Subtitles/subtitles.h"


#define _100NS_UNITS_TO_MILLISECONDS(refTime) \
    ((LONG)(refTime / 10000))

#define sClsid_VsFilter         _T("{93A22E7A-5091-45EF-BA61-6DA26156A5D0}")
#define sClsid_AudioSwitcher    _T("{18C16B08-6497-420E-AD14-22D21C2CEAB7}")

// #define LOGAUTO \
//     LogPrefix logauto(this)


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

    if (m_pCAP)
    {
        AddTextPassThruFilter();
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

    //     if (m_pVideoRenderer)
    //     {
    //         m_pVideoRenderer->RepaintVideo();
    //     }

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
        m_GraphMutex.unlock();
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

    FastMutex::ScopedLock lock(m_GraphMutex);
    if (m_pIMediaSeeking == NULL)
    {
        return E_FAIL;
    }

    LONGLONG llPosition = MILLISECONDS_TO_100NS_UNITS(pos_to_play);
    hr = m_pIMediaSeeking->SetPositions(&llPosition, AM_SEEKING_AbsolutePositioning,//AM_SEEKING_SeekToKeyFrame
        NULL, AM_SEEKING_NoPositioning);

    if (FAILED(hr))
    {
        player_log(kLogLevelError, _T("DirectShowGraph::SetPlayPos, IMediaSeeking->SetPositions failed, hr = 0x%08x"), hr);
    }

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

HRESULT DirectShowGraph::RepaintVideo()
{
    HRESULT hr = S_OK;

    if (m_pVideoRenderer)
    {
        hr = m_pVideoRenderer->RepaintVideo();
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

HRESULT DirectShowGraph::Core_Render()
{
    player_log(kLogLevelTrace, _T("Core_Render==>"));

    HRESULT hr = E_FAIL;
    CodecsListEx fl;
    if (FAILED(hr = Core_EnumSourceFilters(fl)))
    {
        return hr;
    }

    if (m_bAborted)
    {
        player_log(kLogLevelTrace, _T("Core_Render, open aborted before EnumSourceFilters"));
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
                player_log(kLogLevelTrace, _T("Core_Render, open aborted before Core_RenderFilter"));
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
    // OnPreAddFilter
    {
        //         CodecsInfo* pInfo = PlayerCore::GetPlayerCodecs().FindCodecsInfo(sClsid_VsFilter, kCodecsTypeVideoEffect);
        //         if (pInfo)
        //         {
        //             CInterfaceList<IUnknown, &IID_IUnknown> pUnks;
        //             HRESULT hrVsfilter = Core_AddFilter(pInfo, &m_pVSFilter, pUnks);
        //             if (FAILED(hrVsfilter))
        //             {
        //                 player_log(kLogLevelTrace, _T("PreAdd VsFilter FAIL"));
        //             }
        //         }

        if (!m_pAudioSwitcher)
        {
            CodecsInfo* pInfo = PlayerCore::GetPlayerCodecs().FindCodecsInfo(sClsid_AudioSwitcher, kCodecsTypeAudioEffect);
            if (pInfo)
            {
                CInterfaceList<IUnknown, &IID_IUnknown> pUnks;
                HRESULT hrAudioSwitcher = Core_AddFilter(pInfo, &m_pAudioSwitcher, pUnks);
                if (FAILED(hrAudioSwitcher))
                {
                    player_log(kLogLevelTrace, _T("PreAdd AudioSwitcher FAIL"));
                }
            }
        }
    }

    return S_OK;
}

HRESULT DirectShowGraph::Core_OnRenderComplete()
{
    if (m_pVideoRenderer)
    {
        VideoRenderMode mode = m_pVideoRenderer->GetVideoRenderMode();
        if (mode == kVideoRenderEVRCP ||
            mode == kVideoRenderVMR9Renderless ||
            mode == kVideoRenderVMR7Renderless)
        {

        }
    }


    player_log(kLogLevelTrace, _T("DirectShowGraph::Core_OnRenderComplete, Used filter list:"));

    HRESULT hr = S_OK;
    CLSID clsid;
    int nCount = 0;

    //CLSID clsidVideoRenderer;

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

    DWORD substm = SetupSubtitleStreams();
    player_log(kLogLevelTrace, _T("DirectShowGraph::Core_OnRenderComplete, SetupSubtitleStreams = %d"), substm);
    if (substm) {
        SetSubtitle(substm - 1);
    }

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
        player_log(kLogLevelTrace, _T("CheckBytes, '%s' matched, subtype = %s"), 
            info->name, item.subtype);
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

    LookupFiltersInCodecs(NULL, fl);

//     CodecsInfoList& codecs = PlayerCore::GetPlayerCodecs().GetCodecsInfoList();
// 
//     CodecsInfo* info = NULL;
//     POSITION pos = codecs.GetHeadPosition();
//     bool bCheckbytes = false;
//     while (pos)
//     {
//         int group = 0;
//         int exactmatch = 0;
//         int custom_priority = 0;
// 
//         info = codecs.GetNext(pos);
// 
//         if (!info)
//             continue;
//         if (!info->enable)
//             continue;
//         if (info->type != kCodecsTypeSourceFilter)
//             continue;
// 
//         // check protocol & checkbytes
//         if (!Core_CheckProtocol(info))
//             continue;
// 
//         if (!Core_CheckBytes(info))
//         {
//             if (Core_CheckExtension(info))
//                 group = 1;
//             else
//                 continue;
//         }
// 
//         if (!info->checkbytes.IsEmpty())
//             exactmatch = 1;
// 
//         if (Core_CheckExtension(info))
//             custom_priority = 1;
// 
//         bool bInsert = fl.Insert(info, group, exactmatch, custom_priority);
//         player_log(kLogLevelTrace, _T("%sCodecsList::Insert [%d] [%d] [%d] [%d] '%s', result = %d"),
//             m_strPrefix, group, exactmatch, custom_priority, info->priority, 
//             info->name.IsEmpty() ? info->clsid : info->name, bInsert);
//     }

    // special format parse
//     if (!bCheckbytes)
//     {
//         CodecsInfo* info = NULL;
//         ///CString subtype;
//         if (Core_OnSpecialParse(&info))
//         {
//             fl.Insert(info, 0, 1);
//         }
//     }

    return S_OK;
}

BOOL DirectShowGraph::Core_CanAddFilter(IPin* pPinOut, CodecsInfo* info)
{
    GUID clsid = GUIDFromCString(info->clsid);
    for (CComPtr<IBaseFilter> pBFUS = GetFilterFromPin(pPinOut); pBFUS; pBFUS = GetUpStreamFilter(pBFUS))
    {
        CLSID clsidUpper = GetCLSID(pBFUS);
        if (clsidUpper == clsid)
        {
            player_log(kLogLevelTrace, _T("Core_CanAddFilter, can NOT add filter '%s'"), info->name);
            return FALSE;
        }
        // TODO : it's so ugly
        //         else
        //         {
        //             const PlayerCodecs& codecs = PlayerCore::GetPlayerCodecs();
        //             CodecsInfo* pInfoUpper = codecs.FindCodecsInfo(CStringFromGUID(clsidUpper), kCodecsTypeUnknown);
        //             if (pInfoUpper)
        //             {
        //                 if (pInfoUpper->type == kCodecsTypeVideoDecoder &&
        //                     info->type == kCodecsTypeVideoDecoder)
        //                 {
        //                     return FALSE;
        //                 }
        //                 if (pInfoUpper->type == kCodecsTypeAudioDecoder &&
        //                     info->type == kCodecsTypeAudioDecoder)
        //                 {
        //                     return FALSE;
        //                 }
        //             }
        //         }
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
        player_log(kLogLevelError, _T("Create Filter %s FAILED"), info->name);
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

    //player_log(kLogLevelTrace, _T("Core_AddFilter, %s added succeeded"), pName);
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
        player_log(kLogLevelTrace, _T("Try IFileSource::Load as MEDIATYPE_Stream, %s"), pSubtype);

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
        player_log(kLogLevelTrace, _T("Try IFileSource::Load as MEDIATYPE_Stream,MEDIASUBTYPE_NULL"));
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

    player_log(kLogLevelTrace, _T("AddFilter %s added succeeded"), info->name);

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

    player_log(kLogLevelTrace, _T("RenderPin => [%08x]"), pPinOut);


    bool fDeadEnd = true;

    // 1. Try filters in the graph
    {
        //LookupFiltersInGraph();

        player_log(kLogLevelTrace, _T("RenderPin, Trying filters in the graph"));

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

            if (SUCCEEDED(hr = Core_ConnectFilterDirect(pPinOut, pBF, NULL)))
            {
                player_log(kLogLevelTrace, _T("RenderPin, %08x, connect filter '%s'[%08x] OK"),
                    pPinOut, strFilterName, pBF);

                if (!IsStreamEnd(pBF))
                {
                    fDeadEnd = false;
                }

                if (fDeadEnd)
                {
                    player_log(kLogLevelTrace, _T("==> Reach to stream end '%s'[%08x]"), strFilterName, pBF);
                }
                else
                {
                    player_log(kLogLevelTrace, _T("==> Continue to render '%s'[%08x]"), strFilterName, pBF);
                }
                if (SUCCEEDED(hr = Core_RenderFilter(pBF)))
                {
                    return hr;
                }
                else
                {
                    player_log(kLogLevelTrace, _T("RenderFilter '%s'[%08x] failed, disconnect it."),
                        strFilterName, pBF);
                }
            }
//             else
//             {
//                 player_log(kLogLevelTrace, _T("Trying connect to '%s' FAIL"), strFilterName);
//             }

            EXECUTE_ASSERT(Disconnect(pPinOut));
        }
    }

    // 2. Look up filters in the <codecs>
    {
        player_log(kLogLevelTrace, _T("RenderPin, Trying filters in the codecs"));


        CodecsListEx fl;
        LookupFiltersInCodecs(pPinOut, fl);

        /*
        const PlayerSettings& s = PlayerCore::GetPlayerSettings();
        player_log(kLogLevelTrace, _T("Core_RenderPin, Trying filters in <codecs>"));

        CodecsListEx fl;
        CAtlArray<GUID> types;
        ExtractMediaTypes(pPinOut, types);

        player_log(kLogLevelTrace, _T("PinOut media types:"));
        for (int i = 0, len = types.GetCount() & ~1; i < len; i+=2)
        {
            player_log(kLogLevelTrace, _T("major = %s, sub = %s"), CStringFromGUID(types[i]), CStringFromGUID(types[i+1]));
        }

        CodecsInfoList& transforms = PlayerCore::GetPlayerCodecs().GetTransforms();
        POSITION pos = transforms.GetHeadPosition();
        while (pos)
        {
            int custom_priority = 0;
            CodecsInfo* info = transforms.GetNext(pos);
            if (info->enable && info->CheckTypes(types, false))
            {
                //                 if (info->clsid.CompareNoCase(sClsid_VsFilter) == 0 &&
                //                     !s.m_fVsfilterEnabled)
                //                 {
                //                     continue;
                //                 }
                //                 if (info->clsid.CompareNoCase(sClsid_AudioSwitcher) == 0 &&
                //                     !s.m_fAudioSwitcherEnabled)
                //                 {
                //                     continue;
                //                 }

                if (Core_CheckExtension(info))
                {
                    custom_priority = 1;
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
        //*/

        POSITION pos = fl.GetHeadPosition();
        while (pos)
        {
            CodecsInfo* info = fl.GetNext(pos);
            //player_log(kLogLevelTrace, _T("Connecting '%s'"), info->name);

            if (!Core_CanAddFilter(pPinOut, info))
            {
                continue;
            }

            CComPtr<IBaseFilter> pBF;
            CInterfaceList<IUnknown, &IID_IUnknown> pUnks;
            if (FAILED(hr = Core_AddFilter(info, &pBF, pUnks)))
            {
                //player_log(kLogLevelError, );
                pUnks.RemoveAll();
                pBF.Release();
                continue;
            }

            hr = Core_ConnectFilterDirect(pPinOut, pBF, NULL);
            if (SUCCEEDED(hr))
            {
                player_log(kLogLevelTrace, _T("ConnectFilterDirect '%s' => '%s' OK"), 
                    GetFilterName(GetFilterFromPin(pPinOut)), info->name);

                if (!IsStreamEnd(pBF))
                {
                    fDeadEnd = false;
                }

                if (fDeadEnd)
                {
                    player_log(kLogLevelTrace, _T("==> Reach to stream end '%s'"),
                        info->name);
                }
                else
                {
                    player_log(kLogLevelTrace, _T("==> Continue to render '%s'"),
                        info->name);
                }

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
                player_log(kLogLevelTrace, _T("ConnectFilterDirect to '%s' FAIL"),
                    info->name);
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

    CString strFilterName = GetFilterName(pBF);
    player_log(kLogLevelTrace, _T("RenderFilter => %s[%08x]"), strFilterName, pBF);

    BeginEnumPins(pBF, pEP, pPin)
    {
        BOOL bOutput = (S_OK == IsPinDirection(pPin, PINDIR_OUTPUT));
        BOOL bConnected = (S_OK == IsPinConnected(pPin));
        if (bOutput && !bConnected)
        {
            player_log(kLogLevelTrace, _T("RenderFilter, %s[%08x], render pin %d [%08x]"),
                strFilterName, pBF, nTotal, pPin);
            HRESULT hr = Core_RenderPin(pPin);
            if (SUCCEEDED(hr))
            {
                player_log(kLogLevelTrace, _T("RenderFilter, %s[%08x], render pin %d [%08x] OK"),
                    strFilterName, pBF, nTotal, pPin);
                nRendered++;
            }
        }

        if (bOutput)
            ++nTotal;
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

    player_log(kLogLevelTrace, _T("RenderFilter <= %s[%08x], %d / %d pins rendered, result=%08x"),
        strFilterName, pBF, nRendered, nTotal, hr);

    return hr;
}

HRESULT DirectShowGraph::LookupFiltersInCodecs(IPin* pPinUpper, CodecsListEx& filters)
{
    filters.RemoveAll();

    bool bEnumSource = false;
    CAtlArray<GUID> types;
    if (pPinUpper)
    {
        ExtractMediaTypes(pPinUpper, types);

        player_log(kLogLevelTrace, _T("PinOut[%p] media types:"), pPinUpper);
        for (int i = 0, len = types.GetCount() & ~1; i < len; i+=2)
        {
            player_log(kLogLevelTrace, _T("major=%s, sub=%s"), CStringFromGUID(types[i]), CStringFromGUID(types[i+1]));
        }
    }
    else
    {
        bEnumSource = true;
    }
    
    const PlayerSettings& settings = PlayerCore::GetPlayerSettings();

    const CodecsInfoList& codecs = PlayerCore::GetPlayerCodecs().GetCodecsInfoList();
    POSITION pos = codecs.GetHeadPosition();
    while (pos)
    {
        CodecsInfo* pInfo = codecs.GetNext(pos);

        if (!pInfo)
            continue;
        if (!pInfo->enable)
            continue;

        if (bEnumSource && pInfo->type != kCodecsTypeSourceFilter)
            continue;

        if (!settings.m_fVsfilterEnabled &&
            pInfo->clsid.CompareNoCase(sClsid_VsFilter) == 0)
            continue;
        if (!settings.m_fAudioSwitcherEnabled &&
            pInfo->clsid.CompareNoCase(sClsid_AudioSwitcher) == 0)
            continue;

        int group = 0;
        int exactmatch = 0;
        int custom_priority = 0;

        if (bEnumSource)
        {
            if (!Core_CheckProtocol(pInfo))
                continue;

            if (!Core_CheckBytes(pInfo))
                continue;

            if (!pInfo->checkbytes.IsEmpty())
                exactmatch = 1;
        }
        else
        {
            if (!pInfo->CheckTypes(types, false))
                continue;

            exactmatch = pInfo->CheckTypes(types, true);
        }

        if (Core_CheckExtension(pInfo))
        {
            custom_priority = kCustomPriorityExtension;
        }

        if (pInfo->type == kCodecsTypeVideoRenderer ||
            pInfo->type == kCodecsTypeAudioRenderer)
        {
            custom_priority = kCustomPriorityStreamEnd;
        }

        if (pInfo->type == kCodecsTypeVideoRenderer &&
            pInfo->clsid.CompareNoCase(settings.m_SelVideoRendererClsid) == 0)
        {
            custom_priority = kCustomPriorityUserPreferred;
        }

        bool bInsert = filters.Insert(pInfo, group, exactmatch, custom_priority);
        player_log(kLogLevelTrace, _T("CodecsList::Insert [%d] [%d] [%d] [%d] '%s', result = %d"),
            group, exactmatch, custom_priority, pInfo->priority, 
            pInfo->name.IsEmpty() ? pInfo->clsid : pInfo->name, bInsert);
    }

    player_log(kLogLevelTrace, _T("Sorting filters:"));
    pos = filters.GetHeadPosition();
    int i = 0;
    while (pos)
    {
        CodecsInfo* pInfo = filters.GetNext(pos);
        if (pInfo)
            player_log(kLogLevelTrace, _T("Filter[%d] - %s, %s, %d"),
            i++, pInfo->name, pInfo->clsid, pInfo->priority);
    }

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


// HRESULT DirectShowGraph::Connect(IPin* pPinOut, IPin* pPinIn)
// {
//     return Connect(pPinOut, pPinIn, true);
// }

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

    // load preload dll
    POSITION pos = pInfo->preloads.GetHeadPosition();
    while (pos)
    {
        CString dllpath(PlayerCore::GetPlayerCodecs().GetCodecsPath());
        ::PathAppend(dllpath.GetBuffer(MAX_PATH), pInfo->preloads.GetNext(pos));
        dllpath.ReleaseBuffer();

        CString dllname = PathFindFileName(dllpath);

        HMODULE hPreload = ::GetModuleHandle(dllname);
        if (hPreload == NULL)
        {
            hPreload = LoadLibrary(dllpath);
            if (hPreload == NULL)
            {
                player_log(kLogLevelError, _T("OnCreateFilterPrepare, Preload module %s failed, LastError=%d"),
                    dllpath, ::GetLastError());
            }
        }
        else
        {
            player_log(kLogLevelTrace, _T("OnCreateFilterPrepare, Preload module %s already loaded"), dllpath);
        }

    }

    if (clsid == __uuidof(PlayerAsyncReader))
    {
        *pParam = (void*)m_pPlayer->GetStream();
    }
    else if (clsid == CLSID_EVRAllocatorPresenter)
    {
        HWND hVideoWindow = m_pPlayer->GetVideoWindow();
        bool bFullScreen = false;
        HRESULT hr;
        m_pVideoRenderer = new VideoRendererEVRCP(hr, hVideoWindow, bFullScreen);

        if (SUCCEEDED(hr))
        {
            *pParam = m_pVideoRenderer;
        }
    }
    else if (clsid == CLSID_EnhancedVideoRenderer)
    {
        HRESULT hr;
        m_pVideoRenderer = new VideoRendererEVR(hr);
        if (SUCCEEDED(hr))
        {
            *pParam = m_pVideoRenderer;
        }
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

    if (CComQIPtr<ISubPicAllocatorPresenter> pCAP = pBF)
    {
        m_pCAP = pCAP;
    }

    return hr;
}

HRESULT DirectShowGraph::OnRenderFilterEnd(CodecsInfo* pInfo, IBaseFilter* pBF)
{
    HRESULT hr = S_OK;

    if (pInfo->type == kCodecsTypeVideoRenderer)
    {
        GUID clsid = GUIDFromCString(pInfo->clsid);

        //SAFE_DELETE(m_pVideoRenderer);

        //         if (clsid == CLSID_EnhancedVideoRenderer)
        //         {
        //             m_pVideoRenderer = new VideoRendererEVR(hr, pBF);
        //         }
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

void DirectShowGraph::AddTextPassThruFilter()
{
    BeginEnumFilters(m_pIGraphBuilder, pEF, pBF) {
        if (!IsSplitter(pBF)) {
            continue;
        }

        BeginEnumPins(pBF, pEP, pPin) {
            CComPtr<IPin> pPinTo;
            AM_MEDIA_TYPE mt;
            if (FAILED(pPin->ConnectedTo(&pPinTo)) || !pPinTo
                || FAILED(pPin->ConnectionMediaType(&mt))
                || mt.majortype != MEDIATYPE_Text && mt.majortype != MEDIATYPE_Subtitle) {
                    continue;
            }

            InsertTextPassThruFilter(pBF, pPin, pPinTo);
        }
        EndEnumPins;
    }
    EndEnumFilters;
}

HRESULT DirectShowGraph::InsertTextPassThruFilter(IBaseFilter* pBF, IPin* pPin, IPin* pPinTo)
{
    HRESULT hr;
    CComQIPtr<IBaseFilter> pTPTF = new CTextPassThruFilter(this);
    CStringW name;
    name.Format(L"TextPassThru%p", pTPTF);
    if (FAILED(hr = m_pIGraphBuilder->AddFilter(pTPTF, name))) {
        return hr;
    }

    hr = pPinTo->Disconnect();
    hr = pPin->Disconnect();

    if (FAILED(hr = m_pIGraphBuilder->ConnectDirect(pPin, GetFirstPin(pTPTF, PINDIR_INPUT), NULL))
        || FAILED(hr = m_pIGraphBuilder->ConnectDirect(GetFirstPin(pTPTF, PINDIR_OUTPUT), pPinTo, NULL)))
    {
        hr = m_pIGraphBuilder->ConnectDirect(pPin, pPinTo, NULL);
    }
    else 
    {
        SubtitleInput subInput(CComQIPtr<ISubStream>(pTPTF), pBF);
        m_pSubStreams.AddTail(subInput);
    }
    return hr;
}

void DirectShowGraph::ReplaceSubtitle(const ISubStream* pSubStreamOld, ISubStream* pSubStreamNew)
{
    POSITION pos = m_pSubStreams.GetHeadPosition();
    while (pos) {
        POSITION cur = pos;
        if (pSubStreamOld == m_pSubStreams.GetNext(pos).subStream) {
            m_pSubStreams.GetAt(cur).subStream = pSubStreamNew;
            if (m_pCurrentSubStream == pSubStreamOld) {
                SetSubtitle(pSubStreamNew);
            }
            break;
        }
    }
}

bool DirectShowGraph::SetSubtitle(int i, bool bIsOffset /*= false*/, bool bDisplayMessage /*= false*/, bool bApplyDefStyle /*= false*/)
{
    if (!m_pCAP) {
        return false;
    }

    SubtitleInput* pSubInput = GetSubtitleInput(i, bIsOffset);
    bool success = false;

    if (pSubInput) {
        WCHAR* pName = NULL;
        if (CComQIPtr<IAMStreamSelect> pSSF = pSubInput->sourceFilter) {
            DWORD dwFlags;
            if (FAILED(pSSF->Info(i, NULL, &dwFlags, NULL, NULL, &pName, NULL, NULL))) {
                dwFlags = 0;
                pName = NULL;
            }
            // Enable the track only if it isn't already the only selected track in the group
            if (!(dwFlags & AMSTREAMSELECTINFO_EXCLUSIVE)) {
                pSSF->Enable(i, AMSTREAMSELECTENABLE_ENABLE);
            }
            i = 0;
        }
        {
            // m_csSubLock shouldn't be locked when using IAMStreamSelect::Enable
            CAutoLock cAutoLock(&m_csSubLock);
            pSubInput->subStream->SetStream(i);
            SetSubtitle(pSubInput->subStream, bApplyDefStyle);
        }

        if (bDisplayMessage) {
            if (pName || SUCCEEDED(pSubInput->subStream->GetStreamInfo(0, &pName, NULL))) {
                //                 CString strMessage;
                //                 strMessage.Format(IDS_SUBTITLE_STREAM, pName);
                //                 m_OSD.DisplayMessage(OSD_TOPLEFT, strMessage);
            }
        }
        if (pName) {
            CoTaskMemFree(pName);
        }

        success = true;
    }

    return success;
}

void DirectShowGraph::SetSubtitle(ISubStream* pSubStream, bool bApplyDefStyle /*= false*/)
{
    PlayerSettings& s = PlayerCore::GetPlayerSettings();

    if (pSubStream)
    {
        bool found = false;
        POSITION pos = m_pSubStreams.GetHeadPosition();
        while (pos)
        {
            if (pSubStream == m_pSubStreams.GetNext(pos).subStream)
            {
                found = true;
                break;
            }
        }
        // We are trying to set a subtitles stream that isn't in the list so we abort here.
        if (!found)
        {
            return;
        }

        CLSID clsid;
        pSubStream->GetClassID(&clsid);

        SetSubStreamStyle(clsid, pSubStream, bApplyDefStyle);
    }

    m_pCurrentSubStream = pSubStream;

    if (m_pCAP && s.m_fEnableSubtitles) {
        m_pCAP->SetSubPicProvider(CComQIPtr<ISubPicProvider>(pSubStream));
        //m_wndSubresyncBar.SetSubtitle(pSubStream, m_pCAP->GetFPS());
    }
}

// Returns the the corresponding subInput or NULL in case of error.
// i is modified to reflect the locale index of track
SubtitleInput* DirectShowGraph::GetSubtitleInput(int& i, bool bIsOffset /*= false*/)
{
    // Only 1, 0 and -1 are supported offsets
    if ((bIsOffset && (i < -1 || i > 1)) || (!bIsOffset && i < 0)) {
        return NULL;
    }

    POSITION pos = m_pSubStreams.GetHeadPosition();
    SubtitleInput* pSubInput = NULL, *pSubInputPrec = NULL;
    int iLocalIdx = -1, iLocalIdxPrec = -1;
    bool bNextTrack = false;

    while (pos && !pSubInput) {
        SubtitleInput& subInput = m_pSubStreams.GetNext(pos);

        if (CComQIPtr<IAMStreamSelect> pSSF = subInput.sourceFilter) {
            DWORD cStreams;
            if (FAILED(pSSF->Count(&cStreams))) {
                continue;
            }

            for (int j = 0, cnt = (int)cStreams; j < cnt; j++) {
                DWORD dwFlags, dwGroup;

                if (FAILED(pSSF->Info(j, NULL, &dwFlags, NULL, &dwGroup, NULL, NULL, NULL))) {
                    continue;
                }

                if (dwGroup != 2) {
                    continue;
                }

                if (bIsOffset) {
                    if (bNextTrack) { // We detected previously that the next subtitles track is the one we want to select
                        pSubInput = &subInput;
                        iLocalIdx = j;
                        break;
                    } else if (subInput.subStream == m_pCurrentSubStream
                        && dwFlags & (AMSTREAMSELECTINFO_ENABLED | AMSTREAMSELECTINFO_EXCLUSIVE)) {
                            if (i == 0) {
                                pSubInput = &subInput;
                                iLocalIdx = j;
                                break;
                            } else if (i > 0) {
                                bNextTrack = true; // We want to the select the next subtitles track
                            } else {
                                // We want the previous subtitles track and we know which one it is
                                if (pSubInputPrec) {
                                    pSubInput = pSubInputPrec;
                                    iLocalIdx = iLocalIdxPrec;
                                    break;
                                }
                            }
                    }

                    pSubInputPrec = &subInput;
                    iLocalIdxPrec = j;
                } else {
                    if (i == 0) {
                        pSubInput = &subInput;
                        iLocalIdx = j;
                        break;
                    }

                    i--;
                }
            }
        } else {
            if (bIsOffset) {
                if (bNextTrack) { // We detected previously that the next subtitles track is the one we want to select
                    pSubInput = &subInput;
                    iLocalIdx = 0;
                    break;
                } else if (subInput.subStream == m_pCurrentSubStream) {
                    iLocalIdx = subInput.subStream->GetStream() + i;
                    if (iLocalIdx >= 0 && iLocalIdx < subInput.subStream->GetStreamCount()) {
                        // The subtitles track we want to select is part of this substream
                        pSubInput = &subInput;
                    } else if (i > 0) { // We want to the select the next subtitles track
                        bNextTrack = true;
                    } else {
                        // We want the previous subtitles track and we know which one it is
                        if (pSubInputPrec) {
                            pSubInput = pSubInputPrec;
                            iLocalIdx = iLocalIdxPrec;
                        }
                    }
                } else {
                    pSubInputPrec = &subInput;
                    iLocalIdxPrec = subInput.subStream->GetStreamCount() - 1;
                }
            } else {
                if (i < subInput.subStream->GetStreamCount()) {
                    pSubInput = &subInput;
                    iLocalIdx = i;
                } else {
                    i -= subInput.subStream->GetStreamCount();
                }
            }
        }

        // Handle special cases
        if (!pos && !pSubInput && bIsOffset) {
            if (bNextTrack) { // The last subtitles track was selected and we want the next one
                // Let's restart the loop to select the first subtitles track
                pos = m_pSubStreams.GetHeadPosition();
            } else if (i < 0) { // The first subtitles track was selected and we want the previous one
                pSubInput = pSubInputPrec; // We select the last track
                iLocalIdx = iLocalIdxPrec;
            }
        }
    }

    i = iLocalIdx;

    return pSubInput;
}

void DirectShowGraph::InvalidateSubtitle(DWORD_PTR nSubtitleId, REFERENCE_TIME rtInvalidate)
{
    if (m_pCAP) {
        if (nSubtitleId == -1 || nSubtitleId == (DWORD_PTR)m_pCurrentSubStream) {
            m_pCAP->Invalidate(rtInvalidate);
        }
    }
}

DWORD DirectShowGraph::SetupSubtitleStreams()
{
    const PlayerSettings& s = PlayerCore::GetPlayerSettings();

    size_t cStreams = m_pSubStreams.GetCount();
    if (cStreams > 0)
    {
        bool externalPriority = false;
        CAtlArray<CString> langs;
        int tPos = 0;
        CString lang = s.strSubtitlesLanguageOrder.Tokenize(_T(",; "), tPos);
        while (tPos != -1)
        {
            langs.Add(lang.MakeLower());
            lang = s.strSubtitlesLanguageOrder.Tokenize(_T(",; "), tPos);
        }

        DWORD selected = 0;
        DWORD i = 0;
        int  maxrating = 0;
        POSITION pos = m_pSubStreams.GetHeadPosition();
        while (pos)
        {
            //             if (m_posFirstExtSub == pos) {
            //                 externalPriority = s.fPrioritizeExternalSubtitles;
            //             }
            SubtitleInput& subInput = m_pSubStreams.GetNext(pos);
            CComPtr<ISubStream> pSubStream = subInput.subStream;
            CComQIPtr<IAMStreamSelect> pSSF = subInput.sourceFilter;

            //             bool bAllowOverridingSplitterChoice = s.bAllowOverridingExternalSplitterChoice;
            //             CLSID clsid;
            //             if (!bAllowOverridingSplitterChoice && pSSF && SUCCEEDED(subInput.sourceFilter->GetClassID(&clsid))) {
            //                 // We always allow overriding the splitter choice for our splitters that
            //                 // support the IAMStreamSelect interface and thus would have been ignored.
            //                 bAllowOverridingSplitterChoice = !!(clsid == __uuidof(CMpegSplitterFilter));
            //             }

            int count = 0;
            if (pSSF)
            {
                DWORD cStreams;
                if (SUCCEEDED(pSSF->Count(&cStreams)))
                {
                    count = (int)cStreams;
                }
            }
            else
            {
                count = pSubStream->GetStreamCount();
            }

            for (int j = 0; j < count; j++)
            {
                WCHAR* pName;
                HRESULT hr;
                if (pSSF)
                {
                    DWORD dwFlags, dwGroup = 2;
                    hr = pSSF->Info(j, NULL, &dwFlags, NULL, &dwGroup, &pName, NULL, NULL);
                    if (dwGroup != 2) {
                        CoTaskMemFree(pName);
                        continue;
                    }
                    //                     else if (!bAllowOverridingSplitterChoice && !(dwFlags & (AMSTREAMSELECTINFO_ENABLED | AMSTREAMSELECTINFO_EXCLUSIVE)))
                    //                     {
                    //                         // If we aren't allowed to modify the splitter choice and the current
                    //                         // track isn't already selected at splitter level we need to skip it.
                    //                         CoTaskMemFree(pName);
                    //                         i++;
                    //                         continue;
                    //                     }
                }
                else
                {
                    hr = pSubStream->GetStreamInfo(j, &pName, NULL);
                }
                CString name(pName);
                CoTaskMemFree(pName);
                name.Trim();
                name.MakeLower();

                int rating = 0;
                for (size_t k = 0; k < langs.GetCount(); k++)
                {
                    int num = _tstoi(langs[k]) - 1;
                    if (num >= 0)
                    { // this is track number
                        if (i != num)
                        {
                            continue;  // not matched
                        }
                    }
                    else
                    { // this is lang string
                        int len = langs[k].GetLength();
                        if (name.Left(len) != langs[k] && name.Find(_T("[") + langs[k]) < 0)
                        {
                            continue; // not matched
                        }
                    }
                    rating = 16 * int(langs.GetCount() - k);
                    break;
                }
                if (externalPriority)
                {
                    rating += 8;
                }
                if (s.bPreferDefaultForcedSubtitles)
                {
                    if (name.Find(_T("[default,forced]")) != -1) // for LAV Splitter
                    {
                        rating += 4 + 2;
                    }
                    if (name.Find(_T("[forced]")) != -1)
                    {
                        rating += 4;
                    }
                    if (name.Find(_T("[default]")) != -1)
                    {
                        rating += 2;
                    }
                }

                if (rating > maxrating || !selected)
                {
                    maxrating = rating;
                    selected = i + 1;
                }
                i++;
            }
        }
        return selected;
    }

    return 0;
}

