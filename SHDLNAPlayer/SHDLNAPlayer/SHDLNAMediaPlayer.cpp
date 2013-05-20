#include "stdafx.h"
#include "SHDLNAMediaPlayer.h"
#include "SHDLNAMediaServer.h"
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
{
	memset(&m_MediaInfo, 0, sizeof(m_MediaInfo));	
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

	m_MediaController = new CSHDLNAMediaController(ctrlPoint, message_to_notify);

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
|   CSHDLNAMediaPlayer::Open
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaPlayer::Open(const char* url)
{
	NPT_Result result = NPT_FAILURE;

	MEDIA_LOAD_STATE mls = GetMediaLoadState();
	if (mls == MLS_LOADED ||
		mls == MLS_LOADING)
	{
		Close();
		memset(&m_MediaInfo, 0, sizeof(m_MediaInfo));
	}

	m_TaskManager.AddTask(new CSHDLNAMediaDownloader(url, this));

	if (NPT_SUCCEEDED(BuildMediaInfo(url)))
	{
		if (!m_MediaController.IsNull())
			result = m_MediaController->OpenMedia(m_MediaInfo);

		CSHDLNAMediaServer* pMediaServer = dynamic_cast<CSHDLNAMediaServer*>(m_MediaServer.AsPointer());
		if (pMediaServer != NULL)
		{
			pMediaServer->SetMediaInfo(&m_MediaInfo);
		}
	}

	if (NPT_SUCCEEDED(result))
		m_MediaInfo.opened = true;

	return result;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaPlayer::Close
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaPlayer::Close(void)
{
	if (m_MediaInfo.opened)
	{
		Stop();
	}
	memset(&m_MediaInfo, 0, sizeof(m_MediaInfo));

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
	return NPT_SUCCESS;
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
	return NPT_SUCCESS;
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
	return NPT_SUCCESS;
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
	if (m_MediaInfo.opened)
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
	if (m_MediaInfo.opened)
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
	if (m_MediaInfo.opened)
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
|   
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaPlayer::BuildMediaInfo(const char* url_from_ui_utf8)
{
	NPT_Result result = NPT_FAILURE;

	PLT_DeviceDataReference device;
	NPT_String ip("127.0.0.1");
	int port = 0;

	if (m_MediaController.IsNull() || m_MediaServer.IsNull())
	{
		return NPT_FAILURE;
	}

	m_MediaController->GetCurMediaRenderer(device);
	if (!device.IsNull())
	{
		ip = device->GetLocalIP().ToString();
	}
	port = m_MediaServer->GetPort();

	NPT_String title;
	if (NPT_SUCCEEDED(result = ParseMediaTitle(url_from_ui_utf8, title)))
	{
		m_MediaInfo.url_from_ui = url_from_ui_utf8;
		m_MediaInfo.title = title;

		NPT_HttpUrl base_uri(ip, port, NPT_HttpUrl::PercentEncode("/", NPT_Uri::PathCharsToEncode));
		m_MediaInfo.url_to_device = CSHDLNAMediaServerDelegate::BuildSafeResourceUri(base_uri, ip, title);
	}

	return result;
}

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
NPT_Result CSHDLNAMediaPlayer::OnMediaHeaderDownloadCompleted(SHDLNAMediaInfo_t* media_info)
{
	NPT_Result result = NPT_SUCCESS;

	m_MediaInfo = media_info;

	if (NPT_SUCCEEDED(BuildMediaInfo(url_utf8)))
	{
		if (!m_MediaController.IsNull())
			result = m_MediaController->OpenMedia(m_MediaInfo);

		CSHDLNAMediaServer* pMediaServer = dynamic_cast<CSHDLNAMediaServer*>(m_MediaServer.AsPointer());
		if (pMediaServer != NULL)
		{
			pMediaServer->SetMediaInfo(&m_MediaInfo);
		}
	}

	if (NPT_SUCCEEDED(result))
	{
		SetMediaLoadState(MLS_LOADED);
	}

	return result;
}