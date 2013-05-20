// mp4downloader.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

class ClientStuckTask : public PLT_ThreadTask 
{
public:
	ClientStuckTask(NPT_HttpUrl& url, NPT_HttpClient& client) 
		: m_Url(url), m_Client(client) {}

	// PLT_ThreadTask methods
	virtual void DoRun() { 
		NPT_HttpRequest request(m_Url, NPT_HTTP_METHOD_GET);
		NPT_HttpResponse* response = NULL;
		m_Client.SendRequest(request, response);
	}

	virtual void DoAbort()   {
		m_Client.Abort();
	}

private:
	NPT_HttpUrl     m_Url;
	NPT_HttpClient& m_Client;
	bool            m_Aborted;
};

#define SH_URL		"http://hot.vrs.sohu.com/vrs_flash.action?vid=1121728&ver=21&ref=0001"

int _tmain(int argc, _TCHAR* argv[])
{
	NPT_HttpClient client;

	PLT_TaskManager TaskManager;
	NPT_HttpUrl http_url(SH_URL);

	TaskManager.StartTask(new ClientStuckTask(http_url, client));

	bool abort = false;
	while (!abort)
	{


	}

	return 0;
}

