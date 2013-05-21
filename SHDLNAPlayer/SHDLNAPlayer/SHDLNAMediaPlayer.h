#ifndef _SH_DLNA_MEDIA_PLAYER_H_
#define _SH_DLNA_MEDIA_PLAYER_H_

#include "SHDLNAPlayer.h"
#include "QtSequenceMerge.h"
#include "Mp4DownloadClient.h"

class CSHDLNAMediaController;

/*
 *	PLT_UPnPReference
 */
typedef NPT_Reference<PLT_UPnP>						PLT_UPnPReference;

/*
 *	
 */
typedef enum MEDIA_LOAD_STATE {
	MLS_CLOSED,
	MLS_LOADING,
	MLS_LOADED,
	MLS_CLOSING
};

/*
 *	
 */
typedef enum SH_DLNA_PLAYER_PLAY_STATE {
	PS_INVALID = -1,
	PS_PLAY   = 0,
	PS_PAUSE  = 1,
	PS_STOP   = 2,
};


/*
 *	SHMP4HeaderInfo_t
 */
typedef struct tagSHMP4HeaderInfo {
	QtSequence sequence;
	SHVideoInfo info;
	std::vector<SHCDNInfo> cdninfo;

	~tagSHMP4HeaderInfo() {
		std::vector<SHCDNInfo>().swap(cdninfo);
	}
} SHMP4HeaderInfo_t;

/*
 *	SHDLNAMediaInfo_t
 */
typedef struct tagSHDLNAMediaInfo {
	//NPT_String						url_to_device;
	NPT_String						url;
	NPT_String						title;
	long							duration;
	//long							play_pos;
	SHMP4HeaderInfo_t*				header_info;
	
	tagSHDLNAMediaInfo() {
		duration = 0;
		header_info = NULL;
	}

} SHDLNAMediaInfo_t;

/*----------------------------------------------------------------------
|   class CSHDLNAMediaPlayer
+---------------------------------------------------------------------*/
class CSHDLNAMediaPlayer
{
public:
	friend class CSHDLNAMediaDownloader;

	CSHDLNAMediaPlayer();
	~CSHDLNAMediaPlayer();

	NPT_Result		Init(SH_DLNAPlayer_MessageNotifyUI message_to_notify);
	NPT_Result		Uninit(void);
	NPT_Result		ChooseDevice(const char* device_uuid);
	const char*		GetCurrentDevice();

	NPT_Result		Open(const char* url_utf8);
	NPT_Result		Close(void);
	NPT_Result		Play(void);
	NPT_Result		Seek(long pos_to_play); // ms
	NPT_Result		Pause(void);
	NPT_Result		Stop(void);
	NPT_Result		SetVolume(int volume);	// 0-100

	NPT_Result		GetMediaDuration(void); // ms
	NPT_Result		GetCurPlayPos(void);	// ms
	NPT_Result		GetVolume(void);		// 0-100

	//static NPT_Result GetInputStream(NPT_InputStreamReference& stream);

	SH_DLNA_PLAYER_PLAY_STATE	GetPlayState() {
		//NPT_AutoLock lock(m_PlayStateLock);
		return m_PlayState;
	}

	void			OnDeviceListUpdated(void* param);
	void			OnOpenResult(NPT_Result result);
	void			OnGetMediaDurationResult(NPT_Result result, void* param);
	void			OnGetCurPlayPosResult(NPT_Result result, void* param);
	void			OnGetVolumeResult(NPT_Result result, void* param);

protected:
	NPT_Result		BuildMediaInfo(const char* url_from_ui_utf8);
	NPT_Result		ParseMediaTitle(const char* url_from_ui_utf8, NPT_String& title);
	NPT_Result		OpenMedia(bool media_info_got);


	// 	MEDIA_LOAD_STATE	GetMediaLoadState() {
	// 		NPT_AutoLock lock(m_MediaLoadStateLock);
	// 		return m_MediaLoadState;
	// 	}

	// 	NPT_Result		SetMediaLoadState(MEDIA_LOAD_STATE state) {
	// 		NPT_AutoLock lock(m_MediaLoadStateLock);
	// 		m_MediaLoadState = state;
	// 		return NPT_SUCCESS;
	// 	}

	// 	NPT_Result		SetPlayState(SH_DLNA_PLAYER_PLAY_STATE state) {
	// 		NPT_AutoLock lock(m_PlayStateLock);
	// 		m_PlayState = state;
	// 		return NPT_SUCCESS;
	// 	}

protected:
	/*
	 *	UPnP manager
	 */
	PLT_UPnPReference					m_UPnP;

	/*
	 *	internal media server
	 */
	PLT_DeviceHostReference				m_MediaServer;

	/*
	 *	media controller
	 */
	NPT_Reference<CSHDLNAMediaController>	m_MediaController;

	/*
	 *	message to notify UI
	 */
	SH_DLNAPlayer_MessageNotifyUI		m_MessageNotifyUI;

	/*
	 *	input stream
	 */
	//static	NPT_InputStreamReference	m_InputStream;

	/*
	 *	media info
	 */
	SHDLNAMediaInfo_t*					m_MediaInfo;

	/*
	 *	media load state
	 */
	MEDIA_LOAD_STATE					m_MediaLoadState;
	//NPT_Mutex							m_MediaLoadStateLock;

	/*
	 *	play state
	 */
	SH_DLNA_PLAYER_PLAY_STATE			m_PlayState;
	//NPT_Mutex							m_PlayStateLock;

	PLT_TaskManager						m_TaskManager;
};


#endif // _SH_DLNA_MEDIA_PLAYER_H_
