#include "stdafx.h"
#include "SHDLNAMediaDownloader.h"
#include "SHDLNAMediaPlayer.h"

/*----------------------------------------------------------------------
|   CSHDLNAMediaDownloader::CSHDLNAMediaDownloader
+---------------------------------------------------------------------*/
CSHDLNAMediaDownloader::CSHDLNAMediaDownloader(const char* url, void* user)
: m_User(user)
, m_HeaderDownloadCompleted(false)
{
	
}

/*----------------------------------------------------------------------
|   CSHDLNAMediaDownloader::DoRun
+---------------------------------------------------------------------*/
void CSHDLNAMediaDownloader::DoRun()
{
	//
	if (!m_HeaderDownloadCompleted)
	{
		m_HeaderDownloadCompleted = true;
	}

	CSHDLNAMediaPlayer* player = (CSHDLNAMediaPlayer*)m_User;
	if (player)
	{
		player->m_MediaInfo.title = "";
		player->m_MediaInfo.header_info = new SHMP4HeaderInfo_t;
		//player->OnMediaHeaderDownloadCompleted();
		player->OpenMedia();
	}
}
