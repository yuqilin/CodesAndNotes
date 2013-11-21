
// mfcTest.h : main header file for the mfcTest application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CmfcTestApp:
// See mfcTest.cpp for the implementation of this class
//

class CmfcTestApp : public CWinAppEx
{
public:
	CmfcTestApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CmfcTestApp theApp;
