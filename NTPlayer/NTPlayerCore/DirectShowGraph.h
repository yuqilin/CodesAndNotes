#pragma once

#include "Mutex.h"
#include "BaseGraph.h"
#include "CodecsInfo.h"
#include "PlayerSettings.h"
#include "SubtitleInput.h"

class BaseVideoRenderer;

class DirectShowGraph : public BaseGraph
{
    friend class CTextPassThruFilter;
    friend class CTextPassThruInputPin;
public:
    DirectShowGraph(PlayerCore* pPlayer, HRESULT& hr);
    ~DirectShowGraph();

    // IPlayControl
    HRESULT OpenMedia(MediaInfo* media_info);
    HRESULT CloseMedia();
    HRESULT Play();
    HRESULT Pause();
    HRESULT Stop();
    HRESULT Abort();
    HRESULT GetDuration(long* duration);
    HRESULT GetCurrentPlayPos(long* current_play_pos);
    HRESULT SetPlayPos(long pos_to_play);
    bool         IsPlaying();

    // IVideoControl
    HRESULT SetVideoWindow(void* video_window);
    HRESULT SetVideoPosition(LPRECT lpRect);
    HRESULT RepaintVideo();
    HRESULT GetVideoSize(VideoSize* pVideoSize);
    HRESULT SetColorControl(int brightness, int contrast, int hue, int staturation);
    HRESULT LoadExternalSubtitle(const char* subtitle_path);
    HRESULT GrabCurrentVideoFrame(const char* save_file_name);

    // IAudioControl
    HRESULT GetMute(bool* mute);
    HRESULT SetMute(bool mute);
    HRESULT GetVolume(int* volume);
    HRESULT SetVolume(int volume);

protected:
    //virtual HRESULT PrepareRenderFile();

    HRESULT RenderFile(LPCWSTR lpwcsUrl);
    HRESULT ConnectFilter(IBaseFilter* pBF, IPin* pPinIn);
    HRESULT ConnectFilter(IPin* pPinOut, IBaseFilter* pBF);
    //HRESULT Connect(IPin* pPinOut, IPin* pPinIn);
    //HRESULT Connect(IPin* pPinOut, IPin* pPinIn, bool bContinueRender);
    HRESULT ConnectFilterDirect(IPin* pPinOut, IBaseFilter* pBF, const AM_MEDIA_TYPE* pmt);
    HRESULT ConnectDirect(IPin* pPinOut, IPin* pPinIn, const AM_MEDIA_TYPE* pmt);
    HRESULT Disconnect(IPin* ppin);
    
    HRESULT NukeDownstream(IUnknown* pUnk);
    HRESULT RemoveFilter(IBaseFilter* pFilter);

    HRESULT IsPinDirection(IPin* pPin, PIN_DIRECTION dir1);
    HRESULT IsPinConnected(IPin* pPin);

    static bool CheckBytes(HANDLE hFile, CString chkbytes);
    static bool CheckBytes(MEMORY_DATA* header, CString chkbytes);


    HRESULT FindFilterByClsid(LPCTSTR pClsid, IBaseFilter** ppBF);

    // Core functions
    BOOL Core_CheckProtocol(CodecsInfo* info);
    BOOL Core_CheckBytes(CodecsInfo* info);
    BOOL Core_CheckExtension(CodecsInfo* info);
    BOOL Core_OnSpecialParse(CodecsInfo** info);

    HRESULT Core_OnRenderPrepare();
    HRESULT Core_Render();
    HRESULT Core_OnRenderComplete();

    HRESULT Core_EnumSourceFilters(CodecsListEx& fl);
    HRESULT Core_AddSourceFilter(CodecsInfo* info, IBaseFilter** ppBF);
    HRESULT Core_FileSourceFilterLoad(IBaseFilter* pBF, LPCTSTR pSubtype);
    HRESULT Core_AddFilter(CodecsInfo* info,
                           IBaseFilter** ppBF,
                           CInterfaceList<IUnknown, &IID_IUnknown>& pUnks);

    HRESULT AddFilter(IBaseFilter* pBF, LPCWSTR pName);
    HRESULT Core_RenderFilter(IBaseFilter* pBF);
    HRESULT Core_RenderPin(IPin* pPinOut);
    HRESULT Core_ConnectFilterDirect(IPin* pPinOut, IBaseFilter* pBF, const AM_MEDIA_TYPE* pmt);

    BOOL Core_CanAddFilter(IPin* pPinOut, CodecsInfo* info);

    HRESULT LookupFiltersInGraph();
    HRESULT LookupFiltersInCodecs(IPin* pPinUpper, CodecsListEx& filters);

    HRESULT FindInterface(REFIID iid, void** ppv, BOOL bRemove);

    HRESULT OnCreateFilterPrepare(CodecsInfo* pInfo, void** pParam);
    HRESULT OnCreateFilterCompelete(CodecsInfo* pInfo, IBaseFilter* pBF);
    HRESULT OnRenderFilterEnd(CodecsInfo* pInfo, IBaseFilter* pBF);

    HRESULT RemoveAllFilter();


    void AddTextPassThruFilter();
    HRESULT InsertTextPassThruFilter(IBaseFilter* pBF, IPin* pPin, IPin* pPinTo);
    void ReplaceSubtitle(const ISubStream* pSubStreamOld, ISubStream* pSubStreamNew);
    bool SetSubtitle(int i, bool bIsOffset = false, bool bDisplayMessage = false, bool bApplyDefStyle = false);
    void SetSubtitle(ISubStream* pSubStream, bool bApplyDefStyle = false);
    SubtitleInput* GetSubtitleInput(int& i, bool bIsOffset = false);
    void InvalidateSubtitle(DWORD_PTR nSubtitleId, REFERENCE_TIME rtInvalidate);

    DWORD SetupSubtitleStreams();

protected:
    FastMutex m_GraphMutex;
    CCritSec m_Lock;

    CComPtr<IGraphBuilder> m_pIGraphBuilder;
    CComQIPtr<IMediaControl> m_pIMediaControl;
    CComQIPtr<IMediaEventEx> m_pIMediaEventEx;
    CComQIPtr<IVideoWindow> m_pIVideoWindow;
    CComQIPtr<IBasicVideo> m_pIBasicVideo;
    CComQIPtr<IBasicAudio> m_pIBasicAudio;
    CComQIPtr<IMediaSeeking> m_pIMediaSeeking;

    CInterfaceList<IUnknown, &IID_IUnknown> m_pUnks;

    CComPtr<IBaseFilter> m_pVSFilter;
    CComPtr<IBaseFilter> m_pAudioSwitcher;

    BaseVideoRenderer* m_pVideoRenderer;

    volatile BOOL m_bAborted;

    CComPtr<ISubPicAllocatorPresenter> m_pCAP;
    CAtlList<SubtitleInput> m_pSubStreams;
    ISubStream* m_pCurrentSubStream;

    CCritSec m_csSubLock;

    //CString m_strPrefix;

// private:
//     class LogPrefix{
//         #define kCoreRenderRoutinePrefix      _T("    ")
//     public:
//         LogPrefix(DirectShowGraph* graph) : m_graph(graph) {
//             m_graph->m_strPrefix += kCoreRenderRoutinePrefix;
//         }
//         ~LogPrefix() {
//             m_graph->m_strPrefix.TrimRight(kCoreRenderRoutinePrefix);
//         }
//         DirectShowGraph* m_graph;
//     };
};

