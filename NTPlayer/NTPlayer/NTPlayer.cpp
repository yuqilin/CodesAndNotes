// NTPlayer.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "NTPlayer.h"
#include "NTPlayerImpl.h"

static CNTPlayer*  s_pPlayer = NULL;

NTPLAYER_API HRESULT ntplayer_init()
{
    if (s_pPlayer == NULL)
    {
        s_pPlayer = new CNTPlayer;
    }

    if (s_pPlayer != NULL)
    {
        s_pPlayer->LoadCodecsInfo();
    }

    return S_OK;
}

NTPLAYER_API HRESULT ntplayer_uninit()
{
    SAFE_DELETE(s_pPlayer);

    return S_OK;
}

NTPLAYER_API HRESULT ntplayer_get_codecs_info_string(const char** info)
{
    if (info != NULL && s_pPlayer != NULL)
    {
        return s_pPlayer->GetCodecsInfoString(info);
    }
    return E_FAIL;
}

NTPLAYER_API HRESULT ntplayer_open_url(const char* url)
{
    if (s_pPlayer != NULL)
    {
        return s_pPlayer->OpenUrl(url);
    }
    return E_FAIL;
}