// libdlna.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "libdlna.h"

#include "Platinum.h"

static CLibDLNA* g_spDLNA = NULL;

LIBDLNA_API int DLNA_Init(IOCallbacks* io) {

	NPT_LogManager::GetDefault().Configure("plist:.level=ALL;.handlers=FileHandler;"
		".FileHandler.filename=C:\\log\\libDLNA.log;.FileHandler.filter=9;.FileHandler.append=false;.FileHandler.flush=true;");

	if (g_spDLNA) {
		delete g_spDLNA;
	}
	g_spDLNA = new CLibDLNA;
	if (g_spDLNA) {
		g_spDLNA->Init();		
		CLibDLNA::SetIOCallbacks(io);
	}
	return 0;
}

LIBDLNA_API int	DLNA_Uninit(void) {
	if (g_spDLNA) {
		delete g_spDLNA;
		g_spDLNA = NULL;
	}
	return 0;
}

LIBDLNA_API int	DLNA_DiscoverDevices(char devices[DLNA_MAX_DEVICE_COUNT][DLNA_MAX_DEVICE_NAME_LEN], int* count) {
	if (g_spDLNA) {
		return g_spDLNA->DiscoverDevices(devices, count);
	}
	return 0;
}

LIBDLNA_API int	DLNA_ChooseDevice(int device_index) {
	if (g_spDLNA) {
		return g_spDLNA->ChooseDevice(device_index);
	}
	return 0;
}

LIBDLNA_API int	DLNA_Open(const char* url, int type) {
	if (g_spDLNA) {
		return g_spDLNA->Open(url, type);
	}
	return 0;
}

LIBDLNA_API	int	DLNA_Close() {
	if (g_spDLNA) {
		return g_spDLNA->Close();
	}
	return 0;
}

LIBDLNA_API int DLNA_Play() {
	if (g_spDLNA) {
		return g_spDLNA->Play();
	}
	return 0;
}

LIBDLNA_API int	DLNA_Seek(const char* time_pos/*long pos*/ /* ms */) {
	if (g_spDLNA) {
		return g_spDLNA->Seek(time_pos);
	}
	return 0;
}

LIBDLNA_API int	DLNA_Pause() {
	if (g_spDLNA) {
		return g_spDLNA->Pause();
	}
	return 0;

}

LIBDLNA_API int	DLNA_Stop() {
	if (g_spDLNA) {
		return g_spDLNA->Stop();
	}
	return 0;
}

LIBDLNA_API int	DLNA_SetVolume(int volume) {
	if (g_spDLNA) {
		return g_spDLNA->SetVolume(volume);
	}
	return 0;
}

LIBDLNA_API int	DLNA_GetVolume()
{
	int volume = 0;
	if (g_spDLNA) {
		return g_spDLNA->GetVolume(&volume);
	}
	return volume;
}
