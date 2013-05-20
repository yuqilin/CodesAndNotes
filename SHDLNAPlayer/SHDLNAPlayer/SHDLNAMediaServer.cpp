#include "stdafx.h"
#include "SHDLNAMediaServer.h"
#include "SHDLNAMediaPlayer.h"
//#include "SHDLNAMediaController.h"
#include "SHDLNAMediaStream.h"

NPT_SET_LOCAL_LOGGER("shdlnaplayer.shdlnamediaserver")

/*----------------------------------------------------------------------
|   CSHDLNAMediaServerDelegate::CSHDLNAMediaServerDelegate
+---------------------------------------------------------------------*/
CSHDLNAMediaServerDelegate::CSHDLNAMediaServerDelegate(const char* url_root)
: m_UrlRoot(url_root)
, m_Mediainfo(NULL)
{

}

/*----------------------------------------------------------------------
|   CSHDLNAMediaServerDelegate::~CSHDLNAMediaServerDelegate
+---------------------------------------------------------------------*/
CSHDLNAMediaServerDelegate::~CSHDLNAMediaServerDelegate()
{
	m_Mediainfo = NULL;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaServerDelegate::ProcessFileRequest
+---------------------------------------------------------------------*/
NPT_Result 
CSHDLNAMediaServerDelegate::ProcessFileRequest(NPT_HttpRequest&              request, 
											   const NPT_HttpRequestContext& context,
											   NPT_HttpResponse&             response)
{
	NPT_InputStreamReference stream;
	//NPT_HttpUrlQuery query(request.GetUrl().GetQuery());
	NPT_String mime_type("video/mp4");
	PLT_HttpRequestContext tmp_context(request, context);

	PLT_LOG_HTTP_MESSAGE(NPT_LOG_LEVEL_FINE, "CSHDLNAMediaServerDelegate::ProcessFileRequest:", &request);

	if (request.GetMethod().Compare("GET") && request.GetMethod().Compare("HEAD")) {
		response.SetStatus(500, "Internal Server Error");
		NPT_LOG_SEVERE("request parse got: Internal Server Error");
		return NPT_SUCCESS;
	}

	/* Extract file path and file type from url */
	NPT_String file_path;
// 	if (NPT_FAILED(ExtractResourcePath(request.GetUrl(), file_path)))
// 		goto failure;
// 
// 	if (file_path.StartsWith("http:") && file_path[6]!='/')
// 	{
// 		file_path.Insert("/", 6);
// 	}
	if (m_Mediainfo != NULL)
	{
		file_path = m_Mediainfo->url_from_ui;
	}
	else
	{
		if (NPT_FAILED(ExtractResourcePath(request.GetUrl(), file_path)))
			goto failure;
		if (file_path.StartsWith("http:") && file_path[6]!='/')
		{
			file_path.Insert("/", 6);
		}
	}

	stream = new CSHDLNAMediaStream;
	if (stream.IsNull() || 
		NPT_FAILED(((CSHDLNAMediaStream*)stream.AsPointer())->Open(file_path)))
	{
		return NPT_ERROR_NO_SUCH_ITEM;
	}

	if (!file_path.StartsWith("http://"))
	{
		mime_type = PLT_MimeType::GetMimeType(file_path, &tmp_context);
	}

	return ServeStream(request, context, response, stream, mime_type/*PLT_MimeType::GetMimeType(file_path, &tmp_context)*/);

failure:
	response.SetStatus(404, "File Not Found");
	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaServerDelegate::ServeStream
+---------------------------------------------------------------------*/
NPT_Result 
CSHDLNAMediaServerDelegate::ServeStream(const NPT_HttpRequest&        request, 
										const NPT_HttpRequestContext& context,
										NPT_HttpResponse&             response,
										NPT_InputStreamReference&     body, 
										const char*                   content_type)
{
	NPT_CHECK_WARNING(PLT_HttpServer::ServeStream(request, context, response, body, content_type));
	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaServerDelegate::OnBrowseMetadata
+---------------------------------------------------------------------*/
NPT_Result
CSHDLNAMediaServerDelegate::OnBrowseMetadata(PLT_ActionReference&          action, 
											 const char*                   object_id, 
											 const char*                   filter,
											 NPT_UInt32                    starting_index,
											 NPT_UInt32                    requested_count,
											 const char*                   sort_criteria,
											 const PLT_HttpRequestContext& context)
{
	NPT_COMPILER_UNUSED(sort_criteria);
	NPT_COMPILER_UNUSED(requested_count);
	NPT_COMPILER_UNUSED(starting_index);

	return NPT_ERROR_NOT_IMPLEMENTED;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaServerDelegate::OnBrowseDirectChildren
+---------------------------------------------------------------------*/
NPT_Result
CSHDLNAMediaServerDelegate::OnBrowseDirectChildren(PLT_ActionReference&          action, 
												   const char*                   object_id, 
												   const char*                   filter,
												   NPT_UInt32                    starting_index,
												   NPT_UInt32                    requested_count,
												   const char*                   sort_criteria,
												   const PLT_HttpRequestContext& context)
{
	NPT_COMPILER_UNUSED(sort_criteria);

	return NPT_ERROR_NOT_IMPLEMENTED;
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaServerDelegate::OnSearchContainer
+---------------------------------------------------------------------*/
NPT_Result
CSHDLNAMediaServerDelegate::OnSearchContainer(PLT_ActionReference&          action, 
											  const char*                   object_id, 
											  const char*                   search_criteria,
											  const char*                   /* filter */,
											  NPT_UInt32                    /* starting_index */,
											  NPT_UInt32                    /* requested_count */,
											  const char*                   /* sort_criteria */,
											  const PLT_HttpRequestContext& /* context */)
{
	return NPT_ERROR_NOT_IMPLEMENTED;
}

/*----------------------------------------------------------------------
|   CFlyfoxMediaServerDelegate::BuildSafeResourceUri
+---------------------------------------------------------------------*/
NPT_String CSHDLNAMediaServerDelegate::BuildSafeResourceUri(const NPT_HttpUrl&	base_uri, 
															const char*			host, 
															const char*			file_path)
{
	NPT_String result;
	NPT_HttpUrl uri = base_uri;

	if (host) uri.SetHost(host);

	NPT_String uri_path = uri.GetPath();
	if (!uri_path.EndsWith("/")) uri_path += "/";

	/* some controllers (like WMP) will call us with an already urldecoded version.
	We're intentionally prepending a known urlencoded string
	to detect it when we receive the request urlencoded or already decoded to avoid double decoding*/
	uri_path += "%/";
	uri_path += file_path;

	/* set path */
	uri.SetPath(uri_path);

	/* 360 hack: force inclusion of port in case it's 80 */
	return uri.ToStringWithDefaultPort(0);
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaServerDelegate::ExtractResourcePath(const NPT_HttpUrl&	url, 
														   NPT_String&			file_path)
{
	/* Extract non decoded path, we need to autodetect urlencoding */
	NPT_String uri_path = url.GetPath();
	NPT_String url_root_encode = NPT_Uri::PercentEncode(m_UrlRoot, NPT_Uri::PathCharsToEncode);

	NPT_Ordinal skip = 0;
	if (uri_path.StartsWith(m_UrlRoot)) {
		skip = m_UrlRoot.GetLength();
	} else if (uri_path.StartsWith(url_root_encode)) {
		skip = url_root_encode.GetLength();
	} else {
		return NPT_FAILURE;
	}

	/* account for extra slash */
	skip += ((m_UrlRoot=="/")?0:1);
	file_path = uri_path.SubString(skip);

	/* detect if client such as WMP sent a non urlencoded url */
	if (file_path.StartsWith("%/")) {
		NPT_LOG_FINE("Received a urldecoded version of our url!");
		file_path.Erase(0, 2);
	} else {
		/* remove our prepended string we used to detect urldecoded version */
		if (file_path.StartsWith("%25/")) file_path.Erase(0, 4);

		/* ok to urldecode */
		//file_path = NPT_Uri::PercentDecode(file_path);
	}
	file_path = NPT_Uri::PercentDecode(file_path);

	return NPT_SUCCESS;
}
