#ifndef _SH_DLNAPLAYER_H_
#define _SH_DLNAPLAYER_H_

#ifdef SH_DLNAPLAYER_EXPORTS
#define SH_DLNAPLAYER_API __declspec(dllexport)
#else
#define SH_DLNAPLAYER_API __declspec(dllimport)
#endif // SH_DLNAPLAYER_EXPORTS

/*----------------------------------------------------------------------
|   DLNA向UI层的消息通知
+---------------------------------------------------------------------*/

/*
*	回调函数，DLNA消息通知
*/
typedef void (*SH_DLNAPlayer_MessageNotifyUI)(int msg, void* wParam, void* lParam);


/*
*	DLNA通知消息
*/
typedef enum {
	SH_DLNAPLAYER_UI_MESSAGE_NONE = 0,
	SH_DLNAPLAYER_UI_MESSAGE_OPEN_MEDIA_SUCCEEDED,						// 打开媒体成功
	SH_DLNAPLAYER_UI_MESSAGE_OPEN_MEDIA_FAILED,							// 打开媒体失败

	SH_DLNAPLAYER_UI_MESSAGE_SEEK_SUCCEEDED,							// seek成功
	SH_DLNAPLAYER_UI_MESSAGE_SEEK_FAILED,								// seek失败

	SH_DLNAPLAYER_UI_MESSAGE_GET_MEDIA_INFO_SUCCEEDED,					// 获取媒体信息成功
	SH_DLNAPLAYER_UI_MESSAGE_GET_MEDIA_INFO_FAILED,						// 获取媒体信息失败

	SH_DLNAPLAYER_UI_MESSAGE_MEDIA_TOTAL_DURATION,						// 整个视频时长 ms，wParam返回时长(long)
	SH_DLNAPLAYER_UI_MESSAGE_MEDIA_CURRENT_POS,							// 当前播放位置 ms，wParam返回当前位置(long)

	SH_DLNAPLAYER_UI_MESSAGE_DEVICE_LIST_UPDATED,						// 设备列表更新，wParam返回设备列表(SH_DLNAPlayer_DeviceList*)

	SH_DLNAPLAYER_UI_MESSAGE_DEVICE_CURRENT_VOLUME,						// 设备当前播放音量，wParam返回当前音量（int)

} SH_DLNAPlayer_UI_Message;

/*----------------------------------------------------------------------
|   DLNA设备信息相关
+---------------------------------------------------------------------*/
#define SH_DLNAPLAYER_DEVICE_COUNT_MAX				64					// 设备数目最大值
#define SH_DLNAPLAYER_DEVICE_NAME_LENGTH_MAX		256					// 设备名称字符串最大长度
#define SH_DLNAPLAYER_DEVICE_UUID_LENGTH_MAX		64					// 设备UUID字符串最大长度

// 设备信息
typedef struct {
	char device_uuid[SH_DLNAPLAYER_DEVICE_UUID_LENGTH_MAX];				// 设备UUID
	char device_name[SH_DLNAPLAYER_DEVICE_NAME_LENGTH_MAX];				// 设备名称
} SH_DLNAPlayer_DeviceInfo;

// 设备列表
typedef struct {
	SH_DLNAPlayer_DeviceInfo	device[SH_DLNAPLAYER_DEVICE_COUNT_MAX];	// 设备信息
	int							count;									// 设备数量
} SH_DLNAPlayer_DeviceList;

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus


	/*----------------------------------------------------------------------
	|   DLNA 接口
	+---------------------------------------------------------------------*/

	/*
	*	SH_DLNAPlayer_Init
	*	描述：			模块初始化，启动DLNA功能，自动查询同网段下的DLNA设备
	*	参数：			[in] message_to_notify - 回调函数，DLNA模块向UI层发送通知消息
	*	返回值：		0 - 成功；非0 - 失败
	*/
	SH_DLNAPLAYER_API int		SH_DLNAPlayer_Init(SH_DLNAPlayer_MessageNotifyUI message_to_notify);

	/*
	*	SH_DLNAPlayer_Uninit
	*	描述：			模块反初始化，停止DLNA功能
	*	参数：			无
	*	返回值：		0 - 成功；非0 - 失败
	*/
	SH_DLNAPLAYER_API int		SH_DLNAPlayer_Uninit(void);

	/*
	*	SH_DLNAPlayer_ChooseDevice
	*	描述：			选择DLNA播放设备
	*	参数：			[in] device_uuid - 所选设备的UUID
	*	返回值：		0 - 成功；非0 - 失败
	*/
	SH_DLNAPLAYER_API int		SH_DLNAPlayer_ChooseDevice(const char* device_uuid);

	/*
	*	SH_DLNAPlayer_Open
	*	描述：			打开文件
	*	参数：			[in] path_or_url - 本地文件路径或在线视频URL, （utf-8编码）
	*	返回值：		0 - 成功；非0 - 失败
	*/
	SH_DLNAPLAYER_API int		SH_DLNAPlayer_Open(const char* url_utf8);

	/*
	*	SH_DLNAPlayer_Close
	*	描述：			关闭文件
	*	参数：			无
	*	返回值：		0 - 成功；非0 - 失败
	*/
	SH_DLNAPLAYER_API	int		SH_DLNAPlayer_Close(void);

	/*
	*	SH_DLNAPlayer_Play
	*	描述：			开始播放
	*	参数：			无
	*	返回值：		0 - 成功；非0 - 失败
	*/
	SH_DLNAPLAYER_API int		SH_DLNAPlayer_Play(void);

	/*
	*	SH_DLNAPlayer_Seek
	*	描述：			Seek
	*	参数：			[in] pos_to_play - 指定播放位置，单位为毫秒
	*	返回值：		0 - 成功；非0 - 失败
	*/
	SH_DLNAPLAYER_API int		SH_DLNAPlayer_Seek(long pos_to_play); // ms

	/*
	*	SH_DLNAPlayer_Pause
	*	描述：			暂停播放
	*	参数：			无
	*	返回值：		0 - 成功；非0 - 失败
	*/
	SH_DLNAPLAYER_API int		SH_DLNAPlayer_Pause(void);

	/*
	*	SH_DLNAPlayer_Stop
	*	描述：			停止播放
	*	参数：			无
	*	返回值：		0 - 成功；非0 - 失败
	*/
	SH_DLNAPLAYER_API int		SH_DLNAPlayer_Stop(void);

	/*
	*	SH_DLNAPlayer_SetVolume
	*	描述：			设置播放音量
	*	参数：			[in] volume - 音量，范围为0-100
	*	返回值：		0 - 成功；非0 - 失败
	*/
	SH_DLNAPLAYER_API int		SH_DLNAPlayer_SetVolume(int volume); // 0-100


	/*
	*	SH_DLNAPlayer_GetMediaDuration
	*	描述：			获取当前媒体播放时长，以消息通知方式返回结果
	*	参数：			无
	*	返回值：		0 - 成功；非0 - 失败
	*/
	SH_DLNAPLAYER_API int		SH_DLNAPlayer_GetMediaDuration(void);

	/*
	*	SH_DLNAPlayer_GetCurPlayPos
	*	描述：			获取当前媒体播放位置，以消息通知方式返回结果
	*	参数：			无
	*	返回值：		0 - 成功；非0 - 失败
	*/
	SH_DLNAPLAYER_API int		SH_DLNAPlayer_GetCurPlayPos(void);

	/*
	*	SH_DLNAPlayer_GetVolume
	*	描述：			获取当前播放音量，以消息通知方式返回结果
	*	参数：			无
	*	返回值：		0 - 成功；非0 - 失败
	*/
	SH_DLNAPLAYER_API int		SH_DLNAPlayer_GetVolume(void);


#ifdef __cplusplus
};
#endif // __cplusplus

#endif // _SH_DLNAPLAYER_H_
