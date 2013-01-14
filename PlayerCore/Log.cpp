
#include "stdafx.h"
#include "Log.h"

CLog::CLog()
: m_bEnable(FALSE)
, m_hFile(INVALID_HANDLE_VALUE)
{
	::InitializeCriticalSection(&m_csLock);
	memset(m_szFilePath, 0, sizeof(m_szFilePath));
}

CLog::~CLog()
{
	::DeleteCriticalSection(&m_csLock);
}

BOOL CLog::SetLogFilePath(LPCTSTR lpszFilePath)
{
	BOOL bRet = TRUE;

	_tcscpy_s(m_szFilePath, lpszFilePath);

	return bRet;
}

void CLog::EnableLog(BOOL bEnable)
{
	if(m_bEnable == bEnable)
		return;

	if(bEnable)
	{
		m_hFile = ::CreateFile(m_szFilePath, GENERIC_WRITE, FILE_SHARE_WRITE|FILE_SHARE_READ, NULL,
			CREATE_ALWAYS, 0, NULL);
		if(m_hFile != INVALID_HANDLE_VALUE)
		{
			// Write unicode file header
			WORD dwHeader = 0xFEFF;
			DWORD dwWriteLen = 0;
			::WriteFile(m_hFile, &dwHeader, sizeof(dwHeader), &dwWriteLen, NULL);
		}
		m_bEnable = TRUE;
	}
	else
	{
		m_bEnable = FALSE;
		if(m_hFile != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(m_hFile);
			m_hFile = INVALID_HANDLE_VALUE;
		}
	}
}

BOOL CLog::IsLogEnable(void)
{
	return m_bEnable;
}

void CLog::Log(const TCHAR* pcszFormat, ...)
{
	if(!m_bEnable)
		return;

	TCHAR szLog[1024];
	va_list vl;
	va_start(vl, pcszFormat);
	vswprintf_s(szLog, pcszFormat, vl);
	va_end(vl);

	TCHAR szText[2048];
	SYSTEMTIME st;
	::GetLocalTime(&st);
	_stprintf_s(szText, _T("FlyfoxLocalPlayer[%02d:%02d:%02d.%03d][%04X]: %s\r\n"),
		st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, ::GetCurrentThreadId(), szLog);

	if(m_hFile != INVALID_HANDLE_VALUE)
	{
		::EnterCriticalSection(&m_csLock);
		DWORD dwWriteLen = 0;
		BOOL bRet = ::WriteFile(m_hFile, szText, _tcslen(szText) * sizeof(TCHAR), &dwWriteLen, NULL);
		::LeaveCriticalSection(&m_csLock);
	}

#ifdef _DEBUG
	::OutputDebugString(szText);
#endif
}