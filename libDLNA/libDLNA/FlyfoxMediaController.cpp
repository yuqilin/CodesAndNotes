#include "stdafx.h"
#include "FlyfoxMediaController.h"

/*----------------------------------------------------------------------
|   CFlyfoxMediaController::CFlyfoxMediaController
+---------------------------------------------------------------------*/
CFlyfoxMediaController::CFlyfoxMediaController(PLT_CtrlPointReference& ctrlPoint) 
: PLT_MediaController(ctrlPoint)
{
	// create the stack that will be the directory where the
	// user is currently browsing. 
	// push the root directory onto the directory stack.
	//m_CurBrowseDirectoryStack.Push("0");

	PLT_MediaController::SetDelegate(this);
}

/*----------------------------------------------------------------------
|   CFlyfoxMediaController::CFlyfoxMediaController
+---------------------------------------------------------------------*/
CFlyfoxMediaController::~CFlyfoxMediaController()
{
}

/*----------------------------------------------------------------------
|   CFlyfoxMediaController::OnMRAdded
+---------------------------------------------------------------------*/
bool
CFlyfoxMediaController::OnMRAdded(PLT_DeviceDataReference& device)
{
	NPT_String uuid = device->GetUUID();

	//NPT_String ip = device->GetLocalIP().ToString();

	// test if it's a media renderer
	PLT_Service* service;
	if (NPT_SUCCEEDED(device->FindServiceByType("urn:schemas-upnp-org:service:AVTransport:*", service))) {
		NPT_AutoLock lock(m_MediaRenderers);
		m_MediaRenderers.Put(uuid, device);
	}

	return true;
}

/*----------------------------------------------------------------------
|   CFlyfoxMediaController::OnMRRemoved
+---------------------------------------------------------------------*/
void
CFlyfoxMediaController::OnMRRemoved(PLT_DeviceDataReference& device)
{
	NPT_String uuid = device->GetUUID();

	{
		NPT_AutoLock lock(m_MediaRenderers);
		m_MediaRenderers.Erase(uuid);
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
|   CFlyfoxMediaController::GetCurMediaRenderer
+---------------------------------------------------------------------*/
void CFlyfoxMediaController::GetCurMediaRenderer(PLT_DeviceDataReference& renderer)
{
	NPT_AutoLock lock(m_CurMediaRendererLock);

	if (m_CurMediaRenderer.IsNull()) {
		//printf("No renderer selected, select one with setmr\n");
	} else {
		renderer = m_CurMediaRenderer;
	}
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
NPT_Result CFlyfoxMediaController::DiscoverDevices(char devices[DLNA_MAX_DEVICE_COUNT][DLNA_MAX_DEVICE_NAME_LEN], int* count)
{
	NPT_AutoLock lock(m_CurMediaRendererLock);
	
	const NPT_List<PLT_DeviceMapEntry*>& entries = m_MediaRenderers.GetEntries();
	NPT_List<PLT_DeviceMapEntry*>::Iterator entry = entries.GetFirstItem();
	int i = 0;
	while (entry)
	{
		if (i < DLNA_MAX_DEVICE_COUNT)
		{
			PLT_DeviceDataReference device = (*entry)->GetValue();
			NPT_String              name   = device->GetFriendlyName();

			strcpy_s(devices[i], DLNA_MAX_DEVICE_NAME_LEN, name.GetChars());
			++i;
		}
		else
			break;

		++entry;
	}

	if (count)
		*count = i;	

	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CFlyfoxMediaController::SetCurMediaRenderer
+---------------------------------------------------------------------*/
NPT_Result CFlyfoxMediaController::SetCurMediaRenderer(NPT_Ordinal index)
{
	NPT_AutoLock lock(m_CurMediaRendererLock);

	const NPT_List<PLT_DeviceMapEntry*>& entries = m_MediaRenderers.GetEntries();
	NPT_Ordinal count = entries.GetItemCount();

	NPT_List<PLT_DeviceMapEntry*>::Iterator entry = entries.GetFirstItem();
	NPT_Ordinal num = 0;
	while (entry)
	{
		if (num == index)
		{
			PLT_DeviceDataReference* renderer = NULL;
			NPT_Result result = m_MediaRenderers.Get((*entry)->GetKey(), renderer);
			if (NPT_SUCCEEDED(result))
				m_CurMediaRenderer = *renderer;
			return result;
		}
		else if (num > index)
		{
			break;
		}
		++entry;
		++num;
	}

	return NPT_ERROR_OUT_OF_RANGE;
}
