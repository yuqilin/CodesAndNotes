
#ifndef _PLAYERCORE_DSUTIL_H_
#define _PLAYERCORE_DSUTIL_H_

//////////////////////////////////////////////////////////////////////////
#define QI(i)  (riid == __uuidof(i)) ? GetInterface((i*)this, ppv) :
#define QI2(i) (riid == IID_##i) ? GetInterface((i*)this, ppv) :

#define SAFE_DELETE(p)       { if (p) { delete (p);     (p) = NULL; } }
#define SAFE_DELETE_ARRAY(p) { if (p) { delete [] (p);  (p) = NULL; } }
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p) = NULL; } }


//////////////////////////////////////////////////////////////////////////
void RepaireSystemVolume(void);


//////////////////////////////////////////////////////////////////////////

int  CountPins(IBaseFilter* pBF, int& nIn, int& nOut, int& nInC, int& nOutC);
bool IsSplitter(IBaseFilter* pBF, bool fCountConnectedOnly = false);
bool IsMultiplexer(IBaseFilter* pBF, bool fCountConnectedOnly = false);
bool IsStreamStart(IBaseFilter* pBF);
bool IsStreamEnd(IBaseFilter* pBF);
bool IsVideoRenderer(IBaseFilter* pBF);
bool IsAudioWaveRenderer(IBaseFilter* pBF);

IBaseFilter* GetUpStreamFilter(IBaseFilter* pBF, IPin* pInputPin = NULL);
IPin* GetUpStreamPin(IBaseFilter* pBF, IPin* pInputPin = NULL);
IPin* GetFirstPin(IBaseFilter* pBF, PIN_DIRECTION dir = PINDIR_INPUT);
IPin* GetFirstDisconnectedPin(IBaseFilter* pBF, PIN_DIRECTION dir);

CString GetPinName(IPin* pPin);
IBaseFilter* GetFilterFromPin(IPin* pPin);
IBaseFilter* FindFilter(LPCWSTR clsid, IFilterGraph* pFG);
IBaseFilter* FindFilter(const CLSID& clsid, IFilterGraph* pFG);


void  ExtractMediaTypes(IPin* pPin, CAtlArray<GUID>& types);
void  ExtractMediaTypes(IPin* pPin, CAtlList<CMediaType>& mts);

//////////////////////////////////////////////////////////////////////////
CString GetMediaTypeName(const GUID& guid);
GUID GUIDFromCString(CString str);
HRESULT GUIDFromCString(CString str, GUID& guid);
CString CStringFromGUID(const GUID& guid);

//////////////////////////////////////////////////////////////////////////
CString MakeFullPath(LPCTSTR path);
HRESULT LoadExternalObject(LPCTSTR path, REFCLSID clsid, REFIID iid, void** ppv);
HRESULT LoadExternalFilter(LPCTSTR path, REFCLSID clsid, IBaseFilter** ppBF);


#endif
