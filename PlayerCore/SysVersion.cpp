
#include "stdafx.h"
#include "CSysVersion.h"

const OSVERSIONINFOEX CSysVersion::fullVersion = InitFullVersion();
const DWORD CSysVersion::version = MAKEWORD(fullVersion.dwMinorVersion, fullVersion.dwMajorVersion);

OSVERSIONINFOEX CSysVersion::InitFullVersion()
{
	OSVERSIONINFOEX fullVersion = {0};
	fullVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((LPOSVERSIONINFO)&fullVersion);

	return fullVersion;
}
