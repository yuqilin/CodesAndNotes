
#include "stdafx.h"
#include "DShowFilters.h"

#include "detours/detours.h"
#pragma comment(lib, "detours.lib")

//////////////////////////////////////////////////////////////////////////

// PathFlag
#define PATHFLAG_PLAYER				_T("player")
#define PATHFLAG_CODECS				_T("codecs")
#define PATHFLAG_REGISTRY			_T("registry")
#define PATHFLAG_SYSTEM				_T("system")
#define PATHFLAG_PROGRAM_FILES		_T("programfiles")
#define PATHFLAG_COMMON_FILES		_T("commonfiles")
#define PATHFLAG_CURRENT_USER		_T("currentuser")
#define PATHFLAG_CURRENT_APPDATA	_T("currentappdata")
#define PATHFLAG_ALL_USERS			_T("allusers")
#define PATHFLAG_ALL_APPDATA		_T("allappdata")

// FilterDeviceItem
struct FilterDeviceItem
{
	EFilterDevice	device;
	LPCTSTR			name;
};
FilterDeviceItem FilterDevices[] = 
{
	{ kFilterDeviceUnknown,	_T("unknown") },
	{ kFilterDeviceFilter,	_T("filter") },
	{ kFilterDeviceDmo,		_T("dmo") },
	{ kFilterDeviceVfw,		_T("vfw") },
	{ kFilterDeviceImage,	_T("image") },
};
static int nFilterDeviceCount = _countof(FilterDevices);

// FilterTypeItem
struct FilterTypeItem
{
	EFilterType		type;
	LPCTSTR			name;
};
FilterTypeItem FilterTypes[] = 
{
	{ kFilterTypeUnknown,				_T("unknown") },
	{ kFilterTypeSource,				_T("src") },
	{ kFilterTypeSplitter,				_T("splt") },
	{ kFilterTypeAudioDecoder,			_T("adec") },
	{ kFilterTypeVideoDecoder,			_T("vdec") },
	{ kFilterTypeAudioEffect,			_T("aeffect") },
	{ kFilterTypeVideoEffect,			_T("veffect") },
	{ kFilterTypeAudioRenderer,			_T("arender") },
	{ kFilterTypeVideoRenderer,			_T("vrender") },
	{ kFilterTypeNullRenderer,			_T("nullrender") },
	{ kFilterTypeAudioEncoder,			_T("aenc") },
	{ kFilterTypeVideoEncoder,			_T("venc") },
	{ kFilterTypeMuxer,					_T("muxer") },
	{ kFilterTypeWriter,				_T("writer") },
};
static int nFilterTypeCount = _countof(FilterTypes);

// InternalFilter
// typedef HRESULT (*TypeCreate)(IBaseFilter** ppFilter);
// struct InternalFilter
// {
// 	CLSID		clsid;
// 	TypeCreate	create;
// };
// InternalFilter InternalFilters[] = 
// {
// 	{__uuidof(CZPlayerSource), CZPlayerFilter::Create<CZPlayerSource>},
// 	{__uuidof(CZPlayerNullRenderer), CZPlayerFilter::Create<CZPlayerNullRenderer>},
// };
// static int nInternalFilterCount = _countof(InternalFilters);

// Caller
LPCTSTR Callers[] =
{
	_T("vsfilter.dll"),
	_T("ffdshow.dll"),
	_T("ffdshow.ax"),
	_T("splitter.ax"),
	_T("v3d.ax")
};
static int nCallerCount = _countof(Callers);

static BOOL g_bDisableNotifyIcon = FALSE;


// g_filters
CDShowFilters g_filters;

//////////////////////////////////////////////////////////////////////////
// CDShowFilters
//////////////////////////////////////////////////////////////////////////
DETOUR_TRAMPOLINE(HRESULT WINAPI Real_CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID * ppv), CoCreateInstance);
DETOUR_TRAMPOLINE(BOOL WINAPI Real_Shell_NotifyIconA(DWORD dwMessage, PNOTIFYICONDATAA lpData), Shell_NotifyIconA);
DETOUR_TRAMPOLINE(BOOL WINAPI Real_Shell_NotifyIconW(DWORD dwMessage, PNOTIFYICONDATAW lpData), Shell_NotifyIconW);


CDShowFilters::CDShowFilters()
{
	m_bInitialized = FALSE;
}

CDShowFilters::~CDShowFilters()
{
	UnInitialize();
}

HRESULT CDShowFilters::Initialize()
{
	if(m_bInitialized)
		return TRUE;
	m_bInitialized = TRUE;

	// Hook API
	DetourFunctionWithTrampoline((PBYTE)Real_CoCreateInstance, (PBYTE)MyCoCreateInstance);
	DetourFunctionWithTrampoline((PBYTE)Real_Shell_NotifyIconA, (PBYTE)MyShell_NotifyIconA);
	DetourFunctionWithTrampoline((PBYTE)Real_Shell_NotifyIconW, (PBYTE)MyShell_NotifyIconW);

	::InitializeCriticalSection(&m_csPlayCores);

	TCHAR szCodecsPath[MAX_PATH];
	wsprintf(szCodecsPath, _T("%s\\codecs"), g_utility.GetModulePath());
	if(!this->SetCodecsPath(szCodecsPath))
		return FALSE;
	return TRUE;
}

void CDShowFilters::UnInitialize(void)
{
	if (!m_bInitialized)
		return;

	this->UninstallAllVfwDrivers();
	this->ClearInfo();
	this->ClearPathFlags();

	::DeleteCriticalSection(&m_csPlayCores);

	// Unhook API
	DetourRemove((PBYTE)Real_CoCreateInstance, (PBYTE)MyCoCreateInstance);
	DetourRemove((PBYTE)Real_Shell_NotifyIconA, (PBYTE)MyShell_NotifyIconA);
	DetourRemove((PBYTE)Real_Shell_NotifyIconW, (PBYTE)MyShell_NotifyIconW);

	m_bInitialized = FALSE;
}

LPCTSTR CDShowFilters::GetCodecsPath(void)
{
	return m_strCodecsPath;
}

BOOL CDShowFilters::SetCodecsPath(LPCTSTR pcszPath)
{
	////////////////////////////
	// Check & create directory
	////////////////////////////
	if(!::IsDirectoryExists(pcszPath))
	{
		g_utility.Log(_T("CDShowFilters::SetCodecsPath %s not exist"));
		return FALSE;
	}
	m_strCodecsPath = pcszPath;

	////////////////////////////
	// Reload all
	////////////////////////////

	// Start counter
	DWORD dwStart = ::timeGetTime();

	// Clear all
	this->UninstallAllVfwDrivers();
	this->ClearInfo();
	this->ClearPathFlags();

	// Load all
	this->SetPathFlags();
	this->LoadInfo();
	this->InitializeAllVfwDrivers();

	// Stop counter
	DWORD dwTime = ::timeGetTime() - dwStart;
	g_utility.Log(_T("Load filters info %d, cost %u ms"), m_InfoList.GetCount(), dwTime);
	return TRUE;
}


int CDShowFilters::GetCount(void)
{
	return m_InfoList.GetCount();
}

DShowFilterInfo* CDShowFilters::FindInfoByClsid(const TCHAR * pcszClsId)
{
	if(pcszClsId == NULL)
		return NULL;
	DShowFilterInfo* pInfo = NULL;
	POSITION pos = m_InfoList.GetHeadPosition();
	while (pos)
	{
		pInfo = m_InfoList.GetNext(pos);
		if (_tcsicmp(pInfo->clsid, pcszClsId) == 0)
		{
			return pInfo;
		}
	}
	return NULL;
}

DShowFilterInfo* CDShowFilters::FindInfoByName(const TCHAR * pcszName)
{
	if(pcszName == NULL)
		return NULL;
	POSITION pos = m_InfoList.GetHeadPosition();
	while (pos)
	{
		DShowFilterInfo* pInfo = m_InfoList.GetNext(pos);
		if (_tcsicmp(pInfo->name, pcszName) == 0)
		{
			return pInfo;
		}
	}
	return NULL;
}

BOOL CDShowFilters::IsSupportsMediaTypes(DShowFilterInfo* pInfo, AM_MEDIA_TYPE** pmts, int nMediaTypeCount)
{
	BOOL bExclude = FALSE;
	BOOL bSupports = FALSE;

	for(int i=0; i<nMediaTypeCount; i++)
	{
		AM_MEDIA_TYPE * pmt = pmts[i];

		// Check exclude
		POSITION pos = pInfo->excludetypes.GetHeadPosition();
		while (pos)
		{
			MediaTypeItem& item = pInfo->excludetypes.GetNext(pos);
			if (item.majortype == pmt->majortype &&
				item.subtype == pmt->subtype)
			{
				bExclude = TRUE;
				break;
			}
		}
		if(bExclude)
			break;

		// Check supports
		pos = pInfo->mediatypes.GetHeadPosition();
		while (pos)
		{
			MediaTypeItem& item= pInfo->mediatypes.GetNext(pos);
			if ((item.majortype == pmt->majortype || item->majortype == GUID_NULL) &&
				(item.subtype == pmt->subtype || item.subtype == GUID_NULL))
			{
				bSupports = TRUE;
				break;
			}
		}
		if (bSupports)
			break;
	}
	if(bExclude)
		return FALSE;
	return bSupports;
}

BOOL CDShowFilters::GetDisableList(TCHAR* pClsIdList, int nListSize)
{
	if(pClsIdList == NULL)
		return FALSE;

	DShowFilterInfo* pInfo = NULL;
	TCHAR* p = pClsIdList;
	TCHAR* pEnd = pClsIdList + nListSize;

	POSITION pos = m_InfoList.GetHeadPosition();
	while (pos)
	{
		if(p + MAX_GUID_LENGTH > pEnd)
			break;

		//pInfo = m_pInfo[i];
		pInfo = m_InfoList.GetNext(pos);
		if(pInfo->disabled)
		{
			if(p > pClsIdList)
			{
				*p = ';';
				p++;
			}

			_tcscpy_s(p, MAX_GUID_LENGTH, pInfo->clsid);
			p += _tcslen(pInfo->clsid);
		}
	}
	return TRUE;
}


BOOL CDShowFilters::SetDisableList(LPCTSTR pClsIdList)
{
	if(pClsIdList == NULL)
		return FALSE;

	DShowFilterInfo* pInfo = NULL;
	TCHAR szClsId[MAX_GUID_LENGTH];
	TCHAR * pList = (TCHAR *)pClsIdList;
	while( ::StringFromList(&pList, _T(";"), szClsId, _countof(szClsId)) )
	{
		DShowFilterInfo* pInfo = this->FindInfoByClsid(szClsId);
		if(pInfo != NULL)
		{
			g_utility.Log(_T("Disable codec: %s"), szClsId);
			pInfo->disabled = TRUE;
		}
	}
	return TRUE;
}


BOOL CDShowFilters::IsCodecExists(LPCTSTR pcszClsId)
{
	DShowFilterInfo* pInfo = this->FindInfoByClsid(pcszClsId);
	if(pInfo == NULL)
		return FALSE;
	return this->IsCodecExists(pInfo);
}


BOOL CDShowFilters::IsCodecExists(DShowFilterInfo* pInfo)
{
	CheckPointer(pInfo, FALSE);

	// Check dependences
	BOOL bFound = TRUE;
	POSITION pos = pInfo->dependences.GetHeadPosition();
	while (pos)
	{
		CString& dllpath = pInfo->dependences.GetNext(pos);
		if (!IsFileExists(dllpath))
		{
			bFound = FALSE;
			break;
		}
	}
	if (!bFound)
		return FALSE;

	// Check self
	if( (_tcsicmp(pInfo->pathflag, PATHFLAG_REGISTRY) != 0) &&
		(_tcsicmp(pInfo->pathflag, PATHFLAG_PLAYER) != 0) )
	{
		if(!::IsFileExists(pInfo->path))
		{
			g_utility.Log(_T("%s file not exists!"), pInfo->path);
			return FALSE;
		}
	}
	return TRUE;
}


HRESULT CDShowFilters::CreateCodec(DShowFilterInfo* pInfo,
								REFIID riid,
								LPUNKNOWN pUnkOuter,
								LPVOID * ppv)
{
	CheckPointer(pInfo, E_INVALIDARG);
	CheckPointer(pv, E_INVALIDARG);	

	HRESULT hr = this->NewCodec(pInfo, riid, pUnkOuter, ppv);
	return hr;
}


HRESULT CDShowFilters::CreateCodec(LPCTSTR pcszClsId,
								REFIID riid,
								LPUNKNOWN pUnkOuter,
								LPVOID* ppv)
{
	CheckPointer(pcszClsId, E_INVALIDARG);
	CheckPointer(ppv, E_INVALIDARG);

	DShowFilterInfo* pInfo = this->FindInfoByClsid(pcszClsId);
	if(pInfo == NULL)
		return E_INVALIDARG;

	HRESULT hr = this->CreateCodec(pInfo, riid, pUnkOuter, ppv);
	return hr;
}


HRESULT CDShowFilters::CreateFilter(DShowFilterInfo* pInfo, IBaseFilter** ppBaseFilter)
{
	CheckPointer(ppBaseFilter, E_INVALIDARG);
	CheckPointer(pInfo, E_INVALIDARG);

	HRESULT hr = this->NewCodec(pInfo, IID_IBaseFilter, NULL, (LPVOID *)ppBaseFilter);
	return hr;
}

HRESULT CDShowFilters::CreateFilter(LPCTSTR pcszClsId, IBaseFilter** ppBaseFilter)
{
	CheckPointer(pcszClsId, E_INVALIDARG);
	CheckPointer(ppBaseFilter, E_INVALIDARG);

	DShowFilterInfo* pInfo = this->FindInfoByClsid(pcszClsId);
	if(pInfo == NULL)
		return E_INVALIDARG;

	HRESULT hr = this->CreateFilter(pInfo, ppBaseFilter);
	return hr;
}

void CDShowFilters::AttachPlayCore(CZPlayerCore* pCore)
{
	CAutoCriticalSection acs(&m_csPlayCores, TRUE);

	// Add PlayCore
	DWORD dwThreadId = ::GetCurrentThreadId();
	m_shPlayCores.AddItem((HWND)dwThreadId, pCore);

	// Set current directory
	if(m_shPlayCores.GetItemCount() == 1)
		this->SetMyCurrentDirectory();
}


void CDShowFilters::DetachPlayCore(CZPlayerCore* pCore)
{
	CAutoCriticalSection acs(&m_csPlayCores, TRUE);

	// Delete PlayCore
	DWORD dwThreadId = ::GetCurrentThreadId();
	m_shPlayCores.DeleteItem((HWND)dwThreadId);

	// Restore current directory
	if(m_shPlayCores.GetItemCount() == 0)
		this->RestoreOldCurrentDirectory();
}

void CDShowFilters::SortInfoByExtension(LPCTSTR pcszExtension)
{
	if (pcszExtension == NULL || _tcslen(pcszExtension)<=0)
		return;
	BOOL bFound = FALSE;
	DShowFilterInfo* pInfo = NULL;
	POSITION pos = m_InfoList.GetHeadPosition();
	while (pos)
	{
		pInfo = m_InfoList.GetNext(pos);
		POSITION pos1 = pInfo->extensions.GetHeadPosition();
		while (pos1)
		{
			CString& extension = pInfo->extensions.GetNext(pos1);
			if (extension.CompareNoCase(pcszExtension) == 0)
			{
				bFound = TRUE;
				break;
			}
		}
		
		if (bFound)
		{
			g_utility.Log(_T("Bring filter front: %s"), pInfo->name);
			this->BringInfoToTop(pInfo);
		}
	}

}

BOOL CDShowFilters::BringInfoToTop(DShowFilterInfo* pFilterInfo)
{
	DShowFilterInfo* pInfo = NULL;
 	POSITION pos = m_InfoList.GetHeadPosition();
 	BOOL bFound = FALSE;
 	while (pos)
 	{
 		pInfo = m_InfoList.GetAt(pos);
 		if (pInfo == pFilterInfo)
 		{
			m_InfoList.MoveToHead(pos);
 			bFound = TRUE;
 			break;
 		}
 		m_InfoList.GetNext(pos);
 	}
	return bFound;
}

//----------------------------------------- protected methods --------------------------------------//

HRESULT CDShowFilters::MyCoCreateInstance(REFCLSID rclsid,
									   LPUNKNOWN pUnkOuter,
									   DWORD dwClsContext,
									   REFIID riid,
									   LPVOID * ppv)
{
	TCHAR szClsId[MAX_PATH];
	::GuidToString(rclsid, szClsId);
	HRESULT hr = E_FAIL;

	// g_utility.Log(_T("Real_CoCreateInstance, clsid=%s"), szClsId);
	hr = Real_CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
	if(hr == REGDB_E_CLASSNOTREG && CLSCTX_INPROC_SERVER)
	{
		g_utility.Log(_T("%s not register, try create in filters list"), szClsId);
		hr = g_filters.CreateCodec(szClsId, riid, pUnkOuter, ppv);
		if(hr != S_OK)
		{
			g_utility.Log(_T("%s not register and not in filters list, try create in quartz.dll"), szClsId);

			TCHAR szPath[MAX_PATH];
			wsprintf(szPath, _T("%s\\quartz.dll"), g_utility.GetSystemPath());
			hr = g_filters.CreateInstanceFromFile(szPath, rclsid, riid, pUnkOuter, ppv);
			g_utility.Log(_T("%s create in quartz.dll, hr=%08x"), szClsId, hr);
		}
	}
	return hr;
}


BOOL CDShowFilters::MyShell_NotifyIconA(DWORD dwMessage, PNOTIFYICONDATAA lpData)
{
	g_utility.Log(_T("MyShell_NotifyIconA - dwMessage = %d"), dwMessage);

	BOOL bResult = FALSE;
	if((dwMessage == NIM_ADD))
	{
		TCHAR szCaller[MAX_PATH];
		int nResult = ::GetCaller(szCaller);
		if(nResult == 0)
		{
			BOOL bFind = FALSE;
			for(int i=0; i<caller_count; i++)
			{
				if(_tcsicmp(szCaller, caller_list[i]) == 0)
				{
					bFind = TRUE;
					break;
				}
			}

			if(bFind)
			{
				g_utility.Log(_T("MyShell_NotifyIconA, NIM_ADD is called, Is my watching module: %s"), szCaller);
				bResult = TRUE;
			}
			else
			{
				g_utility.Log(_T("MyShell_NotifyIconA, NIM_ADD is called, not my watching module: %s"), szCaller);
				bResult = Real_Shell_NotifyIconA(dwMessage, lpData);
			}
		}
		else
		{
			g_utility.Log(_T("MyShell_NotifyIconA, NIM_ADD is called, but GetCaller failed, result = %d."), nResult);

			if(g_bDisableNotifyIcon)
			{
				g_utility.Log(_T("Force disable NotifyIcon"));
				g_bDisableNotifyIcon = FALSE;
				bResult = TRUE;
			}
			else
				bResult = Real_Shell_NotifyIconA(dwMessage, lpData);
		}
	}
	else
		bResult = Real_Shell_NotifyIconA(dwMessage, lpData);
	return bResult;
}


BOOL CDShowFilters::MyShell_NotifyIconW(DWORD dwMessage, PNOTIFYICONDATAW lpData)
{
	g_utility.Log(_T("MyShell_NotifyIconW - dwMessage = %d"), dwMessage);

	BOOL bResult = FALSE;
	if((dwMessage == NIM_ADD))
	{
		TCHAR szCaller[MAX_PATH];
		int nResult = ::GetCaller(szCaller);
		if(nResult == 0)
		{
			BOOL bFind = FALSE;
			for(int i=0; i<nCallerCount; i++)
			{
				if(_tcsicmp(szCaller, Callers[i]) == 0)
				{
					bFind = TRUE;
					break;
				}
			}

			if(bFind)
			{
				g_utility.Log(_T("MyShell_NotifyIconW, NIM_ADD is called, Is my watching module: %s"), szCaller);
				bResult = TRUE;
			}
			else
			{
				g_utility.Log(_T("MyShell_NotifyIconW, NIM_ADD is called, not my watching module: %s"), szCaller);
				bResult = Real_Shell_NotifyIconW(dwMessage, lpData);
			}
		}
		else
		{
			g_utility.Log(_T("MyShell_NotifyIconW, NIM_ADD is called, but GetCaller failed, result = %d."), nResult);

			if(g_bDisableNotifyIcon)
			{
				g_utility.Log(_T("Force disable NotifyIcon"));
				g_bDisableNotifyIcon = FALSE;
				bResult = TRUE;
			}
			else
				bResult = Real_Shell_NotifyIconW(dwMessage, lpData);
		}
	}
	else
		bResult = Real_Shell_NotifyIconW(dwMessage, lpData);
	return bResult;
}


void CDShowFilters::SetMyCurrentDirectory(void)
{
	if(!m_strOldCurrentDirectory.IsEmpty())
		return;
	CString currentdir;
	DWORD dwResult = ::GetCurrentDirectory(MAX_PATH, currentdir.GetBufferSetLength(MAX_PATH));
	if(dwResult > 0)
	{
		m_strOldCurrentDirectory = currentdir;
		BOOL bOK = ::SetCurrentDirectory(m_strCodecsPath);
		g_utility.Log(_T("Change current directory, %s => %s, result = %d"), m_strOldCurrentDirectory, m_strCodecsPath, bOK);
	}
}


void CDShowFilters::RestoreOldCurrentDirectory(void)
{
	if(m_strOldCurrentDirectory.IsEmpty())
		return;
	CString currentdir;
	DWORD dwResult = ::GetCurrentDirectory(MAX_PATH, currentdir.GetBufferSetLength(MAX_PATH));
	if (dwResult > 0)
	{
		BOOL bOK = ::SetCurrentDirectory(m_strOldCurrentDirectory);
		g_utility.Log(_T("Restore current directory, %s => %s, result = %d"), currentdir, m_strOldCurrentDirectory, bOK);
		m_strOldCurrentDirectory.Empty();
	}
}


void CDShowFilters::ClearPathFlags(void)
{
	m_PathFlags.RemoveAll();
}


BOOL CDShowFilters::SetPathFlags(void)
{
	TCHAR szPath[MAX_PATH] = {0};

	// <codecs>
	this->AddPathFlag(PATHFLAG_CODECS, m_strCodecsPath);

	// <system>
	_tcscpy_s(szPath, g_utility.GetSystemPath());
	this->AddPathFlag(PATHFLAG_SYSTEM, szPath);
	TCHAR system_driver = szPath[0];

	// <programfiles>
	HRESULT hr = S_OK;
	hr = ::SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, 0, szPath);
	if(FAILED(hr))
		wsprintf(szPath, _T("%c:\\Program Files"), system_driver);
	this->AddPathFlag(PATHFLAG_PROGRAM_FILES, szPath);

	// <commonfiles>
	hr = ::SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES_COMMON, NULL, 0, szPath);
	if(FAILED(hr))
		wsprintf(szPath, _T("%c:\\Program Files\\Common Files"), system_driver);
	this->AddPathFlag(PATHFLAG_COMMON_FILES, szPath);

	// <currentuser>
	hr = ::SHGetFolderPath(NULL, 0x0028, NULL, 0, szPath);
	if(hr == S_OK)
		this->AddPathFlag(PATHFLAG_CURRENT_USER, szPath);
	else
		this->AddPathFlag(PATHFLAG_CURRENT_USER, _T("Error:"));

	// <currentappdata>
	hr = ::SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szPath);
	if(hr == S_OK)
		this->AddPathFlag(PATHFLAG_CURRENT_APPDATA, szPath);
	else
		this->AddPathFlag(PATHFLAG_CURRENT_APPDATA, _T("Error:"));

	// <alluser>
	hr = ::SHGetFolderPath(NULL, 0x0019, NULL, 0, szPath);
	if(hr == S_OK)
	{
		TCHAR * pEnd = szPath + lstrlen(szPath) - 1;
		while(pEnd > szPath && *pEnd != '\\') pEnd--;
		*pEnd = 0;

		this->AddPathFlag(PATHFLAG_ALL_USERS, szPath);
	}
	else
		this->AddPathFlag(PATHFLAG_ALL_USERS, _T("Error:"));

	// <allappdata>
	hr = ::SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szPath);
	if(hr == S_OK)
		this->AddPathFlag(PATHFLAG_ALL_APPDATA, szPath);
	else
		this->AddPathFlag(PATHFLAG_ALL_APPDATA, _T("Error:"));

	// Output all info
	g_utility.Log(_T("Path flags:"));
	POSITION pos = m_PathFlags.GetHeadPosition();
	int nIndex = 0;
	while (pos)
	{
		PathFlagItem& item = m_PathFlags.GetNext(pos);
		g_utility.Log(_T("%d, %s = %s"), ++nIndex, item.flag, item.path);
	}

	return TRUE;
}

BOOL CDShowFilters::AddPathFlag(LPCTSTR pcszFlag, LPCTSTR pcszPath)
{
	CheckPointer(pcszFlag, FALSE);
	CheckPointer(pcszPath, FALSE);

	PathFlagItem item;
	item.flag = pcszFlag;
	item.path = pcszPath;
	m_PathFlags.AddTail(item);

	return TRUE;
}


BOOL CDShowFilters::ExpandPathString(const TCHAR * pcszFlag, TCHAR * pszPath)
{
	// Find path flag and expand it
	BOOL bFind = FALSE;
	POSITION pos = m_PathFlags.GetHeadPosition();
	while (pos)
	{
		PathFlagItem& item = m_PathFlags.GetNext(pos);
		if (item.flag.CompareNoCase(pcszFlag) == 0)
		{
			CString fullpath;
			fullpath.Format(_T("%s\\%s"), item.path, pszPath);
			_tcscpy_s(pszPath, MAX_PATH, fullpath);
			bFind = TRUE;
			break;
		}
	}

	// If not matching, copy directly
	if(!bFind)
		_tcscpy_s(pszPath, MAX_PATH, pcszFlag);
	return TRUE;
}


void CDShowFilters::ClearInfo(void)
{
	DShowFilterInfo* pInfo = NULL;
	POSITION pos = m_InfoList.GetHeadPosition();
	while (pos)
	{
		pInfo = m_InfoList.GetNext(pos);
		if(pInfo != NULL)
		{
			delete pInfo;
		}
	}
	m_InfoList.RemoveAll();
}

BOOL CDShowFilters::LoadInfo(void)
{
	BOOL bResult = FALSE;
	TCHAR* pText = ::LoadResourceText(_T("TEXT"), IDR_FILTERINFO);
	if(pText != NULL)
	{
		bResult = this->ParseInfoBuffer(pText);
		::FreeResourceText(pText);
	}
	return bResult;
}

BOOL CDShowFilters::ParseInfoBuffer(LPCTSTR pcszInfoBuffer)
{
	BOOL bResult = TRUE;
	rapidxml::xml_document<TCHAR> doc;

	try
	{
		doc.parse<0>(pcszInfoBuffer);
	}
	catch(rapidxml::parse_error& error)
	{
		g_utility.Log(_T("RapidXml got parse error:%s"), error.what());
		return FALSE;
	}

	rapidxml::xml_node<TCHAR>* node = doc.first_node();
	if (node != NULL)
		node = node->first_node(_T("filter"));
	for(; node!=NULL; node=node->next_sibling())
	{
		if (_tcsicmp(node->name(), _T("filter")) != 0)
		{
			continue;
		}

		DShowFilterInfo* pInfo = NewInfo();
		rapidxml::xml_attribute<TCHAR>* attr=node->first_attribute();
		for(; attr!=NULL; attr=attr->next_attribute())
		{
			SetInfo(pInfo, attr->name(), attr->value());
		}

		rapidxml::xml_node<TCHAR>* subnode = node->first_node();
		for (; subnode!=NULL; subnode=subnode->next_sibling())
		{
			SetInfo(pInfo, subnode);
		}
	}

	DShowFilterInfo* pInfo = NULL;
	POSITION pos = m_InfoList.GetHeadPosition();
	while (pos)
	{
		pInfo = m_InfoList.GetNext(pos);
		if (pInfo)
		{
			if (!this->ExpandPathString(pInfo->pathflag, pInfo->path))
			{
				bResult = FALSE;
			}
		}
	}

	return bResult;
}

DShowFilterInfo* CDShowFilters::NewInfo(void)
{
	DShowFilterInfo* pInfo = new DShowFilterInfo;
	if(pInfo == NULL)
	{
		g_utility.Log(_T("CDShowFilters::NewInfo, new failed E_OUTOFMEMORY"));
		return NULL;
	}

	// Fill default value
	pInfo->name = _T("noname");
	pInfo->device = kFilterDeviceUnknown;
	pInfo->filtertype = kFilterTypeUnknown;
	pInfo->merit = MERIT_DO_NOT_USE;

	m_InfoList.AddTail(pInfo);

	return pInfo;
}

BOOL CDShowFilters::SetInfo(CFGFilter::FilterInfo* pInfo, LPCTSTR pcszKey, LPCTSTR pcszValue)
{
	CheckPointer(pInfo, FALSE);

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
		CheckByteItem item;
		CString value(pcszValue);		
		int i = value.Find(_T("|"));
		if(i > 0)
		{
			item.checkbyte = value.Mid(0, i);
			item.subtype = value.Mid(i+1);
		}
		else
		{
			item.checkbyte = value;
		}
		pInfo->checkbytes.AddTail(item);
	}
	else
	{
		g_utility.Log(_T("Unknown filter info item:%s"), pcszKey);
		assert(0);
		return FALSE;
	}
	return TRUE;
}

BOOL CDShowFilters::SetInfo(CFGFilter::FilterInfo* pInfo, rapidxml::xml_node<TCHAR>* node)
{
	CheckPointer(pInfo, FALSE);
	CheckPointer(node, FALSE);

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
			pInfo->checkbytes.AddTail(attr->value());
		}
	}
	else if (_tcsicmp(pcszKey, _T("depend")) == 0)
	{
		rapidxml::xml_attribute<TCHAR>* attr=node->first_attribute(_T("dll"));
		if (attr)
		{
			CString dependpath;
			dependpath.Format(_T("%s\\%s"), m_strCodecsPath, attr->value());
			pInfo->dependences.AddTail(dependpath);
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
			pInfo->preloaddlls.AddTail(preloadpath);
		}
	}
	else
	{
		g_utility.Log(_T("Unknown filter info item:%s"), pcszKey);
		assert(0);
		return FALSE;
	}

	return TRUE;
}


BOOL CDShowFilters::ParseFilterDevice(DShowFilterInfo* pInfo, LPCTSTR pcszFilterDevice)
{
	CheckPointer(pInfo, FALSE);
	BOOL bResult = FALSE;
	for(int i=0; i<nFilterDeviceCount; i++)
	{
		if(_tcsicmp(FilterDevices[i].name, pcszFilterDevice) == 0)
		{
			pInfo->device = FilterDevices[i].device;
			bResult = TRUE;
			break;
		}
	}
	return bResult;
}

BOOL CDShowFilters::ParseFilterType(DShowFilterInfo* pInfo, LPCTSTR pcszFilterType)
{
	CheckPointer(pInfo, FALSE);
	BOOL bResult = FALSE;
	for(int i=0; i<nFilterTypeCount; i++)
	{
		if(_tcsicmp(FilterTypes[i].name, pcszFilterType) == 0)
		{
			pInfo->filtertype = FilterTypes[i].type;
			bResult = TRUE;
			break;
		}
	}
	return bResult;
}

BYTE* CDShowFilters::BuildHexStringBytes(LPCTSTR pcszHexString)
{
	// Check param
	if(pcszHexString == NULL)
		return NULL;
	int nLength = lstrlen(pcszHexString);
	if(nLength < 2)
		return NULL;

	// Build
	int nCount = nLength / 2;
	BYTE * pBytes = new BYTE[nCount];
	if(pBytes == NULL)
		return NULL;

	TCHAR szByte[3];
	const TCHAR * p = pcszHexString;
	for(int i=0; i<nCount; i++)
	{
		memcpy(szByte, p, 2 * sizeof(TCHAR));
		szByte[2] = 0;
		pBytes[i] = (BYTE)_tcstoul(szByte, NULL, 16);
		p += 2;
	}
	return pBytes;
}

void CDShowFilters::OrderInfoByMerit(void)
{
	int nCount = m_InfoList.GetCount();
	for(int i=0; i<nCount-1; i++)
	{
		for(int j=0; j<nCount-i-1; j++)
		{
			POSITION pos1 = m_InfoList.FindIndex(j);
			POSITION pos2 = m_InfoList.FindIndex(j+1);
			DShowFilterInfo* pInfo1 = m_InfoList.GetAt(pos1);
			DShowFilterInfo* pInfo2 = m_InfoList.GetAt(pos2);
			if(pInfo1->merit < pInfo2->merit)
			{
				m_InfoList.SwapElements(pos1, pos2);
			}
		}
	}
}


HRESULT CDShowFilters::NewCodec(DShowFilterInfo* pInfo,
						  REFIID riid,
						  LPUNKNOWN pUnkOuter,
						  LPVOID * ppv)
{
	/////////////////////////
	// Check param
	/////////////////////////
	if(pInfo == NULL || ppv == NULL)
		return E_INVALIDARG;

	/////////////////////////
	// Check is disabled
	/////////////////////////
	if(pInfo->disabled)
	{
		g_utility.Log(_T("NewCodec, Codec \"%s\" is disabled."), pInfo->name);
		return E_INVALIDARG;
	}

	/////////////////////////
	// Check filter exists
	/////////////////////////
	if(!this->IsCodecExists(pInfo))
	{
		if (_tcsicmp(pInfo->pathflag, PATHFLAG_SYSTEM) == 0)
		{
			g_utility.Log(_T("NewCodec, System Codec \"%s\" is not exists"));
			return CO_E_DLLNOTFOUND;
		}
		g_utility.Log(_T("NewCodec, Codec \"%s\" is not exists, download it"), pInfo->name);
		if(!this->DownloadCodec(pInfo))
		{
			g_utility.Log(_T("NewCodec, Codec \"%s\" download failed."), pInfo->name);
			return CO_E_DLLNOTFOUND;
		}
	}

	/////////////////////////
	// Load preload dlls
	/////////////////////////
	this->LoadPreloadDlls(pInfo);

	/////////////////////////
	// Get codec path
	/////////////////////////
	BOOL bPlayerFilter = FALSE;
	TCHAR szDllPath[MAX_PATH];
	if(_tcsicmp(pInfo->pathflag, PATHFLAG_REGISTRY) == 0)
	{
		BOOL bOK = FALSE;
		TCHAR szSubKey[MAX_PATH];
		wsprintf(szSubKey, _T("CLSID\\%s\\InprocServer32"), pInfo->clsid);
		HKEY hKey = NULL;
		LONG lResult = ::RegOpenKey(HKEY_CLASSES_ROOT, szSubKey, &hKey);
		if(lResult == ERROR_SUCCESS)
		{
			DWORD dwType = REG_SZ;
			DWORD dwLength = sizeof(szDllPath);
			lResult = ::RegQueryValueEx(hKey, NULL, NULL, &dwType, (LPBYTE)szDllPath, &dwLength);
			if(lResult == ERROR_SUCCESS)
				bOK = TRUE;
			::RegCloseKey(hKey);
		}
		if(!bOK)
		{
			g_utility.Log(_T("NewCodec, Query DLL path from registry failed"));
			return REGDB_E_CLASSNOTREG;
		}
	}
	else if(_tcsicmp(pInfo->pathflag, PATHFLAG_PLAYER) == 0)
	{
		if(riid == IID_IBaseFilter)
		{
			_tcscpy_s(szDllPath, EMPTY_STRING);
			bPlayerFilter = TRUE;
		}
		else
		{
			g_utility.Log(_T("NewCodec, PlayerFilter only provide IBaseFilter interface!"));
			return E_INVALIDARG;
		}
	}
	else
	{
		_tcscpy_s(szDllPath, pInfo->path);
	}
	g_utility.Log(_T("NewCodec, name=%s, DLL=%s, clsid=%s"), pInfo->name, szDllPath, pInfo->clsid);

	/////////////////////////
	// Get CLSID
	/////////////////////////
	CLSID clsid;
	if(!::StringToGuid(pInfo->clsid, &clsid))
	{
		g_utility.Log(_T("NewCodec, StringToGuid failed"));
		return E_FAIL;
	}

	/////////////////////////
	// Create instance
	/////////////////////////
	HRESULT hr = E_FAIL;
	if(bPlayerFilter)
	{
		// Create player filter
		hr = this->CreatePlayerFilter(clsid, (IBaseFilter **)ppv);
		if(FAILED(hr))
		{
			g_utility.Log(_T("NewCodec, CreatePlayerFilter failed, hr=%08x"), hr);
			return hr;
		}
	}
	else
	{
		// Create instance from file
		hr = this->CreateInstanceFromFile(szDllPath, clsid, riid, pUnkOuter, ppv);
		if(FAILED(hr))
			return hr;
	}

	/////////////////////////
	// Config type data
	/////////////////////////
	hr = this->ConfigTypeData(*ppv, pInfo);
	if(hr != S_OK)
	{
		g_utility.Log(_T("NewCodec, config type data failed."));
		if (*ppv)
			( (IUnknown *)(*ppv) )->Release();
		return hr;
	}
	return hr;
}


HRESULT CDShowFilters::CreateInstanceFromFile(const TCHAR * pcszPath,
										REFCLSID rclsid,
										REFIID riid,
										IUnknown * pUnkOuter,
										LPVOID * ppv)
{
	// Output debug info
	g_utility.Log( _T("CreateInstanceFromFile, DLL=%s"), pcszPath);

	// Load the library
	const TCHAR * pcszFileName = ::GetFileNameFromPath(pcszPath);
	HMODULE hDll = ::GetModuleHandle(pcszFileName);
	if(hDll == NULL)
	{
		// Check download failed
		if(!::IsFileExists(pcszPath))
		{
			g_utility.Log( _T("CreateInstanceFromFile - DLL not exists."), ::GetLastError()); 
			return CO_E_DLLNOTFOUND;
		}

		// Load the library
		hDll = ::LoadLibraryEx(pcszPath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
				
		if(hDll == NULL)
		{
			// Load fail
			g_utility.Log( _T("CreateInstanceFromFile - Load DLL failed, Last error = %d."), ::GetLastError()); 
			return TYPE_E_CANTLOADLIBRARY;
		}
	}

	// Get pointer of DllGetClassObject
	typedef HRESULT (STDAPICALLTYPE *_pfnDllGetClassObject)(REFCLSID, REFIID, LPVOID*);
	_pfnDllGetClassObject pfnDllGetClassObject = (_pfnDllGetClassObject)
		GetProcAddress(hDll, "DllGetClassObject");
	if(pfnDllGetClassObject == NULL)
	{
		g_utility.Log( _T("CreateInstanceFromFile - GetProcAddress failed: %08x"), CO_E_ERRORINDLL); 
		return CO_E_ERRORINDLL;
	}

	// Call DllGetClassObject
	CComPtr<IClassFactory> spCF;
	HRESULT hr = (*pfnDllGetClassObject)(rclsid, IID_IClassFactory, (LPVOID*)&spCF);
	if(FAILED(hr))
	{
		g_utility.Log( _T("CreateInstanceFromFile - DllGetClassObject failed: %08x"), hr); 
		return hr;
	}

	// Check is create ffdshow instance, return FALSE to continue next step
	if(this->CreateFFDShowInstance(spCF, rclsid, riid, ppv, &hr))
	{
		if(SUCCEEDED(hr))
			g_utility.Log( _T("CreateInstanceFromFile - Create ffdshow instance okay!"));
		else
			g_utility.Log( _T("CreateInstanceFromFile - Create ffdshow instance failed: %08x"), hr);
		return hr;
	}

	// Create instance
	hr = spCF->CreateInstance(pUnkOuter, riid, ppv);
	if(FAILED(hr))
	{
		g_utility.Log( _T("CreateInstanceFromFile - CreateInstance failed: %08x"), hr);
		return hr;
	}

	// Create completed
	g_utility.Log( _T("CreateInstanceFromFile - all okay!"));
	return S_OK;
}



HRESULT CDShowFilters::CreateInternalFilter(REFCLSID rclsid, IBaseFilter** ppFilter)
{
	HRESULT hr = E_NOTIMPL;
	return hr;
}



BOOL CDShowFilters::CreateFFDShowInstance(IClassFactory * pCF,
										  REFCLSID rclsid,
										  REFIID riid,
										  LPVOID * ppv,
										  HRESULT * phr)
{
	if(riid != IID_IBaseFilter)
		return FALSE;

	TCHAR szGUID[MAX_GUID_LENGTH];
	::GuidToString(rclsid, szGUID);
	if(_tcsicmp(szGUID, _T("{0F40E1E5-4F79-4988-B1A9-CC98794E6B55}")) == 0 || // Aduio
	   _tcsicmp(szGUID, _T("{04FE9017-F873-410E-871E-AB91661A4EF7}")) == 0 || // Video
	   _tcsicmp(szGUID, _T("{0B390488-D80F-4A68-8408-48DC199F0E97}")) == 0 || // Raw video
	   _tcsicmp(szGUID, _T("{0B0EFF97-C750-462C-9488-B10E7D87F1A6}")) == 0 )  // DXVA
	{
		IffDShowBase * pffdshow = NULL;
		HRESULT hr = pCF->CreateInstance(NULL, IID_IffDShowBase, (void **)&pffdshow);
		if(SUCCEEDED(hr))
		{
			// Set for ffdshow
			this->ConfigFFDShow(pffdshow, szGUID);
			
			// Query IBaseFilter interface
			LPVOID pvTemp = NULL;
			hr = pffdshow->QueryInterface(IID_IBaseFilter, &pvTemp);
			if(SUCCEEDED(hr))
			{
				*ppv = pvTemp;
			}
			else
				g_utility.Log(_T("CreateFFDShowInstance - QueryInterface failed: %08x"), hr);
			pffdshow->Release();
		}
		else
			g_utility.Log(_T("CreateFFDShowInstance - CreateInstance failed: %08x"), hr);

		*phr = hr;
		return TRUE;
	}
	else
		return FALSE;
}


void CDShowFilters::ConfigFFDShow(IffDShowBase * pffdshow, LPCTSTR pcszGUID)
{
	if(pffdshow == NULL)
		return;

	// Set generic options
	TCHAR szPath[MAX_PATH];
	wsprintf(szPath, _T("%s\\ffdshow"), m_strCodecsPath);
	pffdshow->putParamStr(IDFF_installPath, szPath);
	pffdshow->putParam(IDFF_isWhitelist, 0);
	pffdshow->putParam(IDFF_trayIcon, 0);

	// Set for special filter
	if(_tcsicmp(pcszGUID, _T("{0B0EFF97-C750-462C-9488-B10E7D87F1A6}")) == 0)
	{
		g_utility.Log(_T("Is ffdshow DXVA decoder, enable text pin"));
		pffdshow->putParam(IDFF_subTextpin, 1);
	}
	else
	{
		g_utility.Log(_T("Is not ffdshow DXVA decoder, disable text pin"));
		pffdshow->putParam(IDFF_subTextpin, 0);
	}

	// Set codecs
	pffdshow->putParam(IDFF_xvid, 1);
	pffdshow->putParam(IDFF_div3, 1);
	pffdshow->putParam(IDFF_mp4v, 1);
	pffdshow->putParam(IDFF_dx50, 1);
	pffdshow->putParam(IDFF_fvfw, 1);
	pffdshow->putParam(IDFF_mp43, 1);
	pffdshow->putParam(IDFF_mp42, 1);
	pffdshow->putParam(IDFF_mp41, 1);
	pffdshow->putParam(IDFF_h263, 1);
	pffdshow->putParam(IDFF_h264, 1);
	pffdshow->putParam(IDFF_h261, 1);
	pffdshow->putParam(IDFF_wmv1, 0xc);
	pffdshow->putParam(IDFF_wmv2, 0xc);
	pffdshow->putParam(IDFF_wmv3, 0xc);
	pffdshow->putParam(IDFF_wvc1, 0xc);
	pffdshow->putParam(IDFF_cavs, 1);
	pffdshow->putParam(IDFF_vp5, 1);
	pffdshow->putParam(IDFF_vp6, 1);
	pffdshow->putParam(IDFF_vp6f, 1);
	pffdshow->putParam(IDFF_rt21, 1);
	pffdshow->putParam(IDFF_vixl, 1);
	pffdshow->putParam(IDFF_aasc, 1);
	pffdshow->putParam(IDFF_qtrpza, 1);
	pffdshow->putParam(IDFF_mjpg, 1);
	pffdshow->putParam(IDFF_dvsd, 1);
	pffdshow->putParam(IDFF_hfyu, 1);
	pffdshow->putParam(IDFF_cyuv, 1);
	pffdshow->putParam(IDFF_mpg1, 5);
	pffdshow->putParam(IDFF_mpg2, 5);
	pffdshow->putParam(IDFF_mpegAVI, 1);
	pffdshow->putParam(IDFF_asv1, 1);
	pffdshow->putParam(IDFF_vcr1, 1);
	pffdshow->putParam(IDFF_rle, 1);
	pffdshow->putParam(IDFF_theo, 1);
	pffdshow->putParam(IDFF_rv10, 1);
	pffdshow->putParam(IDFF_ffv1, 1);
	pffdshow->putParam(IDFF_vp3, 1);
	pffdshow->putParam(IDFF_tscc, 1);
	pffdshow->putParam(IDFF_rawv, 1);
	pffdshow->putParam(IDFF_alternateUncompressed, 1);
	pffdshow->putParam(IDFF_svq1, 1);
	pffdshow->putParam(IDFF_svq3, 1);
	pffdshow->putParam(IDFF_cram, 1);
	pffdshow->putParam(IDFF_iv32, 1);
	pffdshow->putParam(IDFF_cvid, 1);
	pffdshow->putParam(IDFF_mszh, 1);
	pffdshow->putParam(IDFF_zlib, 1);
	pffdshow->putParam(IDFF_flv1, 1);
	pffdshow->putParam(IDFF_8bps, 1);
	pffdshow->putParam(IDFF_png1, 1);
	pffdshow->putParam(IDFF_qtrle, 1);
	pffdshow->putParam(IDFF_duck, 1);
	pffdshow->putParam(IDFF_qpeg, 1);
	pffdshow->putParam(IDFF_loco, 1);
	pffdshow->putParam(IDFF_wnv1, 1);
	pffdshow->putParam(IDFF_cscd, 1);
	pffdshow->putParam(IDFF_zmbv, 1);
	pffdshow->putParam(IDFF_ulti, 1);
	pffdshow->putParam(IDFF_wma7, 1);
	pffdshow->putParam(IDFF_wma8, 1);
	pffdshow->putParam(IDFF_mp2, 1);
	pffdshow->putParam(IDFF_mp3, 1);
	pffdshow->putParam(IDFF_ac3, 1);
	pffdshow->putParam(IDFF_eac3, 1);
	pffdshow->putParam(IDFF_dts, 1);
	pffdshow->putParam(IDFF_dtsinwav, 1);
	pffdshow->putParam(IDFF_aac, 1);
	pffdshow->putParam(IDFF_amr, 1);
	pffdshow->putParam(IDFF_iadpcm, 1);
	pffdshow->putParam(IDFF_msadpcm, 1);
	pffdshow->putParam(IDFF_otherAdpcm, 1);
	pffdshow->putParam(IDFF_law, 1);
	pffdshow->putParam(IDFF_gsm, 1);
	pffdshow->putParam(IDFF_flac, 1);
	pffdshow->putParam(IDFF_tta, 1);
	pffdshow->putParam(IDFF_qdm2, 1);
	pffdshow->putParam(IDFF_mace, 1);
	pffdshow->putParam(IDFF_truespeech, 1);
	pffdshow->putParam(IDFF_vorbis, 1);
	pffdshow->putParam(IDFF_lpcm, 4);
	pffdshow->putParam(IDFF_fps1, 0);
	pffdshow->putParam(IDFF_ra, 1);
	pffdshow->putParam(IDFF_imc, 1);
	pffdshow->putParam(IDFF_mss2, 0xc);
	pffdshow->putParam(IDFF_wvp2, 0xc);
	pffdshow->putParam(IDFF_em2v, 1);
	pffdshow->putParam(IDFF_avrn, 1);
	pffdshow->putParam(IDFF_cdvc, 1);
	pffdshow->putParam(IDFF_atrac3, 1);
	pffdshow->putParam(IDFF_nellymoser, 1);
	pffdshow->putParam(IDFF_wavpack, 1);
	pffdshow->putParam(IDFF_rawa, 4);
	pffdshow->putParam(IDFF_avisV, 0xa);
	pffdshow->putParam(IDFF_avisA, 0xa);
	pffdshow->putParam(IDFF_mlp, 1);
	pffdshow->putParam(IDFF_truehd, 1);
	pffdshow->putParam(IDFF_rv40, 0);
	pffdshow->putParam(IDFF_rv30, 0);
	pffdshow->putParam(IDFF_cook, 1);
	pffdshow->putParam(IDFF_vp8, 1);
	pffdshow->putParam(IDFF_iv50, 1);
	pffdshow->putParam(IDFF_i263, 0);
}


BOOL CDShowFilters::LoadPreloadDlls(DShowFilterInfo* pInfo)
{
	CheckPointer(pInfo, FALSE);

	CString dllpath;
	POSITION pos = pInfo->preloaddlls.GetHeadPosition();
	while (pos)
	{
		dllpath = pInfo->preloaddlls.GetNext(pos);

		g_utility.Log(_T("LoadPreloadDlls for %s"), pInfo->name);

		LPCTSTR pcszFileName = ::GetFileNameFromPath(dllpath);
		HMODULE hDll = ::GetModuleHandle(pcszFileName);
		if (hDll == NULL)
		{
			g_utility.Log(_T("LoadPreloadDlls, LoadLibrary \"%s\""), dllpath);
			hDll = ::LoadLibrary(dllpath);
			if(hDll == NULL)
			{
				g_utility.Log(_T("LoadPreloadDlls, LoadLibrary \"%s\" failed, last error=%d"),
					dllpath, (int)::GetLastError());
			}
		}
		else
		{
			TCHAR szFileName[MAX_PATH] = _T("Unknown");
			::GetModuleFileName(hDll, szFileName, MAX_PATH);
			g_utility.Log(_T("LoadPreloadDlls, Library \"%s\" already loaded, path=%s"),
				dllpath, szFileName);
		}
	}
	return TRUE;
}

BOOL CDShowFilters::InitializeAllVfwDrivers(void)
{
	DShowFilterInfo* pInfo = NULL;
	POSITION pos = m_InfoList.GetHeadPosition();
	while (pos)
	{
		pInfo = m_InfoList.GetNext(pos);
		if (pInfo->device == kFilterDeviceVfw)
			pInfo->merit = 0;
	}
	return TRUE;
}

BOOL CDShowFilters::InstallVfwDriver(DShowFilterInfo* pInfo)
{
	// Check param
	if(pInfo == NULL || pInfo->device != kFilterDeviceVfw || pInfo->merit != 0)
		return FALSE;
	g_utility.Log(_T("InstallVfwDriver: %s"), pInfo->name);

	// Install the VFW driver
	BOOL bOK = FALSE;
	DWORD fccType, fccHandler;
	if(this->ParseDeviceData_VFW(pInfo->devicedata, &fccType, &fccHandler))
	{
		HMODULE hDriver = ::LoadLibraryEx(pInfo->path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
		if(hDriver != NULL)
		{
			typedef LRESULT (*TypeDriverProc)
				(DWORD dwID, HDRVR hDriver, UINT uiMessage, LPARAM lParam1, LPARAM lParam2); 
			TypeDriverProc pfn = (TypeDriverProc)::GetProcAddress(hDriver, "DriverProc");
			if(pfn != NULL)
			{
				BOOL bResult = ::ICInstall(fccType, fccHandler, (LPARAM)pfn, NULL, ICINSTALL_FUNCTION);
				if(bResult)
				{
					pInfo->merit = (DWORD)(LONG_PTR)hDriver;
					bOK = TRUE;
				}
				else
					g_utility.Log(_T("InstallVfwDriver, ICInstall failed"));
			}
			else
				g_utility.Log(_T("InstallVfwDriver, get DriverProc function failed, %s"), pInfo->path);
			if(!bOK)
				::FreeLibrary(hDriver);
		}
		else
			g_utility.Log(_T("InstallVfwDriver, LoadLibrary failed, %s, LastError=%x"), pInfo->path, ::GetLastError());
	}
	else
		g_utility.Log(_T("InstallVfwDriver, ParseDeviceData_VFW failed, %s"), pInfo->devicedata);
	return bOK;
}

BOOL CDShowFilters::UninstallAllVfwDrivers(void)
{
	POSITION pos = m_InfoList.GetHeadPosition();
	while (pos)
	{
		DShowFilterInfo* pInfo = NULL;
		pInfo = m_InfoList.GetNext(pos);
		if(pInfo->device == kFilterDeviceVfw)
		{
			if(pInfo->merit != 0)
			{
				DWORD fccType, fccHandler;
				if(this->ParseDeviceData_VFW(pInfo->devicedata, &fccType, &fccHandler))
				{
					BOOL bResult = ::ICRemove(fccType, fccHandler, 0);
					if(bResult)
					{
						::FreeLibrary((HMODULE)(LONG_PTR)(pInfo->merit));
						pInfo->merit = 0;
					}
					else
						g_utility.Log(_T("InstallVfwDrivers, ICRemove failed"));
				}
			}
		}
	}
	return TRUE;
}

HRESULT CDShowFilters::ConfigTypeData(LPVOID pv, DShowFilterInfo* pInfo)
{
	CheckPointer(pv, E_INVALIDARG);
	CheckPointer(pInfo, E_INVALIDARG);

	HRESULT hr = E_FAIL;
	switch(pInfo->device)
	{
	case kFilterDeviceUnknown:
	case kFilterDeviceFilter:
	case kFilterDeviceVfw:
		{
			hr = S_OK;
		}
		break;
	case kFilterDeviceDmo:
		{
			// Set System DMO Wapper
			///*
			CLSID clsidCat, clsidDMO;
			TCHAR * pList = pInfo->devicedata;
			TCHAR szClsid[MAX_PATH];
			int nIndex = 0;
			while(::StringFromList(&pList, _T(","), szClsid, _countof(szClsid)))
			{
				if(nIndex == 0)
					::StringToGuid(szClsid, &clsidCat);
				else if(nIndex == 1)
					::StringToGuid(szClsid, &clsidDMO);
				nIndex++;
			}

			IBaseFilter * pBaseFilter = (IBaseFilter *)pv;
			IDMOWrapperFilter * pWrapper = NULL;
			hr = pBaseFilter->QueryInterface(__uuidof(IDMOWrapperFilter), (void **)&pWrapper);
			if(hr == S_OK)
			{
				hr = pWrapper->Init(clsidDMO, clsidCat);
				if(hr != S_OK)
					g_utility.Log(_T("ConfigTypeData - pWrapper->Init failed: %08x"), hr);
				pWrapper->Release();
			}
			//*/
		}
		break;
	}
	return hr;
}

BOOL CDShowFilters::ParseDeviceData_DMO(LPCTSTR pcszData, CLSID* pDmoClsId)
{
	// Check param
	if(pcszData == NULL || pDmoClsId == NULL)
		return FALSE;
	int nDataLength = _tcslen(pcszData);
	if(nDataLength < 3)
		return FALSE;

	// Get CLSID
	BOOL bResult = ::StringToGuid(pcszData, pDmoClsId);
	return bResult;
}


BOOL CDShowFilters::ParseDeviceData_VFW(LPCTSTR pcszData, DWORD* pfccType, DWORD* pfccHandler)
{
	// Check param
	if(pcszData == NULL || pfccType == NULL || pfccHandler == NULL)
		return FALSE;
	int nDataLength = _tcslen(pcszData);
	if(nDataLength < 9)
		return FALSE;

	// Pasre media type
	const int fccLength = 5;
	TCHAR szType[fccLength];
	TCHAR szHandler[fccLength];
	BOOL bOK = FALSE;
	TCHAR * p = _tcsstr((TCHAR*)pcszData, _T(","));
	if(p != NULL)
	{
		p += 1;
		int nCopyLength = (int)(LONG_PTR)(p-pcszData);
		if(nCopyLength <= fccLength)
		{
			::lstrcpyn(szType, pcszData, nCopyLength);

			nCopyLength = (int)(LONG_PTR)(pcszData + nDataLength - p + 1);
			if(nCopyLength <= fccLength)
			{
				::lstrcpyn(szHandler, p, nCopyLength);
				bOK = TRUE;
			}
		}
	}
	if(!bOK)
		return FALSE;

	// Set result
	char * pChar = NULL;
	pChar = (char *)pfccType;
	pChar[0] = (char)szType[0];
	pChar[1] = (char)szType[1];
	pChar[2] = (char)szType[2];
	pChar[3] = (char)szType[3];
	pChar = (char *)pfccHandler;
	pChar[0] = (char)szHandler[0];
	pChar[1] = (char)szHandler[1];
	pChar[2] = (char)szHandler[2];
	pChar[3] = (char)szHandler[3];
	return TRUE;
}
