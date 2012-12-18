
#ifndef _ZPLAYERCORE_H_
#define _ZPLAYERCORE_H_

#include "flyfox_media_player_def.h"
#include "BaseEngine.h"

//typedef void (*TypePlayEventCallback)(int nEventCode, long nEventParam, LPVOID pCallbackParam);
typedef enum MEDIA_LOAD_STATE {
	MLS_CLOSED,
	MLS_LOADING,
	MLS_LOADED,
	MLS_CLOSING
};

typedef enum PLAY_STATE {
	PS_PLAY   = 0,
	PS_PAUSE  = 1,
	PS_STOP   = 2,
	PS_UNUSED = 3
};

class CPlayerCore
{
public:
	CPlayerCore();
	~CPlayerCore();

public:
	// Create/Destroy
	HRESULT				Create(void);
	void				Destroy(void);

	// Play Control
	HRESULT				Open(LPCTSTR pcszUrl);
	HRESULT				Close(void);
	HRESULT				Play(void);
	HRESULT				Stop(void);
	HRESULT				Pause(void);
	void				SetMediaLoadState(MEDIA_LOAD_STATE iState);
	void				SetPlayState(PLAY_STATE iState);

	HRESULT				GetPlayState();
	HRESULT				GetPlayPos(LONG* pnPlayPos);
	HRESULT				SetPlayPos(LONG nPlayPos);
	HRESULT				GetVolume(LONG* pnVolume);
	HRESULT				SetVolume(LONG nVolume);

	// MediaInfo
	HRESULT				GetTotalDuration(LONG* pnDuration);
	HRESULT				GetVideoWidth(LONG* pnVideoWidth);
	HRESULT				GetVideoHeight(LONG* pnVideoHeight);

	// VideoPosition
	HRESULT				SetVideoWindow(HWND hVideoWindow);
	HRESULT				SetFullScreen(BOOL bFullScreen);
	HWND				GetVideoWindow();
	BOOL				IsFullScreen();

	HRESULT				SetVideoDestPosition(int nStreamIndex, const RECT& rcVideoDest);
	HRESULT				RepaintVideo();
	HRESULT				SetVideoRenderMode(VIDEO_RENDER_MODE eVideoRenderMode);
	HRESULT				GetVideoRenderMode(VIDEO_RENDER_MODE* peVideoRenderMode);

	HWND				GetNotifyWindow();

	static const CDSFilters& GetFilters();
	static const CPlayerSettings& GetPlayerSettings();
protected:
	// Notify window
	HRESULT				CreateNotifyWindow(void);
	void				DestroyNotifyWindow(void);
	static LONG WINAPI	NotifyWindowProcStatic(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LONG				NotifyWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void				SetPlayState(PLAY_STATE ePlayState);
	void				SetPlayError(DWORD dwErrorCode);

	void				OnOpenSuccess();
	void				OnOpenFailed();

protected:

	MEDIA_LOAD_STATE	m_MediaLoadState;
	PLAY_STATE			m_PlayState;

	CBaseEngine*		m_pEngine;
	CMediaInfo			m_MediaInfo;

	HWND				m_hVideoWindow;
	BOOL				m_bFullScreen;
	HWND				m_hNotifyWindow;

	BOOL				m_bAutoPlay;
	CRITICAL_SECTION	m_csBusy;
	DWORD				m_dwPlayResult;

	static CDSFilters	m_Filters;
	static CPlayerSettings m_Settings;
};


#endif /*_ZPLAYERCORE_H_*/