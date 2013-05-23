#include "stdafx.h"
#include "SHDLNAMediaPlayer.h"
#include "SHDLNAMediaServer.h"
#include "SHDLNAMediaDownloader.h"
#include "Mp4DownloadClient.h"

NPT_SET_LOCAL_LOGGER("shdlnaplayer.shdlnamediaplayer")

//NPT_InputStreamReference CSHDLNAMediaPlayer::m_InputStream;

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::CSHDLNAMediaPlayer
+---------------------------------------------------------------------*/
CSHDLNAMediaPlayer::CSHDLNAMediaPlayer()
: m_MessageNotifyUI(NULL)
, m_UPnP(NULL)
, m_MediaServer(NULL)
, m_MediaLoadState(MLS_CLOSED)
, m_PlayState(SH_DLNAPLAYER_PLAY_STATE_INVALID)
, m_MediaInfo(NULL)
, m_MediaDownloader(NULL)
{
	//memset(&m_MediaInfo, 0, sizeof(m_MediaInfo));	
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::~CSHDLNAMediaPlayer
+---------------------------------------------------------------------*/
CSHDLNAMediaPlayer::~CSHDLNAMediaPlayer()
{
	Uninit();
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::Init
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaPlayer::Init(SH_DLNAPlayer_MessageNotifyUI message_to_notify)
{
	m_UPnP = new PLT_UPnP;

	m_MediaServer = new CSHDLNAMediaServer("SHPlayer UPnP Media Server");
	m_MediaServer->m_ModelDescription = "SHPlayer UPnP Media Server";
	m_MediaServer->m_ModelURL = "http://tv.sohu.com/";
	m_MediaServer->m_ModelNumber = "1.0";
	m_MediaServer->m_ModelName = "SHPlayer UPnP Media Server";
	m_MediaServer->m_Manufacturer = "SHPlayer";
	m_MediaServer->m_ManufacturerURL = "http://tv.sohu.com/";

	// add device
	m_UPnP->AddDevice(m_MediaServer);
	//m_UPnP->SetIgnoreLocalUUIDs(false);

	PLT_CtrlPointReference ctrlPoint(new PLT_CtrlPoint());

	m_MediaController = new CSHDLNAMediaController(ctrlPoint, /*message_to_notify,*/ this);

	if (!m_MediaController.IsNull())
	{
		m_MediaController->SetCurMeidaServer(m_MediaServer);
	}

	m_UPnP->AddCtrlPoint(ctrlPoint);
	m_UPnP->Start();

	m_MessageNotifyUI = message_to_notify;

	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::Uninit
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaPlayer::Uninit()
{
	if (!m_UPnP.IsNull())
		m_UPnP->Stop();

	Close();

	m_UPnP = NULL;

	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::ChooseDevice
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaPlayer::ChooseDevice(const char* device_uuid)
{
	NPT_Result result = NPT_ERROR_NO_SUCH_ITEM;

	if (!m_MediaController.IsNull())
		result = m_MediaController->ChooseDevice(device_uuid);

	if (NPT_SUCCEEDED(result))
	{
		NPT_String protocol;
		NPT_List<NPT_String> protocol_sinks;
		NPT_Result res_protocol_sink = m_MediaController->GetProtocolInfoSink(device_uuid, protocol_sinks);

		for (NPT_List<NPT_String>::Iterator iter = protocol_sinks.GetFirstItem();
			iter;
			iter++)
		{
			protocol += *iter;
		}

		NPT_String transport_state;
		NPT_Result res_trans_state = m_MediaController->GetTransportState(device_uuid, transport_state);


		NPT_UInt32 volume = 0;
		NPT_Result res_volume_state = m_MediaController->GetVolumeState(device_uuid, volume);

		int a = 1;
	}

	return result;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::GetCurrentDevice
+---------------------------------------------------------------------*/
const char* CSHDLNAMediaPlayer::GetCurrentDevice()
{
	const char* device_uuid = NULL;

	if (!m_MediaController.IsNull())
	{
		PLT_DeviceDataReference device;
		m_MediaController->GetCurMediaRenderer(device);

		if (!device.IsNull())
		{
			device_uuid = device->GetUUID().GetChars();
		}
	}
	
	return device_uuid;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::Open
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaPlayer::Open(const char* url)
{
	NPT_Result result = NPT_FAILURE;

	//NPT_AutoLock lock(m_MediaLoadStateLock);
// 	if (m_MediaLoadState == MLS_LOADED ||
// 		m_MediaLoadState == MLS_LOADING)
// 	{
// 		
// 	}
	Close();

	m_MediaLoadState = MLS_LOADING;

	m_MediaInfo = new SHDLNAMediaInfo_t;
	m_MediaInfo->url = url;

	NPT_String strUrl(url);
	if (strUrl.StartsWith("http://"))
	{
		//m_TaskManager.StartTask(new CSHDLNAMediaDownloader(url, this), NULL, false);
		m_MediaDownloader = new CSHDLNAMediaDownloader(url, this);
		if (m_MediaDownloader != NULL)
		{
			m_MediaDownloader->Start();
		}
	}
	else
	{		
		m_MediaInfo->title = NPT_FilePath::BaseName(strUrl);

		std::wstring wstrUrl = mbs2wcs(CP_UTF8, url);

		OpenMedia(true);
	}

	return result;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::Close
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaPlayer::Close(void)
{
// 	if (m_MediaLoadState == MLS_LOADED || m_MediaLoadState == MLS_LOADING)
// 	{
// 		m_MediaLoadState = MLS_CLOSING;
// 		Stop();
// 
// 		//m_TaskManager.StopAllTasks();
// 		if (m_MediaDownloader != NULL)
// 		{
// 			m_MediaDownloader->Interrupt();
// 
// 			delete m_MediaDownloader;
// 			m_MediaDownloader = NULL;
// 		}
// 
// 	}
	m_MediaLoadState = MLS_CLOSING;
	Stop();

	//m_TaskManager.StopAllTasks();
	if (m_MediaDownloader != NULL)
	{
		m_MediaDownloader->Interrupt();

		delete m_MediaDownloader;
		m_MediaDownloader = NULL;
	}
	
	delete m_MediaInfo;
	m_MediaInfo	= NULL;
	//memset(&m_MediaInfo, 0, sizeof(m_MediaInfo));

	CSHDLNAMediaServer* pMediaServer = dynamic_cast<CSHDLNAMediaServer*>(m_MediaServer.AsPointer());
	if (pMediaServer != NULL)
	{
		pMediaServer->CloseMedia();
	}

	m_MediaLoadState = MLS_CLOSED;

	{
		NPT_AutoLock lock(m_PlayStateLock);
		m_PlayState = SH_DLNAPLAYER_PLAY_STATE_INVALID;
	}
	
	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::Play
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaPlayer::Play(void)
{
	{
		//NPT_AutoLock lock(m_MediaLoadStateLock);
		if (m_MediaLoadState != MLS_LOADED)
		{
			return NPT_FAILURE;
		}
	}

// 	{
// 		NPT_AutoLock lock(m_PlayStateLock);
// 		if (m_PlayState == SH_DLNAPLAYER_PLAY_STATE_PLAYING)
// 		{
// 			return NPT_SUCCESS;
// 		}
// 	}
	
	if (!m_MediaController.IsNull())
	{
		PLT_DeviceDataReference device;
		m_MediaController->GetCurMediaRenderer(device);
		if (!device.IsNull())
		{
			return m_MediaController->Play(device, 0, "1", NULL);
		}
	}
	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::Seek
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaPlayer::Seek(long pos_to_play)
{
// 	{
// 		//NPT_AutoLock lock(m_MediaLoadStateLock);
// 		if (m_MediaLoadState != MLS_LOADED)
// 		{
// 			return NPT_FAILURE;
// 		}
// 	}

	if (!m_MediaController.IsNull())
	{
		PLT_DeviceDataReference device;
		m_MediaController->GetCurMediaRenderer(device);
		if (!device.IsNull())
		{
			NPT_String target = MillisecondsToString(pos_to_play);
			return m_MediaController->Seek(device, 0, "REL_TIME", target, NULL);
		}
	}
	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::Pause
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaPlayer::Pause()
{
// 	{
// 		//NPT_AutoLock lock(m_MediaLoadStateLock);
// 		if (m_MediaLoadState != MLS_LOADED)
// 		{
// 			return NPT_FAILURE;
// 		}
// 	}
// 	{
// 		NPT_AutoLock lock(m_PlayStateLock);
// 		if (m_PlayState != SH_DLNAPLAYER_PLAY_STATE_PLAYING)
// 			return NPT_SUCCESS;
// 	}

	if (!m_MediaController.IsNull())
	{
		PLT_DeviceDataReference device;
		m_MediaController->GetCurMediaRenderer(device);
		if (!device.IsNull())
		{
			return m_MediaController->Pause(device, 0, NULL);
		}
	}
	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::Stop
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaPlayer::Stop()
{
// 	{
// 		NPT_AutoLock lock(m_PlayStateLock);
// 		if (m_PlayState == SH_DLNAPLAYER_PLAY_STATE_INVALID || m_PlayState == SH_DLNAPLAYER_PLAY_STATE_STOP)
// 			return NPT_SUCCESS;
// 	}

	if (!m_MediaController.IsNull())
	{
		PLT_DeviceDataReference device;
		m_MediaController->GetCurMediaRenderer(device);
		if (!device.IsNull())
		{
			return m_MediaController->Stop(device, 0, NULL);
		}
	}
	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::SetVolume
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaPlayer::SetVolume(int volume)
{
	if (!m_MediaController.IsNull())
	{
		PLT_DeviceDataReference device;
		m_MediaController->GetCurMediaRenderer(device);
		if (!device.IsNull())
		{
			return m_MediaController->SetVolume(device, 0, "Master", volume, NULL);
		}
	}
	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::GetMediaDuration
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaPlayer::GetMediaDuration(void)
{
	//NPT_AutoLock lock(m_MediaLoadStateLock);
	if (m_MediaLoadState == MLS_LOADED)
	{
		if (!m_MediaController.IsNull())
		{
			PLT_DeviceDataReference device;
			m_MediaController->GetCurMediaRenderer(device);
			if (!device.IsNull())
			{
				m_MediaController->GetMediaInfo(device, 0, NULL);
			}
		}
	}	
	return NPT_FAILURE;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::GetCurPlayPos
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaPlayer::GetCurPlayPos(void)
{
	//NPT_AutoLock lock(m_MediaLoadStateLock);
	if (m_MediaLoadState == MLS_LOADED)
	{
		if (!m_MediaController.IsNull())
		{
			PLT_DeviceDataReference device;
			m_MediaController->GetCurMediaRenderer(device);
			if (!device.IsNull())
			{
				return m_MediaController->GetPositionInfo(device, 0, NULL);
			}
		}
	}
	return NPT_FAILURE;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::GetVolume
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaPlayer::GetVolume(void)
{
	if (!m_MediaController.IsNull())
	{
		PLT_DeviceDataReference device;
		m_MediaController->GetCurMediaRenderer(device);
		if (!device.IsNull())
		{
			return m_MediaController->GetVolume(device, 0, "Master", NULL);
		}
	}
	return NPT_FAILURE;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::GetPlayState
+---------------------------------------------------------------------*/
SH_DLNAPlayer_PlayState CSHDLNAMediaPlayer::GetPlayState(void)
{
	NPT_AutoLock lock(m_PlayStateLock);
	return m_PlayState;
}

NPT_Result CSHDLNAMediaPlayer::GetTransportInfo()
{
	NPT_Result result = NPT_SUCCESS;

	if (!m_MediaController.IsNull())
	{
		PLT_DeviceDataReference device;
		m_MediaController->GetCurMediaRenderer(device);
		if (!device.IsNull())
		{
			return m_MediaController->GetTransportInfo(device, 0, NULL);
		}
	}

	return result;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::OpenMedia
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaPlayer::OpenMedia(bool media_info_got)
{
	NPT_Result result = NPT_SUCCESS;

	if (!media_info_got)
	{
		result = NPT_FAILURE;
	}
	else
	{
		if (!m_MediaController.IsNull())
			result = m_MediaController->OpenMedia(m_MediaInfo);

		CSHDLNAMediaServer* pMediaServer = dynamic_cast<CSHDLNAMediaServer*>(m_MediaServer.AsPointer());
		if (pMediaServer != NULL)
		{
			pMediaServer->OpenMedia(m_MediaInfo);
		}
	}

	if (NPT_FAILED(result))
	{
		m_MediaLoadState = MLS_CLOSED;
		if (m_MessageNotifyUI != NULL)
		{
			m_MessageNotifyUI(SH_DLNAPLAYER_UI_MESSAGE_OPEN_MEDIA_FAILED, NULL, NULL);
		}
	}

	return result;
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
void CSHDLNAMediaPlayer::OnCurrentDeviceDisconnect()
{
	Close();
	if (m_MessageNotifyUI != NULL)
	{
		m_MessageNotifyUI(SH_DLNAPLAYER_UI_MESSAGE_CURRENT_DEVICE_DISCONNECT, NULL, NULL);
	}
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
void CSHDLNAMediaPlayer::OnNoDeviceChoosen()
{
	if (m_MessageNotifyUI != NULL)
	{
		m_MessageNotifyUI(SH_DLNAPLAYER_UI_MESSAGE_NO_DEVICE_CHOOSEN, NULL, NULL);
	}
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
void CSHDLNAMediaPlayer::OnDeviceListUpdated(void* param)
{
	if (m_MessageNotifyUI != NULL)
	{
		m_MessageNotifyUI(SH_DLNAPLAYER_UI_MESSAGE_DEVICE_LIST_UPDATED, param, NULL);
	}
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::OnOpenResult
+---------------------------------------------------------------------*/
void CSHDLNAMediaPlayer::OnOpenResult(NPT_Result result)
{
	if (NPT_SUCCEEDED(result))
	{
		m_MediaLoadState = MLS_LOADED;
		if (m_MessageNotifyUI != NULL)
		{
			m_MessageNotifyUI(SH_DLNAPLAYER_UI_MESSAGE_OPEN_MEDIA_SUCCEEDED, NULL, NULL);
		}
	}
	else
	{
		m_MediaLoadState = MLS_CLOSED;
		if (m_MessageNotifyUI != NULL)
		{
			m_MessageNotifyUI(SH_DLNAPLAYER_UI_MESSAGE_OPEN_MEDIA_FAILED, NULL, NULL);
		}
	}
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
void CSHDLNAMediaPlayer::OnPlayResult(NPT_Result result)
{
	NPT_AutoLock lock(m_PlayStateLock);
	if (NPT_SUCCEEDED(result))
	{
		m_PlayState = SH_DLNAPLAYER_PLAY_STATE_PLAYING;
	}
	else
	{
		m_PlayState = SH_DLNAPLAYER_PLAY_STATE_INVALID;
	}
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
void CSHDLNAMediaPlayer::OnPauseResult(NPT_Result result)
{
	NPT_AutoLock lock(m_PlayStateLock);
	if (NPT_SUCCEEDED(result))
	{
		m_PlayState = SH_DLNAPLAYER_PLAY_STATE_PAUSE;
	}
	else
	{
		m_PlayState = SH_DLNAPLAYER_PLAY_STATE_INVALID;
	}
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
void CSHDLNAMediaPlayer::OnStopResult(NPT_Result result)
{
	NPT_AutoLock lock(m_PlayStateLock);
	if (NPT_SUCCEEDED(result))
	{
		m_PlayState = SH_DLNAPLAYER_PLAY_STATE_STOP;
	}
	else
	{
		m_PlayState = SH_DLNAPLAYER_PLAY_STATE_INVALID;
	}
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
void CSHDLNAMediaPlayer::OnSeekResult(NPT_Result result)
{

}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
void CSHDLNAMediaPlayer::OnSetVolumeResult(NPT_Result result)
{
	
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::OnGetMediaDurationResult
+---------------------------------------------------------------------*/
void CSHDLNAMediaPlayer::OnGetMediaDurationResult(NPT_Result result, void* param)
{
	if (NPT_SUCCEEDED(result) && m_MessageNotifyUI != NULL)
	{
		if (m_MediaInfo)
		{
			m_MediaInfo->duration = (long)param;
		}
		m_MessageNotifyUI(SH_DLNAPLAYER_UI_MESSAGE_MEDIA_TOTAL_DURATION, param, NULL);
	}
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::OnGetCurPlayPosResult
+---------------------------------------------------------------------*/
void CSHDLNAMediaPlayer::OnGetCurPlayPosResult(NPT_Result result, void* param)
{
	if (NPT_SUCCEEDED(result) && m_MessageNotifyUI != NULL)
	{
		m_MessageNotifyUI(SH_DLNAPLAYER_UI_MESSAGE_MEDIA_CURRENT_POS, param, NULL);
	}
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::OnGetVolumeResult
+---------------------------------------------------------------------*/
void CSHDLNAMediaPlayer::OnGetVolumeResult(NPT_Result result, void* param)
{
	if (NPT_SUCCEEDED(result) && m_MessageNotifyUI != NULL)
	{
		m_MessageNotifyUI(SH_DLNAPLAYER_UI_MESSAGE_DEVICE_CURRENT_VOLUME, param, NULL);
	}
}

void CSHDLNAMediaPlayer::OnGetTransportInfoResult(NPT_Result result, void* param)
{
	if (NPT_SUCCEEDED(result) && m_MessageNotifyUI != NULL)
	{
		PLT_TransportInfo* transport_info = (PLT_TransportInfo*)param;
		if (transport_info != NULL)
		{
			int transport_state = SH_DLNAPLAYER_PLAY_STATE_INVALID;
			if (transport_info->cur_transport_state == "PLAYING")
			{
				transport_state = SH_DLNAPLAYER_PLAY_STATE_PLAYING;
			}
			else if (transport_info->cur_transport_state == "")
			{

			}
			m_MessageNotifyUI(SH_DLNAPLAYER_UI_MESSAGE_TRANSPORT_INFO, (void*)transport_state, NULL);
		}
	}
}