#include "stdafx.h"
#include "SHDLNAMediaPlayer.h"
#include "SHDLNAMediaServer.h"
#include "SHDLNAMediaDownloader.h"
#include "Mp4DownloadClient.h"

NPT_SET_LOCAL_LOGGER("shdlnaplayer.shdlnamediaplayer")

#define SH_DLNAPLAYER_DEFAULT_VOLUME		10

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::CSHDLNAMediaPlayer
+---------------------------------------------------------------------*/
CSHDLNAMediaPlayer::CSHDLNAMediaPlayer()
: m_MessageNotifyUI(NULL)
, m_UPnP(NULL)
, m_MediaServer(NULL)
//, m_MediaLoadState(MLS_CLOSED)
, m_PlayState(SH_DLNAPLAYER_PLAY_STATE_INVALID)
, m_MediaInfo(NULL)
, m_MediaDownloader(NULL)
{
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

	Close();

	//m_MediaLoadState = MLS_LOADING;

	m_MediaInfoLock.Lock();
	m_MediaInfo = new SHDLNAMediaInfo_t;
	m_MediaInfo->url = url;
	m_MediaInfoLock.Unlock();

	NPT_String strUrl(url);
	if (strUrl.StartsWith("http://"))
	{
		m_MediaDownloader = new CSHDLNAMediaDownloader(url, this);
		if (m_MediaDownloader != NULL)
		{
			m_MediaDownloader->Start();
		}
	}
	else
	{		
		m_MediaInfoLock.Lock();
		m_MediaInfo->title = NPT_FilePath::BaseName(strUrl);
		m_MediaInfoLock.Unlock();

		std::wstring wstrUrl = mbs2wcs(CP_UTF8, url);

		//OpenMedia(true);
		this->OnOpenResult(PushUrlToDevice());
	}

	return result;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::Close
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaPlayer::Close(void)
{
	//m_MediaLoadState = MLS_CLOSING;
	Stop();

	//m_TaskManager.StopAllTasks();
	if (m_MediaDownloader != NULL)
	{
		m_MediaDownloader->Interrupt();

		delete m_MediaDownloader;
		m_MediaDownloader = NULL;
	}
	
	{
		NPT_AutoLock lock(m_MediaInfoLock);
		delete m_MediaInfo;
		m_MediaInfo	= NULL;
	}
	
	CSHDLNAMediaServer* pMediaServer = dynamic_cast<CSHDLNAMediaServer*>(m_MediaServer.AsPointer());
	if (pMediaServer != NULL)
	{
		pMediaServer->CloseMedia();
	}

	//m_MediaLoadState = MLS_CLOSED;

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
	if (!m_MediaController.IsNull())
	{
		PLT_DeviceDataReference device;
		m_MediaController->GetCurMediaRenderer(device);
		if (!device.IsNull())
		{
			return m_MediaController->Play(device, 0, "1", NULL);
		}
	}
	return NPT_FAILURE;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::Seek
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaPlayer::Seek(long pos_to_play)
{
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
	return NPT_FAILURE;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::Pause
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaPlayer::Pause()
{
	if (!m_MediaController.IsNull())
	{
		PLT_DeviceDataReference device;
		m_MediaController->GetCurMediaRenderer(device);
		if (!device.IsNull())
		{
			return m_MediaController->Pause(device, 0, NULL);
		}
	}
	return NPT_FAILURE;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::Stop
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaPlayer::Stop()
{
	if (!m_MediaController.IsNull())
	{
		PLT_DeviceDataReference device;
		m_MediaController->GetCurMediaRenderer(device);
		if (!device.IsNull())
		{
			return m_MediaController->Stop(device, 0, NULL);
		}
	}
	return NPT_FAILURE;
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
	return NPT_FAILURE;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::GetMediaDuration
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaPlayer::GetMediaDuration(long* duration)
{
	NPT_CHECK_POINTER(duration);

	*duration = 0;
	
	NPT_AutoLock lock(m_MediaInfoLock);
	if (m_MediaInfo)
	{
		if (m_MediaInfo->duration != 0)
		{
			*duration = m_MediaInfo->duration;
			return NPT_SUCCESS;
		}
		
		if (!m_MediaController.IsNull())
		{
			PLT_DeviceDataReference device;
			m_MediaController->GetCurMediaRenderer(device);
			if (!device.IsNull())
			{
				return m_MediaController->GetMediaInfo(device, 0, NULL);
			}
		}
	}
	
	return NPT_FAILURE;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::GetCurPlayPos
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaPlayer::GetCurPlayPos(long* cur_play_pos)
{
	NPT_CHECK_POINTER(cur_play_pos);

	*cur_play_pos = 0;

	{
		NPT_AutoLock lock(m_MediaInfoLock);
		if (m_MediaInfo)
		{
			*cur_play_pos = m_MediaInfo->play_pos;
		}
	}

	if (!m_MediaController.IsNull())
	{
		PLT_DeviceDataReference device;
		m_MediaController->GetCurMediaRenderer(device);
		if (!device.IsNull())
		{
			return m_MediaController->GetPositionInfo(device, 0, NULL);
		}
	}

	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::GetVolume
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaPlayer::GetVolume(int* volume)
{
	NPT_CHECK_POINTER(volume);

	*volume = 0;

	if (!m_MediaController.IsNull())
	{
		PLT_DeviceDataReference device;
		m_MediaController->GetCurMediaRenderer(device);
		if (!device.IsNull())
		{
			NPT_UInt32 volume_state;
			if (NPT_SUCCEEDED(m_MediaController->GetVolumeState(device->GetUUID(), volume_state)))
			{
				*volume = (int)volume_state;
				return NPT_SUCCESS;
			}
		}
	}
	return NPT_FAILURE;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::GetPlayState
+---------------------------------------------------------------------*/
SH_DLNAPlayer_PlayState CSHDLNAMediaPlayer::GetPlayState(void)
{
	//SH_DLNAPlayer_PlayState play_state = SH_DLNAPLAYER_PLAY_STATE_INVALID;

	NPT_AutoLock lock(m_PlayStateLock);
	m_PlayState = SH_DLNAPLAYER_PLAY_STATE_INVALID;

	if (!m_MediaController.IsNull())
	{
		PLT_DeviceDataReference device;
		m_MediaController->GetCurMediaRenderer(device);
		if (!device.IsNull())
		{
			NPT_String state;
			if (NPT_SUCCEEDED(m_MediaController->GetTransportState(device->GetUUID(), state)))
			{				
				m_PlayState = TransportStatetoPlayState(state);
			}
		}
	}
	return m_PlayState;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::GetTransportInfo
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaPlayer::GetTransportInfo()
{
	NPT_Result result = NPT_FAILURE;

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
// NPT_Result CSHDLNAMediaPlayer::OpenMedia(bool media_info_got)
// {
// 	NPT_Result result = NPT_SUCCESS;
// 
// 	if (!media_info_got)
// 	{
// 		result = NPT_FAILURE;
// 	}
// 	else
// 	{
// 		if (!m_MediaController.IsNull())
// 			result = m_MediaController->OpenMedia(m_MediaInfo);
// 
// 		CSHDLNAMediaServer* pMediaServer = dynamic_cast<CSHDLNAMediaServer*>(m_MediaServer.AsPointer());
// 		if (pMediaServer != NULL)
// 		{
// 			pMediaServer->OpenMedia(m_MediaInfo);
// 		}
// 	}
// 
// 	if (NPT_FAILED(result))
// 	{
// 		//m_MediaLoadState = MLS_CLOSED;
// 		if (m_MessageNotifyUI != NULL)
// 		{
// 			m_MessageNotifyUI(SH_DLNAPLAYER_UI_MESSAGE_OPEN_MEDIA_FAILED, NULL, NULL);
// 		}
// 	}
// 
// 	return result;
// }

NPT_Result CSHDLNAMediaPlayer::PushUrlToDevice()
{
	NPT_Result result = NPT_FAILURE;

	if (!m_MediaController.IsNull())
		result = m_MediaController->OpenMedia(m_MediaInfo);

	if (NPT_FAILED(result))
	{
		this->OnOpenResult(result);
	}

	return result;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::OnCurrentDeviceDisconnect
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
|   CSHDLNAMediaPlayer::OnNoDeviceChoosen
+---------------------------------------------------------------------*/
void CSHDLNAMediaPlayer::OnNoDeviceChoosen()
{
	if (m_MessageNotifyUI != NULL)
	{
		m_MessageNotifyUI(SH_DLNAPLAYER_UI_MESSAGE_NO_DEVICE_CHOOSEN, NULL, NULL);
	}
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::OnDeviceListUpdated
+---------------------------------------------------------------------*/
void CSHDLNAMediaPlayer::OnDeviceListUpdated(void* param)
{
	if (m_MessageNotifyUI != NULL)
	{
		m_MessageNotifyUI(SH_DLNAPLAYER_UI_MESSAGE_DEVICE_LIST_UPDATED, param, NULL);
	}
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
void CSHDLNAMediaPlayer::OnSetAVTransportURIResult(NPT_Result result)
{
	if (NPT_SUCCEEDED(result))
	{
		
	}
	else
	{
		OnOpenResult(result);
	}
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::OnOpenResult
+---------------------------------------------------------------------*/
void CSHDLNAMediaPlayer::OnOpenResult(NPT_Result result)
{
	if (NPT_SUCCEEDED(result))
	{
		CSHDLNAMediaServer* pMediaServer = dynamic_cast<CSHDLNAMediaServer*>(m_MediaServer.AsPointer());
		if (pMediaServer != NULL)
		{
			pMediaServer->OpenMedia(m_MediaInfo);
		}

		//m_MediaLoadState = MLS_LOADED;
		if (m_MessageNotifyUI != NULL)
		{
			m_MessageNotifyUI(SH_DLNAPLAYER_UI_MESSAGE_OPEN_MEDIA_SUCCEEDED, NULL, NULL);
		}
	}
	else
	{
		//m_MediaLoadState = MLS_CLOSED;
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
		NPT_AutoLock lock(m_MediaInfoLock);
		if (m_MediaInfo)
		{
			m_MediaInfo->duration = (long)param;
		}
	}
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::OnGetCurPlayPosResult
+---------------------------------------------------------------------*/
void CSHDLNAMediaPlayer::OnGetCurPlayPosResult(NPT_Result result, void* param)
{
	if (NPT_SUCCEEDED(result) && m_MessageNotifyUI != NULL)
	{
		NPT_AutoLock lock(m_MediaInfoLock);
		if (m_MediaInfo)
		{
			m_MediaInfo->play_pos = (long)param;
		}
	}
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::OnGetVolumeResult
+---------------------------------------------------------------------*/
void CSHDLNAMediaPlayer::OnGetVolumeResult(NPT_Result result, void* param)
{
	if (NPT_SUCCEEDED(result) && m_MessageNotifyUI != NULL)
	{
	}
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::OnGetTransportInfoResult
+---------------------------------------------------------------------*/
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
			//m_MessageNotifyUI(SH_DLNAPLAYER_UI_MESSAGE_TRANSPORT_INFO, (void*)transport_state, NULL);
		}
	}
}

void CSHDLNAMediaPlayer::OnVolumeStateChanged(const NPT_String& volume_state)
{
	if (m_PlayState == SH_DLNAPLAYER_PLAY_STATE_INVALID)
		return;
	if (m_MessageNotifyUI)
	{
		int volume = 0;
		volume_state.ToInteger(volume);
		m_MessageNotifyUI(SH_DLNAPLAYER_UI_MESSAGE_DEVICE_VOLUME_CHANGED, (void*)volume, NULL);
	}
}

void CSHDLNAMediaPlayer::OnTransportStateChanged(const NPT_String& transport_state)
{
	NPT_AutoLock lock(m_PlayStateLock);
	m_PlayState = TransportStatetoPlayState(transport_state);
}


SH_DLNAPlayer_PlayState	CSHDLNAMediaPlayer::TransportStatetoPlayState(const NPT_String& transport_state)
{
	SH_DLNAPlayer_PlayState play_state = SH_DLNAPLAYER_PLAY_STATE_INVALID;

	if (transport_state == "PLAYING" ||
		transport_state == "TRANSITIONING" ||
		transport_state == "RECORDING")
	{
		play_state = SH_DLNAPLAYER_PLAY_STATE_PLAYING;
	}
	else if (transport_state == "PAUSED_PLAYBACK" ||
		transport_state == "PAUSED_RECORDING")
	{
		play_state = SH_DLNAPLAYER_PLAY_STATE_PAUSE;
	}
	else if (transport_state == "STOPPED")
	{
		play_state = SH_DLNAPLAYER_PLAY_STATE_STOP;
	}
	else if (transport_state == "NO_MEDIA_PRESENT")
	{
		
	}
	else
	{
		NPT_LOG_WARNING_1("Unknown TransportState : %s", transport_state);
	}

	return play_state;
}