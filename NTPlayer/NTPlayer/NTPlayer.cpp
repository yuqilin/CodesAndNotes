// NTPlayer.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "NTPlayer.h"
#include "NTPlayerImpl.h"

static CPlayerCore*  s_pPlayer = NULL;

NTPLAYER_API NTPLAYER_RESULT ntplayer_init()
{
    if (s_pPlayer == NULL)
    {
        s_pPlayer = new CPlayerCore;
    }

//     if (s_pPlayer != NULL)
//     {
//         s_pPlayer->LoadCodecsInfo();
//     }

    return S_OK;
}

NTPLAYER_API NTPLAYER_RESULT ntplayer_uninit()
{
    SAFE_DELETE(s_pPlayer);

    return S_OK;
}

NTPLAYER_API NTPLAYER_RESULT ntplayer_get_codecs_info_string(const char** info)
{
    if (info != NULL && s_pPlayer != NULL)
    {
        //return s_pPlayer->GetCodecsInfoString(info);
        // TODO: not implemented
        *info = NULL;
        return S_OK;
    }
    return E_FAIL;
}

NTPLAYER_API NTPLAYER_RESULT ntplayer_open(const char* url)
{
    if (s_pPlayer != NULL)
    {
        return s_pPlayer->Open(url);
    }
    return E_FAIL;
}

NTPLAYER_API NTPLAYER_RESULT ntplayer_close(const char* url)
{
    if (s_pPlayer != NULL)
    {
        return s_pPlayer->Close(url);
    }
    return E_FAIL;
}