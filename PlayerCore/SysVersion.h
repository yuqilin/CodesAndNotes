
#ifndef _PLAYERCORE_SYSVERSION_H_
#define _PLAYERCORE_SYSVERSION_H_

class CSysVersion
{
	CSysVersion() {};
	~CSysVersion() {};

	static OSVERSIONINFOEX InitFullVersion();

	static const OSVERSIONINFOEX fullVersion;
	static const DWORD version;

public:
	static OSVERSIONINFOEX GetFullVersion() { return fullVersion; }
	static DWORD GetVersion() { return version; }

	static bool IsXPOrLater() { return (version >= 0x0501); }
	static bool IsVista() { return (version == 0x0600); }
	static bool IsVistaOrLater() { return (version >= 0x0600); }
	static bool Is7() { return (version == 0x0601); }
	static bool Is7OrLater() { return (version >= 0x0601); }
	//static bool Is8() { return (version == 0x0602); }
};

#endif