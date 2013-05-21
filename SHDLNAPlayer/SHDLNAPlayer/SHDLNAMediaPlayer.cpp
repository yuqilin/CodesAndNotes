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
, m_PlayState(PS_INVALID)
, m_MediaInfo(NULL)
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
	Close();

	if (!m_UPnP.IsNull())
		m_UPnP->Stop();

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
	if (m_MediaLoadState == MLS_LOADED ||
		m_MediaLoadState == MLS_LOADING)
	{
		Close();
	}

	m_MediaLoadState = MLS_LOADING;

	m_MediaInfo = new SHDLNAMediaInfo_t;
	
	NPT_String strUrl(url);
	if (strUrl.StartsWith("http://"))
	{
		m_TaskManager.StartTask(new CSHDLNAMediaDownloader(url, this));
	}
	else
	{
		m_MediaInfo->url = url;
		m_MediaInfo->title = NPT_FilePath::BaseName(url);
		OpenMedia(true);
	}

	return result;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::Close
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaPlayer::Close(void)
{
	//NPT_AutoLock(m_MediaLoadStateLock);

	if (m_MediaLoadState == MLS_LOADED || m_MediaLoadState == MLS_LOADING)
	{
		m_MediaLoadState = MLS_CLOSING;
		Stop();

		m_TaskManager.StopAllTasks();
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

	{
		//NPT_AutoLock lock(m_PlayStateLock);
		if (m_PlayState == PS_PLAY)
		{
			return NPT_SUCCESS;
		}
	}
	
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
	{
		//NPT_AutoLock lock(m_MediaLoadStateLock);
		if (m_MediaLoadState != MLS_LOADED)
		{
			return NPT_FAILURE;
		}
	}

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
	{
		//NPT_AutoLock lock(m_MediaLoadStateLock);
		if (m_MediaLoadState != MLS_LOADED)
		{
			return NPT_FAILURE;
		}
	}
	{
		//NPT_AutoLock lock(m_PlayState);
		if (m_PlayState != PS_PLAY)
			return NPT_SUCCESS;
	}	

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
	{
		//NPT_AutoLock lock(m_MediaLoadStateLock);
		if (m_MediaLoadState != MLS_LOADED)
		{
			return NPT_FAILURE;
		}
	}

	{
		//NPT_AutoLock lock(m_PlayStateLock);

		if (m_PlayState == PS_INVALID || m_PlayState == PS_STOP)
			return NPT_SUCCESS;
	}

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
				return m_MediaController->GetMediaInfo(device, 0, NULL);
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
|   CSHDLNAMediaPlayer::GetInputStream
+---------------------------------------------------------------------*/
// NPT_Result CSHDLNAMediaPlayer::GetInputStream(NPT_InputStreamReference& stream)
// {
// 	stream = m_InputStream;
// 	return NPT_SUCCESS;
// }


/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::ParseMediaTitle
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaPlayer::ParseMediaTitle(const char* url_from_ui_utf8, NPT_String& title)
{
	NPT_Result result = NPT_SUCCESS;

	NPT_String strUri(url_from_ui_utf8);
	if (strUri.StartsWith("http://"))
	{
		CMp4DownloadClient client;
		CString filename;
		client.GetFileName(url_from_ui_utf8, filename);
		title = wcs2mbs(CP_UTF8, filename).c_str();
	}
	else if (NPT_File::Exists(url_from_ui_utf8))
	{
		title = NPT_FilePath::BaseName(url_from_ui_utf8);
	}
	else
	{
		result = NPT_ERROR_NO_SUCH_ITEM;
	}

	return result;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::OnMediaHeaderDownloadCompleted
+---------------------------------------------------------------------*/
// NPT_Result CSHDLNAMediaPlayer::OnMediaHeaderDownloadCompleted(SHDLNAMediaInfo_t* media_info)
// {
// 	NPT_Result result = NPT_SUCCESS;
// 
// 	result = OpenMedia(media_info);
// 
// 	return result;
// }

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
		if (m_MessageNotifyUI != NULL)
		{
			m_MessageNotifyUI(SH_DLNAPLAYER_UI_MESSAGE_OPEN_MEDIA_FAILED, NULL, NULL);
		}
		m_MediaLoadState = MLS_CLOSED;
	}

	return result;
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
		if (m_MessageNotifyUI != NULL)
		{
			m_MessageNotifyUI(SH_DLNAPLAYER_UI_MESSAGE_OPEN_MEDIA_SUCCEEDED, NULL, NULL);
		}
		m_MediaLoadState = MLS_LOADED;
	}
	else
	{
		if (m_MessageNotifyUI != NULL)
		{
			m_MessageNotifyUI(SH_DLNAPLAYER_UI_MESSAGE_OPEN_MEDIA_FAILED, NULL, NULL);
		}
		m_MediaLoadState = MLS_CLOSED;
	}
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::OnGetMediaDurationResult
+---------------------------------------------------------------------*/
void CSHDLNAMediaPlayer::OnGetMediaDurationResult(NPT_Result result, void* param)
{
	if (NPT_SUCCEEDED(result) && m_MessageNotifyUI != NULL)
	{
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
