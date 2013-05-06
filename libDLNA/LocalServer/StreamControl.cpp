#include "StreamControl.h"

CStreamCtrl::CStreamCtrl()
{
	m_fileSize=0;
	m_available=0;
	m_curPos=0;
	m_openFile=NULL;
	m_openFileName=NULL;
	m_sSequence=NULL;
}

CStreamCtrl::~CStreamCtrl()
{
	Close();
}

//0 for success
int CStreamCtrl::Open(const char* name, int file_type)
{
	int ret=0;
	m_type=file_type;
	m_openFileName=const_cast<char*>(name);
	if (m_type==1)
	{
		ret=fopen_s(&m_openFile,name,"rb");
		if(!ret)
		{		
			fseek(m_openFile,0,SEEK_END);
			m_fileSize=ftell(m_openFile);
			m_available=m_fileSize;
			rewind(m_openFile);
		}
	}
	else if (m_type==2)
	{
		NPT_String path="/";
		path+=m_openFileName;
		NPT_HttpUrl url=NPT_HttpUrl(CDNIPAddress, CDNPort, path.GetChars());
		NPT_DataBuffer buffer;
		bool ret=true;

		ret=RecvHeaderData(url,buffer);
		if (ret)
		{
			NPT_Byte* pdata=const_cast<unsigned char*>(buffer.GetData());

			SequenceInfo sInfo;
			unsigned int tmp_size(0);
			memcpy(&tmp_size,pdata,4);
			unsigned int length=4;
			unsigned int inum=sInfo.nSectionCount=tmp_size;
			sInfo.pSectioninfo=new SectionInfo[inum];
			pdata+=4;
			for (unsigned int i=0;i<inum;++i)
			{
				memcpy(&tmp_size,pdata,4);
				pdata+=4;
				sInfo.pSectioninfo[i].ui32SectionInfoBufferSize=tmp_size;
				sInfo.pSectioninfo[i].pSectionInfoBuffer=pdata;
				pdata+=tmp_size;
			}

			m_sSequence=new QtSequence(&sInfo);

			_uint32_t ui32FileSize = 0;
			m_sSequence->getFileSize(&ui32FileSize);
			m_fileSize=ui32FileSize;
			m_available=ui32FileSize;
		}
	}
	return ret;
}

//0 for success
int CStreamCtrl::Close()
{
	int ret=0;
	if (m_type==1)
	{
		if (m_openFile)
		{
			ret=fclose(m_openFile);
		}
	}
	else if(m_type==2)
	{
		if (m_sSequence)
		{
			delete m_sSequence;
		}
	}
	m_openFileName=NULL;

	return ret;
}

int CStreamCtrl::Read(void* buffer, unsigned int bytes_to_read, unsigned int* bytes_read)
{
	int ret=0;
	if (m_type==1)
	{
		*bytes_read=fread(buffer,1,bytes_to_read,m_openFile);
	}
	else if (m_type==2)
	{
		_pbyte_t pHeaderBuffer = NULL;
		_uint32_t ui32HeaderBufferSize = 0;
		m_sSequence->getHeaderBuffer(&pHeaderBuffer,&ui32HeaderBufferSize);
		if (m_curPos<ui32HeaderBufferSize)
		{
			unsigned int tmp_bytes=ui32HeaderBufferSize-static_cast<unsigned int>(m_curPos);
			*bytes_read= (bytes_to_read>tmp_bytes) ? tmp_bytes : bytes_to_read;
			pHeaderBuffer+=m_curPos;
			memcpy(buffer,pHeaderBuffer,*bytes_read);
			m_curPos+=*bytes_read;
		}
		else
		{
			_uint32_t pos=static_cast<unsigned int>(m_curPos);
			_uint32_t startclipnum = 0;
			_uint32_t endclipnum = 0;

			if (pos + bytes_to_read <= m_fileSize)
				m_sSequence->TranslateSectionRange(pos, pos+bytes_to_read, &startclipnum, &endclipnum);
			else
				m_sSequence->TranslateSectionRange(pos, m_fileSize, &startclipnum, &endclipnum);
			unsigned int index(0);
			for (_uint32_t j = startclipnum; j <= endclipnum; j++)
			{
				ClipOfRangeInfo sClipInfo;
				m_sSequence->getSectionInfoByIndex(j, &sClipInfo);
				char query[10]={};

				NPT_String path="/";
				path+=m_openFileName;

				NPT_HttpUrl url=NPT_HttpUrl(P2PIPAddress, 8082, path);
				_itoa_s(j,query,10);
				url.SetQuery(query);

				NPT_DataBuffer mediadata;
				RecvMediaData(url,sClipInfo.StartOffset,sClipInfo.EndOffset,mediadata);	

				memcpy((unsigned char*)buffer+index,mediadata.GetData(),mediadata.GetBufferSize());
				index+=mediadata.GetBufferSize();
				m_curPos+=mediadata.GetBufferSize();
			}
			*bytes_read=index;
		}
	}
	return ret;
}

//0 for success
int CStreamCtrl::Seek(unsigned __int64 offset)
{
	int ret=0;
	if (m_type==1)
	{
		ret=fseek(m_openFile,(long)offset,SEEK_CUR);
	}
	else if (m_type==2)
	{
		m_curPos=offset;
	}
	return ret;
}
int CStreamCtrl::Tell(unsigned __int64* offset)
{
	int ret=0;
	if (m_type==1)
	{
		//On failure, -1L is returned
		*offset=ftell(m_openFile);
		return (int)*offset;
	}
	else if (m_type==2)
	{
		*offset=m_curPos;
	}
	return ret;
}

int CStreamCtrl::GetSize(unsigned __int64* size)
{
	*size = m_fileSize;
	return 0;
}

int CStreamCtrl::GetAvailable(unsigned __int64* available)
{
	*available=m_available;
	return 0;
}

bool CStreamCtrl::RecvHeaderData(NPT_HttpUrl url,NPT_DataBuffer& buffer)
{
	NPT_HttpClient client;
	NPT_String rdUrl=url.ToString();
	// first request
	NPT_HttpRequest request(url, NPT_HTTP_METHOD_GET, NPT_HTTP_PROTOCOL_1_1);

	NPT_HttpResponse* response = NULL;
	client.SendRequest(request, response);
	NPT_HttpEntity* entity = NULL;
	if (response && (entity = response->GetEntity())) {
		if (NPT_FAILED(entity->Load(buffer))) return false;
	}
	else
		return false;

	return true;
}

bool CStreamCtrl::RecvMediaData(NPT_HttpUrl url,int in_nStartPos, 
									int in_nEndPos,NPT_DataBuffer& buffer)
{
	NPT_HttpClient client;
	NPT_String rdUrl=url.ToString();
	// first request
	NPT_HttpRequest request(url, NPT_HTTP_METHOD_GET, NPT_HTTP_PROTOCOL_1_1);
	char range[100]={0};
	sprintf(range," bytes=%d-%d",in_nStartPos,in_nEndPos-1);
	request.GetHeaders().SetHeader(NPT_HTTP_HEADER_RANGE,range);

	NPT_HttpResponse* response = NULL;
	client.SendRequest(request, response);
	NPT_HttpEntity* entity = NULL;
	if (response && (entity = response->GetEntity())) {
		if (NPT_FAILED(entity->Load(buffer))) return false;
	}
	else
		return false;

	return true;
}