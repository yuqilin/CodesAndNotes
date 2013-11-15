#include "stdafx.h"
#include "MediaInfo.h"
#include "PlayerBaseStream.h"

#define MAX_URL_LENGTH          2048

MediaInfo::MediaInfo(LPCTSTR lpcszUrl, HRESULT& hr)
: m_pUrl(NULL)
, m_pProtocol(NULL)
, m_pExtension(NULL)
, m_pTitle(NULL)
{
    hr = Create(lpcszUrl);
}

MediaInfo::~MediaInfo()
{
    Destroy();
}

HRESULT MediaInfo::Create(LPCTSTR pcszUrl)
{
    CheckPointer(pcszUrl, E_POINTER);

    int nUrlLength = _tcslen(pcszUrl)+1;
    if(nUrlLength < 1 || nUrlLength > MAX_URL_LENGTH)
        return E_INVALIDARG;

    /*
     *	Parse Url
     */
    m_pUrl = new TCHAR[nUrlLength];
    m_pExtension = new TCHAR[nUrlLength];
    m_pProtocol = new TCHAR[nUrlLength];

    if(m_pUrl == NULL || m_pProtocol == NULL || m_pExtension == NULL)
        return E_OUTOFMEMORY;

    _tcscpy_s(m_pUrl, nUrlLength+1, pcszUrl);
    GetFileExtnameFromURL(m_pUrl, m_pExtension);
    player_log(kLogLevelTrace, _T("Media Extname = %s"), m_pExtension);

    GetProtocolFromURL(m_pUrl, m_pProtocol);
    player_log(kLogLevelTrace, _T("Media Protocol = %s"), m_pProtocol);

    // TODO: move read stream to PlayerCore::DoOpen
    /*
     *	allocate stream
     */

    /*
     *	Read Media Header
     */
//     m_Header.data = new BYTE[MEDIA_HEADER_LENGTH];
//     if(m_Header.data == NULL)
//         return E_OUTOFMEMORY;
//     // Read media header
//     HRESULT hr = m_pStream->Open(m_pUrl, TRUE);
//     if(SUCCEEDED(hr))
//     {
//         UINT64 nTotal = 0, nAvailable = 0;
//         if (SUCCEEDED(m_pStream->GetSize(&nTotal, &nAvailable)))
//             m_nSize = nTotal;
// 
//         DWORD dwRead = 0;
//         hr = m_pStream->Read(m_Header.data, MEDIA_HEADER_LENGTH, &dwRead);
//         if(SUCCEEDED(hr))
//             m_Header.size = dwRead;
//     }

    return S_OK;
}

void MediaInfo::Destroy()
{
    SAFE_DELETE(m_pUrl);
    SAFE_DELETE(m_pProtocol);
    SAFE_DELETE(m_pExtension);
    SAFE_DELETE(m_pTitle);
}

MediaProtocol ProtocolFromString(LPCTSTR pcszProtocol)
{
    if (_tcsicmp(pcszProtocol, _T("file")) == 0)
    {
        return kProtocolFile;
    }
    else if (_tcsicmp(pcszProtocol, _T("http")) == 0)
    {
        return kProtocolHttp;
    }
    else if (_tcsicmp(pcszProtocol, _T("https")) == 0)
    {
        return kProtocolHttps;
    }
    else if (_tcsicmp(pcszProtocol, _T("qvod")) == 0)
    {
        return kProtocolQvod;
    }
    else if (_tcsicmp(pcszProtocol, _T("bdhd")) == 0)
    {
        return kProtocolBdhd;
    }
    return kProtocolUnknown;
}