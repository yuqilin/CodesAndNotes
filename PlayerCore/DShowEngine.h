
#ifndef _DSHOW_ENGINE_H_
#define _DSHOW_ENGINE_H_

class CDShowEngine
	: public CBaseEngine
{
public:
	CDShowEngine();
	virtual ~CDShowEngine();

public:
	// Play Control
	virtual HRESULT		Open(CMediaInfo* pMediaInfo);
	virtual HRESULT		Close(void);
	virtual HRESULT		Play(void);
	virtual HRESULT		Stop(void);
	virtual HRESULT		Pause(void);
	virtual HRESULT		GetPlayState();
	virtual HRESULT		GetPlayPos(LONG* pnPlayPos);
	virtual HRESULT		SetPlayPos(LONG nPlayPos);
	virtual HRESULT		GetVolume(LONG* pnVolume);
	virtual HRESULT		SetVolume(LONG nVolume);

	// MediaInfo
	virtual HRESULT		GetTotalDuration(LONG* pnDuration);
	virtual HRESULT		GetVideoWidth(LONG* pnVideoWidth);
	virtual HRESULT		GetVideoHeight(LONG* pnVideoHeight);
	
	// VideoWindow
	virtual HRESULT		SetVideoWindow(HWND hVideoWnd, BOOL bFullScreen);
	virtual HRESULT		SetVideoDestPosition(int nStreamIndex, const RECT& rcVideoDest);
	virtual HRESULT		RepaintVideo();
	virtual HRESULT		SetVideoRenderMode(VIDEO_RENDER_MODE eVideoRenderMode);
	virtual HRESULT		GetVideoRenderMode(VIDEO_RENDER_MODE* peVideoRenderMode);

protected:
	HRESULT				OpenMedia();
	void				CloseMedia();

protected:
	CDShowGraph*		m_pGraph;

	BOOL				m_bRenderOpenChain;

};

#endif