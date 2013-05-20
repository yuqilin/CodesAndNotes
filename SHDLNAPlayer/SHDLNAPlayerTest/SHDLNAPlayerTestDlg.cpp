// SHDLNAPlayerTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SHDLNAPlayerTest.h"
#include "SHDLNAPlayerTestDlg.h"

#include <string>


/*----------------------------------------------------------------------
|   mbs2wcs
+---------------------------------------------------------------------*/
std::wstring mbs2wcs(int nCodePage, const char* mbs)
{
	int cchWideChar = MultiByteToWideChar(nCodePage,
		0, 
		mbs, 
		-1, 
		NULL, 
		0);  
	wchar_t *pText = new wchar_t[cchWideChar+1];
	memset(pText,0,sizeof(pText));
	MultiByteToWideChar(nCodePage, 
		0, 
		mbs,
		-1, 
		pText, 
		cchWideChar);

	std::wstring wstrText(pText);
	delete[] pText;
	return wstrText;

}

/*----------------------------------------------------------------------
|   wcs2mbs
+---------------------------------------------------------------------*/
std::string	wcs2mbs(int nCodePage, const wchar_t* wcs)
{
	char* pText;
	int cbMultiByte = WideCharToMultiByte(nCodePage,
		0,
		wcs,
		-1,
		NULL,
		0,
		NULL,
		NULL);
	pText=new char[cbMultiByte+1];
	memset(pText,0,sizeof(pText));
	WideCharToMultiByte(nCodePage,
		0,
		wcs,
		-1,
		pText,
		cbMultiByte,
		NULL,
		NULL);

	std::string strText(pText);
	delete[] pText;
	return strText;
}

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIMERID_GET_MEDIA_DURATION		1
#define TIMERID_GET_CUR_PLAY_POS		2

#define ELAPSE_GET_MEDIA_DURATION		1000 // ms
#define ELAPSE_GET_CUR_PLAY_POS			1500 // ms


CString MsToString(int time_ms)
{
	CString strTime;
	int hours = time_ms / 1000 / 3600;
	int minutes = time_ms / 1000 / 60 % 60;
	int seconds = time_ms / 1000 % 60;
	strTime.Format(_T("%02d:%02d:%02d"), hours, minutes, seconds);
	return strTime;
}

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


// CSHDLNAPlayerTestDlg dialog

CSHDLNAPlayerTestDlg* CSHDLNAPlayerTestDlg::m_spThis = NULL;


CSHDLNAPlayerTestDlg::CSHDLNAPlayerTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSHDLNAPlayerTestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_spThis = this;
}

CSHDLNAPlayerTestDlg::~CSHDLNAPlayerTestDlg()
{
	m_spThis = NULL;
}

void CSHDLNAPlayerTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DEVICES, m_ctrlDeviceList);
	DDX_Control(pDX, IDC_LIST_ONLINE_MEDIA, m_ctrlOnlineMedia);
	DDX_Control(pDX, IDC_SLIDER_PLAY_PROGRESS, m_ctrlPlayProgress);
	DDX_Control(pDX, IDC_SLIDER_VOLUME, m_ctrlVolume);
	DDX_Control(pDX, IDC_CHECK_ENABLE_DLNA, m_btnEnableDLNA);
}

BEGIN_MESSAGE_MAP(CSHDLNAPlayerTestDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_OPEN_LOCAL_MEDIA, &CSHDLNAPlayerTestDlg::OnBnClickedBtnOpenLocalMedia)
	ON_BN_CLICKED(IDC_BTN_PLAY, &CSHDLNAPlayerTestDlg::OnBnClickedBtnPlay)
	ON_BN_CLICKED(IDC_BTN_PAUSE, &CSHDLNAPlayerTestDlg::OnBnClickedBtnPause)
	ON_BN_CLICKED(IDC_BTN_STOP, &CSHDLNAPlayerTestDlg::OnBnClickedBtnStop)
	ON_BN_CLICKED(IDC_BTN_OPEN_ONLINE_MEDIA, &CSHDLNAPlayerTestDlg::OnBnClickedBtnOpenOnlineMedia)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_CHECK_ENABLE_DLNA, &CSHDLNAPlayerTestDlg::OnBnClickedCheckEnableDlna)
	ON_WM_DESTROY()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CSHDLNAPlayerTestDlg message handlers

BOOL CSHDLNAPlayerTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
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
	memset(&m_DLNADeviceList, 0, sizeof(SH_DLNAPlayer_DeviceList));
	
	m_ctrlVolume.SetRange(VOLUME_MIN, VOLUME_MAX);
	m_ctrlVolume.SetPos(VOLUME_MAX - 70);

	m_mapOnlineVideos["功夫[原画版]"] = "http://hot.vrs.sohu.com/vrs_flash.action?vid=1078239&ver=31&ref=0001";
	m_mapOnlineVideos["功夫[超清版]"] = "http://hot.vrs.sohu.com/vrs_flash.action?vid=621849&ver=21&ref=0001";
	m_mapOnlineVideos["功夫[高清版]"] = "http://hot.vrs.sohu.com/vrs_flash.action?vid=621847&ver=1&ref=0001";
	m_mapOnlineVideos["功夫[普通版]"] = "http://hot.vrs.sohu.com/vrs_flash.action?vid=621848&ver=2&ref=0001";
	m_mapOnlineVideos["箭在弦上第一集[高清]"] = "http://hot.vrs.sohu.com/vrs_flash.action?vid=1023937&ver=1&ref=0001";
	m_mapOnlineVideos["枪花第1集[超清版]"] = "http://hot.vrs.sohu.com/vrs_flash.action?vid=1095711&ver=31&ref=0001";
	m_mapOnlineVideos["枪花第1集[高清版]"] = "http://hot.vrs.sohu.com/vrs_flash.action?vid=1095709&ver=1&ref=0001";
	m_mapOnlineVideos["新马永贞"] = "http://hot.vrs.sohu.com/vrs_flash.action?vid=1115814&ver=1&ref=0001";

	StringMapIterator it = m_mapOnlineVideos.begin();
	for (; it != m_mapOnlineVideos.end(); ++it)
	{
		m_ctrlOnlineMedia.AddString((CString)(it->first.c_str()));
	}
	
	m_PlayState = PLAY_STATE_NONE;

	UpdateData(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

BOOL CSHDLNAPlayerTestDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam==VK_ESCAPE)
		return TRUE;

	if (pMsg->message == WM_KEYDOWN && pMsg->wParam==VK_RETURN)
		return TRUE;
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CSHDLNAPlayerTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSHDLNAPlayerTestDlg::OnPaint()
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
HCURSOR CSHDLNAPlayerTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//
void CSHDLNAPlayerTestDlg::SHDLNAPlayerMessageNotifyUI(int msg, void* wParam, void* lParam)
{
	if (m_spThis == NULL)
		return;
	switch (msg)
	{
	case SH_DLNAPLAYER_UI_MESSAGE_OPEN_MEDIA_SUCCEEDED:
		m_spThis->OnDLNAPlayerOpenMediaSucceeded();
		break;
	case SH_DLNAPLAYER_UI_MESSAGE_OPEN_MEDIA_FAILED:
		m_spThis->OnDLNAPlayerOpenMediaFailed();
		break;
	case SH_DLNAPLAYER_UI_MESSAGE_SEEK_SUCCEEDED:
		m_spThis->OnDLNAPlayerSeekSucceeded();
		break;
	case SH_DLNAPLAYER_UI_MESSAGE_SEEK_FAILED:
		m_spThis->OnDLNAPlayerSeekFailed();
		break;
	case SH_DLNAPLAYER_UI_MESSAGE_GET_MEDIA_INFO_SUCCEEDED:

		break;
	case SH_DLNAPLAYER_UI_MESSAGE_GET_MEDIA_INFO_FAILED:
		
		break;
	case SH_DLNAPLAYER_UI_MESSAGE_MEDIA_TOTAL_DURATION:
		m_spThis->OnDLNAPlayerGetMediaDuration(wParam);
		break;
	case SH_DLNAPLAYER_UI_MESSAGE_MEDIA_CURRENT_POS:
		m_spThis->OnDLNAPlayerGetCurrentPlayPos(wParam);
		break;
	case SH_DLNAPLAYER_UI_MESSAGE_DEVICE_LIST_UPDATED:
		m_spThis->OnDLNAPlayerDeviceListUpdated(wParam);
		break;
	case SH_DLNAPLAYER_UI_MESSAGE_DEVICE_CURRENT_VOLUME:
		m_spThis->OnDLNAPlayerGetVolume(wParam);
		break;
	}
}

void CSHDLNAPlayerTestDlg::OnDLNAPlayerOpenMediaSucceeded()
{
	if (0 == SH_DLNAPlayer_Play())
	{
		m_PlayState = PLAY_STATE_PLAYING;

		SetTimer(TIMERID_GET_MEDIA_DURATION, ELAPSE_GET_MEDIA_DURATION, NULL);
		SetTimer(TIMERID_GET_CUR_PLAY_POS, ELAPSE_GET_CUR_PLAY_POS, NULL);
	}
}

void CSHDLNAPlayerTestDlg::OnDLNAPlayerOpenMediaFailed()
{
	MessageBox(_T("DLNAPlayer Open media failed"));
}

void CSHDLNAPlayerTestDlg::OnDLNAPlayerSeekSucceeded()
{

}

void CSHDLNAPlayerTestDlg::OnDLNAPlayerSeekFailed()
{

}

void CSHDLNAPlayerTestDlg::OnDLNAPlayerGetMediaDuration(void* wParam)
{
	if (wParam)
	{
		m_MediaDuration = (long)wParam;
		
		((CSliderCtrl*)GetDlgItem(IDC_SLIDER_PLAY_PROGRESS))->SetRange(0, m_MediaDuration);

		CString strMediaDuration = MsToString(m_MediaDuration);
		GetDlgItem(IDC_STATIC_MEDIA_DURATION)->SetWindowText(strMediaDuration);

		KillTimer(TIMERID_GET_MEDIA_DURATION);
	}
}

void CSHDLNAPlayerTestDlg::OnDLNAPlayerGetCurrentPlayPos(void* wParam)
{
	if (wParam)
	{
		long lCurPlayPos = (long)wParam;

		((CSliderCtrl*)GetDlgItem(IDC_SLIDER_PLAY_PROGRESS))->SetPos(lCurPlayPos);
	
		CString strCurPlayPos = MsToString(lCurPlayPos);
		GetDlgItem(IDC_STATIC_CUR_PLAY_POS)->SetWindowText(strCurPlayPos);

		if (lCurPlayPos >= m_MediaDuration)
		{
			KillTimer(TIMERID_GET_CUR_PLAY_POS);
		}
	}
}

void CSHDLNAPlayerTestDlg::OnDLNAPlayerDeviceListUpdated(void* wParam)
{
	m_ctrlDeviceList.ResetContent();

	SH_DLNAPlayer_DeviceList* pList = (SH_DLNAPlayer_DeviceList*)wParam;
	if (pList)
	{
		memcpy(&m_DLNADeviceList, pList, sizeof(SH_DLNAPlayer_DeviceList));
		for (int i=0; i<m_DLNADeviceList.count; ++i)
		{
			CString device_name(m_DLNADeviceList.device[i].device_name);
			m_ctrlDeviceList.AddString(device_name);
			m_ctrlDeviceList.SetItemDataPtr(i, &m_DLNADeviceList.device[i]);
		}
	}	
}

void CSHDLNAPlayerTestDlg::OnDLNAPlayerGetVolume(void* wParam)
{
	int volume = (int)wParam;

	m_ctrlVolume.SetPos(VOLUME_MAX - volume);
}


void CSHDLNAPlayerTestDlg::OnBnClickedBtnOpenLocalMedia()
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(TRUE);
	dlg.DoModal();

	if (_tcslen(dlg.m_ofn.lpstrFile) > 0)
	{
		m_FilePath = dlg.m_ofn.lpstrFile;

		GetDlgItem(IDC_EDT_LOCAL_MEDIA_PATH)->SetWindowText(m_FilePath);
	}
	else
	{
		GetDlgItem(IDC_EDT_LOCAL_MEDIA_PATH)->SetWindowText(_T(""));
	}
}

void CSHDLNAPlayerTestDlg::OnBnClickedBtnOpenOnlineMedia()
{
	// TODO: Add your control notification handler code here

	int sel = m_ctrlOnlineMedia.GetCurSel();
	if (sel >= 0)
	{
		CString strOnlineTitle;
		m_ctrlOnlineMedia.GetText(sel, strOnlineTitle);
	
		std::string title = (CStringA)strOnlineTitle;
		
		StringMapIterator it;
		if ((it = m_mapOnlineVideos.find(title)) != m_mapOnlineVideos.end())
		{
			m_FilePath = it->second.c_str();
		}
	}
}

void CSHDLNAPlayerTestDlg::OnBnClickedBtnPlay()
{
	// TODO: Add your control notification handler code here

	if (m_FilePath.IsEmpty() || m_FilePath.GetLength() <= 0)
	{
		return;
	}

	if (m_PlayState == PLAY_STATE_PAUSE || 
		m_PlayState == PLAY_STATE_STOP)
	{
		SH_DLNAPlayer_Play();
	}
	else
	{
		int index = m_ctrlDeviceList.GetCurSel();
		if (index >= 0)
		{
			SH_DLNAPlayer_DeviceInfo* pDeviceInfo = 
				(SH_DLNAPlayer_DeviceInfo*)m_ctrlDeviceList.GetItemDataPtr(index);
			if (pDeviceInfo)
			{
				if (0 == SH_DLNAPlayer_ChooseDevice(pDeviceInfo->device_uuid))
				{
					std::string file_path_utf8 = wcs2mbs(CP_UTF8, m_FilePath);
					if (0 == SH_DLNAPlayer_Open(file_path_utf8.c_str()))
					{

					}
				}
			}
		}
	}
}

void CSHDLNAPlayerTestDlg::OnBnClickedBtnPause()
{
	// TODO: Add your control notification handler code here

	if (0 == SH_DLNAPlayer_Pause())
	{
		m_PlayState = PLAY_STATE_PAUSE;
	}

}

void CSHDLNAPlayerTestDlg::OnBnClickedBtnStop()
{
	// TODO: Add your control notification handler code here
	if (0 == SH_DLNAPlayer_Stop())
	{
		m_PlayState = PLAY_STATE_STOP;
	}
}

void CSHDLNAPlayerTestDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default

	CWnd* pPlayProgress = GetDlgItem(IDC_SLIDER_PLAY_PROGRESS);
	if (pScrollBar == pPlayProgress)
	{
		int pos = ((CSliderCtrl*)pPlayProgress)->GetPos();
		SH_DLNAPlayer_Seek(pos);
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CSHDLNAPlayerTestDlg::OnBnClickedCheckEnableDlna()
{
	// TODO: Add your control notification handler code here
	int enable = m_btnEnableDLNA.GetCheck();
	if (enable)
	{
		SH_DLNAPlayer_Init(CSHDLNAPlayerTestDlg::SHDLNAPlayerMessageNotifyUI);
	}
	else
	{
		SH_DLNAPlayer_Uninit();
	}
}

void CSHDLNAPlayerTestDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
	SH_DLNAPlayer_Uninit();
}

void CSHDLNAPlayerTestDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	if (TIMERID_GET_CUR_PLAY_POS == nIDEvent)
	{
		SH_DLNAPlayer_GetCurPlayPos();
	}
	else if (TIMERID_GET_MEDIA_DURATION == nIDEvent)
	{
		SH_DLNAPlayer_GetMediaDuration();
	}

	CDialog::OnTimer(nIDEvent);
}
