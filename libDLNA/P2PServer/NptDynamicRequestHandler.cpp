#include "NptDynamicRequestHandler.h"
//#include "NptStdcFile.cpp"
#include <Windows.h>
#include <fstream>

WCHAR* AsciiToUnicode(char* str)  
{  
	int widesize = MultiByteToWideChar(CP_ACP,
		0, 
		str, 
		-1, 
		NULL, 
		0);  
	WCHAR *pText=new WCHAR[widesize+1];
	memset(pText,0,sizeof(pText));
	MultiByteToWideChar(CP_ACP, 
		0, 
		str, 
		-1, 
		pText, 
		widesize);  
	return pText;  
} 

char* UnicodeToUTF8(WCHAR* str)
{
	char* pText;
	int iLen;
	iLen=WideCharToMultiByte(CP_UTF8,
		0,
		str,
		-1,
		NULL,
		0,
		NULL,
		NULL);
	pText=new char[iLen+1];
	memset(pText,0,sizeof(pText));
	WideCharToMultiByte(CP_UTF8,
		0,
		str,
		-1,
		pText,
		iLen,
		NULL,
		NULL);
	return pText;
}

NPT_HttpDynamicRequestHandler::
NPT_HttpDynamicRequestHandler(bool flag,
							  GetDataBufferCallBack func,
							  const char* name,
							  const char* type)
	:NPT_HttpFileRequestHandler("/",name),
	dynamic_or_static(flag),
	GetDataBuffer(func),
	directory_path(name)
{
};

/*----------------------------------------------------------------------
|   NPT_HttpDynamicRequestHandler::SetupResponse
+---------------------------------------------------------------------*/
NPT_Result 
NPT_HttpDynamicRequestHandler::SetupResponse(NPT_HttpRequest& request, 
	                                         const NPT_HttpRequestContext& context, 
	                                         NPT_HttpResponse& response)
{
	if (dynamic_or_static)
	{
		NPT_HttpEntity* entity = response.GetEntity();
		if (entity == NULL) return NPT_ERROR_INVALID_STATE;

		// check the method
		if (request.GetMethod() != NPT_HTTP_METHOD_GET &&
			request.GetMethod() != NPT_HTTP_METHOD_HEAD) {
				response.SetStatus(405, "Method Not Allowed");
				return NPT_SUCCESS;
		}

		// set some default headers
		response.GetHeaders().SetHeader(NPT_HTTP_HEADER_ACCEPT_RANGES, "bytes");

		// declare HTTP/1.1 if the client asked for it
		if (request.GetProtocol() == NPT_HTTP_PROTOCOL_1_1) {
			response.SetProtocol(NPT_HTTP_PROTOCOL_1_1);
		}

		// TODO: we need to normalize the request path

		// compute the filename

		NPT_String root= directory_path.TrimRight("/\\");
		NPT_String url = NPT_Url::PercentDecode(request.GetUrl().GetPath().GetChars());
		NPT_String query=NPT_Url::PercentDecode(request.GetUrl().GetQuery().GetChars());
		root+=url+"µÚ"+query+"¶Î.mp4";
		NPT_String filename=root.Replace('/','\\');
		NPT_LOG_FINE_1("filename = %s", filename.GetChars());

		// get info about the file
		NPT_FileInfo info;
		NPT_String path=UnicodeToUTF8(AsciiToUnicode((char*)filename.GetChars()));
		NPT_File::GetInfo(path, &info);

		// open the file
		NPT_File file(path);
		NPT_Result result = file.Open(NPT_FILE_OPEN_MODE_READ);
		if (NPT_FAILED(result)) {
			NPT_LOG_FINE("file not found");
			return NPT_ERROR_NO_SUCH_ITEM;
		}
		NPT_InputStreamReference stream;
		file.GetInputStream(stream);

		//stdcFile=new NPT_StdcFileWrapper(file,filename);
		//stream=new NPT_StdcFileInputStream(stdcFile);

		// check for range requests
		const NPT_String* range_spec = request.GetHeaders().GetHeaderValue(NPT_HTTP_HEADER_RANGE);

		// setup entity body
		NPT_CHECK(SetupResponseBody(response, stream, range_spec));

		const NPT_String* range_len=response.GetHeaders().GetHeaderValue(NPT_HTTP_HEADER_CONTENT_RANGE);
		/*std::ofstream out("C:\\log\\data2.log",std::ios::app);
		out<<query<<" "<<range_spec->GetChars()<<std::endl;
		out.close();*/
		// set the response body
		entity->SetContentType(GetContentType(filename));

		return NPT_SUCCESS;
	}
	else
	{
		NPT_HttpEntity* entity = response.GetEntity();
		if (entity == NULL) return NPT_ERROR_INVALID_STATE;

		entity->SetContentType(mime_type);

		NPT_String url=NPT_Url::PercentDecode(request.GetUrl().GetPath().GetChars());
		NPT_String absolate_path=directory_path.TrimRight("/\\")+url;
		NPT_String path_url=absolate_path.Replace('\\','/');
		NPT_Byte* buffer=NULL;
		NPT_Size size(0);
		GetDataBuffer(absolate_path.GetChars(),buffer,&size);
		entity->SetInputStream(buffer,size);

		return NPT_SUCCESS;
	}
	

	return NPT_SUCCESS;
}