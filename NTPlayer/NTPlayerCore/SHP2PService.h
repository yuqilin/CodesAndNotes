#ifndef _SH_P2PSERVICE_H_
#define _SH_P2PSERVICE_H_

#include "flyfox_common.h"

#ifdef SHP2PSERVICE_EXPORTS
#define SH_P2PSERVICE_API __declspec(dllexport)
#else
#define SH_P2PSERVICE_API __declspec(dllimport)
#endif

#ifdef _cplusplus
extern "C" 
{
#endif
	class  SHP2PService
	{
	public:

		/************************************************/
		/* 
			打开播放地址
			返回 0 代表成功， 否则失败
		*/
		/************************************************/
		virtual int	Open(const std::string& a_url, const std::string& a_cfgname, 
						 MessageNotifyCB a_msg_cb, DownMediaMode a_downmode, 
						 DownloadType a_type, const void* a_userdata) = 0;
	    
		/************************************************/
		/* 
			关闭数据下载
			返回 0 代表成功， 否则失败
		*/
		/************************************************/
		virtual int	Close() = 0;
	    
		/************************************************/
		/* 
			播放时触发的seek事件
			返回 0 代表成功， 否则失败
		*/
		/************************************************/
		virtual int	Seek(unsigned long a_seekpos, bool getheader = false) = 0;
	    
		/************************************************/
		/* 
			读取一定大小的数据
			返回 0 代表成功， 否则失败
		*/
		/************************************************/
		virtual int	Read(unsigned long a_readpos, unsigned char* a_buf, unsigned long a_readsize, unsigned long& a_available) = 0;
	};

	SH_P2PSERVICE_API SHP2PService* P2PInstance();
	SH_P2PSERVICE_API void P2PFree(SHP2PService*);
#ifdef _cplusplus
};
#endif

#endif