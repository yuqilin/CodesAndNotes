// NTPlayerCoreTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "NTPlayerCore.h"

#pragma comment(lib, "NTPlayerCore.lib")

int _tmain(int argc, _TCHAR* argv[])
{

    HINSTANCE hInstance = GetModuleHandle(NULL);

    ntplayer_init();


	return 0;
}

