#include "stdafx.h"
#include "MainDlg.h"

LRESULT CMainDlg::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if (wParam == TIMERID_CHECK_DLNA_DEVICES)
	{
		static int nLastCount = 0;
		int nCount = 0;

		if (!m_bDLNAEnabled)
			return 0;

		char devices[DLNA_MAX_DEVICE_COUNT][DLNA_MAX_DEVICE_NAME_LEN];
		memset(&devices, 0, sizeof(devices));
		DLNA_DiscoverDevices(devices, &nCount);

		BOOL bDeviceUpdated = FALSE;

		if (m_nDeviceCount != nCount)
		{
			bDeviceUpdated = TRUE;
		}
		else
		{
			for (int i=0; i<m_nDeviceCount; i++)
			{
				if (strcmp(m_Devices[i], devices[i]) != 0)
				{
					bDeviceUpdated = TRUE;
					break;
				}
			}
		}

		if (bDeviceUpdated)
		{
			CComboBox combox = GetDlgItem(IDC_COMBO_MEDIARENDERERS);
			combox.ResetContent();
			memset(m_Devices, 0, sizeof(m_Devices));
			memcpy(m_Devices, devices, nCount * DLNA_MAX_DEVICE_NAME_LEN);
			m_nDeviceCount = nCount;
			for (int i=m_nDeviceCount-1; i>=0; --i)
			{
				combox.AddString(ATL::CString(m_Devices[i]));
			}
		}
	}

	return 0;
}


LRESULT CMainDlg::OnBnClickedBtnEnable(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	DLNA_Init(m_IOCallbacks);

	m_bDLNAEnabled = TRUE;

	return 0;
}

LRESULT CMainDlg::OnBnClickedBtnDisable(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	DLNA_Uninit();

	m_bDLNAEnabled = FALSE;

	return 0;
}

LRESULT CMainDlg::OnBnClickedBtnSetdevice(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	CComboBox combox = GetDlgItem(IDC_COMBO_MEDIARENDERERS);
	
	int nCurSel = combox.GetCurSel();

	DLNA_ChooseDevice(nCurSel);

	return 0;
}

LRESULT CMainDlg::OnBnClickedBtnOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
// 	CFileDialog dlg(TRUE);
// 
// 	dlg.DoModal();
// 
// 	m_strFilePath = dlg.m_ofn.lpstrFile;

	//m_strFilePath = _T("Z:\\Shared\\Media\\2013Oscar.mp4");

	//ATL::CStringA path(m_strFilePath);

	const char* path = "π¶∑Ú–‹√®2[≥¨«Â∞Ê]";

	CComboBox combox(GetDlgItem(IDC_COMBO_MEDIARENDERERS));

	if (0 == DLNA_ChooseDevice(combox.GetCurSel()))
	{
		//DLNA_Open(path, DLNA_FILE_LOCAL);
		DLNA_Open(path, DLNA_FILE_ONLINE);
	}

	return 0;
}

LRESULT CMainDlg::OnBnClickedBtnClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here

	DLNA_Close();

	return 0;
}

LRESULT CMainDlg::OnBnClickedBtnPlay(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here

	DLNA_Play();

	return 0;
}

LRESULT CMainDlg::OnBnClickedBtnPause(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here

	DLNA_Pause();

	return 0;
}

LRESULT CMainDlg::OnBnClickedBtnStop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here

	DLNA_Stop();

	return 0;
}

LRESULT CMainDlg::OnBnClickedBtnSeek(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	
// 	ATL::CString time_pos;
// 	GetDlgItemText(IDC_EDT_SEEKTIME, time_pos);
	//DLNA_Seek();

	return 0;
}
