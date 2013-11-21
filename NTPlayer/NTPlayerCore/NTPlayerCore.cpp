// NTPlayerCore.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "NTPlayerCore.h"
#include "PlayerCore.h"

static PlayerCore*  s_pPlayer = NULL;

NTPLAYERCORE_API long ntplayer_init(ntplayer_notify_to_ui notify_func, void* pUser)
{
    player_log(kLogLevelTrace, _T("ntplayer_init"));

    if (s_pPlayer == NULL)
    {
        s_pPlayer = new PlayerCore;

        HRESULT hr = s_pPlayer->Create(notify_func, pUser);
        if (FAILED(hr))
        {
            SAFE_DELETE(s_pPlayer);
            return hr;
        }
    }

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


NTPLAYERCORE_API long ntplayer_set_video_display(void* video_window, void* display_rect, bool is_full_screen)
{
    HRESULT hr = E_UNEXPECTED;
    if (s_pPlayer != NULL)
    {
        hr = s_pPlayer->SetVideoWindow((HWND)video_window);

        hr = s_pPlayer->SetVideoPosition((LPRECT)display_rect, FALSE);
    }

    return hr;
}

NTPLAYERCORE_API long ntplayer_update_video_display(void* display_rect, bool is_full_screen)
{
    HRESULT hr = E_UNEXPECTED;
    if (s_pPlayer != NULL)
    {
        hr = s_pPlayer->SetVideoPosition((LPRECT)display_rect, TRUE);
    }
    return hr;
}

NTPLAYERCORE_API ntplayer_state ntplayer_get_player_state()
{
    ntplayer_state state = kPlayerStateNothingSpecial;
    if (s_pPlayer != NULL)
    {
        state = s_pPlayer->GetPlayerState();
    }
    return state;
}