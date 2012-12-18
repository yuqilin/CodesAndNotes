
#include "stdafx.h"
#include "DShowGraph.h"

#define PREFIX_STRING	_T("   ")

CDSGraph::CDSGraph()
{
	m_hVideoWindow = NULL;
	m_hNotifyWindow = NULL;
	m_pEngine = NULL;
	m_pFilters = NULL;
}

CDSGraph::~CDSGraph()
{

}

HRESULT CDSGraph::Initialize(HWND hVideoWindow,
							   HWND hNotifyWindow,
							   CDSEngine* pEngine)
{
	CheckPointer(hVideoWindow, E_INVALIDARG);
	CheckPointer(hNotifyWindow, E_INVALIDARG);
	CheckPointer(pEngine, E_INVALIDARG);

	m_hVideoWindow = hVideoWindow;
	m_hNotifyWindow = hNotifyWindow;
	m_pEngine = pEngine;
	m_pFilters = CZPlayerCore::GetFilters();

	return S_OK;
}

void CDSGraph::UnInitialize()
{
	m_hVideoWindow = NULL;
	m_hNotifyWindow = NULL;
	m_pEngine = NULL;
}

HRESULT CDSGraph::Render(BOOL bOpenChain)
{
	HRESULT hr = S_OK;

	Kernel_SortInfoByExtension();

	// Call before render
	Kernel_BeforeRender();

	// Render media
	HRESULT hr = E_FAIL;
	if(bOpenChain)
	{
		LPCWSTR pwszUrl = m_pMediaInfo->GetUrl();
		hr = m_pGraph->RenderFile(pwszUrl, NULL);
	}
	else
		hr = Kernel_Render();

	// Call after render
	Kernel_AfterRender(hr);
	if(FAILED(hr))
		return hr;

	// Show used filters
	ShowUsedFilters();

	return hr;
}

BOOL CDSGraph::BringInfoToTop(DSFILTER_INFO * pInfo)
{
	BOOL bResult = TRUE;
	
	bResult = g_filte.BringInfoToTop(pInfo);

	return bResult;
}

HRESULT CDSGraph::AddFilter(DSFILTER_INFO * pInfo,
							  AM_MEDIA_TYPE ** pmts = NULL,
							  int nMediaTypeCount = 0,
							  IBaseFilter ** ppFilter = NULL)
{
	HRESULT hr = S_OK;

	return hr;
}

IBaseFilter* CDSGraph::FindFilter(const TCHAR * pcszClsId)
{
	IBaseFilter* pFilter = NULL;

	return pFilter;
}

BOOL CDSGraph::IsFilterExists(const TCHAR * pcszClsId)
{
	BOOL bResult = TRUE;

	return bResult;
}

BOOL CDSGraph::GetFilterPin(IBaseFilter * pFilter, BOOL bOutPin, IPin ** ppPin)
{
	BOOL bResult = TRUE;

	return bResult;
}

BOOL CDSGraph::GetPinsCount(IBaseFilter * pFilter,
								 int * pnAll,
								 int * pnIn,
								 int * pnInConnected,
								 int * pnOut,
								 int * pnOutConnected)
{
	BOOL bResult = TRUE;

	return bResult;
}

HRESULT	CDSGraph::RemoveFilter(IBaseFilter * pFilter, const TCHAR * pcszClsId = NULL)
{
	HRESULT hr = S_OK;

	return hr;
}

void CDSGraph::RemoveAllFilter(void)
{
	
}

void CDSGraph::RemoveChain(IPin * pOutputPin)
{
	
}

//-------------------------------------------------------------------------
// Kernel Functions
//-------------------------------------------------------------------------
void CDSGraph::Kernel_SortInfoByExtension()
{
	LPCTSTR pcszExtension = m_pMediaInfo->GetExtension();
	if(pcszExtension == NULL)
		return;

	g_utility.Log(_T("Kernel_SortInfoByExtName: %s"), pcszExtension);

	if (m_pFilters != NULL)
	{
		m_pFilters->SortInfoByExtension(pcszExtension);
	}
}

// read FilterInfo.xml, store info : source, transform
// video/audio render selected by settings
// xml only contain the default render
// enum source filters to load file
// render source filter
// enum filters in transform
// 

HRESULT CDSGraph::Kernel_Render()
{
	CheckPointer(m_pFilters, E_INVALIDARG);

	HRESULT hr = S_OK;

	struct MatchingFilterItem
	{
		DSFilterInfo*		info;
		CString				subtype;
	};
	CAtlList<MatchingFilterItem> MatchingList;

	CAtlList<DSFilterInfo*>& InfoList = m_pFilters->GetInfoList();

	CString strSubtype;
	DSFilterInfo* pInfo = NULL;
	POSITION pos = InfoList.GetHeadPosition();
	while (pos)
	{
		pInfo = InfoList.GetNext(pos);
		if (pInfo != NULL)
		{
			if(pInfo->device != FD_FILTER)
				continue;

			if (pInfo->filtertype != FT_SOURCE)
				continue;

			if(pInfo->merit <= MERIT_DO_NOT_USE)
				continue;

			if(!this->Kernel_CheckProtocol(pInfo))
				continue;

			if(!this->Kernel_CheckBytes(pInfo, strSubtype))
				continue;

			MatchingFilterItem item;
			item.info = pInfo;
			item.subtype = strSubtype;
			MatchingList.AddTail(item);
		}
	}

	if (MatchingList.IsEmpty())
	{
		// If no matching filter, try other
		if(!OnSpecialParse(m_pMediaInfo, &pInfo, strSubtype))
		{
			g_utility.Log(_T("No found matching source filter, render failed."));
			return VFW_E_CANNOT_LOAD_SOURCE_FILTER;
		}
		MatchingFilterItem item;
		item.info = pInfo;
		item.subtype = strSubtype;

	}

	pos = MatchingList.GetHeadPosition();
	while (pos)
	{
		MatchingFilterItem& item = MatchingList.GetNext(pos);
		IBaseFilter * pFilter = NULL;
		hr = this->AddFilter(item.info, NULL, 0, &pFilter);
		if (SUCCEEDED(hr))
		{
			hr = this->Kernel_FileSourceFilterLoad(pFilter, item.subtype);
			if(SUCCEEDED(hr))
			{
				// Render source filter
				hr = this->Kernel_RenderFilter(pFilter);
				if(SUCCEEDED(hr))
				{
					g_utility.Log(_T("Source filter \"%s\" render okay, hr=%08x"), pInfo->name, hr);
				}
				else
				{
					g_utility.Log(_T("Source filter \"%s\" render failed, hr=%08x, reset graph"), pInfo->name, hr);
					this->Kernel_ResetGraph(hr);
				}
			}
			else
			{
				g_utility.Log(_T("Source filter \"%s\" load media file failed, hr=%08x"), pInfo->name, hr);

				// If failed, remove this source filter
				if(FAILED(hr))
					this->RemoveFilter(pFilter);
			}
		}

		if(SUCCEEDED(hr))
			break;
	}

	
	return hr;
}

BOOL CDSGraph::Kernel_CheckProtocol(DSFilterInfo* pInfo)
{
	CheckPointer(pInfo, FALSE);
	BOOL bResult = FALSE;
	POSITION pos = pInfo->protocols;
	while (pos)
	{
		CString& protocol = pInfo->protocols.GetNext(pos);
		if (protocol.CompareNoCase(m_pMediaInfo->GetProtocol()) == 0)
		{
			bResult = TRUE;
			break;
		}
	}
	return bResult;
}

BOOL CDSGraph::Kernel_CheckBytes(DSFilterInfo* pInfo, CString& strSubtype)
{
	// If no check bytes, mean match any, return TRUE directly
	if(pInfo->checkbytes.IsEmpty())
	{
		strSubtype.Empty();
		return TRUE;
	}

	HANDLE hFile= m_pMediaInfo->GetFileHandle();
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	BOOL bResult = FALSE;
	POSITION pos = pInfo->checkbytes.GetHeadPosition();
	while (pos)
	{
		CheckByteItem& item = pInfo->checkbytes.GetNext(pos);
		if (!item.checkbyte.IsEmpty())
		{
			if (CheckBytes(hFile, item.checkbyte))
			{
				strSubtype = item.subtype;
				bResult = TRUE;
				break;
			}
		}
	}
	return bResult;
}

bool CDSGraph::CheckBytes(HANDLE hFile, CString chkbytes)
{
	CAtlList<CString> sl;
	Explode(chkbytes, sl, ',');

	if (sl.GetCount() < 4)
	{
		return false;
	}

	ASSERT(!(sl.GetCount() & 3));

	LARGE_INTEGER size = {0, 0};
	GetFileSizeEx(hFile, &size);

	while (sl.GetCount() >= 4)
	{
		CString offsetstr = sl.RemoveHead();
		CString cbstr = sl.RemoveHead();
		CString maskstr = sl.RemoveHead();
		CString valstr = sl.RemoveHead();

		long cb = _ttol(cbstr);

		if (offsetstr.IsEmpty() || cbstr.IsEmpty()
			|| valstr.IsEmpty() || (valstr.GetLength() & 1)
			|| cb * 2 != valstr.GetLength())
		{
				return false;
		}

		LARGE_INTEGER offset;
		offset.QuadPart = _ttoi64(offsetstr);
		if (offset.QuadPart < 0)
		{
			offset.QuadPart = size.QuadPart - offset.QuadPart;
		}
		SetFilePointerEx(hFile, offset, &offset, FILE_BEGIN);

		// LAME
		while (maskstr.GetLength() < valstr.GetLength())
		{
			maskstr += 'F';
		}

		CAtlArray<BYTE> mask, val;
		CStringToBin(maskstr, mask);
		CStringToBin(valstr, val);

		for (size_t i = 0; i < val.GetCount(); i++)
		{
			BYTE b;
			DWORD r;
			if (!ReadFile(hFile, &b, 1, &r, NULL) || (b & mask[i]) != val[i])
			{
				return false;
			}
		}
	}
	return sl.IsEmpty();
}

HRESULT CDSGraph::Kernel_FileSourceFilterLoad(IBaseFilter* pFilter, LPCTSTR pcszSubtype)
{
	IFileSourceFilter * pFileSource = NULL;
	HRESULT hr = pFilter->QueryInterface(__uuidof(IFileSourceFilter), (void **)&pFileSource);
	if(SUCCEEDED(hr))
	{
		// Call source IFileSource::Load
		LPCWSTR pwszUrl = m_pMediaInfo->GetUrl();
		if(_tcslen(pcszSubtype) > 0)
		{
			g_utility.Log(_T("Call IFileSource::Load as MEDIATYPE_Stream,%s"), pcszSubtype);

			AM_MEDIA_TYPE mt;
			memset(&mt, 0, sizeof(mt));
			mt.majortype = MEDIATYPE_Stream;
			::StringToGuid(pcszSubtype, &mt.subtype);

			__try
			{
				hr = pFileSource->Load(pwszUrl, &mt);
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				hr = DISP_E_EXCEPTION;
				g_utility.Log(_T("pFileSource->Load raise exception"));
			}
		}
		else
		{
			g_utility.Log(_T("Call IFileSource::Load as MEDIATYPE_Stream,MEDIASUBTYPE_NULL"));
			hr = pFileSource->Load(pwszUrl, NULL);
		}
		pFileSource->Release();
	}
	return hr;
}

HRESULT CDSGraph::Kernel_RenderFilter(IBaseFilter * pFilter)
{
	this->EnterFunction();
	HRESULT hr = S_OK;

	TCHAR szName[MAX_PATH] = _T("Unknown");
	::GetFilterName(pFilter, szName);
	g_utility.Log(_T("%sRenderFilter => %s[%08x]"), m_strPrefix, szName, pFilter);

	int nPinOutCount = 0;
	int nPinOutConnected = 0;
	BeginEnumPins(pFilter, pEP, pPin)
	{
		if (S_OK == IsPinDirection(pPin, PINDIR_OUTPUT))
		{
			nPinOutCount++;
			if (S_OK != IsPinConnected(pPin))
			{
				g_utility.Log(_T("%sRenderFilter, %s[%08x], render pin %d [%08x]."),
					m_strPrefix, szName, pFilter, nPinOutCount, pPin);
				hr = this->Kernel_RenderPin(pPin);
				if (SUCCEEDED(hr))
				{
					g_utility.Log(_T("%sRenderFilter, %s[%08x], render pin %d [%08x] okay."),
						m_strPrefix, szName, pFilter, nPinOutCount, pPin);
					nPinOutConnected++;
				}
			}
			else
			{
				nPinOutConnected++;
			}
		}
	}
	EndEnumPins;

	g_utility.Log(_T("%sRenderFilter <= %s[%08x], %d / %d pins rendered, result=%08x"),
		m_strPrefix, szName, pFilter, nPinOutConnected, nPinOutCount, hr);

	this->LeaveFunction();
	return hr;
}

HRESULT CDSGraph::Kernel_RenderPin(IPin* pPinOut)
{
	this->EnterFunction();

	g_utility.Log(_T("%sRenderPin => [%08x]"), m_strPrefix, pPinOut);

	HRESULT result = VFW_E_CANNOT_RENDER;
	IEnumMediaTypes * pEMT = NULL;
	HRESULT hr = pPinOut->EnumMediaTypes(&pEMT);
	if(SUCCEEDED(hr) && pEMT != NULL)
	{
		const int kMaxMediaTypeCount = 128;		
		AM_MEDIA_TYPE * mts[kMaxMediaTypeCount];
		memset(&mts, 0, sizeof(mts));

		ULONG cFetched = 0;
		hr = pEMT->Next(kMaxMediaTypeCount, mts, &cFetched);
		if(hr == S_FALSE)	//Did not retrieve as many media types as requested.
		{
			if(cFetched > 0)
			{
				BOOL bVideo = (mts[0]->majortype == MEDIATYPE_Video);
				if(bVideo)
				{
					if(m_bVideoRendered)
					{
						g_utility.Log(_T("%sVideo stream is rendered, skip."), m_strPrefix); 
						result = S_OK;
					}
					else
					{
						hr = this->Kernel_RenderMediaTypes(pPinOut, mts, cFetched);
						if(hr == S_OK)
						{
							m_bVideoRendered = TRUE;
							result = S_OK;
						}
					}
				}
				else
				{
					hr = this->Kernel_RenderMediaTypes(pPinOut, mts, cFetched);
					if(hr == S_OK)
						result = S_OK;
				}
			}
			else
			{
				g_utility.Log(_T("%sRenderPin, IEnumMediaTypes cFetched=%d"), m_strPrefix, cFetched);
			}
		}
		else if(hr == S_OK)
		{
			g_utility.Log(_T("%sRenderPin, More media types not got."), m_strPrefix);
		}
		else
		{
			g_utility.Log(_T("%sRenderPin failed, hr=%08x."), m_strPrefix, hr);
		}

		if(cFetched > 0)
		{
			for(ULONG i=0; i<cFetched; i++)
				DeleteMediaType(mts[i]);
		}

		pEMT->Release();
	}

	CAtlArray<GUID> types;
	ExtractMediaTypes(pPinOut, types);

	POSITION pos = m_transform.GetHeadPosition();
	while (pos)
	{
		CFGFilter* pFGF = m_transform.GetNext(pos);
		if (pFGF->m_merit < MERIT_DO_USE || pFGF->CheckTypes(types, false))
		{
			fl.Insert(pFGF, 0, pFGF->CheckTypes(types, true), false);
		}
	}


	g_utility.Log(_T("%sRenderPin <= [%08x], result=%08x"), m_strPrefix, pPinOut, result);

	this->LeaveFunction();

	return result;
}

HRESULT CDSGraph::Kernel_RenderMediaTypes(IPin* pPinOut, AM_MEDIA_TYPE** pmts, int nMediaTypeCount)
{
	this->EnterFunction();

	HRESULT hr = VFW_E_CANNOT_RENDER;

	g_utility.Log(_T("%sRenderMediaTypes => [%08x]"), m_strPrefix, pPinOut);

	if (g_utility.HaveLog())
	{
		CString strMediaType;
		for(int i=0; i<nMediaTypeCount; i++)
		{
			if(MediaTypeToString(pmts[i], strMediaType))
				g_utility.Log(_T("%sMediaType: %s"), m_strPrefix, strMediaType);
		}
	}

// 	const int kMaxFilterCount = 64; // Max filter number 64
// 	IBaseFilter * pFilterList[kMaxFilterCount];
// 	ZeroMemory(pFilterList, sizeof(IBaseFilter*) * kMaxFilterCount);

	CInterfaceList<IBaseFilter*> pBFs;

	// 1. Try filters in the graph
	g_utility.Log(_T("%sRenderMediaType, try connect filter in graph."), m_strPrefix);
	{
		//int nFilterCount = 0;
		BeginEnumFilters(m_pFilterGraph, pEF, pBF)
		{
			if (GetFilterFromPin(pPinOut) == pBF)
			{
				continue;
			}
			pBFs.AddTail(pBF);
		}
		EndEnumFilters;

		IBaseFilter* pBF = NULL;
		CString strFilterName;
		POSITION pos = pBFs.GetHeadPosition();
		while (pos)
		{
			pBF = pBFs.GetNext(pos);
			::GetFilterName(pBF, strFilterName);

			hr = Kernel_ConnectDirect(pPinOut, pBF, NULL);
			if (SUCCEEDED(hr))
			{
				g_utility.Log(_T("%sRenderMediaType [%08x], connect filter [%08x]%s okay."),
					m_strPrefix, pPinOut, pBF, strFilterName);

				hr = this->Kernel_RenderFilter(pBF);
				if(SUCCEEDED(hr))
				{
					this->LeaveFunction();
					return hr;
				}
				else
				{
					g_utility.Log(_T("%sRenderFilter [%08x] failed, disconnect it."), m_strPrefix, pBF);
					HRESULT hr2 = pPinOut->Disconnect();
				}
			}
			else
			{
				g_utility.Log(_T("%sRenderMediaType [%08x], connect filter [%08x]%s failed."),
					m_strPrefix, pPinOut, pBF, strFilterName);
			}
		}
	}

	// 2. Try filters in the g_filters (the global filter table/list)
	g_utility.Log(_T("%sRenderMediaType, try connect filter in the global filter list."), m_strPrefix);
	{
		SFilterInfo* pInfo = NULL;
		int nInfoCount = g_filters.GetCount();
		for (int i=0; i<nInfoCount; i++)
		{
			//pInfo = m_pInfo[i];
			pInfo = g_filters.GetInfo(i);

			if (pInfo->filtertype == FT_SOURCE)
				continue;

			if (pInfo->merit <= MERIT_DO_NOT_USE)
				continue;

			EFilterType nType = pInfo->filtertype;
			BOOL bAudioFilter = 
				(nType == kFilterTypeAudioDecoder		||
				nType == kFilterTypeAudioEffect			||
				nType == kFilterTypeAudioRenderer
				);
			BOOL bVideoFilter = 
				(nType == kFilterTypeVideoDecoder		||
				nType == kFilterTypeVideoEffect			||
				nType == kFilterTypeVideoRenderer
				);

			if (pmts[0]->majortype == MEDIATYPE_Audio && !bAudioFilter)
				continue;
			if (pmts[0]->majortype == MEDIATYPE_Video && !bVideoFilter)
				continue;
			if ((pmts[0]->majortype == MEDIATYPE_Text || 
				pmts[0]->majortype == MEDIATYPE_Subtitle ||
				pmts[0]->majortype == MEDIATYPE_ScriptCommand)
				&& nType != kFilterTypeVideoEffect)
				continue;

			// add only one vsfilter or audioswitcher
			if (IsFilterExists(pInfo->clsid) &&
				(_tcsicmp(pInfo->clsid, sCLSID_VsFilter) == 0 ||
				_tcsicmp(pInfo->clsid, sCLSID_AudioSwitcher) == 0))
				continue;
			// skip EVRRender in WinXP OS
			if (_tcsicmp(pInfo->clsid, sCLSID_EVR) == 0 && g_utility.GetOS() != OS_VISTAWIN7)
				continue;

			BOOL bMatching = g_filters.IsSupportsMediaTypes(pInfo, pmts, nMediaTypeCount);
			if (!bMatching)
				continue;

			// Create source filter
			IBaseFilter* pFilter = NULL;
			hr = this->AddFilter(pInfo, NULL, 0, &pFilter);
			if(SUCCEEDED(hr))
			{
				BOOL bNeedRemove = FALSE;
				hr = Kernel_ConnectDirect(pPinOut, pFilter, NULL);
				if (SUCCEEDED(hr))
				{
					g_utility.Log(_T("%sRenderMediaType, %08x, connect filter %s[%08x] okay."),
						m_strPrefix, pPinOut, pInfo->name, pFilter);

					hr = Kernel_RenderFilter(pFilter);
					if (SUCCEEDED(hr))
					{
					}
					else
					{
						bNeedRemove = TRUE;
						g_utility.Log(_T("%sRender \"%s\" failed, disconnect and remove it."), m_strPrefix, pInfo->name);
						HRESULT hr2 = pPinOut->Disconnect();
					}
				}
				else
				{
					bNeedRemove = TRUE;
				}
				if (bNeedRemove)
				{
					RemoveFilter(pFilter, pInfo->clsid);
				}
			}

			if (SUCCEEDED(hr))
				break;
		}
	}

	this->LeaveFunction();
	return hr;
}

HRESULT CDSGraph::Kernel_ConnectDirect(IPin* pPinOut, IBaseFilter* pFilter, AM_MEDIA_TYPE* pMediaType)
{
	this->EnterFunction();

	CString strFilterName(_T("Unknown"));
	::GetFilterName(pFilter, strFilterName);
	g_utility.Log(_T("%sConnectDirect => %s[%08x]"), m_strPrefix, strFilterName, pFilter);

	if (S_OK != IsPinDirection(pPinOut, PINDIR_OUTPUT))
	{
		g_utility.Log(_T("%sPin[%08x] is not output pin."), m_strPrefix, pPinOut);
		this->LeaveFunction();
		return VFW_E_INVALID_DIRECTION;
	}

	BeginEnumPins(pFilter, pEP, pPin)
	{
		if (S_OK == IsPinDirection(pPin, PINDIR_INPUT)
			&& S_OK != IsPinConnected(pPin))
		{
			HRESULT hr = m_pFilterGraph->ConnectDirect(pPinOut, pPin, pMediaType);
			if (SUCCEEDED(hr))
			{
				AM_MEDIA_TYPE mt;
				HRESULT hr2 = pPinOut->ConnectionMediaType(&mt);
				if(hr2 == S_OK)
				{
					if (g_utility.HaveLog())
					{
						CString strMediaType;
						if(::MediaTypeToString(&mt, strMediaType))
							g_utility.Log(_T("%sConnectDirect Okay: %s"), m_strPrefix, strMediaType);

					}
					FreeMediaType(mt);
				}
				this->LeaveFunction();
				return hr;
			}
			else
			{
				g_utility.Log(_T("%sConnectToFilter, connect pin failed, hr=%08x"), m_strPrefix, hr);
			}
		}
	}
	EndEnumPins;

	g_utility.Log(_T("%sConnectDirect <= %s[%08x]"), m_strPrefix, strFilterName, pFilter);

	this->LeaveFunction();

	return VFW_E_CANNOT_CONNECT;
}

void CDSGraph::EnterFunction()
{
	++m_nCallLevel;
	m_strPrefix += PREFIX_STRING;
}

void CDSGraph::LeaveFunction()
{
	--m_nCallLevel;
	m_strPrefix = m_strPrefix.Mid(0, m_strPrefix.GetLength() - _tcslen(PREFIX_STRING));
}

void CDSGraph::ShowUsedFilters(void)
{
	g_utility.Log(_T("Used filter list:"));

	HRESULT hr;
	TCHAR szName[MAX_PATH];
	TCHAR szClsId[MAX_GUID_LENGTH];
	CLSID ClsId;
	int nCount = 0;

	BeginEnumFilters(m_pGraph, pEF, pBF)
	{
		nCount++;
		if(::GetFilterName(pBF, szName))
		{
			hr = pBF->GetClassID(&ClsId);
			if(SUCCEEDED(hr))
			{
				if(GuidToString(ClsId, szClsId))
					g_utility.Log(_T("[Filter %d], %s, %s"), nCount, szName, szClsId);
			}
		}
		pBF.Release();
	}
	EndEnumFilters
}


HRESULT CDSGraph::IsPinDirection(IPin* pPin, PIN_DIRECTION dir1)
{
	CAutoLock cAutoLock(this);

	CheckPointer(pPin, E_POINTER);

	PIN_DIRECTION dir2;
	if (FAILED(pPin->QueryDirection(&dir2)))
	{
		return E_FAIL;
	}

	return dir1 == dir2 ? S_OK : S_FALSE;
}

HRESULT CDSGraph::IsPinConnected(IPin* pPin)
{
	CAutoLock cAutoLock(this);

	CheckPointer(pPin, E_POINTER);

	CComPtr<IPin> pPinTo;
	return SUCCEEDED(pPin->ConnectedTo(&pPinTo)) && pPinTo ? S_OK : S_FALSE;
}
