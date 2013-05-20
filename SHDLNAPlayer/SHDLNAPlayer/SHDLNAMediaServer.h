#ifndef _SH_DLNA_MEDIA_SERVER_H_
#define _SH_DLNA_MEDIA_SERVER_H_

#include "SHDLNAMediaController.h"

#define SH_DLNA_MEDIA_SERVER_UUID		"7D9035AA-EDFC-464d-995F-50B491D23752"

/*----------------------------------------------------------------------
|   class CSHDLNAMediaServerDelegate
+---------------------------------------------------------------------*/
class CSHDLNAMediaServerDelegate : public PLT_MediaServerDelegate
{
public:
	static NPT_String BuildSafeResourceUri(const NPT_HttpUrl& base_uri, 
		const char*			host, 
		const char*			file_path);

	CSHDLNAMediaServerDelegate(const char* url_root);
	virtual ~CSHDLNAMediaServerDelegate();

	virtual void SetMediaInfo(SHDLNAMediaInfo_t* mediainfo) {
		m_Mediainfo = mediainfo;
	}

protected:
	// PLT_MediaServerDelegate methods
	virtual NPT_Result ProcessFileRequest(
		NPT_HttpRequest&              request,
		const NPT_HttpRequestContext& context,
		NPT_HttpResponse&             response);

	virtual NPT_Result ServeStream(
		const NPT_HttpRequest&        request,
		const NPT_HttpRequestContext& context,
		NPT_HttpResponse&             response,
		NPT_InputStreamReference&     body,
		const char*                   content_type);

	NPT_Result ExtractResourcePath(
		const NPT_HttpUrl&			  url,
		NPT_String&					  file_path);

	virtual NPT_Result OnBrowseMetadata(
		PLT_ActionReference&          action, 
		const char*                   object_id, 
		const char*                   filter,
		NPT_UInt32                    starting_index,
		NPT_UInt32                    requested_count,
		const char*                   sort_criteria,
		const PLT_HttpRequestContext& context);

	virtual NPT_Result OnBrowseDirectChildren(
		PLT_ActionReference&          action, 
		const char*                   object_id, 
		const char*                   filter,
		NPT_UInt32                    starting_index,
		NPT_UInt32                    requested_count,
		const char*                   sort_criteria,
		const PLT_HttpRequestContext& context);

	virtual NPT_Result OnSearchContainer(
		PLT_ActionReference&          action, 
		const char*                   object_id, 
		const char*                   search_criteria,
		const char*                   filter,
		NPT_UInt32                    starting_index,
		NPT_UInt32                    requested_count,
		const char*                   sort_criteria, 
		const PLT_HttpRequestContext& context);

protected:
	NPT_String m_UrlRoot;

	SHDLNAMediaInfo_t* m_Mediainfo;
};

/*----------------------------------------------------------------------
|   class CSHDLNA_MediaServer
+---------------------------------------------------------------------*/
class CSHDLNAMediaServer	: public PLT_MediaServer
							, public CSHDLNAMediaServerDelegate
{
public:    // constructor
	CSHDLNAMediaServer(
		const char*  friendly_name,
		bool         show_ip = false,
		const char*  uuid = NULL,
		NPT_UInt16   port = 0,
		bool         port_rebind = false) :
	PLT_MediaServer(friendly_name, 
		show_ip,
		SH_DLNA_MEDIA_SERVER_UUID/*uuid*/,
		port,
		port_rebind),
		CSHDLNAMediaServerDelegate("/")
	{SetDelegate(this);}

protected:
	virtual ~CSHDLNAMediaServer() {}
};



#endif // _SH_DLNA_MEDIA_SERVER_H_
