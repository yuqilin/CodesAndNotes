#include "stdafx.h"
#include "PlayerQvodStream.h"
#include "SHP2PService.h"

PlayerQvodStream::PlayerQvodStream()
: m_shp2p(NULL)
{
}

PlayerQvodStream::~PlayerQvodStream()
{
    Close();
}

void PlayerQvodStream::SetSavedPath(LPCTSTR path)
{
    m_strSavedPath = path;
}


HRESULT PlayerQvodStream::Open(LPCTSTR lpUrl)
{
    HRESULT hr = E_FAIL;

    Close();

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
        }
        else
        {
            player_log(kLogLevelError, "SHP2PService Open got error");
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

HRESULT PlayerQvodStream::SetPointer(LONGLONG llPos)
{
    HRESULT hr = E_FAIL;

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

    if (m_shp2p)
    {
        DWORD dwBytesRead = 0;
        if (0 == m_shp2p->Read(m_llPosition, pbBuffer, dwBytesToRead, dwBytesRead))
        {
            if (pdwBytesRead)
                *pdwBytesRead = dwBytesRead;
            hr = S_OK;
        }
    }

    if (FAILED(hr))
    {
        if (pdwBytesRead)
            *pdwBytesRead = 0;
    }

    return hr;
}

LONGLONG PlayerQvodStream::Size(LONGLONG *pSizeAvailable)
{
    if (*pSizeAvailable)
        pSizeAvailable = m_llSize;
    return m_llSize;
}

int PlayerQvodStream::MessageNotify(const void* pUser, int msg, void* wParam, void* lParam)
{
    PlayerQvodStream* pThis = (PlayerQvodStream*)pUser;
    if (!pThis)
    {
        player_log(_T("PlayerQvodStream::MessageNotify, null pUser"));
        return -1;
    }

    switch (msg)
    {
    case MESSAGE_MEDIA_INFO:
        {
            FileTorrentInfo_t* fileinfo = (FileTorrentInfo_t*)lParam;
            if (fileinfo)
            {
                pThis->m_llSize = fileinfo->filelength;
                pThis->m_fLoaded = true;
            }
            if (pThis->m_evtLoad)
            {
                pThis->m_evtLoad.Set();
            }
        }
        break;
    case MESSAGE_MEDIA_P2P_DATA:
        break;
    case MESSAGE_MEDIA_CDN_DATA:
        break;
    case MESSAGE_MEDIA_STAT:
        break;
    case MESSAGE_MEDIA_FINI:
        {
            if (pThis->m_evtLoad)
            {
                pThis->m_evtLoad.Set();
            }
        }
        break;
    case MESSAGE_MEDIA_NET_ERROR:
        {
            if (pThis->m_evtLoad)
            {
                pThis->m_evtLoad.Set();
            }
        }
        break;
    }
    return 0;
}