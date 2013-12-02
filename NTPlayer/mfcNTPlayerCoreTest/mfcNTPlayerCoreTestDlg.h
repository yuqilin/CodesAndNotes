
// mfcNTPlayerCoreTestDlg.h : header file
//

#pragma once

#include <string>
#include "PlayerSeekBar.h"

// CmfcNTPlayerCoreTestDlg dialog
class CmfcNTPlayerCoreTestDlg : public CDialog
{
    //DECLARE_EASYSIZE()
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

    CMenu m_popup;

    CPlayerSeekBar m_wndSeekBar;

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

    afx_msg void OnFileOpenFile();
    afx_msg void OnFileOpenUrl();
    afx_msg void OnFileClose();
    afx_msg void OnPlayPlay();
    afx_msg void OnPlayPause();
    afx_msg void OnPlayStop();


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
    long m_msDuration;
    void OnOpenFileToPlay();
public:
    afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

std::string	wcs2mbs(int nCodePage, const wchar_t* wcs);
