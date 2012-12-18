
#include "stdafx.h"
#include "DSEngine.h"

CDSEngine::CDSEngine(CPlayerCore* pPlayerCore)
	: CBaseEngine(pPlayerCore)
	, m_EngineType(ET_DIRECTSHOW)
{
	if (m_pPlayerCore != NULL)
	{
		m_DSGraph.Create(m_pPlayerCore->GetVideoWindow(),
			m_pPlayerCore->GetNotifyWindow(),
			this);
	}	
}

CDSEngine::~CDSEngine()
{
	Close();
}

HRESULT CDSEngine::Open(CMediaInfo* pMediaInfo)
{
	CheckPointer(pMediaInfo, E_INVALIDARG);
	m_pMediaInfo = pMediaInfo;
	HRESULT hr = OpenMedia();
	return hr;
}

HRESULT CDSEngine::Close(void)
{
	HRESULT hr = S_OK;

	hr = CloseMedia();

	return hr;
}

HRESULT	CDSEngine::Play(void)
{
	HRESULT hr = S_OK;

	if (m_pGraph != NULL)
	{
		m_pGraph->Run();
	}

	return hr;
}

HRESULT	CDSEngine::Stop(void)
{
	HRESULT hr = S_OK;

	if (m_pGraph != NULL)
	{
		m_pGraph->Stop();
	}

	return hr;
}

HRESULT	CDSEngine::Pause(void)
{
	HRESULT hr = S_OK;

	if (m_pGraph != NULL)
	{
		m_pGraph->Pause();
	}

	return hr;
}

HRESULT	CDSEngine::GetPlayState()
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT	CDSEngine::GetPlayPos(LONG* pnPlayPos)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT	CDSEngine::SetPlayPos(LONG nPlayPos)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT	CDSEngine::GetVolume(LONG* pnVolume)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT	CDSEngine::SetVolume(LONG nVolume)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT CDSEngine::GetTotalDuration(LONG* pnDuration)
{
	HRESULT hr = S_OK;

	if (m_pGraph != NULL)
	{
		m_pGraph->GetTotalDuration();
	}

	return hr;
}

HRESULT	CDSEngine::GetVideoWidth(LONG* pnVideoWidth)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT	CDSEngine::GetVideoHeight(LONG* pnVideoHeight)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT	CDSEngine::OpenMedia()
{
	HRESULT hr = S_OK;

	CPlayerSettings& s = CPlayerCore::GetPlayerSettings();
	
	hr = this->RenderFile(s.m_bRenderOpenChain);

	return hr;
}

void CDSEngine::CloseMedia()
{
	
}

HRESULT	CDSEngine::RenderFile(BOOL bOpenChain)
{
	HRESULT hr = S_OK;

	const TCHAR * pcszExtension = m_pMediaInfo->GetExtension();
	if(pcszExtension == NULL || _tcslen(pcszExtension)<2)
	{
		return E_FAIL;
	}

	CDSFilters& filters = CPlayerCore::GetFilters();
	filters.OrderInfoByExtension(pcszExtension);

	// Call before render
	OnBeforeRender();

	// Render media
	HRESULT hr = E_FAIL;
	if(bOpenChain)
	{
		LPCWSTR pwszUrl = m_pMediaInfo->GetUrl();
		if (m_pJFilterGraph != NULL)
		{
			hr = m_pJFilterGraph->RenderFile(pwszUrl, NULL);
		}
	}
	else
	{
		hr = Kernel_Render();
	}

	// Call after render
	OnAfterRender(hr);
	if(FAILED(hr))
		return hr;

	// Show used filters
	ShowUsedFilters();

	return hr;
}

void CDSEngine::Kernel_OrderInfoByExtension(void)
{
	const TCHAR * pcszExtension = m_pMediaInfo->GetExtension();
	if(pcszExtension == NULL || _tcslen(pcszExtension)<2)
		return;

	g_utility.Log(_T("Kernel_OrderInfoByExtension: %s"), pcszExtension);

	CDSFilters& filters = CPlayerCore::GetFilters();
	filters.OrderInfoByExtension(pcszExtension);

}
