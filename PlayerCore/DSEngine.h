
#ifndef _PLAYERCORE_DSENGINE_H_
#define _PLAYERCORE_DSENGINE_H_

class CDSEngine
	: public CBaseEngine
{
public:
	CDSEngine();
	virtual ~CDSEngine();

public:
	// Play Control
	virtual HRESULT		Open(CMediaInfo* pMediaInfo);
	virtual HRESULT		Close(void);
	virtual HRESULT		Play(void);
	virtual HRESULT		Stop(void);
	virtual HRESULT		Pause(void);
	//virtual HRESULT		GetPlayState();
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
	CDSGraph			m_Graph;

	struct MatchedSourceFilter
	{
		DSFilterInfo* info;
		CString subtype;
	};

	BOOL				m_bRenderOpenChain;

	CComPtr<IFilterGraph>	m_pJFilterGraph;
	

};

#endif