#include "P2PRead.h"

CP2PRead::CP2PRead(const char* media_template_path, int in_start, int in_end)
{
	m_cb = NULL;
	m_status_cb = NULL;

	m_iStartClipNumber = in_start;
	m_iEndClipNumber = in_end;

	m_sInfo.pSectioninfo = new SectionInfo[in_end-in_start];
	m_sInfo.nSectionCount = in_end-in_start;
	for (int i = in_start; i < in_end - in_start; i++)
	{
		char media_path[266];
		sprintf_s(media_path, media_template_path, i);
		FILE* filehandle = NULL;
		errno_t err = fopen_s(&filehandle, media_path, "rb");
		if (filehandle)
		{
			m_sInfo.pSectioninfo[i-in_start].pSectionInfoBuffer = new unsigned char[1024 * 1024 * 1]; //1M
			int nIn = fread(m_sInfo.pSectioninfo[i-in_start].pSectionInfoBuffer, 1, 1024 * 1024 * 1, filehandle);
			if (nIn)
			{					
				QtHeader sHeader;	
				sHeader.Fill(m_sInfo.pSectioninfo[i-in_start].pSectionInfoBuffer, nIn);

				m_sInfo.pSectioninfo[i-in_start].ui32SectionInfoBufferSize = sHeader.ftyp.dwSize + sHeader.moov.dwSize;
			}
			else
			{
				delete [] m_sInfo.pSectioninfo[i-in_start].pSectionInfoBuffer;
				m_sInfo.pSectioninfo[i-in_start].pSectionInfoBuffer = NULL;
			}
			
			m_vFileHandle.push_back(filehandle);
		}
	}
}

CP2PRead::~CP2PRead(void)
{
	for (unsigned int i = 0; i < m_vFileHandle.size(); i++)
	{
		fclose(m_vFileHandle[i]);
	}

	m_vFileHandle.clear();

	for (unsigned int i = 0; i < m_sInfo.nSectionCount; i++)
	{
		delete [] m_sInfo.pSectioninfo[i].pSectionInfoBuffer;
		m_sInfo.pSectioninfo[i].pSectionInfoBuffer = NULL;
		m_sInfo.pSectioninfo[i].ui32SectionInfoBufferSize = 0;
	}

	for (unsigned int i = 0; i < m_sInfo.nSectionCount; i++)
	{
		delete [] m_sInfo.pSectioninfo;
		m_sInfo.pSectioninfo = NULL;
	}

	m_sInfo.nSectionCount = 0;
}

void CP2PRead::getSequenceInfo(SequenceInfo* out_psInfo)
{
	if (out_psInfo)
		*out_psInfo = m_sInfo;
}

void CP2PRead::flyfox_media_data_cache_init(flyfox_player_request_media_cb RequestMedia_cb, 
								  flyfox_player_request_media_status_cb Error_cb)
{
	m_cb = RequestMedia_cb;
	m_status_cb = Error_cb;
}

/****关闭数据缓冲****
****/
void CP2PRead::flyfox_media_data_cache_uninit()
{
	m_cb = NULL;
	m_status_cb = NULL;
}

/****请求数据片段****
*in_pPieceName :片段标示
*in_nStartPos  :片段开始播放起始点  单位：byte
****/
bool CP2PRead::flyfox_media_data_cache_request_piece(int in_nPieceIndex, int  in_nStartPos, int in_nEndPos)
{
	FILE* pfile = m_vFileHandle[in_nPieceIndex];
	if (pfile)
	{
		unsigned char* pBuffer = new unsigned char[1024 * 1024 * 1];
		fseek(pfile, in_nStartPos, SEEK_SET);
		int nIn = 0;
		int nreadbytes = 0;
		do
		{
			if (in_nEndPos - in_nStartPos - nreadbytes < 1024 * 1024 * 1)
			{
				nIn = fread(pBuffer, 1, in_nEndPos - in_nStartPos - nreadbytes, pfile);
				nreadbytes += nIn;
				if (m_cb)
					m_cb(in_nPieceIndex, pBuffer, nIn);

				break;
			}
			else
			{
				nIn = fread(pBuffer, 1, 1024 * 1024 * 1, pfile);
				nreadbytes += nIn;
				if (m_cb)
					m_cb(in_nPieceIndex, pBuffer, nIn);
			}
		}
		while(nIn > 4);
		delete [] pBuffer;
	}

	return true;
}

bool CP2PRead::flyfox_media_data_cache_cancel_request_piece(int in_nPieceIndex)
{
	return true;
}
