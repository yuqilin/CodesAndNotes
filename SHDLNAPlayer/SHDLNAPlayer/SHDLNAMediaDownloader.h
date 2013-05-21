#ifndef _SH_DLNA_MEDIA_DOWNLOADER_H_
#define _SH_DLNA_MEDIA_DOWNLOADER_H_

/*----------------------------------------------------------------------
|   class CSHDLNAMediaDownloader
+---------------------------------------------------------------------*/
class CSHDLNAMediaDownloader : public PLT_ThreadTask
{
public:
	CSHDLNAMediaDownloader(const char* url, void* user);
	virtual ~CSHDLNAMediaDownloader() {}

	static int ProgressCallBack(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);

protected:
	// PLT_ThreadTask methods
	virtual void DoRun();

	NPT_String m_Url;
	bool	m_HeaderDownloadCompleted;

	void*	m_User;

	int		m_DownloadStatus;
};

#endif // _SH_DLNA_MEDIA_DOWNLOADER_H_