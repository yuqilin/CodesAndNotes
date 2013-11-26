
// mfcNTPlayerCoreTestDlg.h : header file
//

#pragma once

#include <string>

// CmfcNTPlayerCoreTestDlg dialog
class CmfcNTPlayerCoreTestDlg : public CDialog
{
    DECLARE_EASYSIZE()
// Construction
public:
	CmfcNTPlayerCoreTestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MFCNTPLAYERCORETEST_DIALOG };

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
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnDestroy();
    afx_msg void OnBnClickedBtnOpen();
    afx_msg void OnBnClickedBtnPlay();
    afx_msg void OnBnClickedBtnPause();
    afx_msg void OnBnClickedBtnStop();
    afx_msg void OnBnClickedBtnClose();

public:
    static void OnNtPlayerNotify(void* pUser, int msg, void* pParam);

protected:
    void OnOpenSucceeded();
    void OnOpenFailed();
public:
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnDropFiles(HDROP hDropInfo);

protected:
    CString m_strFileName;
    void OnOpenFileToPlay();
};

std::string	wcs2mbs(int nCodePage, const wchar_t* wcs);
