#include "stdafx.h"
#include <assert.h>
#include "SHDLNAMediaStreamController.h"

NPT_SET_LOCAL_LOGGER("shdlnaplayer.shdlnastreamctrl")


/*----------------------------------------------------------------------
|   CSHDLNAMediaStreamController
+---------------------------------------------------------------------*/

CSHDLNAMediaStreamController::CSHDLNAMediaStreamController()
{
	m_type = SHDLNAPLAYER_MEDIA_UNKNOWN;
	m_localMedia = NULL;
	m_onlineMedia = NULL;
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
CSHDLNAMediaStreamController::~CSHDLNAMediaStreamController()
{
	Close();
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNAMediaStreamController::Open(const char* name, void* header)
{
	int ret = 0;
	NPT_String fileName = name;

	if ( fileName.StartsWith("http:") || fileName.StartsWith("https:") )
	{
		m_type = SHDLNAPLAYER_MEDIA_ONLINE;

		m_onlineMedia = new CSHDLNAOnlineMediaStreamController;
		ret = m_onlineMedia->Open(name, header);
	}
	else
	{
		m_type = SHDLNAPLAYER_MEDIA_LOCAL;

		m_localMedia = new CSHDLNALocalMediaStreamController;
		ret = m_localMedia->Open(name, header);
	}
	return ret;
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNAMediaStreamController::Close()
{
	int ret = 0;
	if (m_type == SHDLNAPLAYER_MEDIA_LOCAL)
	{
		if (m_localMedia)
		{
			delete m_localMedia;
			m_localMedia = NULL;
		}
	}
	else if(m_type == SHDLNAPLAYER_MEDIA_ONLINE)
	{
		if (m_onlineMedia)
		{
			delete m_onlineMedia;
			m_onlineMedia = NULL;
		}
	}
	return ret;
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNAMediaStreamController::Read(void* buffer, unsigned int bytes_to_read, unsigned int* bytes_read)
{
	int ret = 0;
	if (m_type == SHDLNAPLAYER_MEDIA_LOCAL)
	{
		ret = m_localMedia->Read(buffer, bytes_to_read, bytes_read);
	}
	else if (m_type == SHDLNAPLAYER_MEDIA_ONLINE)
	{
		ret = m_onlineMedia->Read(buffer, bytes_to_read, bytes_read);
	}
	return ret;
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNAMediaStreamController::Seek(unsigned __int64 offset)
{
	int ret = 0;
	if (m_type == SHDLNAPLAYER_MEDIA_LOCAL)
	{
		ret = m_localMedia->Seek(offset);
	}
	else if (m_type == SHDLNAPLAYER_MEDIA_ONLINE)
	{
		ret = m_onlineMedia->Seek(offset);
	}
	return ret;
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNAMediaStreamController::Tell(unsigned __int64* offset)
{
	int ret = 0;
	if (m_type == SHDLNAPLAYER_MEDIA_LOCAL)
	{
		ret = m_localMedia->Tell(offset);
	}
	else if (m_type == SHDLNAPLAYER_MEDIA_ONLINE)
	{
		ret = m_onlineMedia->Tell(offset);
	}
	return ret;
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNAMediaStreamController::GetSize(unsigned __int64* size)
{
	int ret = 0;
	if (m_type == SHDLNAPLAYER_MEDIA_LOCAL)
	{
		ret = m_localMedia->GetSize(size);
	}
	else if (m_type == SHDLNAPLAYER_MEDIA_ONLINE)
	{
		ret = m_onlineMedia->GetSize(size);
	}
	return ret;
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNAMediaStreamController::GetAvailable(unsigned __int64* available)
{
	int ret = 0;
	if (m_type == SHDLNAPLAYER_MEDIA_LOCAL)
	{
		ret = m_localMedia->GetAvailable(available);
	}
	else if (m_type == SHDLNAPLAYER_MEDIA_ONLINE)
	{
		ret = m_onlineMedia->GetAvailable(available);
	}
	return ret;
}

/*----------------------------------------------------------------------
|   class CSHDLNALocalMediaStreamController
+---------------------------------------------------------------------*/
CSHDLNALocalMediaStreamController::CSHDLNALocalMediaStreamController()
{
	m_fileSize = 0;
	m_available = 0;
	m_openFile = NULL;
	m_openFileName = "";
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
CSHDLNALocalMediaStreamController::~CSHDLNALocalMediaStreamController()
{
	Close();
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNALocalMediaStreamController::Open(const char* name, void* header)
{
	m_openFileName = name;

	int ret = _wfopen_s(&m_openFile,mbs2wcs(CP_UTF8,m_openFileName).c_str(),L"rb,ccs=UNICODE");
	if(!ret)
	{		
		fseek(m_openFile,0,SEEK_END);
		m_fileSize = ftell(m_openFile);
		m_available = m_fileSize;
		rewind(m_openFile);
	}

	return ret;
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNALocalMediaStreamController::Close()
{
	int ret = 0;
	if (m_openFile)
	{
		ret = fclose(m_openFile);
		m_openFile = NULL;
	}

	return ret;
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNALocalMediaStreamController::Read(void* buffer, unsigned int bytes_to_read, unsigned int* bytes_read)
{
	int ret = 0;
	if (m_openFile)
		*bytes_read = fread(buffer,1,bytes_to_read,m_openFile);
	else
		ret = -1;

	return ret;
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNALocalMediaStreamController::Seek(unsigned __int64 offset)
{
	int ret = 0;
	if (m_openFile)
		ret = fseek(m_openFile,(long)offset,SEEK_CUR);
	else
		ret = -1;

	return ret;
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNALocalMediaStreamController::Tell(unsigned __int64* offset)
{
	int ret =0;
	if (m_openFile)
	{
		*offset = ftell(m_openFile);
		ret = (int)*offset;
	}
	else
		ret = -1;
	return ret;
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNALocalMediaStreamController::GetSize(unsigned __int64* size)
{
	int ret = 0;
	if (m_openFile)
		*size = m_fileSize;
	else
		ret = -1;
	return ret;
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNALocalMediaStreamController::GetAvailable(unsigned __int64* available)
{
	int ret = 0;
	if (m_openFile)
		*available = m_available;
	else
		ret = -1;
	return ret;
}

/*----------------------------------------------------------------------
|   class CSHDLNAOnlineMediaStreamController
+---------------------------------------------------------------------*/
CSHDLNAOnlineMediaStreamController::CSHDLNAOnlineMediaStreamController()
{
	m_fileSize = 0;
	m_available = 0;
	m_firstOffset = 0;
	m_curPos = 0;
	m_reqPos = 0;

	m_status = 0;
	m_isSeek = false;
	m_isRun = true;
	m_handle = NULL;

	m_openFileName = "";
}

CSHDLNAOnlineMediaStreamController::~CSHDLNAOnlineMediaStreamController()
{
	Close();
}

int CSHDLNAOnlineMediaStreamController::Open(const char* name, void* header)
{
	int ret = 0;

	CHeaderInfo* headerInfo = (CHeaderInfo*)header;
	if (headerInfo == NULL)
		return -1;
	
	if( !headerInfo->cdninfo.empty() && headerInfo->sequence.GetSectionCount() == headerInfo->cdninfo.size() )
	{
		m_client.InitialRequest(&headerInfo->sequence,&headerInfo->info,&headerInfo->cdninfo);
		unsigned int filesize = 0;
		m_client.GetRequestFileInfo(&filesize);

		m_reqPos = 0;
		m_fileSize = filesize;
		m_available = filesize;
		SectionDataInfo tmpInfo;
		m_firstOffset = headerInfo->sequence.getSectionDataInfoByIndex(0, &tmpInfo);
		m_firstOffset = tmpInfo.ui32DataOffset;

		NPT_LOG_FATAL_1("Open success, filesize is %d\n.",filesize);
	}
	else
	{
		NPT_LOG_FATAL("Open failed, waiting for header.");
		return -1;
	}

	m_handle = (HANDLE)_beginthreadex( NULL,          
		0,              
		ThreadEntryPoint,  
		this,             
		0,    
		NULL );

	return ret;	
}

int CSHDLNAOnlineMediaStreamController::Close()
{
	{
		NPT_AutoLock lock(m_statusMutex);
		m_status = -1;
		NPT_LOG_FATAL("status set to -1");
	}

	m_reqPos = m_fileSize;
	m_isRun = false;
	WaitForSingleObject(m_handle,INFINITE);
	CloseHandle(m_handle);
	m_handle = NULL;

	{
		NPT_AutoLock lock(m_bufferMutex);
		while (m_bufferList.GetItemCount())
		{
			NPT_List<CDataBuffer>::Iterator it = m_bufferList.GetFirstItem();
			delete []it->buffer;
			it->buffer = NULL;
			m_bufferList.Erase(it);
		}
		m_bufferList.Clear();
	}
	m_client.FlushRequest();

	return 0;
}

int CSHDLNAOnlineMediaStreamController::Read(void* buffer, unsigned int bytes_to_read, unsigned int* bytes_read)
{
	if(!m_fileSize)
		return -1;

	unsigned char* ppBuffer = NULL;
	unsigned int pBufferSize(0);
	m_client.GetRequestFileHeader(&ppBuffer,&pBufferSize);
	if (pBufferSize == 0)
		return -1;

	if (m_curPos < pBufferSize)
	{
		unsigned int tmp_bytes = pBufferSize-static_cast<unsigned int>(m_curPos);
		*bytes_read = (bytes_to_read > tmp_bytes) ? tmp_bytes : bytes_to_read;
		memcpy(buffer,ppBuffer + m_curPos,*bytes_read);
		m_curPos += *bytes_read;

		NPT_String strRange="file size is ";
		strRange += NPT_String::FromIntegerU(m_fileSize);
		strRange += ",Header size is ";
		strRange += NPT_String::FromIntegerU(pBufferSize);
		strRange += ",current size is ";
		strRange += NPT_String::FromIntegerU(m_curPos);

		NPT_LOG_FATAL(strRange);
	}
	else
	{
		if (m_bufferList.GetItemCount())
		{
			NPT_List<CDataBuffer>::Iterator it = m_bufferList.GetFirstItem();

			if(it->readpos+bytes_to_read <= it->size)
			{
				*bytes_read = bytes_to_read;
				memcpy(buffer,it->buffer+it->readpos,*bytes_read);

			}
			else
			{
				*bytes_read = it->size-it->readpos;
				memcpy(buffer,it->buffer+it->readpos,*bytes_read);

			}

			it->readpos += *bytes_read;
			m_curPos += *bytes_read;

			NPT_String strRange = "File size is ";
			strRange += NPT_String::FromIntegerU(m_fileSize);
			strRange += ",header size is ";
			strRange += NPT_String::FromIntegerU(pBufferSize);
			strRange += ",current size is ";
			strRange += NPT_String::FromIntegerU(m_curPos);

			NPT_LOG_FATAL(strRange);

			{
				NPT_AutoLock lock(m_bufferMutex);
				if (it->readpos == it->size)
				{
					delete []it->buffer;
					it->buffer = NULL;
					m_bufferList.Erase(it);
				}
			}
			
		}
		else
		{
			*bytes_read = 0;
			::Sleep(100);
		}
	}

	return 0;
}

int CSHDLNAOnlineMediaStreamController::Seek(unsigned __int64 offset)
{
	int ret = 0;
	{
		NPT_AutoLock lock(m_statusMutex);
		m_status = -1;
	}

	if (m_fileSize)
	{
		m_curPos = offset;
		m_isSeek = true;
		NPT_LOG_FATAL_1("Seek position %u",m_curPos);
	}
	else
		ret = -1;

	return ret;
}

int CSHDLNAOnlineMediaStreamController::Tell(unsigned __int64* offset)
{
	int ret = 0;
	if (m_fileSize)
	{
		*offset = m_curPos;
		ret = (*offset);
		NPT_LOG_FATAL_1("Tell position %u\n",m_curPos);
	}
	else
		ret = -1;

	return ret;
}

int CSHDLNAOnlineMediaStreamController::GetSize(unsigned __int64* size)
{
	int ret = 0;
	if (m_fileSize)
	{
		*size = m_fileSize;
	}
	else
		ret = -1;

	return ret;
}

int CSHDLNAOnlineMediaStreamController::GetAvailable(unsigned __int64* available)
{
	int ret = 0;
	if (m_available)
	{
		*available = m_available;
	}
	else
		ret = -1;

	return ret;
}

int CSHDLNAOnlineMediaStreamController::ProgressCallBack(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	int status(0);
	if( clientp == NULL )
		status = 0;
	else
	{
		CEasyRequest* str = dynamic_cast<CEasyRequest*>((CEasyRequest *)clientp);
		if (str)
		{
			CSHDLNAOnlineMediaStreamController* pThis = dynamic_cast<CSHDLNAOnlineMediaStreamController*>((CSHDLNAOnlineMediaStreamController*)str->m_customparam);
			if (pThis)
			{
				status = pThis->m_status;
			}
			else
				status = 0;
		}
		else
			status = 0;
	}

	//NPT_LOG_FATAL_1("process status is %d\n",status);
	return status;
}

size_t CSHDLNAOnlineMediaStreamController::RecvMediaDataCallBackFunc(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	if (buffer == NULL)
		return 0;

	CEasyRequest* str = dynamic_cast<CEasyRequest*>((CEasyRequest *)lpVoid);
	CSHDLNAOnlineMediaStreamController* pThis = NULL;
	if (str)
	{
		pThis = dynamic_cast<CSHDLNAOnlineMediaStreamController*>((CSHDLNAOnlineMediaStreamController *)str->m_customparam);
	}

	if(str && str->m_response_datasize)
	{
		size_t copysize = size * nmemb;
		if (str->m_response_size + copysize > str->m_response_datasize)
		{
			copysize = str->m_response_datasize - str->m_response_size;
		}

		str->m_response_size += copysize;

		CDataBuffer recv_data;
		recv_data.size = copysize;
		recv_data.buffer = new unsigned char[copysize]();
		memcpy(recv_data.buffer,buffer,copysize);

		NPT_LOG_FATAL_1("receive data is %u",copysize);

		if (pThis)
		{
			NPT_AutoLock lock(pThis->m_bufferMutex);
			pThis->GetBufferList().Add(recv_data);
		}

	}
	else
	{
		CDataBuffer recv_data;
		recv_data.size = nmemb;
		recv_data.buffer = new unsigned char[nmemb]();
		memcpy(recv_data.buffer,buffer,nmemb);

		NPT_LOG_FATAL_1("receive data is %u",nmemb);

		if (pThis)
		{
			NPT_AutoLock lock(pThis->m_bufferMutex);
			pThis->GetBufferList().Add(recv_data);
		}
	}

	return nmemb;
}


void CSHDLNAOnlineMediaStreamController::RequestMediaData()
{    
	//m_status=0;
	unsigned int pBufferSize;
	m_client.GetRequestFileHeader(NULL,&pBufferSize);

	unsigned int startPos(0);

	//播放到结尾停止
	while (m_isRun)
	{
		// 用户seek了视频
		if (m_isSeek)
		{
			NPT_String msg = "seek event, pos is ";
			msg += NPT_String::FromIntegerU(m_curPos);
			NPT_LOG_FATAL(msg);

			startPos = m_curPos;
			m_isSeek = false;

			{
				NPT_AutoLock lock(m_statusMutex);
				m_status = 0;
			}

			if(startPos <= pBufferSize)
			{
				startPos = pBufferSize;
				if (startPos < m_firstOffset)
				{
					startPos = m_firstOffset;
					CDataBuffer data;
					data.size = m_firstOffset-pBufferSize;
					data.buffer = new unsigned char[m_firstOffset-pBufferSize]();

					NPT_AutoLock lock(m_bufferMutex);
					m_bufferList.Add(data);
				}
			}

			while(m_reqPos < m_fileSize)
			{
				if ( m_curPos>= m_reqPos )
				{
					int result ;
					{
						//NPT_AutoLock lock(m_headerMutex);
						result = m_client.DataRequest(startPos, startPos + 1 * 1024 *1024, 
							(unsigned int*)&m_reqPos,RecvMediaDataCallBackFunc,ProgressCallBack,this);
					}
					if (result >=  0)
					{
						startPos = m_reqPos;
					}
					else
					{
						break;
					}
				}
				else
					Sleep(100);
			}
		}
		else
			Sleep(100);
	}
}

unsigned __stdcall CSHDLNAOnlineMediaStreamController::ThreadEntryPoint(void * pThis)  
{  
	CSHDLNAOnlineMediaStreamController * pthX = (CSHDLNAOnlineMediaStreamController*)pThis;    
	if (pthX->m_fileSize)
	{
		pthX->RequestMediaData();
	}
	else
		NPT_LOG_FATAL("can't RequestMediaData");

	return 0;												   
}
