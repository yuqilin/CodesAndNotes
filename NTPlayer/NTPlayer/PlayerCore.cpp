#include "stdafx.h"
#include "PlayerCore.h"
#include "RenderersSettings.h"
#include "FGManager.h"

#define sCLSID_AudioSwitcher		_T("{18C16B08-6497-420E-AD14-22D21C2CEAB7}")
#define sCLSID_VsFilter				_T("{93A22E7A-5091-45EF-BA61-6DA26156A5D0}")

// {18C16B08-6497-420E-AD14-22D21C2CEAB7}
DEFINE_GUID(CLSID_AudioSwitcher, 
            0x18C16B08, 0x6497, 0x420E, 0xAD, 0x14, 0x22, 0xD2, 0x1C, 0x2C, 0xEA, 0xB7);


enum {
    WM_GRAPHNOTIFY = WM_RESET_DEVICE + 1,
    WM_RESUMEFROMSTATE,
    WM_SAVESETTINGS,
    WM_TUNER_SCAN_PROGRESS,
    WM_TUNER_SCAN_END,
    WM_TUNER_STATS,
    WM_TUNER_NEW_CHANNEL
};


CPlayerCore::CPlayerCore()
{
    m_settings.LoadDefaultSettings();

    m_ZoomX = 1;
    m_ZoomY = 1;
    m_PosX = 0.5;
    m_PosY = 0.5;

    m_AngleX = m_AngleY = m_AngleZ = 0;
}

CPlayerCore::~CPlayerCore()
{

}

HRESULT CPlayerCore::Open(const char* url)
{
    HRESULT hr = E_FAIL;

    m_codecs.LoadCodecsInfo();

    OpenFileData* pOFD = new OpenFileData();
    if (pOFD == NULL)
    {
        return E_OUTOFMEMORY;
    }

    pOFD->strFileName = url;

    CAutoPtr<OpenMediaData> pOMD(pOFD);

    if (pOMD)
    {
        hr = OpenMedia(pOMD);
    }
    return hr;
}

HRESULT CPlayerCore::Close()
{
    HRESULT hr = S_OK;

    hr = CloseMedia();

    return hr;
}

HRESULT CPlayerCore::SetCodecsPath(LPCTSTR lpszCodecsPath)
{
    if (!PathIsDirectory(lpszCodecsPath) || !PathFileExists(lpszCodecsPath))
    {
        return E_INVALIDARG;
    }

    m_settings.m_strCodecsPath = lpszCodecsPath;

    return S_OK;
}

HRESULT CPlayerCore::OpenMedia(CAutoPtr<OpenMediaData> pOMD)
{
    HRESULT hr = S_OK;
    // shortcut
    if (OpenDeviceData* p = dynamic_cast<OpenDeviceData*>(pOMD.m_p))
    {
        // TODO: not implemented
        return E_NOTIMPL;
    }

    if (m_iMediaLoadState != MLS_CLOSED)
    {
        CloseMedia();
    }

    //m_iMediaLoadState = MLS_LOADING; // HACK: hides the logo

    bool fUseThread = m_pGraphThread && m_settings.fEnableWorkerThreadForOpening;

    if (OpenFileData* p = dynamic_cast<OpenFileData*>(pOMD.m_p))
    {
        if (p->strFileName.GetLength() > 0)
        {
            engine_t e = m_settings.m_Formats.GetEngine(p->strFileName);
            if (e != DirectShow /*&& e != RealMedia && e != QuickTime*/)
            {
                fUseThread = false;
            }
        }
    }
    else if (OpenDeviceData* p = dynamic_cast<OpenDeviceData*>(pOMD.m_p))
    {
        fUseThread = false;
    }

    // Create D3DFullscreen window if launched in fullscreen
    //     if (m_settings.IsD3DFullscreen() && m_fStartInD3DFullscreen)
    //     {
    //         m_fStartInD3DFullscreen = false;
    //     }

    if (fUseThread) 
    {
        m_pGraphThread->OpenMedia(pOMD);
        m_bOpenedThruThread = true;
    }
    else
    {
        hr = OpenMediaPrivate(pOMD);
        m_bOpenedThruThread = false;
    }

    return hr;
}

HRESULT CPlayerCore::CloseMedia()
{
    HRESULT hr = S_OK;

    if (m_iMediaLoadState == MLS_CLOSING)
    {
        TRACE(_T("WARNING: CMainFrame::CloseMedia() called twice or more\n"));
        return hr;
    }

    int nTimeWaited = 0;

    while (m_iMediaLoadState == MLS_LOADING)
    {
        m_fOpeningAborted = true;

        if (m_pGB)
        {
            m_pGB->Abort();    // TODO: lock on graph objects somehow, this is not thread safe
        }

        if (nTimeWaited > 5 * 1000 && m_pGraphThread)
        {
            //MessageBeep(MB_ICONEXCLAMATION);
            TRACE(_T("CRITICAL ERROR: !!! Must kill opener thread !!!"));
            //TerminateThread(m_pGraphThread->m_hThread, (DWORD) - 1);
            m_pGraphThread->Terminate();
            //m_pGraphThread = (CGraphThread*)AfxBeginThread(RUNTIME_CLASS(CGraphThread));
            m_bOpenedThruThread = false;
            break;
        }

        Sleep(50);

        nTimeWaited += 50;
    }

    m_fOpeningAborted = false;

    SetLoadState(MLS_CLOSING);

    //OnFilePostClosemedia();

    if (m_pGraphThread && m_bOpenedThruThread)
    {
        m_pGraphThread->CloseMedia();        
    }
    else
    {
        CloseMediaPrivate();
    }

    UnloadExternalObjects();

    return hr;
}

HRESULT CPlayerCore::OpenMediaPrivate(CAutoPtr<OpenMediaData> pOMD)
{
    HRESULT hr = S_OK;
    CPlayerSettings& s = m_settings;

    if (m_iMediaLoadState != MLS_CLOSED)
    {
        return E_FAIL;
    }

    OpenFileData* pFileData = dynamic_cast<OpenFileData*>(pOMD.m_p);
    OpenDVDData* pDVDData = dynamic_cast<OpenDVDData*>(pOMD.m_p);
    OpenDeviceData* pDeviceData = dynamic_cast<OpenDeviceData*>(pOMD.m_p);
    if (!pFileData && !pDVDData  && !pDeviceData)
    {
        return E_FAIL;
    }

    // Clear DXVA state ...
    //ClearDXVAState();

    SetLoadState(MLS_LOADING);

    CString err;

    CComPtr<IVMRMixerBitmap9>    pVMB;
    CComPtr<IMFVideoMixerBitmap> pMFVMB;
    if (m_fOpeningAborted)
    {
        return E_ABORT;
    }

    OpenCreateGraphObject(pOMD);

    if (m_fOpeningAborted)
    {
        return E_ABORT;
    }

    if (pFileData)
    {
        OpenFile(pFileData);
    }

    m_pCAP2 = NULL;
    m_pCAP = NULL;

    m_pGB->FindInterface(__uuidof(ISubPicAllocatorPresenter), (void**)&m_pCAP, TRUE);
    m_pGB->FindInterface(__uuidof(ISubPicAllocatorPresenter2), (void**)&m_pCAP2, TRUE);
    m_pGB->FindInterface(__uuidof(IVMRWindowlessControl9), (void**)&m_pVMRWC, FALSE); // might have IVMRMixerBitmap9, but not IVMRWindowlessControl9
    m_pGB->FindInterface(__uuidof(IVMRMixerControl9), (void**)&m_pVMRMC, TRUE);
    m_pGB->FindInterface(__uuidof(IVMRMixerBitmap9), (void**)&pVMB, TRUE);
    m_pGB->FindInterface(__uuidof(IMFVideoMixerBitmap), (void**)&pMFVMB, TRUE);
    //pMVTO = m_pCAP;

//     if (s.fShowOSD || s.fShowDebugInfo) // Force OSD on when the debug switch is used
//     {
//         if (pVMB)
//         {
//             m_OSD.Start(m_hVideoWindow, pVMB, IsD3DFullScreenMode());
//         }
//         else if (pMFVMB)
//         {
//             m_OSD.Start(m_hVideoWindow, pMFVMB, IsD3DFullScreenMode());
//         }
//     }

    SetupVMR9ColorControl();

    // === EVR !
    m_pGB->FindInterface(__uuidof(IMFVideoDisplayControl), (void**)&m_pMFVDC, TRUE);
    m_pGB->FindInterface(__uuidof(IMFVideoProcessor), (void**)&m_pMFVP, TRUE);
    if (m_pMFVDC)
    {
        m_pMFVDC->SetVideoWindow(m_hVideoWindow);
    }

    //SetupEVRColorControl();
    //does not work at this location
    //need to choose the correct mode (IMFVideoProcessor::SetVideoProcessorMode)

    //         BeginEnumFilters(m_pGB, pEF, pBF)
    //         {
    //             if (m_pLN21 = pBF)
    //             {
    //                 m_pLN21->SetServiceState(s.fClosedCaptions ? AM_L21_CCSTATE_On : AM_L21_CCSTATE_Off);
    //                 break;
    //             }
    //         }
    //         EndEnumFilters;

    if (m_fOpeningAborted)
    {
        return E_ABORT;
    }

    OpenCustomizeGraph();

    if (m_fOpeningAborted)
    {
        return E_ABORT;
    }

    OpenSetupVideo();

    if (m_fOpeningAborted)
    {
        return E_ABORT;
    }

    OpenSetupAudio();

    if (m_fOpeningAborted)
    {
        return E_ABORT;
    }

    if (m_pCAP && (!m_fAudioOnly || m_fRealMediaGraph))
    {
        if (s.fDisableInternalSubtitles)
        {
            m_pSubStreams.RemoveAll(); // Needs to be replaced with code that checks for forced subtitles.
        }

        m_posFirstExtSub = NULL;
        POSITION pos = pOMD->subs.GetHeadPosition();
        while (pos)
        {
            //LoadSubtitle(pOMD->subs.GetNext(pos), NULL, true);
        }
    }

    if (m_fOpeningAborted)
    {
        return E_ABORT;
    }

    //OpenSetupWindowTitle();

    ///*
    while (m_iMediaLoadState != MLS_LOADED
        && m_iMediaLoadState != MLS_CLOSING // FIXME
        ) 
    {
        Sleep(50);
    }
    //*/

    DWORD audstm = SetupAudioStreams();
    DWORD substm = SetupSubtitleStreams();

    if (audstm)
    {
        //OnPlayAudio(ID_AUDIO_SUBITEM_START + audstm);
    }
    if (substm)
    {
        //SetSubtitle(substm - 1);
    }

    // PostMessage instead of SendMessage because the user might call CloseMedia and then we would deadlock

    //PostMessage(WM_COMMAND, ID_PLAY_PAUSE);

    //m_bFirstPlay = true;

    //         if (!(s.nCLSwitches & CLSW_OPEN) && (s.nLoops > 0)) {
    //             PostMessage(WM_COMMAND, ID_PLAY_PLAY);
    //         } else {
    //             // If we don't start playing immediately, we need to initialize
    //             // the seekbar and the time counter.
    //             OnTimer(TIMER_STREAMPOSPOLLER);
    //             OnTimer(TIMER_STREAMPOSPOLLER2);
    //         }

    //s.nCLSwitches &= ~CLSW_OPEN;

    //         if (pFileData) {
    //             if (pFileData->rtStart > 0) {
    //                 PostMessage(WM_RESUMEFROMSTATE, (WPARAM)PM_FILE, (LPARAM)(pFileData->rtStart / 10000));  // REFERENCE_TIME doesn't fit in LPARAM under a 32bit env.
    //             }
    //         } else if (pDVDData) {
    //             if (pDVDData->pDvdState) {
    //                 PostMessage(WM_RESUMEFROMSTATE, (WPARAM)PM_DVD, (LPARAM)(CComPtr<IDvdState>(pDVDData->pDvdState).Detach()));    // must be released by the called message handler
    //             }
    //         } else if (pDeviceData) {
    //             m_wndCaptureBar.m_capdlg.SetVideoInput(pDeviceData->vinput);
    //             m_wndCaptureBar.m_capdlg.SetVideoChannel(pDeviceData->vchannel);
    //             m_wndCaptureBar.m_capdlg.SetAudioInput(pDeviceData->ainput);
    //         }



    //EndWaitCursor();

    //     if (!err.IsEmpty()) {
    //         CloseMediaPrivate();
    //         m_closingmsg = err;
    // 
    //         if (err != ResStr(IDS_AG_ABORTED)) {
    //             if (pFileData) {
    //                 m_wndPlaylistBar.SetCurValid(false);
    // 
    //                 if (m_wndPlaylistBar.IsAtEnd()) {
    //                     m_nLoops++;
    //                 }
    // 
    //                 if (s.fLoopForever || m_nLoops < s.nLoops) {
    //                     bool hasValidFile = false;
    // 
    //                     if (m_nLastSkipDirection == ID_NAVIGATE_SKIPBACK) {
    //                         hasValidFile = m_wndPlaylistBar.SetPrev();
    //                     } else {
    //                         hasValidFile = m_wndPlaylistBar.SetNext();
    //                     }
    // 
    //                     if (hasValidFile) {
    //                         OpenCurPlaylistItem();
    //                     }
    //                 } else if (m_wndPlaylistBar.GetCount() > 1) {
    //                     DoAfterPlaybackEvent();
    //                 }
    //             } else {
    //                 OnNavigateSkip(ID_NAVIGATE_SKIPFORWARD);
    //             }
    //         }
    //     }
    //     else
    //     {
    //         m_wndPlaylistBar.SetCurValid(true);
    // 
    //         // Apply command line audio shift
    //         if (s.rtShift != 0)
    //         {
    //             SetAudioDelay(s.rtShift);
    //             s.rtShift = 0;
    //         }
    //     }
    // 
    //     m_nLastSkipDirection = 0;
    // 
    //     if (s.AutoChangeFullscrRes.bEnabled && (m_fFullScreen || IsD3DFullScreenMode()))
    //     {
    //         AutoChangeMonitorMode();
    //     }
    //     if (m_fFullScreen && s.fRememberZoomLevel)
    //     {
    //         m_fFirstFSAfterLaunchOnFS = true;
    //     }
    // 
    //     m_LastOpenFile = pOMD->title;
    // 
    //     PostMessage(WM_KICKIDLE); // calls main thread to update things
    // 
    //     if (!m_bIsBDPlay)
    //     {
    //         m_MPLSPlaylist.RemoveAll();
    //         m_LastOpenBDPath = _T("");
    //     }
    //     m_bIsBDPlay = false;

    return hr;
}

HRESULT CPlayerCore::CloseMediaPrivate()
{
    SetLoadState(MLS_CLOSING); // why it before OnPlayStop()? // TODO: remake or add detailed comments
    OnPlayStop(); // SendMessage(WM_COMMAND, ID_PLAY_STOP);


    if (m_pMC)
    {
        m_pMC->Stop(); // needed for StreamBufferSource, because m_iMediaLoadState is always MLS_CLOSED // TODO: fix the opening for such media
    }
    SetPlaybackMode(PM_NONE);

    m_fLiveWM = false;
    m_fEndOfStream = false;
    m_rtDurationOverride = -1;
    m_kfs.clear();
    m_pCB.Release();

    {
        CAutoLock cAutoLock(&m_csSubLock);
        m_pSubStreams.RemoveAll()   ;
    }
    m_pSubClock.Release();

    //if (m_pVW) m_pVW->put_Visible(OAFALSE);
    //if (m_pVW) m_pVW->put_MessageDrain((OAHWND)NULL), m_pVW->put_Owner((OAHWND)NULL);

    // IMPORTANT: IVMRSurfaceAllocatorNotify/IVMRSurfaceAllocatorNotify9 has to be released before the VMR/VMR9, otherwise it will crash in Release()
    //m_OSD.Stop();
    m_pCAP2.Release();
    m_pCAP.Release();
    m_pVMRWC.Release();
    m_pVMRMC.Release();
    m_pMFVP.Release();
    m_pMFVDC.Release();
    //     m_pLN21.Release();
    //     m_pSyncClock.Release();

    m_pAMXBar.Release();
    m_pAMDF.Release();
    m_pAMVCCap.Release();
    m_pAMVCPrev.Release();
    m_pAMVSCCap.Release();
    m_pAMVSCPrev.Release();
    m_pAMASC.Release();
    m_pVidCap.Release();
    m_pAudCap.Release();
    m_pAMTuner.Release();
    m_pCGB.Release();

    m_pDVDC.Release();
    m_pDVDI.Release();
    m_pAMOP.Release();
    //m_pBI.Release();
    m_pQP.Release();
    m_pFS.Release();
    m_pMS.Release();
    m_pBA.Release();
    m_pBV.Release();
    m_pVW.Release();
    m_pME.Release();
    m_pMC.Release();
    m_pFSF.Release();

    if (m_pGB)
    {
        m_pGB->RemoveFromROT();
        m_pGB.Release();
    }

    //m_pProv.Release();

    m_fRealMediaGraph = m_fShockwaveGraph = m_fQuicktimeGraph = false;

    m_VidDispName.Empty();
    m_AudDispName.Empty();

    //m_closingmsg.LoadString(IDS_CONTROLS_CLOSED);

    //AfxGetAppSettings().nCLSwitches &= CLSW_OPEN | CLSW_PLAY | CLSW_AFTERPLAYBACK_MASK | CLSW_NOFOCUS;

    SetLoadState(MLS_CLOSED);

    return S_OK;
}

void CPlayerCore::OnPlayStop()
{
    if (m_iMediaLoadState == MLS_LOADED)
    {
        if (GetPlaybackMode() == PM_FILE)
        {
            LONGLONG pos = 0;
            m_pMS->SetPositions(&pos, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
            m_pMC->Stop();
        }
        else if (GetPlaybackMode() == PM_DVD)
        {
            m_pDVDC->SetOption(DVD_ResetOnStop, TRUE);
            m_pMC->Stop();
            m_pDVDC->SetOption(DVD_ResetOnStop, FALSE);
        }
        else if (GetPlaybackMode() == PM_CAPTURE)
        {
            m_pMC->Stop();
        }

        m_dSpeedRate = 1.0;

        //         if (m_fFrameSteppingActive) // FIXME
        //         {            
        //             m_fFrameSteppingActive = false;
        //             m_pBA->put_Volume(m_nVolumeBeforeFrameStepping);
        //         }
    }

    //m_nLoops = 0;

    if (m_hVideoWindow)
    {
        MoveVideoWindow();
    }

    //     if (!m_fEndOfStream)
    //     {
    //         CString strOSD = ResStr(ID_PLAY_STOP);
    //         int i = strOSD.Find(_T("\n"));
    //         if (i > 0)
    //         {
    //             strOSD.Delete(i, strOSD.GetLength() - i);
    //         }
    //         m_OSD.DisplayMessage(OSD_TOPLEFT, strOSD, 3000);
    //         m_Lcd.SetStatusMessage(ResStr(IDS_CONTROLS_STOPPED), 3000);
    //     }
    //     else
    //     {
    //         m_fEndOfStream = false;
    //     }
    //SetPlayState(PS_STOP);
}

void CPlayerCore::SetLoadState(MPC_LOADSTATE state)
{
    m_iMediaLoadState = state;
}

HRESULT CPlayerCore::OpenCreateGraphObject(OpenMediaData* pOMD)
{
    ASSERT(m_pGB == NULL);

    m_fCustomGraph = false;
    m_fRealMediaGraph = m_fShockwaveGraph = m_fQuicktimeGraph = false;

    const CPlayerSettings& s = m_settings;

    if (OpenFileData* p = dynamic_cast<OpenFileData*>(pOMD))
    {
        engine_t engine = s.m_Formats.GetEngine(p->strFileName);

        CStringA ct = "";//GetContentType(p->strFileName);

        if (ct == "video/x-ms-asf")
        {
            // TODO: put something here to make the windows media source filter load later
        }
        else if (ct == "audio/x-pn-realaudio"
            || ct == "audio/x-pn-realaudio-plugin"
            || ct == "audio/x-realaudio-secure"
            || ct == "video/vnd.rn-realvideo-secure"
            || ct == "application/vnd.rn-realmedia"
            || ct.Find("vnd.rn-") >= 0
            || ct.Find("realaudio") >= 0
            || ct.Find("realvideo") >= 0)
        {
            engine = RealMedia;
        }
        else if (ct == "application/x-shockwave-flash")
        {
            engine = ShockWave;
        }
        else if (ct == "video/quicktime"
            || ct == "application/x-quicktimeplayer")
        {
            engine = QuickTime;
        }

        HRESULT hr = E_FAIL;
        CComPtr<IUnknown> pUnk;

        if (engine == RealMedia)
        {
            // TODO : see why Real SDK crash here ...
            //if (!IsRealEngineCompatible(p->fns.GetHead()))
            //  throw ResStr(IDS_REALVIDEO_INCOMPATIBLE);

            //pUnk = (IUnknown*)(INonDelegatingUnknown*)new DSObjects::CRealMediaGraph(m_pVideoWnd->m_hWnd, hr);
            //             if (!pUnk)
            //             {
            //                 throw (UINT)IDS_AG_OUT_OF_MEMORY;
            //             }

            //             if (SUCCEEDED(hr))
            //             {
            //                 m_pGB = CComQIPtr<IGraphBuilder>(pUnk);
            //                 if (m_pGB)
            //                 {
            //                     m_fRealMediaGraph = true;
            //                 }
            //             }
        }
        else if (engine == ShockWave)
        {
            //pUnk = (IUnknown*)(INonDelegatingUnknown*)new DSObjects::CShockwaveGraph(m_pVideoWnd->m_hWnd, hr);
            //             if (!pUnk)
            //             {
            //                 throw (UINT)IDS_AG_OUT_OF_MEMORY;
            //             }

            //             if (SUCCEEDED(hr))
            //             {
            //                 m_pGB = CComQIPtr<IGraphBuilder>(pUnk);
            //             }
            //             if (FAILED(hr) || !m_pGB)
            //             {
            //                 throw (UINT)IDS_MAINFRM_77;
            //             }
            //m_fShockwaveGraph = true;
        }
        else if (engine == QuickTime)
        {
            //pUnk = (IUnknown*)(INonDelegatingUnknown*)new DSObjects::CQuicktimeGraph(m_pVideoWnd->m_hWnd, hr);
            //             if (!pUnk)
            //             {
            //                 throw (UINT)IDS_AG_OUT_OF_MEMORY;
            //             }

            if (SUCCEEDED(hr))
            {
                m_pGB = CComQIPtr<IGraphBuilder>(pUnk);
                if (m_pGB)
                {
                    m_fQuicktimeGraph = true;
                }
            }
        }

        m_fCustomGraph = m_fRealMediaGraph || m_fShockwaveGraph || m_fQuicktimeGraph;

        if (!m_fCustomGraph)
        {
            m_pGB = new CFGManagerPlayer(_T("CFGManagerPlayer"), NULL, m_hVideoWindow);
        }
    }
    else if (OpenDVDData* p = dynamic_cast<OpenDVDData*>(pOMD))
    {
        m_pGB = new CFGManagerDVD(_T("CFGManagerDVD"), NULL, m_hVideoWindow);
    }
    else if (OpenDeviceData* p = dynamic_cast<OpenDeviceData*>(pOMD))
    {
        //         if (s.iDefaultCaptureDevice == 1)
        //         {
        //             m_pGB = new CFGManagerBDA(_T("CFGManagerBDA"), NULL, m_hVideoWindow);
        //         }
        //         else
        //         {
        //             m_pGB = new CFGManagerCapture(_T("CFGManagerCapture"), NULL, m_hVideoWindow);
        //         }
    }

    if (!m_pGB)
    {
        //throw (UINT)IDS_MAINFRM_80;
        return E_FAIL;
    }

    //m_pGB->AddToROT();

    m_pMC = m_pGB;
    m_pME = m_pGB;
    m_pMS = m_pGB; // general
    m_pVW = m_pGB;
    m_pBV = m_pGB; // video
    m_pBA = m_pGB; // audio
    m_pFS = m_pGB;

    if (!(m_pMC && m_pME && m_pMS)
        || !(m_pVW && m_pBV)
        || !(m_pBA))
    {
        //throw (UINT)IDS_GRAPH_INTERFACES_ERROR;
        return E_FAIL;
    }

    if (FAILED(m_pME->SetNotifyWindow((OAHWND)m_hNotifyWindow, WM_GRAPHNOTIFY, 0)))
    {
        //throw (UINT)IDS_GRAPH_TARGET_WND_ERROR;
        return E_FAIL;
    }

    //m_pProv = (IUnknown*)new CKeyProvider();

    //     if (CComQIPtr<IObjectWithSite> pObjectWithSite = m_pGB)
    //     {
    //         pObjectWithSite->SetSite(m_pProv);
    //     }
    // 
    //     m_pCB = new CDSMChapterBag(NULL, NULL);

    return S_OK;
}

void CPlayerCore::OpenCustomizeGraph()
{
    // TODO: not implemented
}

void CPlayerCore::OpenSetupVideo()
{
    // TODO: not implemented

}

void CPlayerCore::OpenSetupAudio()
{
    // TODO: not implemented

}


DWORD CPlayerCore::SetupAudioStreams()
{
    return 0;
}

DWORD CPlayerCore::SetupSubtitleStreams()
{
    return 0;
}

HRESULT CPlayerCore::OpenFile(OpenFileData* pOFD)
{
    if (pOFD == NULL || pOFD->strFileName.IsEmpty())
    {
        return E_INVALIDARG;
    }

    CString fn = pOFD->strFileName;

    HRESULT hr = m_pGB->RenderFile(CStringW(fn), NULL);

    if (FAILED(hr))
    {
        if (m_settings.fReportFailedPins)
        {
            CComQIPtr<IGraphBuilderDeadEnd> pGBDE = m_pGB;
            if (pGBDE && pGBDE->GetCount())
            {
                //CMediaTypesDlg(pGBDE, GetModalParent()).DoModal();
            }
        }
    }
    return hr;


//     if (!(m_pAMOP = m_pGB))
//     {
//         BeginEnumFilters(m_pGB, pEF, pBF);
//         if (m_pAMOP = pBF)
//         {
//             break;
//         }
//         EndEnumFilters;
//     }

//     CComQIPtr<IKeyFrameInfo> pKFI;
//     BeginEnumFilters(m_pGB, pEF, pBF);
//     if (pKFI = pBF)
//     {
//         break;
//     }
//     EndEnumFilters;
// 
//     UINT nKFs = 0;
//     if (pKFI && S_OK == pKFI->GetKeyFrameCount(nKFs) && nKFs > 0)
//     {
//         UINT k = nKFs;
//         m_kfs.resize(k);
//         if (FAILED(pKFI->GetKeyFrames(&TIME_FORMAT_MEDIA_TIME, m_kfs.data(), k)) || k != nKFs)
//         {
//             m_kfs.clear();
//         }
//     }

    SetPlaybackMode(PM_FILE);
}

HRESULT CPlayerCore::Play()
{
    m_pMS->SetRate(m_dSpeedRate);
    m_pMC->Run();

    return S_OK;
}

HRESULT CPlayerCore::Pause()
{
    m_pMC->Pause();
    return S_OK;
}



void CPlayerCore::OnOpenResult(HRESULT hr)
{

}

void CPlayerCore::MoveVideoWindow(bool fShowStats)
{
    if ((m_iMediaLoadState == MLS_LOADED) && !m_fAudioOnly && IsWindowVisible(m_hVideoWindow))
    {
//        RECT wr;
        // fullscreen
//         if (IsD3DFullScreenMode())
//         {
//             m_pFullscreenWnd->GetClientRect(&wr);
//         }
        // windowed Mode
//         else if (!m_fFullScreen)
//         {
//             m_wndView.GetClientRect(&wr);
//         }
        // fullscreen on non-primary monitor
//         else
//         {
//             GetWindowRect(&wr);
//             RECT r;
//             m_wndView.GetWindowRect(&r);
//             wr.left   -= r.left;
//             wr.right  -= r.left;
//             wr.top    -= r.top;
//             wr.bottom -= r.top;
//         }

        RECT wr;
        GetClientRect(m_hVideoWindow, &wr);        

        double dWRWidth  = (double)(wr.right - wr.left);
        double dWRHeight = (double)(wr.bottom - wr.top);

        RECT vr = {0, 0, 0, 0};

        OAFilterState fs = GetMediaState();
        if ((fs == State_Paused) || (fs == State_Running) || ((fs == State_Stopped) 
            && (m_fShockwaveGraph || m_fQuicktimeGraph)))
        {
            SIZE arxy = GetVideoSize();
            double dARx = (double)(arxy.cx);
            double dARy = (double)(arxy.cy);

            //dvstype iDefaultVideoSize = static_cast<dvstype>(AfxGetAppSettings().iDefaultVideoSize);
            double dVRWidth, dVRHeight;
//             if (iDefaultVideoSize == DVS_HALF) {
//                 dVRWidth  = dARx * 0.5;
//                 dVRHeight = dARy * 0.5;
//             } else if (iDefaultVideoSize == DVS_NORMAL) {
//                 dVRWidth  = dARx;
//                 dVRHeight = dARy;
//             } else if (iDefaultVideoSize == DVS_DOUBLE) {
//                 dVRWidth  = dARx * 2.0;
//                 dVRHeight = dARy * 2.0;
//             } else {
//                 dVRWidth  = dWRWidth;
//                 dVRHeight = dWRHeight;
//             }
            dVRWidth = dWRWidth;
            dVRHeight = dWRHeight;

            if (!m_fShockwaveGraph) { // && !m_fQuicktimeGraph)
                double dCRWidth = dVRHeight * dARx / dARy;

//                 if (iDefaultVideoSize == DVS_FROMINSIDE) {
//                     if (dVRWidth < dCRWidth) {
//                         dVRHeight = dVRWidth * dARy / dARx;
//                     } else {
//                         dVRWidth = dCRWidth;
//                     }
//                 } else if (iDefaultVideoSize == DVS_FROMOUTSIDE) {
//                     if (dVRWidth > dCRWidth) {
//                         dVRHeight = dVRWidth * dARy / dARx;
//                     } else {
//                         dVRWidth = dCRWidth;
//                     }
//                 } else if ((iDefaultVideoSize == DVS_ZOOM1) || (iDefaultVideoSize == DVS_ZOOM2)) {
//                     double minw = dCRWidth;
//                     double maxw = dCRWidth;
// 
//                     if (dVRWidth < dCRWidth) {
//                         minw = dVRWidth;
//                     } else {
//                         maxw = dVRWidth;
//                     }
// 
//                     double scale = (iDefaultVideoSize == DVS_ZOOM1) ?
//                         1.0 / 3.0 :
//                     2.0 / 3.0;
//                     dVRWidth  = minw + (maxw - minw) * scale;
//                     dVRHeight = dVRWidth * dARy / dARx;
//                 }
            }

            double dScaledVRWidth = m_ZoomX * dVRWidth;
            double dScaledVRHeight = m_ZoomY * dVRHeight;
            // Rounding is required here, else the left-to-right and top-to-bottom sizes will get distorted through rounding twice each
            // Todo: clean this up using decent intrinsic rounding instead of floor(x+.5) and truncation cast to LONG on (y+.5)
            double dPPLeft = floor(m_PosX * (dWRWidth * 3.0 - dScaledVRWidth) - dWRWidth + 0.5);
            double dPPTop  = floor(m_PosY * (dWRHeight * 3.0 - dScaledVRHeight) - dWRHeight + 0.5);
            // left and top parts are allowed to be negative
            vr.left   = (LONG)(dPPLeft);
            vr.top    = (LONG)(dPPTop);
            // right and bottom parts are always at picture center or beyond, so never negative
            vr.right  = (LONG)(dScaledVRWidth + dPPLeft + 0.5);
            vr.bottom = (LONG)(dScaledVRHeight + dPPTop + 0.5);

            if (fShowStats) {
                CString info;
                info.Format(_T("Pos %.3f %.3f, Zoom %.3f %.3f, AR %.3f"), m_PosX, m_PosY, m_ZoomX, m_ZoomY, dScaledVRWidth / dScaledVRHeight);
                //SendStatusMessage(info, 3000);
            }
        }

        if (m_pCAP) {
            m_pCAP->SetPosition(wr, vr);
            Vector v(Vector::DegToRad(m_AngleX), Vector::DegToRad(m_AngleY), Vector::DegToRad(m_AngleZ));
            m_pCAP->SetVideoAngle(v);
        } else {
            HRESULT hr;
            hr = m_pBV->SetDefaultSourcePosition();
            hr = m_pBV->SetDestinationPosition(vr.left, vr.top, vr.right - vr.left, vr.bottom - vr.top);
            hr = m_pVW->SetWindowPosition(wr.left, wr.top, wr.right - wr.left, wr.bottom - wr.top);

            if (m_pMFVDC) {
                m_pMFVDC->SetVideoPosition(NULL, &wr);
            }
        }
        //m_OSD.SetSize(wr, vr);
    }
    else
    {
        //m_wndView.SetVideoRect();
    }

    //UpdateThumbarButton();
}

CSize CPlayerCore::GetVideoSize() const
{
    const CPlayerSettings& s = m_settings;

    bool fKeepAspectRatio = s.fKeepAspectRatio;
    bool fCompMonDeskARDiff = s.fCompMonDeskARDiff;

    CSize ret(0, 0);
    if (m_iMediaLoadState != MLS_LOADED || m_fAudioOnly)
    {
        return ret;
    }

    CSize wh(0, 0), arxy(0, 0);

    if (m_pCAP)
    {
        wh = m_pCAP->GetVideoSize(false);
        arxy = m_pCAP->GetVideoSize(fKeepAspectRatio);
    }
    else if (m_pMFVDC)
    {
        m_pMFVDC->GetNativeVideoSize(&wh, &arxy);   // TODO : check AR !!
    }
    else
    {
        m_pBV->GetVideoSize(&wh.cx, &wh.cy);

        long arx = 0, ary = 0;
        CComQIPtr<IBasicVideo2> pBV2 = m_pBV;
        // FIXME: It can hang here, for few seconds (CPU goes to 100%), after the window have been moving over to another screen,
        // due to GetPreferredAspectRatio, if it happens before CAudioSwitcherFilter::DeliverEndFlush, it seems.
        if (pBV2 && SUCCEEDED(pBV2->GetPreferredAspectRatio(&arx, &ary)) && arx > 0 && ary > 0)
        {
            arxy.SetSize(arx, ary);
        }
    }

    if (wh.cx <= 0 || wh.cy <= 0)
    {
        return ret;
    }

    // with the overlay mixer IBasicVideo2 won't tell the new AR when changed dynamically
    DVD_VideoAttributes VATR;
    if (GetPlaybackMode() == PM_DVD && SUCCEEDED(m_pDVDI->GetCurrentVideoAttributes(&VATR))) {
        arxy.SetSize(VATR.ulAspectX, VATR.ulAspectY);
    }

    const CSize& ar = s.sizeAspectRatio;
    if (ar.cx && ar.cy)
    {
        arxy = ar;
    }

    ret = (!fKeepAspectRatio || arxy.cx <= 0 || arxy.cy <= 0)
        ? wh
        : CSize(::MulDiv(wh.cy, arxy.cx, arxy.cy), wh.cy);

    if (fCompMonDeskARDiff)
    {
        if (HDC hDC = ::GetDC(0))
        {
            int _HORZSIZE = GetDeviceCaps(hDC, HORZSIZE);
            int _VERTSIZE = GetDeviceCaps(hDC, VERTSIZE);
            int _HORZRES = GetDeviceCaps(hDC, HORZRES);
            int _VERTRES = GetDeviceCaps(hDC, VERTRES);

            if (_HORZSIZE > 0 && _VERTSIZE > 0 && _HORZRES > 0 && _VERTRES > 0)
            {
                double a = 1.0 * _HORZSIZE / _VERTSIZE;
                double b = 1.0 * _HORZRES / _VERTRES;

                if (b < a)
                {
                    ret.cy = (DWORD)(1.0 * ret.cy * a / b);
                }
                else if (a < b)
                {
                    ret.cx = (DWORD)(1.0 * ret.cx * b / a);
                }
            }

            ::ReleaseDC(0, hDC);
        }
    }

    return ret;
}

OAFilterState CPlayerCore::GetMediaState() const
{
    OAFilterState ret = -1;
    if (m_iMediaLoadState == MLS_LOADED)
    {
        m_pMC->GetState(0, &ret);
    }
    return ret;
}

void CPlayerCore::SetPlaybackMode(int iNewStatus)
{
    m_iPlaybackMode = iNewStatus;
}

void CPlayerCore::SetupVMR9ColorControl()
{
    if (m_pVMRMC)
    {
        CPlayerSettings& s = m_settings;

        if (FAILED(m_pVMRMC->GetProcAmpControlRange(0, NULL/*pApp->GetVMR9ColorControl(ProcAmp_Brightness)*/))) {
            return;
        }
        if (FAILED(m_pVMRMC->GetProcAmpControlRange(0, NULL/*pApp->GetVMR9ColorControl(ProcAmp_Contrast)*/))) {
            return;
        }
        if (FAILED(m_pVMRMC->GetProcAmpControlRange(0, NULL/*pApp->GetVMR9ColorControl(ProcAmp_Hue)*/))) {
            return;
        }
        if (FAILED(m_pVMRMC->GetProcAmpControlRange(0, NULL/*pApp->GetVMR9ColorControl(ProcAmp_Saturation)*/))) {
            return;
        }

        SetColorControl(ProcAmp_All, s.iBrightness, s.iContrast, s.iHue, s.iSaturation);
    }
}

void CPlayerCore::SetColorControl(DWORD flags, int& brightness, int& contrast, int& hue, int& saturation)
{
    /*
    static VMR9ProcAmpControl  ClrControl;
    static DXVA2_ProcAmpValues ClrValues;

    COLORPROPERTY_RANGE* cr;
    if (flags & ProcAmp_Brightness) {
        cr = GetColorControl(ProcAmp_Brightness);
        brightness = min(max(brightness, cr->MinValue), cr->MaxValue);
    }
    if (flags & ProcAmp_Contrast) {
        cr = GetColorControl(ProcAmp_Contrast);
        contrast = min(max(contrast, cr->MinValue), cr->MaxValue);
    }
    if (flags & ProcAmp_Hue) {
        cr = GetColorControl(ProcAmp_Hue);
        hue = min(max(hue, cr->MinValue), cr->MaxValue);
    }
    if (flags & ProcAmp_Saturation) {
        cr = GetColorControl(ProcAmp_Saturation);
        saturation = min(max(saturation, cr->MinValue), cr->MaxValue);
    }

    if (m_pVMRMC) {
        ClrControl.dwSize     = sizeof(ClrControl);
        ClrControl.dwFlags    = flags;
        ClrControl.Brightness = (float)brightness;
        ClrControl.Contrast   = (float)(contrast + 100) / 100;
        ClrControl.Hue        = (float)hue;
        ClrControl.Saturation = (float)(saturation + 100) / 100;

        m_pVMRMC->SetProcAmpControl(0, &ClrControl);
    } else if (m_pMFVP) {
        ClrValues.Brightness = IntToFixed(brightness);
        ClrValues.Contrast   = IntToFixed(contrast + 100, 100);
        ClrValues.Hue        = IntToFixed(hue);
        ClrValues.Saturation = IntToFixed(saturation + 100, 100);

        m_pMFVP->SetProcAmpValues(flags, &ClrValues);
    }
    */
}
