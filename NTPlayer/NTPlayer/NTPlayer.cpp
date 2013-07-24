// NTPlayer.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "NTPlayer.h"
#include "NTPlayerImpl.h"

static CNTPlayer*  s_pPlayer = NULL;

NTPLAYER_API int ntplayer_init()
{
    if (s_pPlayer == NULL)
    {
        s_pPlayer = new CNTPlayer;
    }

    s_pPlayer->LoadCodecsInfo();

    return 0;
}

NTPLAYER_API int ntplayer_uninit()
{
    SAFE_DELETE(s_pPlayer);

    return 0;
}

NTPLAYER_API int ntplayer_get_codecs_info_string(const char** info)
{
    if (info != NULL && s_pPlayer != NULL)
    {
        s_pPlayer->GetCodecsInfoString(info);

        return 0;
    }
    return -1;
}