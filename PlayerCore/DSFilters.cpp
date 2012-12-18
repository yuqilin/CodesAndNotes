
#include "stdafx.h"
#include "DSFilters.h"

//
struct FILTER_DEVICE_ITEM
{
	FILTER_DEVICE	device;
	const TCHAR *	name;
};

FILTER_DEVICE_ITEM g_FilterDeviceList[] = 
{
	{ FD_UNKNOWN,	_T("unknown") },
	{ FD_FILTER,	_T("filter") },
	{ FD_DMO,		_T("dmo") },
	{ FD_VFW,		_T("vfw") },
	{ FD_IMAGE,		_T("image") },
};
int g_FilterDeviceCount = _countof(g_FilterDeviceList);

//
struct FILTER_TYPE_ITEM
{
	FILTER_TYPE		type;
	const TCHAR *	name;
};
FILTER_TYPE_ITEM g_FilterTypeList[] = 
{
	{ FT_UNKNOWN,				_T("unknown") },
	{ FT_SOURCE,				_T("src") },
	{ FT_SPLITTER,				_T("splt") },
	{ FT_AUDIO_DECODER,			_T("adec") },
	{ FT_VIDEO_DECODER,			_T("vdec") },
	{ FT_AUDIO_EFFECT,			_T("aeffect") },
	{ FT_VIDEO_EFFECT,			_T("veffect") },
	{ FT_AUDIO_RENDERER,		_T("arender") },
	{ FT_VIDEO_RENDERER,		_T("vrender") },
	{ FT_NULL_RENDERER,			_T("nullrender") },
	{ FT_AUDIO_ENCODER,			_T("aenc") },
	{ FT_VIDEO_ENCODER,			_T("venc") },
	{ FT_MUXER,					_T("muxer") },
	{ FT_WRITER,				_T("writer") },
};
int g_FilterTypeCount = _countof(g_FilterTypeList);

//////////////////////////////////////////////////////////////////////////
CDSFilters::CDSFilters()
{
}

CDSFilters::~CDSFilters()
{
	
}

HRESULT	CDSFilters::LoadCodecsInfo()
{
	HRESULT hr = E_FAIL;

	TCHAR * pText = ::LoadResourceText(_T("TEXT"), IDR_FILTERINFO);
	if(pText != NULL)
	{
		hr = this->ParseInfoBuffer(pText);
		::FreeResourceText(pText);
	}

	return hr;
}

void CDSFilters::UnLoadCodecsInfo()
{
	DSFilterInfo* pInfo = NULL;
	POSITION pos = m_source.GetHeadPosition();
	while (pos)
	{
		pInfo = m_source.GetNext(pos);
		SAFE_DELETE(pInfo);
	}

	pos = m_transform.GetHeadPosition();
	while (pos)
	{
		pInfo = m_transform.GetNext(pos);
		SAFE_DELETE(pInfo);
	}

	m_source.RemoveAll();
	m_transform.RemoveAll();
}

HRESULT CDSFilters::SetCodecsPath(CString& strPath)
{
	if (m_strCodecsPath.IsEmpty())
	{
		DWORD dwStart = ::timeGetTime();
		this->LoadCodecsInfo();
		DWORD dwTime = ::timeGetTime() - dwStart;
		g_utility.Log(_T("CDSFilters::SetCodecsPath, LoadCodecsInfo cost %d ms"), dwTime);
	}

	if (m_strCodecsPath.CompareNoCase(strPath.Trim()) == 0)
	{
		g_utility.Log(_T("CDSFilters::SetCodecsPath, codecs path not changed"));
		return S_OK;
	}
	if (!::PathFileExists(strPath))
	{
		g_utility.Log(_T("CDSFilters::SetCodecsPath, set path not exists: %s"), strPath);
		return S_OK;
	}
	{
		ReplaceCodecsPath(m_source, strPath);

		ReplaceCodecsPath(m_transform, strPath);

		m_strCodecsPath = strPath;
	}

	return S_OK;
}

HRESULT CDSFilters::ReplaceCodecsPath(CAtlList<DSFilterInfo*>& InfoList, CString& strNewPath)
{
	DSFilterInfo* pInfo = NULL;
	POSITION pos = InfoList.GetHeadPosition();
	while (pos)
	{
		pInfo = InfoList.GetNext(pos);
		if (pInfo != NULL)
		{
			pInfo->path.Replace(m_strCodecsPath, strNewPath);

			POSITION pos2 = pInfo->depends.GetHeadPosition();
			while (pos2)
			{
				CString& depend = pInfo->depends.GetNext(pos2);
				depend.Replace(m_strCodecsPath, strNewPath);
			}

			pos2 = pInfo->preloads.GetHeadPosition();
			while (pos2)
			{
				CString& preload = pInfo->preloads.GetNext(pos2);
				preload.Replace(m_strCodecsPath, strNewPath);
			}
		}
	}
	
	return S_OK;
}

LPCTSTR CDSFilters::GetCodecsPath()
{
	return m_strCodecsPath;
}

HRESULT CDSFilters::ParseInfoBuffer(TCHAR* pszBuffer)
{
	HRESULT hr = S_OK;

	rapidxml::xml_document<TCHAR> doc;

	try
	{
		doc.parse<0>(pszBuffer);
	}
	catch(rapidxml::parse_error& error)
	{
		CString strError(error.what());
		g_utility.Log(_T("RapidXml got parse error:%s"), strError);
		return FALSE;
	}

	rapidxml::xml_node<TCHAR>* node = doc.first_node();
	if (node)
		node = node->first_node(_T("filter"));
	for(; node!=NULL; node=node->next_sibling())
	{
		if (_tcsicmp(node->name(), _T("filter")) != 0)
		{
			continue;
		}

		DSFilterInfo* pInfo  = new DSFilterInfo();
		if (pInfo == NULL)
		{
			hr = E_OUTOFMEMORY;
			break;
		}

		rapidxml::xml_attribute<TCHAR>* attr = node->first_attribute();
		for(; attr!=NULL; attr=attr->next_attribute())
		{
			SetFilterInfo(pInfo, attr->name(), attr->value());
		}

		rapidxml::xml_node<TCHAR>* subnode = node->first_node();
		for (; subnode!=NULL; subnode=subnode->next_sibling())
		{
			SetFilterInfo(pInfo, subnode);
		}

		if (pInfo->filtertype == FT_SOURCE)
		{
			m_source.AddTail(pInfo);
		}
		else
		{
			m_transform.AddTail(pInfo);
		}
	}
	return hr;
}

HRESULT	CDSFilters::SetFilterInfo(DSFilterInfo* pInfo, LPCTSTR pcszKey, LPCTSTR pcszValue)
{
	CheckPointer(pcszKey, E_POINTER);
	CheckPointer(pcszValue, E_POINTER);

	HRESULT hr = S_OK;

	if(_tcsicmp(pcszKey, _T("name")) == 0)
	{
		pInfo->name = pcszValue;
	}
	else if(_tcsicmp(pcszKey, _T("pathflag")) == 0)
	{
		pInfo->pathflag = pcszValue;
	}
	else if(_tcsicmp(pcszKey, _T("path")) == 0)
	{
		pInfo->path = pcszValue;
	}
	else if(_tcsicmp(pcszKey, _T("clsid")) == 0)
	{
		pInfo->clsid = pcszValue;
	}
	else if(_tcsicmp(pcszKey, _T("device")) == 0)
	{
		this->ParseFilterDevice(pInfo, pcszValue);
	}
	else if(_tcsicmp(pcszKey, _T("filtertype")) == 0)
	{
		this->ParseFilterType(pInfo, pcszValue);
	}
	else if(_tcsicmp(pcszKey, _T("devicedata")) == 0)
	{
		pInfo->devicedata = pcszValue;
	}
	else if(_tcsicmp(pcszKey, _T("merit")) == 0)
	{
		pInfo->merit = _tcstoul(pcszValue, NULL, 16);
	}
	else if(_tcsicmp(pcszKey, _T("checkbyte")) == 0)
	{
		this->ParseCheckByte(pInfo, pcszValue);
	}
	else
	{
		g_utility.Log(_T("Unknown filter info item:%s"), pcszKey);
		assert(0);		
		hr = E_INVALIDARG;
	}

	return hr;
}

HRESULT	CDSFilters::SetFilterInfo(DSFilterInfo* pInfo, rapidxml::xml_node<TCHAR>* node)
{
	CheckPointer(node, E_POINTER);

	HRESULT hr = S_OK;

	const TCHAR* pcszKey = node->name();

	if (_tcsicmp(pcszKey, _T("protocol")) == 0)
	{
		rapidxml::xml_attribute<TCHAR>* attr=node->first_attribute(_T("value"));
		if (attr)
		{
			pInfo->protocols.AddTail(attr->value());
		}
	}
	else if (_tcsicmp(pcszKey, _T("extname")) == 0)
	{
		rapidxml::xml_attribute<TCHAR>* attr=node->first_attribute(_T("value"));
		if (attr)
		{
			pInfo->extensions.AddTail(attr->value());
		}
	}
	else if (_tcsicmp(pcszKey, _T("checkbyte")) == 0)
	{
		rapidxml::xml_attribute<TCHAR>* attr=node->first_attribute(_T("value"));
		if (attr)
		{
			ParseCheckByte(pInfo, attr->value());
		}
	}
	else if (_tcsicmp(pcszKey, _T("depend")) == 0)
	{
		rapidxml::xml_attribute<TCHAR>* attr=node->first_attribute(_T("dll"));
		if (attr)
		{
			CString dependpath;
			dependpath.Format(_T("%s\\%s"), m_strCodecsPath, attr->value());
			pInfo->depends.AddTail(dependpath);
		}
	}
	else if (_tcsicmp(pcszKey, _T("mediatype")) == 0)
	{
		rapidxml::xml_attribute<TCHAR>* major=node->first_attribute(_T("major"));
		rapidxml::xml_attribute<TCHAR>* sub=node->first_attribute(_T("sub"));
		if (major!=NULL && sub!=NULL)
		{
			MediaTypeItem mediatype;
			::CLSIDFromString(major->value(), &(mediatype.majortype));
			::CLSIDFromString(sub->value(), &(mediatype.subtype));
			pInfo->mediatypes.AddTail(mediatype);
		}
	}
	else if (_tcsicmp(pcszKey, _T("preload")) == 0)
	{
		rapidxml::xml_attribute<TCHAR>* attr=node->first_attribute(_T("dll"));
		if (attr)
		{
			CString preloadpath;
			preloadpath.Format(_T("%s\\%s"), m_strCodecsPath, attr->value());
			pInfo->preloads.AddTail(preloadpath);
		}
	}
	else
	{
		g_utility.Log(_T("Unknown filter info item:%s"), pcszKey);
		assert(0);
		hr = E_INVALIDARG;
	}

	return hr;
}

BOOL CDSFilters::ParseFilterDevice(DSFilterInfo* pInfo, const TCHAR* pcszFilterDevice)
{
	BOOL bResult = FALSE;
	for(int i=0; i<g_FilterDeviceCount; i++)
	{
		if(_tcsicmp(g_FilterDeviceList[i].name, pcszFilterDevice) == 0)
		{
			pInfo->device = g_FilterDeviceList[i].device;
			bResult = TRUE;
			break;
		}
	}
	return bResult;
}

BOOL CDSFilters::ParseFilterType(DSFilterInfo* pInfo, const TCHAR* pcszFilterType)
{
	BOOL bResult = FALSE;
	for(int i=0; i<g_FilterTypeCount; i++)
	{
		if(_tcsicmp(g_FilterTypeList[i].name, pcszFilterType) == 0)
		{
			pInfo->filtertype = g_FilterTypeList[i].type;
			bResult = TRUE;
			break;
		}
	}
	return bResult;
}

BOOL CDSFilters::ParseCheckByte(DSFilterInfo* pInfo, const TCHAR* pcszValue)
{
	CheckPointer(pcszValue, FALSE);

	CheckByteItem item;
	CString checkbytes(pcszValue);		
	int i = checkbytes.Find(_T("|"));
	if(i > 0)
	{
		item.checkbyte = checkbytes.Mid(0, i);
		item.subtype = checkbytes.Mid(i+1);
	}
	else
	{
		item.checkbyte = checkbytes;
	}
	pInfo->checkbytes.AddTail(item);
	return TRUE;
}

void CDSFilters::OrderInfoByExtension(const TCHAR* pcszExtension)
{
	OrderInfoByExtension(m_source, pcszExtension);
	OrderInfoByExtension(m_transform, pcszExtension);
}

void CDSFilters::OrderInfoByExtension(CAtlList<DSFilterInfo*>& InfoList, const TCHAR* pcszExtension)
{
	DSFilterInfo* pInfo = NULL;
	POSITION pos = InfoList.GetHeadPosition();
	while (pos)
	{
		BOOL bMatched = FALSE;
		pInfo = InfoList.GetAt(pos);
		if (pInfo != NULL)
		{
			POSITION pos2 = pInfo->extensions.GetHeadPosition();
			while (pos2)
			{
				CString& extension = pInfo->extensions.GetNext(pos2);
				if (extension.CompareNoCase(pcszExtension+1))
				{
					bMatched = TRUE;
					break;
				}
			}
			if (bMatched)
			{
				InfoList.MoveToHead(pos);
			}
		}
		InfoList.GetNext(pos);
	}
}

//////////////////////////////////////////////////////////////////////////
//
