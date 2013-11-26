#pragma once

#include "../filters/renderer/VideoRenderers/RenderersSettings.h"

typedef enum {
    kVideoRenderDefault = 0,
    kVideoRenderOldRenderer,
    kVideoRenderOverlayMixer,
    kVideoRenderVMR7Windowed,
    kVideoRenderVMR9Windowed,
    kVideoRenderVMR7Renderless,
    kVideoRenderVMR9Renderless,
    kVideoRenderDXR,
    kVideoRenderNullComp,
    kVideoRenderNullUnComp,
    kVideoRenderEVR,
    kVideoRenderEVRCP,
    kVideoRenderMadVR,
    kVideoRenderSync
} VideoRenderMode;

class PlayerSettings
{
public:
    PlayerSettings();
    ~PlayerSettings();

    HRESULT LoadSettings();

    CString SelVideoRenderer();

    CRenderersSettings& GetRenderersSettings() {
        return m_RenderersSettings;
    }
    CRenderersData* GetRenderersData() {
        return &m_Renderers;
    }

    VideoRenderMode m_VideoRenderMode;
    CString m_AudioRendererDisplayName;
    CString m_SelVideoRendererClsid;

    bool m_fVsfilterEnabled;
    bool m_fAudioSwitcherEnabled;
    
    bool m_fEnableSubtitles;
    
    CStringW strSubtitlesLanguageOrder;

    bool bPreferDefaultForcedSubtitles;

protected:
    CRenderersData m_Renderers;
    CRenderersSettings m_RenderersSettings;
};

