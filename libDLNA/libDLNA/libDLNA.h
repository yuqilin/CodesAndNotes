#ifndef _LIBDLNA_H_
#define _LIBDLNA_H_

#ifdef LIBDLNA_EXPORTS
#define LIBDLNA_API __declspec(dllexport)
#else
#define LIBDLNA_API __declspec(dllimport)
#endif

// DLNA controller

#define DLNA_FILE_UNKNOWN				0
#define DLNA_FILE_LOCAL					1
#define DLNA_FILE_ONLINE				2
#define DLNA_MAX_DEVICE_COUNT			64
#define DLNA_MAX_DEVICE_NAME_LEN		256

typedef int (*OpenCallback)(const char* url, int type);
typedef int (*CloseCallback)();
typedef int (*ReadCallback)(void* buffer, unsigned int bytes_to_read, unsigned int* bytes_read);
typedef int (*SeekCallback)(unsigned __int64 offset);
typedef int (*TellCallback)(unsigned __int64* offset);
typedef int (*GetSizeCallback)(unsigned __int64* size);
typedef int (*GetAvailableCallback)(unsigned __int64* available);

//typedef void (*DeviceUpdateCallback)(const char* devices, int count);

typedef struct {
	OpenCallback Open;
	CloseCallback Close;
	ReadCallback Read;
	SeekCallback Seek;
	TellCallback Tell;
	GetSizeCallback GetSize;
	GetAvailableCallback GetAvailable;
} IOCallbacks;

LIBDLNA_API int		DLNA_Init(IOCallbacks* IO);
LIBDLNA_API int		DLNA_Uninit(void);
LIBDLNA_API int		DLNA_DiscoverDevices(char devices[DLNA_MAX_DEVICE_COUNT][DLNA_MAX_DEVICE_NAME_LEN], int* count);
LIBDLNA_API int		DLNA_DeviceListChanged();
LIBDLNA_API int		DLNA_ChooseDevice(int device_index);
LIBDLNA_API int		DLNA_Open(const char* url, int type = DLNA_FILE_LOCAL);
LIBDLNA_API	int		DLNA_Close();
LIBDLNA_API int		DLNA_Play();
LIBDLNA_API int		DLNA_Seek(const char* time_pos/*long pos*/ /* ms */);
LIBDLNA_API int		DLNA_Pause();
LIBDLNA_API int		DLNA_Stop();
LIBDLNA_API int		DLNA_SetVolume(int volume);
LIBDLNA_API int		DLNA_GetVolume();

//LIBDLNA_API int		DLNA_SetRange(unsigned long long start, unsigned long long end);

#endif