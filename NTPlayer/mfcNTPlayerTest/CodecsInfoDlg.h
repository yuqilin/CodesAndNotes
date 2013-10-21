#pragma once


// CCodecsInfoDlg dialog

class CCodecsInfoDlg : public CDialog
{
	DECLARE_DYNAMIC(CCodecsInfoDlg)
    DECLARE_EASYSIZE

public:
	CCodecsInfoDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCodecsInfoDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_CODECSINFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:
    virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnSize(UINT nType, int cx, int cy);
};
