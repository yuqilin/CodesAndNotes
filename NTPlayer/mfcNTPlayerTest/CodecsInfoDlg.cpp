// CodecsInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "mfcNTPlayerTest.h"
#include "CodecsInfoDlg.h"


// CCodecsInfoDlg dialog

IMPLEMENT_DYNAMIC(CCodecsInfoDlg, CDialog)

CCodecsInfoDlg::CCodecsInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCodecsInfoDlg::IDD, pParent)
{

}

CCodecsInfoDlg::~CCodecsInfoDlg()
{
}

void CCodecsInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL CCodecsInfoDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    INIT_EASYSIZE;
    return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

BEGIN_MESSAGE_MAP(CCodecsInfoDlg, CDialog)
    ON_WM_SIZE()
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CCodecsInfoDlg)
    EASYSIZE(IDC_EDIT_CODECSINFO, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0);
END_EASYSIZE_MAP

// CCodecsInfoDlg message handlers

void CCodecsInfoDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    // TODO: Add your message handler code here
    UPDATE_EASYSIZE;
}
