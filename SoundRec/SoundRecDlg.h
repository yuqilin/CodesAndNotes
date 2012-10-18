// SoundRecDlg.h : header file
//

#pragma once
#include <MMSystem.h>

#include "faac.h"

#define MAX_BUFFERS	3



// CSoundRecDlg dialog
class CSoundRecDlg : public CDialog
{
// Construction
public:
	CSoundRecDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_SOUNDREC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedRec();
	afx_msg void OnBnClickedBrowse();
	afx_msg void OnCbnSelchangeDevices();

public:
	VOID StartRecording();
	VOID ProcessHeader(WAVEHDR * pHdr);

private:
	VOID SetStatus(LPCTSTR lpszFormat, ...);
	VOID OpenDevice();
	VOID CloseDevice();
	VOID PrepareBuffers();
	VOID UnPrepareBuffers();
	UINT FillDevices();
	CString StoreError(MMRESULT mRes,BOOL bDisplay,LPCTSTR lpszFormat, ...);

	BOOL m_bRun;
	HWAVEIN m_hWaveIn;
	WAVEFORMATEX m_stWFEX;
	WAVEHDR m_stWHDR[MAX_BUFFERS];
	HANDLE m_hThread;
	CString m_csErrorText;
	HMMIO m_hOPFile;
	MMIOINFO m_stmmIF;
	MMCKINFO m_stckOut,m_stckOutRIFF;

	faacEncHandle m_hAACEncoder;
	faacEncConfigurationPtr m_pAACConfiguration;
	BYTE* m_pbAACBuffer;
	unsigned long m_nMaxInputSamples;
	unsigned long m_nMaxOutputBytes;
	FILE* m_fpAACOutput;
};
