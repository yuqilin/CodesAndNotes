#include "QtSequenceMerge.h"
#pragma once

typedef enum tagFlyfox_player_request_media_status_code
{
	FLYFOX_PLAYER_REQUEST_MEDIA_STATUS_CLOSED_PIECE  = 0,
	FLYFOX_PLAYER_REQUEST_MEDIA_STATUS_NETWORK_ERROR          //网络出现错误
}Flyfox_player_request_media_status_code_e;

/****返回数据回调****
* in_bNewPiece              :数据段index
* in_pReceiveDataBuffer     :接受数据
* in_MediaPieceNum          :接受数据长度
****/

typedef void(*flyfox_player_request_media_cb)(int nPieceIndex, unsigned char* in_pReceiveDataBuffer, int nBufferLen);
typedef void(*flyfox_player_request_media_status_cb)(int in_bPieceIndex, Flyfox_player_request_media_status_code_e e_StatusCode);

class CP2PRead
{
public:
	CP2PRead(const char* media_template_path, int in_start, int in_end);
	~CP2PRead(void);

public:
	//test
	void getSequenceInfo(SequenceInfo* out_psInfo);

private:
	SequenceInfo m_sInfo;
	vector<FILE*> m_vFileHandle;
	int m_iStartClipNumber;
	int m_iEndClipNumber;

public:
	/****打开数据缓冲****
	****/
	void flyfox_media_data_cache_init(flyfox_player_request_media_cb RequestMedia_cb, 
		flyfox_player_request_media_status_cb Error_cb);

	/****关闭数据缓冲****
	****/
	void flyfox_media_data_cache_uninit();

	/****请求数据片段****
	*in_pPieceName :片段标示
	*in_nStartPos  :片段开始播放起始点  单位：byte
	****/
	bool flyfox_media_data_cache_request_piece(int in_nPieceIndex, int  in_nStartPos, int in_nEndPos); 
	bool flyfox_media_data_cache_cancel_request_piece(int in_nPieceIndex);

private:
	flyfox_player_request_media_cb m_cb;
	flyfox_player_request_media_status_cb m_status_cb;
};
