
#include "stdafx.h"
#include <process.h>
#include "simple_thread.h"

CSimpleThread::CSimpleThread()

{
	m_hRunThread = NULL;
	m_uThreadID = 0;

	m_hRunNotifyEvent = NULL;
	m_hStopReplyEvent = NULL;
	m_hRunReplyEvent = NULL;

	m_bExitRunThread = FALSE;
	m_bPause = TRUE;
}

CSimpleThread::~CSimpleThread()
{
	if(m_hRunThread)
		StopThread();
}

int CSimpleThread::StartThread()
{
	//Create analysis thread

	m_hRunNotifyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hStopReplyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hRunReplyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	if(m_hRunNotifyEvent == NULL || m_hStopReplyEvent == NULL || m_hRunReplyEvent == NULL)
	{
		ASSERT(FALSE);
		return -1;
	}

	//create and initialize the analysis thread
	m_bExitRunThread = FALSE;
	m_bPause = TRUE;
	m_hRunThread = (HANDLE)_beginthreadex(NULL, 0, RunProcess, this, 0, &m_uThreadID);
	if(m_hRunThread == NULL)
	{
		CloseHandle(m_hRunNotifyEvent);
		m_hRunNotifyEvent = NULL;
		CloseHandle(m_hStopReplyEvent);
		m_hStopReplyEvent = NULL;
		CloseHandle(m_hRunReplyEvent);
		m_hRunReplyEvent = NULL;

		ASSERT(FALSE);
		return -2;
	}	

	return 1;
}

int CSimpleThread::StopThread()
{
	if(m_hRunThread)
	{
		m_bExitRunThread = TRUE;
		SetEvent(m_hRunNotifyEvent);
		if(WAIT_OBJECT_0 != WaitForSingleObject(m_hRunThread, 20000))
		{
			ASSERT(FALSE);
			TerminateThread(m_hRunThread, 0);
		}

		CloseHandle( m_hRunThread);
		m_hRunThread = NULL;
	}

	if(m_hRunNotifyEvent != NULL)
	{
		ResetEvent(m_hRunNotifyEvent);  //need it?
		CloseHandle(m_hRunNotifyEvent);
		m_hRunNotifyEvent = NULL;
	}

	if(m_hRunReplyEvent != NULL)
	{
		CloseHandle(m_hRunReplyEvent);
		m_hRunReplyEvent = NULL;
	}

	if(m_hStopReplyEvent != NULL)
	{
		CloseHandle(m_hStopReplyEvent);
		m_hStopReplyEvent = NULL;
	}

	return 1;
}

UINT CSimpleThread::RunProcess(LPVOID lParam)
{
	UINT uRet;
	CSimpleThread* pThis = (CSimpleThread*)lParam;

	// for com
	CoInitialize(NULL);

	uRet = pThis->Run();

	CoUninitialize();

	return uRet;
}

UINT CSimpleThread::Run()
{
	DWORD dwWaitRet;

	OnEnterThread();

	while(TRUE)
	{
		dwWaitRet = WaitForSingleObject(m_hRunNotifyEvent, INFINITE);
		if(m_bExitRunThread) //Exit run thread
			break;
		if(m_bPause)  //While be pausing state, only video will be displayed
		{
			ResetEvent(m_hRunNotifyEvent);
			//LOG_MODULE_DEBUG(keLogPkgTsWMHelper, keLogPkgTsWMHelperFuncInformation, \
			//	"GphoneSimpleThdHelper::Run: Paused");
			OnPause();
			//LOG_MODULE_DEBUG(keLogPkgTsWMHelper, keLogPkgTsWMHelperFuncInformation, \
			//	"GphoneSimpleThdHelper::Run: Pause end");
			SetEvent(m_hStopReplyEvent);
			continue;
		}
		//Todo Run
		SetEvent(m_hRunReplyEvent);
		OnProcess();
	}

	OnExitThread();

	return 1;
}

int CSimpleThread::Pause()
{
	CAutoLock cLock(&m_csLock);

	if(m_bPause)
		return 1;

	m_bPause = TRUE;
	WaitForSingleObject(m_hStopReplyEvent, 0);
	SetEvent(m_hRunNotifyEvent);
	DWORD dwWaitRet = WaitForSingleObject(m_hStopReplyEvent, 5000);
	if(dwWaitRet != WAIT_OBJECT_0)
	{
		ASSERT( FALSE);
		return -1;
	}

	return 1;
}

int CSimpleThread::Play()
{
	CAutoLock cLock(&m_csLock);

	if(!m_bPause)
		return 1;

	m_bPause = FALSE;
	WaitForSingleObject(m_hRunReplyEvent, 0);
	SetEvent(m_hRunNotifyEvent);
	DWORD dwWaitRet = WaitForSingleObject(m_hRunReplyEvent, 20000);
	if(dwWaitRet != WAIT_OBJECT_0)
	{
		ASSERT( FALSE);
		return -1;
	}

	return 1;
}

BOOL CSimpleThread::bIsRunning()
{
	CAutoLock cLock(&m_csLock);
	return (!m_bPause);
}

void CSimpleThread::OnEnterThread()
{
	return;
}

void CSimpleThread::OnExitThread()
{
	return;
}

void CSimpleThread::OnPause()
{
	return;
}