#ifndef __FLYFOX_COMMON_H__
#define __FLYFOX_COMMON_H__

/************************************************/
/* 
	文件管理模块提供的回调函数
	返回 0 代表成功， 否则失败
*/
/************************************************/
typedef int (*MessageNotifyCB)(const void* , int msg, void* wParam, void* lParam);

//选择数据的下载模式
typedef enum DownloadType_t
{
	UDPType,							//udp下载模式（默认）
	TCPType								//tcp下载模式
}DownloadType;

//选择是播放还是下载
typedef enum DownMediaMode_t
{
	DownMode,							//下载模式
	PlayMode							//播放模式
}DownMediaMode;

//定义回调函数消息
typedef enum DownloadMessageType_t
{
	MESSAGE_MEDIA_INFO,					//文件的torrent信息，包括：文件的大小，piece大小，piece的hash值
	MESSAGE_MEDIA_P2P_DATA,				//P2P下载数据，包括：16k数据的绝对位置，16k数据内容
	MESSAGE_MEDIA_CDN_DATA,				//CDN下载数据，包括：16k数据的绝对位置，16k数据内容
	MESSAGE_MEDIA_STAT,					//当前下载状态，包括：下载数据总大小，下载分片位置等信息
	MESSAGE_MEDIA_FINI,					//下载结束
	MESSAGE_MEDIA_NET_ERROR				//网络出错，如无法解析域名，无法上网,无法得到peer
}DownloadMessageType;

typedef struct CDNDataPos_t
{
	unsigned long abspos;				//数据所在文件的绝对位置
	unsigned long size;					//要写入数据大小
}CDNDataPos;

//通过分析torrent文件得到文件信息
typedef struct FileTorrentInfo_t
{
	unsigned long	filelength;			//文件总大小
	unsigned long	piecelength;		//文件分片大小
	unsigned long	piecesize;			//文件总分片个数
	unsigned char*	piecehash;			//每个分片hash值
}FileTorrentInfo;


#endif
