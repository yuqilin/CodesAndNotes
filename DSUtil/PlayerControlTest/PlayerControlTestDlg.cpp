
// PlayerControlTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include <process.h>
#include "PlayerControlTest.h"
#include "PlayerControlTestDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CPlayerControlTestDlg dialog




CPlayerControlTestDlg::CPlayerControlTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPlayerControlTestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_uPlayThreadID = 0;
	m_hPlayThread = NULL;
	m_bPlayThreadRunning = FALSE;
	m_PlayOperation = kOperationInvalid;
	m_hOperationNotifyEvent = NULL;
}

void CPlayerControlTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPlayerControlTestDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_OPEN, &CPlayerControlTestDlg::OnBnClickedBtnOpen)
	ON_BN_CLICKED(IDC_BTN_PLAY, &CPlayerControlTestDlg::OnBnClickedBtnPlay)
	ON_BN_CLICKED(IDC_BTN_PAUSE, &CPlayerControlTestDlg::OnBnClickedBtnPause)
	ON_BN_CLICKED(IDC_BTN_STOP, &CPlayerControlTestDlg::OnBnClickedBtnStop)

	ON_MESSAGE(WM_PLAY_OPEN, &CPlayerControlTestDlg::OnPlayOpen)
	ON_MESSAGE(WM_PLAY_PLAY, &CPlayerControlTestDlg::OnPlayPlay)
	ON_MESSAGE(WM_PLAY_PAUSE, &CPlayerControlTestDlg::OnPlayPause)
	ON_MESSAGE(WM_PLAY_STOP, &CPlayerControlTestDlg::OnPlayStop)
	ON_MESSAGE(WM_PLAY_OPEN_SUCCEEDED, &CPlayerControlTestDlg::OnOpenSucceeded)
	ON_MESSAGE(WM_PLAY_OPEN_FAILED, &CPlayerControlTestDlg::OnOpenFailed)
END_MESSAGE_MAP()


// CPlayerControlTestDlg message handlers

BOOL CPlayerControlTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	CreatePlayThread();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPlayerControlTestDlg::OnDestroy()
{
	ExitPlayThread();

	__super::OnDestroy();
}

void CPlayerControlTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPlayerControlTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPlayerControlTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CPlayerControlTestDlg::OnBnClickedBtnOpen()
{
	// TODO: Add your control notification handler code here
	if (PostOperation(kOperationOpen))
	{
		//GetDlgItem(IDC_STATIC_STATE)->SetWindowText(_T("Opening..."));
		SetEvent(m_hOperationNotifyEvent);
	}
}

void CPlayerControlTestDlg::OnBnClickedBtnPlay()
{
	// TODO: Add your control notification handler code here
	if (PostOperation(kOperationPlay))
	{
		//GetDlgItem(IDC_STATIC_STATE)->SetWindowText(_T("Playing..."));
		SetEvent(m_hOperationNotifyEvent);
	}
}

void CPlayerControlTestDlg::OnBnClickedBtnPause()
{
	// TODO: Add your control notification handler code here
	if (PostOperation(kOperationPause))
	{
		//GetDlgItem(IDC_STATIC_STATE)->SetWindowText(_T("Paused..."));
		SetEvent(m_hOperationNotifyEvent);
	}
}

void CPlayerControlTestDlg::OnBnClickedBtnStop()
{
	// TODO: Add your control notification handler code here
	if (PostOperation(kOperationStop))
	{
		//GetDlgItem(IDC_STATIC_STATE)->SetWindowText(_T("Stopped..."));
		SetEvent(m_hOperationNotifyEvent);
	}	
}


BOOL CPlayerControlTestDlg::CreatePlayThread()
{
	InitializeCriticalSection(&m_csOperation);
	m_hOperationNotifyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	BOOL bRet = FALSE;
	unsigned uThreadId;
	m_hPlayThread = (HANDLE)_beginthreadex(NULL, 0, PlayThreadStatic, this, 0, &uThreadId);
	if (m_hPlayThread)
	{
		m_uPlayThreadID = uThreadId;
		m_bPlayThreadRunning = TRUE;
		bRet = TRUE;
	}
	return bRet;
}

unsigned int __stdcall CPlayerControlTestDlg::PlayThreadStatic(void* pParam)
{
	CPlayerControlTestDlg* pThis = (CPlayerControlTestDlg*)pParam;

	if (pThis)
		pThis->PlayThreadProc();

	return 0;
}

void CPlayerControlTestDlg::PlayThreadProc()
{
	while (m_bPlayThreadRunning)
	{
		WaitForSingleObject(m_hOperationNotifyEvent, INFINITE);

		EPlayOperation operation;
		if (!PeekOperation(&operation))
		{
			ResetEvent(m_hOperationNotifyEvent);
			continue;
		}

		switch (m_PlayOperation)
		{
		case kOperationOpen:
			OperationOpen();
			break;
		case kOperationClose:
			OperationClose();
			break;
		case kOperationPlay:
			OperationPlay();
			break;
		case kOperationStop:
			OperationStop();
			break;
		case kOperationPause:
			OperationPause();
			break;
		default:
			ResetEvent(m_hOperationNotifyEvent);
		}
	}
}

void CPlayerControlTestDlg::ExitPlayThread()
{
	DeleteCriticalSection(&m_csOperation);

	if (m_hPlayThread != NULL)
	{
		m_PlayOperation = kOperationInvalid;
		m_bPlayThreadRunning = FALSE;
		SetEvent(m_hOperationNotifyEvent);
		if( WAIT_OBJECT_0 != WaitForSingleObject(m_hPlayThread, 2000))
		{
			TerminateThread(m_hPlayThread, 0);
		}
		CloseHandle(m_hPlayThread);
		m_hPlayThread = NULL;
	}

	if (m_hOperationNotifyEvent != NULL)
	{
		CloseHandle(m_hOperationNotifyEvent);
		m_hOperationNotifyEvent = NULL;
	}
}

BOOL CPlayerControlTestDlg::PostOperation(EPlayOperation operation)
{
	if (operation > kOperationInvalid && operation < kOperationLast)
	{
		::EnterCriticalSection(&m_csOperation);
		m_PlayOperation = operation;
		::LeaveCriticalSection(&m_csOperation);
		return TRUE;
	}
	return FALSE;
}

BOOL CPlayerControlTestDlg::PeekOperation(EPlayOperation* operation)
{
	if (operation && 
		m_PlayOperation > kOperationInvalid &&
		m_PlayOperation < kOperationLast)
	{
		::EnterCriticalSection(&m_csOperation);
		*operation = m_PlayOperation;
		::LeaveCriticalSection(&m_csOperation);
		return TRUE;
	}
	return FALSE;
}

void CPlayerControlTestDlg::OperationOpen()
{
	ShowDebug(_T("CPlayerControlTestDlg::OperationOpen"));

	PostMessage(WM_PLAY_OPEN);

	// do something
	Sleep(4000);

	PostMessage(WM_PLAY_OPEN_SUCCEEDED);

	ResetEvent(m_hOperationNotifyEvent);
}

void CPlayerControlTestDlg::OperationClose()
{
	ShowDebug(_T("CPlayerControlTestDlg::OperationClose"));

	// do something
	
	ResetEvent(m_hOperationNotifyEvent);
}

void CPlayerControlTestDlg::OperationPlay()
{
	ShowDebug(_T("CPlayerControlTestDlg::OperationPlay"));

	// do something
	PostMessage(WM_PLAY_PLAY);

	ResetEvent(m_hOperationNotifyEvent);
}

void CPlayerControlTestDlg::OperationPause()
{
	ShowDebug(_T("CPlayerControlTestDlg::OperationPause"));

	PostMessage(WM_PLAY_PAUSE);

	ResetEvent(m_hOperationNotifyEvent);
}

void CPlayerControlTestDlg::OperationStop()
{
	ShowDebug(_T("CPlayerControlTestDlg::OperationStop"));

	PostMessage(WM_PLAY_STOP);

	ResetEvent(m_hOperationNotifyEvent);
}

LRESULT CPlayerControlTestDlg::OnPlayOpen(WPARAM,LPARAM)
{
	GetDlgItem(IDC_STATIC_STATE)->SetWindowText(_T("Openning..."));
	return 0;
}

LRESULT CPlayerControlTestDlg::OnPlayPlay(WPARAM,LPARAM)
{
	GetDlgItem(IDC_STATIC_STATE)->SetWindowText(_T("Playing..."));
	return 0;
}

LRESULT CPlayerControlTestDlg::OnPlayPause(WPARAM,LPARAM)
{
	GetDlgItem(IDC_STATIC_STATE)->SetWindowText(_T("Paused..."));
	return 0;
}

LRESULT CPlayerControlTestDlg::OnPlayStop(WPARAM,LPARAM)
{
	GetDlgItem(IDC_STATIC_STATE)->SetWindowText(_T("Stopped..."));
	return 0;
}

LRESULT CPlayerControlTestDlg::OnOpenSucceeded(WPARAM,LPARAM)
{
	GetDlgItem(IDC_STATIC_STATE)->SetWindowText(_T("Open Succeeded..."));
	return 0;
}

LRESULT CPlayerControlTestDlg::OnOpenFailed(WPARAM,LPARAM)
{
	GetDlgItem(IDC_STATIC_STATE)->SetWindowText(_T("Open Failed..."));
	return 0;
}