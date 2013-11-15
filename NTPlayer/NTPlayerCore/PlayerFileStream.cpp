#include "stdafx.h"
#include "PlayerFileStream.h"

PlayerFileStream::PlayerFileStream()
: m_hFile(INVALID_HANDLE_VALUE)
{
}

PlayerFileStream::~PlayerFileStream()
{
    Close();
}

HRESULT PlayerFileStream::Open(LPCTSTR lpszFileName)
{
    HRESULT hr = E_FAIL;

    Close();
    
    m_hFile = CreateFile(lpszFileName, GENERIC_READ, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        m_strUrl = lpszFileName;
        
        hr = LoadHeader();
    }

    return hr;
}

void PlayerFileStream::Close()
{
    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }

    __super::Close();
}

HRESULT PlayerFileStream::SetPointer(LONGLONG llPos)
{
    HRESULT hr = E_FAIL;

    LARGE_INTEGER llNewPos;
    LARGE_INTEGER llOff;
    llOff.QuadPart = llPos;

    BOOL bSeek = SetFilePointerEx(m_hFile, llOff, &llNewPos, FILE_BEGIN);
    if (bSeek)
        hr = S_OK;

    return hr;
}

HRESULT PlayerFileStream::Read(PBYTE pbBuffer,
             DWORD dwBytesToRead,
             BOOL bAlign,
             LPDWORD pdwBytesRead)
{
    HRESULT hr = E_FAIL;

    DWORD dwBytesRead = 0;
    BOOL bRead = ReadFile(m_hFile, pbBuffer, dwBytesToRead, &dwBytesRead, NULL);
    if (bRead)
    {
        hr = S_OK;
        if (pdwBytesRead)
            *pdwBytesRead = dwBytesRead;
    }
    else if (pdwBytesRead)
    {
        *pdwBytesRead = 0;
    }

    return hr;
}

LONGLONG PlayerFileStream::Size(LONGLONG *pSizeAvailable)
{
    LARGE_INTEGER llSize;
    if (GetFileSizeEx(m_hFile, &llSize))
    {
        m_llSize = llSize.QuadPart;
        if (pSizeAvailable)
            *pSizeAvailable = llSize.QuadPart;
    }
    else if (pSizeAvailable)
    {
        *pSizeAvailable = 0;
    }
    return m_llSize;
}