
#ifndef _UTILS_H_
#define _UTILS_H_

//////////////////////////////////////////////////////////////////////////

#define EMPTY_STRING	_T("")

enum OPERATION_SYSTEM
{
	OS_OTHER,
	OS_WIN2K,
	OS_WINXP,
	OS_VISTAWIN7,
};

////////////////////////////////////////////////////////////////////////////////////
// Enum macros defines
////////////////////////////////////////////////////////////////////////////////////
#define BeginEnumFilters(pFilterGraph, pEnumFilters, pBaseFilter)                                                  \
{                                                                                                                  \
	CComPtr<IEnumFilters> pEnumFilters;                                                                            \
	if (pFilterGraph && SUCCEEDED(pFilterGraph->EnumFilters(&pEnumFilters)))                                       \
{                                                                                                              \
	for (CComPtr<IBaseFilter> pBaseFilter; S_OK == pEnumFilters->Next(1, &pBaseFilter, 0); pBaseFilter = NULL) \
{

#define EndEnumFilters }}}

#define BeginEnumPins(pBaseFilter, pEnumPins, pPin)                                 \
{                                                                                   \
	CComPtr<IEnumPins> pEnumPins;                                                   \
	if (pBaseFilter && SUCCEEDED(pBaseFilter->EnumPins(&pEnumPins)))                \
{                                                                               \
	for (CComPtr<IPin> pPin; S_OK == pEnumPins->Next(1, &pPin, 0); pPin = NULL) \
{

#define EndEnumPins }}}

#define BeginEnumMediaTypes(pPin, pEnumMediaTypes, pMediaType)                                                      \
{                                                                                                                   \
	CComPtr<IEnumMediaTypes> pEnumMediaTypes;                                                                       \
	if (pPin && SUCCEEDED(pPin->EnumMediaTypes(&pEnumMediaTypes)))                                                  \
{                                                                                                               \
	AM_MEDIA_TYPE* pMediaType = NULL;                                                                           \
	for (; S_OK == pEnumMediaTypes->Next(1, &pMediaType, NULL); DeleteMediaType(pMediaType), pMediaType = NULL) \
{

#define EndEnumMediaTypes(pMediaType)                                                                         \
	}                                                                                                     \
	if (pMediaType)                                                                                           \
	DeleteMediaType(pMediaType);                                                                          \
	}                                                                                                         \
	}

#define BeginEnumSysDev(clsid, pMoniker)                                                                      \
{                                                                                                             \
	CComPtr<ICreateDevEnum> pDevEnum4$##clsid;                                                                \
	pDevEnum4$##clsid.CoCreateInstance(CLSID_SystemDeviceEnum);                                               \
	CComPtr<IEnumMoniker> pClassEnum4$##clsid;                                                                \
	if (SUCCEEDED(pDevEnum4$##clsid->CreateClassEnumerator(clsid, &pClassEnum4$##clsid, 0))                   \
	&& pClassEnum4$##clsid)                                                                               \
{                                                                                                         \
	for (CComPtr<IMoniker> pMoniker; pClassEnum4$##clsid->Next(1, &pMoniker, 0) == S_OK; pMoniker = NULL) \
{

#define EndEnumSysDev }}}


//////////////////////////////////////////////////////////////////////////

// DS
BOOL			GetFilterName(IBaseFilter * pFilter, CString& strFilterName);
IBaseFilter*	GetFilterFromPin(IPin* pPin);

// String functions
BOOL			StringFromList(TCHAR** ppList, const TCHAR* pcszSplitter, TCHAR* pszBuffer, int nBufferChars);
void			JumpSpace(const TCHAR** ppText);
void			TrimSpace(TCHAR* pText);
void			AddBeginSpace(TCHAR* pszText, int nTotalLength);
void			AddEndSpace(TCHAR* pszText, int nTotalLength);
void			CutEndSpace(TCHAR* pszText);
BOOL			AppendString(TCHAR** ppText, const TCHAR* pcszTextEnd, const TCHAR* pcszAppend);
BOOL			AppendNumber(TCHAR** ppText, const TCHAR* pcszTextEnd, int nNumber);
int				ReplaceString(TCHAR* pszText, const TCHAR* pcszOld, const TCHAR* pcszNew); // 该函数速度较慢
BOOL			GuidToString(const GUID& theGuid, TCHAR* pBuffer);	// Buffer size MAX_GUID_LENGTH
BOOL			StringToGuid(const TCHAR* pcszString, GUID* pGuid);

BOOL			MByteToWChar(LPCSTR lpcszSrc, LPWSTR lpwszDst, DWORD dwSize);
BOOL			WCharToMByte(LPCWSTR lpcwszSrc, LPSTR lpszDst, DWORD dwSize);

void			CStringToBin(CString str, CAtlArray<BYTE>& data);
CString			BinToCString(const BYTE* ptr, size_t len);

BOOL			MediaTypeToString(AM_MEDIA_TYPE* pMT, CString& strMediaType);


//
BOOL			GetModulePath(HMODULE hModule, TCHAR* pszPath);
const TCHAR*	GetFileNameFromPath(const TCHAR* pcszFilePath);
BOOL			IsFileExists(const TCHAR* pcszFileName);
BOOL			IsDirectoryExists(const TCHAR* pcszDirectory);
BOOL			GetFileVersionString(const TCHAR* pcszFileName, TCHAR* pszVersionString);
int				GetFileBuildNumber(const TCHAR* pcszFileName);
BOOL			GetFileDescription(const TCHAR* pcszFileName, TCHAR* pBuffer);

// Resource
TCHAR*			LoadResourceText(LPCTSTR pcszType, UINT uResourceId);
void			FreeResourceText(TCHAR* pText);

// Miscs
int				GetCaller(TCHAR * pszCaller);


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
struct MemoryData
{
	int		size;
	BYTE*	data;
};

//////////////////////////////////////////////////////////////////////////
// CAutoCriticalSection
//////////////////////////////////////////////////////////////////////////
class CAutoCriticalSection
{
public:
	CAutoCriticalSection(CRITICAL_SECTION* pCriticalSection, BOOL bBlock);
	~CAutoCriticalSection();
	BOOL				IsEnter(void);

protected:
	CRITICAL_SECTION*	m_pCriticalSection;
	BOOL				m_bBlock;
	BOOL				m_bEnter;
};

///////////////////////////////////////////////////////////
//	CSubclassHelper
///////////////////////////////////////////////////////////

#define MAX_SUBCLASS_ITEM	32

class CSubclassHelper
{
public:
	CSubclassHelper();
	~CSubclassHelper();

	int			GetItemCount(void);
	void *		GetItem(int nIndex);

	BOOL		AddItem(HWND hWnd, void * pObject);
	void		DeleteItem(HWND hWnd);
	void *		FindItem(HWND hWnd);

protected:
	typedef struct SUBCLASS_ITEM
	{
		HWND	hWnd;
		void *	pObject;
	};

protected:
	int					m_nItemCount;
	SUBCLASS_ITEM *		m_pItems[MAX_SUBCLASS_ITEM];
};

//////////////////////////////////////////////////////////////////////////
// CUtility
//////////////////////////////////////////////////////////////////////////
class CUtility
{
public:
	CUtility();
	~CUtility();
	BOOL				Initialize(void);
	void				UnInitialize(void);

	// System info
	LPCTSTR				GetExePath(void);
	LPCTSTR				GetExeName(void);
	LPCTSTR				GetModulePath(void);
	LPCTSTR				GetSystemPath(void);
	LPCTSTR				GetTemporaryPath(void);
	OPERATION_SYSTEM	GetOS(void);
	int					GetIEVersion(void);
	DWORD				GetProcessId(void);

	// Profile
	int					GetProfileInt(const TCHAR * pcszKey, int nDefault);
	BOOL				GetProfileStr(const TCHAR * pcszKey, const TCHAR * pcszDefault, TCHAR * pszValue, int nValueSize);
	// Log
	void				EnableLog(BOOL bEnable);
	BOOL				HaveLog(void);
	void				Log(const TCHAR * pcszFormat, ... );

protected:
	void				GetVersionInfo(void);
	void				GetIniFile(TCHAR * pszIniFile);

protected:
	// Generic variables
	TCHAR				m_szExePath[MAX_PATH];
	TCHAR				m_szExeName[MAX_PATH];
	TCHAR				m_szModulePath[MAX_PATH];
	TCHAR				m_szSystemPath[MAX_PATH];
	TCHAR				m_szTemporaryPath[MAX_PATH];
	OPERATION_SYSTEM	m_OperationSystem;
	int					m_nIEVersion;
	DWORD				m_dwProcessId;

	// Log settings
	BOOL				m_bLogEnable;
	TCHAR				m_szLogFile[MAX_PATH];
	HANDLE				m_hFile;
	CRITICAL_SECTION	m_csLog;
};

#endif