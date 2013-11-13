#include "stdafx.h"
#include "PlayerFileStream.h"

PlayerFileStream::PlayerFileStream()
: m_hFile(INVALID_HANDLE_VALUE)
{
}

BOOL PlayerFileStream::Open(LPCTSTR lpszFileName)
{
    m_hFile = CreateFile(lpszFileName, GENERIC_READ, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        m_strFileName = lpszFileName;
        return TRUE;
    }

    return FALSE;
}

void PlayerFileStream::Close()
{
    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }
}

HRESULT PlayerFileStream::SetPointer(LONGLONG llPos)
{
    return (llPos >= 0 && llPos < m_llFileSize) ? m_llPosition = llPos, S_OK : S_FALSE;
}

HRESULT PlayerFileStream::Read(PBYTE pbBuffer,
             DWORD dwBytesToRead,
             BOOL bAlign,
             LPDWORD pdwBytesRead)
{
    
}

LONGLONG PlayerFileStream::Size(LONGLONG *pSizeAvailable = NULL)
{
    
}