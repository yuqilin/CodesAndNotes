//-----------------------------------------------------------------------------
//
//	Monogram AAC Encoder
//
//	Base on GNU LGPL libfaac
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------

#include "stdafx.h"

//-----------------------------------------------------------------------------
//
//	Registry Information
//
//-----------------------------------------------------------------------------

const AMOVIESETUP_MEDIATYPE sudPinTypes[] =   
		{ 
			{	&MEDIATYPE_Audio, 	&MEDIASUBTYPE_PCM	},
			{	&MEDIATYPE_Audio,	&MEDIASUBTYPE_AAC	}
		};

const AMOVIESETUP_PIN psudPins[] = 
		{ 
			{ 	L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, 1, &sudPinTypes[0]	}, 
			{ 	L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, 1, &sudPinTypes[1]	} 
		};   


const AMOVIESETUP_FILTER sudAACEncoder = 
		{ 
			&CLSID_MonogramAACEncoder, L"MONOGRAM AAC Encoder", MERIT_NORMAL, 2, psudPins						
		};                     

CFactoryTemplate g_Templates[]=
		{
			{
				L"MONOGRAM AAC Encoder",
				&CLSID_MonogramAACEncoder,
				CAACEncoder::CreateInstance, 
				NULL,
				&sudAACEncoder 
			},
			{
				L"MONOGRAM AAC Encoder",
				&CLSID_MonogramAACEncoderPropertypage,
				CAACPropertyPage::CreateInstance
			}
		};

int g_cTemplates = sizeof(g_Templates)/sizeof(g_Templates[0]);


//-----------------------------------------------------------------------------
//
//	DLL Entry Points
//
//-----------------------------------------------------------------------------

STDAPI DllRegisterServer() 
{
	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2(FALSE);
}

// if we declare the correct C runtime entrypoint and then forward it to the DShow base
// classes we will be sure that both the C/C++ runtimes and the base classes are initialized
// correctly
HINSTANCE	g_hInstance = NULL;
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);
BOOL APIENTRY DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if(ul_reason_for_call == DLL_PROCESS_ATTACH)
		g_hInstance = (HINSTANCE)hModule;
	return DllEntryPoint(reinterpret_cast<HINSTANCE>(hModule), ul_reason_for_call, lpReserved);
}