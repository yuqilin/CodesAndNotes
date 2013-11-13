
#include "stdafx.h"
#include "FileStream.h"

FileStream::FileStream()
: m_hFile(INVALID_HANDLE_VALUE)
{

}

FileStream::~FileStream()
{
    Close();
}

HRESULT FileStream::Load(LPCWSTR fn)
{
    HRESULT hr = E_FAIL;

    if (TRUE == Open(fn))
    {
        hr = S_OK;

    }

    return hr;
}

HRESULT FileStream::SetPointer(LONGLONG llPos)
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

HRESULT FileStream::Read(PBYTE pbBuffer,
                         DWORD dwBytesToRead,
                         BOOL bAlign,
                         LPDWORD pdwBytesRead)
{
    HRESULT hr = E_FAIL;

    DWORD dwBytesRead = 0;
    BOOL bRead = ReadFile(m_hFile, pbBuffer, dwBytesToRead, &dwBytesRead, NULL);
    if (bRead)
    {
        if (pdwBytesRead)
            *pdwBytesRead = dwBytesRead;
        hr = S_OK;
    }

    if (S_OK == hr && pdwBytesRead)
    {
        *pdwBytesRead = dwBytesRead;
    }
    else if (pdwBytesRead)
    {
        *pdwBytesRead = 0;
    }

    return hr;
}

LONGLONG FileStream::Size(LONGLONG *pSizeAvailable)
{
    LARGE_INTEGER llSize;
    if (GetFileSizeEx(m_hFile, &llSize))
    {
        m_llSize = llSize.QuadPart;
        if (pSizeAvailable)
            *pSizeAvailable = m_llSize;
    }
    else if (pSizeAvailable)
    {
        *pSizeAvailable = 0;
    }
    return m_llSize;
}

BOOL FileStream::Open(LPCTSTR path)
{
    Close();

    m_hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        m_strPath = path;
    }

    return (m_hFile != INVALID_HANDLE_VALUE);
}

void FileStream::Close()
{
    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }
}

