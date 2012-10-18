// SoundRecDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SoundRec.h"
#include "SoundRecDlg.h"
#include ".\soundrecdlg.h"


#pragma comment(lib,"winmm.lib")
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSoundRecDlg dialog



CSoundRecDlg::CSoundRecDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSoundRecDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bRun=FALSE;
	m_hThread=NULL;
	m_hWaveIn=NULL;
	ZeroMemory(&m_stWFEX,sizeof(WAVEFORMATEX));
	ZeroMemory(m_stWHDR,MAX_BUFFERS*sizeof(WAVEHDR));

	m_hAACEncoder = NULL;
	m_pAACConfiguration = NULL;
	m_pbAACBuffer = NULL;
	m_nMaxInputSamples = 0;
	m_nMaxOutputBytes = 0;
	m_fpAACOutput = NULL;
	
}

void CSoundRecDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSoundRecDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(ID_REC, OnBnClickedRec)
	ON_BN_CLICKED(IDC_BROWSE, OnBnClickedBrowse)
	ON_CBN_SELCHANGE(IDC_DEVICES, OnCbnSelchangeDevices)
END_MESSAGE_MAP()


// CSoundRecDlg message handlers

BOOL CSoundRecDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	if(FillDevices()<=0)
	{
		AfxMessageBox("NO Input Devices Found..",MB_ICONERROR);
		CDialog::OnOK();
	}
	char szFileName[MAX_PATH]={0};
	GetModuleFileName(NULL,szFileName,MAX_PATH);
	szFileName[lstrlen(szFileName)-3]='\0';
	lstrcat(szFileName,"wav");
	GetDlgItem(IDC_FILENAME)->SetWindowText(szFileName);
	return TRUE;  // return TRUE  unless you set the focus to a control
}
UINT CSoundRecDlg::FillDevices()
{
	CComboBox *pBox=(CComboBox*)GetDlgItem(IDC_DEVICES);
	UINT nDevices,nC1;
	WAVEINCAPS stWIC={0};
	MMRESULT mRes;

	pBox->ResetContent();
	nDevices=waveInGetNumDevs();

	for(nC1=0;nC1<nDevices;++nC1)
	{
		ZeroMemory(&stWIC,sizeof(WAVEINCAPS));
		mRes=waveInGetDevCaps(nC1,&stWIC,sizeof(WAVEINCAPS));
		if(mRes==0)
			pBox->AddString(stWIC.szPname);
		else
			StoreError(mRes,TRUE,"File: %s ,Line Number:%d",__FILE__,__LINE__);
	}
	if(pBox->GetCount())
	{
		pBox->SetCurSel(0);
		OnCbnSelchangeDevices();
	}
	return nDevices;
}
// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSoundRecDlg::OnPaint() 
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
HCURSOR CSoundRecDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

DWORD WINAPI ThFunc(LPVOID pDt)
{
	CSoundRecDlg *pOb=(CSoundRecDlg*)pDt;
	pOb->StartRecording();
	return 0;
}

void CALLBACK waveInProc(HWAVEIN hwi,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2)
{
	WAVEHDR *pHdr=NULL;
	switch(uMsg)
	{
		case WIM_CLOSE:
			break;

		case WIM_DATA:
			{
				CSoundRecDlg *pDlg=(CSoundRecDlg*)dwInstance;
				pDlg->ProcessHeader((WAVEHDR *)dwParam1);
			}
			break;

		case WIM_OPEN:
			break;

		default:
			break;
	}
}

VOID CSoundRecDlg::ProcessHeader(WAVEHDR * pHdr)
{
	MMRESULT mRes=0;

	TRACE("%d",pHdr->dwUser);
	if(WHDR_DONE==(WHDR_DONE &pHdr->dwFlags))
	{
		ShowDebug(_T("CSoundRecDlg::ProcessHeader, pHdr->dwBytesRecorded=%d"), pHdr->dwBytesRecorded);

		unsigned long nInputSamples = pHdr->dwBytesRecorded / (m_stWFEX.wBitsPerSample / 8);
		ShowDebug(_T("CSoundRecDlg::ProcessHeader faacEncEncode nInputSamples=%d"), nInputSamples);

		long nLeftSamples = nInputSamples;
		BYTE* pData = (BYTE*)pHdr->lpData;
		while (nLeftSamples > 0)
		{
			int nBytesEncoded = faacEncEncode(m_hAACEncoder, (int*)pData, m_nMaxInputSamples, m_pbAACBuffer, m_nMaxOutputBytes);

			ShowDebug(_T("CSoundRecDlg::ProcessHeader faacEncEncode nBytesEncoded=%d"), nBytesEncoded);

			int nBytesWritten = fwrite(m_pbAACBuffer, 1, nBytesEncoded, m_fpAACOutput);

			ShowDebug(_T("CSoundRecDlg::ProcessHeader fwrite nBytesWritten=%d"), nBytesWritten);

			nLeftSamples -= m_nMaxInputSamples;
			pData += m_nMaxInputSamples;
		}
		

		mmioWrite(m_hOPFile,pHdr->lpData,pHdr->dwBytesRecorded);
		mRes=waveInAddBuffer(m_hWaveIn,pHdr,sizeof(WAVEHDR));
		if(mRes!=0)
			StoreError(mRes,TRUE,"File: %s ,Line Number:%d",__FILE__,__LINE__);

		
	}
}

VOID CSoundRecDlg::OpenDevice()
{
	int nT1=0;
	CString csT1;
	double dT1=0.0;
	MMRESULT mRes=0;
	CComboBox *pDevices=(CComboBox*)GetDlgItem(IDC_DEVICES);
	CComboBox *pFormats=(CComboBox*)GetDlgItem(IDC_FORMATS);

	nT1=pFormats->GetCurSel();
	if(nT1==-1)
		throw "";
	pFormats->GetLBText(nT1,csT1);
	sscanf((PCHAR)(LPCTSTR)csT1,"%lf",&dT1);
	dT1=dT1*1000;
	m_stWFEX.nSamplesPerSec=(int)dT1;
	csT1=csT1.Right(csT1.GetLength()-csT1.Find(',')-1);
	csT1.Trim();
	if(csT1.Find("mono")!=-1)
		m_stWFEX.nChannels=1;
	if(csT1.Find("stereo")!=-1)
		m_stWFEX.nChannels=2;
	csT1=csT1.Right(csT1.GetLength()-csT1.Find(',')-1);
	csT1.Trim();
	sscanf((PCHAR)(LPCTSTR)csT1,"%d",&m_stWFEX.wBitsPerSample);
	m_stWFEX.wFormatTag=WAVE_FORMAT_PCM;
	m_stWFEX.nBlockAlign=m_stWFEX.nChannels*m_stWFEX.wBitsPerSample/8;
	m_stWFEX.nAvgBytesPerSec=m_stWFEX.nSamplesPerSec*m_stWFEX.nBlockAlign;
	m_stWFEX.cbSize=sizeof(WAVEFORMATEX);
	mRes=waveInOpen(&m_hWaveIn,pDevices->GetCurSel(),&m_stWFEX,(DWORD_PTR)waveInProc,(DWORD_PTR)this,CALLBACK_FUNCTION);
	if(mRes!=MMSYSERR_NOERROR)
	{
		StoreError(mRes,FALSE,"File: %s ,Line Number:%d",__FILE__,__LINE__);
		throw m_csErrorText;
	}
	GetDlgItem(IDC_FILENAME)->GetWindowText(csT1);
	ZeroMemory(&m_stmmIF,sizeof(MMIOINFO));
	DeleteFile((PCHAR)(LPCTSTR)csT1);
	m_hOPFile=mmioOpen((PCHAR)(LPCTSTR)csT1,&m_stmmIF,MMIO_WRITE | MMIO_CREATE);
	if(m_hOPFile==NULL)
		throw "Can not open file...";

	ZeroMemory(&m_stckOutRIFF,sizeof(MMCKINFO));
	m_stckOutRIFF.fccType = mmioFOURCC('W', 'A', 'V', 'E'); 
	mRes=mmioCreateChunk(m_hOPFile, &m_stckOutRIFF, MMIO_CREATERIFF);
	if(mRes!=MMSYSERR_NOERROR)
	{
		StoreError(mRes,FALSE,"File: %s ,Line Number:%d",__FILE__,__LINE__);
		throw m_csErrorText;
	}
	ZeroMemory(&m_stckOut,sizeof(MMCKINFO));
	m_stckOut.ckid = mmioFOURCC('f', 'm', 't', ' ');
	m_stckOut.cksize = sizeof(m_stWFEX);
	mRes=mmioCreateChunk(m_hOPFile, &m_stckOut, 0);
	if(mRes!=MMSYSERR_NOERROR)
	{
		StoreError(mRes,FALSE,"File: %s ,Line Number:%d",__FILE__,__LINE__);
		throw m_csErrorText;
	}
	nT1=mmioWrite(m_hOPFile, (HPSTR) &m_stWFEX, sizeof(m_stWFEX));
	if(nT1!=sizeof(m_stWFEX))
	{
		m_csErrorText.Format("Can not write Wave Header..File: %s ,Line Number:%d",__FILE__,__LINE__);
		throw m_csErrorText;
	}
	mRes=mmioAscend(m_hOPFile, &m_stckOut, 0);
	if(mRes!=MMSYSERR_NOERROR)
	{
		StoreError(mRes,FALSE,"File: %s ,Line Number:%d",__FILE__,__LINE__);
		throw m_csErrorText;
	}
	m_stckOut.ckid = mmioFOURCC('d', 'a', 't', 'a');
	mRes=mmioCreateChunk(m_hOPFile, &m_stckOut, 0);
	if(mRes!=MMSYSERR_NOERROR)
	{
		StoreError(mRes,FALSE,"File: %s ,Line Number:%d",__FILE__,__LINE__);
		throw m_csErrorText;
	}

	// Open FAAC encoder
	m_hAACEncoder = faacEncOpen(m_stWFEX.nSamplesPerSec, m_stWFEX.nChannels, &m_nMaxInputSamples, &m_nMaxOutputBytes);
	
	ShowDebug(_T("CSoundRecDlg::faacEncOpen m_nMaxInputSamples=%d, m_nMaxOutputBytes=%d"), m_nMaxInputSamples, m_nMaxOutputBytes);

	if (m_hAACEncoder == NULL)
	{
		StoreError(mRes,FALSE,"File: %s ,Line Number:%d",__FILE__,__LINE__);
		throw m_csErrorText;
	}

	m_pbAACBuffer = new BYTE[m_nMaxOutputBytes];
	if (m_pbAACBuffer == NULL)
	{
		return;
	}
	
	// Get current encoding configuration
	m_pAACConfiguration = faacEncGetCurrentConfiguration(m_hAACEncoder);
// 	switch (m_stWFEX.wBitsPerSample)
// 	{
// 	case 16:
// 		m_pAACConfiguration->inputFormat = FAAC_INPUT_16BIT;
// 		break;
// 	case 24:
// 		m_pAACConfiguration->inputFormat = FAAC_INPUT_24BIT;
// 		break;
// 	case 32:
// 		m_pAACConfiguration->inputFormat = FAAC_INPUT_32BIT;
// 		break;
// 	default:
// 		m_pAACConfiguration->inputFormat = FAAC_INPUT_16BIT;
// 	}
	m_pAACConfiguration->inputFormat = FAAC_INPUT_16BIT;

	// Set encoding configuration
	int nRet = faacEncSetConfiguration(m_hAACEncoder, m_pAACConfiguration);

	errno_t err = fopen_s(&m_fpAACOutput, "out.aac", "wb");


}
VOID CSoundRecDlg::CloseDevice()
{
	MMRESULT mRes=0;
	
	if(m_hWaveIn)
	{
		UnPrepareBuffers();
		mRes=waveInClose(m_hWaveIn);
	}
	if(m_hOPFile)
	{
		mRes=mmioAscend(m_hOPFile, &m_stckOut, 0);
		if(mRes!=MMSYSERR_NOERROR)
		{
			StoreError(mRes,FALSE,"File: %s ,Line Number:%d",__FILE__,__LINE__);
		}
		mRes=mmioAscend(m_hOPFile, &m_stckOutRIFF, 0);
		if(mRes!=MMSYSERR_NOERROR)
		{
			StoreError(mRes,FALSE,"File: %s ,Line Number:%d",__FILE__,__LINE__);
		}
		mmioClose(m_hOPFile,0);
		m_hOPFile=NULL;
	}
	m_hWaveIn=NULL;

	// Close FAAC encoder
	int nRet = faacEncClose(m_hAACEncoder);
	m_hAACEncoder = 0;
	if (m_pbAACBuffer)
		delete[] m_pbAACBuffer;
	m_pbAACBuffer = NULL;
	if (m_fpAACOutput)
		fclose(m_fpAACOutput);
	m_fpAACOutput = NULL;
}

VOID CSoundRecDlg::PrepareBuffers()
{
	MMRESULT mRes=0;
	int nT1=0;

	for(nT1=0;nT1<MAX_BUFFERS;++nT1)
	{
		m_stWHDR[nT1].lpData=(LPSTR)HeapAlloc(GetProcessHeap(),8,m_stWFEX.nAvgBytesPerSec);
		m_stWHDR[nT1].dwBufferLength=m_stWFEX.nAvgBytesPerSec;
		m_stWHDR[nT1].dwUser=nT1;
		mRes=waveInPrepareHeader(m_hWaveIn,&m_stWHDR[nT1],sizeof(WAVEHDR));
		if(mRes!=0)
		{
			StoreError(mRes,FALSE,"File: %s ,Line Number:%d",__FILE__,__LINE__);
			throw m_csErrorText;
		}
		mRes=waveInAddBuffer(m_hWaveIn,&m_stWHDR[nT1],sizeof(WAVEHDR));
		if(mRes!=0)
		{
			StoreError(mRes,FALSE,"File: %s ,Line Number:%d",__FILE__,__LINE__);
			throw m_csErrorText;
		}
	}
}

VOID CSoundRecDlg::UnPrepareBuffers()
{
	MMRESULT mRes=0;
	int nT1=0;

	if(m_hWaveIn)
	{
		mRes=waveInStop(m_hWaveIn);
		for(nT1=0;nT1<3;++nT1)
		{
			if(m_stWHDR[nT1].lpData)
			{
				mRes=waveInUnprepareHeader(m_hWaveIn,&m_stWHDR[nT1],sizeof(WAVEHDR));
				HeapFree(GetProcessHeap(),0,m_stWHDR[nT1].lpData);
				ZeroMemory(&m_stWHDR[nT1],sizeof(WAVEHDR));
			}
		}
	}
}

VOID CSoundRecDlg::StartRecording()
{	
	MMRESULT mRes;
	SetStatus("Recording...");

	try
	{
		OpenDevice();
		PrepareBuffers();
		mRes=waveInStart(m_hWaveIn);
		if(mRes!=0)
		{
			StoreError(mRes,FALSE,"File: %s ,Line Number:%d",__FILE__,__LINE__);
			throw m_csErrorText;
		}
		while(m_bRun)
		{
			SleepEx(100,FALSE);
		}
	}
	catch(PCHAR pErrorMsg)
	{
		AfxMessageBox(pErrorMsg);
	}
	CloseDevice();
	CloseHandle(m_hThread);
	m_hThread=NULL;
	SetStatus("Recording stopped...");
}

void CSoundRecDlg::OnBnClickedRec()
{
	CString csT1;
	BOOL bEnable=FALSE;

	GetDlgItem(ID_REC)->GetWindowText(csT1);
	GetDlgItem(ID_REC)->EnableWindow(FALSE);
	if(csT1.Compare("&Start")==0)
	{
		m_bRun=TRUE;
		GetDlgItem(ID_REC)->SetWindowText("&Stop");
		m_hThread=CreateThread(NULL,0,ThFunc,this,0,NULL);
	}
	else
	{
		m_bRun=FALSE;
		while(m_hThread)
		{
			SleepEx(100,FALSE);
		}
		bEnable=TRUE;AfxMessageBox("Recording Stopped..");
		GetDlgItem(ID_REC)->SetWindowText("&Start");
	}
	GetDlgItem(IDC_BROWSE)->EnableWindow(bEnable);
	GetDlgItem(IDC_DEVICES)->EnableWindow(bEnable);
	GetDlgItem(IDC_FORMATS)->EnableWindow(bEnable);
	GetDlgItem(ID_REC)->EnableWindow(TRUE);
}

void CSoundRecDlg::OnBnClickedBrowse()
{
	CFileDialog oDlg(FALSE,".wav","SndRec",OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT,"WAV Files|*.wav||",this,0);
	if(oDlg.DoModal()==IDOK)
	{
		GetDlgItem(IDC_FILENAME)->SetWindowText(oDlg.GetPathName());
	}
}

void CSoundRecDlg::OnCbnSelchangeDevices()
{
	CComboBox *pDevices=(CComboBox*)GetDlgItem(IDC_DEVICES);
	CComboBox *pFormats=(CComboBox*)GetDlgItem(IDC_FORMATS);
	int nSel;
	WAVEINCAPS stWIC={0};
	MMRESULT mRes;


	SetStatus("Querying device informations...");
	pFormats->ResetContent();
	nSel=pDevices->GetCurSel();
	if(nSel!=-1)
	{
		ZeroMemory(&stWIC,sizeof(WAVEINCAPS));
		mRes=waveInGetDevCaps(nSel,&stWIC,sizeof(WAVEINCAPS));
		if(mRes==0)
		{
			if(WAVE_FORMAT_1M08==(stWIC.dwFormats&WAVE_FORMAT_1M08))
				pFormats->SetItemData(pFormats->AddString("11.025 kHz, mono, 8-bit"),WAVE_FORMAT_1M08);
			if(WAVE_FORMAT_1M16==(stWIC.dwFormats&WAVE_FORMAT_1M16))
				pFormats->SetItemData(pFormats->AddString("11.025 kHz, mono, 16-bit"),WAVE_FORMAT_1M16);
			if(WAVE_FORMAT_1S08==(stWIC.dwFormats&WAVE_FORMAT_1S08))
				pFormats->SetItemData(pFormats->AddString("11.025 kHz, stereo, 8-bit"),WAVE_FORMAT_1S08);
			if(WAVE_FORMAT_1S16==(stWIC.dwFormats&WAVE_FORMAT_1S16))
				pFormats->SetItemData(pFormats->AddString("11.025 kHz, stereo, 16-bit"),WAVE_FORMAT_1S16);
			if(WAVE_FORMAT_2M08==(stWIC.dwFormats&WAVE_FORMAT_2M08))
				pFormats->SetItemData(pFormats->AddString("22.05 kHz, mono, 8-bit"),WAVE_FORMAT_2M08);
			if(WAVE_FORMAT_2M16==(stWIC.dwFormats&WAVE_FORMAT_2M16))
				pFormats->SetItemData(pFormats->AddString("22.05 kHz, mono, 16-bit"),WAVE_FORMAT_2M16);
			if(WAVE_FORMAT_2S08==(stWIC.dwFormats&WAVE_FORMAT_2S08))
				pFormats->SetItemData(pFormats->AddString("22.05 kHz, stereo, 8-bit"),WAVE_FORMAT_2S08);
			if(WAVE_FORMAT_2S16==(stWIC.dwFormats&WAVE_FORMAT_2S16))
				pFormats->SetItemData(pFormats->AddString("22.05 kHz, stereo, 16-bit"),WAVE_FORMAT_2S16);
			if(WAVE_FORMAT_4M08==(stWIC.dwFormats&WAVE_FORMAT_4M08))
				pFormats->SetItemData(pFormats->AddString("44.1 kHz, mono, 8-bit"),WAVE_FORMAT_4M08);
			if(WAVE_FORMAT_4M16==(stWIC.dwFormats&WAVE_FORMAT_4M16))
				pFormats->SetItemData(pFormats->AddString("44.1 kHz, mono, 16-bit"),WAVE_FORMAT_4M16);
			if(WAVE_FORMAT_4S08==(stWIC.dwFormats&WAVE_FORMAT_4S08))
				pFormats->SetItemData(pFormats->AddString("44.1 kHz, stereo, 8-bit"),WAVE_FORMAT_4S08);
			if(WAVE_FORMAT_4S16==(stWIC.dwFormats&WAVE_FORMAT_4S16))
				pFormats->SetItemData(pFormats->AddString("44.1 kHz, stereo, 16-bit"),WAVE_FORMAT_4S16);
			if(WAVE_FORMAT_96M08==(stWIC.dwFormats&WAVE_FORMAT_96M08))
				pFormats->SetItemData(pFormats->AddString("96 kHz, mono, 8-bit"),WAVE_FORMAT_96M08);
			if(WAVE_FORMAT_96S08==(stWIC.dwFormats&WAVE_FORMAT_96S08))
				pFormats->SetItemData(pFormats->AddString("96 kHz, stereo, 8-bit"),WAVE_FORMAT_96S08);
			if(WAVE_FORMAT_96M16==(stWIC.dwFormats&WAVE_FORMAT_96M16))
				pFormats->SetItemData(pFormats->AddString("96 kHz, mono, 16-bit"),WAVE_FORMAT_96M16);
			if(WAVE_FORMAT_96S16==(stWIC.dwFormats&WAVE_FORMAT_96S16))
				pFormats->SetItemData(pFormats->AddString("96 kHz, stereo, 16-bit"),WAVE_FORMAT_96S16);
			if(pFormats->GetCount())
				pFormats->SetCurSel(0);
		}
		else
			StoreError(mRes,TRUE,"File: %s ,Line Number:%d",__FILE__,__LINE__);
	}
	SetStatus("Waiting to start...");
}

CString CSoundRecDlg::StoreError(MMRESULT mRes,BOOL bDisplay,LPCTSTR lpszFormat, ...)
{
	MMRESULT mRes1=0;
	char szErrorText[1024]={0};
	char szT1[2*MAX_PATH]={0};
	
	va_list args;
	va_start(args, lpszFormat);
	_vsntprintf(szT1, MAX_PATH, lpszFormat, args);
	va_end(args);

	m_csErrorText.Empty();
	if(m_bRun)
	{
		mRes1=waveInGetErrorText(mRes,szErrorText,1024);
		if(mRes1!=0)
			wsprintf(szErrorText,"Error %d in querying the error string for error code %d",mRes1,mRes);
		m_csErrorText.Format("%s: %s",szT1,szErrorText);
		if(bDisplay)
			AfxMessageBox(m_csErrorText);
	}
	return m_csErrorText;
}

VOID CSoundRecDlg::SetStatus(LPCTSTR lpszFormat, ...)
{
	CString csT1;
	va_list args;

	va_start(args, lpszFormat);
	csT1.FormatV(lpszFormat,args);
	va_end(args);
	if(IsWindow(m_hWnd) && GetDlgItem(IDC_STATUS))
		GetDlgItem(IDC_STATUS)->SetWindowText(csT1);
}
