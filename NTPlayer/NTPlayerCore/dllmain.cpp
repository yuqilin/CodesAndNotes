// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "NTPlayerCore.h"

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
            g_hInstance = (HINSTANCE)hModule;
        }
        break;
	case DLL_THREAD_ATTACH:
        break;
	case DLL_THREAD_DETACH:
        break;
	case DLL_PROCESS_DETACH:
        {
            ntplayer_uninit();
        }
		break;
	}
	return TRUE;
}

