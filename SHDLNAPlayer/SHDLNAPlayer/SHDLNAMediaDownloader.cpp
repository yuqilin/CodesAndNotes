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
void CSHDLNAMediaDownloader::Run()
{
	CSHDLNAMediaPlayer* player = (CSHDLNAMediaPlayer*)m_User;
	if (player == NULL || player->m_MediaInfo == NULL)
	{
		player->OpenMedia(false);
		return;
	}

	assert(player->m_MediaInfo->header_info == NULL);

	player->m_MediaInfo->header_info = new SHMP4HeaderInfo_t;

	SHMP4HeaderInfo_t* header_info = player->m_MediaInfo->header_info;

	assert(header_info != NULL);

	CMp4DownloadClient client;
	client.InitialRequest(&header_info->sequence,&header_info->info,&header_info->cdninfo);

	ATL::CString title;
	int result = client.HeaderRequest(m_Url, title, ProgressCallBack, ProgressCallBack, ProgressCallBack, this);

	if (result<0)
	{
		assert(false);
		player->OpenMedia(false);
	}
	else
	{
		player->m_MediaInfo->title = wcs2mbs(CP_UTF8, title).c_str();
		player->OpenMedia(true);
	}
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
NPT_Result CSHDLNAMediaDownloader::Interrupt()
{
	NPT_Result result = NPT_SUCCESS;

	m_DownloadStatus  = DOWNLOAD_BREAK;

	return result;
}


/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNAMediaDownloader::ProgressCallBack(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	CEasyRequest* str = dynamic_cast<CEasyRequest*>((CEasyRequest *)clientp);
	if (str)
	{
		CMp4DownloadClient* pClient = dynamic_cast<CMp4DownloadClient*>((CMp4DownloadClient*)str->m_customparam);
		if (pClient)
		{
			CSHDLNAMediaDownloader* pThis = dynamic_cast<CSHDLNAMediaDownloader*>((CSHDLNAMediaDownloader*)pClient->m_customparam);
			if (pThis)
			{
				return pThis->m_DownloadStatus;
			}
		}

	}

	return 0;
}
