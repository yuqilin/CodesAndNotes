
#ifndef _PLAYERCORE_DSUTIL_LOG_H_
#define _PLAYERCORE_DSUTIL_LOG_H_


class CLog
{
public:
	CLog();
	~CLog();

	static BOOL SetLogFilePath(LPCTSTR lpszFilePath);
	static void EnableLog(BOOL bEnable);
	static BOOL IsLogEnable(void);
	static void Log(const TCHAR* pcszFormat, ...);

protected:
	CRITICAL_SECTION m_csLock;
	BOOL m_bEnable;
	BOOL m_bFixLogFile;
	TCHAR m_szFilePath[MAX_PATH];
	HANDLE m_hFile;
};


#endif