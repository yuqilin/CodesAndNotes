#include "stdafx.h"
#include "PlayerCore.h"

CPlayerCore::CPlayerCore()
{
    m_settings.LoadDefaultSettings();
}

CPlayerCore::~CPlayerCore()
{

}

HRESULT CPlayerCore::Open(const char* url)
{
    HRESULT hr = E_FAIL;

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
    if (m_settings.IsD3DFullscreen() && m_fStartInD3DFullscreen)
    {
        m_fStartInD3DFullscreen = false;
    }

    if (fUseThread) 
    {
        //m_pGraphThread->PostGraphMessage(CGraphThread::GM_OPEN, (LPVOID)pOMD.Detach());
        m_pGraphThread->Open(pOMD);
        m_pGraphThread->CallWorker(CGraphThread::GM_OPEN);
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
            TerminateThread(m_pGraphThread->m_hThread, (DWORD) - 1);
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
//         CAMEvent e;
//         m_pGraphThread->PostThreadMessage(CGraphThread::TM_CLOSE, 0, (LPARAM)&e);
//         e.Wait(); // either opening or closing has to be blocked to prevent reentering them, closing is the better choice
        m_pGraphThread->PostGraphMessage(CGraphThread::GM_CLOSE);
        
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
        //CComPtr<IMadVRTextOsd>       pMVTO;
         if (m_fOpeningAborted)
         {
             //throw (UINT)IDS_AG_ABORTED;
             return E_ABORT;
         }

        OpenCreateGraphObject(pOMD);

         if (m_fOpeningAborted)
         {
             //throw (UINT)IDS_AG_ABORTED;
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

        if (s.fShowOSD || s.fShowDebugInfo) // Force OSD on when the debug switch is used
        {
            if (pVMB)
            {
                m_OSD.Start(m_hVideoWindow, pVMB, IsD3DFullScreenMode());
            }
            else if (pMFVMB)
            {
                m_OSD.Start(m_hVideoWindow, pMFVMB, IsD3DFullScreenMode());
            }
        }

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

        OpenSetupWindowTitle();

        /*
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

void CPlayerCore::CloseMediaPrivate()
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
        m_pSubStreams.RemoveAll();
    }
    m_pSubClock.Release();

    //if (m_pVW) m_pVW->put_Visible(OAFALSE);
    //if (m_pVW) m_pVW->put_MessageDrain((OAHWND)NULL), m_pVW->put_Owner((OAHWND)NULL);

    // IMPORTANT: IVMRSurfaceAllocatorNotify/IVMRSurfaceAllocatorNotify9 has to be released before the VMR/VMR9, otherwise it will crash in Release()
    m_OSD.Stop();
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
    m_pBI.Release();
    m_pQP.Release();
    m_pFS.Release();
    m_pMS.Release();
    m_pBA.Release();
    m_pBV.Release();
    m_pVW.Release();
    m_pME.Release();
    m_pMC.Release();
    m_pFSF.Release();

    if (m_pGB) {
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
}

void CPlayerCore::OpenCreateGraphObject(OpenMediaData* pOMD)
{
    ASSERT(m_pGB == NULL);

    m_fCustomGraph = false;
    m_fRealMediaGraph = m_fShockwaveGraph = m_fQuicktimeGraph = false;

    const CPlayerSettings& s = m_settings;

    if (OpenFileData* p = dynamic_cast<OpenFileData*>(pOMD))
    {
        engine_t engine = s.m_Formats.GetEngine(p->fns.GetHead());

        CStringA ct = GetContentType(p->fns.GetHead());

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
    if (m_iMediaLoadState != MLS_LOADED)
    {
        return 0;
    }

    CComQIPtr<IAMStreamSelect> pSS = FindFilter(__uuidof(CAudioSwitcherFilter), m_pGB);
//     if (!pSS)
//     {
//         pSS = FindFilter(CLSID_MorganStreamSwitcher, m_pGB);
//     }

    DWORD cStreams = 0;
    if (pSS && SUCCEEDED(pSS->Count(&cStreams)) && cStreams > 1)
    {
        const CPlayerSettings& s = m_settings;

        CAtlArray<CString> langs;
        int tPos = 0;
        CString lang = s.strAudiosLanguageOrder.Tokenize(_T(",; "), tPos);
        while (tPos != -1)
        {
            langs.Add(lang.MakeLower());
            lang = s.strAudiosLanguageOrder.Tokenize(_T(",; "), tPos);
        }

        DWORD selected = 1;
        int  maxrating = 0;
        for (DWORD i = 0; i < cStreams; i++)
        {
            DWORD dwFlags, dwGroup;
            WCHAR* pName = NULL;
            CComPtr<IUnknown> pObject;
            if (FAILED(pSS->Info(i, NULL, &dwFlags, NULL, &dwGroup, &pName, &pObject, NULL)))
            {
                continue;
            }
            CString name(pName);
            CoTaskMemFree(pName);

            // If the track is controlled by a splitter and isn't selected at splitter level
            if (dwGroup == 1)
            {
                bool bSkipTrack;

                // If the splitter is the internal LAV Splitter and no language preferences
                // have been set at splitter level, we can override its choice safely
                CComQIPtr<IBaseFilter> pBF = pObject;
                if (pBF && CFGFilterLAV::IsInternalInstance(pBF))
                {
                    bSkipTrack = false;
                    if (CComQIPtr<ILAVFSettings> pLAVFSettings = pBF)
                    {
                        LPWSTR langPrefs = NULL;
                        if (SUCCEEDED(pLAVFSettings->GetPreferredLanguages(&langPrefs)) && langPrefs && wcslen(langPrefs))
                        {
                            bSkipTrack = true;
                        }
                        CoTaskMemFree(langPrefs);
                    }
                }
                else
                {
                    bSkipTrack = !s.bAllowOverridingExternalSplitterChoice;
                }

                if (bSkipTrack)
                {
                    continue;
                }
            }

            name.Trim();
            name.MakeLower();

            int rating = 0;
            for (size_t j = 0; j < langs.GetCount(); j++)
            {
                int num = _tstoi(langs[j]) - 1;
                if (num >= 0)
                { // this is track number
                    if (i != num)
                    {
                        continue;  // not matched
                    }
                }
                else
                { // this is lang string
                    int len = langs[j].GetLength();
                    if (name.Left(len) != langs[j] && name.Find(_T("[") + langs[j]) < 0)
                    {
                        continue; // not matched
                    }
                }
                rating = 16 * int(langs.GetCount() - j);
                break;
            }

            if (name.Find(_T("[default,forced]")) != -1)
            { // for LAV Splitter
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
            if (i == 0)
            {
                rating += 1;
            }

            if (rating > maxrating)
            {
                maxrating = rating;
                selected = i;
            }
        }
        return selected + 1;
    }

    return 0;
}

DWORD CPlayerCore::SetupSubtitleStreams()
{
    const CPlayerSettings& s = m_settings;

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
            if (m_posFirstExtSub == pos)
            {
                externalPriority = s.fPrioritizeExternalSubtitles;
            }
            SubtitleInput& subInput = m_pSubStreams.GetNext(pos);
            CComPtr<ISubStream> pSubStream = subInput.subStream;
            CComQIPtr<IAMStreamSelect> pSSF = subInput.sourceFilter;

            bool bAllowOverridingSplitterChoice;
            // If the internal LAV Splitter has its own language preferences set, we choose not to override its choice
            if (pSSF && CFGFilterLAV::IsInternalInstance(subInput.sourceFilter))
            {
                bAllowOverridingSplitterChoice = true;
                if (CComQIPtr<ILAVFSettings> pLAVFSettings = subInput.sourceFilter)
                {
                    LPWSTR langPrefs = NULL;
                    if (SUCCEEDED(pLAVFSettings->GetPreferredSubtitleLanguages(&langPrefs)) && langPrefs && wcslen(langPrefs))
                    {
                        bAllowOverridingSplitterChoice = false;
                    }
                    CoTaskMemFree(langPrefs);
                }
            }
            else
            {
                bAllowOverridingSplitterChoice = s.bAllowOverridingExternalSplitterChoice;
            }

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
                    if (dwGroup != 2)
                    {
                        CoTaskMemFree(pName);
                        continue;
                    }
                    else if (!bAllowOverridingSplitterChoice && !(dwFlags & (AMSTREAMSELECTINFO_ENABLED | AMSTREAMSELECTINFO_EXCLUSIVE)))
                    {
                        // If we aren't allowed to modify the splitter choice and the current
                        // track isn't already selected at splitter level we need to skip it.
                        CoTaskMemFree(pName);
                        i++;
                        continue;
                    }
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
                { // External tracks are given a higher priority than language matches
                    rating += 16 * int(langs.GetCount() + 1);
                }
                if (s.bPreferDefaultForcedSubtitles)
                {
                    if (name.Find(_T("[default,forced]")) != -1)
                    { // for LAV Splitter
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

HRESULT CPlayerCore::OpenFile(OpenFileData* pOFD)
{
    if (pOFD->fns.IsEmpty())
    {
        //throw (UINT)IDS_MAINFRM_81;
        return E_FAIL;
    }

    CPlayerSettings& s = m_settings;

    bool bMainFile = true;

    POSITION pos = pOFD->fns.GetHeadPosition();
    while (pos)
    {
        CString fn = pOFD->fns.GetNext(pos);

        fn.Trim();
        if (fn.IsEmpty() && !bMainFile)
        {
            break;
        }

        HRESULT hr = m_pGB->RenderFile(CStringW(fn), NULL);

        if (bMainFile)
        {
            // Don't try to save file position if source isn't seekable
            REFERENCE_TIME rtDur = 0;
            m_pMS->GetDuration(&rtDur);

//             m_bRememberFilePos = s.fKeepHistory && s.fRememberFilePos && rtDur > 0;
// 
//             if (m_bRememberFilePos && !s.filePositions.AddEntry(fn))
//             {
//                 REFERENCE_TIME rtPos = s.filePositions.GetLatestEntry()->llPosition;
//                 if (m_pMS)
//                 {
//                     m_pMS->SetPositions(&rtPos, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
//                 }
//             }
        }
        QueryPerformanceCounter(&m_liLastSaveTime);

        if (FAILED(hr))
        {
            if (bMainFile)
            {
                if (s.fReportFailedPins)
                {
                    CComQIPtr<IGraphBuilderDeadEnd> pGBDE = m_pGB;
                    if (pGBDE && pGBDE->GetCount())
                    {
                        //CMediaTypesDlg(pGBDE, GetModalParent()).DoModal();
                    }
                }

                UINT err;

//                 switch (hr)
//                 {
//                     case E_ABORT:
//                     case RFS_E_ABORT:
//                         err = IDS_MAINFRM_82;
//                         break;
//                     case E_FAIL:
//                     case E_POINTER:
//                     default:
//                         err = IDS_MAINFRM_83;
//                         break;
//                     case E_INVALIDARG:
//                         err = IDS_MAINFRM_84;
//                         break;
//                     case E_OUTOFMEMORY:
//                         err = IDS_AG_OUT_OF_MEMORY;
//                         break;
//                     case VFW_E_CANNOT_CONNECT:
//                         err = IDS_MAINFRM_86;
//                         break;
//                     case VFW_E_CANNOT_LOAD_SOURCE_FILTER:
//                         err = IDS_MAINFRM_87;
//                         break;
//                     case VFW_E_CANNOT_RENDER:
//                         err = IDS_MAINFRM_88;
//                         break;
//                     case VFW_E_INVALID_FILE_FORMAT:
//                         err = IDS_MAINFRM_89;
//                         break;
//                     case VFW_E_NOT_FOUND:
//                         err = IDS_MAINFRM_90;
//                         break;
//                     case VFW_E_UNKNOWN_FILE_TYPE:
//                         err = IDS_MAINFRM_91;
//                         break;
//                     case VFW_E_UNSUPPORTED_STREAM:
//                         err = IDS_MAINFRM_92;
//                         break;
//                     case RFS_E_NO_FILES:
//                         err = IDS_RFS_NO_FILES;
//                         break;
//                     case RFS_E_COMPRESSED:
//                         err = IDS_RFS_COMPRESSED;
//                         break;
//                     case RFS_E_ENCRYPTED:
//                         err = IDS_RFS_ENCRYPTED;
//                         break;
//                     case RFS_E_MISSING_VOLS:
//                         err = IDS_RFS_MISSING_VOLS;
//                         break;
//                 }

                //throw err;
                return E_FAIL;
            }
        }

        // We don't keep track of piped inputs since that hardly makes any sense
        if (s.fKeepHistory && fn.Find(_T("pipe:")) != 0)
        {
            CRecentFileList* pMRU = bMainFile ? &s.MRU : &s.MRUDub;
            pMRU->ReadList();
            pMRU->Add(fn);
            pMRU->WriteList();
            SHAddToRecentDocs(SHARD_PATH, fn);
        }

        if (bMainFile)
        {
            pOFD->title = fn;
            if (!(m_pFSF = m_pGB))
            {
                BeginEnumFilters(m_pGB, pEF, pBF);
                if (m_pFSF = pBF)
                {
                    break;
                }
                EndEnumFilters;
            }
        }

        bMainFile = false;

        if (m_fCustomGraph)
        {
            break;
        }
    }

    if (s.fReportFailedPins)
    {
        CComQIPtr<IGraphBuilderDeadEnd> pGBDE = m_pGB;
        if (pGBDE && pGBDE->GetCount())
        {
            CMediaTypesDlg(pGBDE, GetModalParent()).DoModal();
        }
    }

    if (!(m_pAMOP = m_pGB))
    {
        BeginEnumFilters(m_pGB, pEF, pBF);
        if (m_pAMOP = pBF)
        {
            break;
        }
        EndEnumFilters;
    }

    if (FindFilter(CLSID_MPCShoutcastSource, m_pGB))
    {
        m_fUpdateInfoBar = true;
    }

    SetupChapters();

    CComQIPtr<IKeyFrameInfo> pKFI;
    BeginEnumFilters(m_pGB, pEF, pBF);
    if (pKFI = pBF)
    {
        break;
    }
    EndEnumFilters;

    UINT nKFs = 0;
    if (pKFI && S_OK == pKFI->GetKeyFrameCount(nKFs) && nKFs > 0)
    {
        UINT k = nKFs;
        m_kfs.resize(k);
        if (FAILED(pKFI->GetKeyFrames(&TIME_FORMAT_MEDIA_TIME, m_kfs.data(), k)) || k != nKFs)
        {
            m_kfs.clear();
        }
    }

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