
#ifndef _BASE_ENGINE_H_
#define _BASE_ENGINE_H_

enum PLAY_STATE
{
	PS_INVALID,
	PS_READY_TO_PLAY,
	PS_PLAYING,
	PS_PAUSE,
	PS_STOP,
};

enum VIDEO_RENDER_MODE
{
	VRM_DEFAULT,
	VRM_OLDRENDERER,
	VRM_OVERLAY_MIXER,
	VRM_VMR7_WINDOWED,
	VRM_VMR7_WINDOWLESS,
	VRM_VMR7_RENDERLESS,
	VRM_VMR9_WINDOWED,
	VRM_VMR9_WINDOWLESS,
	VRM_VMR9_RENDERLESS,
	VRM_EVR,
	VRM_EVRCP,
};


class CBaseEngine
{
public:
	CBaseEngine();
	virtual ~CBaseEngine();

public:
	virtual HRESULT		Initialize(CZPlayerCore* pPlayerCore) = 0;
	virtual void		UnInitialize() = 0;

	// Play Control
	virtual HRESULT		Open(CMediaInfo* pMediaInfo) = 0;
	virtual HRESULT		Close(void) = 0;
	virtual HRESULT		Play(void) = 0;
	virtual HRESULT		Stop(void) = 0;
	virtual HRESULT		Pause(void) = 0;
	virtual HRESULT		GetPlayState() = 0;
	virtual HRESULT		GetPlayPos(LONG* pnPlayPos) = 0;
	virtual HRESULT		SetPlayPos(LONG nPlayPos) = 0;
	virtual HRESULT		GetVolume(LONG* pnVolume) = 0;
	virtual HRESULT		SetVolume(LONG nVolume) = 0;

	// MediaInfo
	virtual HRESULT		GetTotalDuration(LONG* pnDuration) = 0;
	virtual HRESULT		GetVideoWidth(LONG* pnVideoWidth) = 0;
	virtual HRESULT		GetVideoHeight(LONG* pnVideoHeight) = 0;

	// VideoWindow
	virtual HRESULT		SetVideoWindow(HWND hVideoWnd, BOOL bFullScreen) = 0;
	virtual HRESULT		SetVideoDestPosition(int nStreamIndex, const RECT& rcVideoDest) = 0;
	virtual HRESULT		RepaintVideo() = 0;
	virtual HRESULT		SetVideoRenderMode(VIDEO_RENDER_MODE eVideoRenderMode) = 0;
	virtual HRESULT		GetVideoRenderMode(VIDEO_RENDER_MODE* peVideoRenderMode) = 0;

protected:
	CZPlayerCore*		m_pPlayerCore;
	CMediaInfo*			m_pMediaInfo;
	HWND				m_hVideoWindow;
	HWND				m_hNotifyWindow;
};


#endif