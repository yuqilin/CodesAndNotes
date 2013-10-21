
// mfcNTPlayerTestDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "mfcNTPlayerTest.h"
#include "CodecsInfoDlg.h"
#include "mfcNTPlayerTestDlg.h"
#include "NTPlayer.h"

#ifdef _DEBUG
#define new new
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CmfcNTPlayerTestDlg 对话框




CmfcNTPlayerTestDlg::CmfcNTPlayerTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CmfcNTPlayerTestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CmfcNTPlayerTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CmfcNTPlayerTestDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_BTN_OPENFILE, &CmfcNTPlayerTestDlg::OnBnClickedBtnOpenfile)
    ON_WM_DESTROY()
    ON_BN_CLICKED(IDC_BTN_CODECSINFO, &CmfcNTPlayerTestDlg::OnBnClickedBtnCodecsinfo)
    ON_WM_SIZE()
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CmfcNTPlayerTestDlg)
    EASYSIZE(IDC_BTN_CODECSINFO, ES_KEEPSIZE, ES_BORDER, ES_BORDER, ES_KEEPSIZE, 0)
END_EASYSIZE_MAP


// CmfcNTPlayerTestDlg 消息处理程序

BOOL CmfcNTPlayerTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
    m_CodecsInfoDlg = NULL;
    ntplayer_init();

    INIT_EASYSIZE;
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CmfcNTPlayerTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CmfcNTPlayerTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CmfcNTPlayerTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CmfcNTPlayerTestDlg::OnBnClickedBtnOpenfile()
{
    // TODO: Add your control notification handler code here
    ntplayer_open_url("D:\\Media\\一路狂奔BD1280高清国语版[www.spp54.com].rmvb");

}

void CmfcNTPlayerTestDlg::OnDestroy()
{
    CDialog::OnDestroy();

    // TODO: Add your message handler code here
    if (m_CodecsInfoDlg != NULL)
    {
        m_CodecsInfoDlg->DestroyWindow();
        delete m_CodecsInfoDlg;
    }
    ntplayer_uninit();
}

void CmfcNTPlayerTestDlg::OnBnClickedBtnCodecsinfo()
{
    // TODO: Add your control notification handler code here
    const char* info = NULL;
    ntplayer_get_codecs_info_string(&info);

    CStringA straInfo(info);
    CString strInfo(straInfo);

    if (m_CodecsInfoDlg == NULL)
    {
        m_CodecsInfoDlg = new CCodecsInfoDlg;
        m_CodecsInfoDlg->Create(IDD_DIALOG_CODECSINFO, this);
        m_CodecsInfoDlg->ShowWindow(SW_SHOW);
        m_CodecsInfoDlg->SetDlgItemText(IDC_EDIT_CODECSINFO, strInfo);
    }
    else
    {
        // 判断对话框可见状态，可见点击后隐藏，隐藏则点击可见
        BOOL bVisible = m_CodecsInfoDlg->IsWindowVisible();
        int nCmdShow = bVisible ? SW_HIDE : SW_SHOW;
        m_CodecsInfoDlg->ShowWindow(nCmdShow);
    }
}

void CmfcNTPlayerTestDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    // TODO: Add your message handler code here
    UPDATE_EASYSIZE;
}
