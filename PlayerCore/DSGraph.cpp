
#include "stdafx.h"
#include "DSGraph.h"

#define PREFIX_STRING	_T("   ")

CDSGraph::CDSGraph()
		: m_hVideoWindow(NULL)
		, m_hNotifyWindow(NULL)
		, m_pEngine(NULL)
		, m_pJFilterGraph(NULL)
{

}

CDSGraph::~CDSGraph()
{

}

HRESULT CDSGraph::Create(HWND hVideoWindow,
							   HWND hNotifyWindow,
							   CDSEngine* pEngine)
{
	CheckPointer(hVideoWindow, E_INVALIDARG);
	CheckPointer(hNotifyWindow, E_INVALIDARG);
	CheckPointer(pEngine, E_INVALIDARG);

	m_hVideoWindow = hVideoWindow;
	m_hNotifyWindow = hNotifyWindow;
	m_pEngine = pEngine;

	return S_OK;
}

void CDSGraph::Destory()
{
	m_hVideoWindow = NULL;
	m_hNotifyWindow = NULL;
	m_pEngine = NULL;
}

HRESULT	CDSGraph::OpenMedia(CMediaInfo* pMediaInfo)
{
	HRESULT hr = S_OK;

	const CPlayerSettings& s = CPlayerCore::GetPlayerSettings();
	hr = this->Render(s.m_bRenderOpenChain);

	return hr;
}

void CDSGraph::CloseMedia()
{

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
		if (m_pJFilterGraph)
			hr = m_pJFilterGraph->RenderFile(pwszUrl, NULL);
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

HRESULT CDSGraph::AddFilter(DSFilterInfo* pInfo,
							CAtlList<MediaTypeItem>& mts,
							IBaseFilter** ppBF = NULL);
{
	HRESULT hr = S_OK;

	CInterfaceList<IUnknown, &IID_IUnknown> pUnks;
	hr = CDSFilters::CreateFilter(pInfo, ppBF, pUnks);
	
	

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

HRESULT	CDSGraph::RemoveFilter(IBaseFilter * pBF, const TCHAR * pcszClsId = NULL)
{
	HRESULT hr = S_OK;

	if(m_pJFilterGraph == NULL || pBF == NULL)
		return E_INVALIDARG;

	// Get class id
	HRESULT hr = E_FAIL;
	const TCHAR * pClsId = NULL;
	if(pcszClsId == NULL)
	{
		CLSID ClsId;
		TCHAR szClsId[MAX_GUID_LENGTH];
		hr = pBF->GetClassID(&ClsId);
		if(FAILED(hr))
		{
			g_utility.Log(_T("%sRemoveFilter: get class id failed, hr=%08x"), m_szPrefix, hr);
			return hr;
		}
		::GuidToString(ClsId, szClsId);
		pClsId = szClsId;
	}
	else
		pClsId = pcszClsId;
	if(g_utility.HaveLog())
		g_utility.Log(_T("%sRemoveFilter: %s"), m_szPrefix, pClsId);

	// Remove filter
	hr = m_pJFilterGraph->RemoveFilter(pBF);
	if(FAILED(hr))
	{
		g_utility.Log(_T("%sRemoveFilter: m_pGraph->RemoveFilter failed, hr=%08x"), m_szPrefix, hr);
		return hr;
	}

	return S_OK;
}

void CDSGraph::RemoveAllFilter(void)
{
	g_utility.Log(_T("CDirectShowGraph::RemoveAllFilter Begin."));

	// Enumerate the filters in the graph.
	int nRefCount = 0;
	IEnumFilters *pEnum = NULL;
	HRESULT hr = m_pJFilterGraph->EnumFilters(&pEnum);
	if(SUCCEEDED(hr))
	{
		int nCounter = 0;
		IBaseFilter * pFilter = NULL;
		while (S_OK == pEnum->Next(1, &pFilter, NULL))
		{
			// Remove the filter.
			this->RemoveFilter(pFilter);

			// Reset the enumerator.
			pEnum->Reset();
			nRefCount = pFilter->Release();
		}
		pEnum->Release();
	}

	g_utility.Log(_T("CDirectShowGraph::RemoveAllFilter End."));
}

// void CDSGraph::RemoveChain(IPin * pOutputPin)
// {
// 	
// }

//-------------------------------------------------------------------------
// Kernel Functions
//-------------------------------------------------------------------------
void CDSGraph::Kernel_SortInfoByExtension()
{
	LPCTSTR pcszExtension = m_pMediaInfo->GetExtension();
	if(pcszExtension == NULL)
		return;

	g_utility.Log(_T("Kernel_SortInfoByExtName: %s"), pcszExtension);

	const CDSFilters& filters = CPlayerCore::GetFilters();
	filters.SortInfoByExtension(pcszExtension);
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
	HRESULT hr = S_OK;

	struct MatchingFilterItem
	{
		DSFilterInfo*		info;
		CString				subtype;
	};
	CAtlList<MatchingFilterItem> MatchingList;

	CAtlList<DSFilterInfo*>& InfoList = CPlayerCore::GetFilters().m_source;

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

	CAtlList<MediaTypeItem> mts;
	ExtractMediaTypes(pPinOut, mts);

	Kernel_RenderMediaTypes(pPinOut, mts);

	


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

	g_utility.Log(_T("%sRenderPin <= [%08x], result=%08x"), m_strPrefix, pPinOut, result);

	this->LeaveFunction();

	return result;
}

HRESULT CDSGraph::Kernel_RenderMediaTypes(IPin* pPinOut, CAtlList<MediaTypeItem>& mts)
{
	HRESULT result = VFW_E_CANNOT_RENDER;

	// 1. Try filters in the graph
	g_utility.Log(_T("%sRenderMediaType, try connect filter in graph."), m_strPrefix);
	{
		CInterfaceList<IBaseFilter*> pBFs;
		//int nFilterCount = 0;
		BeginEnumFilters(m_pJFilterGraph, pEF, pBF)
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

			HRESULT hr = Kernel_ConnectDirect(pPinOut, pBF, NULL);
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
	BOOL bAudioMediaType = FALSE;
	BOOL bVideoMediaType = FALSE;
	POSITION pos2 = mts.GetHeadPosition();
	if (pos2 != NULL && mts.GetAt(pos2).majortype == MEDIATYPE_Audio)
		bAudioMediaType = TRUE;
	if (pos2 != NULL && mts.GetAt(pos2).majortype == MEDIATYPE_Video)
		bVideoMediaType = TRUE;

	g_utility.Log(_T("%sRenderMediaType, try connect filter in the global filter list."), m_strPrefix);
	{
		DSFilterInfo* pInfo = NULL;
		CAtlList<DSFilterInfo*>& transforms = CPlayerCore::GetFilters().m_transform;
		POSITION pos = transforms.GetHeadPosition();
		while (pos)
		{
			DSFilterInfo* pInfo = transforms.GetNext(pos);

			if (pInfo->merit <= MERIT_DO_NOT_USE)
				continue;

			FILTER_TYPE type = pInfo->filtertype;

			BOOL bAudioFilter = 
				(type == FT_AUDIO_DECODER		||
				type == FT_AUDIO_EFFECT			||
				type == FT_AUDIO_ENCODER
				);
			BOOL bVideoFilter = 
				(type == FT_VIDEO_DECODER		||
				type == FT_VIDEO_EFFECT			||
				type == FT_VIDEO_ENCODER
				);

			if (bAudioMediaType && !bAudioFilter)
				continue;
			if (bVideoMediaType && !bVideoFilter)
				continue;

			// add only one vsfilter or audioswitcher
// 			if ((pInfo->clsid == CLSID_VsFilter ||
// 				pInfo->clsid == CLSID_AudioSwitcher == 0) &&
// 				FindFilter(pInfo->clsid, m_pJFilterGraph))
// 				continue;

			const CDSFilters& filters = CPlayerCore::GetFilters();
			if (filters.CheckTypes(pInfo, mts))
			{
				// Check we can add this filter to graph
				if(this->Kernel_CanAddFilter(pInfo->clsid))
				{
					// Create filter
					IBaseFilter* pBF = NULL;
					hr = this->AddFilter(pInfo, mts, &pBF);
					if(SUCCEEDED(hr))
					{
						// Connect to filter
						BOOL bNeedRemove = FALSE;
						HRESULT hr = Kernel_ConnectDirect(pPinOut, pBF, pInfo->clsid, NULL);
						if(hr == S_OK)
						{
							g_utility.Log(_T("%sRenderMediaType, %08x, connect filter %s[%08x] okay."),
								m_szPrefix, pPinOut, pInfo->name, pBF);

							// Render the filter
							hr = this->Kernel_RenderFilter(pBF);
							if(SUCCEEDED(hr))
							{
								result = S_OK;
							}
							else
							{
								g_utility.Log(_T("%sRender \"%s\" failed, disconnect and remove it."), m_szPrefix, pInfo->name);

								hr = pPinOut->Disconnect();
								if(hr != S_OK)
								{
									g_utility.Log(_T("%sDisconnect \"%s\" failed, hr=%08x."), m_szPrefix, pInfo->name, hr);
								}
								bNeedRemove = TRUE;
							}
						}
						else
						{
							g_utility.Log(_T("%sConnect to \"%s\" failed, remove it."), m_szPrefix, pInfo->name);
							bNeedRemove = TRUE;
						}

						if(bNeedRemove)
						{
							hr = this->RemoveFilter(pBF, pInfo->clsid);
							if(FAILED(hr))
							{
								g_utility.Log(_T("%sRemoveFilter \"%s\" failed, hr=%08x."), m_szPrefix, pInfo->name, hr);
							}
						}

						// We render succeeded
						if(result == S_OK)
							break;
					}
				}
			}
		}
	}

	return result;
}

HRESULT CDSGraph::Kernel_ConnectDirect(IPin* pPinOut, IBaseFilter* pBF, AM_MEDIA_TYPE* pMediaType)
{
	this->EnterFunction();

	CString strFilterName(_T("Unknown"));
	::GetFilterName(pBF, strFilterName);
	g_utility.Log(_T("%sConnectDirect => %s[%08x]"), m_strPrefix, strFilterName, pBF);

	if (S_OK != IsPinDirection(pPinOut, PINDIR_OUTPUT))
	{
		g_utility.Log(_T("%sPin[%08x] is not output pin."), m_strPrefix, pPinOut);
		this->LeaveFunction();
		return VFW_E_INVALID_DIRECTION;
	}

	BeginEnumPins(pBF, pEP, pPin)
	{
		if (S_OK == IsPinDirection(pPin, PINDIR_INPUT)
			&& S_OK != IsPinConnected(pPin))
		{
			if (m_pJFilterGraph != NULL)
			{
				HRESULT hr = m_pJFilterGraph->ConnectDirect(pPinOut, pPin, pMediaType);
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
	}
	EndEnumPins;

	g_utility.Log(_T("%sConnectDirect <= %s[%08x]"), m_strPrefix, strFilterName, pBF);

	this->LeaveFunction();

	return VFW_E_CANNOT_CONNECT;
}

BOOL CDSGraph::Kernel_CanAddFilter(DSFilterInfo* pInfo)
{
	BOOL bCanAdded = TRUE;

	// only one vsfilter or audioswitcher
	if ((pInfo->clsid == CLSID_VsFilter ||
				pInfo->clsid == CLSID_AudioSwitcher == 0) &&
				FindFilter(pInfo->clsid, m_pJFilterGraph))
	{
		bCanAdded = FALSE;
	}

	return bCanAdded;
}

void CDSGraph::Kernel_ResetGraph(HRESULT hrLast)
{
	g_utility.Log(_T("Kernel_ResetGraph begin"));

	////Save params
	//CMediaInfo * pMediaInfo = m_pMediaInfo;

	//// Reset graph
	//this->Kernel_AfterRender(hrLast);
	//this->Destroy();
	//this->Create(pMediaInfo);
	//this->Kernel_BeforeRender();

	g_utility.Log(_T("Kernel_ResetGraph end"));
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

void CDSGraph::Kernel_BeforeRender(void)
{
	
}

void CDSGraph::Kernel_AfterRender(HRESULT hrResult)
{
	if(SUCCEEDED(hrResult))
	{
		// Apply start stop position
		if(m_nStartPosition > 0 || m_nStopPosition > 0)
		{
			IMediaSeeking * pSeeking = this->GetSeeking();
			if(pSeeking != NULL)
			{
				LONGLONG llCurrent = LONGLONG(m_nStartPosition) * MSNS;
				LONGLONG llStop = LONGLONG(m_nStopPosition) * MSNS;

				HRESULT hr = pSeeking->SetPositions
					(&llCurrent, AM_SEEKING_AbsolutePositioning, &llStop, AM_SEEKING_AbsolutePositioning);
				if(FAILED(hr))
					g_utility.Log(_T("SetPositions for applay start stop position failed, hr=%08x"), hr);
			}
		}

		// Remove unused filter
		this->Kernel_RemoveUnusedFilters();
	}

	// Call control
	m_pControl->OnAfterRender(hrResult);
}

BOOL CDSGraph::OnSpecialParse(CMediaInfo * pMediaInfo,
							   DSFilterInfo** ppInfo,
							   CString& strSubtype)
{
	BOOL bOK = FALSE;
	if(!bOK)
		bOK = this->SpecialParse_Extension(pMediaInfo, ppInfo, strSubtype);
	if(!bOK)
		bOK = this->SpecialParse_Header(pMediaInfo, ppInfo, strSubtype);
	return bOK;
}

BOOL CDSGraph::SpecialParse_Extension(CMediaInfo* pMediaInfo, DSFilterInfo** ppInfo, CString& strSubtype)
{
	CheckPointer(ppInfo, FALSE);

	// Extname parser list
	typedef BOOL (*ParseExtname)(CMediaInfo * pMediaInfo);
	struct PARSEEXTNAME_ITEM
	{
		const TCHAR *	extname;
		ParseExtname	parse;
		const TCHAR *	clsid;
	};
	PARSEEXTNAME_ITEM list[] =
	{
		{_T(".avs"), ParseExtname_AVS, _T("{D3588AB0-0781-11CE-B03A-0020AF0BA770}")},
		{_T(".asx"), ParseExtname_ASX, _T("{187463A0-5BB7-11D3-ACBE-0080C75E246E}")},
	};
	int list_count = _countof(list);

	// Loop parse list
	BOOL bResult = FALSE;
	const CDSFilters& filters = CPlayerCore::GetFilters();
	for(int i=0; i<list_count; i++)
	{
		if(lstrcmpi(pMediaInfo->GetExtname(), list[i].extname) == 0)
		{
			if(list[i].parse(pMediaInfo))
			{
				DSFilterInfo* pInfo = filters.FindFilterByClsid(list[i].clsid);
				if(pInfo != NULL)
				{
					g_utility.Log(_T("Add source by parse extname: \"%s\" => \"%s\""), list[i].extname, pInfo->name);
					*ppInfo = pInfo;
					strSubtype.Empty();
					bResult = TRUE;
				}
			}
			break;
		}
	}
	return bResult;
}

BOOL CDSGraph::SpecialParse_Header(CMediaInfo* pMediaInfo, DSFilterInfo** ppInfo, CString& strSubtype)
{
	const CDSFilters& filters = CPlayerCore::GetFilters();
	DSFilterInfo* pInfo = filters.FindFilterByClsid(CLSID_FileSourceAsync);
	if(pInfo == NULL)
		return FALSE;

	// Header paser list
	typedef BOOL (*ParseHeader)(CMediaInfo* pMediaInfo);
	struct PARSEHEADER_ITEM
	{
		ParseHeader		parse;
		const TCHAR *	subtype;
	};
	PARSEHEADER_ITEM list[] =
	{
		{ ParseHeader_TS,	_T("{E06D8023-DB46-11CF-B4D1-00805F6CBBEA}") }, //MEDIASUBTYPE_MPEG2_TRANSPORT
		// { ParseHeader_MP3,	_T("{E436EB87-524F-11CE-9F53-0020AF0BA770}") }, no splitter
	};
	int list_count = _countof(list);

	// Loop parse list
	BOOL bResult = FALSE;
	for(int i=0; i<list_count; i++)
	{
		if(list[i].parse(pMediaInfo))
		{
			g_utility.Log(_T("Add source by parse header: \"%s\""), list[i].subtype);
			*ppInfo = pInfo;
			strSubtype = list[i].subtype;
			bResult = TRUE;
			break;
		}
	}
	return bResult;
}

BOOL CDSGraph::ParseHeader_TS(CMediaInfo* pMediaInfo)
{
	// Check param
	if(pMediaInfo == NULL)
		return FALSE;

	MEMORY_DATA* pHeader = pMediaInfo->GetHeader();
	if(pHeader->size < 1024) // We need 1KB
		return FALSE;
	if(pHeader->data == NULL)
		return FALSE;

	// Find TS length
	int nTsStart = 0;
	int nTsLength = 0;
	int i=0;
	int nCount = 0;
	const int kMinMatchingCount = 3;
	const int kMaxMatchingCount = 10;
	for(i=0; i<pHeader->size && nCount<kMaxMatchingCount;)
	{
		if(pHeader->data[i] == 0x47)
		{
			nTsStart = i;
			if(pHeader->data[i+188] == 0x47)
			{
				ASSERT(nTsLength == 188 || nTsLength == 0);
				nTsLength = 188; // TS
				nCount++;
			}
			else if(pHeader->data[i+192] == 0x47)
			{
				ASSERT(nTsLength == 192 || nTsLength == 0);
				nTsLength = 192; // M2TS
				nCount++;
			}
		}
		i += (nTsLength==0)?1:nTsLength;
	}

	BOOL bResult = nCount>kMinMatchingCount;

	g_utility.Log(_T("CDirectShowControl::ParseHeader_TS, bResult=%d, nTsLength=%d"), bResult, nTsLength);

	return bResult;
}


LONGLONG CDSGraph::GetCurrentPlayPos()
{
	HRESULT hr = S_OK;

	LONGLONG llCurrent = 0, llStop = 0;
	IMediaSeeking * pSeeking = NULL;
	if (m_pJFilterGraph != NULL)
	{
		HRESULT hr = m_pJFilterGraph->QueryInterface(IID_IMediaSeeking, (void **)&pSeeking);
		if (SUCCEEDED(hr))
		{
			hr = pSeeking->GetPositions(&llCurrent, &llStop);
			if (FAILED(hr))
			{
				DSLOG(_T("pSeeking->GetPositions failed, hr=%08x"), hr);
			}
		}
	}
	else
	{

	}
	return llCurrent;
}