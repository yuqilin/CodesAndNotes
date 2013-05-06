#ifndef _LIBDLNAIMPL_H_
#define _LIBDLNAIMPL_H_


/*----------------------------------------------------------------------
|   CLibDLNA
+---------------------------------------------------------------------*/

typedef NPT_Reference<PLT_UPnP>		PLT_UPnPReference;

class CFlyfoxMediaController;
class CFlyfoxStreamCtrl;

class CLibDLNA
{
public:
	CLibDLNA(void);
	~CLibDLNA(void);

public:
	NPT_Result Init();
	NPT_Result Uninit();
	NPT_Result DiscoverDevices(char devices[DLNA_MAX_DEVICE_COUNT][DLNA_MAX_DEVICE_NAME_LEN], int* count);
	NPT_Result ChooseDevice(int device_index);
	NPT_Result Open(const char* url, int type);
	NPT_Result Close();
	NPT_Result Play();
	NPT_Result Pause();
	NPT_Result Stop();
	NPT_Result Seek(const char* time_pos/*long pos*/);

	NPT_Result GetVolume(int* volume);
	NPT_Result SetVolume(int volume);

	static void SetIOCallbacks(IOCallbacks* io) {
		m_spIO = io;
	}
	static IOCallbacks* GetIOCallbacks() {
		return m_spIO;
	}

	static CFlyfoxStreamCtrl* GetStreamCtrl() {
		return m_StreamCtrl;
	}

private:
	PLT_UPnPReference					m_UPnP;
	CFlyfoxMediaController*				m_MediaController;
	PLT_DeviceHostReference				m_DLNAServer;
	//NPT_List<PLT_DeviceHostReference>   m_Devices;

	static IOCallbacks*					m_spIO;

	static CFlyfoxStreamCtrl*			m_StreamCtrl;
};

#endif