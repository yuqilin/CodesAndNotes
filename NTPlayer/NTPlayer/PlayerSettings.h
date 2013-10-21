#ifndef _NTPLAYER_PLAYERSETTINGS_H_
#define _NTPLAYER_PLAYERSETTINGS_H_

enum {
    VIDRNDT_DS_DEFAULT,
    VIDRNDT_DS_OLDRENDERER,
    VIDRNDT_DS_OVERLAYMIXER,
    VIDRNDT_DS_VMR7WINDOWED,
    VIDRNDT_DS_VMR9WINDOWED,
    VIDRNDT_DS_VMR7RENDERLESS,
    VIDRNDT_DS_VMR9RENDERLESS,
    VIDRNDT_DS_DXR,
    VIDRNDT_DS_NULL_COMP,
    VIDRNDT_DS_NULL_UNCOMP,
    VIDRNDT_DS_EVR,
    VIDRNDT_DS_EVR_CUSTOM,
    VIDRNDT_DS_MADVR,
    VIDRNDT_DS_SYNC
};


#pragma pack(push, 1)
typedef struct {
    bool fValid;
    CSize size;
    int bpp, freq;
    DWORD dmDisplayFlags;
} dispmode;

typedef struct {
    double vfr_from;
    double vfr_to;
    bool fChecked;
    dispmode dmFSRes;
    bool fIsData;
} fpsmode;

#define MAX_FPS_COUNT 30
typedef struct {
    bool bEnabled;
    fpsmode dmFullscreenRes[MAX_FPS_COUNT];
    bool bApplyDefault;
}   AChFR; //AutoChangeFullscrRes
#pragma pack(pop)


struct CPlayerSettings // : public Sington<>
{
    HMODULE m_hInstance;
    CString m_strCodecsPath;

    bool fEnableWorkerThreadForOpening;

    // Formats
    CMediaFormats   m_Formats;

    // Fullscreen
    bool            fLaunchfullscreen;
    bool            fShowBarsWhenFullScreen;
    int             nShowBarsWhenFullScreenTimeOut;
    bool            fExitFullScreenAtTheEnd;
    CStringW        strFullScreenMonitor;
    AChFR           AutoChangeFullscrRes;
    bool            fRestoreResAfterExit;


    bool            IsD3DFullscreen() const;

    bool fDisableInternalSubtitles;

    bool fKeepHistory;
    bool fRememberFilePos;

    bool fReportFailedPins;

    bool fEnableAudioSwitcher;

    int iDSVideoRendererType;

    bool fPlayingInFullScreen;

public:
    void LoadDefaultSettings();

};

#endif