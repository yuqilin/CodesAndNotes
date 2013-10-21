#pragma once

class CDSGraph : public CUnknown
                , public IGraphBuilder2
                , public IGraphBuilderDeadEnd
                , public CCritSec
{
protected:
    


public:
    CDSGraph();
    ~CDSGraph();


    HRESULT EnumSourceFilters(LPCWSTR lpcwstrFileName, CFGFilterList& fl);
    HRESULT AddSourceFilter(CFGFilter* pFGF, LPCWSTR lpcwstrFileName, LPCWSTR lpcwstrFilterName, IBaseFilter** ppBF);
    HRESULT Connect(IPin* pPinOut, IPin* pPinIn, bool bContinueRender);

    // IFilterGraph

    STDMETHODIMP AddFilter(IBaseFilter* pFilter, LPCWSTR pName);
    STDMETHODIMP RemoveFilter(IBaseFilter* pFilter);
    STDMETHODIMP EnumFilters(IEnumFilters** ppEnum);
    STDMETHODIMP FindFilterByName(LPCWSTR pName, IBaseFilter** ppFilter);
    STDMETHODIMP ConnectDirect(IPin* pPinOut, IPin* pPinIn, const AM_MEDIA_TYPE* pmt);
    STDMETHODIMP Reconnect(IPin* ppin);
    STDMETHODIMP Disconnect(IPin* ppin);
    STDMETHODIMP SetDefaultSyncSource();

    // IGraphBuilder

    STDMETHODIMP Connect(IPin* pPinOut, IPin* pPinIn);
    STDMETHODIMP Render(IPin* pPinOut);
    STDMETHODIMP RenderFile(LPCWSTR lpcwstrFile, LPCWSTR lpcwstrPlayList);
    STDMETHODIMP AddSourceFilter(LPCWSTR lpcwstrFileName, LPCWSTR lpcwstrFilterName, IBaseFilter** ppFilter);
    STDMETHODIMP SetLogFile(DWORD_PTR hFile);
    STDMETHODIMP Abort();
    STDMETHODIMP ShouldOperationContinue();

    // IFilterGraph2

    STDMETHODIMP AddSourceFilterForMoniker(IMoniker* pMoniker, IBindCtx* pCtx, LPCWSTR lpcwstrFilterName, IBaseFilter** ppFilter);
    STDMETHODIMP ReconnectEx(IPin* ppin, const AM_MEDIA_TYPE* pmt);
    STDMETHODIMP RenderEx(IPin* pPinOut, DWORD dwFlags, DWORD* pvContext);

    // IGraphBuilder2

    STDMETHODIMP IsPinDirection(IPin* pPin, PIN_DIRECTION dir);
    STDMETHODIMP IsPinConnected(IPin* pPin);
    STDMETHODIMP ConnectFilter(IBaseFilter* pBF, IPin* pPinIn);
    STDMETHODIMP ConnectFilter(IPin* pPinOut, IBaseFilter* pBF);
    STDMETHODIMP ConnectFilterDirect(IPin* pPinOut, IBaseFilter* pBF, const AM_MEDIA_TYPE* pmt);
    STDMETHODIMP NukeDownstream(IUnknown* pUnk);
    STDMETHODIMP FindInterface(REFIID iid, void** ppv, BOOL bRemove);
    STDMETHODIMP AddToROT();
    STDMETHODIMP RemoveFromROT();

    // IGraphBuilderDeadEnd

    STDMETHODIMP_(size_t) GetCount();
    STDMETHODIMP GetDeadEnd(int iIndex, CAtlList<CStringW>& path, CAtlList<CMediaType>& mts);

public:
    CFGManager(LPCTSTR pName, LPUNKNOWN pUnk);
    virtual ~CFGManager();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

};