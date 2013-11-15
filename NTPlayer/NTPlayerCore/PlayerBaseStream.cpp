#include "stdafx.h"
#include "PlayerBaseStream.h"

PlayerBaseStream::PlayerBaseStream()
: m_llSize(0)
, m_llPosition(0)
, m_subtype(GUID_NULL)
, m_bAbort(FALSE)
{

}

PlayerBaseStream::~PlayerBaseStream()
{
    Close();
}

void PlayerBaseStream::Close()
{
    m_Header.Reset();
    m_strUrl.Empty();
    m_llSize = 0;
    m_llPosition = 0;
}

HRESULT PlayerBaseStream::LoadHeader()
{
    HRESULT hr = E_FAIL;

    m_Header.Reset();

    const int kMediaHeaderLength = 65536;
    int nDataLength = max(kMediaHeaderLength, (min(Size(), kMediaHeaderLength)));

    assert(nDataLength > 0);

    BYTE* pData = new BYTE[nDataLength];
    if (pData == NULL)
        return E_OUTOFMEMORY;

    BYTE* pHeader = pData;
    int  nBytesToRead = nDataLength;
    DWORD dwBytesRead = 0;
    LONGLONG llPos = 0;

    do {
        Sleep(50);
        if (SUCCEEDED(hr = SetPointer(llPos)))
        {
            dwBytesRead = 0;
            hr = Read(pHeader, nBytesToRead, FALSE, &dwBytesRead);
            if (SUCCEEDED(hr))
            {
                pHeader += dwBytesRead;
                llPos += dwBytesRead;
                nBytesToRead -= dwBytesRead;
            }
        }
    } while (!m_bAbort && (FAILED(hr) || nBytesToRead > 0));

    if (nBytesToRead == 0)
    {
        assert(SUCCEEDED(hr));

        m_Header.data = pData;
        m_Header.size = nDataLength;
    }
    else if (nBytesToRead > 0 && nBytesToRead < nDataLength)
    {
        assert(SUCCEEDED(hr));

        //hr = S_OK;

        m_Header.data = pData;
        m_Header.size = nDataLength - nBytesToRead;
    }
    else
    {
        delete[] pData;
        hr = E_FAIL;
    }

    return hr;
}