#ifndef _SH_DLNA_MEDIA_DOWNLOADER_H_
#define _SH_DLNA_MEDIA_DOWNLOADER_H_

class CSHDLNAMediaDownloader : public PLT_ThreadTask
{
public:
	CSHDLNAMediaDownloader(const char* url, void* user);
	virtual ~CSHDLNAMediaDownloader() {}

protected:
	// PLT_ThreadTask methods
	virtual void DoRun();

	bool	m_HeaderDownloadCompleted;

	void*	m_User;
};

#endif // _SH_DLNA_MEDIA_DOWNLOADER_H_