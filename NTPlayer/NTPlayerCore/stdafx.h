// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>



// TODO: reference additional headers your program requires here
#include <tchar.h>

#include <atlstr.h>
#include <atlcoll.h>

#define __AFX_H__
#include "streams.h"
#undef __AFX_H__

#include "PlayerCommon.h"
#include "PlayerLog.h"
#include "../DSUtil/DSUtil.h"

#include "resource.h"


#define ENABLE_SHOW_DEBUG
#include "ShowDebug.h"

extern HINSTANCE g_hInstance;