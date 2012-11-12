//-----------------------------------------------------------------------------
//
//	Monogram AAC Encoder
//
//	Base on GNU LGPL libfaac
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

#ifndef WINVER
	#define WINVER 0x0501
#endif
#ifndef _WIN32_WINNT
	#define _WIN32_WINNT 0x0501
#endif						
#ifndef _WIN32_WINDOWS
	#define _WIN32_WINDOWS 0x0410
#endif
#ifndef _WIN32_IE
	#define _WIN32_IE 0x0600
#endif

#define WIN32_LEAN_AND_MEAN
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS

#ifndef VC_EXTRALEAN
	#define VC_EXTRALEAN
#endif

// #include <afx.h>
// #include <afxwin.h>
// #include <afxext.h>
#include <atlbase.h>
#include <atlstr.h>

// #ifndef _AFX_NO_OLE_SUPPORT
// 	#include <afxdtctl.h>
// #endif
// #ifndef _AFX_NO_AFXCMN_SUPPORT
// 	#include <afxcmn.h>
// #endif

#include <iostream>
#include <windows.h>
#include <windowsx.h>

#include <streams.h>
#include <initguid.h>

#include "..\resource.h"
#include "faac.h"
#include "faaccfg.h"

#include "aac_types.h"
#include "aac_filter.h"
#include "aac_prop.h"


inline void ShowDebug(const TCHAR * pcszFormat, ... )
{
	TCHAR szBuffer[1024];

	va_list vl;
	va_start(vl, pcszFormat);
	_vstprintf_s(szBuffer, pcszFormat, vl);
	va_end(vl);

	::OutputDebugString(szBuffer);
}
