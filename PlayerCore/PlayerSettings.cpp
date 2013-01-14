
#include "stdafx.h"
#include "PlayerSettings.h"

#define PROFILE_APP_NAME	_T("Config")

void CPlayerSettings::GetIniFile(TCHAR * pszIniFile)
{
	_stprintf_s(pszIniFile, MAX_PATH, _T("%s\\%s"), m_szPlayerPath, _T("FlyfoxLocalPlayer.ini"));
	if(!::PathFileExists(pszIniFile))
		_stprintf_s(pszIniFile, MAX_PATH, _T("%s\\%s"), m_szPlayerPath, _T("FlyfoxLocalPlayer.txt"));
}

int CPlayerSettings::GetProfileInt(const TCHAR * pcszKey, int nDefault)
{
	TCHAR szIniFile[MAX_PATH];
	this->GetIniFile(szIniFile);
	int nResult = ::GetPrivateProfileInt(PROFILE_APP_NAME, pcszKey, nDefault, szIniFile);
	return nResult;
}

BOOL CPlayerSettings::GetProfileStr(const TCHAR * pcszKey,
							 const TCHAR * pcszDefault,
							 TCHAR * pszValue,
							 int nValueSize)
{
	TCHAR szIniFile[MAX_PATH];
	this->GetIniFile(szIniFile);
	::GetPrivateProfileString(PROFILE_APP_NAME, pcszKey, pcszDefault, pszValue, nValueSize, szIniFile);
	return TRUE;
}

BOOL CPlayerSettings::LoadSettings()
{
	BOOL bRet = TRUE;

	if(!::GetModulePath(g_hInstance, m_szPlayerPath))
		return FALSE;


	if (GetProfileInt(_T("Log"), 0))
	{
		m_bLogEnable = TRUE;
	}

	if (GetProfileInt(_T("FixLogFile"), 0))
	{
		m_bFixedLogFile = TRUE;
	}




	return bRet;
}

