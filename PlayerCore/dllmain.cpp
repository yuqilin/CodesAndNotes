// dllmain.cpp : Defines the entry point for the DLL application.

#include "stdafx.h"
#include <windows.h>

HINSTANCE g_hInstance = NULL;

BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			g_hInstance = hModule;
			DWORD dwTimeStart = ::timeGetTime();
			g_utility.Initialize();
			DWORD dwTime = ::timeGetTime() - dwTimeStart;
			g_utility.Log(_T("Take settings cost %d ms"), dwTime);
			g_utility.Log(_T("DLL_PROCESS_ATTACH"));
		}
		break;
	case DLL_THREAD_ATTACH:
		g_utility.Log(_T("DLL_THREAD_ATTACH"));
		break;
	case DLL_THREAD_DETACH:
		g_utility.Log(_T("DLL_THREAD_DETACH"));
		break;
	case DLL_PROCESS_DETACH:
		g_utility.Log(_T("DLL_PROCESS_DETACH"));
		break;
	}
	return TRUE;
}

