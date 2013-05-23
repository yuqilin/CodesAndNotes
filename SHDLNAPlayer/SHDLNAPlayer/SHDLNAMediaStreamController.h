#ifndef _SH_DLNA_MEDIA_STREAM_CONTROLLER_H_
#define _SH_DLNA_MEDIA_STREAM_CONTROLLER_H_

#include "SHDLNAMediaStream.h"
#include "Mp4DownloadClient.h"

//播放数据类型
typedef enum {
	SHDLNAPLAYER_MEDIA_UNKNOWN	= 0,		// 未知媒体
	SHDLNAPLAYER_MEDIA_LOCAL	= 1,		// 本地媒体
	SHDLNAPLAYER_MEDIA_ONLINE	= 2		    // 在线媒体
} SHDLNAPlayer_MediaType;

//存储头信息
struct CHeaderInfo 
{
	QtSequence			sequence;
	SHVideoInfo			info;
	vector<SHCDNInfo>	cdninfo;
	~CHeaderInfo()
	{
		vector<SHCDNInfo>().swap(cdninfo);
	}
};

//存储数据单元
struct CDataBuffer
{
	unsigned char*	buffer;
	unsigned int	size;
	unsigned int	readpos;
	CDataBuffer():buffer(NULL),size(0),readpos(0){};
};

class CSHDLNALocalMediaStreamController;
class CSHDLNAOnlineMediaStreamController;

/*----------------------------------------------------------------------
|   class CSHDLNAMediaStreamController
+---------------------------------------------------------------------*/
class CSHDLNAMediaStreamController : public IMediaStream
{
public:
	CSHDLNAMediaStreamController();
	virtual ~CSHDLNAMediaStreamController();

	int Open(const char* name, void* header);
	int Close();
	int Read(void* buffer, unsigned int bytes_to_read, unsigned int* bytes_read);
	int Seek(unsigned __int64 offset);
	int Tell(unsigned __int64* offset);
	int GetSize(unsigned __int64* size);
	int GetAvailable(unsigned __int64* available);

private:

	SHDLNAPlayer_MediaType				m_type;
	CSHDLNALocalMediaStreamController*	m_localMedia;
	CSHDLNAOnlineMediaStreamController*	m_onlineMedia;
};


/*----------------------------------------------------------------------
|   class CSHDLNALocalMediaStreamController
+---------------------------------------------------------------------*/
class CSHDLNALocalMediaStreamController
{
public:
	CSHDLNALocalMediaStreamController();
	~CSHDLNALocalMediaStreamController();

	int Open(const char* name, void* header);
	int Close();
	int Read(void* buffer, unsigned int bytes_to_read, unsigned int* bytes_read);
	int Seek(unsigned __int64 offset);
	int Tell(unsigned __int64* offset);
	int GetSize(unsigned __int64* size);
	int GetAvailable(unsigned __int64* available);
private:
	//文件信息
	FILE*			 m_openFile;
	NPT_String		 m_openFileName;

	unsigned __int64 m_fileSize;
	unsigned __int64 m_available;
};

/*----------------------------------------------------------------------
|   class CSHDLNAOnlineMediaStreamController
+---------------------------------------------------------------------*/
class CSHDLNAOnlineMediaStreamController
{
public:
	CSHDLNAOnlineMediaStreamController();
	~CSHDLNAOnlineMediaStreamController();

	int Open(const char* name, void* header);
	int Close();
	int Read(void* buffer, unsigned int bytes_to_read, unsigned int* bytes_read);
	int Seek(unsigned __int64 offset);
	int Tell(unsigned __int64* offset);
	int GetSize(unsigned __int64* size);
	int GetAvailable(unsigned __int64* available);

	NPT_List<CDataBuffer>&		GetBufferList(){		return m_bufferList;	}
	static size_t				RecvMediaDataCallBackFunc(void* buffer, size_t size, 
		size_t nmemb, void* lpVoid);
	static int					ProgressCallBack(void *clientp, double dltotal, 
		double dlnow, double ultotal, double ulnow);
	static unsigned __stdcall	ThreadEntryPoint(void * pThis);

public:
	//0 执行，-1 停止
	int m_status;

	NPT_Mutex m_bufferMutex;
	NPT_Mutex m_statusMutex;
	NPT_Mutex m_headerMutex;
private:
	void					RequestMediaData();
	HANDLE					m_handle;
	unsigned int			m_firstOffset;
	unsigned __int64		m_fileSize;
	unsigned __int64		m_available;
	unsigned __int64		m_curPos;
	unsigned __int64		m_reqPos;
	bool					m_isSeek;
	bool					m_isRun;
	//文件信息
	NPT_String				m_openFileName;
	CMp4DownloadClient		m_client;
	NPT_List<CDataBuffer>	m_bufferList;
};



#endif // _SH_DLNA_MEDIA_STREAM_CONTROLLER_H_
