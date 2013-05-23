#ifndef _SH_DLNA_MEDIA_DOWNLOADER_H_
#define _SH_DLNA_MEDIA_DOWNLOADER_H_

/*----------------------------------------------------------------------
|   class CSHDLNAMediaDownloader
+---------------------------------------------------------------------*/
class CSHDLNAMediaDownloader : public NPT_Thread
{
public:
	CSHDLNAMediaDownloader(const char* url, void* user);
	virtual ~CSHDLNAMediaDownloader() {}

	static int ProgressCallBack(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);

	// NPT_Interruptible methods
	virtual NPT_Result Interrupt();

protected:
	// NPT_Runnable methods
	virtual void Run();

	NPT_String m_Url;
	bool	m_HeaderDownloadCompleted;

	void*	m_User;

	int		m_DownloadStatus;
};

#endif // _SH_DLNA_MEDIA_DOWNLOADER_H_
