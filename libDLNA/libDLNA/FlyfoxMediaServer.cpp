#include "stdafx.h"
#include "FlyfoxMediaServer.h"

//NPT_SET_LOCAL_LOGGER("libdlna.flyfoxmediaserver")

static NPT_File* s_pFile = NULL;

static int StdcFileOpen(const char* path, int mode) {
	if (s_pFile) delete s_pFile;

	s_pFile = new NPT_File(path);
	if (s_pFile)
		return s_pFile->Open((NPT_FileInterface::OpenMode)mode);
	return 0;
}

static int StdcFileClose(){
	if (s_pFile)
		return s_pFile->Close();
	return 0;
}

static int StdcFileRead(void* buffer, unsigned int bytes_to_read, unsigned int* bytes_read) {
	if (s_pFile)
	{
		NPT_InputStreamReference stream;
		if (NPT_SUCCEEDED(s_pFile->GetInputStream(stream)))
		{
			return stream->Read(buffer, bytes_to_read, bytes_read);
		}
	}
	return 0;
}

static int StdcFileSeek(unsigned __int64 offset) {
	if (s_pFile)
	{
		NPT_InputStreamReference stream;
		if (NPT_SUCCEEDED(s_pFile->GetInputStream(stream)))
		{
			return stream->Seek(offset);
		}
	}
	return 0;
}

static int StdcFileTell(unsigned __int64* offset) {
	if (s_pFile)
	{
		NPT_InputStreamReference stream;
		if (NPT_SUCCEEDED(s_pFile->GetInputStream(stream)))
		{
			return stream->Tell(*offset);
		}
	}
	return 0;
}

static int StdcFileGetSize(unsigned __int64* size) {
	if (s_pFile)
	{
		NPT_InputStreamReference stream;
		if (NPT_SUCCEEDED(s_pFile->GetInputStream(stream)))
		{
			return stream->GetSize(*size);
		}
	}
	return 0;
}

static int StdcFileGetAvailable(unsigned __int64* available) {
	if (s_pFile)
	{
		NPT_InputStreamReference stream;
		if (NPT_SUCCEEDED(s_pFile->GetInputStream(stream)))
		{
			return stream->GetAvailable(*available);
		}
	}
	return 0;
}

/*----------------------------------------------------------------------
|   CFlyfoxMediaServerDelegate::CFlyfoxMediaServerDelegate
+---------------------------------------------------------------------*/
CFlyfoxMediaServerDelegate::CFlyfoxMediaServerDelegate(const char* url_root)
: m_UrlRoot(url_root)
{

}

/*----------------------------------------------------------------------
|   CFlyfoxMediaServerDelegate::~CFlyfoxMediaServerDelegate
+---------------------------------------------------------------------*/
CFlyfoxMediaServerDelegate::~CFlyfoxMediaServerDelegate()
{

}

/*----------------------------------------------------------------------
|   CFlyfoxMediaServerDelegate::ProcessFileRequest
+---------------------------------------------------------------------*/
NPT_Result 
CFlyfoxMediaServerDelegate::ProcessFileRequest(NPT_HttpRequest&              request, 
											   const NPT_HttpRequestContext& context,
											   NPT_HttpResponse&             response)
{
	NPT_InputStreamReference stream;
	NPT_HttpUrlQuery query(request.GetUrl().GetQuery());
	int file_type = DLNA_FILE_UNKNOWN;	
	PLT_HttpRequestContext tmp_context(request, context);

	PLT_LOG_HTTP_MESSAGE(NPT_LOG_LEVEL_FINE, "CFlyfoxMediaServerDelegate::ProcessFileRequest:", &request);

	if (request.GetMethod().Compare("GET") && request.GetMethod().Compare("HEAD")) {
		response.SetStatus(500, "Internal Server Error");
		return NPT_SUCCESS;
	}

	/* Extract file path and file type from url */
	NPT_String file_path;
	/*NPT_CHECK_LABEL_WARNING(*/
	if (NPT_FAILED(ExtractResourcePath(request.GetUrl(), file_path)))
		goto failure;
	NPT_ParseInteger(query.GetField("type"), file_type);

	file_path = "¹¦·òÐÜÃ¨2[³¬Çå°æ]";
	//file_path = "Z:\\Shared\\Media\\2013Oscar.mp4";

	stream = new CFlyfoxFileInputStream(file_path, file_type/*, CLibDLNA::GetStreamCtrl()*//*CLibDLNA::GetIOCallbacks()*/);
// 	if (NPT_FAILED((stream)->Open(file_path, file_type)))
// 		goto failure;

	return ServeStream(request, context, response, stream, PLT_MimeType::GetMimeType(file_path, &tmp_context));

failure:
	response.SetStatus(404, "File Not Found");
	return NPT_SUCCESS;
}

// /*----------------------------------------------------------------------
// |   CFlyfoxMediaServerDelegate::ServeFileStream
// +---------------------------------------------------------------------*/
// NPT_Result 
// CFlyfoxMediaServerDelegate::ServeFileStream(const NPT_HttpRequest&		request, 
// 											const NPT_HttpRequestContext&	context,
// 											NPT_HttpResponse&				response,
// 											CFlyfoxFileStream&			file_stream)
// {
// 	// open file stream
// 	NPT_InputStreamReference stream;
// 	if (NPT_FAILED(file_stream.GetInputStream(stream))        ||
// 		stream.IsNull()) {
// 			return NPT_ERROR_NO_SUCH_ITEM;
// 	}
// 
// 	PLT_HttpRequestContext tmp_context(request, context);
// 	return ServeStream(request, context, response, stream, PLT_MimeType::GetMimeType(file_stream.GetPath(), &tmp_context));
// }

/*----------------------------------------------------------------------
|   CFlyfoxMediaServerDelegate::ServeStream
+---------------------------------------------------------------------*/
NPT_Result 
CFlyfoxMediaServerDelegate::ServeStream(const NPT_HttpRequest&        request, 
										const NPT_HttpRequestContext& context,
										NPT_HttpResponse&             response,
										NPT_InputStreamReference&     body, 
										const char*                   content_type)
{
	NPT_CHECK_WARNING(PLT_HttpServer::ServeStream(request, context, response, body, content_type));
	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CFlyfoxMediaServerDelegate::OnBrowseMetadata
+---------------------------------------------------------------------*/
NPT_Result
CFlyfoxMediaServerDelegate::OnBrowseMetadata(PLT_ActionReference&          action, 
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
|   PLT_FileMediaServerDelegate::OnBrowseDirectChildren
+---------------------------------------------------------------------*/
NPT_Result
CFlyfoxMediaServerDelegate::OnBrowseDirectChildren(PLT_ActionReference&          action, 
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
|   PLT_FileMediaServerDelegate::OnSearchContainer
+---------------------------------------------------------------------*/
NPT_Result
CFlyfoxMediaServerDelegate::OnSearchContainer(PLT_ActionReference&          action, 
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
NPT_String CFlyfoxMediaServerDelegate::BuildSafeResourceUri(const NPT_HttpUrl&	base_uri, 
															const char*			host, 
															const char*			file_path,
															int					file_type)
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

	/* set query */
	NPT_String uri_query = NPT_String::Format("type=%d", file_type);
	uri.SetQuery(uri_query);

	/* 360 hack: force inclusion of port in case it's 80 */
	return uri.ToStringWithDefaultPort(0);
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
NPT_Result CFlyfoxMediaServerDelegate::ExtractResourcePath(const NPT_HttpUrl&	url, 
														   NPT_String&			file_path/*,
														   int&					file_type*/)
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