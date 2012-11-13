
#pragma once

class CSimpleThread
{
	CSimpleThread();
	virtual ~CSimpleThread();

	int StartThread();
	int StopThread();
	int Pause();
	int Play();
	BOOL bIsRunning();

protected:
	virtual void OnEnterThread();
	virtual void OnExitThread();
	virtual void OnPause();

	//Must be overloaded
	virtual int OnProcess() = 0;

private:

	static UINT __stdcall RunProcess(LPVOID lParam);
	UINT Run();

protected:

	//detail analysis thread control
	HANDLE m_hRunThread;
	UINT m_uThreadID;
	BOOL m_bExitRunThread;
	HANDLE m_hRunNotifyEvent;
	HANDLE m_hStopReplyEvent;
	HANDLE m_hRunReplyEvent;

	BOOL m_bPause;
	CCritSec m_csLock;
};