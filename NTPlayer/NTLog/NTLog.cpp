// NTLog.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "NTLog.h"
#include "SectionProtect.h"

#pragma warning(disable : 4996)

#define MAX_USERDATA_SIZE		(4096 - 1)	// 用户数据长
#define MAX_PRIVATEDATA_SIZE	256		    // NTLOG内部数据
#define MAX_LOGFILE_INDEX		999		    // 日志文件名使用的索引
#define MAX_HEX_DUMP_SIZE  		512
//no config, default
#define DEFAULT_LOGFILE_PATH				"C:\\NTLOG\\"
#define __TDEFAULT_LOGFILE_PATH				_T(DEFAULT_LOGFILE_PATH)
#define DEFAULT_MAX_LOGFILE_SIZE			20480
#define MAX_MAX_LOGFILE_SIZE				(DEFAULT_MAX_LOGFILE_SIZE * 100)
#define MIN_MAX_LOGFILE_SIZE				1
#define DEFAULT_MAX_LOGFILE_COUNT			20
#define MAX_MAX_LOGFILE_COUNT				1000
#define MIN_MAX_LOGFILE_COUNT				1
#define DEFAULT_MAX_HISTROY_PROCESS_COUNT	10
#define MAX_MAX_HISTROY_PROCESS_COUNT		100
#define MIN_MAX_HISTROY_PROCESS_COUNT		-1
#define KBTYE								1024

#define MAX_LEAVE_FUNNAME_LENGTH 256

#define OVERFLOW_CHECK(x)  \
    if((DWORD)-1 == x) \
    m_dwUsedSize = m_dwMaxLogDataSize;		
#define OPERATOR_XX_AUTOGUARD() \
    EnterCriticalSection(& _cs); \
    if(s_lOperatorStarted) \
    LeaveCriticalSection(& _cs); \
    else \
    InterlockedExchangeAdd(&s_lOperatorStarted,1L);

#define OUTPUTLOG_FORLEVEL(XLevel, pszFormat) \
    SectionProtect sp(&_cs); \
    va_list ap; \
    va_start(ap, pszFormat); \
    OutputLog(XLevel, pszFormat, ap); \
    va_end(ap);

static inline LONGLONG NTGetTickCount64(void)
{		
    __asm __emit 0x0F;
    __asm __emit 0x31;
}

//////////////////////////////////////////////////////////////////////////
HANDLE CNTLog::s_hMutex = NULL;
HANDLE CNTLog::s_hFileMap = NULL;
CNTLog* CNTLog::s_pThis = NULL;
HINSTANCE CNTLog::s_hInst = NULL;
TCHAR CNTLog::s_szModuleFileName[_MAX_PATH];
CRITICAL_SECTION CNTLog::s_cs;
DWORD CNTLog::s_dwTlsIndex = TLS_OUT_OF_INDEXES;
DWORD CNTLog::s_dwTlsIndex_this = TLS_OUT_OF_INDEXES;
CNTLog::CRelease CNTLog::s_release(1);
LONG32 CNTLog::s_lVer = NTLOG_VERSION;
const size_t CNTLog::CEnterLeaveFunCallBack::OFFSET_LEAVE_FUNCTION_FLAG = _tcslen(LEAVE_FUNCTION_FLAG);
HANDLE CNTLog::s_hLogFile = INVALID_HANDLE_VALUE;
DWORD CNTLog::s_dwLogFileCnt = 0;
INT32 CNTLog::s_nModuleID = 0;
volatile LONG CNTLog::s_lOperatorStarted = 0;


//////////////////////////////////////////////////////////////////////////
CNTLog::CNTLog()
{
    
}

CNTLog::~CNTLog()
{

}

//////////////////////////////////////////////////////////////////////////
void CNTLog::NTTrace(LPCTSTR pszFormat, ...)
{
    OUTPUTLOG_FORLEVEL(LEVEL_TRACE, pszFormat);
}

void CNTLog::NTDebug(LPCTSTR pszFormat, ...)
{
    OUTPUTLOG_FORLEVEL(LEVEL_DEBUG, pszFormat);
}

void CNTLog::NTInfo(LPCTSTR pszFormat, ...)
{
    OUTPUTLOG_FORLEVEL(LEVEL_INFO, pszFormat);
}

void CNTLog::NTWarn(LPCTSTR pszFormat, ...)
{
    OUTPUTLOG_FORLEVEL(LEVEL_WARN, pszFormat);
}

void CNTLog::NTError(LPCTSTR pszFormat, ...)
{
    OUTPUTLOG_FORLEVEL(LEVEL_ERROR, pszFormat);
}

void CNTLog::NTFatal(LPCTSTR pszFormat, ...)
{
    OUTPUTLOG_FORLEVEL(LEVEL_FATAL, pszFormat);
}

void CNTLog::NTLog(LPCTSTR pszLogFilePath,LPCTSTR pszFormat, ...)
{
    SectionProtect sp(&_cs); 
    if(NULL == pszLogFilePath && INVALID_HANDLE_VALUE != _hConfigFileChanged)
    {
        //not call by GetConfig, if (NULL == _hConfigFileChanged) output prompt log.
        static TCHAR szLogFilePath[_MAX_PATH] = {0};		
        LPCTSTR pszProcessName = NULL;
        if(NULL == pszProcessName)
            pszProcessName = GetCurrentModuleName();
        if('\0'== szLogFilePath[0])
            _sntprintf(szLogFilePath, _MAX_PATH, _T("%s%s.%s.log"), __TDEFAULT_LOGFILE_PATH, __TNTLOG_GROUP, pszProcessName);	
        pszLogFilePath = szLogFilePath;
    }
    va_list ap; 
    va_start(ap, pszFormat);
    OutputLog(LEVEL_PROMPT, pszFormat, ap,pszLogFilePath, NULL != pszLogFilePath);
    va_end(ap);	 
}

#pragma warning(push)
#pragma warning(disable : 4702 )
void CNTLog::NTHexDump(LPCVOID p, ULONG uSize, LPCTSTR pszPrompt, TLevel level)
{
#define MAX_HEX_DUMP_COUNT 16
#define MAX_ALL_HEX_DUMP_SIZE MAX_HEX_DUMP_COUNT * MAX_HEX_DUMP_SIZE
    if(uSize > MAX_ALL_HEX_DUMP_SIZE)
    {
        NTWarn(_T("Current HexDump is %d, Max HexDump Size is %d, Some of the Excess will be Cut."),uSize, MAX_ALL_HEX_DUMP_SIZE);
        uSize = MAX_ALL_HEX_DUMP_SIZE;		
    }
    EnterCriticalSection(& _cs);
    ULONG uNextSize = 0;
    static BYTE* pNext = NULL;
    static INT nIndex = 1;
    static INT nCount = 1;
    //va_list ap = (va_list)_ADDRESSOF(p);
    if(NULL == pNext)
    {
        nIndex = 1;
        nCount = (uSize - 1) / (MAX_HEX_DUMP_SIZE) + 1;
        TCHAR szDumpHead[MAX_HEX_DUMP_SIZE] = {0};
        _sntprintf(szDumpHead, MAX_HEX_DUMP_SIZE, _T("NTLOG MEMORY HEXDUMP Begin [0x%p], Total Lenght : [%lu] bytes, Prompt : %s"), p, uSize, pszPrompt);
        OutputLog(level, szDumpHead, NULL);
    }
    if(MAX_HEX_DUMP_SIZE < uSize)
    {
        uNextSize = uSize - (MAX_HEX_DUMP_SIZE);
        uSize = MAX_HEX_DUMP_SIZE;
        pNext = (BYTE*)p + uSize;		
    }
    else
    {
        uNextSize = 0;
        pNext = NULL;		
    }
    //static MEMORY_BASIC_INFORMATION info;
    //static SIZE_T dwBufferSize = 0;
    //dwBufferSize = VirtualQuery(p, &info, (SIZE_T)uSize);	
    static TCHAR szHex[MAX_PRIVATEDATA_SIZE + MAX_USERDATA_SIZE + 1] = {0};	
    ULONG cbWLineCount = 0;
    ULONG cbFLineSize = 0;
    ULONG cbDataSize = 0;
    ULONG i = 0;
    BYTE* pbyData = NULL;
    pbyData = (BYTE*)p;
    BYTE* pszData = NULL;
    pszData = (BYTE*)p;	
    cbWLineCount = uSize / 16;
    cbFLineSize  = uSize % 16;
    cbDataSize = 0;
    cbDataSize += _sntprintf(&szHex[cbDataSize], MAX_PRIVATEDATA_SIZE, _T("HEXDUMP As Follows(%d/%d): \r\n00000000\t00:00:00.000\t[0000] %s - HEXDUMP %d(0x%08x) bytes from [0x%p]\r\n"), nIndex++, nCount,
        pszPrompt ? pszPrompt : __TNTLOG_GROUP, uSize, uSize, p);
#define EXCEPTION_HANDLE() if(0 < uNextSize) { uNextSize = 0; pNext = NULL;} goto output
#define TOPRINTCHAR(X)  isprint((int)X) && X != '%' ? X : '.'
    for (i = 0; i < cbWLineCount; i++, pbyData += 16,pszData += 16)
    {
        __try
        {
            cbDataSize += _sntprintf(&szHex[cbDataSize], MAX_PRIVATEDATA_SIZE + MAX_USERDATA_SIZE - cbDataSize, 
                _T("00000000\t00:00:00.000\t[0000] 0x%p %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %3s %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\r\n"),pbyData,
                pbyData[0],pbyData[1],pbyData[2],pbyData[3],pbyData[4],pbyData[5],pbyData[6],pbyData[7],
                pbyData[8],pbyData[9],pbyData[10],pbyData[11],pbyData[12],pbyData[13],pbyData[14],pbyData[15],_T(""),
                TOPRINTCHAR(pszData[0]),TOPRINTCHAR(pszData[1]),TOPRINTCHAR(pszData[2]),TOPRINTCHAR(pszData[3]),
                TOPRINTCHAR(pszData[4]),TOPRINTCHAR(pszData[5]),TOPRINTCHAR(pszData[6]),TOPRINTCHAR(pszData[7]),
                TOPRINTCHAR(pszData[8]),TOPRINTCHAR(pszData[9]),TOPRINTCHAR(pszData[10]),TOPRINTCHAR(pszData[11]),
                TOPRINTCHAR(pszData[12]),TOPRINTCHAR(pszData[13]),TOPRINTCHAR(pszData[14]),TOPRINTCHAR(pszData[15])
                );
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            cbDataSize += _sntprintf(&szHex[cbDataSize], MAX_PRIVATEDATA_SIZE + MAX_USERDATA_SIZE - cbDataSize , _T("00000000\t00:00:00.000\t[0000] (!!!)Access Error, Invalid Memory Address : 0x%p, ExceptionCode : 0x%08x"), pbyData, GetExceptionCode());
            EXCEPTION_HANDLE();
            break;
        }
    }		
    for (i = 0, pbyData = (BYTE*)p + 16 * cbWLineCount; i < cbFLineSize; i++, pbyData++)
    {
        if(0 == i)
            cbDataSize +=_sntprintf(&szHex[cbDataSize], MAX_PRIVATEDATA_SIZE + MAX_USERDATA_SIZE - cbDataSize, _T("00000000\t00:00:00.000\t[0000] 0x%p "), pbyData);
        __try
        {
            cbDataSize += _sntprintf(&szHex[cbDataSize], MAX_PRIVATEDATA_SIZE + MAX_USERDATA_SIZE - cbDataSize, _T("%02x "), pbyData[0]);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            cbDataSize += _sntprintf(&szHex[cbDataSize], MAX_PRIVATEDATA_SIZE + MAX_USERDATA_SIZE - cbDataSize, _T("(!!!)Access Error, Invalid Memory Address : 0x%p, ExceptionCode : 0x%08x"), pbyData, GetExceptionCode());
            EXCEPTION_HANDLE();
            break;
        }
    }
    if(0 < cbFLineSize)
    {
        TCHAR szFormat[8];
        _sntprintf(szFormat, 7, _T("%%%ds "), (16 - cbFLineSize) * 3 + 3);
        cbDataSize += _sntprintf(&szHex[cbDataSize], MAX_PRIVATEDATA_SIZE + MAX_USERDATA_SIZE - cbDataSize, szFormat, _T(""));
    }
    for (i = 0, pszData = (BYTE*)p + 16 * cbWLineCount; i < cbFLineSize; i++, pszData++)
    {
        __try
        {
            cbDataSize += _sntprintf(&szHex[cbDataSize], MAX_PRIVATEDATA_SIZE + MAX_USERDATA_SIZE - cbDataSize, _T("%c"), TOPRINTCHAR(pszData[0]));
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            cbDataSize += _sntprintf(&szHex[cbDataSize], MAX_PRIVATEDATA_SIZE + MAX_USERDATA_SIZE - cbDataSize, _T("(!!!)Access Error, Invalid Memory Address : 0x%p, ExceptionCode : 0x%08x"), pbyData, GetExceptionCode());
            EXCEPTION_HANDLE();
            break;
        }
    }
output:
    OutputLog(level, szHex);
    //	va_end(ap);
    if(0 < uNextSize)
        NTHexDump(pNext, uNextSize, pszPrompt, level);
    LeaveCriticalSection(& _cs);	
}
#pragma warning(pop)

BOOL CNTLog::InstallNTLogIni(LPCTSTR szSrcIniAbsolutePath)
{
    SectionProtect sp(& _cs);
    struct _stat stSrcbuf;
    struct _stat stDestbuf;
    time_t tTime = time(NULL);
    struct tm* ptmTime = localtime(&tTime);
    if(0 != _tstat(szSrcIniAbsolutePath, &stSrcbuf))
    {
        NTPrompt(_T("Source File %s don't find."), szSrcIniAbsolutePath);
        return FALSE;
    }
    if(0 == _tstat(__TNTLOG_CONFIG_FILE_PATH, &stDestbuf))
    {
        if(stSrcbuf.st_mtime <= stDestbuf.st_mtime) //只针对有更新文件时，才复制
        {
            NTPrompt(_T("Don't Update NTLog Config File. Source File %s Last Modify Time = %ld, Target File %s Last Modify Time = %ld"),szSrcIniAbsolutePath , stSrcbuf.st_mtime, __TNTLOG_CONFIG_FILE_PATH, stDestbuf.st_mtime );
            return FALSE;
        }
        TCHAR szTargetFilePath[_MAX_PATH] = {0};
        TCHAR szTime[40] = {0};
        _sntprintf(szTime, sizeof(szTime) / sizeof(TCHAR), _T("%4d-%02d-%02d") , 1900 + ptmTime->tm_year, 1 + ptmTime->tm_mon ,ptmTime->tm_mday);
        _sntprintf(szTargetFilePath, sizeof(szTargetFilePath)/ sizeof(TCHAR), _T("%s(%s)"), __TNTLOG_CONFIG_FILE_PATH, szTime);
        if(INVALID_FILE_ATTRIBUTES != GetFileAttributes(szTargetFilePath))
            DeleteFile(szTargetFilePath);
        _trename(__TNTLOG_CONFIG_FILE_PATH, szTargetFilePath);
        NTPrompt(_T("Rename Old ConfigFile %s to Other File %s, ErrorCode : %d(0x%08x)"),__TNTLOG_CONFIG_FILE_PATH , szTargetFilePath, GetLastError());
    }
    else	//路径不存在，重建
    {
        LPSTR pszConfigDirA = NULL;
#if defined(UNICODE) || defined(_UNICODE)
        std::string strConfigDir;
        UnicodeToMultiByte(__TNTLOG_CONFIG_DIR, strConfigDir);
        pszConfigDirA = (LPSTR)strConfigDir.c_str();
#else
        pszConfigDirA = __TNTLOG_CONFIG_DIR;
#endif
        if(!MakeDirectoryA(pszConfigDirA))
        {
            NTPrompt(_T("Fail to Make Directory %s"), __TNTLOG_CONFIG_DIR);
            return FALSE;
        }
    }
    BOOL bRet = CopyFile(szSrcIniAbsolutePath, __TNTLOG_CONFIG_FILE_PATH, FALSE );
    NTPrompt(_T("Copy Source File %s to Target File %s, ErrorCode : %d(0x%08x)"),szSrcIniAbsolutePath, __TNTLOG_CONFIG_FILE_PATH, GetLastError());
    return bRet;
}

//////////////////////////////////////////////////////////////////////////
CNTLog& CNTLog::operator << (const bool _Val)
{
	OPERATOR_XX_AUTOGUARD();
	if(m_dwUsedSize < m_dwMaxLogDataSize)
	{
		int n = 0;
		if(_Val)
			n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, (LPCTSTR)_T("true"));
		else
			n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, (LPCTSTR)_T("false"));
		if(0 < n)
			m_dwUsedSize += n;
		OVERFLOW_CHECK(n);
	}		
	return *this;
}

//字符
CNTLog& CNTLog::operator << (const TCHAR _Val)
{
	OPERATOR_XX_AUTOGUARD();
	if(m_dwUsedSize < m_dwMaxLogDataSize)
	{
		int n = 0;
		if(isprint((int)_Val))
			m_dwUsedSize += _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("%c"), _Val);
		else
			m_dwUsedSize += _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("%d"), (INT)_Val);
		if(0 < n)
			m_dwUsedSize += n;
		OVERFLOW_CHECK(n);
	}		
	return *this;
}

#if (defined(UNICODE) || defined(_UNICODE))
CNTLog& CNTLog::operator << (const char _Val)
#else
CNTLog& CNTLog::operator << (const wchar_t  _Val)
#endif
{
	OPERATOR_XX_AUTOGUARD();
	if(m_dwUsedSize < m_dwMaxLogDataSize)
	{
		int n = 0;
		if(isprint((int)_Val))
			n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("%C"), _Val);
		else
			n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("%d"), (INT)_Val);
		if(0 < n)
			m_dwUsedSize += n;
		OVERFLOW_CHECK(n);
	}
	return *this;
}

CNTLog& CNTLog::operator << (const signed char _Val)
{
	OPERATOR_XX_AUTOGUARD();
	if(m_dwUsedSize < m_dwMaxLogDataSize)
	{
		int n = 0;
		n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("%u"), (INT)_Val);
		if(0 < n)
			m_dwUsedSize += n;
		OVERFLOW_CHECK(n);
	}
	return *this;
}

CNTLog& CNTLog::operator << (const unsigned char _Val)
{
	OPERATOR_XX_AUTOGUARD();
	if(m_dwUsedSize < m_dwMaxLogDataSize)
	{
		int n = 0;
		n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("%u"), (UINT)_Val);
		if(0 < n)
			m_dwUsedSize += n;
		OVERFLOW_CHECK(n);
	}
	return *this;
}

//2字节整数
CNTLog& CNTLog::operator << (const signed short _Val)
{
	OPERATOR_XX_AUTOGUARD();
	if(m_dwUsedSize < m_dwMaxLogDataSize)
	{
		int n = 0;
		n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("%hd"), _Val);
		if(0 < n)
			m_dwUsedSize += n;
		OVERFLOW_CHECK(n);
	}
	return *this;
}

#if (defined _WCHAR_T_DEFINED && defined _NATIVE_WCHAR_T_DEFINED)
//与 operator <<(WCHAR)冲突
CNTLog& CNTLog::operator << (const unsigned short _Val)
{	
	OPERATOR_XX_AUTOGUARD();
	if(m_dwUsedSize < m_dwMaxLogDataSize)
	{
		int n = 0;
		n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("%hu"), _Val);
		if(0 < n)
			m_dwUsedSize += n;
		OVERFLOW_CHECK(n);
	}
	return *this;
}
#endif

//四字节整数
/*
//#if (defined _WIN64)
CNTLog& CNTLog::operator << (const  unsigned int _W64 _Val)
{
	EnterCriticalSection(&_cs);
	if(0 < m_dwUsedSize)
		LeaveCriticalSection(&_cs);
	if(m_dwUsedSize < m_dwMaxLogDataSize)
	{
		m_dwUsedSize += _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("%I64u"), _Val);
		OVERFLOW_CHECK();
	}
	return *this;
}
//#endif
*/

CNTLog& CNTLog::operator << (const  signed int  _Val)
{
	OPERATOR_XX_AUTOGUARD();
	if(m_dwUsedSize < m_dwMaxLogDataSize)
	{
		int n = 0;
		n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("%d"), _Val);
		if(0 < n)
			m_dwUsedSize += n;
		OVERFLOW_CHECK(n);
	}
	return *this;
}	

CNTLog& CNTLog::operator << (const unsigned int  _Val)
{
	OPERATOR_XX_AUTOGUARD();
	if(m_dwUsedSize < m_dwMaxLogDataSize)
	{
		int n = 0;
		n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("%u"), _Val);
		if(0 < n)
			m_dwUsedSize += n;
		OVERFLOW_CHECK(n);
	}
	return *this;
}

CNTLog& CNTLog::operator << (const long  _Val)
{
	OPERATOR_XX_AUTOGUARD();
	if(m_dwUsedSize < m_dwMaxLogDataSize)
	{
		int n = 0;
		n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("%ld"), _Val);
		if(0 < n)
			m_dwUsedSize += n;
		OVERFLOW_CHECK(n);
	}
	return *this;
}

CNTLog& CNTLog::operator << (const unsigned long  _Val)
{
	OPERATOR_XX_AUTOGUARD();
	if(m_dwUsedSize < m_dwMaxLogDataSize)
	{
		int n = 0;
		n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("%lu"), _Val);
		if(0 < n)
			m_dwUsedSize += n;
		OVERFLOW_CHECK(n);
	}
	return *this;
}

CNTLog& CNTLog::operator << (const signed __int64  _Val)
{
	OPERATOR_XX_AUTOGUARD();
	if(m_dwUsedSize < m_dwMaxLogDataSize)
	{
		int n = 0;
		n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("%I64d"), _Val);
		if(0 < n)
			m_dwUsedSize += n;
		OVERFLOW_CHECK(n);
	}
	return *this;
}

CNTLog& CNTLog::operator << (const  unsigned __int64 _Val)
{
	OPERATOR_XX_AUTOGUARD();
	if(m_dwUsedSize < m_dwMaxLogDataSize)
	{
		int n = 0;
		n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("%I64u"), _Val);
		if(0 < n)
			m_dwUsedSize += n;
		OVERFLOW_CHECK(n);
	}
	return *this;
}

//float
CNTLog& CNTLog::operator << (const float _Val)
{
	OPERATOR_XX_AUTOGUARD();
	if(m_dwUsedSize < m_dwMaxLogDataSize)
	{
		int n = 0;
		n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("%f"), _Val);
		if(0 < n)
			m_dwUsedSize += n;
		OVERFLOW_CHECK(n);
	}
	return *this;
}

//double
CNTLog& CNTLog::operator << (const double _Val)
{
	OPERATOR_XX_AUTOGUARD();
	if(m_dwUsedSize < m_dwMaxLogDataSize)
	{
		int n = 0;
		n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("%f"), _Val);
		if(0 < n)
			m_dwUsedSize += n;
		OVERFLOW_CHECK(n);
	}
	return *this;
}

//long double
CNTLog& CNTLog::operator << (const long double _Val)  
{
	OPERATOR_XX_AUTOGUARD();
	if(m_dwUsedSize < m_dwMaxLogDataSize)
	{
		int n = 0;
		n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("%lf"), _Val);
		if(0 < n)
			m_dwUsedSize += n;
		OVERFLOW_CHECK(n);
	}
	return *this;
}

CNTLog& CNTLog::operator << (const void * _Val)
{
	OPERATOR_XX_AUTOGUARD();
	if(m_dwUsedSize < m_dwMaxLogDataSize)
	{
		int n = 0;
		n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("0x%p"), _Val);
		if(0 < n)
			m_dwUsedSize += n;
		OVERFLOW_CHECK(n);
	}
	return *this;
}

//字符串
CNTLog& CNTLog::operator << (const TCHAR * _Val) throw()
{
	OPERATOR_XX_AUTOGUARD();
	if(m_dwUsedSize < m_dwMaxLogDataSize)
	{
		int n = 0;
		__try
		{
			n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("%s"), _Val);
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			n =_sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("\t(!!!)Access Error, Invalid Memory Address : 0x%p, ExceptionCode : 0x%08x\t"), _Val, GetExceptionCode());
		}
		if(0 < n)
			m_dwUsedSize += n;
		OVERFLOW_CHECK(n);
	}
	return *this;
}

#if (defined(UNICODE) || defined(_UNICODE))
CNTLog& CNTLog::operator << (const char * _Val) throw()
{
	OPERATOR_XX_AUTOGUARD();
	if(m_dwUsedSize < m_dwMaxLogDataSize)
	{
		WCHAR szBufferW[MAX_USERDATA_SIZE + 1] = {0};
		INT nLen = 0;
		UINT code_page = GetStringCodePage(_Val);
		__try
		{
			nLen = MultiByteToWideChar(code_page  ,0,_Val,-1,NULL,0);
			if(0 == nLen)
			{
				code_page = 0;
				nLen = MultiByteToWideChar(code_page  ,0,_Val,-1,NULL,0);
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{	
			nLen = 0;
			int n = 0;
			m_dwUsedSize +=_sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("\t(!!!)Access Error, Invalid Memory Address : 0x%p, ExceptionCode : 0x%08x\t"), _Val, GetExceptionCode());				
			if(0 < n)
				m_dwUsedSize += n;
			OVERFLOW_CHECK(n);
		}
		if(0 < nLen)
		{
			if(MAX_USERDATA_SIZE + 1 < nLen)
			{
				CHAR szLogDataW[MAX_USERDATA_SIZE + 1] = {0};
				memcpy(szLogDataW, _Val, MAX_USERDATA_SIZE * sizeof(CHAR));
				_Val = szLogDataW;
				nLen = MAX_USERDATA_SIZE + 1;
				nLen = MultiByteToWideChar(code_page, 0,_Val, -1, szBufferW,nLen);
				if(0 == nLen)
				{
					code_page = 0;
					nLen = MultiByteToWideChar(code_page, 0,_Val, -1, szBufferW,nLen);
				}
			}
			else
			{
				nLen = MultiByteToWideChar(code_page, 0,_Val, -1, szBufferW,nLen);
				if(0 == nLen)
				{
					code_page = 0;
					nLen = MultiByteToWideChar(code_page, 0,_Val, -1, szBufferW,nLen);
				}
			}
			if(0 < nLen)
			{
				int n = 0;
				__try
				{
					n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("%s"), szBufferW);
				}
				__except(EXCEPTION_EXECUTE_HANDLER)
				{
					n =_sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("\t(!!!)Access Error, Invalid Memory Address : 0x%p, ExceptionCode : 0x%08x\t"), szBufferW, GetExceptionCode());
				}
				if(0 < n)
					m_dwUsedSize += n;
				OVERFLOW_CHECK(n);
			}
		}
	}
	return *this;
}
#else
CNTLog& CNTLog::operator << (const wchar_t * _Val) throw()
{
	OPERATOR_XX_AUTOGUARD();
	if(m_dwUsedSize < m_dwMaxLogDataSize)
	{
		CHAR szBufferA[MAX_USERDATA_SIZE + 1] = {0};	//调用频繁
		INT nLen = 0;
		__try
		{
			nLen = WideCharToMultiByte(936, 0, _Val, -1,NULL, 0 ,NULL, NULL);
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			nLen = 0;
			int n = 0;
			n =_sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("\t(!!!)Access Error, Invalid Memory Address : 0x%p, ExceptionCode : 0x%08x\t"), _Val, GetExceptionCode());
			if(0 < n)
				m_dwUsedSize += n;
			OVERFLOW_CHECK(n);
		}
		if(0 < nLen)
		{
			if(MAX_USERDATA_SIZE + 1 < nLen)
			{
				WCHAR szLogDataW[MAX_USERDATA_SIZE + 1] = {0};
				memcpy(szLogDataW, _Val, MAX_USERDATA_SIZE * sizeof(WCHAR));
				_Val = szLogDataW;
				nLen = MAX_USERDATA_SIZE + 1;
				nLen = WideCharToMultiByte(936, 0, _Val, -1, szBufferA, nLen,NULL,NULL);
				if(0 == nLen)
				{

					nLen = WideCharToMultiByte(936, 0, _Val, -1, szBufferA, nLen,NULL,NULL);
				}
			}
			else
				nLen = WideCharToMultiByte(936, 0, _Val, -1, szBufferA, nLen,NULL,NULL);
			if(0 < nLen)
			{
				int n = 0;
				__try
				{
					n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("%s"), szBufferA);
				}
				__except(EXCEPTION_EXECUTE_HANDLER)
				{
					n =_sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("\t(!!!)Access Error, Invalid Memory Address : 0x%p, ExceptionCode : 0x%08x\t"), szBufferA, GetExceptionCode());
				}
				if(0 < n)
					m_dwUsedSize += n;
				OVERFLOW_CHECK(n);
			}
		}
	}
	return *this;
}
#endif

//IID
CNTLog& CNTLog::operator << (REFGUID _Val)  throw()
{
	OPERATOR_XX_AUTOGUARD();
	if(m_dwUsedSize < m_dwMaxLogDataSize)
	{
		OLECHAR szGUID[40];
		HRESULT hr = S_OK;
		hr = StringFromGUID2(_Val, szGUID, 40);
		if(SUCCEEDED(hr))
			this->operator << (szGUID);
		else
		{
			this->operator << ("{StringFromIID ERROR}");                                
		}
	}
	return *this;
}

//tagVARIANT
CNTLog& CNTLog::operator << (const tagVARIANT _Val)  throw()
{
	OPERATOR_XX_AUTOGUARD();
	if(m_dwUsedSize < m_dwMaxLogDataSize)
	{
		int n = 0;
		const tagVARIANT* pVar = &_Val;
		__try
		{
			switch (pVar->vt)
			{
			case VT_EMPTY:
				n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("{VT_EMPTY}"));
				break;
			case VT_NULL:
				n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("{VT_NULL}"));
				break;
			case VT_ERROR:
				n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("{0x%08x(%d) VT_ERROR}"), pVar->scode, pVar->scode);
				break;
			case VT_I1:
				n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("{0x%08x(%d) VT_I1}"), (INT)pVar->bVal, (INT)pVar->bVal);
				break;
			case VT_I2:
				n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("{0x%08x(%d) VT_I2}"), (INT)pVar->iVal, (INT)pVar->iVal);
				break;
			case VT_I4:
				n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("{0x%08x(%d) VT_I4}"), (INT)pVar->intVal, (INT)pVar->intVal);
				break;
			case VT_I8:
				n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("{0x%I64x(%I64d) VT_I8}"), (ULONGLONG)pVar->ullVal, (LONGLONG)pVar->llVal);
				break;
			case VT_UI1:
				n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("{0x%08x(%u) VT_UI1}"), (UINT)pVar->bVal, (UINT)pVar->bVal);
				break;
			case VT_UI2:
				n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("{0x%08x(%u) VT_UI2}"), (UINT)pVar->iVal, (UINT)pVar->iVal);
				break;
			case VT_UI4:
				n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("{0x%08x(%u) VT_UI4}"), (UINT)pVar->intVal, (UINT)pVar->intVal);
				break;
			case VT_UI8:
				n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("{0x%I64x(%I64u) VT_UI8}"), (ULONGLONG)pVar->ullVal, (LONGLONG)pVar->llVal);
			case VT_BOOL:
				n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("{0x%08x(%d) VT_BOOL}"), (INT)pVar->intVal, (INT)pVar->intVal);
				break;
			case VT_INT:
				n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("{0x%08x(%d) VT_INT}"), (INT)pVar->intVal, (INT)pVar->intVal);
			case VT_UINT:
				n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("{0x%08x(%u) VT_UINT}"), (UINT)pVar->intVal, (UINT)pVar->intVal);
			case VT_HRESULT:
				n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("{0x%08x(%d) VT_HRESULT}"), (UINT)pVar->intVal, (INT)pVar->intVal);
				break;
			case VT_R8:
				n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("{%f VT_R8}"), pVar->dblVal);
				break;
			case VT_BSTR:
				(*this)<<"{L\""<<(pVar->bstrVal)<<"\" VT_BSTR}";
				break; 
			case VT_LPWSTR:
				(*this)<<"{L\""<<(pVar->bstrVal)<<"\" VT_LPWSTR}";
				break;
			case VT_LPSTR:
				n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("{\"%s\" VT_LPSTR}"), pVar->pcVal);
				break;
			case VT_DISPATCH:
				n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("{0x%p VT_DISPATCH}"), pVar->pdispVal);
				break;
			default:
				n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize, _T("{0x%I64x(%I64d) VT_?[0x%08x(%d)]}"), (ULONGLONG)pVar->ullVal , (LONGLONG)pVar->llVal, pVar->vt, pVar->vt);
				break;
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			n = _sntprintf(&m_pszLogDataA[m_dwUsedSize], m_dwMaxLogDataSize - m_dwUsedSize,_T("{VT_?[0x%08x(%d)], (!!!)Access Error}"), pVar->vt, pVar->vt);
		}
		if(0 < n)
			m_dwUsedSize += n;
		OVERFLOW_CHECK(n);
	}
	return *this;
}

//std::string
CNTLog& CNTLog::operator << (const std::basic_string<WCHAR> _Val)  throw()	
{
	this->operator << (_Val.c_str());
	return *this;
}

CNTLog& CNTLog::operator << (const std::basic_string<CHAR> _Val)  throw()	
{
	this->operator << (_Val.c_str());
	return *this;
}

//从缓冲到输出
void CNTLog::NTLog4CXX_Flush(const TLevel level)
{
	s_lOperatorStarted = 0;
	m_dwUsedSize = 0;
	OutputLog(level, &m_pszLogDataA[0]);
	LeaveCriticalSection(&_cs);
}



//////////////////////////////////////////////////////////////////////////
// static
//////////////////////////////////////////////////////////////////////////
CNTLog& CNTLog::GetInstance(LPCTSTR pszCallFunName, LPCVOID pvThis)
{
    if( s_pThis && s_pThis->_lRef >0 )//多线程启动时，同步等 InterlockedExchange( &s_pThis->_nRef, 1 );完成
    {
        TlsSetValue(s_dwTlsIndex, (LPVOID)pszCallFunName);
        TlsSetValue(s_dwTlsIndex_this, (LPVOID)pvThis);
        return *s_pThis;
    }
    if (s_release.m_bIsModuleUnloading)
    {
        MessageBox(NULL, _T("启动异常"), _T("错误"), MB_OK);
        TerminateProcess(GetCurrentProcess(), (UINT)-1);
        return *s_pThis;
        //ExitThread((UINT)-1);
    }
    SectionProtect sp(&s_cs);
    if( s_pThis )
    {
        TlsSetValue(s_dwTlsIndex, (LPVOID)pszCallFunName);
        TlsSetValue(s_dwTlsIndex_this, (LPVOID)pvThis);
        return *s_pThis;
    }
    s_hInst = reinterpret_cast<HINSTANCE>(GetCurrentModuleHandle());
    ::GetModuleFileName(s_hInst, s_szModuleFileName, MAX_PATH - 1);
    TCHAR szMutex[MAX_PATH] = _T("#mutexNTLOG") __TNTLOG_GROUP;
    TCHAR szFileMap[MAX_PATH] = _T("#filemapNTLOG") __TNTLOG_GROUP;
    DWORD dwPID = GetCurrentProcessId();
    _sntprintf( szMutex + _tcslen(szMutex), MAX_PATH/2, _T("%d"), dwPID);
    _sntprintf( szFileMap + _tcslen(szFileMap),MAX_PATH/2 , _T("%d"), dwPID);
    s_hMutex = CreateMutex(NULL, true, szMutex);
    if(NULL == s_hMutex)
    {
        ::MessageBox(NULL, _T("CreateMutex failure"), s_szModuleFileName, MB_OK | MB_ICONERROR);
        ::ExitProcess((UINT)-1);
    }
    bool bExist = (ERROR_ALREADY_EXISTS == ::GetLastError());
    if(bExist)
        ::WaitForSingleObject(s_hMutex, INFINITE);				// 等待第一个模块把本单一实例创建完成，或Release完成
    s_hFileMap = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(CNTLog), szFileMap);
    if( NULL == s_hFileMap )
    {
        ::MessageBox( NULL, _T("CreateFileMapping failure"), s_szModuleFileName, MB_OK | MB_ICONERROR );
        ::ReleaseMutex( s_hMutex );
        ::CloseHandle( s_hMutex );
        s_hMutex = NULL;
        ::ExitProcess((UINT)-1);
    }
    bExist = (ERROR_ALREADY_EXISTS == ::GetLastError());
    // 如果第一个模块刚创建单一实例，又立即Release，会导致第二个模块hMutex已存在，但hFileMap已关闭的情况。此时，第二个模块当作新建单一实例	 
    s_pThis = (CNTLog*) ::MapViewOfFile(s_hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(CNTLog));		
    if(NULL == s_pThis)
    {
        ::MessageBox(NULL, _T("MapViewOfFile failure"), s_szModuleFileName, MB_OK | MB_ICONERROR);
        ::CloseHandle( s_hFileMap );
        s_hFileMap = NULL;
        ::ReleaseMutex( s_hMutex );
        ::CloseHandle( s_hMutex );
        s_hMutex = NULL;
        ::ExitProcess((UINT)-1);
    }
    TlsSetValue(s_dwTlsIndex, (LPVOID)pszCallFunName);
    TlsSetValue(s_dwTlsIndex_this, (LPVOID)pvThis);
    s_release.m_bIsModuleUnloading = FALSE; //禁止 Release 优化,这一行非写不可,2当模块退出时，不检测日志是否被更改
    if( bExist )
    {
        if(MAX_USERDATA_SIZE + 1 > s_pThis->m_dwMaxLogDataSize)
        {
            HGLOBAL hGlobal = (HGLOBAL)s_pThis->m_pszLogDataA;
            if(hGlobal)
            {
                GlobalUnlock(hGlobal);
                GlobalFree(hGlobal);
                hGlobal = NULL;
                s_pThis->m_dwMaxLogDataSize = 0;
                s_pThis->m_pszLogDataA = NULL;
            }
            hGlobal = GlobalAlloc(GMEM_FIXED, (MAX_USERDATA_SIZE + 1) * sizeof(WCHAR));
            s_pThis->m_pszLogDataA = (LPTSTR)GlobalLock(hGlobal);
            if(s_pThis->m_pszLogDataA)
            {
                memset(s_pThis->m_pszLogDataA, 0x00, (MAX_USERDATA_SIZE + 1) * sizeof(WCHAR));
                s_pThis->m_dwMaxLogDataSize = MAX_USERDATA_SIZE + 1;
            }
        }
    }
    else
    {
        LONG32 size = (LONG32)sizeof(CNTLog);
        memset(s_pThis, 0x00, size);
        HGLOBAL hGlobal = GlobalAlloc(GMEM_FIXED, (MAX_USERDATA_SIZE + 1) * sizeof(WCHAR));
        s_pThis->m_pszLogDataA = (LPTSTR)GlobalLock(hGlobal);
        if(s_pThis->m_pszLogDataA)
        {
            memset(s_pThis->m_pszLogDataA, 0x00, (MAX_USERDATA_SIZE + 1) * sizeof(WCHAR));
            s_pThis->m_dwMaxLogDataSize = MAX_USERDATA_SIZE + 1;
        }
        ::InitializeCriticalSection( &s_pThis->_cs );
        s_pThis->_lVer = NTLOG_VERSION;
        s_pThis->_cbThis  = size;
        s_pThis->ResetParams(TRUE);
        s_pThis->GetConfig(TRUE);
    }
    ::InterlockedExchangeAdd((volatile LONG*)&s_pThis->_lRef, 1L );
    s_nModuleID = s_pThis->_lRef;	
    s_dwLogFileCnt = (INVALID_HANDLE_VALUE == s_pThis->m_hLogFile && s_pThis->m_bFileLog) ? s_pThis->m_dwAllLogFileCnt + 1 : s_pThis->m_dwAllLogFileCnt;
    s_pThis->PrintModuleInit();	
    ::ReleaseMutex( s_hMutex );
    return *s_pThis;
}

BOOL CNTLog::IsModuleUnloading(LPCTSTR szFunction)
{
    if(s_release.m_bIsModuleUnloading)
    {
        TCHAR szDebugViewMsg[MAX_PRIVATEDATA_SIZE] = {0}; //module exit, so alloc memory from stack 
        _sntprintf(szDebugViewMsg, MAX_PRIVATEDATA_SIZE, _T("[%d] <WARN> <%s> "), GetCurrentThreadId(), CNTLog::GetCurrentModuleName());
        std::basic_string<TCHAR> strFunction = (std::basic_string<TCHAR>)szDebugViewMsg + (std::basic_string<TCHAR>) _T("[") + (std::basic_string<TCHAR>)szFunction +_T("]") +_T(" Module Unloading!!!\n");
        OutputDebugString(strFunction.c_str());
    }
    return s_release.m_bIsModuleUnloading;
}

BOOL CNTLog::IsLogValid(CNTLog::TLevel l)
{
    return (s_pThis->m_eLevel<=l && (s_pThis->m_bDebugViewLog || s_pThis->m_bFileLog));
}

BOOL CNTLog::IsDebugValid()
{
    return (s_pThis->m_eLevel<= LEVEL_DEBUG && (s_pThis->m_bDebugViewLog || s_pThis->m_bFileLog)); 
}

BOOL CNTLog::IsInfoValid()
{
    return (s_pThis->m_eLevel<= LEVEL_INFO && (s_pThis->m_bDebugViewLog || s_pThis->m_bFileLog));
}

BOOL CNTLog::IsWarnValid()
{
    return (s_pThis->m_eLevel<= LEVEL_WARN && (s_pThis->m_bDebugViewLog || s_pThis->m_bFileLog));
}

BOOL CNTLog::IsErrorValid()
{
    return (s_pThis->m_eLevel<= LEVEL_ERROR && (s_pThis->m_bDebugViewLog || s_pThis->m_bFileLog));
}

BOOL CNTLog::IsFatalValid()
{
    return (s_pThis->m_eLevel<= LEVEL_FATAL && (s_pThis->m_bDebugViewLog || s_pThis->m_bFileLog));
}

BOOL CNTLog::IsTRACEValid()
{
    return (s_pThis && (s_pThis->m_eLevel <= LEVEL_TRACE) && (s_pThis->m_bDebugViewLog || s_pThis->m_bFileLog));
}

UINT CNTLog::GetStringCodePage(const char* szData)
{
	UINT code_page = 0;//default value
	if(NULL == szData || s_release.m_dwMainThreadID != GetCurrentThreadId()) //is not main thread , can not print
		return code_page;
	IMultiLanguage2*  pMLang = NULL;
	HRESULT hr = S_FALSE;
	HRESULT hresult = S_FALSE;    
	int srclen = (int)strlen(szData); 
	DetectEncodingInfo dei[8] = {0};		
	int dei_num = 8;
	INT confidence = 0;
	typedef HRESULT (STDAPICALLTYPE *_pfnDllGetClassObject)(REFCLSID , REFIID , LPVOID*);
	_pfnDllGetClassObject pfnDllGetClassObject = (_pfnDllGetClassObject)GetProcAddress(s_release.m_hLang, "DllGetClassObject");
	__try
	{	 
		if(pfnDllGetClassObject)
		{
			CoInitialize(NULL);
			IClassFactory* pCF = NULL;
			IMultiLanguage2* pMLang = NULL;
			hr = (*pfnDllGetClassObject)(CLSID_CMultiLanguage, IID_IClassFactory, (LPVOID*) &pCF);
			if(pCF && SUCCEEDED(hr))
			{
				hr = pCF->CreateInstance(NULL, IID_IMultiLanguage2, (void **)&pMLang);	
			}
			if(pMLang && SUCCEEDED(hr))
			{
				hresult = pMLang->DetectInputCodepage(MLDETECTCP_NONE, 0 , (char *)szData, &srclen , dei , &dei_num);  
				if (SUCCEEDED(hresult))
				{
					confidence = 50;
					for (int i = 0 ; i < dei_num; i++)
					{
						code_page = dei[i].nCodePage;
						if(936 == code_page)
							break;
						else if(confidence < dei[i].nConfidence)
						{
							confidence = dei[i].nConfidence;						
						}
					}
				}
				pMLang->Release();
			}
			CoUninitialize();
		}//pfnDllGetClassObject
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		s_pThis->NTPrompt(_T("IMultiLanguage2 Fatal. code_page = %lu, datalen = %d, CoCreateInstance = %lu, pMLang = %lu, dei_num = %d, confidence = %d, DetectInputCodepage = %d "), 
			code_page, srclen, hr, pMLang, dei_num, confidence, hresult);
	}
	return code_page;
}

LPCTSTR CNTLog::TraceAuto(LPCSTR pszFuncSig, DWORD dwEBP, BOOL bIsStatic)
{	
    if(s_release.m_bIsModuleUnloading)
    {
        return NULL;
    }
    EnterCriticalSection(&s_pThis-> _cs);
    static std::string ntlog_strDestFuncSig;			//保存返回结果必须静态
    LPCTSTR ntlog_ptszDestFuncSig = NULL;		//保存返回结果
    ntlog_strDestFuncSig = pszFuncSig;
    if(CNTLog::IsTRACEValid())
    {
        std::vector<stParamTypeInfo> ntlog_vecIndex2TypeName;//第一个存绝对位置, 第二个开始都是相对第一个的位置
        CNTLog::CALLTYPE_NTLOG ntlog_ct = CT_OTHERCALL;
        BOOL ntlog_bRet = FALSE;
        int ntlog_nSize = 0;
        BOOL ntlog_bRetClass = FALSE;
        ntlog_vecIndex2TypeName.clear();
        ntlog_bRet = CNTLog::GetParamInfo(pszFuncSig,bIsStatic, ntlog_ct, ntlog_vecIndex2TypeName, ntlog_bRetClass);
        ntlog_nSize = (INT)ntlog_vecIndex2TypeName.size();
        if(ntlog_bRet)
        {
            INT ntlog_nIndex = 0;		  
            INT ntlog_nPos	= 0;		 
            INT ntlog_nLen	= 0;		  
            DWORD ntlog_dwRegValue  = 0;   
            DWORD ntlog_nRegValue2 = 0;	  
            INT ntlog_nRegOffset	  = 8;    
            CHAR ntlog_szRegValue[MAX_USERDATA_SIZE + 1] = {'\0'};  //memset(ntlog_szRegValue, 0x00, sizeof(ntlog_szRegValue));
            //			LPSTR ntlog_pszRegValue = NULL;
            CNTLog::PARAM_TYPE ntlog_eParamType = PT_INVALID;
            if(CNTLog::CT_STDTHISCALL == ntlog_ct || CNTLog::CT_CDECLTHISCALL == ntlog_ct)
                ntlog_nRegOffset += 4;
            if(ntlog_bRetClass)
                ntlog_nRegOffset += 4;
            while(ntlog_nIndex < ntlog_nSize)
            {
                ntlog_vecIndex2TypeName[ntlog_nIndex].nValidCommaPos += (INT)ntlog_nLen;
                ntlog_nPos = ntlog_nIndex ? (ntlog_nPos + ntlog_vecIndex2TypeName[ntlog_nIndex].nValidCommaPos) : ntlog_vecIndex2TypeName[ntlog_nIndex].nValidCommaPos;
                ntlog_nPos = (INT)ntlog_strDestFuncSig.find_first_of(",", ntlog_nPos);
                if((INT)std::string::npos  == ntlog_nPos)
                {
                    ntlog_nPos = (INT)ntlog_strDestFuncSig.rfind(" throw(",std::string::npos);
                    if((INT)std::string::npos == ntlog_nPos)
                        ntlog_nPos = (INT)ntlog_strDestFuncSig.find_last_of(")",std::string::npos);
                    else
                        ntlog_nPos -= 1;
                }
                ntlog_eParamType = ntlog_vecIndex2TypeName[ntlog_nIndex].pt;
                INT nRet = 0;
                DWORD dwpParam = 0;
                LPINT ntlog_pnRegValue =NULL;
                LPINT ntlog_pnRegValue2 = NULL;
                dwpParam = dwEBP + ntlog_nRegOffset; 
                ntlog_dwRegValue = GetSafeValidValue(PT_INVALID, 0, dwpParam, 0,0,0,0,0,&nRet);
                if(nRet < 0)
                    ntlog_dwRegValue = 0xcccccccd;
                switch (ntlog_eParamType)
                {
                case CNTLog::PT_INT64:
                    ntlog_nRegValue2 = GetSafeValidValue(PT_INVALID, 0, 0, dwpParam+4, 0,0,0,0,&nRet);
                    if(nRet < 0)
                        ntlog_dwRegValue = 0xcccccccd;
                    ntlog_nRegOffset += 8;
                    break;
                case CNTLog::PT_VARIANT:
                    memcpy(&ntlog_pnRegValue, &dwpParam, sizeof(dwpParam));
                    ntlog_nRegOffset +=sizeof(tagVARIANT);
                    break;
                case CNTLog::PT_GUID:
                    memcpy(&ntlog_pnRegValue, &dwpParam, sizeof(dwpParam));
                    ntlog_nRegOffset +=sizeof(_GUID);
                    break;
                default:
                    ntlog_nRegOffset += 4;
                    break;
                }
                LPSTR ntlog_pRegValueA = NULL;
                LPWSTR ntlog_pRegValueW = NULL;
                switch(ntlog_eParamType)
                {
                case CNTLog::PT_VOID:
                    break;
                case CNTLog::PT_bool:
                    if((DWORD)0 == ntlog_dwRegValue)
                        ntlog_nLen = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = 0x%08x(false)", ntlog_dwRegValue);
                    else
                        ntlog_nLen = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE, " = 0x%08x(true)", ntlog_dwRegValue);
                    break;				
                case CNTLog::PT_UINT1:
                    ntlog_nLen = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE, " = 0x%08x(%u)", ntlog_dwRegValue,(unsigned int)(unsigned char)ntlog_dwRegValue);
                    break;
                case CNTLog::PT_INT2:
                    ntlog_nLen = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE, " = 0x%08x(%hd)", ntlog_dwRegValue,(short)ntlog_dwRegValue);
                    break;
                case CNTLog::PT_UINT2:
                    ntlog_nLen = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE, " = 0x%08x(%hu)", ntlog_dwRegValue,(unsigned short)ntlog_dwRegValue);
                    break;
                case CNTLog::PT_INT:
                    ntlog_nLen = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE, " = 0x%08x(%d)", ntlog_dwRegValue,(int)ntlog_dwRegValue);
                    break;
                case CNTLog::PT_UINT:
                    ntlog_nLen = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE, " = 0x%08x(%u)", ntlog_dwRegValue,ntlog_dwRegValue);
                    break;
                case CNTLog::PT_CHAR:
                    ntlog_nLen = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE, " = 0x%08x('%c')",ntlog_dwRegValue,(char) ntlog_dwRegValue);
                    ntlog_strDestFuncSig.insert(ntlog_nPos, ntlog_szRegValue);
                    break;
                case CNTLog::PT_WCHAR:
                    ntlog_nLen = _snprintf(ntlog_szRegValue,  MAX_USERDATA_SIZE," = 0x%08x(L'%C')",ntlog_dwRegValue,(wchar_t) ntlog_dwRegValue);
                    break;
                case CNTLog::PT_INT64:
                    {
                        UINT64 ullRegValue=(UINT64)ntlog_nRegValue2<<32;
                        ullRegValue+=ntlog_dwRegValue;
                        ntlog_nLen = _snprintf(ntlog_szRegValue,  MAX_USERDATA_SIZE," = 0x%I64x(%I64d)", (UINT64)ullRegValue,(INT64)ullRegValue);
                    }
                    break;
                case CNTLog::PT_UINT64:
                    {
                        UINT64 ullRegValue=(UINT64)ntlog_nRegValue2<<32;
                        ullRegValue+=ntlog_dwRegValue;
                        ntlog_nLen = _snprintf(ntlog_szRegValue,  MAX_USERDATA_SIZE," = 0x%I64x(%I64u)", (UINT64)ullRegValue,(UINT64)ullRegValue);
                    }
                    break;
                case CNTLog::PT_PEND:
                    {
                        DWORD ntlog_dwRegValue_b1 = 0;
                        ntlog_dwRegValue_b1 = ntlog_dwRegValue & 0x000000FF;
                        DWORD ntlog_dwRegValue_b2 = 0;
                        ntlog_dwRegValue_b2 = (ntlog_dwRegValue & 0x0000FF00) >> 8;
                        DWORD ntlog_dwRegValue_b3;
                        ntlog_dwRegValue_b3 = (ntlog_dwRegValue & 0x00FF0000) >> 16;
                        DWORD ntlog_dwRegValue_b4;
                        ntlog_dwRegValue_b4 = (ntlog_dwRegValue & 0xFF000000) >> 24;
                        ntlog_nLen = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE, " = ??? [%02x %02x %02x %02x]", 
                            ntlog_dwRegValue_b1, ntlog_dwRegValue_b2,  ntlog_dwRegValue_b3, ntlog_dwRegValue_b4 );
                    }
                    break;
                    //.........follow, need safe value....................
                case CNTLog::PT_UINT1REF:
                case CNTLog::PT_INT2REF:
                case CNTLog::PT_UINT2REF:
                case CNTLog::PT_INTREF:
                case CNTLog::PT_UINTREF:
                case CNTLog::PT_boolREF:
                case CNTLog::PT_CHARREF: 
                case CNTLog::PT_WCHARREF: 
                case CNTLog::PT_INT64REF:
                case CNTLog::PT_UINT64REF:
                case CNTLog::PT_CHARPOINT:
                case CNTLog::PT_WCHARPOINT:
                case CNTLog::PT_CHARPOINTREF:
                case CNTLog::PT_WCHARPOINTREF:
                case CNTLog::PT_VARIANT:
                case CNTLog::PT_VARIANTREF:
                case CNTLog::PT_GUID:
                case CNTLog::PT_GUIDREF:
                    ntlog_nLen = GetSafeValidValue(ntlog_eParamType, ntlog_szRegValue,  ntlog_dwRegValue, ntlog_nRegValue2, ntlog_pnRegValue, ntlog_pnRegValue2, ntlog_pRegValueA, ntlog_pRegValueW);
                    break;
                default:
                    ntlog_nLen = _snprintf(ntlog_szRegValue,MAX_USERDATA_SIZE, " = 0x%08x", ntlog_dwRegValue);
                    break;
                }
                ntlog_nIndex++;
                ntlog_strDestFuncSig.insert(ntlog_nPos, ntlog_szRegValue);
            }
            if(MAX_USERDATA_SIZE <= ntlog_nPos)
                ntlog_strDestFuncSig = pszFuncSig;
        }
    }
#if (defined(UNICODE) || defined(_UNICODE))
    static std::basic_string<WCHAR>ntlog_strDestFuncSigW;
    CNTLog::MultiByteToUnicode(ntlog_strDestFuncSig,ntlog_strDestFuncSigW );
    ntlog_ptszDestFuncSig = ntlog_strDestFuncSigW.c_str();
#else
    ntlog_ptszDestFuncSig = ntlog_strDestFuncSig.c_str();
#endif
    return ntlog_ptszDestFuncSig;
}

BOOL CNTLog::UnicodeToMultiByte(const std::basic_string<WCHAR> &strSrc, std::string &strDst)
{
    INT nSrcUnicodeLen = 0;
    nSrcUnicodeLen = (INT)strSrc.length();
    if(0 == nSrcUnicodeLen)
        return TRUE;
    static INT nLen = 0; 
    nLen = WideCharToMultiByte(936, 0, strSrc.c_str(), -1,NULL, 0 ,NULL, NULL);
    CHAR szBuf[MAX_USERDATA_SIZE + 1] = {0};
    LPSTR pszBuf = szBuf;
    if(MAX_USERDATA_SIZE < nLen)
        nLen = MAX_USERDATA_SIZE;
    INT nResult = 0;
    nResult = WideCharToMultiByte(936, 0, strSrc.c_str(), -1, pszBuf, nLen,NULL,NULL);
    if( nResult <= 0)
        return FALSE;
    if(pszBuf)
        strDst = pszBuf;
    return TRUE;
}
BOOL CNTLog::MultiByteToUnicode(const std::string& strSrc, std::basic_string<WCHAR> &strDst)
{
    INT nSrcAnsiLen = 0;
    nSrcAnsiLen = (INT)strSrc.length();
    if(0 == nSrcAnsiLen)
        return TRUE;
    UINT code_page = GetStringCodePage(strSrc.c_str());
    INT nLen = 0;
    nLen = MultiByteToWideChar(code_page,0,strSrc.c_str(),-1,NULL,0);
    if(0 == nLen)
    {
        code_page = 0;
        nLen = MultiByteToWideChar(code_page,0,strSrc.c_str(),-1,NULL,0);
    }
    if(MAX_USERDATA_SIZE < nLen)
        nLen = MAX_USERDATA_SIZE;
    WCHAR szBuf[MAX_USERDATA_SIZE + 1] = {0};
    LPWSTR pszBuf = szBuf;
    INT nResult = 0;
    nResult = MultiByteToWideChar(code_page,0,strSrc.c_str(), -1, pszBuf,nLen);
    if(nResult <= 0 )
        return FALSE;	
    if(pszBuf)
        strDst = pszBuf;
    return TRUE;
}

LPCTSTR CNTLog::GetCurrentModuleVersion()
{
#define CB_FILE_VERSION 0x168
    HINSTANCE s_hInst = reinterpret_cast<HINSTANCE>(GetCurrentModuleHandle());
    TCHAR szModuleFileName[_MAX_PATH] = {0};
    LPTSTR pszInfoBuf = NULL;
    VS_FIXEDFILEINFO* pvi = NULL;
    UINT  cbTranslate = 0;
    static TCHAR szVerInfo[CB_FILE_VERSION] = {0};
    ::GetModuleFileName( s_hInst,szModuleFileName , MAX_PATH);
    DWORD dwInfoSize = 0, dwHandle = 0;
    dwInfoSize = GetFileVersionInfoSize(szModuleFileName,&dwHandle);
    if((DWORD)0 == dwInfoSize)
        return NULL;
    pszInfoBuf = new TCHAR[dwInfoSize];
    if(NULL == pszInfoBuf)
        return NULL;
    if(GetFileVersionInfo(szModuleFileName,0,dwInfoSize,pszInfoBuf))
    {
        if(VerQueryValue(pszInfoBuf, _T("\\"),(LPVOID*)&pvi,&cbTranslate))
        {
            _sntprintf(szVerInfo,CB_FILE_VERSION, _T("FileVersion : %d.%d.%d.%d [ FileVersionMS : 0x%08x, FileVersionLS : 0x%08x, ProductVersionMS : 0x%08x, ProductVersionLS : 0x%08x, ") 
                _T("Signature : 0x%08x, FileFlags : 0x%08x, FileFlagsMask : 0x%08x, FileOS : 0x%08x, ")
                _T("FileType : 0x%08x, FileSubType : 0x%08x, FileDateMS : 0x%08x, FileDateLS : 0x%08x ]") ,
                HIWORD(pvi->dwFileVersionMS), LOWORD(pvi->dwFileVersionMS), HIWORD(pvi->dwFileVersionLS), LOWORD(pvi->dwFileVersionLS),
                pvi->dwFileVersionMS, pvi->dwFileVersionLS, pvi->dwProductVersionMS, pvi->dwProductVersionLS ,
                pvi->dwSignature, pvi->dwFileFlags, pvi->dwFileFlagsMask, pvi->dwFileOS,
                pvi->dwFileType, pvi->dwFileSubtype, pvi->dwFileDateMS, pvi->dwFileDateLS);
        }
    }
    delete pszInfoBuf;
    pszInfoBuf = NULL;
    return szVerInfo[0] ? szVerInfo : NULL;
}

HINSTANCE CNTLog::GetCurrentModuleHandle(void)
{
    static HINSTANCE hCurrentModule = 0;
    if(NULL == hCurrentModule)
    {
        MEMORY_BASIC_INFORMATION m = { 0 };
        VirtualQuery(&hCurrentModule, &m, sizeof(MEMORY_BASIC_INFORMATION));
        hCurrentModule = (HINSTANCE) m.AllocationBase;
    }
    return hCurrentModule;
}

bool CNTLog::FileCmp(const PAIRPID2FILEINFO &val1, const PAIRPID2FILEINFO &val2)
{
    if(val1.second.stFileTime_.dwHighDateTime != val2.second.stFileTime_.dwHighDateTime)
        return val1.second.stFileTime_.dwHighDateTime < val2.second.stFileTime_.dwHighDateTime;
    else
        return val1.second.stFileTime_.dwLowDateTime < val2.second.stFileTime_.dwLowDateTime;
}

void CNTLog::PrintModuleInit(void)
{
    static bool bFirstInit = true;
    TCHAR szData[MAX_USERDATA_SIZE + 1] = {0};
    if(bFirstInit)
    {
        bFirstInit = false;
        _sntprintf(szData, MAX_USERDATA_SIZE, _T("Module Loaded. Entry Address(0x%08x), Path = %s, Current LogGroup : \"%s\", ModuleID : %d, RefCount : %d, NTLOG_VERSION_IN_FILEMAP : 0x%08x, NTLOG_VERSION_IN_STAITC : 0x%08x"),
            (ULONG)(ULONG_PTR)GetCurrentModuleHandle(), s_szModuleFileName, __TNTLOG_GROUP, s_nModuleID, _lRef, _lVer, s_lVer);
    }
    else
    {
        _sntprintf(szData, MAX_USERDATA_SIZE, _T("Entry Address(0x%08x), Path = %s, Current LogGroup : \"%s\", ModuleID : %d, RefCount : %d, NTLOG_VERSION_IN_FILEMAP : 0x%08x, NTLOG_VERSION_IN_STAITC : 0x%08x"),
            (ULONG)(ULONG_PTR)GetCurrentModuleHandle(), s_szModuleFileName, __TNTLOG_GROUP, s_nModuleID, _lRef, _lVer, s_lVer);
    }	
    NTPrompt(szData);
    LPCTSTR pszVertionType = NULL;
    LPCTSTR pszLogFilePath = NULL;
#if (defined(UNICODE) || defined(_UNICODE))
    pszVertionType =  L"'UNICODE'";
    std::basic_string<WCHAR>strLogFilePathW;
    MultiByteToUnicode(m_szLogFileDirA,strLogFilePathW );
    pszLogFilePath = strLogFilePathW.c_str();	
#else
    pszVertionType = "'MBCS'",
        pszLogFilePath = m_szLogFileDirA;
#endif
    NTPrompt(_T("BuildType : %s, FileVersionInfo : { %s } Rebuild Time : %s, LogFileDir : %s, AllLogFileCount : %lu"), 
        pszVertionType, GetCurrentModuleVersion(), __TTIMESTAMP__, (INVALID_HANDLE_VALUE == m_hLogFile) ? NULL : pszLogFilePath, m_dwAllLogFileCnt);	
}

LPCTSTR CNTLog::GetCurrentProcessName(void)
{
    static TCHAR szfname[_MAX_FNAME] = {0};
    if('\0' != szfname[0])
        return szfname;
    TCHAR szCurProcessName[_MAX_FNAME] = {0};
    TCHAR szdrive[_MAX_DRIVE] = {0};
    TCHAR szdir[_MAX_DIR] = {0};
    TCHAR szext[_MAX_EXT] = {0};
    GetModuleFileName(NULL,szCurProcessName , _MAX_PATH);
    _tsplitpath(szCurProcessName, szdrive, szdir, szfname, szext );
    return szfname;
}

LPCTSTR CNTLog::GetCurrentModuleName(void)
{
    static TCHAR szCurModuleName[_MAX_FNAME] = {0};
    if('\0' != szCurModuleName[0])
        return szCurModuleName;
    HINSTANCE s_hInst = reinterpret_cast<HINSTANCE>(GetCurrentModuleHandle());
    TCHAR szModuleFileName[_MAX_PATH] = {0};
    LPTSTR pszModuleFileName = szModuleFileName;
    ::GetModuleFileName( s_hInst,szModuleFileName , MAX_PATH);
    pszModuleFileName += _tcslen(szModuleFileName);
    while ('\\' != *(--pszModuleFileName)) ;
    ++pszModuleFileName;
    _tcsncpy(szCurModuleName, pszModuleFileName,_MAX_FNAME); //
    pszModuleFileName = szCurModuleName;			//去除 后缀名
    while ( '\0' != *pszModuleFileName)
    {
        ++pszModuleFileName;
        if( '.'== *pszModuleFileName)
        {
            *pszModuleFileName = '\0';
            break;
        }
    }
    return szCurModuleName;
}

BOOL CNTLog::CheckConfigFileChanged(void)
{
#define FILE_NOTIFY_NTLOG \
    (FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | \
    FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE | \
    FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SECURITY)
    
    INT nStatus = 0;
    if(s_release.m_bIsModuleUnloading)
        return FALSE;
    if(INVALID_HANDLE_VALUE == _hConfigFileChanged)
    {
        _hConfigFileChanged = FindFirstChangeNotification(__TNTLOG_CONFIG_DIR,  FALSE, FILE_NOTIFY_NTLOG);
        if(INVALID_HANDLE_VALUE != _hConfigFileChanged)     // 存在这个目录
        {
            if(INVALID_FILE_ATTRIBUTES == GetFileAttributes(__TNTLOG_CONFIG_FILE_PATH) 
                && INVALID_FILE_ATTRIBUTES == GetFileAttributes(__TNTLOG_CONFIG_FILE_PATH2)
                && INVALID_FILE_ATTRIBUTES == GetFileAttributes(__TNTLOG_CONFIG_FILE_PATH_DEFAULT)
                && INVALID_FILE_ATTRIBUTES == GetFileAttributes(__TNTLOG_CONFIG_FILE_PATH_DEFAULT2))
                return FALSE;   // 不存在
            GetConfig();
        }
        return TRUE;    // 下一条才打印
    }
    nStatus = WaitForSingleObject(_hConfigFileChanged, 0);
    switch(nStatus)
    {
    case WAIT_OBJECT_0:
        FindCloseChangeNotification(_hConfigFileChanged);
        _hConfigFileChanged = INVALID_HANDLE_VALUE;
        if(GetConfig())
            _hConfigFileChanged = FindFirstChangeNotification(__TNTLOG_CONFIG_DIR,  FALSE, FILE_NOTIFY_NTLOG);
        return TRUE;
        break;
    case WAIT_TIMEOUT:
        return FALSE;
        break;
    default:
        _hConfigFileChanged = FindFirstChangeNotification(__TNTLOG_CONFIG_DIR,  FALSE, FILE_NOTIFY_NTLOG);
        return FALSE;
        break;
    }
}

BOOL CNTLog::MakeDirectoryA(LPCSTR lpszDestDirA)
{
    LPSTR lpszHead = (LPSTR)lpszDestDirA;
    LPSTR lpszDir = (LPSTR)lpszDestDirA;
    CHAR szTempDir[_MAX_PATH] = {0};
    DWORD dwAttributes = 0;
    if (NULL == lpszDir)
        return FALSE;
    while ( '\0' != *(++lpszDir))
    {
        if ( '\\' == *lpszDir)
        {
            memcpy(szTempDir, lpszHead, (lpszDir - lpszHead) * sizeof(CHAR));
            szTempDir[lpszDir - lpszHead] = '\0';
            dwAttributes = GetFileAttributesA(szTempDir);
            if((DWORD)0xFFFFFFFF == dwAttributes || !(FILE_ATTRIBUTE_DIRECTORY & dwAttributes))
                if(!CreateDirectoryA(szTempDir, NULL))
                    return FALSE;
        }
    }
    return TRUE;
}

BOOL CNTLog::MakeDirectory(LPCTSTR lpszDestPath) //no other version of MakeDirectoryA
{
    LPTSTR lpszHead = (LPTSTR)lpszDestPath;
    LPTSTR lpszDir = (LPTSTR)lpszDestPath;
    TCHAR szTempDir[_MAX_PATH] = {0};
    DWORD dwAttributes = 0;
    if (NULL == lpszDir)
        return FALSE;
    while ( '\0' != *(++lpszDir))
    {
        if ( '\\' == *lpszDir)
        {
            memcpy(szTempDir, lpszHead, (lpszDir - lpszHead) * sizeof(TCHAR));
            szTempDir[lpszDir - lpszHead] = '\0';
            dwAttributes = GetFileAttributes(szTempDir);
            if( INVALID_FILE_ATTRIBUTES == dwAttributes || !(FILE_ATTRIBUTE_DIRECTORY & dwAttributes))
                if(!CreateDirectory(szTempDir, NULL))
                    return FALSE;
        }
    }
    return TRUE;
}
void CNTLog::ResetParams(BOOL bModuleInit)
{
#ifdef _DEBUG
    m_bDebugViewLog				= TRUE;
    m_bFileLog					= TRUE;
    m_eLevel					= LEVEL_TRACE;
#else
    m_bDebugViewLog				= FALSE;
    m_bFileLog					= FALSE;
    m_dwCurFileSize				= 0;
    if(INVALID_HANDLE_VALUE != m_hLogFile)
    {
        CloseHandle(m_hLogFile);
        m_hLogFile = INVALID_HANDLE_VALUE;
    }
#endif
    m_ghvInclude.Clear();
    m_ghvExclude.Clear();
    m_ghvInclude.Add(_T("*"));
    m_dwMaxLogFileSize			= DEFAULT_MAX_LOGFILE_SIZE;
    m_nMaxLogFileCnt			= DEFAULT_MAX_LOGFILE_COUNT;
    m_nMaxHistroyProcessCnt		= DEFAULT_MAX_HISTROY_PROCESS_COUNT;
    if(bModuleInit)
    {
        m_hLogFile					= INVALID_HANDLE_VALUE;
        _hConfigFileChanged			= INVALID_HANDLE_VALUE;
        m_nLogFileCnt				= 0;
        m_nFileLogLineCnt			= 0;		
        strncpy(m_szLogFileDirA, DEFAULT_LOGFILE_PATH, strlen(DEFAULT_LOGFILE_PATH) + 1);
    }
}
BOOL CNTLog::GetConfig(BOOL bModuleInit) //如果没有配置文件，将不打印日志，历史日志也没有必要删除
{	
    struct _stat stBuf;	
    if(0 != _tstat(__TNTLOG_CONFIG_FILE_PATH, &stBuf)) 
    {
        //NTLOG_GROUP.ini不存在
        if(0 != _tstat(__TNTLOG_CONFIG_FILE_PATH2, &stBuf))
        {
            //NTLOG_GROUP.txt也不存在			
            if(0 != _tstat(__TNTLOG_CONFIG_FILE_PATH_DEFAULT, &stBuf))
            {
                //ntlog.ini不存在
                if(0 != _tstat(__TNTLOG_CONFIG_FILE_PATH_DEFAULT2, &stBuf))
                {
                    //ntlog.txt不存在
                    ResetParams(FALSE);
                    return FALSE;
                }
                else 
                {
                    MoveFile(__TNTLOG_CONFIG_FILE_PATH_DEFAULT2, __TNTLOG_CONFIG_FILE_PATH_DEFAULT);
                }
            }
            CopyFile(__TNTLOG_CONFIG_FILE_PATH_DEFAULT, __TNTLOG_CONFIG_FILE_PATH, FALSE);
        }
        else
        {
            //存在ntlog.txt不存在ntlog.ini, 把 ntlog.txt改名为ntlog.ini
            MoveFile(__TNTLOG_CONFIG_FILE_PATH2, __TNTLOG_CONFIG_FILE_PATH);
        }
    }
    LPCTSTR pszConfigFileName = __TNTLOG_CONFIG_FILE_PATH;	
    BOOL bWriteConfig = FALSE;
    BOOL bLastFileLog = m_bFileLog;
    if((_off_t)0 == stBuf.st_size)
    {
        HANDLE hFile = CreateFile(pszConfigFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0 , NULL);
        if(INVALID_HANDLE_VALUE != hFile)			
        {
            DWORD dwByteWritten = 0;
            LPCSTR szBufferA = ";[Output]\r\n"
                ";DebugView                  =ON     ;ON 打开 OFF 关闭                                                           默认关		\r\n"
                ";FileLog                    =ON     ;ON 打开 OFF 关闭                                                           默认关		\r\n"
                "\r\n"
                ";[Common]\r\n"
                ";Level                      = TRACE   ; 共有TRACE,DEBUG,INFO,WARN,ERROR,FATAL                                   DEBUG		\r\n"
                ";Include                    = *       ; *表示任意 打印包括*字符的日志                                           *			\r\n"
                ";Exclude                    = a;b     ;           打印不包括\"a\" 和 \"b\" 字符的日志用\";\"分隔							\r\n"
                "\r\n"
                ";[FileLogOption]\r\n"
                ";LogFilePath这个参数若需运行时生效必须先设置 FileLog=OFF，保存当前文件，再设置 FileLog=ON， 保存当前文件，其它参数在保存后立即生效\r\n"
                ";LogFilePath                = C:\\NTLOG\\   ;日志存放目录                                                          C:\\ntlog\\\r\n"
                ";MaxLogFileSize             = 20480       ;日志大小 20480KB                                                      20480		\r\n"
                ";MaxLogFileCnt              = 20          ;当前进程日志文件最多数目                                              20			\r\n"
                ";MaxCountOfHistroyProcess   = 10          ;只保留最新 10 个活动进程的所有日志                                    10			\r\n"
                "\r\n";
            WriteFile(hFile, szBufferA, (DWORD)strlen(szBufferA), &dwByteWritten, NULL);
            CloseHandle(hFile);
            bWriteConfig = TRUE;
        }
    }	
    TCHAR szReturnedString[2048] = {0};
    GetPrivateProfileString(_T("Output"), _T("DebugView"), _T("ON"), szReturnedString, sizeof(szReturnedString)/sizeof(TCHAR), pszConfigFileName);	//是否在DebugView中打印日志
    if(0 == _tcsicmp(szReturnedString, _T("ON")) || _tcsicmp(szReturnedString, _T("1")) == 0)
        m_bDebugViewLog = TRUE;
    else if(0 == _tcsicmp(szReturnedString, _T("OFF")) || 0 == _tcsicmp(szReturnedString, _T("0")))
        m_bDebugViewLog = FALSE;
    if(bWriteConfig)
        WritePrivateProfileString(_T("Output"), _T("DebugView"), m_bDebugViewLog ? _T("ON") : _T("OFF"), pszConfigFileName);
    GetPrivateProfileString(_T("Output"), _T("FileLog"), _T("ON"), szReturnedString, sizeof(szReturnedString)/sizeof(TCHAR), pszConfigFileName);	//是否在日志文件中打印日志
    if(0  == _tcsicmp(szReturnedString, _T("ON")) || 0 == _tcsicmp(szReturnedString, _T("1")))
        m_bFileLog = TRUE;
    else if(0 == _tcsicmp(szReturnedString, _T("OFF")) || 0 == _tcsicmp(szReturnedString, _T("0")))
        m_bFileLog = FALSE;
    if(bWriteConfig)
        WritePrivateProfileString(_T("Output"), _T("FileLog"), m_bFileLog ? _T("ON") : _T("OFF"), pszConfigFileName);	
    GetPrivateProfileString(_T("Common"), _T("Level"), _T("TRACE"), szReturnedString, sizeof(szReturnedString)/sizeof(TCHAR), pszConfigFileName);
    if(0 == _tcsicmp(szReturnedString, _T("TRACE")))
        m_eLevel = LEVEL_TRACE;
    else if(0 == _tcsicmp(szReturnedString, _T("DEBUG")))
        m_eLevel = LEVEL_DEBUG;
    else if(0 == _tcsicmp(szReturnedString, _T("INFO")))
        m_eLevel = LEVEL_INFO;
    else if(0 == _tcsicmp(szReturnedString, _T("WARN")))
        m_eLevel = LEVEL_WARN;
    else if(0 == _tcsicmp(szReturnedString, _T("ERROR")))
        m_eLevel = LEVEL_ERROR;
    else if(0 == _tcsicmp(szReturnedString, _T("FATAL")))
        m_eLevel = LEVEL_FATAL;
    if(bWriteConfig)
        WritePrivateProfileString(_T("Common"), _T("Level"), 
        (LEVEL_TRACE == m_eLevel) ? _T("TRACE") : 
        (LEVEL_DEBUG == m_eLevel) ? _T("DEBUG") : 
        (LEVEL_INFO == m_eLevel) ? _T("INFO") : 
        (LEVEL_WARN == m_eLevel) ? _T("WARN") : 
        (LEVEL_ERROR == m_eLevel) ? _T("ERROR") : 
        (LEVEL_FATAL == m_eLevel) ? _T("FATAL") : 
        _T("TRACE"), pszConfigFileName);
    //过滤项
    LPTSTR pToken = NULL;
    std::basic_string<TCHAR> strInclude;
    std::basic_string<TCHAR> strExclude;
    m_ghvInclude.Clear();
    GetPrivateProfileString(_T("Common"), _T("Include"), _T("*"), szReturnedString, sizeof(szReturnedString)/sizeof(TCHAR), pszConfigFileName);
    strInclude = szReturnedString;
#define FILT_SEP _T(";,|")
    pToken = _tcstok(szReturnedString, FILT_SEP); //以分号分隔	
    while(pToken != NULL)
    {
        if(0 == _tcsicmp(pToken, _T("*"))) //如果遇到"*", 把所有已加入的清除掉, 只要"*" 
        {
            m_ghvInclude.Clear();
            m_ghvInclude.Add(pToken);
            if(bWriteConfig)
                WritePrivateProfileString(_T("Common"), _T("Include"), _T("*"), pszConfigFileName);
            break;
        }
        m_ghvInclude.Add(pToken);
        pToken = _tcstok(NULL, FILT_SEP);
    }
    //滤除项
    m_ghvExclude.Clear();
    GetPrivateProfileString(_T("Common"), _T("Exclude"), _T(""), szReturnedString, sizeof(szReturnedString)/sizeof(TCHAR), pszConfigFileName);
    strExclude = szReturnedString;
    pToken = _tcstok(szReturnedString, _T(";")); //以分号分隔	
    if(NULL == pToken && bWriteConfig)		
        WritePrivateProfileString(_T("Common"), _T("Exclude"), _T(""), pszConfigFileName);
    while(pToken != NULL)
    {
        if(0 == _tcsicmp(pToken, _T("*"))) //如果遇到"*", 把所有已加入的清除掉, 只要"*"
        {
            m_ghvExclude.Clear();
            m_ghvExclude.Add(pToken);
            break;
        }
        m_ghvExclude.Add(pToken);
        pToken = _tcstok(NULL, FILT_SEP);
    }
    LPCTSTR pszLogFilePath = NULL; 
    LPCSTR pszConfigFileNameA = NULL;
#if (defined(UNICODE) || defined(_UNICODE))
    std::string strConfigFileName;
    UnicodeToMultiByte(pszConfigFileName,strConfigFileName );
    pszConfigFileNameA = strConfigFileName.c_str();
    std::basic_string<WCHAR>strLogFilePathW;
    MultiByteToUnicode(m_szLogFileDirA, strLogFilePathW);
    pszLogFilePath = strLogFilePathW.c_str();
#else
    pszConfigFileNameA = pszConfigFileName;
    pszLogFilePath = m_szLogFileDirA;
#endif
    if(m_bFileLog)
    {
        if(!bLastFileLog || 0 == m_dwAllLogFileCnt)//上次日志为关或第一次
        {
            GetPrivateProfileStringA(("FileLogOption"), ("LogFilePath"), DEFAULT_LOGFILE_PATH, m_szLogFileDirA, sizeof(m_szLogFileDirA), pszConfigFileNameA);		//日志文件路径
            if(0 == strlen(m_szLogFileDirA) || strlen(m_szLogFileDirA) >= sizeof(m_szLogFileDirA))
                strncpy(m_szLogFileDirA,  DEFAULT_LOGFILE_PATH, sizeof(m_szLogFileDirA));
            if(m_szLogFileDirA[strlen(m_szLogFileDirA)-1] != '\\')//如果没有"\", 加上
                strncat(m_szLogFileDirA,  ("\\"), sizeof(m_szLogFileDirA));
            s_dwLogFileCnt = DWORD(-1);
            if(!bModuleInit)
                PrintModuleInit();
        }		
        m_dwMaxLogFileSize	= GetPrivateProfileInt(_T("FileLogOption"), _T("MaxLogFileSize"), DEFAULT_MAX_LOGFILE_SIZE, pszConfigFileName);		//日志文件的最大尺寸
        if(m_dwMaxLogFileSize < MIN_MAX_LOGFILE_SIZE || m_dwMaxLogFileSize > MAX_MAX_LOGFILE_SIZE) //最大设置不能超过200M
        {
            m_dwMaxLogFileSize = DEFAULT_MAX_LOGFILE_SIZE;
            WritePrivateProfileString(_T("FileLogOption"), _T("MaxLogFileSize"),_T("20480"), pszConfigFileName);
        }
        m_nMaxLogFileCnt = GetPrivateProfileInt(_T("FileLogOption"), _T("MaxLogFileCnt"), DEFAULT_MAX_LOGFILE_COUNT, pszConfigFileName);		//日志文件的数量限制
        if(m_nMaxLogFileCnt < MIN_MAX_LOGFILE_COUNT || m_nMaxLogFileCnt > MAX_MAX_LOGFILE_COUNT) //最大设置不能超过100个文件
        {
            m_nMaxLogFileCnt = DEFAULT_MAX_LOGFILE_COUNT;
            WritePrivateProfileString(_T("FileLogOption"), _T("MaxLogFileCnt"),_T("20"), pszConfigFileName);
        }
        m_nMaxHistroyProcessCnt = GetPrivateProfileInt(_T("FileLogOption"), _T("MaxCountOfHistroyProcess"), DEFAULT_MAX_HISTROY_PROCESS_COUNT, pszConfigFileName);		//历史进程数目限制
        if(m_nMaxHistroyProcessCnt < MIN_MAX_HISTROY_PROCESS_COUNT || m_nMaxHistroyProcessCnt > MAX_MAX_HISTROY_PROCESS_COUNT) //最大设置不能超过100个文件
        {
            m_nMaxHistroyProcessCnt = DEFAULT_MAX_HISTROY_PROCESS_COUNT;
            WritePrivateProfileString(_T("FileLogOption"), _T("MaxCountOfHistroyProcess"),_T("10"), pszConfigFileName);
        }
        if(bWriteConfig)
        {
            WritePrivateProfileStringA(("FileLogOption"), ("LogFilePath"), m_szLogFileDirA, pszConfigFileNameA);		//日志文件路径
            if((DWORD)DEFAULT_MAX_LOGFILE_SIZE == m_dwMaxLogFileSize)			
                WritePrivateProfileString(_T("FileLogOption"), _T("MaxLogFileSize"),_T("20480"), pszConfigFileName);
            if((INT32)DEFAULT_MAX_LOGFILE_COUNT == m_nMaxLogFileCnt)			
                WritePrivateProfileString(_T("FileLogOption"), _T("MaxLogFileCnt"),_T("20"), pszConfigFileName);
            if((INT32)DEFAULT_MAX_HISTROY_PROCESS_COUNT == m_nMaxHistroyProcessCnt)			
                WritePrivateProfileString(_T("FileLogOption"), _T("MaxCountOfHistroyProcess"),_T("10"), pszConfigFileName);
        }
    }
    else
    {
        if(INVALID_HANDLE_VALUE != m_hLogFile)
        {
            CloseHandle(m_hLogFile);
            m_hLogFile = INVALID_HANDLE_VALUE;
            m_dwCurFileSize	= 0;
        }
    }
    if(!bModuleInit)
    {
        NTPrompt(_T("NTLog Config File : %s : \r\n00000000\t00:00:00.000\t[0000] [Output]\r\n00000000\t00:00:00.000\t[0000] DebugView=%s\r\n")
            _T("00000000\t00:00:00.000\t[0000] FileLog=%s\r\n00000000\t00:00:00.000\t[0000] [Common]\r\n00000000\t00:00:00.000\t[0000] Level=%s\r\n") 
            _T("00000000\t00:00:00.000\t[0000] Include=%s\r\n00000000\t00:00:00.000\t[0000] Exclude=%s\r\n00000000\t00:00:00.000\t[0000] [FileLogOption]\r\n")
            _T("00000000\t00:00:00.000\t[0000] LogFilePath=%s\r\n00000000\t00:00:00.000\t[0000] MaxLogFileSize=%d\r\n")
            _T("00000000\t00:00:00.000\t[0000] MaxLogFileCnt=%d\r\n00000000\t00:00:00.000\t[0000] MaxCountOfHistroyProcess=%d\r\n"),
            pszConfigFileName, 
            m_bDebugViewLog ? _T("ON") : _T("OFF"),
            m_bFileLog ? _T("ON") : _T("OFF"),
            (LEVEL_TRACE == m_eLevel) ? _T("TRACE") : (LEVEL_DEBUG == m_eLevel) ? _T("DEBUG") : (LEVEL_INFO == m_eLevel) ? _T("INFO") : 
            (LEVEL_WARN == m_eLevel) ? _T("WARN") : 	(LEVEL_ERROR == m_eLevel) ? _T("ERROR") : (LEVEL_FATAL == m_eLevel) ? _T("FATAL") : _T("TRACE"),
            strInclude.c_str(),
            strExclude.c_str(),
            pszLogFilePath,		
            m_dwMaxLogFileSize,
            m_nMaxLogFileCnt,
            m_nMaxHistroyProcessCnt
            );
        DeleteHistoryLogFiles(); //一个模块只保留最近m_nMaxHistroyProcessCnt个日志文件, 其余的文件删除
    }
    return TRUE;
}

void CNTLog::OutputLog(TLevel level, LPCTSTR pszFormat, va_list ap, LPCTSTR pszLogFilePath, BOOL bCheckFileChanged)
{
    //SectionProtect sp(& _cs);
    LPTSTR _pszCallFunName = NULL;
    if(bCheckFileChanged)
    {
        _pszCallFunName = (LPTSTR)TlsGetValue(s_dwTlsIndex);
        CheckConfigFileChanged();
        TlsSetValue(s_dwTlsIndex, _pszCallFunName);
    }
    if(!m_bDebugViewLog && !m_bFileLog) //
        return;
    if(level < m_eLevel)
        return;
    //#		Time	[PID] [TID] 级别 <模块名> 信息\t\r\n
    //样例:
    //00000000\t00:00:00.000\t[012345] [012345] TRACE  <MyModule> User Msg\t\r\n
    TCHAR szWholeMsg[MAX_USERDATA_SIZE + 1] = {0};
    LPTSTR pWholeMsg = szWholeMsg;// pWholeMsg = szWholeMsg;
    LPTSTR pDebugViewMsg = NULL;
    LPTSTR pLevelMsg = NULL;
    LPTSTR pUserMsg = NULL;
    size_t  nDebugViewMsgoffset = 0;
    size_t  nLevelMsgoffset = 0;
    size_t  nUserMsgoffset = 0;
    //	size_t  nLen = 0;
    //pWholeMsg = szWholeMsg; // new TCHAR[MAX_USERDATA_SIZE];
    //ZeroMemory(pWholeMsg, (MAX_USERDATA_SIZE + MAX_PRIVATEDATA_SIZE)*sizeof(TCHAR));
    //1. 加入前面的项, 即#		Time	[PID] [TID] 
    SYSTEMTIME nowtime = {0};
    static DWORD nCurProcessId = 0;
    if((DWORD)0 == nCurProcessId)
        nCurProcessId = (DWORD)GetCurrentProcessId();
    GetLocalTime(&nowtime);
    nDebugViewMsgoffset = _sntprintf(pWholeMsg, MAX_PRIVATEDATA_SIZE, _T("%08d\t%02d:%02d:%02d.%03d\t[%d] "),
        m_nFileLogLineCnt++, nowtime.wHour, nowtime.wMinute, nowtime.wSecond, nowtime.wMilliseconds, nCurProcessId );
    pDebugViewMsg = pWholeMsg + nDebugViewMsgoffset; //向OutputDebugString传递的字符串的首地址
    //2.加入 [TID] 
    nLevelMsgoffset = _sntprintf(pDebugViewMsg, MAX_PRIVATEDATA_SIZE, _T("[%d] "), GetCurrentThreadId());
    pLevelMsg = pDebugViewMsg + nLevelMsgoffset; //级别字符串的首地址
    //nUserMsgoffset =  _tcslen(_T("TRACE  ")) + _tcslen(GetCurrentrentModuleName()) + _tcslen(_T("<> "));
    //3. 加入日志级别和模块名
    //nLen = nUserMsgoffset;
    switch(level)
    {
    case LEVEL_TRACE:	nUserMsgoffset = _sntprintf(pLevelMsg, MAX_PRIVATEDATA_SIZE, _T("TRACE  <%s> "), GetCurrentModuleName());	break;
    case LEVEL_DEBUG:	nUserMsgoffset = _sntprintf(pLevelMsg, MAX_PRIVATEDATA_SIZE, _T("DEBUG  <%s> "), GetCurrentModuleName());	break;
    case LEVEL_INFO:	nUserMsgoffset = _sntprintf(pLevelMsg, MAX_PRIVATEDATA_SIZE, _T("INFO   <%s> "), GetCurrentModuleName());	break;
    case LEVEL_WARN:	nUserMsgoffset = _sntprintf(pLevelMsg, MAX_PRIVATEDATA_SIZE, _T("WARN   <%s> "), GetCurrentModuleName());	break;
    case LEVEL_ERROR:	nUserMsgoffset = _sntprintf(pLevelMsg, MAX_PRIVATEDATA_SIZE, _T("ERROR  <%s> "), GetCurrentModuleName());	break;
    case LEVEL_FATAL:	nUserMsgoffset = _sntprintf(pLevelMsg, MAX_PRIVATEDATA_SIZE, _T("FATAL  <%s> "), GetCurrentModuleName());	break;
    case LEVEL_PROMPT:	nUserMsgoffset = _sntprintf(pLevelMsg, MAX_PRIVATEDATA_SIZE, _T("PROMPT <%s> "), GetCurrentModuleName());	break;
    default:
        return;
    }
    //nUserMsgoffset = _tcslen(pLevelMsg);
    pUserMsg = pLevelMsg + nUserMsgoffset; //用户信息的字符串的首地址
    //4.增加打印  00000000\t00:00:00.000\t[012345] [012345] TRACE  <MyModule> [CALLFUNNAME]
    size_t nFunNameLen = 0;
    LPTSTR s_pszCallFunName = NULL;
    LPDWORD s_pdwThis = 0;
    pUserMsg[0] = _T('[');
    __try
    {
        s_pszCallFunName = bCheckFileChanged ? (LPTSTR)TlsGetValue(s_dwTlsIndex) : 0;
        s_pdwThis = bCheckFileChanged ? (LPDWORD) TlsGetValue(s_dwTlsIndex_this) : 0;
        if (NULL == s_pszCallFunName)
            nFunNameLen = 0;
        else
        {
            nFunNameLen = _tcslen(s_pszCallFunName);
            memcpy(&pUserMsg[1], s_pszCallFunName, (nFunNameLen) * sizeof(TCHAR));
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        nFunNameLen = _tcslen(_T("!!!"));
        memcpy(&pUserMsg[1], _T("!!!"), (nFunNameLen) * sizeof(TCHAR));
    }
    pUserMsg[nFunNameLen + 1] = _T(']');
    pUserMsg[nFunNameLen + 2] = _T(' ');
    pUserMsg += nFunNameLen + 3;
    if(NULL != s_pdwThis)	//
    {
        INT s_nThisLen = 0;
        DWORD s_dwThis = 0;
        __try
        {
            memcpy(&s_dwThis,&s_pdwThis, sizeof(DWORD));
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            s_dwThis =(DWORD)-1;
        }
        pUserMsg[0] = _T('[');
        _sntprintf(&pUserMsg[1],MAX_PRIVATEDATA_SIZE,_T("0x%08x"), s_dwThis);
        s_nThisLen = (INT)_tcslen(pUserMsg);
        pUserMsg[s_nThisLen] = _T(']');
        pUserMsg[s_nThisLen + 1] = _T(' ');
        pUserMsg += s_nThisLen + 2;
    }
#define FINALFLAG _T("\t\r\n")
    //	size_t  nFinalFlagLen = _tcslen(_T("\t\r\n"));
    __try
    {
        if(NULL == ap)
            nUserMsgoffset = _sntprintf(pUserMsg, MAX_USERDATA_SIZE - (pUserMsg-pWholeMsg) - 64, _T("%s"), pszFormat);
        else
            nUserMsgoffset = _vsntprintf(pUserMsg, MAX_USERDATA_SIZE - (pUserMsg-pWholeMsg) - 64, pszFormat, ap);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        nUserMsgoffset = _sntprintf(pUserMsg, MAX_USERDATA_SIZE - (pUserMsg-pWholeMsg) - 64, _T("(!!!)Access Error, Invalid Memory Address : 0x%p, ExceptionCode : 0x%08x"), pszFormat, GetExceptionCode());
    }
    if((size_t)-1 == nUserMsgoffset)
        nUserMsgoffset = _tcslen(pUserMsg);
    LPTSTR pEnd = pUserMsg + nUserMsgoffset - 1;
    while( '\t' == *pEnd ||  '\r' == *pEnd || '\n' == *pEnd)//清除最后的制表符, 回车和换行
        *pEnd-- = 0;
    //_tcsncat(++pEnd, FINALFLAG , nFinalFlagLen); //在后面加\t\r\n
    LARGE_INTEGER li = {0};
    li.QuadPart = NTGetTickCount64();
    nUserMsgoffset = _sntprintf(++pEnd, 32, _T("\t'%hu '%hu%s"), HIWORD(li.LowPart), LOWORD(li.LowPart), FINALFLAG); 
    pEnd += nUserMsgoffset;
    //_tcsncat(++pEnd, FINALFLAG , nFinalFlagLen); //在后面加\t\r\n
    //5. 过滤, 可能不用打印
    if(LEVEL_PROMPT != level) //只有用户信息才需要过滤, 如果是提示信息, 则不用过滤, 提示信息必须打印
    {
        if(!m_ghvInclude.Find(_T("*")) && !m_ghvInclude.SubFind(pDebugViewMsg))
            return;
        if(m_ghvExclude.Find(_T("*")) || m_ghvExclude.SubFind(pDebugViewMsg))
            return;
    }
    if(m_bDebugViewLog)
        OutputDebugString(pDebugViewMsg);
    if(m_bFileLog)
        WriteToLogFile(level, pWholeMsg, DWORD(pEnd - pWholeMsg), pszLogFilePath);
}

void CNTLog::WriteToLogFile(TLevel level, LPCTSTR pszFileLogMsg, DWORD dwLen, LPCTSTR pszLogFilePath)
{
    if(NULL != pszLogFilePath)
        if(INVALID_HANDLE_VALUE == s_hLogFile)
            if(MakeDirectory(pszLogFilePath))
                s_hLogFile = CreateFile(pszLogFilePath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0 , NULL);
    //该进程第一次打文件日志, 还没有打开日志文件
    if(m_hLogFile == INVALID_HANDLE_VALUE)
    {
        //从日志中得到模块名
        DWORD dwAttri = GetFileAttributesA(m_szLogFileDirA);
        if(dwAttri != INVALID_FILE_ATTRIBUTES && dwAttri & FILE_ATTRIBUTE_DIRECTORY) //目录存在
            ;
        else //目录不存在, 创建目录
        {
            if(!MakeDirectoryA(m_szLogFileDirA))
            {
                if(!GetTempPathA(_MAX_PATH, m_szLogFileDirA))
                    return;
#if (defined(UNICODE) || defined(_UNICODE))
                _snprintf(m_szLogFileDirA, _MAX_PATH, "%s%S\\", m_szLogFileDirA, __TNTLOG_GROUP);
#else
                _snprintf(m_szLogFileDirA, _MAX_PATH, "%s%s\\", m_szLogFileDirA, __TNTLOG_GROUP);
#endif
                if(!MakeDirectoryA(m_szLogFileDirA))
                    return;
            }
        }
        //构造完整路程加文件名
        TCHAR szLogFilePathName[MAX_PATH] = {0};
        GetNthFilePathName(++m_nLogFileCnt, szLogFilePathName, NULL);
        m_dwAllLogFileCnt++; 
        //打开日志文件
        m_hLogFile =  CreateFile(szLogFilePathName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0 , NULL);
        if(INVALID_HANDLE_VALUE == m_hLogFile)
        {
            DWORD dwErrCode = GetLastError();
            if((DWORD)ERROR_DISK_FULL == dwErrCode)
            {
                m_bFileLog = FALSE;
                WritePrivateProfileString(_T("Output"), _T("FileLog"), _T("OFF"), __TNTLOG_CONFIG_FILE_PATH);
#ifdef _DEBUG				
                MessageBox(NULL, _T("Disk is full."), s_szModuleFileName, MB_OK);
#endif
                return;
            }
            else if((DWORD)ERROR_ACCESS_DENIED == dwErrCode)
            {
                //尝试在临时目录下建文件
                if(!GetTempPathA(_MAX_PATH, m_szLogFileDirA))
                    return;				
#if (defined(UNICODE) || defined(_UNICODE))
                _snprintf(m_szLogFileDirA, _MAX_PATH, "%sNTLOG\\%S\\", m_szLogFileDirA, __TNTLOG_GROUP);
#else
                _snprintf(m_szLogFileDirA, _MAX_PATH, "%sNTLOG\\%s\\", m_szLogFileDirA, __TNTLOG_GROUP);
#endif
                GetNthFilePathName(m_nLogFileCnt, szLogFilePathName, NULL);
                m_hLogFile =  CreateFile(szLogFilePathName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0 , NULL);
                if(INVALID_HANDLE_VALUE == m_hLogFile)
                    return;				
            }
            else
                return;

        }
    }
    DWORD dwByteWritten = 0;
#if (defined(UNICODE) || defined(_UNICODE))
    //可以先把Unicode字符串转成ANSI字符串
    CHAR szBufferA[MAX_USERDATA_SIZE + MAX_PRIVATEDATA_SIZE + 1] = {0};	//调用频繁
    INT nLen = 0;
    nLen = WideCharToMultiByte(936, 0, pszFileLogMsg, -1,NULL, 0 ,NULL, NULL);
    if(0 >= nLen)
        return;
    else if(MAX_USERDATA_SIZE + MAX_PRIVATEDATA_SIZE  < nLen)
        return;
    dwLen = WideCharToMultiByte(936, 0, pszFileLogMsg, -1, szBufferA, nLen,NULL,NULL);	
    if(0 >= dwLen)
        return;
    dwLen--;
    if(MAX_USERDATA_SIZE + 1 < dwLen)
        dwLen = MAX_USERDATA_SIZE + 1;
    if(INVALID_HANDLE_VALUE != s_hLogFile)
    {
        SetFilePointer(s_hLogFile,0, 0, FILE_END);
        WriteFile(s_hLogFile, szBufferA, dwLen, &dwByteWritten, NULL);
        CloseHandle(s_hLogFile);
        s_hLogFile = INVALID_HANDLE_VALUE;
    }
    //磁盘满停止打印
    if(!WriteFile(m_hLogFile, szBufferA, dwLen, &dwByteWritten, NULL))
    {
        if((DWORD)ERROR_DISK_FULL == GetLastError())
        {
            m_bFileLog = FALSE;
            WritePrivateProfileString(_T("Output"), _T("FileLog"), _T("OFF"), __TNTLOG_CONFIG_FILE_PATH);
        }
        return;
    }
#else
    if(INVALID_HANDLE_VALUE != s_hLogFile)
    {
        SetFilePointer(s_hLogFile,0, 0, FILE_END);
        WriteFile(s_hLogFile,pszFileLogMsg, dwLen, &dwByteWritten, NULL);
        CloseHandle(s_hLogFile);
        s_hLogFile = INVALID_HANDLE_VALUE;
    }
    //磁盘满停止打印
    if(!WriteFile(m_hLogFile, pszFileLogMsg, dwLen, &dwByteWritten, NULL))
    {
        if((DWORD)ERROR_DISK_FULL == GetLastError())
        {
            m_bFileLog = FALSE;
            WritePrivateProfileString(_T("Output"), _T("FileLog"), _T("OFF"), __TNTLOG_CONFIG_FILE_PATH);
        }
        return;
    }
#endif
    //模块首次打印日志 这个不能置前,要不然会冲掉上一次打印的。
    if(s_dwLogFileCnt < m_dwAllLogFileCnt)
    {
        s_dwLogFileCnt = m_dwAllLogFileCnt;
        PrintModuleInit();
    }
    else if(s_dwLogFileCnt > m_dwAllLogFileCnt)
        s_dwLogFileCnt = m_dwAllLogFileCnt;

    //如果文件大小超过指定大小, 则把之前所有的日志文件改名
    //static volatile DWORD dwSize = 0;	//GetFileSize(m_hLogFile, NULL);
    if(LEVEL_PROMPT != level)
        m_dwCurFileSize += dwByteWritten; 
    if(m_dwCurFileSize >= m_dwMaxLogFileSize * 1 * KBTYE) //m_dwMaxLogSize的单位是KB
    {
        m_dwCurFileSize = 0;
        FlushFileBuffers(m_hLogFile);
        CloseHandle(m_hLogFile);
        TCHAR szLogFilePathName[MAX_PATH] = {0};
        memset(szLogFilePathName, 0x00, sizeof(szLogFilePathName));
        if (m_nLogFileCnt >= m_nMaxLogFileCnt)	//20 - 20 = 1 >= 0
        {
            GetNthFilePathName(m_nLogFileCnt - m_nMaxLogFileCnt + 1, szLogFilePathName, NULL); //20 - 20 + 1 =1
            DeleteFile(szLogFilePathName); // del 1.log , next create 21.log
        }
        //构造完整路程加文件名
        //TCHAR szLogFilePathName[MAX_PATH] = {0};
        memset(szLogFilePathName, 0x00, sizeof(szLogFilePathName));
        GetNthFilePathName(m_nLogFileCnt,  NULL, szLogFilePathName);
        if(MAX_LOGFILE_INDEX <= ++m_nLogFileCnt)
            m_nLogFileCnt = 0;
        m_dwAllLogFileCnt++;
        //打开日志文件
        m_hLogFile =  CreateFile(szLogFilePathName, GENERIC_WRITE, FILE_SHARE_WRITE|FILE_SHARE_READ, NULL,CREATE_ALWAYS, 0,NULL);
        s_dwLogFileCnt = m_dwAllLogFileCnt;
        if(m_hLogFile)
            PrintModuleInit();
    }
}

BOOL CNTLog::GetNthFilePathName(INT nFileCnt, LPTSTR pOleFilePathName, LPTSTR pNewFilePathName)
{
    //pOleFilePathName和pNewFilePathName可以有一个为NULL, 表示不关心该项, 但不能同时为NULL
    if(NULL == pOleFilePathName && NULL == pNewFilePathName)
        return FALSE;
    DWORD dwPID = GetCurrentProcessId();
    LPCTSTR pszCurModuleName = GetCurrentModuleName();
    TCHAR szApplicationFileName[_MAX_PATH] = {0};
    TCHAR szdrive[_MAX_DRIVE] = {0};
    TCHAR szdir[_MAX_DIR] = {0};
    TCHAR szfname[_MAX_FNAME] = {0};
    TCHAR szext[_MAX_EXT] = {0};
    GetModuleFileName(NULL, szApplicationFileName, _MAX_PATH);
    _tsplitpath(szApplicationFileName, szdrive, szdir, szfname, szext);
    LPCTSTR pszLogFilePath = NULL;
#if defined(UNICODE) || defined(_UNICODE)
    std::basic_string<WCHAR> strLogFilePathW;
    MultiByteToUnicode(m_szLogFileDirA, strLogFilePathW);
    pszLogFilePath = strLogFilePathW.c_str();
#else
    pszLogFilePath = m_szLogFileDirA;
#endif
    if(pOleFilePathName != NULL)
    {
        if(0 == nFileCnt) //no reach
            _sntprintf(pOleFilePathName, _MAX_PATH, _T("%s\\%s[%d].log"), pszLogFilePath, pszCurModuleName, dwPID); //no reach
        else
            _sntprintf(pOleFilePathName, _MAX_PATH, _T("%s\\%s.%s[%d].%03d.log"), pszLogFilePath, __TNTLOG_GROUP,szfname , dwPID, nFileCnt);
    }
    if(pNewFilePathName != NULL)
        _sntprintf(pNewFilePathName, _MAX_PATH,  _T("%s\\%s.%s[%d].%03d.log"), pszLogFilePath, __TNTLOG_GROUP, szfname, dwPID,nFileCnt+1);
    return TRUE;
}
BOOL CNTLog::DeleteHistoryLogFiles()
{
    WIN32_FIND_DATA wfd;
    HANDLE hfh = INVALID_HANDLE_VALUE;			//find handle
    //找到该模块对应的所有日志文件, 压入vector	//每个list条目包含文件全路径和最后修改时间	//MAPPID2FILEINFO MathFile;
    std::vector<PAIRPID2FILEINFO> MathFile;
    std::vector<PAIRPID2FILEINFO>::iterator iter;
    INT nPid = 0;
    LPCTSTR pszLogFilePath = NULL;
#if (defined(UNICODE) || defined(_UNICODE))
    std::basic_string<WCHAR> strLogFilePathW;
    MultiByteToUnicode(m_szLogFileDirA, strLogFilePathW);
    pszLogFilePath = strLogFilePathW.c_str();
#else
    pszLogFilePath = m_szLogFileDirA;
#endif
    TCHAR szPathWildcard[MAX_PATH] = {0};
    if(-1 == m_nMaxHistroyProcessCnt)
    {
        _sntprintf(szPathWildcard, sizeof(szPathWildcard)/sizeof(TCHAR), _T("%s*.log"),
            pszLogFilePath);//可能有Module[4048].log和Module[4048].1.log
    }
    else
    {
        _sntprintf(szPathWildcard, sizeof(szPathWildcard)/sizeof(TCHAR), _T("%s%s.%s[*].*.log"),
            pszLogFilePath, __TNTLOG_GROUP,  GetCurrentProcessName());//可能有Module[4048].log和Module[4048].1.log
    }

    hfh = FindFirstFile(szPathWildcard, &wfd);
    FileInfo fileinfo;
    FileInfo* pfileinfo = NULL;
    std::basic_string<TCHAR> strPathName;
    if(hfh != INVALID_HANDLE_VALUE)
    {
        do
        {
            strPathName = pszLogFilePath;
            strPathName += wfd.cFileName;
            if(-1 == m_nMaxHistroyProcessCnt)
            {
                DeleteFile(strPathName.c_str());
                continue;
            }			
            nPid = FindPidFromLogFilePath(strPathName.c_str());
            if(0 > nPid)
                continue;
            iter = MathFile.begin();
            while(iter != MathFile.end())
            {
                if (nPid == iter->first)
                {
                    pfileinfo = &(iter->second);
                    if(pfileinfo->stFileTime_.dwHighDateTime > wfd.ftLastWriteTime.dwHighDateTime)
                    {
                        pfileinfo->strFilePath_ = strPathName;
                        pfileinfo->stFileTime_ = wfd.ftLastWriteTime;
                    }
                    else if(pfileinfo->stFileTime_.dwHighDateTime == wfd.ftLastWriteTime.dwHighDateTime &&
                        pfileinfo->stFileTime_.dwLowDateTime >  wfd.ftLastWriteTime.dwLowDateTime)		//与上同,方便阅读
                    {
                        pfileinfo->strFilePath_ = strPathName;
                        pfileinfo->stFileTime_ = wfd.ftLastWriteTime;
                    }
                    //直接返回
                    break;
                }
                iter++;
            }
            if(iter == MathFile.end())
            {
                fileinfo.strFilePath_ = strPathName;
                fileinfo.stFileTime_ = wfd.ftLastWriteTime;
                MathFile.push_back(std::make_pair(nPid, fileinfo));
            }
        }while(FindNextFile(hfh, &wfd));
    }
    //else 没有找到匹配的文件
    FindClose(hfh);
    INT32 nCurrentSize =(INT32)MathFile.size();
    INT32 nSubSize = (INVALID_HANDLE_VALUE == m_hLogFile) ? 0 : -1;
    NTPrompt(_T("DeleteLogFile, MaxCountOfHistroyProcess : %d, CurrentHistroyProcess : %d, CurrentLogFileHandle : 0x%08x, PathWildcard : %s"), 
        m_nMaxHistroyProcessCnt, nCurrentSize + nSubSize, m_hLogFile, szPathWildcard);
    if((INT32)nCurrentSize + nSubSize > m_nMaxHistroyProcessCnt) //删除对应进程的全有日志
    {
        sort(MathFile.begin(), MathFile.end(), FileCmp);		//先按最后修改时间排序
        for(INT32 i = 0; i < nCurrentSize + nSubSize - m_nMaxHistroyProcessCnt; i++)
        {
            NTPrompt(_T("DeleteLogFiles, AllFilesOfPid : %lu(0x%08x)"),  MathFile[i].first, MathFile[i].first);
            DeleteAllFilesOfPid( MathFile[i].first);					//删除对应进程的全有日志
        }
    }
    MathFile.clear();
    return TRUE;
}

void CNTLog::DeleteAllFilesOfPid(INT nPid)
{
    LPCTSTR pszLogFilePath = NULL;
#if (defined(UNICODE) || defined(_UNICODE))
    std::basic_string<WCHAR> strLogFilePathW;
    MultiByteToUnicode(m_szLogFileDirA, strLogFilePathW);
    pszLogFilePath = strLogFilePathW.c_str();
#else
    pszLogFilePath = m_szLogFileDirA;
#endif
    TCHAR szPathWildcard[MAX_PATH] = {0};
    _sntprintf(szPathWildcard, sizeof(szPathWildcard)/sizeof(TCHAR), _T("%s%s.%s[%d].*.log"),
        pszLogFilePath, __TNTLOG_GROUP,  GetCurrentProcessName(), nPid);//可能有Module[4048].log和Module[4048].1.log
    HANDLE hfh = INVALID_HANDLE_VALUE;			//find handle
    WIN32_FIND_DATA wfd;
    hfh = FindFirstFile(szPathWildcard, &wfd);
    std::basic_string<TCHAR> strPathName;
    if(hfh != INVALID_HANDLE_VALUE)
    {
        do
        {
            strPathName = pszLogFilePath;			
            strPathName += wfd.cFileName;
            if(!DeleteFile(strPathName.c_str()))
                NTPrompt(_T("DeleteLogFile Failed, ErrorCode : %d(0x%08x), LogFilePath : %s"), GetLastError(), GetLastError(), strPathName.c_str());
        }while(FindNextFile(hfh, &wfd));
    }
    FindClose(hfh);
}

INT CNTLog::FindPidFromLogFilePath(LPCTSTR pszFilePath)
{
    LPTSTR pszTemp = (LPTSTR)pszFilePath;
    TCHAR szPID[9] = {0};
    if (NULL == pszFilePath)
        return -1;
    size_t size = _tcslen(pszFilePath);
    INT nStartPos = 0;
    INT nEndPos = 0;
    while (0 < size)
    {
        size--;
        if(']' == pszTemp[size])
            nEndPos = (INT)size;
        else if('[' == pszTemp[size])
            nStartPos = (INT)size;
    }
    if((0 != nStartPos ) && (0 != nEndPos) && (nEndPos - nStartPos < 9))
    {
        memcpy(szPID, &pszTemp[nStartPos + 1], (nEndPos - nStartPos - 1) * sizeof(TCHAR));
        return _tstoi(szPID);
    }
    return -1;
}

void CNTLog::NTPrompt(LPCTSTR pszFormat, ...) //private
{
    TlsSetValue(s_dwTlsIndex, NULL);
    TlsSetValue(s_dwTlsIndex_this, NULL);
    va_list ap; 
    va_start(ap, pszFormat);
    OutputLog(LEVEL_PROMPT, pszFormat, ap,NULL, FALSE);
    va_end(ap);	
}

BOOL CNTLog::GetParamInfo(LPCSTR pszFuncSig, BOOL bIsStatic, CALLTYPE_NTLOG& ct, std::vector<stParamTypeInfo>& vecIndex2TypeName, BOOL& bRetClass)
{
	//保护
#define MAX_FUNCSIG_LEN (2*1024)
	if(NULL == pszFuncSig)
		return FALSE;
	CHAR szFuncSig[MAX_FUNCSIG_LEN] = {0}; //strtok 不能用 常量区 参数 因为里面会更改值
	LPSTR pszCallType = NULL;
	CHAR seps[] = "(,)";
	LPSTR ptoken = NULL; //
	BOOL bStartPend = FALSE;
	INT nIndex = 0;
	pszCallType = (LPSTR)strstr((const char *)pszFuncSig, "__"); 
	if(NULL == pszCallType)
		return FALSE;
	if(0 == memcmp((const void*)pszCallType, (const void*)"__stdcall", 9))
		ct = CT_STDCALL;
	else if(0 == memcmp((const void*)pszCallType, (const void*)"__cdecl", 7))
		ct = CT_CDECL_CALL;
	else if(0 == memcmp((const void*)pszCallType, (const void*)"__thiscall", 10))
		ct = CT_THISCALL;
	else
	{
		ct = CT_OTHERCALL;
		while(NULL != (pszCallType = (LPSTR)strstr((const char *)&pszCallType[2], "__")))
		{
			if(0 == memcmp((const void*)"__stdcall", (const void*)pszCallType, 9))
			{
				ct = CT_STDCALL;
				break;
			}
			else if(0 == memcmp((const void*)"__cdecl", (const void*)pszCallType, 7))
			{
				ct = CT_CDECL_CALL;
				break;
			}
			else if(0 == memcmp((const void*)"__thiscall", (const void*)pszCallType, 10))
			{
				ct = CT_THISCALL;
				break;
			}
		}
	}
	if(CT_OTHERCALL == ct)
		return FALSE;
	bRetClass = FALSE;
	if(0 == memcmp(pszFuncSig, "class ", 6))
		if(' ' == *(pszCallType - 1))
			bRetClass = TRUE;
	pszCallType += 9;
	ptoken = strstr(pszCallType, "(");
	if(CT_THISCALL != ct && !bIsStatic)
	{
		while (ptoken-- > pszCallType)
		{
			if(':' == *ptoken)
			{
				if(ct == CT_STDCALL)
					ct = CT_STDTHISCALL;
				else if(ct == CT_CDECL_CALL)
					ct = CT_CDECLTHISCALL;
				break;
			}
		}
	}
	INT nValidCommaPos = 0;
	nValidCommaPos = (INT)(pszCallType - pszFuncSig) - 1;//"("位置前
	strncpy(szFuncSig, pszFuncSig + nValidCommaPos,MAX_FUNCSIG_LEN);
	ptoken = strtok(szFuncSig , seps);
	if(ptoken)
		nValidCommaPos += (INT)strlen(ptoken);
	ptoken = strtok( NULL, seps ); //过第一次
	if(ptoken)
		nValidCommaPos += (int)strlen(ptoken) + 1;
	int cbLAB = 0;
	BOOL bpend = FALSE;
	while(NULL != ptoken && 0 != strcmp(" throw", ptoken))
	{
		LPSTR pfindLAB = NULL;
		for (pfindLAB = strstr(ptoken, "<"); NULL != pfindLAB; pfindLAB = strstr(pfindLAB + 1, "<"))
			cbLAB++;
		LPSTR pfindRAB = NULL;
		for (pfindRAB = strstr(ptoken, ">"); NULL != pfindRAB; pfindRAB = strstr(pfindRAB + 1, ">"))
		{
			cbLAB--;
			if(!bpend)
				bpend = TRUE;
		}
		if(0 == cbLAB)
		{
			INT nLen = 0;
			PARAM_TYPE eParamType = PT_PEND;
			eParamType = PT_PEND;
			if(!bpend && !bStartPend && !GetParamTypeAndLen(ptoken, eParamType,nLen))
			{
				vecIndex2TypeName.clear();
				return FALSE;
			}
			if(PT_VOID == eParamType)
				return TRUE;
			switch (eParamType)
			{
			case PT_PEND:
				if(!bStartPend)
					bStartPend = TRUE;
				break;
			default:
				break;
			}
			stParamTypeInfo ptinfo = {PT_PEND, 0};
			if(bStartPend)
				ptinfo.pt = PT_PEND;
			else
				ptinfo.pt = eParamType;
			ptinfo.nValidCommaPos = nValidCommaPos;
			vecIndex2TypeName.push_back(ptinfo);
			nIndex++;
			nValidCommaPos = 0;
		}
		ptoken = strtok(NULL, seps);
		if (ptoken)
			nValidCommaPos += (INT)strlen(ptoken) + 1;
	}
	return TRUE;
}

BOOL CNTLog::GetParamTypeAndLen( LPCSTR pszRaw, PARAM_TYPE& pt, INT& nLen)
{
	LPSTR pData = (LPSTR)pszRaw;
	if(NULL == pszRaw)
		return FALSE;
	nLen = 4;
	if(0 == strcmp(pszRaw, "void"))
		pt = PT_VOID;
	else if(0 == strcmp(pszRaw, "bool" ) || 0 == strcmp(pszRaw, "const bool") ||
		0 == strcmp(pszRaw, "volatile bool" ) || 0 == strcmp(pszRaw, "volatile const bool"))
		pt = PT_bool;
	else if(0 == strcmp(pszRaw, "bool &" ) || 0 == strcmp(pszRaw, "const bool &") ||
		0 == strcmp(pszRaw, "volatile bool &" ) || 0 == strcmp(pszRaw, "volatile const bool &"))
		pt = PT_boolREF;
	else if (0 == strcmp(pszRaw, "char &") ||  0 == strcmp(pszRaw, "const char &") ||
		0 == strcmp(pszRaw, "volatile char &") ||  0 == strcmp(pszRaw, "volatile const char &"))
		pt = PT_CHARREF;	
	else if(0 == strcmp(pszRaw, "volatile char") || 0 == strcmp(pszRaw, "volatile const char") ||
		0 == strcmp(pszRaw, "char") || 0 == strcmp(pszRaw, "const char"))
		pt = PT_CHAR;
	else if(0 == strcmp(pszRaw, "wchar_t") || 0 == strcmp(pszRaw, "const wchar_t") ||
		0 == strcmp(pszRaw, "volatile wchar_t") || 0 == strcmp(pszRaw, "volatile const wchar_t"))
		pt = PT_WCHAR;
	else if( 0 == strcmp(pszRaw, "wchar_t &") || 0 == strcmp(pszRaw, "const wchar_t &") ||
		0 == strcmp(pszRaw, "volatile wchar_t &") || 0 == strcmp(pszRaw, "volatile const wchar_t &"))
		pt = PT_WCHARREF;
	else if(0 == strcmp(pszRaw, "unsigned char") || 0 == strcmp(pszRaw, "const unsigned char") ||
		0 == strcmp(pszRaw, "volatile unsigned char") || 0 == strcmp(pszRaw, "volatile const unsigned char"))
		pt = PT_UINT1;
	else if(0 == strcmp(pszRaw, "short") || 0 == strcmp(pszRaw, "const short") ||
		0 == strcmp(pszRaw, "volatile short") || 0 == strcmp(pszRaw, "volatile const short"))
		pt = PT_INT2;
	else if(0 == strcmp(pszRaw, "unsigned short") || 0 == strcmp(pszRaw, "const unsigned short") ||
		0 == strcmp(pszRaw, "volatile unsigned short") || 0 == strcmp(pszRaw, "volatile const unsigned short"))
		pt = PT_UINT2;
	else if(0 == strcmp(pszRaw, "int") || 0 == strcmp(pszRaw, "const int") ||
		0 == strcmp(pszRaw, "volatile int") || 0 == strcmp(pszRaw, "volatile const int"))
		pt = PT_INT;
	else if(0 == strcmp(pszRaw, "unsigned int") || 0 == strcmp(pszRaw, "const unsigned int") ||
		0 == strcmp(pszRaw, "volatile unsigned int") || 0 == strcmp(pszRaw, "volatile const unsigned int"))
		pt = PT_UINT;
	else if(0 == strcmp(pszRaw, "long") || 0 == strcmp(pszRaw, "const long") ||
		0 == strcmp(pszRaw, "volatile long") || 0 == strcmp(pszRaw, "volatile const long"))
		pt = PT_INT;
	else if(0 == strcmp(pszRaw, "unsigned long") || 0 == strcmp(pszRaw, "const unsigned long") ||
		0 == strcmp(pszRaw, "volatile unsigned long") || 0 == strcmp(pszRaw, "volatile const unsigned long"))
		pt = PT_UINT;
	else if(0 == strcmp(pszRaw, "__w64 int") || 0 == strcmp(pszRaw, "__w64 const int") ||
		0 == strcmp(pszRaw, "__w64 long") || 0 == strcmp(pszRaw, "__w64 const long") ||
		0 == strcmp(pszRaw, "volatile __w64 int") || 0 == strcmp(pszRaw, "volatile __w64 const int") ||
		0 == strcmp(pszRaw, "volatile __w64 long") || 0 == strcmp(pszRaw, "volatile __w64 const long"))
	{
#if (defined _Wp64 && !defined _WIN64)
		pt = PT_INT;
#endif
	}
	else if(0 == strcmp(pszRaw, "__w64 unsigned int") || 0 == strcmp(pszRaw, "__w64 const unsigned int") ||
		0 == strcmp(pszRaw, "__w64 unsigned long") || 0 == strcmp(pszRaw, "__w64 const unsigned long") ||
		0 == strcmp(pszRaw, "volatile __w64 unsigned int") || 0 == strcmp(pszRaw, "volatile __w64 const unsigned int") ||
		0 == strcmp(pszRaw, "volatile __w64 unsigned long") || 0 == strcmp(pszRaw, "volatile __w64 const unsigned long"))
	{
#if (defined _Wp64 && !defined _WIN64)
		pt = PT_UINT;
#endif
	}
	else if(0 == strcmp(pszRaw, "unsigned char &") || 0 == strcmp(pszRaw, "const unsigned char &") ||
		0 == strcmp(pszRaw, "volatile unsigned char &") || 0 == strcmp(pszRaw, "volatile const unsigned char &"))
		pt = PT_UINT1REF;
	else if( 0 == strcmp(pszRaw, "short &")|| 0 == strcmp(pszRaw, "const short &") ||
		0 == strcmp(pszRaw, "volatile short &")|| 0 == strcmp(pszRaw, "volatile const short &"))
		pt = PT_INT2REF;
	else if( 0 == strcmp(pszRaw, "unsigned short &") || 0 == strcmp(pszRaw, "const unsigned short &") ||
		0 == strcmp(pszRaw, "volatile unsigned short &") || 0 == strcmp(pszRaw, "volatile const unsigned short &"))
		pt = PT_UINT2REF;
	else if(0 == strcmp(pszRaw, "int &") || 0 == strcmp(pszRaw, "const int &") ||
		0 == strcmp(pszRaw, "volatile int &") || 0 == strcmp(pszRaw, "volatile const int &"))
		pt = PT_INTREF;
	else if( 0 == strcmp(pszRaw, "unsigned int &") || 0 == strcmp(pszRaw, "const unsigned int &") ||
		0 == strcmp(pszRaw, "volatile unsigned int &") || 0 == strcmp(pszRaw, "volatile const unsigned int &"))
		pt = PT_UINTREF;
	else if(0 == strcmp(pszRaw, "long &") || 0 == strcmp(pszRaw, "const long &") ||
		0 == strcmp(pszRaw, "volatile long &") || 0 == strcmp(pszRaw, "volatile const long &"))
		pt = PT_INTREF;
	else if( 0 == strcmp(pszRaw, "unsigned long &") || 0 == strcmp(pszRaw, "const unsigned long &") ||
		0 == strcmp(pszRaw, "volatile unsigned long &") || 0 == strcmp(pszRaw, "volatile const unsigned long &"))
		pt = PT_UINTREF;
	else if(0 == strcmp(pszRaw, "__w64 int &") || 0 == strcmp(pszRaw, "__w64 const int &") ||
		0 == strcmp(pszRaw, "__w64 long &") || 0 == strcmp(pszRaw, "__w64 const long &") ||
		0 == strcmp(pszRaw, "volatile __w64 int &") || 0 == strcmp(pszRaw, "volatile __w64 const int &") ||
		0 == strcmp(pszRaw, "volatile __w64 long &") || 0 == strcmp(pszRaw, "volatile __w64 const long &"))
	{
#if (defined _Wp64 && !defined _WIN64)
		pt = PT_INTREF;
#endif
	}
	else if(0 == strcmp(pszRaw, "__w64 unsigned int &") || 0 == strcmp(pszRaw, "__w64 const unsigned int &") ||
		0 == strcmp(pszRaw, "__w64 unsigned long &") || 0 == strcmp(pszRaw, "__w64 const unsigned long &") ||
		0 == strcmp(pszRaw, "volatile __w64 unsigned int &") || 0 == strcmp(pszRaw, "volatile __w64 const unsigned int &") ||
		0 == strcmp(pszRaw, "volatile __w64 unsigned long &") || 0 == strcmp(pszRaw, "volatile __w64 const unsigned long &"))
	{
#if (defined _Wp64 && !defined _WIN64)
		pt = PT_UINTREF;
#endif        	
	}
	else if(0 == strcmp(pszRaw, "__int64") || 0 == strcmp(pszRaw, "const __int64") ||
		0 == strcmp(pszRaw, "volatile __int64") || 0 == strcmp(pszRaw, "volatile const __int64"))
		pt = PT_INT64;
	else if(0 == strcmp(pszRaw, "unsigned __int64") || 0 == strcmp(pszRaw, "const unsigned __int64") ||
		0 == strcmp(pszRaw, "volatile unsigned __int64") || 0 == strcmp(pszRaw, "volatile const unsigned __int64"))
		pt = PT_UINT64;
	else if(0 == strcmp(pszRaw, "__int64 &") || 0 == strcmp(pszRaw, "const __int64 &") ||
		0 == strcmp(pszRaw, "volatile __int64 &") || 0 == strcmp(pszRaw, "volatile const __int64 &"))
		pt = PT_INT64REF;
	else if( 0 == strcmp(pszRaw, "unsigned __int64 &") || 0 == strcmp(pszRaw, "const unsigned __int64 &") ||
		0 == strcmp(pszRaw, "volatile unsigned __int64 &") || 0 == strcmp(pszRaw, "volatile const unsigned __int64 &"))
		pt = PT_UINT64REF;
	else if(0 == strcmp(pszRaw, "struct tagVARIANT") || 0 == strcmp(pszRaw, "const struct tagVARIANT") ||
		0 == strcmp(pszRaw, "volatile struct tagVARIANT") || 0 == strcmp(pszRaw, "volatile const struct tagVARIANT"))
	{
		pt = PT_VARIANT;
		nLen = sizeof(tagVARIANT);
	}
	else if(0 == strcmp(pszRaw, "struct tagVARIANT &") || 0 == strcmp(pszRaw, "const struct tagVARIANT &") ||
		0 == strcmp(pszRaw, "volatile struct tagVARIANT &") || 0 == strcmp(pszRaw, "volatile const struct tagVARIANT &"))	
		pt = PT_VARIANTREF;
	else if(0 == strcmp(pszRaw, "struct _GUID") || 0 == strcmp(pszRaw, "const struct _GUID") ||
		0 == strcmp(pszRaw, "volatile struct _GUID") || 0 == strcmp(pszRaw, "volatile const struct _GUID"))
	{
		pt = PT_GUID;
		nLen = sizeof(_GUID);
	}
	else if(0 == strcmp(pszRaw, "struct _GUID &") || 0 == strcmp(pszRaw, "const struct _GUID &") ||
		0 == strcmp(pszRaw, "volatile struct _GUID &") || 0 == strcmp(pszRaw, "volatile const struct _GUID &"))	
		pt = PT_GUIDREF;
	else if(0 == strncmp(pszRaw, "enum ", 5))
		pt = PT_INT;
	else
	{
		//* &
		if(0 == strcmp(pszRaw, "char *") || 0 == strcmp(pszRaw, "char *const ") || 0 == strcmp(pszRaw, "const char *") || 0 == strcmp(pszRaw, "const char *const ") ||
			0 == strcmp(pszRaw, "volatile char *") || 0 == strcmp(pszRaw, "volatile char *const ") || 0 == strcmp(pszRaw, "volatile const char *") || 0 == strcmp(pszRaw, "volatile const char *const "))
			pt = PT_CHARPOINT;
		else if(0 == strcmp(pszRaw, "char *& ") || 0 == strcmp(pszRaw, "char *const & ") || 0 == strcmp(pszRaw, "const char *& ")  ||  0 == strcmp(pszRaw, "const char *const & ") || 
			0 == strcmp(pszRaw, "volatile char *& ") || 0 == strcmp(pszRaw, "volatile char *const & ") || 0 == strcmp(pszRaw, "volatile const char *& ")  ||  0 == strcmp(pszRaw, "volatile const char *const & "))
			pt = PT_CHARPOINTREF;
		else if(0 == strcmp(pszRaw, "wchar_t *") || 0 == strcmp(pszRaw, "const wchar_t *") ||
			0 == strcmp(pszRaw, "const wchar_t *const ") || 0 == strcmp(pszRaw, "wchar_t *const ") ||
			0 == strcmp(pszRaw, "volatile wchar_t *") || 0 == strcmp(pszRaw, "volatile const wchar_t *") ||
			0 == strcmp(pszRaw, "volatile const wchar_t *const ") || 0 == strcmp(pszRaw, "volatile wchar_t *const ")) //wchar_t*
			pt = PT_WCHARPOINT;
		else if(0 == strcmp(pszRaw, "const wchar_t *& ") ||  0 == strcmp(pszRaw, "wchar_t *& ")  ||
			0 == strcmp(pszRaw, "const wchar_t *const & ") || 0 == strcmp(pszRaw, "wchar_t *const & ") ||
			0 == strcmp(pszRaw, "volatile const wchar_t *& ") ||  0 == strcmp(pszRaw, "volatile wchar_t *& ")  ||
			0 == strcmp(pszRaw, "volatile const wchar_t *const & ") || 0 == strcmp(pszRaw, "volatile wchar_t *const & ")) //const wchar_t*
			pt = PT_WCHARPOINTREF;
		else if( 0 == strcmp(pszRaw, "short *")|| 0 == strcmp(pszRaw, "const short *") ||
			0 == strcmp(pszRaw, "volatile short *")|| 0 == strcmp(pszRaw, "volatile const short *"))
			pt = PT_INT2REF;
		else if(0 == strcmp(pszRaw, "unsigned short *") || 0 == strcmp(pszRaw, "const unsigned short *") ||
			0 == strcmp(pszRaw, "const unsigned short *const ") || 0 == strcmp(pszRaw, "unsigned short *const ") ||
			0 == strcmp(pszRaw, "volatile unsigned short *") || 0 == strcmp(pszRaw, "volatile const unsigned short *") ||
			0 == strcmp(pszRaw, "volatile const unsigned short *const ") || 0 == strcmp(pszRaw, "volatile unsigned short *const ")) //wchar_t*
		{
#if (!(defined _WCHAR_T_DEFINED && defined _NATIVE_WCHAR_T_DEFINED))
			pt = PT_WCHARPOINT;
#else
			pt = PT_INT2REF;
#endif
		}
		else if(0 == strcmp(pszRaw, "const unsigned short *& ") ||  0 == strcmp(pszRaw, "unsigned short *& ")  ||
			0 == strcmp(pszRaw, "const unsigned short *const & ") || 0 == strcmp(pszRaw, "unsigned short *const & ") ||
			0 == strcmp(pszRaw, "volatile const unsigned short *& ") ||  0 == strcmp(pszRaw, "volatile unsigned short *& ")  ||
			0 == strcmp(pszRaw, "volatile const unsigned short *const & ") || 0 == strcmp(pszRaw, "volatile unsigned short *const & ") ) //const wchar_t*
		{
#if (!(defined _WCHAR_T_DEFINED && defined _NATIVE_WCHAR_T_DEFINED))
			pt = PT_WCHARPOINTREF;
#endif
		}
		else if(0 == strcmp(pszRaw, "bool *" ) || 0 == strcmp(pszRaw, "const bool *") ||
			0 == strcmp(pszRaw, "volatile bool *") || 0 == strcmp(pszRaw, "volatile const bool *"))
			pt = PT_boolREF;
		else if(0 == strcmp(pszRaw, "int *") || 0 == strcmp(pszRaw, "const int *")||
			0 == strcmp(pszRaw, "volatile int *") || 0 == strcmp(pszRaw, "volatile const int *"))
			pt = PT_INTREF;
		else if( 0 == strcmp(pszRaw, "unsigned int *") || 0 == strcmp(pszRaw, "const unsigned int *") ||
			0 == strcmp(pszRaw, "volatile unsigned int *") || 0 == strcmp(pszRaw, "volatile const unsigned int *"))
			pt = PT_INTREF;
		else if(0 == strcmp(pszRaw, "long *") || 0 == strcmp(pszRaw, "const long *") ||
			0 == strcmp(pszRaw, "volatile long *") || 0 == strcmp(pszRaw, "volatile const long*"))
			pt = PT_INTREF;
		else if( 0 == strcmp(pszRaw, "unsigned long *") || 0 == strcmp(pszRaw, "const unsigned long *") ||
			0 == strcmp(pszRaw, "volatile unsigned long *") || 0 == strcmp(pszRaw, "volatile const unsigned long *"))
			pt = PT_INTREF;
		else if(0 == strcmp(pszRaw, "__int64 *") || 0 == strcmp(pszRaw, "const __int64 *") ||
			0 == strcmp(pszRaw, "volatile __int64 *") || 0 == strcmp(pszRaw, "volatile const __int64 *"))
			pt = PT_INT64REF;
		else if( 0 == strcmp(pszRaw, "unsigned __int64 *") || 0 == strcmp(pszRaw, "const unsigned __int64 *") ||
			0 == strcmp(pszRaw, "volatile unsigned __int64 *") || 0 == strcmp(pszRaw, "volatile const unsigned __int64 *"))
			pt = PT_INT64REF;
		else if(0 == strcmp(pszRaw, "__int64 *& ") || 0 == strcmp(pszRaw, "const __int64 *& ") ||
			0 == strcmp(pszRaw, "volatile __int64 *& ") || 0 == strcmp(pszRaw, "volatile const __int64 *& "))
			pt = PT_INT64REF;
		else if( 0 == strcmp(pszRaw, "unsigned __int64 *& ") || 0 == strcmp(pszRaw, "const unsigned __int64 *& ") ||
			0 == strcmp(pszRaw, "volatile unsigned __int64 *& ") || 0 == strcmp(pszRaw, "volatile const unsigned __int64 *& "))
			pt = PT_INT64REF;
		else if(0 == strcmp(pszRaw, "struct tagVARIANT *") || 0 == strcmp(pszRaw, "const struct tagVARIANT *") ||
			0 == strcmp(pszRaw, "volatile struct tagVARIANT *") || 0 == strcmp(pszRaw, "volatile const struct tagVARIANT *"))		
			pt = PT_VARIANTREF;
		else if(0 == strcmp(pszRaw, "struct _GUID *") || 0 == strcmp(pszRaw, "const struct _GUID *") ||
			0 == strcmp(pszRaw, "volatile struct _GUID *") || 0 == strcmp(pszRaw, "volatile const struct _GUID *"))		
			pt = PT_GUIDREF;		
		else
		{
			pt = PT_PENDPOINT;
			while('\0' != *(++pData) && ('*' != *pData)) ; 
			if('\0' == *pData) //不是指针
			{
				nLen = 4;
				pt = PT_PEND;
			}	
		}
	}
	return TRUE;
}

BOOL CNTLog::TranslateCodePage(char ** ppszValA)
{
	UINT code_page = GetStringCodePage(*ppszValA);
	int  nLen = MultiByteToWideChar(code_page  , 0, *ppszValA, -1,NULL,0);
	if(0 == nLen)
	{
		code_page= 0;
		nLen = MultiByteToWideChar(code_page  , 0, *ppszValA, -1,NULL,0);
	}
	wchar_t szBufferW[4096] = {0};
	static char szBufferA[4096] = {0};
	if(nLen > 0 && nLen <2048)
	{
		BOOL b = TRUE;
		nLen = MultiByteToWideChar(code_page, 0, *ppszValA, -1, szBufferW, nLen);
		szBufferW[nLen] = '\0';
		nLen = WideCharToMultiByte(936, 0, szBufferW, nLen, szBufferA, 2048, "_", &b);
		if(nLen > 0)
		{
			szBufferA[nLen] = '\0';
			*ppszValA = szBufferA;
			return TRUE;
		}	
	}
	return FALSE;
}
DWORD CNTLog::GetSafeValidValue(const CNTLog::PARAM_TYPE &ntlog_eParamType,LPSTR ntlog_szRegValue, DWORD ntlog_nRegValue, DWORD ntlog_nRegValue2 /*= 0*/,
				   LPINT ntlog_pnRegValue /*= NULL*/, LPINT ntlog_pnRegValue2 /*= NULL*/ , LPSTR ntlog_pRegValueA /*= NULL */, LPWSTR ntlog_pRegValueW/* = NULL*/, LPINT pnRet/* = 0*/)
{
	DWORD dwRet = 0;
 	__try
	{
#if(defined W2A || defined OLE2A)
		USES_CONVERSION;
#endif
		switch(ntlog_eParamType)
		{
		case CNTLog::PT_INVALID:
#pragma warning(push)
//#pragma warning(disable : 4312 )
			{
				if(0 < ntlog_nRegValue)
					dwRet = *(LPDWORD)(ULONG_PTR)ntlog_nRegValue;
				else if( 0 < ntlog_nRegValue2)
					dwRet = *(LPDWORD)(ULONG_PTR)ntlog_nRegValue2;
				if(NULL != pnRet)
					*pnRet = 0;
				return dwRet;
			}
//#pragma warning(pop)
			break;
		case CNTLog::PT_CHARREF:
			memcpy (&ntlog_pRegValueA, &ntlog_nRegValue, sizeof(ntlog_nRegValue));			
			TranslateCodePage(&ntlog_pRegValueA);
			dwRet = _snprintf(ntlog_szRegValue,MAX_USERDATA_SIZE, " (0x%08x) = 0x%08hx('%c')", ntlog_nRegValue,(UINT)(CHAR)(*ntlog_pRegValueA), (CHAR)*ntlog_pRegValueA);		 			
			break; 
		case CNTLog::PT_WCHARREF:
			memcpy(&ntlog_pRegValueW, &ntlog_nRegValue, sizeof(ntlog_nRegValue));
			dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," (0x%08x) = 0x%08hx(L'%C')", ntlog_nRegValue, (unsigned short)(*ntlog_pRegValueW) , (wchar_t)*ntlog_pRegValueW);
			break;
		case CNTLog::PT_CHARPOINT:
			memcpy(&ntlog_pRegValueA, &ntlog_nRegValue, sizeof(ntlog_nRegValue));
			TranslateCodePage(&ntlog_pRegValueA);
			if(s_release.m_si.lpMinimumApplicationAddress > ntlog_pRegValueA || ntlog_pRegValueA > s_release.m_si.lpMaximumApplicationAddress)
			{
                dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE, " = 0x%08Ix(\"%s\")", ntlog_pRegValueA,  "Is Invalid Point!!!");
			}
			else
			{
				if(NULL != ntlog_pRegValueA && MAX_USERDATA_SIZE/2 < strlen((LPSTR)ntlog_pRegValueA))
					dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE, " = 0x%08Ix(\"%s\")", ntlog_pRegValueA,  "Length TOO LONG");
				else
					dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE, " = 0x%08Ix(\"%s\")", ntlog_pRegValueA,  (LPSTR)ntlog_pRegValueA);
			}
			break;
		case CNTLog::PT_WCHARPOINT:
			memcpy(&ntlog_pRegValueW, &ntlog_nRegValue, sizeof(ntlog_nRegValue));
			if(s_release.m_si.lpMinimumApplicationAddress > ntlog_pRegValueW || ntlog_pRegValueW > s_release.m_si.lpMaximumApplicationAddress)
			{
				dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE, " = 0x%08Ix(\"%s\")", ntlog_pRegValueW,  "Is Invalid Point!!!");
			}
			else
			{
				if(NULL != ntlog_pRegValueW && MAX_USERDATA_SIZE/2 < wcslen((LPWSTR)ntlog_pRegValueW))
					dwRet = _snprintf(ntlog_szRegValue,MAX_USERDATA_SIZE , " = 0x%08Ix(L\"%s\")", ntlog_pRegValueW , "Length TOO LONG");
				else
				{
					char szBufferA[2048] = {0};
					int nLen = (int)wcslen(ntlog_pRegValueW);
					BOOL b = TRUE;
					WideCharToMultiByte(936, 0, ntlog_pRegValueW, nLen, szBufferA, 2048, "_", &b);
					dwRet = _snprintf(ntlog_szRegValue,MAX_USERDATA_SIZE , " = 0x%08Ix(L\"%s\")", ntlog_pRegValueW , szBufferA);
				}				
			}
			break;
		case CNTLog::PT_CHARPOINTREF:
			memcpy(&ntlog_pRegValueA, &ntlog_nRegValue, sizeof(ntlog_nRegValue));
			memcpy(&ntlog_pRegValueA, ntlog_pRegValueA, sizeof(ntlog_nRegValue));
			TranslateCodePage(&ntlog_pRegValueA);
			if(s_release.m_si.lpMinimumApplicationAddress > ntlog_pRegValueA || ntlog_pRegValueA > s_release.m_si.lpMaximumApplicationAddress)
			{
				dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE, " = 0x%08Ix(\"%s\")", ntlog_pRegValueA,  "Is Invalid Point!!!");
			}
			else
			{
				if(NULL != ntlog_pRegValueA && MAX_USERDATA_SIZE/2 < strlen((LPSTR)ntlog_pRegValueA))
					dwRet = _snprintf(ntlog_szRegValue,MAX_USERDATA_SIZE ," (0x%08x) = 0x%08Ix(\"%s\")", ntlog_nRegValue, ntlog_pRegValueA, "Length TOO LONG");
				else
					dwRet = _snprintf(ntlog_szRegValue,MAX_USERDATA_SIZE ," (0x%08x) = 0x%08Ix(\"%s\")", ntlog_nRegValue, ntlog_pRegValueA, (LPSTR)ntlog_pRegValueA);
			}
			break;
		case CNTLog::PT_WCHARPOINTREF:
			memcpy(&ntlog_pRegValueW, &ntlog_nRegValue, sizeof(ntlog_nRegValue));
			memcpy(&ntlog_pRegValueW, ntlog_pRegValueW, sizeof(ntlog_nRegValue));
			if(s_release.m_si.lpMinimumApplicationAddress > ntlog_pRegValueW || ntlog_pRegValueW > s_release.m_si.lpMaximumApplicationAddress)
			{
				dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE, " = 0x%08Ix(\"%s\")", ntlog_pRegValueW,  "Is Invalid Point!!!");
			}
			else
			{
				if(NULL != ntlog_pRegValueW && MAX_USERDATA_SIZE/2 < wcslen((LPWSTR)ntlog_pRegValueW))
					dwRet = _snprintf(ntlog_szRegValue,MAX_USERDATA_SIZE , " (0x%08x) = 0x%08Ix(L\"%s\")", ntlog_nRegValue, ntlog_pRegValueW, "Length TOO LONG");
				else
				{
					char szBufferA[2048] = {0};
					int nLen = (int)wcslen(ntlog_pRegValueW);
					BOOL b = TRUE;
					WideCharToMultiByte(936, 0, ntlog_pRegValueW, nLen, szBufferA, 2048, "_", &b);
					dwRet = _snprintf(ntlog_szRegValue,MAX_USERDATA_SIZE , " (0x%08x) = 0x%08Ix(L\"%s\")", ntlog_nRegValue, ntlog_pRegValueW, szBufferA);					
				}
				/*
#ifdef W2A			
					dwRet = _snprintf(ntlog_szRegValue,MAX_USERDATA_SIZE , " (0x%08x) = 0x%08Ix(L\"%s\")", ntlog_nRegValue, ntlog_pRegValueW, W2A(ntlog_pRegValueW));
#else
					dwRet = _snprintf(ntlog_szRegValue,MAX_USERDATA_SIZE , " (0x%08x) = 0x%08Ix(L\"%S\"?)", ntlog_nRegValue, ntlog_pRegValueW, ntlog_pRegValueW);
#endif			*/
			}
			break;
		case CNTLog::PT_boolREF:
			memcpy(&ntlog_pnRegValue ,&ntlog_nRegValue, sizeof(ntlog_nRegValue));
			if(0 == ntlog_pnRegValue)
			{
				dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE ," (0x%08x) = (null)", ntlog_nRegValue);
			}
			else
			{
				if(0 == *ntlog_pnRegValue)
					dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE, " (0x%08x) = 0x%08x(false)", ntlog_nRegValue ,  *ntlog_pnRegValue);
				else
					dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE, " (0x%08x) = 0x%08x(true)", ntlog_nRegValue , *ntlog_pnRegValue);
			}
			break;		
		case CNTLog::PT_UINT1REF:
			memcpy(&ntlog_pnRegValue ,&ntlog_nRegValue, sizeof(ntlog_nRegValue));
			dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE, " (0x%08x) = 0x%08x(%u)",ntlog_nRegValue , (UINT)(CHAR)*ntlog_pnRegValue,(UINT)(unsigned char)*ntlog_pnRegValue);
			break;
		case CNTLog::PT_INT2REF:
			memcpy(&ntlog_pnRegValue ,&ntlog_nRegValue, sizeof(ntlog_nRegValue));
			dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," (0x%08x) = 0x%08x(%hd)",ntlog_nRegValue , (UINT)(unsigned short)*ntlog_pnRegValue,(short)*ntlog_pnRegValue);
			break;
		case CNTLog::PT_UINT2REF:
			memcpy(&ntlog_pnRegValue ,&ntlog_nRegValue, sizeof(ntlog_nRegValue));
			dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," (0x%08x) = 0x%08x(%hu)",ntlog_nRegValue , (UINT)(unsigned short)*ntlog_pnRegValue,(unsigned short)*ntlog_pnRegValue);
			break;
		case CNTLog::PT_INTREF:
			memcpy(&ntlog_pnRegValue ,&ntlog_nRegValue, sizeof(ntlog_nRegValue));
			if(0 == ntlog_pnRegValue)
				dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE, " (0x%08x) = (null)", ntlog_nRegValue);
			else
				dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE, " (0x%08x) = 0x%08x(%d)", ntlog_nRegValue, *ntlog_pnRegValue,*ntlog_pnRegValue);
			break;
		case CNTLog::PT_UINTREF:
			memcpy(&ntlog_pnRegValue ,&ntlog_nRegValue, sizeof(ntlog_nRegValue));
			if(0 == ntlog_pnRegValue)
				dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE, " (0x%08x) = (null)", ntlog_nRegValue);
			else
				dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE, " (0x%08x) = 0x%08x(%u)", ntlog_nRegValue, *ntlog_pnRegValue,*ntlog_pnRegValue);
			break;
		case CNTLog::PT_INT64REF:
			{
				memcpy(&ntlog_pnRegValue ,&ntlog_nRegValue, sizeof(ntlog_nRegValue));
				memcpy(&ntlog_pnRegValue2 ,&ntlog_nRegValue2, sizeof(ntlog_nRegValue2));
				DWORD nRegValue = 0;
				DWORD nRegValue2 = 0;
				nRegValue =  ntlog_pnRegValue ?  *ntlog_pnRegValue : 0;
				nRegValue2 = ntlog_pnRegValue2 ?  *ntlog_pnRegValue2 : 0;
				UINT64 ullRegValue=(UINT64)nRegValue2<<32;
				ullRegValue+=nRegValue;
				dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE, " (0x%08x)(0x%08x) = 0x%016I64x(%I64d)", ntlog_nRegValue2, ntlog_nRegValue,
					(UINT64)ullRegValue, (INT64)ullRegValue);
			}
			break;
		case CNTLog::PT_UINT64REF:
			{
				memcpy(&ntlog_pnRegValue ,&ntlog_nRegValue, sizeof(ntlog_nRegValue));
				memcpy(&ntlog_pnRegValue2 ,&ntlog_nRegValue2, sizeof(ntlog_nRegValue2));
				DWORD nRegValue = 0;
				DWORD nRegValue2 = 0;
				nRegValue =  ntlog_pnRegValue ?  *ntlog_pnRegValue : 0;
				nRegValue2 = ntlog_pnRegValue2 ?  *ntlog_pnRegValue2 : 0;
				UINT64 ullRegValue=(UINT64)nRegValue2<<32;
				ullRegValue+=nRegValue;
				dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE, " (0x%08x)(0x%08x) = 0x%016I64x(%I64u)", ntlog_nRegValue2, ntlog_nRegValue,
					(UINT64)ullRegValue, (INT64)ullRegValue);
			}
			break;
		case CNTLog::PT_VARIANT:
		case CNTLog::PT_VARIANTREF:
			{
				tagVARIANT * pVar = NULL;
				if(PT_VARIANT == ntlog_eParamType)
					pVar = (tagVARIANT *)ntlog_pnRegValue;
				else
					memcpy(&pVar, &ntlog_nRegValue, sizeof(ntlog_nRegValue));
				if(NULL == pVar)
				{
					dwRet = _snprintf(ntlog_szRegValue,MAX_USERDATA_SIZE, " = NULL");
					break;
				}
				switch (pVar->vt)
				{
				case VT_EMPTY:
					dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = {VT_EMPTY}");
					break;
				case VT_NULL:
					dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = {VT_NULL}");
					break;
				case VT_ERROR:
					dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = {0x%08x(%d) VT_ERROR}", pVar->scode, pVar->scode);
					break;
				case VT_I1:
					dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = {0x%08x(%d) VT_I1}", (INT)pVar->bVal, (INT)pVar->bVal);
                    break;
				case VT_I2:
					dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = {0x%08x(%d) VT_I2}", (INT)pVar->iVal, (INT)pVar->iVal);
					break;
				case VT_I4:
					dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = {0x%08x(%d) VT_I4}", (INT)pVar->intVal, (INT)pVar->intVal);
					break;
				case VT_I8:
					dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = {0x%I64x(%I64d) VT_I8}", (ULONGLONG)pVar->ullVal , (LONGLONG)pVar->llVal);
					break;
				case VT_UI1:
					dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = {0x%08x(%u) VT_UI1}", (UINT)pVar->bVal, (UINT)pVar->bVal);
					break;
				case VT_UI2:
					dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = {0x%08x(%u) VT_UI2}", (UINT)pVar->iVal, (UINT)pVar->iVal);
					break;
				case VT_UI4:
					dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = {0x%08x(%u) VT_UI4}", (UINT)pVar->intVal, (UINT)pVar->intVal);
					break;
				case VT_UI8:
					dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = {0x%I64x(%I64u) VT_UI8}", (ULONGLONG)pVar->ullVal , (LONGLONG)pVar->llVal);
				case VT_BOOL:
					dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = {0x%08x(%d) VT_BOOL}", (INT)pVar->intVal, (INT)pVar->intVal);
					break;
				case VT_INT:
					dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = {0x%08x(%d) VT_INT}", (INT)pVar->intVal, (INT)pVar->intVal);
				case VT_UINT:
					dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = {0x%08x(%u) VT_UINT}", (UINT)pVar->intVal, (UINT)pVar->intVal);
				case VT_HRESULT:
					dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = {0x%08x(%d) VT_HRESULT}", (UINT)pVar->intVal, (INT)pVar->intVal);
					break;
				case VT_R8:
					dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = {%f VT_R8}", pVar->dblVal);
					break;
#ifdef OLE2A 
				case VT_BSTR:
					if(pVar->bstrVal)
					{
						if(MAX_USERDATA_SIZE/2 < wcslen(pVar->bstrVal))
							dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = {L\"%s\" VT_BSTR}", "Length TOO LONG");
						else
						{
							char szBufferA[2048] = {0};
							int nLen = (int)wcslen(pVar->bstrVal);
							BOOL b = TRUE;
							WideCharToMultiByte(936, 0, pVar->bstrVal, nLen, szBufferA, 2048, "_", &b);
							dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE, " = {L\"%s\" VT_BSTR}", szBufferA);
						}							
					}
					else
						dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = {L\"%s\" VT_BSTR}", "NULL");
					break; 
#else
				case VT_BSTR:
					dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = {L\"%S\"? VT_BSTR}", pVar->bstrVal);
					break; 
#endif
#ifdef W2A
				case VT_LPWSTR:
					if(pVar->bstrVal)
					{
						if(MAX_USERDATA_SIZE/2 < wcslen(pVar->bstrVal))
							dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = {L\"%s\" VT_BSTR}", "Length TOO LONG");
						else
							dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = {L\"%s\" VT_LPWSTR}", OLE2A(pVar->bstrVal));
					}
					else
						dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = {L\"%s\" VT_BSTR}", "NULL");
					break;
#else
				case VT_LPWSTR:
					dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = {L\"%S\"? VT_LPWSTR}", pVar->bstrVal);
					break;
#endif
				case VT_LPSTR:
					dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = {\"%s\" VT_LPSTR}", pVar->pcVal);
					break;
				case VT_DISPATCH:
					dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = {0x%p VT_DISPATCH}", (void*)pVar->pdispVal);
					break;
				default:
					dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = {0x%I64x(%I64d) VT_?[0x%08x(%d)]}", (ULONGLONG)pVar->llVal,(LONGLONG)pVar->llVal, pVar->vt, pVar->vt);
					break;
				}
			}
			break;
		case CNTLog::PT_GUID:
		case CNTLog::PT_GUIDREF:
			{
				_GUID * pGUID = NULL;
				LPOLESTR lpGUIDString = NULL;
				if(PT_GUID == ntlog_eParamType)
					pGUID = (_GUID *)ntlog_pnRegValue;
				else
					memcpy(&pGUID, &ntlog_nRegValue, sizeof(ntlog_nRegValue));
				if(NULL == pGUID)
				{
					dwRet = _snprintf(ntlog_szRegValue,MAX_USERDATA_SIZE, " = NULL");
					break;
				}
				if(S_OK == StringFromIID(*pGUID, &lpGUIDString))
				{
#ifdef OLE2A
					dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = %s", W2A(lpGUIDString));
#else
					dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = %S", lpGUIDString);
#endif
					CoTaskMemFree(lpGUIDString);
				}
				else
					dwRet = _snprintf(ntlog_szRegValue, MAX_USERDATA_SIZE," = {StringFromIID ERROR}");
			}
			break;
		default:
			break;
		}
		if((DWORD)-1 == dwRet)
			dwRet = (int)strlen(ntlog_szRegValue);
		if(NULL != pnRet)
			*pnRet = 0;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = _snprintf(ntlog_szRegValue,MAX_USERDATA_SIZE, " = 0x%08x(!!!)", ntlog_nRegValue);
		if(NULL != pnRet)
			*pnRet = -1;
	}
	return dwRet;
}

CNTLog::CEnterLeaveFunCallBack::CEnterLeaveFunCallBack(LPCTSTR pszCallFunName ,LPCTSTR pszFunction, LPCTSTR pszFuncSig,LPCVOID pvThis, BOOL bIsSimple, DWORD dwEBP, LPCSTR pszFuncSigA)
: m_pvThis(pvThis), m_pszFunction(pszFunction), m_pszFuncSig(pszFuncSig), m_pszCallFunName(pszCallFunName), m_dwEBP(dwEBP), m_pszFuncSigA(pszFuncSigA)
{
    if(NULL == CNTLog::s_pThis || CNTLog::s_pThis->s_release.m_bIsModuleUnloading)
    {
        TCHAR szDebugViewMsg[1024] = {0}; //module exit, so alloc memory from stack 
        _sntprintf(szDebugViewMsg, MAX_PRIVATEDATA_SIZE, _T("[%d] <WARN> <%s> "), GetCurrentThreadId(), CNTLog::GetCurrentModuleName());
        std::basic_string<TCHAR> strFunction = (std::basic_string<TCHAR>)szDebugViewMsg + (std::basic_string<TCHAR>) _T("[") + 
            m_pszFunction +_T("] ") + (std::basic_string<TCHAR>)m_pszCallFunName + _T(" Module Unloading!!!") +_T("\n");
        OutputDebugString(strFunction.c_str());
        return;
    }
    SectionProtect sp(&CNTLog::s_pThis->_cs);
    CNTLog::GetInstance(m_pszFunction, m_pvThis).OutputLog(LEVEL_TRACE, m_pszCallFunName, NULL);
    if(bIsSimple)
        ;
    else
        LeaveCriticalSection(&CNTLog::s_pThis->_cs);//TraceAuto ++ EnterCriticalSection
}

CNTLog::CEnterLeaveFunCallBack::~CEnterLeaveFunCallBack()
{
    TCHAR szFunction[MAX_LEAVE_FUNNAME_LENGTH];
    _sntprintf(szFunction , MAX_LEAVE_FUNNAME_LENGTH - 1, _T("%s%s"), LEAVE_FUNCTION_FLAG, &m_pszFunction[OFFSET_LEAVE_FUNCTION_FLAG]);
    if(NULL == CNTLog::s_pThis)
    {
        TCHAR szDebugViewMsg[MAX_PRIVATEDATA_SIZE] = {0}; //module exit, so alloc memory from stack 
        _sntprintf(szDebugViewMsg, MAX_PRIVATEDATA_SIZE, _T("[%d] WARN <%s> "), GetCurrentThreadId(), CNTLog::GetCurrentModuleName());
        std::basic_string<TCHAR> strFunction = (std::basic_string<TCHAR>)szDebugViewMsg + (std::basic_string<TCHAR>) _T("[") + 
            szFunction + _T("] ") + /*(std::basic_string<TCHAR>)m_pszCallFunName +*/ _T(" Module Unloading!!!") + _T("\n");
        OutputDebugString(strFunction.c_str());
        return;
    }
    if(m_dwEBP) 
    {
        LPCTSTR ntlog_lpctszFuncSig = CNTLog::TraceAuto(m_pszFuncSigA,m_dwEBP,NULL == m_pvThis);
        CNTLog::GetInstance(szFunction , m_pvThis).OutputLog(LEVEL_TRACE, ntlog_lpctszFuncSig, NULL);
        LeaveCriticalSection(&CNTLog::s_pThis->_cs);//TraceAuto ++ EnterCriticalSection
    }
    else
    {
        CNTLog::GetInstance(szFunction , m_pvThis).NTTrace( m_pszFuncSig);
    }			
}

CNTLog::CRelease::CRelease(INT) : m_bIsModuleUnloading(FALSE), m_hLang(NULL)
{
    InitializeCriticalSection(&CNTLog::s_cs);
    memset(&m_si, 0x00, sizeof(SYSTEM_INFO));
    GetSystemInfo(&m_si);
    m_hLang = LoadLibrary(_T("mlang.dll"));
    m_dwMainThreadID = GetCurrentThreadId();
    CNTLog::s_dwTlsIndex = TlsAlloc();
    CNTLog::s_dwTlsIndex_this = TlsAlloc();
    if(TLS_OUT_OF_INDEXES == CNTLog::s_dwTlsIndex)
        CNTLog::s_dwTlsIndex = TlsAlloc();
    if(TLS_OUT_OF_INDEXES == CNTLog::s_dwTlsIndex_this)
        CNTLog::s_dwTlsIndex_this = TlsAlloc();
    CNTLog::GetInstance();//保证~CRelease( )不异常，且打印模块初始化，退出等日志
}

CNTLog::CRelease::~CRelease()// 使用静态成员的析构函数，实现单一实例的释放
{
    ::InterlockedExchangeAdd(&m_bIsModuleUnloading,1L);
    s_pThis->NTPrompt(_T("Module Unload. Path = %s, Current LogGroup : \"%s\", ModuleID : %d, RefCount : %d"), s_szModuleFileName, __TNTLOG_GROUP, s_nModuleID , s_pThis->_lRef - 1);
    DeleteCriticalSection(&CNTLog::s_cs);
    TlsFree(CNTLog::s_dwTlsIndex);
    TlsFree(CNTLog::s_dwTlsIndex_this);
    if( NULL == s_hMutex )
        return;
    ::WaitForSingleObject( s_hMutex, INFINITE ); //待第一个初始化完成,其它模块不构建NTLOG实例,因此不需要使用Mutex
    HANDLE hMutex = s_hMutex;
    s_hMutex = NULL;
    LONG nRef = ::InterlockedDecrement((volatile LONG*)&s_pThis->_lRef );
    if( nRef <= 0 )
    {
        if(INVALID_HANDLE_VALUE != s_pThis->_hConfigFileChanged)
            FindCloseChangeNotification(s_pThis->_hConfigFileChanged);
        if( INVALID_HANDLE_VALUE != s_pThis->m_hLogFile )
            ::CloseHandle( s_pThis->m_hLogFile );
        DeleteCriticalSection(&s_pThis->_cs);
        s_pThis->m_ghvInclude.Clear();
        s_pThis->m_ghvExclude.Clear();
        HGLOBAL hGlobal = (HGLOBAL)s_pThis->m_pszLogDataA;
        if(hGlobal)
        {
            GlobalUnlock(hGlobal);
            GlobalFree(hGlobal);
            hGlobal = NULL;
            s_pThis->m_pszLogDataA = NULL;
        }
    }
    if( INVALID_HANDLE_VALUE != s_pThis->s_hLogFile)
        ::CloseHandle( s_pThis->s_hLogFile );			
    ::UnmapViewOfFile( s_pThis );
    s_pThis = NULL;
    ::CloseHandle( s_hFileMap );
    s_hFileMap = NULL;
    if (hMutex)
    {
        ::ReleaseMutex( hMutex );
        ::CloseHandle( hMutex );
    }
    if(m_hLang)
    {
        FreeLibrary(m_hLang);
        m_hLang = NULL;
    }
}

CNTLog::CNTGHeapVector::CNTGHeapVector() :m_pNTGHeapVector(NULL)
{}

void CNTLog::CNTGHeapVector::Add(PNTGHeapVector pNTGHeapVector)
{
    if(NULL == pNTGHeapVector)
        return;
    if(NULL == m_pNTGHeapVector)
    {
        m_pNTGHeapVector = pNTGHeapVector;
        return;
    }
    PNTGHeapVector pTmpNTGHeapVector = NULL;
    pTmpNTGHeapVector = m_pNTGHeapVector;
    for ( ; NULL != pTmpNTGHeapVector->pNext ; pTmpNTGHeapVector = pTmpNTGHeapVector->pNext)
        ;
    pTmpNTGHeapVector->pNext = pNTGHeapVector;
}

void CNTLog::CNTGHeapVector::Add(LPCTSTR lpszFilter)
{
    if(NULL == lpszFilter)
        return;
    {
#if (defined(UNICODE) || defined(_UNICODE))
        std::string strpv;
        CNTLog::UnicodeToMultiByte(lpszFilter, strpv);
        LPCSTR lpszFilter = strpv.c_str();
#endif
        size_t size = strlen(lpszFilter);
        if(0 >= size)
            return;
        HGLOBAL hGlobal = GlobalAlloc(GMEM_FIXED,(size + 1) * sizeof(CHAR) + sizeof(LPSTR) + sizeof(_tagNTGHeapVector*));
        PNTGHeapVector pNTGHeapVector = (PNTGHeapVector)GlobalLock(hGlobal);
        if(NULL == pNTGHeapVector)
            return;
        pNTGHeapVector->pszData = (LPSTR)pNTGHeapVector + sizeof(LPSTR)/sizeof(CHAR) + sizeof(_tagNTGHeapVector*);
        memcpy(pNTGHeapVector->pszData, lpszFilter, (size + 1) * sizeof(CHAR));
        pNTGHeapVector->pNext = NULL;
        Add(pNTGHeapVector);
    }
}
void CNTLog::CNTGHeapVector::Clear()
{
    PNTGHeapVector pTmpNTGHeapVector = NULL;
    PNTGHeapVector pDelNTGHeapVector = NULL;
    pTmpNTGHeapVector = m_pNTGHeapVector;
    while(pTmpNTGHeapVector)
    {
        pDelNTGHeapVector = pTmpNTGHeapVector;
        pTmpNTGHeapVector = pTmpNTGHeapVector->pNext;
        GlobalUnlock(pDelNTGHeapVector);
        GlobalFree(pDelNTGHeapVector);
        pDelNTGHeapVector = NULL;
    }
    m_pNTGHeapVector = NULL;
}

BOOL CNTLog::CNTGHeapVector::Find(LPCTSTR lpszFilter)
{
    PNTGHeapVector pTmpNTGHeapVector = NULL;
    pTmpNTGHeapVector = m_pNTGHeapVector;
    while(pTmpNTGHeapVector)
    {
#if (defined(UNICODE) || defined(_UNICODE))
        std::string strpv;
        CNTLog::UnicodeToMultiByte(lpszFilter, strpv);
        LPCSTR lpszFilter = strpv.c_str();
#endif
        if( 0 == strcmp(pTmpNTGHeapVector->pszData, lpszFilter))
            return TRUE;
        else
            pTmpNTGHeapVector = pTmpNTGHeapVector->pNext;					
    }
    return FALSE;
}

BOOL CNTLog::CNTGHeapVector::SubFind(LPCTSTR lpszFilter)
{
    PNTGHeapVector pTmpNTGHeapVector = NULL;
    pTmpNTGHeapVector = m_pNTGHeapVector;
    while(pTmpNTGHeapVector)
    {
#if (defined(UNICODE) || defined(_UNICODE))
        std::string strpv;
        CNTLog::UnicodeToMultiByte(lpszFilter, strpv);
        LPCSTR lpszFilter = strpv.c_str();
#endif
        if( 0 != strstr(lpszFilter, pTmpNTGHeapVector->pszData))
            return TRUE;
        else
            pTmpNTGHeapVector = pTmpNTGHeapVector->pNext;
    }
    return FALSE;
}

