#ifndef _NTPLAYER_PLAYERCORE_H_
#define _NTPLAYER_PLAYERCORE_H_

#include <d3d9.h>
#include <d3d9types.h>
#include <dxva2api.h>
#include <vmr9.h>
#include <evr.h>
#include <evr9.h>
#include "SyncClock.h"
#include "CodecsManager.h"
#include "PlayerSettings.h"
#include "DSMPropertyBag.h"
#include "ISubPic.h"
#include "IGraphBuilder2.h"

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
#define PlayerGetPlayerSettings() CPlayerCore::m_PlayerSettings

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

enum {
    PM_NONE,
    PM_FILE,
    PM_DVD,
    PM_CAPTURE
};


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

typedef enum {
    ProcAmp_Brightness = 0x1,
    ProcAmp_Contrast   = 0x2,
    ProcAmp_Hue        = 0x4,
    ProcAmp_Saturation = 0x8,
    ProcAmp_All = ProcAmp_Brightness | ProcAmp_Contrast | ProcAmp_Hue | ProcAmp_Saturation
} ControlType;

typedef struct {
    DWORD dwProperty;
    int   MinValue;
    int   MaxValue;
    int   DefaultValue;
    int   StepSize;
} COLORPROPERTY_RANGE;

__inline DXVA2_Fixed32 IntToFixed(__in const int _int_, __in const short divisor = 1)
{
    // special converter that is resistant to MS bugs
    DXVA2_Fixed32 _fixed_;
    _fixed_.Value = SHORT(_int_ / divisor);
    _fixed_.Fraction = USHORT((_int_ % divisor * 0x10000 + divisor / 2) / divisor);
    return _fixed_;
}

__inline int FixedToInt(__in const DXVA2_Fixed32& _fixed_, __in const short factor = 1)
{
    // special converter that is resistant to MS bugs
    return (int)_fixed_.Value * factor + ((int)_fixed_.Fraction * factor + 0x8000) / 0x10000;
}


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

interface ISubClock;


//////////////////////////////////////////////////////////////////////////
// CPlayerCore
//////////////////////////////////////////////////////////////////////////
class CPlayerCore
{
    friend class CGraphThread;

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

    CAtlList<SubtitleInput> m_pSubStreams;
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

    int GetPlaybackMode() const { return m_iPlaybackMode; }


    static CCodecsManager& GetCodecsManager() {
        return m_codecs;
    }



protected:
    HRESULT OpenMedia(CAutoPtr<OpenMediaData> pOMD);
    HRESULT CloseMedia();


    HRESULT OpenMediaPrivate(CAutoPtr<OpenMediaData> pOMD);
    HRESULT CloseMediaPrivate();

    HRESULT OpenCreateGraphObject(OpenMediaData* pOMD);
    HRESULT OpenFile(OpenFileData* pOFD);

    void OpenCustomizeGraph();
    void OpenSetupVideo();
    void OpenSetupAudio();

    DWORD SetupAudioStreams();
    DWORD SetupSubtitleStreams();

    void SetPlaybackMode(int iNewStatus);

    void MoveVideoWindow(bool fShowStats = false);

    void OnPlayStop();

    void SetLoadState(MPC_LOADSTATE state);


    CSize GetVideoSize() const;


    void AutoChangeMonitorMode();
    bool CreateFullScreenWindow();

    void SetupVMR9ColorControl();

    OAFilterState GetMediaState() const;

    void SetColorControl(DWORD flags, int& brightness, int& contrast, int& hue, int& saturation);

public:
    static CPlayerSettings  m_settings;

    bool m_fStartInD3DFullscreen;

protected:
    MPC_LOADSTATE m_iMediaLoadState;

    bool m_fAudioOnly;


    CGraphThread* m_pGraphThread;
    bool m_bOpenedThruThread;

    bool m_fOpeningAborted;

    bool m_bRememberFilePos;

    LARGE_INTEGER m_liLastSaveTime;

    int m_iPlaybackMode;

    double m_dSpeedRate;

    static CCodecsManager m_codecs;

    bool m_fLiveWM;
    bool m_fEndOfStream;
    REFERENCE_TIME m_rtDurationOverride;
    std::vector<REFERENCE_TIME> m_kfs;
    CComPtr<IDSMChapterBag> m_pCB;


    double m_ZoomX;
    double m_ZoomY;
    double m_PosX;
    double m_PosY;

    int m_AngleX, m_AngleY, m_AngleZ;


private:
    HWND m_hVideoWindow;
    HWND m_hNotifyWindow;
};


#endif