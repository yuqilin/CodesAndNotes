#ifndef _MP4_DOWNLOAD_TASK_H_
#define _MP4_DOWNLOAD_TASK_H_

#include <atlstr.h>
#include <vector>

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

class CMP4DownloadTask : public PLT_ThreadTask
{
public:
	
	CMP4DownloadTask(const char* url);
	virtual ~CMP4DownloadTask();

protected:

	virtual void DoRun();


protected:
	NPT_String m_Url;
	PLT_Downloader* m_Downloader;
};

#endif