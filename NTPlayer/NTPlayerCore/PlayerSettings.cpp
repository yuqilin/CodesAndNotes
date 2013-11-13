#include "stdafx.h"
#include "PlayerCore.h"
#include "PlayerSettings.h"
#include "../VideoRenderers/RenderersSettings.h"

//////////////////////////////////////////////////////////////////////////
bool g_bNoDuration = false;
bool g_bExternalSubtitleTime = false;

CRenderersData*      GetRenderersData();
CRenderersSettings&  GetRenderersSettings();

//////////////////////////////////////////////////////////////////////////
PlayerSettings::PlayerSettings()
{
}

PlayerSettings::~PlayerSettings()
{

}

HRESULT PlayerSettings::LoadSettings()
{
    player_log(kLogLevelTrace, _T("PlayerSettings::LoadSettings"));

    if (SysVersion::IsVistaOrLater())
    {
        m_VideoRenderMode = kVideoRenderEVR;
    }
    else
    {
        m_VideoRenderMode = kVideoRenderDefault;
    }

    return S_OK;
}

CString PlayerSettings::SelVideoRenderer()
{
    GUID selVideoRenderer = GUID_NULL;
    switch (m_VideoRenderMode)
    {
    case kVideoRenderOldRenderer:
        selVideoRenderer = CLSID_VideoRenderer;
        break;
    case kVideoRenderOverlayMixer:
        selVideoRenderer = CLSID_OverlayMixer;
        break;
    case kVideoRenderVMR7Windowed:
    case kVideoRenderVMR7Renderless:
        selVideoRenderer = CLSID_VideoMixingRenderer;
        break;
    case kVideoRenderVMR9Windowed:
    case kVideoRenderVMR9Renderless:
        selVideoRenderer = CLSID_VideoMixingRenderer9;
        break;
    case kVideoRenderEVR:
    case kVideoRenderEVRCustom:
        selVideoRenderer = CLSID_EnhancedVideoRenderer;
        break;
    }
    return CStringFromGUID(selVideoRenderer);
}


CRenderersData* GetRenderersData()
{
    return PlayerCore::GetPlayerSettings().GetRenderersData();
}

CRenderersSettings&  GetRenderersSettings()
{
    return PlayerCore::GetPlayerSettings().GetRenderersSettings();
}