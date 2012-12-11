
#include "stdafx.h"
#include "ZPlayerCore.h"

CDShowFilters* CZPlayerCore::m_spFilters = NULL;

CDShowFilters* CZPlayerCore::GetFilters()
{
	return m_spFilters;
}

// VideoWindow
HRESULT	CZPlayerCore::SetVideoWindow(HWND hVideoWindow)
{
	HRESULT hr = S_OK;
	m_hVideoWindow = hVideoWindow;
	return hr;
}

HRESULT	CZPlayerCore::SetFullScreen(BOOL bFullScreen)
{
	HRESULT hr = S_OK;
	m_bFullScreen = bFullScreen;
	return hr;
}

HWND CZPlayerCore::GetVideoWindow()
{
	return m_hVideoWindow;
}

BOOL CZPlayerCore::IsFullScreen()
{
	return m_bFullScreen;
}

HWND  CZPlayerCore::GetNotifyWindow()
{
	return m_hNotifyWindow;
}