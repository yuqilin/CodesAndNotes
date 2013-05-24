#ifndef _SH_DLNA_MEDIA_CONTROLLER_H_
#define _SH_DLNA_MEDIA_CONTROLLER_H_

#include "SHDLNAPlayer.h"
#include "SHDLNAMediaPlayer.h"

/*----------------------------------------------------------------------
|   class CSHDLNAMediaController
+---------------------------------------------------------------------*/
class CSHDLNAMediaController : public PLT_MediaController
							 , public PLT_MediaControllerDelegate
{
public:
	CSHDLNAMediaController(PLT_CtrlPointReference& ctrlPoint,
		/*SH_DLNAPlayer_MessageNotifyUI message_to_notify,*/
		CSHDLNAMediaPlayer* pPlayer);
	virtual ~CSHDLNAMediaController();

public:
	NPT_Result		ChooseDevice(const char* device_uuid);
	NPT_Result		OpenMedia(SHDLNAMediaInfo_t* media_info);

	NPT_Result		SetCurMeidaServer(PLT_DeviceHostReference& server);

	void			GetCurMediaRenderer(PLT_DeviceDataReference& renderer);

protected:
	//NPT_Result		ParseFileNameFromUrl(const char* url, NPT_String& title);
	void			ToDidl(const NPT_String& uri, const NPT_String& name, NPT_String& didl);

	/*
	* PLT_MediaControllerDelegate methods
	*/
	virtual bool	OnMRAdded(PLT_DeviceDataReference& device);
	virtual void	OnMRRemoved(PLT_DeviceDataReference& device);
	virtual void	OnMRStateVariablesChanged(
		PLT_Service* /* service */,
		NPT_List<PLT_StateVariable*>* /* vars */);

	void			OnVolumeStateChanged(const NPT_String& volume_state);
	void			OnTransportStateChanged(const NPT_String& transport_state);

	void			OnDeviceListUpdated();

	// AVTransport
	virtual void	OnGetCurrentTransportActionsResult(
		NPT_Result               /* res */, 
		PLT_DeviceDataReference& /* device */,
		PLT_StringList*          /* actions */, 
		void*                    /* userdata */);

	virtual void	OnGetDeviceCapabilitiesResult(
		NPT_Result               /* res */, 
		PLT_DeviceDataReference& /* device */,
		PLT_DeviceCapabilities*  /* capabilities */,
		void*                    /* userdata */);

	virtual void	OnGetMediaInfoResult(
		NPT_Result               /* res */,
		PLT_DeviceDataReference& /* device */,
		PLT_MediaInfo*           /* info */,
		void*                    /* userdata */);

	virtual void	OnGetPositionInfoResult(
		NPT_Result               /* res */,
		PLT_DeviceDataReference& /* device */,
		PLT_PositionInfo*        /* info */,
		void*                    /* userdata */);

	virtual void	OnGetTransportInfoResult(
		NPT_Result               /* res */,
		PLT_DeviceDataReference& /* device */,
		PLT_TransportInfo*       /* info */,
		void*                    /* userdata */);

	virtual void	OnGetTransportSettingsResult(
		NPT_Result               /* res */,
		PLT_DeviceDataReference& /* device */,
		PLT_TransportSettings*   /* settings */,
		void*                    /* userdata */);

	virtual void	OnNextResult(
		NPT_Result               /* res */,
		PLT_DeviceDataReference& /* device */,
		void*                    /* userdata */);

	virtual void	OnPauseResult(
		NPT_Result               /* res */,
		PLT_DeviceDataReference& /* device */,
		void*                    /* userdata */);

	virtual void	OnPlayResult(
		NPT_Result               /* res */,
		PLT_DeviceDataReference& /* device */,
		void*                    /* userdata */);

	virtual void	OnPreviousResult(
		NPT_Result               /* res */,
		PLT_DeviceDataReference& /* device */,
		void*                    /* userdata */);

	virtual void	OnSeekResult(
		NPT_Result               /* res */,
		PLT_DeviceDataReference& /* device */,
		void*                    /* userdata */);

	virtual void	OnSetAVTransportURIResult(
		NPT_Result               /* res */,
		PLT_DeviceDataReference& /* device */,
		void*                    /* userdata */);

	virtual void	OnSetPlayModeResult(
		NPT_Result               /* res */,
		PLT_DeviceDataReference& /* device */,
		void*                    /* userdata */);

	virtual void	OnStopResult(
		NPT_Result               /* res */,
		PLT_DeviceDataReference& /* device */,
		void*                    /* userdata */);

	// ConnectionManager
	virtual void	OnGetCurrentConnectionIDsResult(
		NPT_Result               /* res */,
		PLT_DeviceDataReference& /* device */,
		PLT_StringList*          /* ids */,
		void*                    /* userdata */);

	virtual void	OnGetCurrentConnectionInfoResult(
		NPT_Result               /* res */,
		PLT_DeviceDataReference& /* device */,
		PLT_ConnectionInfo*      /* info */,
		void*                    /* userdata */);

	virtual void	OnGetProtocolInfoResult(
		NPT_Result               /* res */,
		PLT_DeviceDataReference& /* device */,
		PLT_StringList*          /* sources */,
		PLT_StringList*          /* sinks */,
		void*                    /* userdata */);

	// RenderingControl
	virtual void	OnSetMuteResult(
		NPT_Result               /* res */,
		PLT_DeviceDataReference& /* device */,
		void*                    /* userdata */);

	virtual void	OnGetMuteResult(
		NPT_Result               /* res */,
		PLT_DeviceDataReference& /* device */,
		const char*              /* channel */,
		bool                     /* mute */,
		void*                    /* userdata */);

	virtual void	OnSetVolumeResult(
		NPT_Result               /* res */,
		PLT_DeviceDataReference& /* device */,
		void*                    /* userdata */);

	virtual void	OnGetVolumeResult(
		NPT_Result               /* res */,
		PLT_DeviceDataReference& /* device */,
		const char*              /* channel */,
		NPT_UInt32				 /* volume */,
		void*                    /* userdata */);

protected:
	/* The tables of known devices on the network.  These are updated via the
     * OnMSAddedRemoved and OnMRAddedRemoved callbacks.  Note that you should first lock
     * before accessing them using the NPT_Map::Lock function.
     */
	NPT_Lock<PLT_DeviceMap>				m_MediaRenderers;
	SH_DLNAPlayer_DeviceList			m_DeviceList;	

	/* The currently selected media server as well as 
     * a lock.  If you ever want to hold both the m_CurMediaRendererLock lock and the 
     * m_CurMediaServerLock lock, make sure you grab the server lock first.
     */
    PLT_DeviceHostReference				m_CurMediaServer;
    //NPT_Mutex							m_CurMediaServerLock;

	/* The currently selected media renderer as well as 
     * a lock.  If you ever want to hold both the m_CurMediaRendererLock lock and the 
     * m_CurMediaServerLock lock, make sure you grab the server lock first.
     */
    PLT_DeviceDataReference				m_CurMediaRenderer;
    NPT_Mutex							m_CurMediaRendererLock;

	/* the semaphore on which to block when waiting for a response from over
     * the network 
     */
    NPT_SharedVariable					m_CallbackResponseSemaphore;

	/*
	 *	message to notify UI
	 */
	//SH_DLNAPlayer_MessageNotifyUI		m_MessageNotifyUI;

	/*
	 *	pointer to Player
	 */
	CSHDLNAMediaPlayer*					m_Player;
};


#endif // _SH_DLNA_MEDIA_CONTROLLER_H_
