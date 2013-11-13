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
#include <atlbase.h>
#include <atlcoll.h>
#include <atlstr.h>

#define __AFX_H__
#include "streams.h"
#undef __AFX_H__

#include "Utils.h"
#include "resource.h"

#include "../DSUtil/DSUtil.h"

//using ATL::CString;

#include "NTLog.h"

#include "PlayerCore.h"
#include "GraphThread.h"