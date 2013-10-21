// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

//#include "../DSUtil/SharedInclude.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS  // some CString constructors will be explicit

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN        // Exclude rarely-used stuff from Windows headers
#endif


// TODO: reference additional headers your program requires here

//#include "stdafx_common_dshow.h"
#include <windows.h>
#include <atltypes.h>
#include <atlcoll.h>
#include <atlpath.h>

#define __AFX_H__
#include <streams.h>
#undef __AFX_H__

#include "../DSUtil/DSUtil.h"

