
#include "stdafx.h"
#include "Utils.h"

//////////////////////////////////////////////////////////////////////////
// g_utility
//////////////////////////////////////////////////////////////////////////
CUtility g_utility;

//////////////////////////////////////////////////////////////////////////
// Filter
//////////////////////////////////////////////////////////////////////////
//
BOOL GetFilterName(IBaseFilter* pFilter, CString& strFilterName)
{
	if(pFilter == NULL)
		return FALSE;

	FILTER_INFO fi;
	HRESULT hRet = pFilter->QueryFilterInfo(&fi);
	if(hRet != S_OK)
		return FALSE;

	strFilterName = (CString)(CStringW)fi.achName;

// #ifdef UNICODE
// 	::wcscpy_s(pBuffer, MAX_FILTER_NAME, fi.achName);
// #else
// 	::WideCharToMultiByte(CP_ACP, 0, fi.achName, -1, pBuffer, MAX_FILTER_NAME, NULL, NULL);
// #endif

	if(fi.pGraph != NULL)
		fi.pGraph->Release();
	return TRUE;
}

//





//////////////////////////////////////////////////////////////////////////
// String
//////////////////////////////////////////////////////////////////////////
BOOL StringFromList(TCHAR** ppList, const TCHAR* pcszSplitter, TCHAR* pszBuffer, int nBufferChars)
{
	if(ppList == NULL || *ppList == NULL || pcszSplitter == NULL || pszBuffer == NULL || nBufferChars < 2)
		return FALSE;
	TCHAR* pList = *ppList;
	if(pList[0] == 0)
		return FALSE;

	TCHAR* pBegin = pList;
	TCHAR* pEnd = _tcsstr(pList, pcszSplitter);
	if(pEnd == NULL)
	{
		pEnd = pList + lstrlen(pList);
		pList = pEnd;
	}
	else
	{
		pList = pEnd + lstrlen(pcszSplitter);
	}

	int nCopyLength = (LONG)(LONG_PTR)(pEnd - pBegin);
	if(nCopyLength > nBufferChars - 1)
		nCopyLength = nBufferChars - 1;
	memcpy(pszBuffer, pBegin, nCopyLength * sizeof(TCHAR));
	pszBuffer[nCopyLength] = 0;

	*ppList = pList;
	return TRUE;
}


void JumpSpace(const TCHAR** ppText)
{
	const TCHAR* pText = *ppText;
	while(*pText == ' ' || *pText == '\t')
		pText++;
	*ppText = pText;
}


void TrimSpace(TCHAR* pString)
{
	TCHAR* p1 = pString;
	TCHAR* p2 = pString;
	TCHAR* pEnd = p1 + lstrlen(p1);
	while(p1 < pEnd)
	{
		if(*p1 == ' ' || *p1 == '\t' || *p1 == 0x0d || *p1 == 0x0A )
		{
			p1++;
			continue;
		}
		*p2 = *p1;
		p1++;
		p2++;
	}
	*p2 = '\0';
}


void AddBeginSpace(TCHAR* pszText, int nTotalLength)
{
	int nLength = lstrlen(pszText);
	TCHAR* pTemp = new TCHAR[nLength+1];
	lstrcpy(pTemp, pszText);

	TCHAR * p = pszText;
	for(int i=nLength; i<nTotalLength; i++)
	{
		*p = ' ';
		p++;
	}
	p[0] = 0;
	lstrcat(p, pTemp);
}


void AddEndSpace(TCHAR* pszText, int nTotalLength)
{
	int nLength = lstrlen(pszText);
	TCHAR* p = pszText + nLength;
	for(int i=nLength; i<nTotalLength; i++)
	{
		*p = ' ';
		p++;
	}
	p[0] = 0;
}


void CutEndSpace(TCHAR* pszText)
{
	TCHAR* pEnd = pszText + lstrlen(pszText) - 1;
	while(pEnd > pszText && (*pEnd == ' ' || *pEnd == '\t'))
		pEnd--;
	pEnd++;
	*pEnd = 0;
}


// int ReplaceString(TCHAR * pszText, const TCHAR * pcszOld, const TCHAR * pcszNew)
// {
// 	CString str(pszText);
// 	int nCount = str.Replace(pcszOld, pcszNew);
// 	lstrcpy(pszText, str.GetBuffer());
// 	return nCount;
// }


BOOL AppendString(TCHAR** ppText, const TCHAR* pcszTextEnd, const TCHAR* pcszAppend)
{
	TCHAR* pText = *ppText;
	int nLength = lstrlen(pcszAppend);
	if(nLength > pcszTextEnd - pText - 1)
		return FALSE;
	lstrcpy(pText, pcszAppend);
	pText += nLength;
	*ppText = pText;
	return TRUE;
}


BOOL AppendNumber(TCHAR** ppText, const TCHAR* pcszTextEnd, int nNumber)
{
	TCHAR szNumber[32];
	wsprintf(szNumber, _T("%d"), nNumber);
	BOOL bResult = AppendString(ppText, pcszTextEnd, szNumber);
	return bResult;
}


BOOL GuidToString(const GUID& theGuid, TCHAR* pBuffer)
{
	const TCHAR* pcszFailed = _T("{Get failed}");

	BSTR pGuid = NULL;
	HRESULT hr = ::StringFromCLSID(theGuid, &pGuid);
	if(hr != S_OK)
	{
		_tcscpy_s(pBuffer, MAX_GUID_LENGTH, pcszFailed);
		return FALSE;
	}

#ifdef UNICODE
	::wcscpy_s(pBuffer, MAX_GUID_LENGTH, pGuid);
#else
	char szGuid[40];
	int nCount = ::WideCharToMultiByte(CP_ACP, 0, pGuid, -1, szGuid, sizeof(szGuid), NULL, NULL);
	if(nCount > 0)
		strcpy(pBuffer, szGuid);
	else
		strcpy(pBuffer, pcszFailed);
#endif

	::CoTaskMemFree(pGuid);
	return TRUE;
}


BOOL StringToGuid(const TCHAR* pcszString, GUID* pGuid)
{
	WCHAR szGuid[MAX_GUID_LENGTH];
#ifdef UNICODE
	::wcscpy_s(szGuid, MAX_GUID_LENGTH, pcszString);
#else
	::MultiByteToWideChar(CP_ACP, 0, pcszString, -1, szGuid, MAX_GUID_LENGTH);
#endif
	HRESULT hr = ::CLSIDFromString(szGuid, pGuid);
	if(hr != S_OK)
		return FALSE;
	return TRUE;
}

BOOL MByteToWChar(LPCSTR lpcszSrc, LPWSTR lpwszDst, DWORD dwDstSize)
{
	// Get the required size of the buffer that receives the Unicode 
	// string. 
	DWORD dwMinSize;
	dwMinSize = MultiByteToWideChar(CP_ACP, 0, lpcszSrc, -1, NULL, 0);
	if(dwDstSize < dwMinSize)
	{
		return FALSE;
	}
	// Convert headers from ASCII to Unicode.
	MultiByteToWideChar(CP_ACP, 0, lpcszSrc, -1, lpwszDst, dwDstSize); 
	return TRUE;
}

BOOL WCharToMByte(LPCWSTR lpcwszSrc, LPSTR lpszDst, DWORD dwDstSize)
{
	DWORD dwMinSize;
	dwMinSize = WideCharToMultiByte(CP_ACP, NULL, lpcwszSrc, -1, NULL, 0, NULL, FALSE);
	if(dwDstSize < dwMinSize)
	{
		return FALSE;
	}
	WideCharToMultiByte(CP_ACP, NULL, lpcwszSrc, -1, lpszDst, dwDstSize, NULL, FALSE);
	return TRUE;
}


void CStringToBin(CString str, CAtlArray<BYTE>& data)
{
	str.Trim();
	ASSERT((str.GetLength() & 1) == 0);
	data.SetCount(str.GetLength() / 2);

	BYTE b = 0;

	str.MakeUpper();
	for (int i = 0, j = str.GetLength(); i < j; i++) {
		TCHAR c = str[i];
		if (c >= _T('0') && c <= _T('9')) {
			if (!(i & 1)) {
				b = ((char(c - _T('0')) << 4) & 0xf0) | (b & 0x0f);
			} else {
				b = (char(c - _T('0')) & 0x0f) | (b & 0xf0);
			}
		} else if (c >= _T('A') && c <= _T('F')) {
			if (!(i & 1)) {
				b = ((char(c - _T('A') + 10) << 4) & 0xf0) | (b & 0x0f);
			} else {
				b = (char(c - _T('A') + 10) & 0x0f) | (b & 0xf0);
			}
		} else {
			break;
		}

		if (i & 1) {
			data[i >> 1] = b;
			b = 0;
		}
	}
}

CString BinToCString(const BYTE* ptr, size_t len)
{
	CString ret;
	TCHAR high, low;

	while (len-- > 0) {
		high = (*ptr >> 4) >= 10 ? (*ptr >> 4) - 10 + _T('A') : (*ptr >> 4) + _T('0');
		low = (*ptr & 0xf) >= 10 ? (*ptr & 0xf) - 10 + _T('A') : (*ptr & 0xf) + _T('0');

		ret.AppendFormat(_T("%c%c"), high, low);

		ptr++;
	}

	return ret;
}

BOOL MediaTypeToString(AM_MEDIA_TYPE* pMT, CString& strMediaType)
{
	if(pMT == NULL)
		return FALSE;

	TCHAR szMajor[MAX_GUID_LENGTH];
	TCHAR szSub[MAX_GUID_LENGTH];

	if(pMT->majortype == MEDIATYPE_Stream)
		_tcscpy_s(szMajor, _T("MEDIATYPE_Stream"));
	if(pMT->majortype == MEDIATYPE_Video)
		_tcscpy_s(szMajor, _T("MEDIATYPE_Video"));
	else if(pMT->majortype == MEDIATYPE_Audio)
		_tcscpy_s(szMajor, _T("MEDIATYPE_Audio"));
	else
		::GuidToString(pMT->majortype, szMajor);
	::GuidToString(pMT->subtype, szSub);

	strMediaType.Format(_T("%s,%s"), szMajor, szSub);
	return TRUE;

}


//////////////////////////////////////////////////////////////////////////
// 
BOOL GetModulePath(HMODULE hModule, TCHAR* pszPath)
{
	if(GetModuleFileName(hModule, pszPath, MAX_PATH) <= 0)
		return FALSE;

	TCHAR* pEnd = pszPath + lstrlen(pszPath) - 1;
	while(pEnd > pszPath && *pEnd != '\\') pEnd--;
	*pEnd = 0;
	return TRUE;
}

const TCHAR* GetFileNameFromPath(const TCHAR* pcszFilePath)
{
	const TCHAR* p = pcszFilePath + lstrlen(pcszFilePath) - 1;
	while(p > pcszFilePath && *p != '\\') p--;
	p++;
	return p;
}

BOOL IsFileExists(const TCHAR* pcszFileName)
{
	DWORD dwAttributes = GetFileAttributes(pcszFileName);
	if(dwAttributes == 0xFFFFFFFF)
		return FALSE;
	if(dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
		return FALSE;
	return TRUE;
}


BOOL IsDirectoryExists(const TCHAR* pcszDirectory)
{
	DWORD dwAttributes = GetFileAttributes(pcszDirectory);
	if(dwAttributes == 0xFFFFFFFF)
		return FALSE;
	if(dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
		return TRUE;
	else
		return FALSE;
}

BOOL GetFileVersionString(const TCHAR* pcszFileName, TCHAR* pszVersionString)
{
	if(pcszFileName == NULL || pszVersionString == NULL)
		return FALSE;

	BOOL bResult = FALSE;
	DWORD dwHandle = 0;
	DWORD dwSize = ::GetFileVersionInfoSize(pcszFileName, &dwHandle);
	if(dwSize > 0)
	{
		TCHAR * pVersionInfo = new TCHAR[dwSize+1];
		if(::GetFileVersionInfo(pcszFileName, dwHandle, dwSize, pVersionInfo))
		{
			VS_FIXEDFILEINFO * pvi;
			UINT uLength = 0;
			if(::VerQueryValue(pVersionInfo, _T("\\"), (void **)&pvi, &uLength))
			{
				wsprintf(pszVersionString, _T("%d.%d.%d.%d"),
					HIWORD(pvi->dwFileVersionMS), LOWORD(pvi->dwFileVersionMS),
					HIWORD(pvi->dwFileVersionLS), LOWORD(pvi->dwFileVersionLS));
				bResult = TRUE;
			}
		}
		delete pVersionInfo;
	}
	return bResult;
}

int GetFileBuildNumber(const TCHAR * pcszFileName)
{
	if(pcszFileName == NULL)
		return 0;

	int nResult = 0;
	DWORD dwHandle = 0;
	DWORD dwSize = ::GetFileVersionInfoSize(pcszFileName, &dwHandle);
	if(dwSize > 0)
	{
		char * pVersionInfo = new char[dwSize+1];
		if(::GetFileVersionInfo(pcszFileName, dwHandle, dwSize, pVersionInfo))
		{
			VS_FIXEDFILEINFO * pvi;
			UINT uLength = 0;
			if(::VerQueryValue(pVersionInfo, _T("\\"), (void **)&pvi, &uLength))
			{
				nResult = LOWORD(pvi->dwFileVersionLS);
			}
		}
		delete pVersionInfo;
	}
	return nResult;
}


BOOL GetFileDescription(const TCHAR * pcszFileName, TCHAR * pBuffer)
{
	// Check parameter
	if(pcszFileName == NULL || pBuffer == NULL)
		return FALSE;
	TCHAR * p = pBuffer;

	// Get file name
	lstrcpy(p, ::GetFileNameFromPath(pcszFileName));
	::AddEndSpace(p, 20);
	p += lstrlen(p);

	if(!::IsFileExists(pcszFileName))
	{
		lstrcpy(p, _T("No exists.\r\n"));
		return TRUE;
	}

	// Get file size
	unsigned __int64 nSize = ::GetFileSizeI64(pcszFileName);
	wsprintf(p, _T("%I64i  "), nSize);
	::AddBeginSpace(p, 20);
	p += lstrlen(p);

	// Get file version info size
	DWORD dwHandle = 0;
	DWORD dwSize = ::GetFileVersionInfoSize(pcszFileName, &dwHandle);
	if(dwSize > 0)
	{
		// Get file version info
		TCHAR * pVersionInfo = new TCHAR[dwSize+1];
		if(::GetFileVersionInfo(pcszFileName, dwHandle, dwSize, pVersionInfo))
		{
			// Get file version number
			VS_FIXEDFILEINFO * pvi;
			UINT uLength = 0;
			if(::VerQueryValue(pVersionInfo, _T("\\"), (void **)&pvi, &uLength))
			{
				wsprintf(p, _T("%d.%d.%d.%d"),
					HIWORD(pvi->dwFileVersionMS), LOWORD(pvi->dwFileVersionMS),
					HIWORD(pvi->dwFileVersionLS), LOWORD(pvi->dwFileVersionLS));
				::AddEndSpace(p, 20);
				p += lstrlen(p);
			}

			// Structure used to store enumerated languages and code pages.   
			struct LANGANDCODEPAGE
			{   
				WORD wLanguage;
				WORD wCodePage;
			} *lpTranslate;

			// Get file description
			// Read the list of languages and code pages.
			UINT cbTranslate;
			VerQueryValue(pVersionInfo, _T("\\VarFileInfo\\Translation"), (LPVOID*)&lpTranslate, &cbTranslate);

			// Read the file description for each language and code page.
			TCHAR szSubBlock[MAX_PATH];
			int nCount = cbTranslate / sizeof(LANGANDCODEPAGE);
			if(nCount > 0)
			{
				wsprintf(szSubBlock, _T("\\StringFileInfo\\%04x%04x\\FileDescription"),
					lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);   

				// Retrieve file description for language and code page "i".
				UINT nBytes = 0;
				TCHAR * pDescription = NULL;
				VerQueryValue(pVersionInfo, szSubBlock, (void **)&pDescription, &nBytes);

				lstrcpy(p, pDescription);
				p += lstrlen(p);
			}
			lstrcpy(p, _T("\r\n"));
		}
		else
			lstrcpy(p, _T("Get version info failed.\r\n"));
		delete pVersionInfo;
	}
	else
	{
		lstrcpy(p, _T("No version info.\r\n"));
	}
	return TRUE;
}


// Resource
TCHAR* LoadResourceText(LPCTSTR pcszType, UINT uResourceId)
{
	TCHAR* pResult = NULL;
	HRSRC hRsrc = ::FindResource(g_hInstance, MAKEINTRESOURCE(uResourceId), pcszType);
	if(hRsrc != NULL)
	{
		HGLOBAL hGlobal = ::LoadResource(g_hInstance, hRsrc);
		if(hGlobal != NULL)
		{
			DWORD dwSize = ::SizeofResource(g_hInstance, hRsrc);
			if(dwSize > 0)
			{
				LPCSTR pData = (LPCSTR)::LockResource(hGlobal);
				if(pData != NULL)
				{
#ifdef UNICODE
					int nLength = ::MultiByteToWideChar(CP_ACP, 0, pData, dwSize, NULL, 0);
					TCHAR* pszBuffer = new TCHAR[nLength + 1];
					if(pszBuffer != NULL)
					{
						int nResult = ::MultiByteToWideChar(CP_ACP, 0, pData, dwSize, pszBuffer, nLength);
						if(nResult > 0)
						{
							pszBuffer[nResult] = '\0';
							pResult = pszBuffer;
						}
						else
							delete pszBuffer;
					}
#else
					TCHAR * pszBuffer = new TCHAR[dwSize + 1];
					if(pszBuffer != NULL)
					{
						lstrcpyn(pszBuffer, pData, dwSize);
						pszBuffer[dwSize] = '\0';
						pResult = pszBuffer;
					}
#endif // UNICODE
					UnlockResource(hGlobal);
				}
			}
			::FreeResource(hGlobal);
		}
	}
	return pResult;
}

void FreeResourceText(TCHAR* pText)
{
	if(pText != NULL)
		delete[] pText;
}

// Miscs
int GetCaller(TCHAR * pszCaller)
{
	DWORD dwEBP;
	__asm mov dwEBP, EBP;
	if(::IsBadReadPtr((const void *)(LONG_PTR)dwEBP, sizeof(DWORD)))
		return -1;

	DWORD dwEBPCaller = *(DWORD *)(LONG_PTR)dwEBP;
	if(::IsBadReadPtr((const void *)(LONG_PTR)(dwEBPCaller + 4), sizeof(DWORD)))
		return -2;

	DWORD dwAddress = *(DWORD *)(LONG_PTR)(dwEBPCaller + 4);
	if(::IsBadReadPtr((const void *)(LONG_PTR)dwAddress, sizeof(DWORD)))
		return -3;

	::MEMORY_BASIC_INFORMATION mbi;
	SIZE_T theSize = ::VirtualQuery((LPCVOID)(LONG_PTR)dwAddress, &mbi, sizeof(mbi));
	if(theSize == 0)
		return -4;

	TCHAR szPath[MAX_PATH];
	DWORD dwLength = ::GetModuleFileName((HMODULE)mbi.AllocationBase, szPath, sizeof(szPath)-1);
	if(dwLength == 0)
		return -5;

	const TCHAR * pcszFileName = ::GetFileNameFromPath(szPath);
	lstrcpy(pszCaller, pcszFileName);
	return 0;
}

///////////////////////////////////////////////////////////
//	CAutoCriticalSection
///////////////////////////////////////////////////////////
CAutoCriticalSection::CAutoCriticalSection(CRITICAL_SECTION * pCriticalSection, BOOL bBlock)
{
	m_bBlock = bBlock;
	m_pCriticalSection = pCriticalSection;
	if(m_bBlock)
	{
		::EnterCriticalSection(m_pCriticalSection);
		m_bEnter = TRUE;
	}
	else
	{
		m_bEnter = ::TryEnterCriticalSection(m_pCriticalSection);
	}
}

CAutoCriticalSection::~CAutoCriticalSection()
{
	if(m_bBlock)
	{
		m_bEnter = FALSE;
		::LeaveCriticalSection(m_pCriticalSection);
	}
	else
	{
		if(m_bEnter)
		{
			m_bEnter = FALSE;
			::LeaveCriticalSection(m_pCriticalSection);
		}
	}
}

BOOL CAutoCriticalSection::IsEnter(void)
{
	return m_bEnter;
}

///////////////////////////////////////////////////////////
//	CSubclassHelper
///////////////////////////////////////////////////////////
CSubclassHelper::CSubclassHelper()
{
	m_nItemCount = 0;
	memset(m_pItems, 0, sizeof(m_pItems));
}


CSubclassHelper::~CSubclassHelper()
{
	for(int i=0; i<m_nItemCount; i++)
	{
		delete m_pItems[i];
		m_pItems[i] = NULL;
	}
	m_nItemCount = 0;
}


int CSubclassHelper::GetItemCount(void)
{
	return m_nItemCount;
}


void * CSubclassHelper::GetItem(int nIndex)
{
	if(nIndex < 0 || nIndex >= m_nItemCount)
		return NULL;
	return m_pItems[nIndex];
}


BOOL CSubclassHelper::AddItem(HWND hWnd, void * pObject)
{
	if(m_nItemCount >= MAX_SUBCLASS_ITEM)
	{
		//g_utility.Log(_T("Warning! subclass item is overflow!"));
		return FALSE;
	}

	SUBCLASS_ITEM * pItem = new SUBCLASS_ITEM;
	if(pItem == NULL)
	{
		//g_utility.Log(_T("Create subclass item failed."));
		return FALSE;
	}

	pItem->hWnd = hWnd;
	pItem->pObject = pObject;
	m_pItems[m_nItemCount] = pItem;
	m_nItemCount++;
	return TRUE;
}


void CSubclassHelper::DeleteItem(HWND hWnd)
{
	int nIndex = -1;
	for(int i=0; i<m_nItemCount; i++)
	{
		if(m_pItems[i]->hWnd == hWnd)
		{
			nIndex = i;
			break;
		}
	}
	if(nIndex == -1)
		return;

	delete m_pItems[nIndex];
	for(int i=nIndex; i<m_nItemCount-1; i++)
		m_pItems[i] = m_pItems[i+1];
	m_pItems[m_nItemCount-1] = NULL;

	m_nItemCount--;
}


void * CSubclassHelper::FindItem(HWND hWnd)
{
	for(int i=0; i<m_nItemCount; i++)
	{
		if(m_pItems[i]->hWnd == hWnd)
			return m_pItems[i]->pObject;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
// CUtility
//////////////////////////////////////////////////////////////////////////

#define PROFILE_APP_NAME	_T("Config")

CUtility::CUtility()
{
	// Path
	lstrcpy(m_szExePath, EMPTY_STRING);
	lstrcpy(m_szExeName, EMPTY_STRING);
	lstrcpy(m_szModulePath, EMPTY_STRING);
	lstrcpy(m_szSystemPath, EMPTY_STRING);
	lstrcpy(m_szTemporaryPath, EMPTY_STRING);
	m_OperationSystem = OS_OTHER;
	m_nIEVersion = 0;

	// Log
	m_bLogEnable = FALSE;
	memset(m_szLogFile, 0, sizeof(m_szLogFile));
	m_hFile = INVALID_HANDLE_VALUE;
}

CUtility::~CUtility()
{
	UnInitialize();
}

BOOL CUtility::Initialize(void)
{
	::InitializeCriticalSection(&m_csLog);

	// Get process id
	m_dwProcessId = ::GetCurrentProcessId();

	// Get EXE path
	if(!::GetModuleFileName(NULL, m_szExePath, _countof(m_szExePath)))
		return FALSE;

	// Get EXE name
	lstrcpy(m_szExeName, ::GetFileNameFromPath(m_szExePath));

	// Get Player path
	if(!::GetModulePath(g_hInstance, m_szModulePath))
		return FALSE;

	// Get system path
	UINT uResult = ::GetSystemDirectory(m_szSystemPath, _countof(m_szSystemPath));
	if(uResult == 0)
		return FALSE;
	int nLength = lstrlen(m_szSystemPath);
	if(m_szSystemPath[nLength - 1] == '\\')
		m_szSystemPath[nLength - 1] = '\0';

	// Get temporary path
	uResult = ::GetTempPath(countof(m_szTemporaryPath), m_szTemporaryPath);
	if(uResult == 0)
		return FALSE;
	nLength = lstrlen(m_szTemporaryPath);
	if(m_szTemporaryPath[nLength - 1] == '\\')
		m_szTemporaryPath[nLength - 1] = '\0';

	lstrcat(m_szTemporaryPath, _T("\\Low"));
	if(!::IsDirectoryExists(m_szTemporaryPath))
		::CreateDirectory(m_szTemporaryPath, NULL);

	/////////////////////////////////////////////
	// Load & Apply profile
	/////////////////////////////////////////////

	// Log filename
	const TCHAR * pcszLogFileName = _T("FlyfoxLocalPlayerLog");
	if(this->GetProfileInt(_T("FixLogFile"), 0))
		wsprintf(m_szLogFile, _T("%s\\%s.txt"), m_szTemporaryPath, pcszLogFileName);
	else
		wsprintf(m_szLogFile, _T("%s\\%s_%u.txt"), m_szTemporaryPath, pcszLogFileName, m_dwProcessId);

	// Log enable
	if(this->GetProfileInt(_T("Log"), 0))
		this->EnableLog(TRUE);
	this->Log(_T("DLL_PROCESS_ATTACH"));


	/////////////////////////////////////////////
	// Other initialize
	/////////////////////////////////////////////
	this->GetVersionInfo();
	//::SetTimer(NULL, 0, 2000, TimeProc); // Set timer to monitor log enable
	return TRUE;
}

void CUtility::UnInitialize()
{
	::DeleteCriticalSection(&m_csLog);
}


const TCHAR* CUtility::GetExePath(void)
{
	return m_szExePath;
}

const TCHAR* CUtility::GetExeName(void)
{
	return m_szExeName;
}

const TCHAR* CUtility::GetModulePath(void)
{
	return m_szModulePath;
}

const TCHAR* CUtility::GetSystemPath(void)
{
	return m_szSystemPath;
}

const TCHAR* CUtility::GetTemporaryPath(void)
{
	return m_szTemporaryPath;
}

OPERATION_SYSTEM CUtility::GetOS(void)
{
	return m_OperationSystem;
}

int CUtility::GetIEVersion(void)
{
	return m_nIEVersion;
}

DWORD CUtility::GetProcessId(void)
{
	return m_dwProcessId;
}

int CUtility::GetProfileInt(const TCHAR * pcszKey, int nDefault)
{
	TCHAR szIniFile[MAX_PATH];
	this->GetIniFile(szIniFile);
	int nResult = ::GetPrivateProfileInt(PROFILE_APP_NAME, pcszKey, nDefault, szIniFile);
	return nResult;
}


BOOL CUtility::GetProfileStr(const TCHAR * pcszKey,
							 const TCHAR * pcszDefault,
							 TCHAR * pszValue,
							 int nValueSize)
{
	TCHAR szIniFile[MAX_PATH];
	this->GetIniFile(szIniFile);
	::GetPrivateProfileString(PROFILE_APP_NAME, pcszKey, pcszDefault, pszValue, nValueSize, szIniFile);
	return TRUE;
}


void CUtility::EnableLog(BOOL bEnable)
{
	if(m_bLogEnable == bEnable)
		return;

	if(bEnable)
	{
		m_hFile = ::CreateFile(m_szLogFile, GENERIC_WRITE, FILE_SHARE_WRITE|FILE_SHARE_READ, NULL,
			CREATE_ALWAYS, 0, NULL);
		if(m_hFile != INVALID_HANDLE_VALUE)
		{
			// Write unicode file header
			WORD dwHeader = 0xFEFF;
			DWORD dwWriteLen = 0;
			::WriteFile(m_hFile, &dwHeader, sizeof(dwHeader), &dwWriteLen, NULL);
		}
		m_bLogEnable = TRUE;
	}
	else
	{
		m_bLogEnable = FALSE;
		if(m_hFile != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(m_hFile);
			m_hFile = INVALID_HANDLE_VALUE;
		}
	}
}


void CUtility::Log(const TCHAR* pcszFormat, ...)
{
	if(!m_bLogEnable)
		return;

	CString strLog;
	va_list vl;
	va_start(vl, pcszFormat);
	strLog.FormatV(pcszFormat, vl);
	va_end(vl);

	CString strText;
	SYSTEMTIME st;
	::GetLocalTime(&st);
	strText.Format(_T("XPlayerCore[%02d:%02d:%02d.%03d][%04X]: %s\r\n"),
		st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, ::GetCurrentThreadId(), strLog);

	// Output
	if(m_hFile != INVALID_HANDLE_VALUE)
	{
		::EnterCriticalSection(&m_csLog);
		DWORD dwWriteLen = 0;
		::WriteFile(m_hFile, strText, strText.GetLength()* sizeof(TCHAR), &dwWriteLen, NULL);
		::LeaveCriticalSection(&m_csLog);
	}

#ifdef _DEBUG
	::OutputDebugString(strText);
#endif
}


BOOL CUtility::HaveLog(void)
{
	return m_bLogEnable;
}


void CUtility::GetVersionInfo(void)
{
	// Variables
	TCHAR szTemp[MAX_PATH];

	// Show date
	SYSTEMTIME st;
	::GetLocalTime(&st);
	this->Log(_T("Date: %04d-%02d-%02d"), st.wYear, st.wMonth, st.wDay);

	// Get OS Version
	OSVERSIONINFO osvi;
	memset(&osvi, 0, sizeof(osvi));
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	if(::GetVersionEx(&osvi))
	{
		const TCHAR * pcszOS = EMPTY_STRING;
		if(osvi.dwMajorVersion == 5)
		{
			if(osvi.dwMinorVersion == 0)
			{
				pcszOS = _T("OS: Windows 2000");
				m_OperationSystem = OS_WIN2K;
			}
			else
			{
				pcszOS = _T("OS: Windows XP");
				m_OperationSystem = OS_WINXP;
			}
		}
		else if(osvi.dwMajorVersion == 6)
		{
			pcszOS = _T("OS: Windows Vista or Windows 7");
			m_OperationSystem = OS_VISTAWIN7;
		}
		else
		{
			pcszOS = _T("OS: Other Windows (Windows31, Windows9X, WindowsNT");
			m_OperationSystem = OS_OTHER;
		}

		this->Log(_T("%s (%d.%d.%d)"),
			pcszOS, osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
	}
	else
		this->Log(_T("GetVersionEx failed, error = %d"), ::GetLastError());

	// Get IE Version
	lstrcpy(szTemp, _T("Unknown"));
	HKEY hKey = NULL;
	LONG lResult = ::RegOpenKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Internet Explorer"), &hKey);
	if(lResult == ERROR_SUCCESS)
	{
		DWORD dwType = REG_SZ;
		DWORD dwLength = sizeof(szTemp);
		lResult = ::RegQueryValueEx(hKey, _T("Version"), NULL, &dwType, (LPBYTE)szTemp, &dwLength);
		if(lResult == ERROR_SUCCESS)
			m_nIEVersion = _ttoi(szTemp);
		::RegCloseKey(hKey);
	}
	this->Log(_T("IE Version: (%s)"), szTemp);

	// Get Intel Media SDK Version
// 	lResult = ::RegOpenKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Intel\\MediaSDK\\Dispatch\\hw32-i2-1"), &hKey);
// 	if(lResult == ERROR_SUCCESS)
// 	{
// 		DWORD dwType = REG_SZ;
// 		DWORD dwLength = sizeof(szTemp);
// 		lResult = ::RegQueryValueEx(hKey, _T("Path"), NULL, &dwType, (LPBYTE)szTemp, &dwLength);
// 		if(lResult == ERROR_SUCCESS)
// 		{
// 			if(::IsFileExists(szTemp))
// 				m_bHaveIntelHardwareCodec = TRUE;
// 		}
// 		::RegCloseKey(hKey);
// 	}
//	this->Log(_T("Intel hardware codec: %d"), m_bHaveIntelHardwareCodec);

	// Get ZPlayer Version
	TCHAR szVersion[MAX_PATH];
	DWORD dwResult = ::GetModuleFileName(g_hInstance, szTemp, countof(szTemp)-1);
	if(dwResult > 0)
	{
		if(::GetFileVersionString(szTemp, szVersion))
			this->Log(_T("Player: %s (%s)"), szTemp, szVersion);
		else
			this->Log(_T("Get Player version failed: %s"), szTemp);
	}
	else
		this->Log(_T("Get Player file name failed, last error=%d"), (int)::GetLastError());

	// Get exe module name
	dwResult = ::GetModuleFileName(NULL, szTemp, _countof(szTemp)-1);
	if(dwResult > 0)
	{
		if(::GetFileVersionString(szTemp, szVersion))
			this->Log(_T("Application: %s (%s)"), szTemp, szVersion);
		else
			this->Log(_T("Get application version failed: %s"), szTemp);
	}
	else
		this->Log(_T("Get application file name failed, last error=%d"), (int)::GetLastError());
}


void CUtility::GetIniFile(TCHAR * pszIniFile)
{
	wsprintf(pszIniFile, _T("%s\\%s"), m_szModulePath, _T("FlyfoxLocalPlayer.ini"));
	if(!::IsFileExists(pszIniFile))
		wsprintf(pszIniFile, _T("%s\\%s"), m_szModulePath, _T("FlyfoxLocalPlayer.txt"));
}


