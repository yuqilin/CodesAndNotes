#pragma once

#include <map>
#include "curl.h"

using namespace std;

typedef size_t(*ResponseCallBack)(void* , size_t , size_t , void*);
typedef int(*DebugCallBack)(CURL *, curl_infotype , char *, size_t , void *);
typedef int(*ProgressCallBack)(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow); 

typedef enum tagResponseDataType
{
	Response_Invalid_DataType = -1,
	Response_Whole_DataType,
	Response_Partial_DataType
}EResponseDataType;

class CEasyRequest
{
public:
	CEasyRequest(void);
	~CEasyRequest(void);

public:
	void AddRequestOption(CURLoption option_type, const char* option_value);
	CURL* Request(const char* url, EResponseDataType data_type, unsigned int response_datasize, 
		ResponseCallBack cb, ProgressCallBack progresscb, unsigned int index, void* customparam = NULL, unsigned int timeout = 10, DebugCallBack debugcb = NULL);
	void Done(void);

public:
	unsigned int m_index;
	unsigned int m_response_size;
	unsigned int m_response_datasize;
	CURL* m_handle;
	void* m_customparam;

private:
	map<CURLoption, curl_slist *> request_option_map;
	EResponseDataType m_data_type;
};
