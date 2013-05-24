#ifndef _SH_DLNA_MEDIA_DOWNLOADER_H_
#define _SH_DLNA_MEDIA_DOWNLOADER_H_

#include <atlstr.h>
#include "SHDLNAMediaPlayer.h"

/*----------------------------------------------------------------------
|   class CSHDLNAMediaDownloader
+---------------------------------------------------------------------*/
class CSHDLNAMediaDownloader : public NPT_Thread
{
public:
	CSHDLNAMediaDownloader(const char* url, void* user);
	virtual ~CSHDLNAMediaDownloader() {}

	static size_t	HotVrsResponseCallBackFunc(void* buffer, size_t size, size_t nmemb, void* lpVoid);
	static int		ProgressCallBack(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
	static size_t	CdnResponseCallBackFunc(void* buffer, size_t size, size_t nmemb, void* lpVoid);
	static size_t	HeaderResponseCallBackFunc(void* buffer, size_t size, size_t nmemb, void* lpVoid);

	// NPT_Interruptible methods
	virtual NPT_Result Interrupt();

protected:
	// NPT_Runnable methods
	virtual void	Run();

	NPT_Result		HotVrsRequestVideoInfo(ATL::CString& mainfilename);
	NPT_Result		CDNRequestVideoHeader();
	bool			JsonParse(const void* xml, const unsigned int xmlsize, SHVideoInfo& videoinfo);
	bool			XmlParse(const void* buffer, const unsigned int buffersize, SHCDNInfo& cdnInfo);
	wstring			Utf82W(LPCSTR szContent);

protected:

	NPT_String m_Url;
	bool	m_HeaderDownloadCompleted;

	void*	m_User;

	int		m_DownloadStatus;

	SHMP4HeaderInfo_t*	m_header_info_ptr;
	string*			hotvrs_string_ptr;
	string*			cdn_string_ptr;
	string*			header_string_ptr;

	NPT_Mutex		m_Lock;
};

#endif // _SH_DLNA_MEDIA_DOWNLOADER_H_
