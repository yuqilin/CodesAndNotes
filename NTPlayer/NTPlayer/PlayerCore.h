#ifndef _NTPLAYER_PLAYERCORE_H_
#define _NTPLAYER_PLAYERCORE_H_

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
#define PlayerGetPlayerSettings() CPlayerCore::m_PlayerSettings

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
typedef enum MPC_LOADSTATE {
    MLS_CLOSED,
    MLS_LOADING,
    MLS_LOADED,
    MLS_CLOSING
};


typedef enum MPC_PLAYSTATE {
    PS_PLAY   = 0,
    PS_PAUSE  = 1,
    PS_STOP   = 2,
    PS_UNUSED = 3
};

//////////////////////////////////////////////////////////////////////////
// OpenMediaData
//////////////////////////////////////////////////////////////////////////
class OpenMediaData
{
public:
    //  OpenMediaData() {}
    virtual ~OpenMediaData() {} // one virtual funct is needed to enable rtti
    CString title;
    CAtlList<CString> subs;
};

class OpenFileData : public OpenMediaData
{
public:
    OpenFileData() : rtStart(0) {}
    //CAtlList<CString> fns;
    CString strFileName;
    REFERENCE_TIME rtStart;
};

class OpenDVDData : public OpenMediaData
{
public:
    //  OpenDVDData() {}
    CString path;
    CComPtr<IDvdState> pDvdState;
};

class OpenDeviceData : public OpenMediaData
{
public:
    OpenDeviceData() {
        vinput = vchannel = ainput = -1;
    }
    CStringW DisplayName[2];
    int vinput, vchannel, ainput;
};

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
struct SubtitleInput {
    CComQIPtr<ISubStream> subStream;
    CComPtr<IBaseFilter> sourceFilter;

    SubtitleInput() {};
    SubtitleInput(CComQIPtr<ISubStream> subStream) : subStream(subStream) {};
    SubtitleInput(CComQIPtr<ISubStream> subStream, CComPtr<IBaseFilter> sourceFilter)
        : subStream(subStream), sourceFilter(sourceFilter) {};
};


//////////////////////////////////////////////////////////////////////////
// CPlayerCore
//////////////////////////////////////////////////////////////////////////
class CPlayerCore
{
    // TODO: wrap these graph objects into a class to make it look cleaner

    CComPtr<IGraphBuilder2> m_pGB;
    CComQIPtr<IMediaControl> m_pMC;
    CComQIPtr<IMediaEventEx> m_pME;
    CComQIPtr<IVideoWindow> m_pVW;
    CComQIPtr<IBasicVideo> m_pBV;
    CComQIPtr<IBasicAudio> m_pBA;
    CComQIPtr<IMediaSeeking> m_pMS;
    CComQIPtr<IVideoFrameStep> m_pFS;
    CComQIPtr<IFileSourceFilter> m_pFSF;
    CComQIPtr<IQualProp, &IID_IQualProp> m_pQP;
    //CComQIPtr<IBufferInfo> m_pBI;
    CComQIPtr<IAMOpenProgress> m_pAMOP;
    CComPtr<IVMRMixerControl9> m_pVMRMC;
    CComPtr<IMFVideoDisplayControl> m_pMFVDC;
    CComPtr<IMFVideoProcessor> m_pMFVP;
    CComPtr<IVMRWindowlessControl9> m_pVMRWC;

    CComPtr<ISubPicAllocatorPresenter> m_pCAP;
    CComPtr<ISubPicAllocatorPresenter2> m_pCAP2;

    CComQIPtr<IDvdControl2> m_pDVDC;
    CComQIPtr<IDvdInfo2> m_pDVDI;
    //CComPtr<IAMLine21Decoder_2> m_pLN21;

    CComPtr<ICaptureGraphBuilder2> m_pCGB;
    CStringW m_VidDispName, m_AudDispName;
    CComPtr<IBaseFilter> m_pVidCap, m_pAudCap;
    CComPtr<IAMVideoCompression> m_pAMVCCap, m_pAMVCPrev;
    CComPtr<IAMStreamConfig> m_pAMVSCCap, m_pAMVSCPrev, m_pAMASC;
    CComPtr<IAMCrossbar> m_pAMXBar;
    CComPtr<IAMTVTuner> m_pAMTuner;
    CComPtr<IAMDroppedFrames> m_pAMDF;


    bool m_fCustomGraph;
    bool m_fRealMediaGraph, m_fShockwaveGraph, m_fQuicktimeGraph;

    CComPtr<ISubClock> m_pSubClock;


    // subtitles

    CCritSec m_csSubLock;

    CList<SubtitleInput> m_pSubStreams;
    POSITION m_posFirstExtSub;
    ISubStream* m_pCurrentSubStream;

    SubtitleInput* GetSubtitleInput(int& i, bool bIsOffset = false);

    friend class CTextPassThruFilter;

public:

    CPlayerCore();
    ~CPlayerCore();

    HRESULT Open(const char* url);
    HRESULT Close();



    HRESULT Play();
    HRESULT Pause();

    HRESULT SetCodecsPath(LPCTSTR lpszCodecsPath);


    void OnOpenResult(HRESULT hr);


protected:
    HRESULT OpenMedia(CAutoPtr<OpenMediaData> pOMD);
    HRESULT CloseMedia();


    HRESULT CPlayerCore::OpenMediaPrivate(CAutoPtr<OpenMediaData> pOMD);

    void SetLoadState();


    void AutoChangeMonitorMode();
    bool CreateFullScreenWindow();

public:
    static CPlayerSettings  m_settings;

    bool m_fStartInD3DFullscreen;

protected:
    MPC_LOADSTATE m_iMediaLoadState;

    bool m_fAudioOnly;



    friend class CGraphThread;
    CGraphThread* m_pGraphThread;
    bool m_bOpenedThruThread;

    bool m_fOpeningAborted;

    bool m_bRememberFilePos;

    LARGE_INTEGER m_liLastSaveTime;


    static CCodecsManager m_codecs;

private:
    HWND m_hVideoWindow;
    HWND m_hNotifyWindow;
};


#endif