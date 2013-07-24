// NTLogTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int _tmain(int argc, _TCHAR* argv[])
{
    NTAUTO();
    NTDEBUG(_T("argc=%d, argv=%08X"), argc, argv);

    NTDEBUG4CXX(_T("argc=")<<argc<<_T(", argv=")<<argv);

	return 0;
}

