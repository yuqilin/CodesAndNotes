// SHDLNA.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "SHDLNAMediaPlayer.h"

NPT_SET_LOCAL_LOGGER("shdlnaplayer.shdlnaplayer");

typedef NPT_Reference<CSHDLNAMediaPlayer>		CSHDLNAMediaPlayerRerference;
static CSHDLNAMediaPlayerRerference	g_MediaPlayer;

static void SH_DLNAPlayer_SetLog();

/*----------------------------------------------------------------------
|   SH_DLNAPlayer_Init
+---------------------------------------------------------------------*/
SH_DLNAPLAYER_API int SH_DLNAPlayer_Init(SH_DLNAPlayer_MessageNotifyUI message_to_notify)
{
	SH_DLNAPlayer_SetLog();

	if (g_MediaPlayer.IsNull())
	{
		g_MediaPlayer = new CSHDLNAMediaPlayer();
		if (!g_MediaPlayer.IsNull())
		{
			g_MediaPlayer->Init(message_to_notify);
		}
	}

	return 0;
}

/*----------------------------------------------------------------------
|   SH_DLNAPlayer_Uninit
+---------------------------------------------------------------------*/
SH_DLNAPLAYER_API int SH_DLNAPlayer_Uninit(void)
{
	if (!g_MediaPlayer.IsNull())
		g_MediaPlayer->Uninit();
	g_MediaPlayer = NULL;
	return 0;
}

/*----------------------------------------------------------------------
|   SH_DLNAPlayer_ChooseDevice
+---------------------------------------------------------------------*/
SH_DLNAPLAYER_API int SH_DLNAPlayer_ChooseDevice(const char* device_uuid)
{
	if (!g_MediaPlayer.IsNull())
	{
		return g_MediaPlayer->ChooseDevice(device_uuid);
	}
	return 0;
}

/*----------------------------------------------------------------------
|   SH_DLNAPlayer_Open
+---------------------------------------------------------------------*/
SH_DLNAPLAYER_API int SH_DLNAPlayer_Open(const char* url_utf8)
{
	if (!g_MediaPlayer.IsNull())
	{
		return g_MediaPlayer->Open(url_utf8);
	}
	return 0;
}

/*----------------------------------------------------------------------
|   SH_DLNAPlayer_Close
+---------------------------------------------------------------------*/
SH_DLNAPLAYER_API int SH_DLNAPlayer_Close(void)
{
	if (!g_MediaPlayer.IsNull())
	{
		return g_MediaPlayer->Close();
	}
	return 0;
}

/*----------------------------------------------------------------------
|   SH_DLNAPlayer_Play
+---------------------------------------------------------------------*/
SH_DLNAPLAYER_API int SH_DLNAPlayer_Play(void)
{
	if (!g_MediaPlayer.IsNull())
	{
		return g_MediaPlayer->Play();
	}
	return 0;
}

/*----------------------------------------------------------------------
|   SH_DLNAPlayer_Seek
+---------------------------------------------------------------------*/
SH_DLNAPLAYER_API int SH_DLNAPlayer_Seek(long pos_to_play)
{
	if (!g_MediaPlayer.IsNull())
	{
		return g_MediaPlayer->Seek(pos_to_play);
	}
	return 0;
}

/*----------------------------------------------------------------------
|   SH_DLNAPlayer_Pause
+---------------------------------------------------------------------*/
SH_DLNAPLAYER_API int SH_DLNAPlayer_Pause(void)
{
	if (!g_MediaPlayer.IsNull())
	{
		return g_MediaPlayer->Pause();
	}
	return 0;
}

/*----------------------------------------------------------------------
|   SH_DLNAPlayer_Stop
+---------------------------------------------------------------------*/
SH_DLNAPLAYER_API int SH_DLNAPlayer_Stop(void)
{
	if (!g_MediaPlayer.IsNull())
	{
		return g_MediaPlayer->Stop();
	}
	return 0;
}

/*----------------------------------------------------------------------
|   SH_DLNAPlayer_SetVolume
+---------------------------------------------------------------------*/
SH_DLNAPLAYER_API int SH_DLNAPlayer_SetVolume(int volume)
{
	if (!g_MediaPlayer.IsNull())
	{
		return g_MediaPlayer->SetVolume(volume);
	}
	return 0;
}

/*----------------------------------------------------------------------
|   SH_DLNAPlayer_GetMediaDuration
+---------------------------------------------------------------------*/
SH_DLNAPLAYER_API int SH_DLNAPlayer_GetMediaDuration(void)
{
	if (!g_MediaPlayer.IsNull())
	{
		return g_MediaPlayer->GetMediaDuration();
	}
	return 0;
}

/*----------------------------------------------------------------------
|   SH_DLNAPlayer_GetCurPlayPos
+---------------------------------------------------------------------*/
SH_DLNAPLAYER_API int SH_DLNAPlayer_GetCurPlayPos(void)
{
	if (!g_MediaPlayer.IsNull())
	{
		return g_MediaPlayer->GetCurPlayPos();
	}
	return 0;
}

/*----------------------------------------------------------------------
|   SH_DLNAPlayer_GetVolume
+---------------------------------------------------------------------*/
SH_DLNAPLAYER_API int SH_DLNAPlayer_GetVolume(void)
{
	if (!g_MediaPlayer.IsNull())
	{
		return g_MediaPlayer->GetVolume();
	}
	return 0;
}

/*----------------------------------------------------------------------
|   SH_DLNAPlayer_SetLog
+---------------------------------------------------------------------*/
static void SH_DLNAPlayer_SetLog()
{
	char log_path[MAX_PATH] = {0};
	if (GetTempPathA(MAX_PATH, log_path) <= 0)
	{
		strcpy_s(log_path, MAX_PATH, "C:\\");
	}
	if (log_path[strlen(log_path)-1] != '\\')
		strcat_s(log_path, MAX_PATH, "\\");
	strcat_s(log_path, "SHDLNAPlayer.log");
	NPT_String strLogConfig = NPT_String::Format("plist:.level=ALL;.handlers=FileHandler;"
		".FileHandler.filename=%s;.FileHandler.filter=9;.FileHandler.append=false;.FileHandler.flush=true;", log_path);
	NPT_LogManager::GetDefault().Configure(strLogConfig);
}
