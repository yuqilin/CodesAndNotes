#ifndef _NPT_DYNAMIC_HANDLE_H_
#define _NPT_DYNAMIC_HANDLE_H_

#include <stdio.h>
#include "Neptune.h"


typedef void (*GetDataBufferCallBack)(const char* name,unsigned char* &data,unsigned int* size);
/*----------------------------------------------------------------------
|   NPT_HttpDynamicRequestHandler
+---------------------------------------------------------------------*/
class NPT_HttpDynamicRequestHandler : public NPT_HttpFileRequestHandler
{
public:
	// constructors
	NPT_HttpDynamicRequestHandler(bool flag=true,
		                          GetDataBufferCallBack func=NULL,
								  const char* name="",
								  const char* type="video/mp4");

	// NPT_HttpRequestHandler methods
	NPT_Result SetupResponse(NPT_HttpRequest&              request, 
		                     const NPT_HttpRequestContext& context,
		                     NPT_HttpResponse&             response);

private:
	NPT_String directory_path;
	NPT_String mime_type;
	GetDataBufferCallBack GetDataBuffer;
	//true for dynamic create buffer
	bool dynamic_or_static;
};

#endif