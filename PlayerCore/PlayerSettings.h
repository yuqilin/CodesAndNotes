
#ifndef _PLAYER_SETTINGS_H_
#define _PLAYER_SETTINGS_H_

#define ART_NULL_COMP		_T("Null Audio Renderer (Any)")
#define ART_NULL_UNCOMP		_T("Null Audio Renderer (Uncompressed)")
#define ART_MPC				_T("MPC Audio Renderer")

#define IDS_VRT_OLDRENDERER			_T("Old Video Renderer")
#define IDS_VRT_OVERLAYMIXER		_T("Overlay Mixer Renderer")
#define IDS_VRT_VMR7WINDOWED		_T("Video Mixing Renderer 7 (windowed)")
#define IDS_VRT_VMR9WINDOWED		_T("Video Mixing Renderer 9 (windowed)")
#define IDS_VRT_VMR7RENDERLESS		_T("Video Mixing Renderer 7 (renderless)")
#define IDS_VRT_VMR9RENDERLESS		_T("Video Mixing Renderer 9 (renderless)")
#define IDS_VRT_EVR					_T("Enhanced Video Renderer")
#define IDS_VRT_EVR_CUSTOM			_T("Enhanced Video Renderer (custom presenter)")
#define IDS_VRT_HAALI				_T("Haali Video Renderer")


class CPlayerSettings : public CCritSec
{
protected:
	CPlayerSettings() {}
	~CPlayerSettings() {}
	CPlayerSettings(const CPlayerSettings&) {}
	CPlayerSettings& operator= (const CPlayerSettings&) {}

	static CAutoPtr<CPlayerSettings> m_pThis;

public:
	static CPlayerSettings* GetPlayerSettings() 
	{
		if (m_pThis == NULL)
		{
			this->Lock();
			if (m_pThis == NULL)
			{
				m_pThis = DNew CPlayerSettings();
			}
			this->Unlock();
		}
		return m_pThis;
	}

	VIDEO_RENDERER_TYPE m_VideoRendererType;

	int m_nVolume;
	BOOL m_bAutoPlay;

	BOOL m_bRenderOpenChain;	
};


#endif