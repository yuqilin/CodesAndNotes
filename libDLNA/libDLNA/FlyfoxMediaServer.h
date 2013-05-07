#ifndef _FLYFOX_MEDIA_SERVER_H_
#define _FLYFOX_MEDIA_SERVER_H_

#include "FlyfoxFileStream.h"

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
class CFlyfoxMediaServerDelegate : public PLT_MediaServerDelegate
{
public:
	static NPT_String BuildSafeResourceUri(const NPT_HttpUrl& base_uri, 
		const char*			host, 
		const char*			file_path,
		int					file_type);

	CFlyfoxMediaServerDelegate(const char* url_root);
	virtual ~CFlyfoxMediaServerDelegate();

protected:
	// PLT_MediaServerDelegate methods
	virtual NPT_Result OnBrowseMetadata(PLT_ActionReference&          action, 
		const char*                   object_id, 
		const char*                   filter,
		NPT_UInt32                    starting_index,
		NPT_UInt32                    requested_count,
		const char*                   sort_criteria,
		const PLT_HttpRequestContext& context);
	virtual NPT_Result OnBrowseDirectChildren(PLT_ActionReference&          action, 
		const char*                   object_id, 
		const char*                   filter,
		NPT_UInt32                    starting_index,
		NPT_UInt32                    requested_count,
		const char*                   sort_criteria,
		const PLT_HttpRequestContext& context);
	virtual NPT_Result OnSearchContainer(PLT_ActionReference&          action, 
		const char*                   object_id, 
		const char*                   search_criteria,
		const char*                   filter,
		NPT_UInt32                    starting_index,
		NPT_UInt32                    requested_count,
		const char*                   sort_criteria, 
		const PLT_HttpRequestContext& context);
	virtual NPT_Result ProcessFileRequest(NPT_HttpRequest&              request, 
		const NPT_HttpRequestContext& context,
		NPT_HttpResponse&             response);

// 	virtual NPT_Result ServeFileStream(const NPT_HttpRequest&        request, 
// 		const NPT_HttpRequestContext& context,
// 		NPT_HttpResponse&             response,
// 		CFlyfoxFileStream&	  stream);

	virtual NPT_Result ServeStream(const NPT_HttpRequest&        request, 
		const NPT_HttpRequestContext& context,
		NPT_HttpResponse&             response,
		NPT_InputStreamReference&     body, 
		const char*                   content_type);

	NPT_Result ExtractResourcePath(const NPT_HttpUrl&	url, 
		NPT_String&			file_path/*,
		int&				file_type*/);

protected:
	NPT_String m_UrlRoot;
};

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
class CFlyfoxMediaServer : public PLT_MediaServer
	, public CFlyfoxMediaServerDelegate
{
public:    // constructor
	CFlyfoxMediaServer(/*FileIOCallbacks* io_cb,*/
		const char*  friendly_name,
		bool         show_ip = false,
		const char*  uuid = NULL,
		NPT_UInt16   port = 0,
		bool         port_rebind = false) :
	/*m_IOCallback(io_cb),*/
	PLT_MediaServer(friendly_name, 
		show_ip,
		uuid, 
		port,
		port_rebind),
		CFlyfoxMediaServerDelegate("/") {SetDelegate(this);}

protected:
	virtual ~CFlyfoxMediaServer() {}

	/*FileIOCallbacks*	m_IOCallback;*/
};


#endif