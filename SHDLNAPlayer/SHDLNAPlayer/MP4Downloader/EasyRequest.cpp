#include "StdAfx.h"
#include "EasyRequest.h"

#ifdef _DEBUG
#pragma comment(lib, "libcurld.lib")
#else
#pragma comment(lib, "libcurl.lib")
#endif

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "wldap32.lib")

CEasyRequest::CEasyRequest(void)
{
	m_handle = NULL;
	m_index = 0;
	m_response_size = 0;
	m_response_datasize = 0;
	m_customparam = 0;
}

CEasyRequest::~CEasyRequest(void)
{
	Done();
}

void CEasyRequest::AddRequestOption(CURLoption option_type, const char* option_value)
{
	if (option_value == NULL)
		return;

	map<CURLoption, curl_slist *>::iterator itr = request_option_map.find(option_type);
	if (itr == request_option_map.end())
	{
		curl_slist * slist = NULL;
		slist = curl_slist_append(slist, option_value);
		request_option_map.insert(pair<CURLoption, curl_slist *>(option_type, slist));
	}
	else
	{
		itr->second = curl_slist_append(itr->second, option_value);
	}
}

CURL* CEasyRequest::Request(const char* url, EResponseDataType data_type, unsigned int response_datasize, ResponseCallBack cb, 
							ProgressCallBack progresscb, unsigned int index, void* customparam, unsigned int timeout, DebugCallBack debugcb)
{
	if (url == NULL || m_handle)
		return NULL;

	m_index = index;
	m_data_type = data_type;
	m_response_datasize = response_datasize;
	m_customparam = customparam;

	m_handle = curl_easy_init();

	curl_easy_setopt(m_handle, CURLOPT_URL, url);

	if (debugcb)
	{
		curl_easy_setopt(m_handle, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(m_handle, CURLOPT_DEBUGFUNCTION, debugcb);
	}

	curl_easy_setopt(m_handle, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(m_handle, CURLOPT_WRITEFUNCTION, cb);
	curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, this);

	for (map<CURLoption, curl_slist *>::iterator itr = request_option_map.begin();
		itr != request_option_map.end(); itr++)
	{
		curl_easy_setopt(m_handle, itr->first, itr->second);
	}

	curl_easy_setopt(m_handle, CURLOPT_NOSIGNAL, 1);

	curl_easy_setopt(m_handle, CURLOPT_PROGRESSFUNCTION, progresscb);
	curl_easy_setopt(m_handle, CURLOPT_PROGRESSDATA, this);
	curl_easy_setopt(m_handle, CURLOPT_NOPROGRESS, 0);

	curl_easy_setopt(m_handle, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(m_handle, CURLOPT_SSL_VERIFYHOST, false);

	curl_easy_setopt(m_handle, CURLOPT_CONNECTTIMEOUT, timeout);
	//curl_easy_setopt(m_handle, CURLOPT_TIMEOUT, timeout);

	return m_handle;
}

void CEasyRequest::Done(void)
{
	if (m_handle)
		curl_easy_cleanup(m_handle);
	m_handle = NULL;

	for (map<CURLoption, curl_slist *>::iterator itr = request_option_map.begin();
		itr != request_option_map.end(); itr++)
	{
		if (itr->second)
			curl_slist_free_all(itr->second);
	}

	m_index = 0;
	m_response_size = 0;
	m_response_datasize = 0;
	m_customparam = NULL;
}