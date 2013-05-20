#include "StdAfx.h"
#include "MultiRequest.h"

CMultiRequest::CMultiRequest(EResponseDataType eDataType)
{
	multi_handle = NULL;
	still_running = 0;
	vRequestPtrList.clear();
	m_data_type = eDataType;
}

CMultiRequest::~CMultiRequest(void)
{
	Done();
}

bool CMultiRequest::AddRequest(const char* url, unsigned int response_datasize, ResponseCallBack cb, ProgressCallBack progresscb,
				vector<pair<CURLoption, const char*> >* m_vector_ptr,
				void* customparam,
				unsigned int uiTimeOut, DebugCallBack debugcb)
{
	if (url == NULL)
		return false;

	if (multi_handle == NULL)
		multi_handle = curl_multi_init();

	CEasyRequest* pnewRequest = new CEasyRequest();

	if (m_vector_ptr)
	{
		for (vector<pair<CURLoption, const char*> >::iterator itr = m_vector_ptr->begin();
			itr != m_vector_ptr->end(); itr++)
		{
			pnewRequest->AddRequestOption(itr->first, itr->second);
		}
	}

	CURL * curl_handle = pnewRequest->Request(url, m_data_type, response_datasize, cb, progresscb, vRequestPtrList.size(), customparam, uiTimeOut, debugcb);
	if (curl_handle)
	{	
		curl_multi_add_handle(multi_handle, curl_handle);
		vRequestPtrList.push_back(pnewRequest);
	}

	return true;
}

CURLcode CMultiRequest::Perform(void)
{
	CURLcode ReturnCode = CURLE_OK;

	/* we start some action by calling perform right away */
	while(CURLM_CALL_MULTI_PERFORM ==
		curl_multi_perform(multi_handle, &still_running));

	while(still_running) 
	{
		CURLMcode err;
		struct timeval timeout;
		int rc; /* select() return code */

		fd_set fdread;
		fd_set fdwrite;
		fd_set fdexcep;
		int maxfd;

		FD_ZERO(&fdread);
		FD_ZERO(&fdwrite);
		FD_ZERO(&fdexcep);

		/* set a suitable timeout to play around with */
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		/* get file descriptors from the transfers */
		err = curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);

		/* In a real-world program you OF COURSE check the return code of the
		function calls, *and* you make sure that maxfd is bigger than -1 so
		that the call to select() below makes sense! */

		rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);

		switch(rc) 
		{
		case -1:
			/* select error */
			still_running = 0;
			printf("select() returns error, this is badness\n");
			break;
		default:
			/* timeout or readable/writable sockets */
			Sleep(10);
			while(CURLM_CALL_MULTI_PERFORM ==
				curl_multi_perform(multi_handle, &still_running));
			break;
		}
	}

	CURLMsg *msg; /* for picking up messages with the transfer status */
	int msgs_left; /* how many messages are left */

	/* See how the transfers went */
	while((msg = curl_multi_info_read(multi_handle, &msgs_left)))
	{
		if (msg->msg == CURLMSG_DONE) 
		{
			size_t idx = 0;
			bool found = false;

			/* Find out which handle this message is about */
			for (idx=0; (!found && (idx < vRequestPtrList.size())); idx++) 
				found = (msg->easy_handle == vRequestPtrList[idx]->m_handle);

			if (found)
			{
				if (msg->data.result == CURLE_OK)
				{
					if (vRequestPtrList.size() > 1)
						printf("Response %d Done! Receive size = %d\n", idx-1, vRequestPtrList[idx-1]->m_response_size);	
					else
						printf("Single Response Done! Receive size = %d\n", vRequestPtrList[idx-1]->m_response_size);
				}
				else
				{					
					if (vRequestPtrList.size() > 1)
						printf("Response %d Failed error code = %d\n", idx-1, msg->data.result);
					else
						printf("Single Response Failed error code = %d\n", msg->data.result);

					ReturnCode = msg->data.result;
					break;
				}
			}
		}
	}

	return ReturnCode;
}

void CMultiRequest::Done(void)
{
	curl_multi_cleanup(multi_handle);
	still_running = 0;
	multi_handle = NULL;

	if (!vRequestPtrList.empty())
	{
		for (size_t i = 0; i < vRequestPtrList.size(); i++)
			delete vRequestPtrList[i];
	}

	vRequestPtrList.clear();
}