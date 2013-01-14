
#include "stdafx.h"
#include <atlpath.h>

#include "DSUtil.h"

//////////////////////////////////////////////////////////////////////////
void RepaireSystemVolume(void)
{
	if(CSysVersion::IsVistaOrLater())
	{
		DWORD dwVolume = 0;
		MMRESULT nResult = waveOutGetVolume(0, &dwVolume);
		if(nResult == MMSYSERR_NOERROR)
		{
			if(dwVolume == 0)
			{
				g_utility.Log(_T("Repaire system volume."));
				waveOutSetVolume(0, 0xFFFFFFFF);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////

int CountPins(IBaseFilter* pBF, int& nIn, int& nOut, int& nInC, int& nOutC)
{
	nIn = nOut = 0;
	nInC = nOutC = 0;

	BeginEnumPins(pBF, pEP, pPin) {
		PIN_DIRECTION dir;
		if (SUCCEEDED(pPin->QueryDirection(&dir))) {
			CComPtr<IPin> pPinConnectedTo;
			pPin->ConnectedTo(&pPinConnectedTo);

			if (dir == PINDIR_INPUT) {
				nIn++;
				if (pPinConnectedTo) {
					nInC++;
				}
			} else if (dir == PINDIR_OUTPUT) {
				nOut++;
				if (pPinConnectedTo) {
					nOutC++;
				}
			}
		}
	}
	EndEnumPins;

	return (nIn + nOut);
}

bool IsSplitter(IBaseFilter* pBF, bool fCountConnectedOnly)
{
	int nIn, nOut, nInC, nOutC;
	CountPins(pBF, nIn, nOut, nInC, nOutC);
	return (fCountConnectedOnly ? nOutC > 1 : nOut > 1);
}

bool IsMultiplexer(IBaseFilter* pBF, bool fCountConnectedOnly)
{
	int nIn, nOut, nInC, nOutC;
	CountPins(pBF, nIn, nOut, nInC, nOutC);
	return (fCountConnectedOnly ? nInC > 1 : nIn > 1);
}

bool IsStreamStart(IBaseFilter* pBF)
{
	CComQIPtr<IAMFilterMiscFlags> pAMMF(pBF);
	if (pAMMF && pAMMF->GetMiscFlags()&AM_FILTER_MISC_FLAGS_IS_SOURCE) {
		return true;
	}

	int nIn, nOut, nInC, nOutC;
	CountPins(pBF, nIn, nOut, nInC, nOutC);
	AM_MEDIA_TYPE mt;
	CComPtr<IPin> pIn = GetFirstPin(pBF);
	return ((nOut > 1)
		|| (nOut > 0 && nIn == 1 && pIn && SUCCEEDED(pIn->ConnectionMediaType(&mt)) && mt.majortype == MEDIATYPE_Stream));
}

bool IsStreamEnd(IBaseFilter* pBF)
{
	int nIn, nOut, nInC, nOutC;
	CountPins(pBF, nIn, nOut, nInC, nOutC);
	return (nOut == 0);
}

bool IsVideoRenderer(IBaseFilter* pBF)
{
	int nIn, nOut, nInC, nOutC;
	CountPins(pBF, nIn, nOut, nInC, nOutC);

	if (nInC > 0 && nOut == 0) {
		BeginEnumPins(pBF, pEP, pPin) {
			AM_MEDIA_TYPE mt;
			if (S_OK != pPin->ConnectionMediaType(&mt)) {
				continue;
			}

			FreeMediaType(mt);

			return !!(mt.majortype == MEDIATYPE_Video);
			/*&& (mt.formattype == FORMAT_VideoInfo || mt.formattype == FORMAT_VideoInfo2));*/
		}
		EndEnumPins;
	}

	CLSID clsid;
	memcpy(&clsid, &GUID_NULL, sizeof(clsid));
	pBF->GetClassID(&clsid);

	return (clsid == CLSID_VideoRenderer || clsid == CLSID_VideoRendererDefault);
}

DEFINE_GUID(CLSID_ReClock,
			0x9dc15360, 0x914c, 0x46b8, 0xb9, 0xdf, 0xbf, 0xe6, 0x7f, 0xd3, 0x6c, 0x6a);

bool IsAudioWaveRenderer(IBaseFilter* pBF)
{
	int nIn, nOut, nInC, nOutC;
	CountPins(pBF, nIn, nOut, nInC, nOutC);

	if (nInC > 0 && nOut == 0 && CComQIPtr<IBasicAudio>(pBF)) {
		BeginEnumPins(pBF, pEP, pPin) {
			AM_MEDIA_TYPE mt;
			if (S_OK != pPin->ConnectionMediaType(&mt)) {
				continue;
			}

			FreeMediaType(mt);

			return !!(mt.majortype == MEDIATYPE_Audio);
			/*&& mt.formattype == FORMAT_WaveFormatEx);*/
		}
		EndEnumPins;
	}

	CLSID clsid;
	memcpy(&clsid, &GUID_NULL, sizeof(clsid));
	pBF->GetClassID(&clsid);

	return (clsid == CLSID_DSoundRender || clsid == CLSID_AudioRender || clsid == CLSID_ReClock
		|| clsid == __uuidof(CNullAudioRenderer) || clsid == __uuidof(CNullUAudioRenderer));
}


class CPinInfo : public PIN_INFO
{
public:
	CPinInfo()
	{
		pFilter = NULL;
	}
	~CPinInfo()
	{
		if (pFilter)
		{
			pFilter->Release();
		}
	}
};

CString GetPinName(IPin* pPin)
{
	CString name;
	CPinInfo pi;
	if (pPin && SUCCEEDED(pPin->QueryPinInfo(&pi)))
	{
		name = pi.achName;
	}
	return name;
}

IBaseFilter* GetFilterFromPin(IPin* pPin)
{
	if (!pPin)
	{
		return NULL;
	}
	IBaseFilter* pBF = NULL;
	CPinInfo pi;
	if (pPin && SUCCEEDED(pPin->QueryPinInfo(&pi)))
	{
		pBF = pi.pFilter;
	}
	return pBF;
}

IBaseFilter* FindFilter(LPCWSTR clsid, IFilterGraph* pFG)
{
	CLSID clsid2;
	CLSIDFromString(CComBSTR(clsid), &clsid2);
	return FindFilter(clsid2, pFG);
}

IBaseFilter* FindFilter(const CLSID& clsid, IFilterGraph* pFG)
{
	BeginEnumFilters(pFG, pEF, pBF)
	{
		CLSID clsid2;
		if (SUCCEEDED(pBF->GetClassID(&clsid2)) && clsid == clsid2)
		{
			return pBF;
		}
	}
	EndEnumFilters;
	return NULL;
}

IBaseFilter* GetUpStreamFilter(IBaseFilter* pBF, IPin* pInputPin)
{
	return GetFilterFromPin(GetUpStreamPin(pBF, pInputPin));
}

IPin* GetUpStreamPin(IBaseFilter* pBF, IPin* pInputPin)
{
	BeginEnumPins(pBF, pEP, pPin)
	{
		if (pInputPin && pInputPin != pPin)
		{
			continue;
		}

		PIN_DIRECTION dir;
		CComPtr<IPin> pPinConnectedTo;
		if (SUCCEEDED(pPin->QueryDirection(&dir)) && dir == PINDIR_INPUT
			&& SUCCEEDED(pPin->ConnectedTo(&pPinConnectedTo)))
		{
				IPin* pRet = pPinConnectedTo.Detach();
				pRet->Release();
				return pRet;
		}
	}
	EndEnumPins;

	return NULL;
}

IPin* GetFirstPin(IBaseFilter* pBF, PIN_DIRECTION dir)
{
	if (!pBF)
	{
		return NULL;
	}

	BeginEnumPins(pBF, pEP, pPin)
	{
		PIN_DIRECTION dir2;
		pPin->QueryDirection(&dir2);
		if (dir == dir2)
		{
			IPin* pRet = pPin.Detach();
			pRet->Release();
			return pRet;
		}
	}
	EndEnumPins;

	return NULL;
}

IPin* GetFirstDisconnectedPin(IBaseFilter* pBF, PIN_DIRECTION dir)
{
	if (!pBF)
	{
		return NULL;
	}

	BeginEnumPins(pBF, pEP, pPin)
	{
		PIN_DIRECTION dir2;
		pPin->QueryDirection(&dir2);
		CComPtr<IPin> pPinTo;
		if (dir == dir2 && (S_OK != pPin->ConnectedTo(&pPinTo)))
		{
			IPin* pRet = pPin.Detach();
			pRet->Release();
			return pRet;
		}
	}
	EndEnumPins;

	return NULL;
}

IPin* GetFirstPin(IBaseFilter* pBF, PIN_DIRECTION dir)
{
	if (!pBF)
	{
		return NULL;
	}

	BeginEnumPins(pBF, pEP, pPin)
	{
		PIN_DIRECTION dir2;
		pPin->QueryDirection(&dir2);
		if (dir == dir2)
		{
			IPin* pRet = pPin.Detach();
			pRet->Release();
			return pRet;
		}
	}
	EndEnumPins;

	return NULL;
}

void ExtractMediaTypes(IPin* pPin, CAtlList<MediaTypeItem>& mts)
{
	mts.RemoveAll();

	BeginEnumMediaTypes(pPin, pEM, pmt)
	{
		bool fFound = false;

		POSITION pos = mts.GetHeadPosition();
		while (!fFound && pos)
		{
			MediaTypeItem& mt = mts.GetNext(pos);
			if (mt.majortype == pmt->majortype && mt.subtype == pmt->subtype)
			{
				fFound = true;
			}
		}

		if (!fFound)
		{
			MediaTypeItem mt;
			mt.majortype = pmt->majortype;
			mt.subtype = pmt->subtype;
			mts.AddTail(mt);
		}
	}
	EndEnumMediaTypes(pmt);
}


//////////////////////////////////////////////////////////////////////////
CString GetMediaTypeName(const GUID& guid)
{
	CString ret = guid == GUID_NULL
		? _T("Any type")
		: CString(GuidNames[guid]);

	if (ret == _T("FOURCC GUID"))
	{
		CString str;
		if (guid.Data1 >= 0x10000)
		{
			str.Format(_T("Video: %c%c%c%c"), (guid.Data1 >> 0) & 0xff, (guid.Data1 >> 8) & 0xff, (guid.Data1 >> 16) & 0xff, (guid.Data1 >> 24) & 0xff);
		}
		else
		{
			str.Format(_T("Audio: 0x%08x"), guid.Data1);
		}
		ret = str;
	}
	else if (ret == _T("Unknown GUID Name"))
	{
		WCHAR null[128] = {0}, buff[128];
		StringFromGUID2(GUID_NULL, null, 127);
		ret = CString(CStringW(StringFromGUID2(guid, buff, 127) ? buff : null));
	}

	return ret;
}

GUID GUIDFromCString(CString str)
{
	GUID guid = GUID_NULL;
	HRESULT hr = CLSIDFromString(CComBSTR(str), &guid);
	ASSERT(SUCCEEDED(hr));
	UNREFERENCED_PARAMETER(hr);
	return guid;
}

HRESULT GUIDFromCString(CString str, GUID& guid)
{
	guid = GUID_NULL;
	return CLSIDFromString(CComBSTR(str), &guid);
}

CString CStringFromGUID(const GUID& guid)
{
	WCHAR null[128] = {0}, buff[128];
	StringFromGUID2(GUID_NULL, null, 127);
	return CString(StringFromGUID2(guid, buff, 127) > 0 ? buff : null);
}

//////////////////////////////////////////////////////////////////////////
CString MakeFullPath(LPCTSTR path)
{
	CString full(path);
	full.Replace('/', '\\');

	CString fn;
	fn.ReleaseBuffer(GetModuleFileName(NULL/*AfxGetInstanceHandle()*/, fn.GetBuffer(_MAX_PATH), _MAX_PATH));
	CPath p(fn);

	if (full.GetLength() >= 2 && full[0] == '\\' && full[1] != '\\')
	{
		p.StripToRoot();
		full = CString(p) + full.Mid(1);
	}
	else if (full.Find(_T(":\\")) < 0)
	{
		p.RemoveFileSpec();
		p.AddBackslash();
		full = CString(p) + full;
	}

	CPath c(full);
	c.Canonicalize();
	return CString(c);
}


typedef struct {
	CString path;
	HINSTANCE hInst;
	CLSID clsid;
} ExternalObject;

static CAtlList<ExternalObject> s_extobjs;

HRESULT LoadExternalObject(LPCTSTR path, REFCLSID clsid, REFIID iid, void** ppv)
{
	CheckPointer(ppv, E_POINTER);

	CString fullpath = MakeFullPath(path);

	HINSTANCE hInst = NULL;
	bool fFound = false;

	POSITION pos = s_extobjs.GetHeadPosition();
	while (pos)
	{
		ExternalObject& eo = s_extobjs.GetNext(pos);
		if (!eo.path.CompareNoCase(fullpath))
		{
			hInst = eo.hInst;
			fFound = true;
			break;
		}
	}

	HRESULT hr = E_FAIL;

	if (!hInst)
	{
		hInst = CoLoadLibrary(CComBSTR(fullpath), TRUE);
	}
	if (hInst)
	{
		typedef HRESULT(__stdcall * PDllGetClassObject)(REFCLSID rclsid, REFIID riid, LPVOID * ppv);
		PDllGetClassObject p = (PDllGetClassObject)GetProcAddress(hInst, "DllGetClassObject");

		if (p && FAILED(hr = p(clsid, iid, ppv)))
		{
			CComPtr<IClassFactory> pCF;
			if (SUCCEEDED(hr = p(clsid, __uuidof(IClassFactory), (void**)&pCF)))
			{
				hr = pCF->CreateInstance(NULL, iid, ppv);
			}
		}
	}

	if (FAILED(hr) && hInst && !fFound)
	{
		CoFreeLibrary(hInst);
		return hr;
	}

	if (hInst && !fFound)
	{
		ExternalObject eo;
		eo.path = fullpath;
		eo.hInst = hInst;
		eo.clsid = clsid;
		s_extobjs.AddTail(eo);
	}

	return hr;
}

HRESULT LoadExternalFilter(LPCTSTR path, REFCLSID clsid, IBaseFilter** ppBF)
{
	return LoadExternalObject(path, clsid, __uuidof(IBaseFilter), (void**)ppBF);
}