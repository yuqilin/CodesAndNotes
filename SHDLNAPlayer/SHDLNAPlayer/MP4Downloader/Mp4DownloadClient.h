#pragma once
#include <vector>
#include <atlstr.h>
#include <string>

#include "EasyRequest.h"
#include "QtSequenceMerge.h"

typedef enum tagEMediaDataRequestSource
{
	Invalid_RequestSource = -1,
	P2P_Local_RequestSource,
	CDN_Remote_RequestSource,
	Last_RequestSource
}EMediaDataRequestSource;

//视频版本
enum VIDEO_VERSION
{
	VER_ERROR	 = -1,
	VER_HIGH	 = 1,     //高清版
	VER_NORMAL   = 2,     //普清版
	VER_SUPER	 = 21 ,   //超清版
	VER_ORIGINAL = 31     //原画版
};

using namespace std;
using namespace ATL;
//看点信息
typedef struct tagSHVideoAspect
{
	int				time;	//时间
	CString	desc;	//描述
}SHVideoAspect;

//视频片段信息
typedef struct tagSHVideoSection
{
	double			duration;//时常
	int				size;	 //大小
	CString	url;	 //视频各片段文件地址
	CString	hashId;		 //hashId
	CString	key;		 //视频加密串, 防盗链
	CString	newAddress;	 //新架构视频片段地址
	CString	cachePath;	 //本地缓存地址
}SHVideoSection;

//视频信息
typedef struct tagSHVideoInfo
{
	int				p2pflag;	//传给加速器的参数
	int				vid;		//视频id
	int				norVid;		//对应的普清VID
	int				highVid;	//对应的普清VID
	int				superVid;	//对应的超清VID
	int             oriVid;     //原画vid    
	bool			longVideo;	//是否是长视频
	int				tn;			//请求调度时转发给调度服务器(参数名:cdn)
	int				status;		//视频信息状态, 1为正常
	bool			play;		//1为正常播放 0为禁播
	bool			fms;		//是否为FMS视频源
	bool			fee;		//是否为付费视频
	int				pid;
	int				cid;
	int				playerListId;
	int				fps;		//视频桢率
	int				version;	//视频版本, 1为高清 2为流畅
	int				num;		//该视频在专辑中的位置
	int				st;			//片头时长, 跳片头使用
	int				et;			//片尾时长, 跳片尾使用
	int             systype;    // 0:vrs类型 1：bms类型
	int				width;		//视频宽
	int				height;		//视频高
	CString			name;		//视频名称
	CString			ch;			//视频所属频道
	CString			allot;		//调度服务器地址
	CString			reserveIp;	//备用调度地址，以;分开，如:220.181.61.229;115.25.217.132
	CString			url;		//视频最终播放页
	CString            coverImg;   //视频封面图
	vector<SHVideoAspect>	aspects; //看点
	vector<SHVideoSection>	sections;//视频段信息
	CString			cachePath;
	CString            catcode; //vrs中的新分类代码
	//
	int				size;		 //整个视频的大小
	bool			complete;	 //是否下载完成
	int				index;		 //当前下载段
	int				downloadPos; //当前下载位置
	int				vidFromWeb;	 //从页面传过来的vid
	CString	uid;
	bool			preload;
	bool			downloadEnable;//是否允许下载
	bool            isMy;
	CString    source;

	bool IsPiracy() const
	{
		return  !source.IsEmpty() && (source.GetAt(0) == TCHAR('x') || source.GetAt(0) == TCHAR('X'));
	}

	CString GetUrl() const 
	{
		CString playUrl;
		if(!IsPiracy())
		{
			if(isMy)
				playUrl.Format(_T("http://tv.sohu.com/ifoxplay/?ismytv=1&vid=%d"),vid);
			else
				playUrl.Format(_T("http://tv.sohu.com/ifoxplay/?pid=%d&vid=%d"),pid,vid);
		}
		else
		{
			if(isMy)
				playUrl.Format(_T("http://ent1.3322.org/videoplay.html?ismytv=1&vid=%d"),vid);
			else
				playUrl.Format(_T("http://ent1.3322.org/videoplay.html?pid=%d&vid=%d"),pid,vid);
		}
		return playUrl;
	}

	tagSHVideoInfo()
	{
		p2pflag		= 0;
		vid			= 0;
		norVid		= 0;
		highVid		= 0;
		superVid	= 0;
		oriVid      = 0;
		longVideo	= false;
		tn			= 0;
		status		= 0;
		play		= false;
		fms			= false;
		fee			= false;
		pid			= 0;
		fps			= 0;
		version		= 0;
		num			= 0;
		st			= 0;
		et			= 0;
		size		= 0;
		complete	= false;
		index		= 0;
		downloadPos	= 0;
		cid			= 0;
		playerListId= 0;
		vidFromWeb	= 0;
		width		= 0;
		height		= 0;
		preload		= false;
		downloadEnable = true;
		isMy        = false;
	}
}SHVideoInfo;

//CDN信息
typedef struct tagSHCDNInfo
{
	wstring	ip;
	wstring	key;
	wstring	url;
	int		idc;
	tagSHCDNInfo(const std::wstring& strIp,const wstring& strKey,const wstring& strUrl,int idc = 0)
	{
		ip	= strIp;
		key	= strKey;
		url = strUrl;
		idc = idc;
	}
	tagSHCDNInfo()
	{}
	bool operator==(const tagSHCDNInfo &sp) const
	{
		return ip == sp.ip && key == sp.key && url == sp.url && idc == sp.idc;
	}
}SHCDNInfo;

enum SHDType
{
	SHDType_Normal = 0,
	SHDType_Preload,
	SHDType_PreloadNext,
	SHDType_Download,
	SHDType_ClientPlay,
	SHDType_LivePlay,
	SHDType_TestSpeed,
	SHDType_UserTestSpeed
};

//视频请求参数
typedef struct tagSHVideoRequestParam
{
	int		index;
	bool	download;
	int		pnum;			//当前播放段
	int		dnum;			//当前下载段
	int		ptime;			//当前播放时间
	int		dtime;			//当前下载时间
	int		start;			//开始时间
	int		startpos;		//开始位置
	int		cdnNum;
	int		shdtype;		//下载类型
	int     duration;		//时长，单位s
	int		p2pflag;
	bool	reportLocalData;
	tagSHVideoRequestParam()
	{
		memset(this,0,sizeof(tagSHVideoRequestParam));
		p2pflag = 3*60;
		reportLocalData = true;
	}
}SHVideoRequestParam;

class CMp4DownloadClient
{
public:
	CMp4DownloadClient(void);
	~CMp4DownloadClient(void);

	void InitialRequest(QtSequence* sequence_ptr, SHVideoInfo* info_ptr, vector<SHCDNInfo>* cdnInfolist_ptr, 
		EMediaDataRequestSource eTryResoure = P2P_Local_RequestSource);

	int HeaderRequest(const char* url, 
		CString& mainfilename,
		ProgressCallBack hotvr_processcb, 
		ProgressCallBack cdn_processcb, 
		ProgressCallBack header_processcb);

	void GetRequestFileInfo(unsigned int* pfilesize);
	
	void GetRequestFileHeader(unsigned char** ppbuffer, unsigned int* pbuffersize);

	int DataRequest(unsigned int startpos, unsigned int endpos, unsigned int* pactualfinishpos, 
		ResponseCallBack cb, ProgressCallBack data_progresscb, void* customparam = NULL);
	
	void FlushRequest(void);

	int GetFileName(const char* url,CString& mainfilename);
	static int ProgressCB(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);

private:
	wstring Utf82W(LPCSTR szContent);
	bool JsonParse(const void* xml, const unsigned int xmlsize, SHVideoInfo& videoinfo);
	bool XmlParse(const void* buffer, const unsigned int buffersize, SHCDNInfo& cdnInfo);
	wstring GetUuid();

protected:
	static size_t HotVrsResponseForNameCallBackFunc(void* buffer, size_t size, size_t nmemb, void* lpVoid);
	static size_t HotVrsResponseCallBackFunc(void* buffer, size_t size, size_t nmemb, void* lpVoid);
	static size_t CdnResponseCallBackFunc(void* buffer, size_t size, size_t nmemb, void* lpVoid);
	static size_t HeaderResponseCallBackFunc(void* buffer, size_t size, size_t nmemb, void* lpVoid);
	static size_t DataResponseCallBackFunc(void* buffer, size_t size, size_t nmemb, void* lpVoid);
	static int DataDebugCallBackFunc(CURL *, curl_infotype itype, char * pData, size_t size, void *);

	static string* hotvrs_string_ptr_for_name;
	static string* hotvrs_string_ptr;
	static string* cdn_string_ptr;
	static string* header_string_ptr;

private:
	QtSequence* m_sequence_ptr;
	SHVideoInfo* m_info_ptr;
	vector<SHCDNInfo>* m_cdnInfolist_ptr;

	EMediaDataRequestSource m_eResoureType;
};
