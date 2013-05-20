#include "stdafx.h"
#include "SHDLNAMediaDownloder.h"

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

	SHDLNAMediaInfo_t mediainfo;
	CSHDLNAMediaPlayer* player = (CSHDLNAMediaPlayer*)m_User;
	if (player)
	{
		player->OnMediaHeaderDownloadCompleted(&mediainfo);
	}
}
