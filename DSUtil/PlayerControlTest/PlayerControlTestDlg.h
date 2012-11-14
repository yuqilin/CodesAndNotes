
// PlayerControlTestDlg.h : header file
//

#pragma once

#define WM_PLAY_OPEN		WM_USER + 0x10
#define WM_PLAY_PLAY		WM_USER + 0x11
#define WM_PLAY_STOP		WM_USER + 0x12
#define WM_PLAY_PAUSE		WM_USER + 0x13
#define WM_PLAY_OPEN_SUCCEEDED		WM_USER + 0x14
#define WM_PLAY_OPEN_FAILED		WM_USER + 0x15

// CPlayerControlTestDlg dialog
class CPlayerControlTestDlg : public CDialog
{
// Construction
public:
	CPlayerControlTestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_PLAYERCONTROLTEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;
	
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnOpen();
	afx_msg void OnBnClickedBtnPlay();
	afx_msg void OnBnClickedBtnPause();
	afx_msg void OnBnClickedBtnStop();

protected:
	UINT m_uPlayThreadID;
	HANDLE m_hPlayThread;
	BOOL m_bPlayThreadRunning;

	typedef enum tagEPlayOperation {
		kOperationInvalid = 0,
		kOperationOpen,
		kOperationClose,
		kOperationPlay,
		kOperationPause,
		kOperationStop,
		kOperationLast,
	} EPlayOperation;
	EPlayOperation	m_PlayOperation;
	CRITICAL_SECTION m_csOperation;
	HANDLE m_hOperationNotifyEvent;

	BOOL CreatePlayThread();
	static unsigned int __stdcall PlayThreadStatic(void* pParam);
	void PlayThreadProc();
	void ExitPlayThread();


	BOOL PostOperation(EPlayOperation operation);
	BOOL PeekOperation(EPlayOperation* operation);

	void OperationOpen();
	void OperationClose();
	void OperationPlay();
	void OperationPause();
	void OperationStop();

protected:
	afx_msg LRESULT OnPlayOpen(WPARAM,LPARAM);
	afx_msg LRESULT OnPlayPlay(WPARAM,LPARAM);
	afx_msg LRESULT OnPlayPause(WPARAM,LPARAM);
	afx_msg LRESULT OnPlayStop(WPARAM,LPARAM);
	afx_msg LRESULT OnOpenSucceeded(WPARAM,LPARAM);
	afx_msg LRESULT OnOpenFailed(WPARAM,LPARAM);
public:
	afx_msg void OnClose();
};

inline void ShowDebug(const TCHAR * pcszFormat, ... )
{
	TCHAR szBuffer[1024];

	va_list vl;
	va_start(vl, pcszFormat);
	_vstprintf_s(szBuffer, pcszFormat, vl);
	va_end(vl);

	::OutputDebugString(szBuffer);
}
