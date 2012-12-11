
#include "stdafx.h"
#include "DShowEngine.h"

CDShowEngine::CDShowEngine()
{
	m_pPlayerCore = NULL;
	m_pMediaInfo = NULL;
	m_pGraph = NULL;
}

CDShowEngine::~CDShowEngine()
{
	UnInitialize();
}

HRESULT	CDShowEngine::Initialize(CZPlayerCore* pPlayerCore)
{
	CheckPointer(pPlayerCore, E_INVALIDARG);

	m_pPlayerCore = pPlayerCore;

	HRESULT hr = S_OK;
	m_pGraph = new CDShowGraph;
	if (m_pGraph == NULL)
		return E_OUTOFMEMORY;

	hr = m_pGraph->Initialize(
		m_pPlayerCore->GetVideoWindow(),
		m_pPlayerCore->GetNotifyWindow(),
		this);

	return hr;
}

void CDShowEngine::UnInitialize()
{
	m_pPlayerCore = NULL;
	m_pMediaInfo = NULL;

	if (m_pGraph != NULL)
	{
		m_pGraph->UnInitialize();
		delete m_pGraph;
		m_pGraph = NULL;
	}
}

HRESULT CDShowEngine::Open(CMediaInfo* pMediaInfo)
{
	CheckPointer(pMediaInfo, E_INVALIDARG);
	m_pMediaInfo = pMediaInfo;
	HRESULT hr = OpenMedia();
	return hr;
}

HRESULT CDShowEngine::Close(void)
{
	HRESULT hr = S_OK;

	hr = CloseMedia();

	return hr;
}

HRESULT	CDShowEngine::Play(void)
{
	HRESULT hr = S_OK;

	if (m_pGraph != NULL)
	{
		m_pGraph->Run();
	}

	return hr;
}

HRESULT	CDShowEngine::Stop(void)
{
	HRESULT hr = S_OK;

	if (m_pGraph != NULL)
	{
		m_pGraph->Stop();
	}

	return hr;
}

HRESULT	CDShowEngine::Pause(void)
{
	HRESULT hr = S_OK;

	if (m_pGraph != NULL)
	{
		m_pGraph->Pause();
	}

	return hr;
}

HRESULT	CDShowEngine::GetPlayState()
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT	CDShowEngine::GetPlayPos(LONG* pnPlayPos)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT	CDShowEngine::SetPlayPos(LONG nPlayPos)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT	CDShowEngine::GetVolume(LONG* pnVolume)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT	CDShowEngine::SetVolume(LONG nVolume)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT CDShowEngine::GetTotalDuration(LONG* pnDuration)
{
	HRESULT hr = S_OK;

	if (m_pGraph != NULL)
	{
		m_pGraph->GetTotalDuration();
	}

	return hr;
}

HRESULT	CDShowEngine::GetVideoWidth(LONG* pnVideoWidth)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT	CDShowEngine::GetVideoHeight(LONG* pnVideoHeight)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT	CDShowEngine::OpenMedia()
{
	HRESULT hr = S_OK;

	if (m_pGraph != NULL)
	{
		hr = m_pGraph->Render(m_bRenderOpenChain);
	}

	return hr;
}

void CDShowEngine::CloseMedia()
{
	
}