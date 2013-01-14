
#include "stdafx.h"
#include "ZPlayerCore.h"

CPlayerCore::CPlayerCore()
{
	m_MediaLoadState = MLS_CLOSED;
	m_PlayState = PS_INVALID;
}

CPlayerCore::~CPlayerCore()
{

}

HRESULT CPlayerCore::Create()
{
	HRESULT hr = S_OK;

	hr = CreateNotifyWindow();
	if (FAILED(hr))
	{
		return hr;
	}

	// SetCodecsPath, LoadFilters
	CString strCodecsPath;
	strCodecsPath.Format(_T("%s\\Codecs"), g_utility.GetModulePath());
	hr = m_Filters.SetCodecsPath(strCodecsPath);


	// Log
	TCHAR szTempPath[MAX_PATH];
	TCHAR szLogFilePath[MAX_PATH];
	if (GetTempPath(_countof(szTempPath), szTempPath) > 0)
	{
		if (m_Settings.m_bFixedLogFile)
		{
			_stprintf_s(szLogFilePath, _T("%s\\flyfoxLocalPlayer\\flyfoxLocalPlayer.txt"), szTempPath);
		}
		else
		{
			DWORD dwProcessId = ::GetCurrentProcessId();
			_stprintf_s(szLogFilePath, _T("%s\\flyfoxLocalPlayer\\flyfoxLocalPlayer_%u.txt"), szTempPath, dwProcessId);
		}
		CLog::SetLogFilePath(szLogFilePath);
	}

	

	return hr;
}

void CPlayerCore::Destroy()
{
	Close();

	m_Filters.UnLoadCodecsInfo();

	DestroyNotifyWindow();
}

const CDSFilters& CPlayerCore::GetFilters()
{
	return m_Filters;
}

// VideoWindow
HRESULT	CPlayerCore::SetVideoWindow(HWND hVideoWindow)
{
	HRESULT hr = S_OK;
	m_hVideoWindow = hVideoWindow;
	return hr;
}

HRESULT	CPlayerCore::SetFullScreen(BOOL bFullScreen)
{
	HRESULT hr = S_OK;
	m_bFullScreen = bFullScreen;
	return hr;
}

HWND CPlayerCore::GetVideoWindow()
{
	return m_hVideoWindow;
}

BOOL CPlayerCore::IsFullScreen()
{
	return m_bFullScreen;
}

HWND CPlayerCore::GetNotifyWindow()
{
	return m_hNotifyWindow;
}


//
HRESULT	CPlayerCore::Open(LPCTSTR pcszUrl)
{
	HRESULT hr = S_OK;

	DWORD dwStart = ::timeGetTime();

	// Create Engine
	m_pEngine = new CDSEngine();
	if (m_pEngine == NULL)
	{
		return E_OUTOFMEMORY;
	}

	m_dwPlayResult = S_FALSE;
	hr = m_MediaInfo.Create(m_nMediaId, pcszUrl);
	if(FAILED(hr))
	{
		g_utility.Log(_T("Operation_Open, m_MediaInfo.Create failed, hr=%08x"), hr);
		this->SetPlayError(hr);
		return hr;
	}

	hr = m_pEngine->Open(&m_MediaInfo);

	DWORD dwTime = ::timeGetTime() - dwStart;
	g_utility.Log(_T("Open media cost %u ms"), dwTime);

	if(SUCCEEDED(hr))
	{
		OnOpenSuccess();
		SetMediaLoadState(MLS_LOADED);
	}
	else
	{
		OnOpenFailed();
	}

	return hr;
}

void CPlayerCore::Close()
{
	if (m_MediaLoadState == MLS_CLOSED)
		return;

	DestroyNotifyWindow();

	// Do close
	if(m_pEngine != NULL)
	{
		g_utility.Log(_T("m_pEngine->Close begin"));
		m_pEngine->Close();
		g_utility.Log(_T("m_pEngine->Close end"));
		SAFE_DELETE(m_pEngine);
	}

	m_MediaInfo.Destroy();

	m_MediaLoadState = MLS_CLOSED;
}

HRESULT CPlayerCore::Play(void)
{
	HRESULT hr = S_OK;
	if(m_PlayState == PS_PLAY)
		return hr;

	if(m_pEngine != NULL)
	{
		hr = m_pEngine->Play();
	}
	return hr;
}

HRESULT CPlayerCore::Stop(void)
{
	HRESULT hr = S_OK;
	if(m_PlayState == PS_STOP)
		return hr;

	if(m_pEngine != NULL)
	{
		hr = m_pEngine->Stop();
	}

	return hr;
}

HRESULT CPlayerCore::Pause(void)
{
	HRESULT hr = S_OK;
	if(m_PlayState == PS_PAUSE)
		return hr;

	if(m_pEngine != NULL)
	{
		hr = m_pEngine->Pause();
	}
	return hr;
}

void CPlayerCore::SetMediaLoadState(MEDIA_LOAD_STATE iState)
{
	m_MediaLoadState = iState;
}

void CPlayerCore::SetPlayState(PLAY_STATE iState)
{
	m_PlayState = iState;
}

void CPlayerCore::SetPlayError(DWORD dwErrorCode)
{
	// Make sure is valid error
	if(dwErrorCode == S_OK || dwErrorCode == S_FALSE)
		return;

	// Save error code
	m_dwPlayResult = dwErrorCode;

	this->Close();
}

void CPlayerCore::OnOpenSuccess()
{
	if (m_pEngine == NULL)
		return;
	BOOL bAudioSwitcherEnabled = FALSE;
	if (m_pEngine->GetEngineType() == ET_DIRECTSHOW)
	{
		CDSEngine* pEngine = dynamic_cast<CDSEngine*>(m_pEngine);
		if (pEngine != NULL)
		{
			bAudioSwitcherEnabled = m_pEngine->FindFilterByClsid(sCLSID_AudioSwitcher);
		}
	}

	g_utility.Log(_T("Current engine loud capability = %d"), bAudioSwitcherEnabled);

	if(!bAudioSwitcherEnabled)
	{
		if(m_Settings.m_nVolume > 100)
		{
			g_utility.Log(_T("Limit volume: %d => 100"), m_Settings.m_nVolume);
			m_Settings.m_nVolume = 100;
		}
	}	

	m_pEngine->SetVolume(m_Settings.m_nVolume);

	if(m_Settings.m_bAutoPlay)
	{
		this->Play();
	}
}

void CPlayerCore::OnOpenFailed()
{
	
}


