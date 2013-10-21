
#include "stdafx.h"
#include "PlayerSettings.h"

CPlayerSettings::CPlayerSettings()
{

}

void CPlayerSettings::LoadDefaultSettings()
{

    // codecs path
    TCHAR szPath[MAX_PATH];
    GetModulePath(m_hInstance, szPath);

    m_strCodecsPath = szPath;
    m_strCodecsPath.TrimRight(_T("\\"))
    m_strCodecsPath += _T("\\codecs");



}