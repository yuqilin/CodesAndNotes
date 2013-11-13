// NTPlayerCore.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "NTPlayerCore.h"
#include "PlayerCore.h"

static PlayerCore*  s_pPlayer = NULL;

NTPLAYERCORE_API long ntplayer_init()
{
    player_log(kLogLevelTrace, _T("ntplayer_init"));

    if (s_pPlayer == NULL)
    {
        s_pPlayer = new PlayerCore;

        PlayerCore::SetModuleInstance(g_hInstance);

        s_pPlayer->Create();
    }

    //     if (s_pPlayer != NULL)
    //     {
    //         s_pPlayer->LoadCodecsInfo();
    //     }

    return S_OK;
}

NTPLAYERCORE_API long ntplayer_uninit()
{
    player_log(kLogLevelTrace, _T("ntplayer_uninit"));

    SAFE_DELETE(s_pPlayer);

    return S_OK;
}

NTPLAYERCORE_API long ntplayer_get_codecs_info_string(const char** info)
{
    player_log(kLogLevelTrace, _T("ntplayer_get_codecs_info_string"));

    if (info != NULL && s_pPlayer != NULL)
    {
        //return s_pPlayer->GetCodecsInfoString(info);
        // TODO: not implemented
        *info = NULL;
        return S_OK;
    }
    return E_UNEXPECTED;
}

NTPLAYERCORE_API long ntplayer_open(const char* url)
{
    std::wstring wurl = mbs2wcs(CP_UTF8, url);
    player_log(kLogLevelTrace, _T("ntplayer_open, url = %s"), wurl.c_str());

    if (s_pPlayer != NULL)
    {
        return s_pPlayer->Open(wurl.c_str());
    }
    return E_UNEXPECTED;
}

NTPLAYERCORE_API long ntplayer_close()
{
    player_log(kLogLevelTrace, _T("ntplayer_close"));

    if (s_pPlayer != NULL)
    {
        return s_pPlayer->Close();
    }
    return E_UNEXPECTED;
}

NTPLAYERCORE_API long ntplayer_play()
{
    player_log(kLogLevelTrace, _T("ntplayer_play"));

    if (s_pPlayer)
    {
        return s_pPlayer->Play();
    }
    return E_UNEXPECTED;
}

NTPLAYERCORE_API long ntplayer_pause()
{
    player_log(kLogLevelTrace, _T("ntplayer_pause"));

    if (s_pPlayer)
    {
        return s_pPlayer->Pause();
    }
    return E_UNEXPECTED;
}

NTPLAYERCORE_API long ntplayer_stop()
{
    player_log(kLogLevelTrace, _T("ntplayer_stop"));

    if (s_pPlayer)
    {
        return s_pPlayer->Stop();
    }
    return E_UNEXPECTED;
}

NTPLAYERCORE_API long ntplayer_get_duration(long* duration)
{
    player_log(kLogLevelTrace, _T("ntplayer_get_duration"));

    if (s_pPlayer)
    {
        return s_pPlayer->GetDuration(duration);
    }
    return E_UNEXPECTED;
}

NTPLAYERCORE_API long ntplayer_set_play_pos(long pos_to_play)
{
    player_log(kLogLevelTrace, _T("ntplayer_set_play_pos = %d"));

    if (s_pPlayer)
    {
        return s_pPlayer->SetPlayPos(pos_to_play);
    }
    return E_UNEXPECTED;
}

NTPLAYERCORE_API long ntplayer_get_current_play_pos(long* current_play_pos)
{
    if (s_pPlayer)
    {
        return s_pPlayer->GetCurrentPlayPos(current_play_pos);
    }
    return E_UNEXPECTED;
}

