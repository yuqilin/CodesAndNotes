#pragma once

#include "BaseGraph.h"
#include "CodecsInfo.h"

class DirectShowGraph : public BaseGraph
{
public:
    DirectShowGraph(HRESULT& hr);
    ~DirectShowGraph();

    // IPlayControl
    HRESULT OpenMedia(MediaInfo* media_info);
    HRESULT CloseMedia();
    HRESULT Play();
    HRESULT Pause();
    HRESULT Stop();
    HRESULT Abort();
    HRESULT GetPlayState(PlayerState* state);
    HRESULT GetDuration(long* duration);
    HRESULT GetCurrentPlayPos(long* current_play_pos);
    HRESULT SetPlayPos(long pos_to_play);
    bool         IsPlaying();

    // IVideoControl
    HRESULT SetVideoWindow(void* video_window);
    HRESULT SetVideoPosition(int );
    HRESULT GetVideoSize(int* w, int* h);
    HRESULT SetColorControl(int brightness, int contrast, int hue, int staturation);
    HRESULT LoadExternalSubtitle(const char* subtitle_path);
    HRESULT GrabCurrentVideoFrame(const char* save_file_name);

    // IAudioControl
    HRESULT GetMute(bool* mute);
    HRESULT SetMute(bool mute);
    HRESULT GetVolume(int* volume);
    HRESULT SetVolume(int volume);

protected:
    virtual HRESULT PrepareRenderFile();

    HRESULT RenderFile(LPCWSTR lpwcsUrl);
    HRESULT EnumSourceFilters(LPCWSTR lpcwstrFileName, CodecsList& fl);
    HRESULT AddSourceFilter(CodecsInfo* info, LPCWSTR lpcwstrFileName, LPCWSTR lpcwstrFilterName, IBaseFilter** ppBF);
    HRESULT AddFilter(IBaseFilter* pFilter, LPCWSTR pName);
    HRESULT ConnectFilter(IBaseFilter* pBF, IPin* pPinIn);
    HRESULT ConnectFilter(IPin* pPinOut, IBaseFilter* pBF);
    HRESULT Connect(IPin* pPinOut, IPin* pPinIn);
    HRESULT Connect(IPin* pPinOut, IPin* pPinIn, bool bContinueRender);
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
    HRESULT Core_EnumSourceFilters(CodecsListEx& fl);
    HRESULT Core_AddSourceFilter(CodecsInfo* info, IBaseFilter** ppBF);
    HRESULT Core_FileSourceFilterLoad(IBaseFilter* pBF, LPCTSTR pSubtype);
    HRESULT Core_AddFilter(CodecsInfo* info, IBaseFilter** ppBF);

    HRESULT Core_AddFilter(IBaseFilter* pBF, LPCWSTR pName);
    HRESULT Core_RenderFilter(IBaseFilter* pBF);
    HRESULT Core_RenderPin(IPin* pPinOut);
    HRESULT Core_ConnectFilterDirect(IPin* pPinOut, IBaseFilter* pBF, const AM_MEDIA_TYPE* pmt);
    HRESULT Core_OnRenderComplete();


protected:
    CComPtr<IGraphBuilder> m_pIGraphBuilder;
    CInterfaceList<IUnknown, &IID_IUnknown> m_pUnks;
    CCritSec m_Lock;
    UINT64 m_vrmerit;
    UINT64 m_armerit;

    CComPtr<IBaseFilter> m_pVSFilter;
    CComPtr<IBaseFilter> m_pAudioSwitcher;

    CString m_SourceFilterLoadSubtype;
};

