// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers


// TODO: reference additional headers your program requires here
#include <atlstr.h>

#include <streams.h>


#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC   // include Microsoft memory leak detection procedures

#if 0
#include <crtdbg.h>
#define DNew new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define DNew new(__FILE__, __LINE__)
#endif

#else

#define DNew new

#endif


extern HINSTANCE		g_hInstance;
extern CUtility			g_utility;

