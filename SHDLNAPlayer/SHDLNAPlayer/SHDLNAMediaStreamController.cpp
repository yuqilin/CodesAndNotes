
#include "stdafx.h"
#include <assert.h>
#include "SHDLNAMediaStreamController.h"

//
bool CSHDLNAMediaStreamController::m_isReqHeader=true;

CHeaderInfo* CSHDLNAMediaStreamController::m_headerInfo=NULL;

NPT_String CSHDLNAMediaStreamController::m_lastFileName;

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
CSHDLNAMediaStreamController::CSHDLNAMediaStreamController()
{
	m_fileSize=0;
	m_available=0;
	m_curPos=0;
	m_status=0;

	m_reqPos=0;
	m_isSeek=false;
	m_isRun=true;

	m_handle=NULL;

	m_openFile=NULL;

	m_type=SHDLNAPLAYER_MEDIA_UNKNOWN;

	m_openFileName="";
	m_playFileName="";
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
int CSHDLNAMediaStreamController::Open(const char* name, void* header_data)
{
	int ret=0;

	m_openFileName=name;

	if (m_openFileName.Find("vid")!=-1&&m_openFileName!=m_lastFileName)
	{
		m_isReqHeader = true;
		m_lastFileName=name;
	}

	if (m_openFileName.StartsWith("http:")||m_openFileName.StartsWith("https:"))
	{
		m_type=SHDLNAPLAYER_MEDIA_ONLINE;
	}
	else
	{
		m_type=SHDLNAPLAYER_MEDIA_LOCAL;
	}

	if (m_type==SHDLNAPLAYER_MEDIA_LOCAL)
	{
		ret=fopen_s(&m_openFile,m_openFileName,"rb");
		if(!ret)
		{		
			fseek(m_openFile,0,SEEK_END);
			m_fileSize=ftell(m_openFile);
			m_available=m_fileSize;
			rewind(m_openFile);
		}
	}
	else if (m_type==SHDLNAPLAYER_MEDIA_ONLINE)
	{
		//int result = m_client.InitialRequest(m_openFileName.GetChars(),RecvMediaDataCallBackFunc, IsP2P());
		if (m_isReqHeader)
		{
			m_isReqHeader=false;
			if (m_headerInfo)
			{
				delete m_headerInfo;
				m_headerInfo=NULL;
			}
			m_headerInfo=new CHeaderInfo();
			m_client.InitialRequest(&m_headerInfo->sequence,&m_headerInfo->info,&m_headerInfo->cdninfo);
			m_status=0;

			int result = m_client.HeaderRequest(m_openFileName.GetChars(),m_playFileName,ProgressCallBack,ProgressCallBack,ProgressCallBack);

			if (result<0)
			{
				assert(false);
			}

			unsigned int filesize = 0;
			m_client.GetRequestFileInfo(&filesize);
			//m_headerInfo->sequence.getHeaderBuffer(NULL,(unsigned int*)&m_reqPos);
			m_reqPos=0;
			m_fileSize=filesize;
		}
		else
		{
			if(m_headerInfo->sequence.CheckAnalyzeStatus()==false)
				return -1;
			m_client.InitialRequest(&m_headerInfo->sequence,&m_headerInfo->info,&m_headerInfo->cdninfo);

			unsigned int filesize = 0;
			m_client.GetRequestFileInfo(&filesize);
			//m_headerInfo->sequence.getHeaderBuffer(NULL,(unsigned int*)&m_reqPos);
			m_reqPos=0;
			m_fileSize=filesize;
		}
		m_handle=(HANDLE)_beginthreadex( NULL,          
			0,              
			ThreadEntryPoint,  
			this,             
			0,    
			NULL );  

	}
	return ret;
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNAMediaStreamController::Close()
{
	int ret=0;
	if (m_type==SHDLNAPLAYER_MEDIA_LOCAL)
	{
		if (m_openFile)
		{
			ret=fclose(m_openFile);
			m_openFile=NULL;
		}
	}
	else if(m_type==SHDLNAPLAYER_MEDIA_ONLINE)
	{
		m_status=-1;
		m_isRun=false;

		WaitForSingleObject(m_handle,INFINITE);

		NPT_AutoLock lock(m_mutex);
		while (m_bufferList.GetItemCount())
		{
			NPT_List<CDataBuffer>::Iterator it=m_bufferList.GetFirstItem();
			delete []it->buffer;
			it->buffer=NULL;
			m_bufferList.Erase(it);
		}
		m_bufferList.Clear();
		m_client.FlushRequest();
	}

	return ret;
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNAMediaStreamController::Read(void* buffer, unsigned int bytes_to_read, unsigned int* bytes_read)
{
	int ret=0;
	if (m_type == SHDLNAPLAYER_MEDIA_LOCAL)
	{
		if (m_openFile)
			*bytes_read=fread(buffer,1,bytes_to_read,m_openFile);
	}
	else if (m_type == SHDLNAPLAYER_MEDIA_ONLINE)
	{
		if(!m_fileSize)
			return -1;
		unsigned char* ppBuffer=NULL;
		unsigned int pBufferSize(0);
		m_headerInfo->sequence.getHeaderBuffer(&ppBuffer,&pBufferSize);
		if (m_curPos<pBufferSize)
		{
			unsigned int tmp_bytes=pBufferSize-static_cast<unsigned int>(m_curPos);
			*bytes_read= (bytes_to_read>tmp_bytes) ? tmp_bytes : bytes_to_read;
			memcpy(buffer,ppBuffer+m_curPos,*bytes_read);
			m_curPos+=*bytes_read;

		}
		else
		{
			if (m_bufferList.GetItemCount())
			{
				NPT_List<CDataBuffer>::Iterator it=m_bufferList.GetFirstItem();

				if(it->readpos+bytes_to_read<=it->size)
				{
					*bytes_read=bytes_to_read;
					memcpy(buffer,it->buffer+it->readpos,*bytes_read);

				}
				else
				{
					*bytes_read=it->size-it->readpos;
					memcpy(buffer,it->buffer+it->readpos,*bytes_read);

				}
				it->readpos+=*bytes_read;
				m_curPos+=*bytes_read;
				NPT_AutoLock lock(m_mutex);
				if (it->readpos==it->size)
				{
					delete []it->buffer;
					it->buffer=NULL;
					m_bufferList.Erase(it);
				}
			}
			else
			{
				*bytes_read=0;
			}
		}
	}
	return ret;
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNAMediaStreamController::Seek(unsigned __int64 offset)
{
	int ret=0;
	if (m_type == SHDLNAPLAYER_MEDIA_LOCAL)
	{
		if (m_openFile)
			ret=fseek(m_openFile,(long)offset,SEEK_CUR);
	}
	else if (m_type == SHDLNAPLAYER_MEDIA_ONLINE)
	{
		m_status=-1;
		if (m_fileSize)
		{
			m_curPos=offset;
			m_isSeek=true;
		}
		else
			ret=-1;
	}
	return ret;
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNAMediaStreamController::Tell(unsigned __int64* offset)
{
	int ret=0;
	if (m_type == SHDLNAPLAYER_MEDIA_LOCAL)
	{
		//On failure, -1L is returned
		if (m_openFile)
		{
			*offset=ftell(m_openFile);
		}
		return (int)*offset;
	}
	else if (m_type == SHDLNAPLAYER_MEDIA_ONLINE)
	{
		if (m_fileSize)
		{
			*offset=m_curPos;
		}
		else
			ret=-1;
	}
	return ret;
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNAMediaStreamController::GetSize(unsigned __int64* size)
{
	if (!m_fileSize)
	{
		unsigned int filesize = 0;
		m_client.GetRequestFileInfo(&filesize);
		m_fileSize=filesize;

		*size = filesize;

	}
	else
		*size = m_fileSize;
	return 0;
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNAMediaStreamController::GetAvailable(unsigned __int64* available)
{
	*available=m_available;
	return 0;
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNAMediaStreamController::ProgressCallBack(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	CEasyRequest* str = dynamic_cast<CEasyRequest*>((CEasyRequest *)clientp);
	if (str)
	{
		CSHDLNAMediaStreamController* pThis = dynamic_cast<CSHDLNAMediaStreamController*>((CSHDLNAMediaStreamController *)str->m_customparam);
		if (pThis)
		{
			return pThis->m_status;
		}
	}

	return 0;
}

size_t CSHDLNAMediaStreamController::RecvMediaDataCallBackFunc(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	if (buffer == NULL)
		return 0;

	CEasyRequest* str = dynamic_cast<CEasyRequest*>((CEasyRequest *)lpVoid);
	CSHDLNAMediaStreamController* pThis=NULL;
	if (str)
	{
		pThis = dynamic_cast<CSHDLNAMediaStreamController*>((CSHDLNAMediaStreamController *)str->m_customparam);
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
		recv_data.size=copysize;
		recv_data.buffer=new unsigned char[copysize]();
		memcpy(recv_data.buffer,buffer,copysize);

		if (pThis)
		{
			NPT_AutoLock lock(pThis->m_mutex);
			pThis->GetBufferList().Add(recv_data);
		}

	}
	else
	{
		CDataBuffer recv_data;
		recv_data.size=nmemb;
		recv_data.buffer=new unsigned char[nmemb]();
		memcpy(recv_data.buffer,buffer,nmemb);

		if (pThis)
		{
			NPT_AutoLock lock(pThis->m_mutex);
			pThis->GetBufferList().Add(recv_data);
		}
	}

	return nmemb;
}


void CSHDLNAMediaStreamController::RequestMediaData()
{    
	//m_status=0;
	unsigned int pBufferSize;
	m_headerInfo->sequence.getHeaderBuffer(NULL,&pBufferSize);

	_uint32_t startPos= m_reqPos > pBufferSize ? m_reqPos : pBufferSize; 
	m_reqPos=pBufferSize;

	//播放到结尾停止
	while (m_isRun&&startPos<m_fileSize)
	{
		m_status=0;
		//播放完一个请求的分段
		if (m_curPos==m_reqPos)
		{
			if (startPos<pBufferSize)
				startPos=pBufferSize;
			if (startPos>m_fileSize)
				startPos=m_fileSize;
			int result = m_client.DataRequest(startPos, startPos + 1 * 1024 *1024, 
				(unsigned int*)&m_reqPos,RecvMediaDataCallBackFunc,ProgressCallBack,this);
			if (result>=0)
				startPos=m_reqPos;	
		}
		//用户seek了视频
		if (m_isSeek)
		{
			//清空队列
			{
				NPT_AutoLock lock(m_mutex);
				while (m_bufferList.GetItemCount())
				{
					NPT_List<CDataBuffer>::Iterator it=m_bufferList.GetFirstItem();
					delete []it->buffer;
					it->buffer=NULL;
					m_bufferList.Erase(it);
				}
				m_bufferList.Clear();
			}

			startPos=m_curPos;
			m_isSeek=false;
			if(startPos<pBufferSize)
				startPos=pBufferSize;

			int result = m_client.DataRequest(startPos, startPos + 1 * 1024 *1024, 
				(unsigned int*)&m_reqPos,RecvMediaDataCallBackFunc,ProgressCallBack,this);
			if (result>=0)
			{
				startPos=m_reqPos;
			}
		}
	}
}

unsigned __stdcall CSHDLNAMediaStreamController::ThreadEntryPoint(void * pThis)  
{  
	CSHDLNAMediaStreamController * pthX = (CSHDLNAMediaStreamController*)pThis;    
	if (pthX->m_fileSize)
	{
		pthX->RequestMediaData();
	}

	return 0;												   
}
