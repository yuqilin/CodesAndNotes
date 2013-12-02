#include "stdafx.h"
#include "PlayerQvodStream.h"
#include "SHP2PService.h"

PlayerQvodStream::PlayerQvodStream()
: m_shp2p(NULL)
, m_fLoaded(FALSE)
{
}

PlayerQvodStream::~PlayerQvodStream()
{
    Close();
}

void PlayerQvodStream::SetDownloadSavePath(LPCTSTR path)
{
    m_strSavedPath = path;
}


HRESULT PlayerQvodStream::Open(LPCTSTR lpUrl)
{
    HRESULT hr = E_FAIL;

    Close();

    m_shp2p = P2PInstance();

    if (m_shp2p)
    {
        std::string url = wcs2mbs(CP_ACP, lpUrl);
        std::string savedpath = wcs2mbs(CP_ACP, m_strSavedPath);
        int ret = m_shp2p->Open(url, savedpath, MessageNotify, PlayMode, UDPType, this);
        if (ret == 0)
        {
            m_evtLoad.Wait();

            if (m_fLoaded)
            {
                m_strUrl = lpUrl;

                hr = LoadHeader();
            }
            else if (m_bAbort)
            {
                hr = E_ABORT;
            }
        }
        else
        {
            player_log(kLogLevelError, _T("SHP2PService Open got error, ret = %d"), ret);
        }
    }

    return hr;
}

void PlayerQvodStream::Close()
{
    P2PFree(m_shp2p);
    m_shp2p = NULL;

    __super::Close();
}

void PlayerQvodStream::Abort()
{
    m_bAbort = TRUE;
    m_evtLoad.Set();
}

HRESULT PlayerQvodStream::SetPointer(LONGLONG llPos)
{
    HRESULT hr = S_OK;

    if (llPos >=0 && llPos < m_llSize)
    {
        m_llPosition = llPos;
    }

    return hr;
}

HRESULT PlayerQvodStream::Read(PBYTE pbBuffer,
             DWORD dwBytesToRead,
             BOOL bAlign,
             LPDWORD pdwBytesRead)
{
    HRESULT hr = E_FAIL;

    DWORD dwBytesLeft = dwBytesToRead;
    BYTE* pBuffer = pbBuffer;
    unsigned long uReadPos = m_llPosition;

    do {
        Sleep(10);

        DWORD dwBytesReadOnce = 0;
        if (m_shp2p && 0 == m_shp2p->Read(uReadPos, pBuffer, dwBytesLeft, dwBytesReadOnce))
        {
            dwBytesLeft -= dwBytesReadOnce;
            pBuffer += dwBytesReadOnce;
            uReadPos += dwBytesReadOnce;
        }
        else
        {
            player_log(kLogLevelTrace, _T("PlayerQvodStream::Read, m_shp2p->Read FAILED, pos=%d, bytes_to_read=%d, bytes_actual_read=%d"),
                uReadPos, dwBytesLeft, dwBytesReadOnce);
        }

    } while (!m_bAbort && dwBytesLeft > 0 /*&& pBuffer-pbBuffer<dwBytesToRead*/);

    if (dwBytesLeft == 0)
    {
        if (pdwBytesRead)
            *pdwBytesRead = dwBytesToRead;
        hr = S_OK;
    }

    if (FAILED(hr))
    {
        if (pdwBytesRead)
            *pdwBytesRead = 0;
    }

    if (m_bAbort)
        hr = E_ABORT;

    player_log(kLogLevelTrace, _T("PlayerQvodStream::Read, pos=%I64d, bytes_to_read=%d, bytes_actual_read=%d, hr=0x%08x"),
        m_llPosition, dwBytesToRead, dwBytesToRead-dwBytesLeft, hr);

    return hr;
}

LONGLONG PlayerQvodStream::Size(LONGLONG *pSizeAvailable)
{
    if (pSizeAvailable)
        *pSizeAvailable = m_llSize;
    return m_llSize;
}

int PlayerQvodStream::MessageNotify(const void* pUser, int msg, void* wParam, void* lParam)
{
    PlayerQvodStream* pThis = (PlayerQvodStream*)pUser;
    if (!pThis)
    {
        player_log(kLogLevelError, _T("PlayerQvodStream::MessageNotify, null pUser"));
        return -1;
    }

    switch (msg)
    {
    case MESSAGE_MEDIA_INFO:
        {
            player_log(kLogLevelTrace, _T("PlayerQvodStream::MessageNotify, MESSAGE_MEDIA_INFO"));

            FileTorrentInfo_t* fileinfo = (FileTorrentInfo_t*)lParam;
            if (fileinfo)
            {
                pThis->m_llSize = fileinfo->filelength;
                pThis->m_fLoaded = TRUE;
            }
            if (pThis->m_evtLoad)
            {
                pThis->m_evtLoad.Set();
            }
        }
        break;
    case MESSAGE_MEDIA_P2P_DATA:
        player_log(kLogLevelTrace, _T("PlayerQvodStream::MessageNotify, MESSAGE_MEDIA_P2P_DATA"));

        break;
    case MESSAGE_MEDIA_CDN_DATA:
        player_log(kLogLevelTrace, _T("PlayerQvodStream::MessageNotify, MESSAGE_MEDIA_CDN_DATA"));

        break;
    case MESSAGE_MEDIA_STAT:
        player_log(kLogLevelTrace, _T("PlayerQvodStream::MessageNotify, MESSAGE_MEDIA_STAT"));

        break;
    case MESSAGE_MEDIA_FINI:
        {
            player_log(kLogLevelTrace, _T("PlayerQvodStream::MessageNotify, MESSAGE_MEDIA_FINI"));

            if (pThis->m_evtLoad)
            {
                pThis->m_evtLoad.Set();
            }
        }
        break;
    case MESSAGE_MEDIA_NET_ERROR:
        {
            player_log(kLogLevelTrace, _T("PlayerQvodStream::MessageNotify, MESSAGE_MEDIA_NET_ERROR"));

            if (pThis->m_evtLoad)
            {
                pThis->m_evtLoad.Set();
            }
        }
        break;
    }
    return 0;
}