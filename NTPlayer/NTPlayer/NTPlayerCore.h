#ifndef __NTPLAYER_NTPLAYERCORE_H_
#define __NTPLAYER_NTPLAYERCORE_H_

#include "CodecsManager.h"

class CNTPlayerCore
{
public:
    CNTPlayerCore();
    ~CNTPlayerCore();

    HRESULT LoadCodecsInfo();
    HRESULT LoadPlayerSettings();

    HRESULT GetCodecsInfoString(const char** info);
    
    HRESULT OpenMedia(const char* url);
    HRESULT CloseMedia();

    HRESULT Play();
    HRESULT Pause();
    HRESULT Stop();

    static CCodecsManager m_CodecsManager;

protected:
    HRESULT Kernel_Render();

    typedef struct {
        CodecsInfo* info;
        CString subtype;
    } MatchingItem;
    typedef CAtlList<MatchingItem> MatchingItemList;
    HRESULT EnumSourceFilters(LPCTSTR file_name, MatchingItemList& matching_list);

    BOOL CheckProtocol(CString protocol, const CodecsInfo* info);
    BOOL CheckBytes(HANDLE hFile, const CString& chkbytes);

    CString m_strUrl;

    CBasePlayer* m_pPlayer;

    CComPtr<IGraphBuilder2> m_pGB;
    CComQIPtr<IMediaControl> m_pMC;
    CComQIPtr<IMediaEventEx> m_pME;
    CComQIPtr<IVideoWindow> m_pVW;
    CComQIPtr<IBasicVideo> m_pBV;
    CComQIPtr<IBasicAudio> m_pBA;
    CComQIPtr<IMediaSeeking> m_pMS;
    CComQIPtr<IVideoFrameStep> m_pFS;
    CComQIPtr<IFileSourceFilter> m_pFSF;

    double m_dSpeedRate;


    HWND m_hVideoWindow;
};

#endif