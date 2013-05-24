#include "stdafx.h"
#include "SHDLNAMediaController.h"
#include "SHDLNAMediaServer.h"

NPT_SET_LOCAL_LOGGER("shdlnaplayer.shdlnamediacontroller")

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::CSHDLNAMediaController
+---------------------------------------------------------------------*/
CSHDLNAMediaController::CSHDLNAMediaController(PLT_CtrlPointReference& ctrlPoint,
											   /*SH_DLNAPlayer_MessageNotifyUI message_to_notify,*/
											   CSHDLNAMediaPlayer* pPlayer)
											   : PLT_MediaController(ctrlPoint, this)
											   /*, m_MessageNotifyUI(message_to_notify)*/
											   , m_Player(pPlayer)
{
	memset(&m_DeviceList, 0, sizeof(m_DeviceList));
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::~CSHDLNAMediaController
+---------------------------------------------------------------------*/
CSHDLNAMediaController::~CSHDLNAMediaController()
{
	/*m_MessageNotifyUI = NULL;*/
	m_Player = NULL;
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
NPT_Result CSHDLNAMediaController::OpenMedia(SHDLNAMediaInfo_t* media_info)
{
	NPT_Result result = NPT_FAILURE;

	NPT_String ip("127.0.0.1");
	NPT_UInt16 port = 0;
	PLT_DeviceDataReference device;
	GetCurMediaRenderer(device);
	if (!device.IsNull())
	{
		ip = device->GetLocalIP().ToString();
	}
	else
	{
		if (m_Player != NULL)
		{
			m_Player->OnNoDeviceChoosen();
		}
		return NPT_FAILURE;
	}

	if (!m_CurMediaServer.IsNull())
	{
		port = m_CurMediaServer->GetPort();
	}

	NPT_String title;
	if (media_info != NULL)
		title = media_info->title;
	else
		title = wcs2mbs(CP_UTF8, mbs2wcs(CP_ACP, "หับüสำฦต").c_str()).c_str();

	NPT_HttpUrl base_uri(ip, port, NPT_HttpUrl::PercentEncode("/", NPT_Uri::PathCharsToEncode));
	NPT_String url_to_device = CSHDLNAMediaServerDelegate::BuildSafeResourceUri(base_uri, ip, title);

	NPT_LOG_INFO_1("Push Url to device: %s", url_to_device);

	NPT_String didl;
	ToDidl(url_to_device, title, didl);

	result = SetAVTransportURI(m_CurMediaRenderer, 0, url_to_device, didl, NULL);

	if (NPT_FAILED(result))
	{
		if (m_Player != NULL)
		{
			m_Player->OnOpenResult(result);
		}
	}

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
			if (m_Player != NULL)
			{
				m_Player->OnCurrentDeviceDisconnect();
			}
		}
	}
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaController::OnMRStateVariablesChanged
+---------------------------------------------------------------------*/
void CSHDLNAMediaController::OnMRStateVariablesChanged(PLT_Service* service,
													   NPT_List<PLT_StateVariable*>* vars)
{
	NPT_String strVolume;
	if (service)
	{
		PLT_StateVariable* var = service->FindStateVariable("Volume");
		if (var)
			strVolume = var->GetValue();
	}

	for (NPT_List<PLT_StateVariable*>::Iterator Iter = vars->GetFirstItem();
		Iter;
		++Iter)
	{
		PLT_StateVariable* var = *Iter;
		NPT_String var_name = var->GetName();
		NPT_String var_value = var->GetValue();
		if (var_name == "Volume")
		{
			this->OnVolumeStateChanged(var_value);
		}
		else if (var_name == "TransportState")
		{
			this->OnTransportStateChanged(var_value);
		}
	}

}

void CSHDLNAMediaController::OnVolumeStateChanged(const NPT_String& volume_state)
{
	if (m_Player)
	{
		m_Player->OnVolumeStateChanged(volume_state);
	}
}

void CSHDLNAMediaController::OnTransportStateChanged(const NPT_String& transport_state)
{
	if (m_Player)
	{
		m_Player->OnTransportStateChanged(transport_state);
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

// 	if (m_MessageNotifyUI != NULL)
// 	{
// 		m_MessageNotifyUI(SH_DLNAPLAYER_UI_MESSAGE_DEVICE_LIST_UPDATED, (void*)&m_DeviceList, NULL);
// 	}
	if (m_Player != NULL)
	{
		m_Player->OnDeviceListUpdated((void*)&m_DeviceList);
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
			long millis = info->media_duration.ToMillis();
			if (m_Player != NULL)
			{
				m_Player->OnGetMediaDurationResult(res, (void*)millis);
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
			long pos = info->rel_time.ToMillis();
			if (m_Player != NULL)
			{
				m_Player->OnGetCurPlayPosResult(res, (void*)pos);
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
	if (m_Player != NULL)
	{
		m_Player->OnGetTransportInfoResult(res, (void*)info);
	}
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
	if (m_Player != NULL)
	{
		m_Player->OnPauseResult(res);
	}
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
	if (m_Player != NULL)
	{
		m_Player->OnPlayResult(res);
	}
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
	
	if (m_Player != NULL)
	{
		m_Player->OnSetAVTransportURIResult(res);
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
	if (m_Player)
	{
		m_Player->OnStopResult(res);
	}
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
	if (m_Player != NULL)
	{
		m_Player->OnSetVolumeResult(res);
	}
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
		int vol = volume;
		if (m_Player != NULL)
		{
			m_Player->OnGetVolumeResult(res, (void*)vol);
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

	int start=name.ReverseFind(".");
	NPT_String title=name.Left(start);


	didl += "<dc:title>";
	PLT_Didl::AppendXmlEscape(didl, title);
	didl += "</dc:title>";

	didl +="<dc:creator>tv.sohu.com</dc:creator><upnp:genre>tv.sohu.com</upnp:genre>";

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
|   
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaController::SetCurMeidaServer(PLT_DeviceHostReference& server)
{
	NPT_Result result = NPT_SUCCESS;

	m_CurMediaServer = server;

	return result;
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
