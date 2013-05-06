#ifndef _FLYFOX_MEDIA_CONTROLLER_H_
#define _FLYFOX_MEDIA_CONTROLLER_H_

/*----------------------------------------------------------------------
 |   definitions
 +---------------------------------------------------------------------*/
typedef NPT_Map<NPT_String, NPT_String>        PLT_StringMap;
typedef NPT_Lock<PLT_StringMap>                PLT_LockStringMap;
typedef NPT_Map<NPT_String, NPT_String>::Entry PLT_StringMapEntry;


/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
class CFlyfoxMediaController : public PLT_MediaController
							 , public PLT_MediaControllerDelegate
{
public:
	CFlyfoxMediaController(PLT_CtrlPointReference& ctrlPoint);
	virtual ~CFlyfoxMediaController();

	// PLT_MediaControllerDelegate methods
	bool OnMRAdded(PLT_DeviceDataReference& device);
	void OnMRRemoved(PLT_DeviceDataReference& device);
	void OnMRStateVariablesChanged(PLT_Service* /* service */, 
		NPT_List<PLT_StateVariable*>* /* vars */) {};

	// methods
	NPT_Result DiscoverDevices(char devices[DLNA_MAX_DEVICE_COUNT][DLNA_MAX_DEVICE_NAME_LEN], int* count);
	void       GetCurMediaRenderer(PLT_DeviceDataReference& renderer);
	NPT_Result SetCurMediaRenderer(NPT_Ordinal index);

private:
	/* The tables of known devices on the network.  These are updated via the
     * OnMSAddedRemoved and OnMRAddedRemoved callbacks.  Note that you should first lock
     * before accessing them using the NPT_Map::Lock function.
     */
	NPT_Lock<PLT_DeviceMap> m_MediaRenderers;

	/* The currently selected media renderer as well as 
     * a lock.  If you ever want to hold both the m_CurMediaRendererLock lock and the 
     * m_CurMediaServerLock lock, make sure you grab the server lock first.
     */
    PLT_DeviceDataReference m_CurMediaRenderer;
    NPT_Mutex               m_CurMediaRendererLock;

	/* the semaphore on which to block when waiting for a response from over
     * the network 
     */
    NPT_SharedVariable m_CallbackResponseSemaphore;

};

/*typedef */

#endif