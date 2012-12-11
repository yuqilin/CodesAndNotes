

/*copyright:f
*/
#ifndef _FLYFOX_MEDIA_PLAYER_DEF_H_
#define _FLYFOX_MEDIA_PLAYER_DEF_H_

#include "flyfox_std.h"

#define FLYFOX_PLAYER_VOLUME_LEVEL_MAX     1500  //声音级数
#define FLYFOX_PLAYER_PRELOAD_NEXT_PIECE_TIME    30000  //30s

#ifdef __cplusplus
extern "C"
{
#endif

typedef FF_VOID (*flyfox_media_player_notify_to_ui)(FF_INT a_nNotifyMsg, FF_VOID* wParam,FF_VOID* lParam);
typedef FF_VOID (*flyfox_media_player_open_cb)(FF_BOOL in_bSuceess);


typedef enum flyfox_media_player_status
{
	flyfox_media_player_none = 0,	         //无状态
	flyfox_media_player_load,	             //加载
	flyfox_media_player_play,	             //播放 
	flyfox_media_player_pause,	             //暂停
	flyfox_media_player_ready,	             //停止
	flyfox_media_player_endof
}flyfox_media_player_status_e;


/*
win2000 -2003  默认选择ddraw
win xp         默认选择vmr9
win vista win7 默认选择evr
*/
typedef enum flyfox_media_player_video_render_mode
{
	flyfox_media_player_video_render_none = 0,	                     //无渲染模式
	flyfox_media_player_video_render_vmr7,	             //vmr7
	flyfox_media_player_video_render_vmr9,	             //vmr9 
	flyfox_media_player_video_render_evr,	             //增强渲染模式
	flyfox_media_player_video_render_ddraw	             //ddraw及gdi
}flyfox_media_player_video_render_mode_e;

typedef enum tagFlyfox_player_ui_message
{
	FF_PLAYER_UI_MESSAGE_NONE = 0,
	FF_PLAYER_UI_MESSAGE_NOT_SUPPORTED_FORMAT,        //本视频格式不支持
	FF_PLAYER_UI_MESSAGE_OPEN_MEDIA_SUCCESS,          //打开媒体成功
	FF_PLAYER_UI_MESSAGE_OPEN_MEDIA_FAILED,           //打开媒体失败

	FF_PLAYER_UI_MESSAGE_SEEK_MEDIA_SUCCESS,          //seek成功
	FF_PLAYER_UI_MESSAGE_SEEK_MEDIA_FAILED,           //seek失败

	FF_PLAYER_UI_MESSAGE_BUFFERING_PERCENT,           //开始播放之前缓冲百分比

	FF_PLAYER_UI_MESSAGE_GET_MEDIA_INFO_SUCCESS,      //配置信息获取成功
	FF_PLAYER_UI_MESSAGE_READY_TO_PLAY,               //开始播放
	FF_PLAYER_UI_MESSAGE_PLAY_ENDOF,                  //播出结束


	FF_PLAYER_UI_MESSAGE_MEDIA_TOTAL_DURATION,         //整个视频长度 ms
	FF_PLAYER_UI_MESSAGE_MEDIA_CURRENT_POS,            //当前播放长度 ms
	FF_PLAYER_UI_MESSAGE_MEDIA_PRELOAD_LEN,            //播放时预加载数据信息
	FF_PLAYER_UI_MESSAGE_MEDIA_TOTAL_FILE_LEN,         //整个视频大小

	FF_PLAYER_UI_MESSAGE_CONNECT_NETWORK_FAILED,       //没有网络连接 网络连接超时
	FF_PLAYER_UI_MESSAGE_REQUEST_HOT_VRS_FAILED,       //请求hot_vrs 失败 重试3边后，提示用户
	FF_PLAYER_UI_MESSAGE_NETWORK_MEDIA_PARSER_FAILED,  //网络播放文件解析不正确

	FF_PLAYER_UI_MESSAGE_NETWORK_CLOSED,               //播放时网络断开

	FF_PLAYER_UI_MESSAGE_NETWORK_PRELOAD_OVER,         //预加载结束

	FF_PLAYER_UI_MESSAGE_PAUSE_PLAYING_FOR_BUFFERING,    //因为网速原因，暂停播放，开始缓冲
	FF_PLAYER_UI_MESSAGE_START_PLAYING_FOR_BUFFERING,   //中间缓冲，开始播放

	FF_PLAYER_UI_MESSAGE_PLAYING_PIECE_INDEX,           //当前播放piece index

	FF_PLAYER_UI_MESSAGE_DDSHOW_NOT_SUPPORT,            //ddshow  vmr 显卡加速显示不支持  0:vmr不支持  1：filter连接失败

	FF_PLAYER_UI_MESSAGE_NEED_DOWNLOAD_ACCE,             //下载视频加速

	FF_PLAYER_UI_MESSAGE_NEED_CHANGE_VIDEO_DEFINITION,    //改变视频清晰度

	FF_PLAYER_UI_MESSAGE_MOOV_AT_END,                      //moov在文件末尾
		FF_PLAYER_UI_MESSAGE_RECEIVE_DATA_LEN           //开始播放之前缓冲百分比


}Flyfox_player_ui_message_e;

typedef enum tagFlyfox_player_video_display_ratio
{
	FF_PLAYER_VIDEO_DISPLAY_RATIO_NONE = 0,
	FF_PLAYER_VIDEO_DISPLAY_RATIO_DEFAULT,             //默认比例
	FF_PLAYER_VIDEO_DISPLAY_RATIO_4_3,                 //4：3
	FF_PLAYER_VIDEO_DISPLAY_RATIO_16_9,                //16：9
	FF_PLAYER_VIDEO_DISPLAY_RATIO_FULL_RECT            //填充满区域

}Flyfox_player_video_display_mode_e;



#define  FLYFOX_MEDIA_DES_MAX            128
#define  FLYFOX_MEDIA_PIECE_HASH_ID      128
#define  FLYFOX_MEDIA_PIECE_KEY_ID       128
#define  FLYFOX_MEDIA_NAME_MAX           64
#define  FLYFOX_MEDIA_ASPECT             16
#define  FLYFOX_MEDIA_ONLINE_URL_MAX_LEN  512


typedef struct tagFlyfoxMediaAspect
{
	FF_INT			    time;	//时间
	FF_CHAR             stzDes[FLYFOX_MEDIA_DES_MAX];
}FlyfoxMediaAspect_t;


typedef struct tagFlyfoxMediaSection
{
	FF_DOUBLE			media_piece_duration;                                   //时常
	FF_INT			    media_piece_size;	                                    //大小
	FF_CHAR	            media_piece_url[FLYFOX_MEDIA_ONLINE_URL_MAX_LEN];	    //视频各片段文件地址
	FF_CHAR	            media_piece_hashId[FLYFOX_MEDIA_PIECE_HASH_ID];		    //hashId
	FF_CHAR	            media_piece_key[FLYFOX_MEDIA_PIECE_KEY_ID];		        //视频加密串, 防盗链
	FF_CHAR	            media_piece_newAddress[FLYFOX_MEDIA_ONLINE_URL_MAX_LEN];//新架构视频片段地址
}FlyfoxMediaSection_t;

typedef struct tagFlyfoxOnlineMediaInfo
{
	FF_INT				    id;			                                        //唯一标识
	FF_INT				    p2pflag;											//传给加速器的参数
	FF_INT				    vid;												//视频id
	FF_BOOL			        longVideo;											//是否是长视频
	FF_INT				    tn;													//请求调度时转发给调度服务器(参数名:cdn)
	FF_INT				    status;												//视频信息状态, 1为正常
	FF_BOOL			        play;												//1为正常播放 0为禁播
	FF_BOOL			        fms;												//是否为FMS视频源
	FF_BOOL			        fee;												//是否为付费视频
	FF_INT				    pid;
	FF_INT				    fps;												//视频桢率
	FF_INT				    version;											//视频版本, 1为高清 2为流畅
	FF_INT				    num;												//该视频在专辑中的位置
	FF_INT				    st;													//片头时长, 跳片头使用
	FF_INT				    et;													//片尾时长, 跳片尾使用

	FF_INT                  norVid;                                             //普清vid
	FF_INT                  highVid;                                            //高清vid
	FF_INT                  superVid;                                           //超高清vid

	FF_INT                  nPieceNum;

	FF_CHAR			        name[FLYFOX_MEDIA_NAME_MAX];		               //视频名称
	FF_CHAR			        ch[FLYFOX_MEDIA_NAME_MAX];			               //视频所属频道
	FF_CHAR			        allot[FLYFOX_MEDIA_ONLINE_URL_MAX_LEN];		       //调度服务器地址
	FF_CHAR			        url[FLYFOX_MEDIA_ONLINE_URL_MAX_LEN];		       //视频最终播放页
	FlyfoxMediaAspect_t	    aspects[FLYFOX_MEDIA_ASPECT];                      //看点

	FlyfoxMediaSection_t*	MediasectionsArray;                                //视频段信息

}FlyfoxOnlineMediaInfo_t;

typedef enum tagflyfox_media_player_3d_source_mode
{
	FF_PLAYER_3D_SOUECE_ABOVE_BELOW  = 0,
	FF_PLAYER_3D_SOUECE_LEFT_RIGHT   = 1,
	FF_PLAYER_3D_SOUECE_INTERLACE    = 2,
}flyfox_media_player_3d_source_mode_e;


typedef enum tagflyfox_media_player_3d_convert_mode
{
	FF_PLAYER_3D_CONVERT_TRANS2DTO3D  = 3,
	FF_PLAYER_3D_CONVERT_EASY3D   = 4,
}flyfox_media_player_3d_convert_mode_e;

typedef enum tagflyfox_media_player_3d_display_mode
{
	FF_PLAYER_3D_DISPLAY_2D             = 0, //2D模式显示
	FF_PLAYER_3D_DISPLAY_RED_BLUE       = 1, //红蓝模式显示 
	FF_PLAYER_3D_DISPLAY_GREEN_VIOL     = 2, //绿紫模式显示
	FF_PLAYER_3D_DISPLAY_BROWN_BLUE     = 3, //棕蓝模式显示
}flyfox_media_player_3d_display_mode_e;


typedef enum tagflyfox_media_player_3d_swap
{
	FF_PLAYER_3D_SWAP_NORMAL             = 0,
	FF_PLAYER_3D_SWAP_SWAP               = 1,
}flyfox_media_player_3d_swap_e;

typedef struct tagFlyfox_player_3d_mode 
{
	flyfox_media_player_3d_source_mode_e   source_mode;
	flyfox_media_player_3d_convert_mode_e  convert_mode;
	flyfox_media_player_3d_display_mode_e  display_mode;
	flyfox_media_player_3d_swap_e          swap_mode;
	FF_INT                                 bit_depth;
	FF_BOOL                                b_opened;
}Flyfox_player_3d_mode_t;



typedef enum tagflyfox_media_player_video_resolution
{
	FF_PLAYER_3D_VIDEO_NORMAL_RESOLUTION             = 0,
	FF_PLAYER_3D_VIDEO_HIGH_RESOLUTION               = 1,
	FF_PLAYER_3D_VIDEO_SUPER_RESOLUTION              = 2,
	FF_PLAYER_3D_VIDEO_AUTO_RESOLUTION               = 3,
}flyfox_media_player_video_resolution_e;


//描述：视频帧格式
typedef enum tagVideoSurfaceFmt
{
	FLYFOX_VIDEO_SURFACE_FMT_Invalid = 0,
	FLYFOX_VIDEO_SURFACE_FMT_YV12 ,
	FLYFOX_VIDEO_SURFACE_FMT_II420,
	FLYFOX_VIDEO_SURFACE_FMT_RGB444,
	FLYFOX_VIDEO_SURFACE_FMT_RGB565,
	FLYFOX_VIDEO_SURFACE_FMT_RGB24,
	FLYFOX_VIDEO_SURFACE_FMT_BGR24,
	FLYFOX_VIDEO_SURFACE_FMT_RGB32,
	FLYFOX_VIDEO_SURFACE_FMT_H264,
	FLYFOX_VIDEO_SURFACE_FMT_H263,
	FLYFOX_VIDEO_SURFACE_FMT_MPEG2,
	FLYFOX_VIDEO_SURFACE_FMT_MPEG4,
	FLYFOX_VIDEO_SURFACE_FMT_WMV,
	FLYFOX_VIDEO_SURFACE_FMT_RMVB,
	FLYFOX_VIDEO_SURFACE_FMT_End
} FlyfoxVideoSurfaceFmt_e;


//描述：音频声道。
typedef enum tagFlyfoxAudioChannels
{
	FLYFOX_AUDIO_CHANNELS_MONO = 0x01,//单声道
	FLYFOX_AUDIO_CHANNELS_STEREO  //立体声
}FlyfoxAudioChannels_e;


//描述：音频采样率。
typedef enum tagFlyfoxAudioSampleRates
{
	FLYFOX_AUDIO_SAMPLERATES_8000HZ,            //8000HZ
	FLYFOX_AUDIO_SAMPLERATES_11025HZ,           //11025HZ
	FLYFOX_AUDIO_SAMPLERATES_12000HZ,           //12000HZ
	FLYFOX_AUDIO_SAMPLERATES_16000HZ,           //16000HZ
	FLYFOX_AUDIO_SAMPLERATES_22050HZ,           //22050HZ
	FLYFOX_AUDIO_SAMPLERATES_24000HZ,           //24000HZ
	FLYFOX_AUDIO_SAMPLERATES_32000HZ,           //32000HZ
	FLYFOX_AUDIO_SAMPLERATES_44100HZ,           //44100HZ
	FLYFOX_AUDIO_SAMPLERATES_48000HZ,           //48000HZ
	FLYFOX_AUDIO_SAMPLERATES_64000HZ,           //64000HZ
	FLYFOX_AUDIO_SAMPLERATES_96000HZ            //96000HZ
}FlyfoxAudioSampleRates_e;

//描述：音频帧格式
typedef enum tagFlyfoxAudioSampleFmt
{
	FLYFOX_VIDEO_AUDIO_FMT_Invalide = 0,
	FLYFOX_VIDEO_AUDIO_FMT_PCM         ,
	FLYFOX_VIDEO_AUDIO_FMT_ADPCM         ,
	FLYFOX_VIDEO_AUDIO_FMT_AAC         ,
	FLYFOX_VIDEO_AUDIO_FMT_AMRWB       ,
	FLYFOX_VIDEO_AUDIO_FMT_AMRNB       ,
	FLYFOX_VIDEO_AUDIO_FMT_MP3         ,
	FLYFOX_VIDEO_AUDIO_FMT_WMA         ,
	FLYFOX_VIDEO_AUDIO_FMT_VORBIS      ,
	FLYFOX_VIDEO_AUDIO_FMT_AC3      ,
	FLYFOX_VIDEO_AUDIO_FMT_RM      ,
	FLYFOX_VIDEO_AUDIO_FMT_End
}FlyfoxAudioSampleFmt_e;

//描述：视音频文件封装格式，
typedef enum tagFlyfoxMediaMuxerFmt
{
	FLYFOX_MEDIA_MUXER_FMT_Invalid = 0,
	FLYFOX_MEDIA_MUXER_FMT_3GP ,
	FLYFOX_MEDIA_MUXER_FMT_MP4 ,
	FLYFOX_MEDIA_MUXER_FMT_WMV ,
	FLYFOX_MEDIA_MUXER_FMT_AVI ,
	FLYFOX_MEDIA_MUXER_FMT_MOV ,
	FLYFOX_MEDIA_MUXER_FMT_MKV ,
	FLYFOX_MEDIA_MUXER_FMT_FV,
	FLYFOX_MEDIA_MUXER_FMT_FLV,
	FLYFOX_MEDIA_MUXER_FMT_RMVB ,
	FLYFOX_MEDIA_MUXER_FMT_RTSP,
	FLYFOX_MEDIA_MUXER_FMT_MP3,	
	FLYFOX_MEDIA_MUXER_FMT_WMA,
	FLYFOX_MEDIA_MUXER_FMT_End
} FlyfoxMediaMuxerFmt_e;


//描述：字幕封装格式
typedef enum tagFlyfoxSubTitleFmt
{
	FLYFOX_MEDIA_SUBTITLE_FMT_Invalid = 0,
	FLYFOX_MEDIA_SUBTITLE_FMT_SSA,
	FLYFOX_MEDIA_SUBTITLE_FMT_SRT, 
	FLYFOX_MEDIA_SUBTITLE_FMT_TEXT, 
	FLYFOX_MEDIA_SUBTITLE_FMT_End 
} FlyfoxSubTitleFmt_e;

//描述：reader filter 种类
typedef enum tagFlyfoxReaderFilterFmt
{
	FLYFOX_READER_FILTER_FMT_Invalid = 0,
	FLYFOX_READER_FILTER_FMT_LOCAL_FILE ,
	FLYFOX_READER_FILTER_FMT_HTTP_FILE  ,
	FLYFOX_READER_FILTER_FMT_RTSP_FILE  ,
	FLYFOX_READER_FILTER_FMT_END
} FlyfoxReaderFilterFmt_e;

//描述：字幕帧信息
typedef struct tagFlySubtitleDesc
{
	FlyfoxSubTitleFmt_e eSubtitleFmt;
}FlySubtitleDesc;


//描述：视频帧信息
typedef struct tagFlyfoxVideoDesc
{
	FlyfoxVideoSurfaceFmt_e                    eVideoSurfaceFmt;
	FF_UINT                                  nWidth;
	FF_UINT                                  nHeight;

	FF_UINT                                  nBitRate;
	FF_UINT                                  nFrameRate;

	FF_DWORD                                 nFrameTotalDuration;
	FF_UINT                                  nSizeImage ;

	FlySubtitleDesc                          sFlySubtitleDesc;
}FlyfoxVideoDesc_t;

//描述：音频帧信息
typedef struct tagFlyfoxAudioDesc
{
	FlyfoxAudioSampleFmt_e                       eAudioSampleFmt;

	FlyfoxAudioChannels_e                        eChannelCount;
	FlyfoxAudioSampleRates_e                     euiFreq;            //直接使用数字
	FF_UINT                                      uiCodecFreq;        //直接使用数字

	FF_UINT                                      uiBitCount;
	FF_UINT                                      uiBitsPerSample;

	FF_CHAR                                    strSongName[32]; 
	FF_CHAR                                    strSpecialName[32];
	FF_CHAR                                    strSingerName[32];

	FF_UINT                                    uiKbps;

}FlyfoxAudioDesc_t;

typedef enum tagFlyfoxSubtitleLanguage
{
	FLYFOX_MEDIA_SUBTITLE_LAG_Invalid = 0,
	FLYFOX_MEDIA_SUBTITLE_LAG_ENGLISH,
	FLYFOX_MEDIA_SUBTITLE_LAG_SIMPLE_CHINESE, 
	FLYFOX_MEDIA_SUBTITLE_LAG_TRADITIONAL_CHINESE, 
	FLYFOX_MEDIA_SUBTITLE_LAG_OTHER, 
	FLYFOX_MEDIA_SUBTITLE_LAG_End 
}FlyfoxSubtitleLanguage_e;

//描述：字幕信息
typedef struct tagFlyfoxSubtitleDesc
{
	FlyfoxSubTitleFmt_e             eFmt;
	FF_BOOL							bEmbedded;
	FlyfoxSubtitleLanguage_e		eLanguage;
	FF_CHAR							cLanguageName[260];
}FlyfoxSubtitleDesc_t;

#define FLYFOX_PLAYER_SUBTITLE_STREAM_MAX   8

//封装格式信息
typedef struct tagFlyfoxMediaMuxerDesc
{
	FlyfoxMediaMuxerFmt_e                        eMediaMuxerFmt;

	FlyfoxVideoSurfaceFmt_e                      eVideoSurfaceFmt;
	FlyfoxAudioSampleFmt_e                       eAudioSampleFmt;

	FF_UINT                                    uiVideoSurfaceCount;
	FF_UINT                                    uiAudioSampleCount;

	FF_UINT									uiEmbeddedSubtitleStreamCount; // < FLYFOX_PLAYER_SUBTITLE_STREAM_MAX
	FlyfoxSubtitleDesc_t					sEmbeddedSubtitleStreamInfo[FLYFOX_PLAYER_SUBTITLE_STREAM_MAX];

}FlyfoxMediaMuxerDesc_t;


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

typedef FF_VOID(*flyfox_player_request_media_cb)(FF_INT nPieceIndex, FF_BYTE* in_pReceiveDataBuffer, FF_INT nBufferLen);


typedef FF_VOID(*flyfox_player_request_media_status_cb)(FF_INT in_bPieceIndex, Flyfox_player_request_media_status_code_e e_StatusCode);


/****打开数据缓冲****
****/
typedef FF_VOID (*flyfox_media_data_cache_init)(flyfox_player_request_media_cb RequestMedia_cb,
												flyfox_player_request_media_status_cb Error_cb);

/****关闭数据缓冲****
****/
typedef FF_VOID (*flyfox_media_data_cache_uninit)();

/****请求数据片段****
*in_pPieceName :片段标示
*in_nStartPos  :片段开始播放起始点  单位：ms 
****/
typedef FF_BOOL (*flyfox_media_data_cache_request_piece)(FF_INT in_nPieceIndex, FF_INT  in_nStartPos); //  in_nSeekIndex ms
typedef FF_BOOL (*flyfox_media_data_cache_cancel_request_piece)(FF_INT in_nPieceIndex);


typedef struct tagFlyFoxMediaDataCacheFunc
{
	flyfox_media_data_cache_init                                         data_cache_init;

	flyfox_media_data_cache_uninit                                       data_cache_uninit;

	flyfox_media_data_cache_request_piece                                data_cache_request_piece;
	flyfox_media_data_cache_cancel_request_piece                         data_cache_cancle_request_piece;

}FlyFoxMediaDataCacheFunc_t;

//Grab Frame ERROR Code
#define FF_E_GENIRAIC_FAILED					-1
#define FF_E_NOT_IMPLETMENT				-2
#define FF_E_NOT_SUPPORTED					-3
#define FF_E_OUT_OF_MEMERY					-4
#define FF_E_INVALID_FILE_PATH				-5
#define FF_E_CREATE_FILE_FAILED			-6

#ifdef __cplusplus
}
#endif

#endif

