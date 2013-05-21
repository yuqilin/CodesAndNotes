#include "stdafx.h"
#include <assert.h>
#include "SHDLNAMediaDownloader.h"
#include "SHDLNAMediaPlayer.h"


NPT_SET_LOCAL_LOGGER("shdlnaplayer.shdlnamediadownloader");

#define DOWNLOAD_BREAK			-1
#define DOWNLOAD_CONTINUE		0

/*----------------------------------------------------------------------
|   CSHDLNAMediaDownloader::CSHDLNAMediaDownloader
+---------------------------------------------------------------------*/
CSHDLNAMediaDownloader::CSHDLNAMediaDownloader(const char* url, void* user)
: m_User(user)
, m_HeaderDownloadCompleted(false)
, m_Url(url)
, m_DownloadStatus(DOWNLOAD_CONTINUE)
{
	
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaDownloader::DoRun
+---------------------------------------------------------------------*/
void CSHDLNAMediaDownloader::DoRun()
{
	CSHDLNAMediaPlayer* player = (CSHDLNAMediaPlayer*)m_User;
	if (player == NULL || player->m_MediaInfo == NULL)
	{
		player->OpenMedia(false);
		return;
	}

	SHMP4HeaderInfo_t* header_info = player->m_MediaInfo->header_info;

	assert(header_info == NULL);

	header_info = new SHMP4HeaderInfo_t;

	CMp4DownloadClient client;
	client.InitialRequest(&header_info->sequence,&header_info->info,&header_info->cdninfo);

	ATL::CString title;
	int result = client.HeaderRequest(m_Url, title, ProgressCallBack, ProgressCallBack, ProgressCallBack);

	if (result<0)
	{
		assert(false);
		player->OpenMedia(false);
	}
	else
	{
		player->m_MediaInfo->title = CStringA(title);
		player->OpenMedia(true);
	}
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNAMediaDownloader::ProgressCallBack(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	CEasyRequest* str = dynamic_cast<CEasyRequest*>((CEasyRequest *)clientp);
	if (str)
	{
		CSHDLNAMediaDownloader* pThis = dynamic_cast<CSHDLNAMediaDownloader*>((CSHDLNAMediaDownloader*)str->m_customparam);
		if (pThis)
		{
			return pThis->m_DownloadStatus;
		}
	}

	return 0;
}