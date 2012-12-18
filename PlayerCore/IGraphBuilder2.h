
#ifndef _PLAYERCORE_IGRAPHBUILDER2_H_
#define _PLAYERCORE_IGRAPHBUILDER2_H_

interface __declspec(uuid("011C7527-4F73-42c8-92C1-321D5F7D6E49"))
IGraphBuilder2 : public IFilterGraph2
{
	STDMETHOD(IsPinDirection)(IPin * pPin, PIN_DIRECTION dir) = 0;
	STDMETHOD(IsPinConnected)(IPin * pPin) = 0;
	STDMETHOD(ConnectFilter)(IBaseFilter * pBF, IPin * pPinIn) = 0;
	STDMETHOD(ConnectFilter)(IPin * pPinOut, IBaseFilter * pBF) = 0;
	STDMETHOD(ConnectFilterDirect)(IPin * pPinOut, IBaseFilter * pBF, const AM_MEDIA_TYPE * pmt) = 0;
	STDMETHOD(NukeDownstream)(IUnknown * pUnk) = 0;
	STDMETHOD(FindInterface)(REFIID iid, void** ppv, BOOL bRemove) = 0;
};

// private use only
interface __declspec(uuid("40A121F0-DFCC-4ded-B81F-95391955DE2A"))
IGraphBuilderDeadEnd : public IUnknown
{
	STDMETHOD_(size_t, GetCount)() = 0;
	STDMETHOD(GetDeadEnd)(int iIndex, CAtlList<CString>& path, CAtlList<CMediaType>& mts) = 0;
};


#endif