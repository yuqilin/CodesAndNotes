
#include "stdafx.h"
#include "DSEngine.h"

#define PREFIX_STRING				_T("   ")


CDSEngine::CDSEngine(CPlayerCore* pPlayerCore)
	: CBaseEngine(pPlayerCore)
	, m_EngineType(ET_DIRECTSHOW)
{
	if (m_pPlayerCore != NULL)
	{
		m_DSGraph.Create(m_pPlayerCore->GetVideoWindow(),
			m_pPlayerCore->GetNotifyWindow(),
			this);
	}	
}

CDSEngine::~CDSEngine()
{
	Close();
}

HRESULT CDSEngine::Open(CMediaInfo* pMediaInfo)
{
	CheckPointer(pMediaInfo, E_INVALIDARG);
	m_pMediaInfo = pMediaInfo;
	HRESULT hr = m_Graph.OpenMedia(m_pMediaInfo);
	return hr;
}

HRESULT CDSEngine::Close(void)
{
	HRESULT hr = S_OK;

	hr = CloseMedia();

	return hr;
}

HRESULT	CDSEngine::Play(void)
{
	HRESULT hr = m_Graph.Run();	
	return hr;
}

HRESULT	CDSEngine::Stop(void)
{
	HRESULT hr = S_OK;

	hr = m_Graph.Stop();

	return hr;
}

HRESULT	CDSEngine::Pause(void)
{
	HRESULT hr = S_OK;

	hr = m_Graph.Pause();

	return hr;
}

// HRESULT	CDSEngine::GetPlayState()
// {
// 	HRESULT hr = S_OK;
// 	return hr;
// }

HRESULT	CDSEngine::GetPlayPos(LONG* pnPlayPos)
{
	HRESULT hr = S_OK;

	hr = m_Graph.GetPlayPos(pnPlayPos);

	LONGLONG llCurrent = 0, llStop = 0;
	IMediaSeeking * pSeeking = this->GetSeeking();
	if(pSeeking != NULL)
	{
		HRESULT hr = pSeeking->GetPositions(&llCurrent, &llStop);
		if(FAILED(hr))
			g_utility.Log(_T("pSeeking->GetPositions failed, hr=%08x"), hr);
	}


	return hr;
}

HRESULT	CDSEngine::SetPlayPos(LONG nPlayPos)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT	CDSEngine::GetVolume(LONG* pnVolume)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT	CDSEngine::SetVolume(LONG nVolume)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT CDSEngine::GetTotalDuration(LONG* pnDuration)
{
	HRESULT hr = S_OK;

	if (m_pGraph != NULL)
	{
		m_pGraph->GetTotalDuration();
	}

	return hr;
}

HRESULT	CDSEngine::GetVideoWidth(LONG* pnVideoWidth)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT	CDSEngine::GetVideoHeight(LONG* pnVideoHeight)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT	CDSEngine::OpenMedia()
{
	HRESULT hr = m_Graph.OpenMedia(m_pMediaInfo);

	return hr;
}

void CDSEngine::CloseMedia()
{
	
}

HRESULT	CDSEngine::RenderFile(BOOL bOpenChain)
{
	HRESULT hr = S_OK;

	const TCHAR * pcszExtension = m_pMediaInfo->GetExtension();
	if(pcszExtension == NULL || _tcslen(pcszExtension)<2)
	{
		return E_FAIL;
	}

	CDSFilters& filters = CPlayerCore::GetFilters();
	filters.OrderInfoByExtension(pcszExtension);

	// Call before render
	Kernel_BeforeRender();

	// Render media
	HRESULT hr = E_FAIL;
	if(bOpenChain)
	{
		LPCWSTR pwszUrl = m_pMediaInfo->GetUrl();
		if (m_pJFilterGraph != NULL)
		{
			hr = m_pJFilterGraph->RenderFile(pwszUrl, NULL);
		}
	}
	else
	{
		hr = Kernel_Render();
	}

	// Call after render
	Kernel_AfterRender(hr);
	if(FAILED(hr))
		return hr;

	// Show used filters
	ShowUsedFilters();

	return hr;
}

HRESULT CDSEngine::AddFilter(DSFilterInfo* pInfo,
							CAtlList<MediaTypeItem>& mts,
							IBaseFilter** ppFilter = NULL)
{
	HRESULT hr = S_OK;

	

	return hr;
}


BOOL CDSEngine::Kernel_CheckProtocol(DSFilterInfo* pInfo)
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

BOOL CDSEngine::Kernel_CheckBytes(DSFilterInfo* pInfo, CString& strSubtype)
{
	CheckPointer(pInfo, FALSE);
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

BOOL CDSEngine::CheckBytes(HANDLE hFile, CString& chkbytes)
{
	CAtlList<CString> sl;
	Explode(chkbytes, sl, ',');

	if (sl.GetCount() < 4)
	{
		return FALSE;
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
			return FALSE;
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
				return FALSE;
			}
		}
	}
	return sl.IsEmpty();
}

HRESULT CDSEngine::Kernel_FileSourceFilterLoad(IBaseFilter* pBF, const CString& strSubtype)
{
	CheckPointer(pBF, E_POINTER);

	HRESULT hr = S_OK;
	CComQIPtr<IFileSourceFilter> pFSF = pBF;
	if(pFSF != NULL)
	{
		LPCWSTR pwszUrl = m_pMediaInfo->GetUrl();
		if(!strSubtype.IsEmpty())
		{
			g_utility.Log(_T("Call IFileSource::Load as MEDIATYPE_Stream,%s"), pcszSubtype);

			AM_MEDIA_TYPE mt;
			memset(&mt, 0, sizeof(mt));
			mt.majortype = MEDIATYPE_Stream;
			GUIDFromCString(strSubtype, mt.subtype);

			__try
			{
				hr = pFSF->Load(pwszUrl, &mt);
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
			hr = pFSF->Load(pwszUrl, NULL);
		}
	}
	else
	{
		hr = E_NOINTERFACE;
	}
	return hr;
}

HRESULT CDSEngine::Kernel_RenderFilter(IBaseFilter* pFilter)
{
	this->EnterFunction();
	HRESULT hr = S_OK;

	CString strName = _T("Unknown");
	::GetFilterName(pFilter, strName);
	g_utility.Log(_T("%sRenderFilter => %s[%08x]"), m_strPrefix, strName, pFilter);

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
		m_strPrefix, strName, pFilter, nPinOutConnected, nPinOutCount, hr);

	this->LeaveFunction();
	return hr;
}

HRESULT CDSEngine::IsPinDirection(IPin* pPin, PIN_DIRECTION dir1)
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

HRESULT CDSEngine::IsPinConnected(IPin* pPin)
{
	CAutoLock cAutoLock(this);

	CheckPointer(pPin, E_POINTER);

	CComPtr<IPin> pPinTo;
	return SUCCEEDED(pPin->ConnectedTo(&pPinTo)) && pPinTo ? S_OK : S_FALSE;
}


