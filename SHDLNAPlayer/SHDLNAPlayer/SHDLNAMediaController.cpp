#include "stdafx.h"
#include "SHDLNAMediaController.h"

NPT_SET_LOCAL_LOGGER("shdlnaplayer.shdlnamediacontroller")

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::CSHDLNAMediaController
+---------------------------------------------------------------------*/
CSHDLNAMediaController::CSHDLNAMediaController(PLT_CtrlPointReference& ctrlPoint,
											   SH_DLNAPlayer_MessageNotifyUI message_to_notify)
											   : PLT_MediaController(ctrlPoint, this)
											   , m_MessageNotifyUI(message_to_notify)
{
	memset(&m_DeviceList, 0, sizeof(m_DeviceList));
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::~CSHDLNAMediaController
+---------------------------------------------------------------------*/
CSHDLNAMediaController::~CSHDLNAMediaController()
{
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::ChooseDevice
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaController::ChooseDevice(const char* device_uuid)
{
	NPT_AutoLock lock(m_CurMediaRendererLock);

	NPT_List<PLT_DeviceMapEntry*>::Iterator entry =
		m_MediaRenderers.GetEntries().GetFirstItem();
	while (entry)
	{
		NPT_String entry_uuid = (*entry)->GetKey();
		if (entry_uuid.Compare(device_uuid, true) == 0)
		{
			PLT_DeviceDataReference* renderer = NULL;
			NPT_Result result = m_MediaRenderers.Get((*entry)->GetKey(), renderer);
			if (NPT_SUCCEEDED(result))
				m_CurMediaRenderer = *renderer;
			return result;
		}
		++entry;
	}
	return NPT_ERROR_NO_SUCH_ITEM;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::OpenMedia
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaController::OpenMedia(SHDLNAMediaInfo_t& media_info)
{
	NPT_Result result = NPT_FAILURE;

	NPT_String didl;
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

	ToDidl(media_info.url_to_device, media_info.title, didl);

	result = SetAVTransportURI(m_CurMediaRenderer, 0, media_info.url_to_device, didl, NULL);

	return result;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::OnMRAdded
+---------------------------------------------------------------------*/
bool CSHDLNAMediaController::OnMRAdded(PLT_DeviceDataReference& device)
{
	NPT_String device_uuid = device->GetUUID();
	PLT_Service* service;
	if (NPT_SUCCEEDED(device->FindServiceByType("urn:schemas-upnp-org:service:AVTransport:*", service)))
	{
		NPT_AutoLock lock(m_MediaRenderers);
		m_MediaRenderers.Put(device_uuid, device);

		OnDeviceListUpdated();
	}
	return true;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::OnMRRemoved
+---------------------------------------------------------------------*/
void CSHDLNAMediaController::OnMRRemoved(PLT_DeviceDataReference& device)
{
	NPT_String device_uuid = device->GetUUID();

	{
		NPT_AutoLock lock(m_MediaRenderers);
		m_MediaRenderers.Erase(device_uuid);

		OnDeviceListUpdated();
	}

	{
		NPT_AutoLock lock(m_CurMediaRendererLock);

		// if it's the currently selected one, we have to get rid of it
		if (!m_CurMediaRenderer.IsNull() && m_CurMediaRenderer == device) {
			m_CurMediaRenderer = NULL;
		}
	}
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::OnDeviceListUpdated
+---------------------------------------------------------------------*/
void CSHDLNAMediaController::OnDeviceListUpdated()
{
	memset(&m_DeviceList, 0, sizeof(m_DeviceList));

	NPT_List<PLT_DeviceMapEntry*>::Iterator entry = m_MediaRenderers.GetEntries().GetFirstItem();
	while (entry)
	{
		NPT_String device_uuid = (*entry)->GetKey();
		NPT_String device_name = (*entry)->GetValue()->GetFriendlyName();

		if (m_DeviceList.count < SH_DLNAPLAYER_DEVICE_COUNT_MAX)
		{
			strcpy_s(m_DeviceList.device[m_DeviceList.count].device_uuid, 
				SH_DLNAPLAYER_DEVICE_UUID_LENGTH_MAX, device_uuid.GetChars());
			strcpy_s(m_DeviceList.device[m_DeviceList.count].device_name, 
				SH_DLNAPLAYER_DEVICE_NAME_LENGTH_MAX, device_name.GetChars());
			++m_DeviceList.count;
		}
		else
			break;

		++entry;
	}

	if (m_MessageNotifyUI != NULL)
	{
		m_MessageNotifyUI(SH_DLNAPLAYER_UI_MESSAGE_DEVICE_LIST_UPDATED, (void*)&m_DeviceList, NULL);
	}
}


/*----------------------------------------------------------------------
|   CSHDLNAMediaController::OnGetCurrentTransportActionsResult
+---------------------------------------------------------------------*/
void CSHDLNAMediaController::OnGetCurrentTransportActionsResult(
	NPT_Result               res, 
	PLT_DeviceDataReference& device,
	PLT_StringList*          actions, 
	void*                    userdata)
{

}

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::OnGetDeviceCapabilitiesResult
+---------------------------------------------------------------------*/
void CSHDLNAMediaController::OnGetDeviceCapabilitiesResult(
	NPT_Result               res, 
	PLT_DeviceDataReference& device,
	PLT_DeviceCapabilities*  capabilities,
	void*                    userdata)
{

}

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::OnGetMediaInfoResult
+---------------------------------------------------------------------*/
void CSHDLNAMediaController::OnGetMediaInfoResult(
	NPT_Result                res,
	PLT_DeviceDataReference&  device,
	PLT_MediaInfo*            info,
	void*                     userdata)
{
	if (NPT_SUCCESS == res)
	{
		if (info != NULL)
		{
			//m_MediaInfo = *info;
			if (m_MessageNotifyUI != NULL)
			{
				long millis = info->media_duration.ToMillis();
				m_MessageNotifyUI(SH_DLNAPLAYER_UI_MESSAGE_MEDIA_TOTAL_DURATION, (void*)millis, 0);
			}
		}
	}
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::OnGetPositionInfoResult
+---------------------------------------------------------------------*/
void CSHDLNAMediaController::OnGetPositionInfoResult(
	NPT_Result               res,
	PLT_DeviceDataReference& device,
	PLT_PositionInfo*        info,
	void*                    userdata)
{
	if (NPT_SUCCESS == res)
	{
		if (info != NULL)
		{
			//m_PositionInfo = *info;
			if (m_MessageNotifyUI != NULL)
			{
				long pos = info->rel_time.ToMillis();
				m_MessageNotifyUI(SH_DLNAPLAYER_UI_MESSAGE_MEDIA_CURRENT_POS, (void*)pos, 0);
			}
		}
	}
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::OnGetTransportInfoResult
+---------------------------------------------------------------------*/
void CSHDLNAMediaController::OnGetTransportInfoResult(
	NPT_Result               res,
	PLT_DeviceDataReference& device,
	PLT_TransportInfo*       info,
	void*                    userdata)
{

}

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::OnGetTransportSettingsResult
+---------------------------------------------------------------------*/
void CSHDLNAMediaController::OnGetTransportSettingsResult(
	NPT_Result               res,
	PLT_DeviceDataReference& device,
	PLT_TransportSettings*   settings,
	void*                    userdata)
{

}

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::OnNextResult
+---------------------------------------------------------------------*/
void CSHDLNAMediaController::OnNextResult(
									 NPT_Result               res,
									 PLT_DeviceDataReference& device,
									 void*                    userdata)
{

}

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::OnPauseResult
+---------------------------------------------------------------------*/
void CSHDLNAMediaController::OnPauseResult(
									  NPT_Result               res,
									  PLT_DeviceDataReference& device,
									  void*                    userdata)
{
	NPT_CHECK_ONRESULT_SEVERE(res);
}  

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::OnPlayResult
+---------------------------------------------------------------------*/
void CSHDLNAMediaController::OnPlayResult(
									 NPT_Result               res,
									 PLT_DeviceDataReference& device,
									 void*                    userdata)
{
	NPT_CHECK_ONRESULT_SEVERE(res);
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::OnPreviousResult
+---------------------------------------------------------------------*/
void CSHDLNAMediaController::OnPreviousResult(
	NPT_Result               res,
	PLT_DeviceDataReference& device,
	void*                    userdata)
{

}

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::OnSeekResult
+---------------------------------------------------------------------*/
void CSHDLNAMediaController::OnSeekResult(
									 NPT_Result               res,
									 PLT_DeviceDataReference& device,
									 void*                    userdata)
{
	NPT_CHECK_ONRESULT_SEVERE(res);
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::OnSetAVTransportURIResult
+---------------------------------------------------------------------*/
void CSHDLNAMediaController::OnSetAVTransportURIResult(
	NPT_Result               res,
	PLT_DeviceDataReference& device,
	void*                    userdata)
{
	NPT_CHECK_ONRESULT_SEVERE(res);
	SH_DLNAPlayer_UI_Message msg = (NPT_SUCCEEDED(res) ? SH_DLNAPLAYER_UI_MESSAGE_OPEN_MEDIA_SUCCEEDED
		: SH_DLNAPLAYER_UI_MESSAGE_OPEN_MEDIA_FAILED);
	if (m_MessageNotifyUI != NULL)
	{
		m_MessageNotifyUI(msg, NULL, NULL);
	}
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::OnSetPlayModeResult
+---------------------------------------------------------------------*/
void CSHDLNAMediaController::OnSetPlayModeResult(
	NPT_Result               res,
	PLT_DeviceDataReference& device,
	void*                    userdata)
{

}

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::OnStopResult
+---------------------------------------------------------------------*/
void CSHDLNAMediaController::OnStopResult(
									 NPT_Result               res,
									 PLT_DeviceDataReference& device,
									 void*                    userdata)
{
	NPT_CHECK_ONRESULT_SEVERE(res);
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::OnGetCurrentConnectionIDsResult
+---------------------------------------------------------------------*/
void CSHDLNAMediaController::OnGetCurrentConnectionIDsResult(
	NPT_Result               res,
	PLT_DeviceDataReference& device,
	PLT_StringList*          ids,
	void*                    userdata)
{

}

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::OnGetCurrentConnectionInfoResult
+---------------------------------------------------------------------*/
void CSHDLNAMediaController::OnGetCurrentConnectionInfoResult(
	NPT_Result               res,
	PLT_DeviceDataReference& device,
	PLT_ConnectionInfo*      info,
	void*                    userdata)
{

}

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::OnGetProtocolInfoResult
+---------------------------------------------------------------------*/
void CSHDLNAMediaController::OnGetProtocolInfoResult(
	NPT_Result               res,
	PLT_DeviceDataReference& device,
	PLT_StringList*          sources,
	PLT_StringList*          sinks,
	void*                    userdata)
{
	NPT_CHECK_ONRESULT_SEVERE(res);
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::OnSetMuteResult
+---------------------------------------------------------------------*/
void CSHDLNAMediaController::OnSetMuteResult(
										NPT_Result               res,
										PLT_DeviceDataReference& device,
										void*                    userdata)
{

}

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::OnGetMuteResult
+---------------------------------------------------------------------*/
void CSHDLNAMediaController::OnGetMuteResult(
										NPT_Result               res,
										PLT_DeviceDataReference& device,
										const char*              channel,
										bool                     mute,
										void*                    userdata)
{

}

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::OnSetVolumeResult
+---------------------------------------------------------------------*/
void CSHDLNAMediaController::OnSetVolumeResult(
	NPT_Result               res,
	PLT_DeviceDataReference& device,
	void*                    userdata)
{
	NPT_CHECK_ONRESULT_SEVERE(res);
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::OnGetVolumeResult
+---------------------------------------------------------------------*/
void CSHDLNAMediaController::OnGetVolumeResult(
	NPT_Result					res,
	PLT_DeviceDataReference&	device,
	const char*					channel,
	NPT_UInt32					volume,
	void*						userdata)
{
	if (NPT_SUCCESS == res)
	{
		if (m_MessageNotifyUI != NULL)
		{
			m_MessageNotifyUI(SH_DLNAPLAYER_UI_MESSAGE_DEVICE_CURRENT_VOLUME, 
				(void*)volume, NULL);
		}
	}
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::ToDidl
+---------------------------------------------------------------------*/
void CSHDLNAMediaController::ToDidl(const NPT_String& uri, const NPT_String& name,NPT_String& didl)
{
	didl=didl_header;
	didl += "<item id=\"";
	PLT_Didl::AppendXmlEscape(didl, name);
	didl += "\" parentID=\"0\" restricted=\"1\">";

	int start=name.Find(".");
	NPT_String title=name.Left(start);


	didl += "<dc:title>";
	PLT_Didl::AppendXmlEscape(didl, title);
	didl += "</dc:title>";

	didl +="<dc:creator>SohuTV</dc:creator><upnp:genre>SohuTV</upnp:genre>";

	didl += "<res size=\"\"";

	didl += " protocolInfo=\"";
	const char* mine_type = PLT_MimeType::GetMimeType(name, NULL);
	PLT_ProtocolInfo protocolInfo = PLT_ProtocolInfo::GetProtocolInfoFromMimeType(mine_type, true, NULL);
	PLT_Didl::AppendXmlEscape(didl, protocolInfo.ToString());
	didl += "\">";
	PLT_Didl::AppendXmlEscape(didl, uri);
	didl += "</res>";

	didl+="<upnp:class>object.item.videoItem</upnp:class>";

	didl += "</item>";

	didl+=didl_footer;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::GetCurMediaRenderer
+---------------------------------------------------------------------*/
void CSHDLNAMediaController::GetCurMediaRenderer(PLT_DeviceDataReference& renderer)
{
	NPT_AutoLock lock(m_CurMediaRendererLock);

	if (m_CurMediaRenderer.IsNull()) {
		//printf("No renderer selected, select one with setmr\n");
	} else {
		renderer = m_CurMediaRenderer;
	}
}
