#pragma once

#include "../VideoRenderers/RenderersSettings.h"

enum {
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
    kVideoRenderEVRCustom,
    kVideoRenderMadVR,
    kVideoRenderSync
};

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

    int m_VideoRenderMode;
    CString m_AudioRendererDisplayName;
    CString m_SelVideoRendererClsid;

protected:
    CRenderersData m_Renderers;
    CRenderersSettings m_RenderersSettings;
};

