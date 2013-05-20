#pragma once

#include <vector>
#include "EasyRequest.h"

class CMultiRequest
{
public:
	CMultiRequest(EResponseDataType eDataType);
	~CMultiRequest(void);

public:
	bool AddRequest(const char* url, unsigned int response_datasize, ResponseCallBack cb, ProgressCallBack progresscb,
		vector<pair<CURLoption, const char*> >* m_vector_ptr = NULL,
		void* customparam = NULL,
		unsigned int uiTimeOut = 10, DebugCallBack debugcb = NULL);
	
	CURLcode Perform(void);

	void Done(void);

	unsigned int GetResponseDataSizeByIndex(unsigned int index)
	{
		if (index >= vRequestPtrList.size())
			return 0;

		return vRequestPtrList[index]->m_response_size;
	}

private:
	CURLM *multi_handle;
	int still_running; /* keep number of running handles */
	EResponseDataType m_data_type;
	vector<CEasyRequest*> vRequestPtrList; 
};
