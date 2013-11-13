#include "stdafx.h"
#include "QvodStream.h"
#include "SHP2PService.h"

QvodStream::QvodStream()
: m_llSize(0)
, m_llPos(0)
, m_shp2p(NULL)
//, m_LoadEvent(TRUE)
{
   m_shp2p = P2PInstance();
}

QvodStream::~QvodStream()
{
    P2PFree(m_shp2p);
    m_shp2p = NULL;
}

void QvodStream::SetSavedPath(LPCTSTR path)
{
    m_savedpath = path;
}


HRESULT QvodStream::Load(LPCWSTR fn)
{
    HRESULT hr = E_FAIL;

    if (m_shp2p)
    {
        std::string url = wcs2mbs(CP_ACP, fn);
        std::string config = wcs2mbs(CP_ACP, m_strSavedPath);
        int ret = m_shp2p->Open(url, config, MessageNotify, PlayMode, UDPType, this);
        if (ret == 0)
        {
            m_evtLoad.Wait();

            if (m_fLoaded)
            {
                m_strUrl = fn;
                hr = S_OK;
            }
        }
        else
        {
            player_log(kLogLevelError, "SHP2PService Open got error");
        }
    }
    

    return hr;
}

HRESULT QvodStream::SetPointer(LONGLONG llPos)
{
    HRESULT hr = E_FAIL;
    if (m_shp2p)
    {
        if (0 == m_shp2p->Seek(llPos))
            hr = S_OK;
    }

    return hr;
}

HRESULT QvodStream::Read(PBYTE pbBuffer,
             DWORD dwBytesToRead,
             BOOL bAlign,
             LPDWORD pdwBytesRead)
{
    HRESULT hr = E_FAIL;

    if (m_shp2p)
    {
        DWORD dwBytesRead = 0;
        if (0 == m_shp2p->Read(pbBuffer, dwBytesToRead, dwBytesRead))
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

LONGLONG QvodStream::Size(LONGLONG *pSizeAvailable)
{
    if (*pSizeAvailable)
        pSizeAvailable = m_llSize;
    return m_llSize;
}

int QvodStream::MessageNotify(const void* pUser, int msg, void* wParam, void* lParam)
{
    QvodStream* pThis = (QvodStream*)pUser;
    if (!pThis)
    {
        player_log(_T("QvodStream::MessageNotify, null pUser"));
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
        break;
    }
    return 0;
}