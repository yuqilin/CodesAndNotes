#ifndef _SH_DLNA_MEDIA_STREAM_CONTROLLER_H_
#define _SH_DLNA_MEDIA_STREAM_CONTROLLER_H_

#include "SHDLNAMediaStream.h"
#include "Mp4DownloadClient.h"

//存储头信息
struct CHeaderInfo 
{
	QtSequence sequence;
	SHVideoInfo info;
	vector<SHCDNInfo> cdninfo;
	~CHeaderInfo()
	{
		vector<SHCDNInfo>().swap(cdninfo);
	}
};

//存储数据单元
struct CDataBuffer
{
	unsigned char* buffer;
	unsigned int size;
	unsigned int readpos;
	CDataBuffer():buffer(NULL),size(0),readpos(0){};
};

//播放数据类型
typedef enum {
	SHDLNAPLAYER_MEDIA_UNKNOWN					= 0,		// 未知媒体
	SHDLNAPLAYER_MEDIA_LOCAL					= 1,		// 本地媒体
	SHDLNAPLAYER_MEDIA_ONLINE					= 2		    // 在线媒体
} SHDLNAPlayer_MediaType;

/*----------------------------------------------------------------------
|   class CSHDLNAMediaStreamController
+---------------------------------------------------------------------*/
class CSHDLNAMediaStreamController : public IMediaStream
{
	friend class CMp4DownloadClient;

public:
	CSHDLNAMediaStreamController();
	virtual ~CSHDLNAMediaStreamController();

	int Open(const char* name);
	int Close();
	int Read(void* buffer, unsigned int bytes_to_read, unsigned int* bytes_read);
	int Seek(unsigned __int64 offset);
	int Tell(unsigned __int64* offset);
	int GetSize(unsigned __int64* size);
	int GetAvailable(unsigned __int64* available);

	static size_t RecvMediaDataCallBackFunc(void* buffer, size_t size, size_t nmemb, void* lpVoid);
	static int ProgressCallBack(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
	static unsigned __stdcall ThreadEntryPoint(void * pThis);

public:
	//0 执行，-1 停止
	int m_status;
	NPT_List<CDataBuffer>& GetBufferList(){		return m_bufferList;	}
	NPT_Mutex m_mutex;

protected:
	bool RecvHeaderData(NPT_HttpUrl url,NPT_DataBuffer& buffer);
	bool RecvMediaData(NPT_HttpUrl url,int in_nStartPos, int in_nEndPos,NPT_DataBuffer& buffer);
private:
	void RequestMediaData();

	unsigned __int64 m_fileSize;
	unsigned __int64 m_available;
	unsigned __int64 m_curPos;
	//当前已请求到的数据位置
	unsigned __int64 m_reqPos;

	//是否发生seek
	bool m_isSeek;
	//停止线程执行
	bool m_isRun;

	HANDLE m_handle;

	//文件信息
	FILE *m_openFile;
	NPT_String m_openFileName;
	CString m_playFileName;
	static NPT_String m_lastFileName;
	SHDLNAPlayer_MediaType m_type;

	//http请求客户端
	CMp4DownloadClient m_client;

	//数据列表
	//保证只请求一次头
	static bool m_isReqHeader;
	//存储头结构
	static CHeaderInfo* m_headerInfo;
	//存储数据队列
	NPT_List<CDataBuffer> m_bufferList;
};


#endif // _SH_DLNA_MEDIA_STREAM_CONTROLLER_H_