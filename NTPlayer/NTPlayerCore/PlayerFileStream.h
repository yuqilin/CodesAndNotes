#pragma once

class PlayerFileStream : public PlayerBaseStream
{
public:
    PlayerFileStream();
    ~PlayerFileStream();

    // PlayerBaseStream
    HRESULT Load(LPCWSTR fn);

    // Operations
    BOOL Open(LPCTSTR lpszFileName);
    void Close();

    HRESULT SetPointer(LONGLONG llPos);
    HRESULT Read(PBYTE pbBuffer,
        DWORD dwBytesToRead,
        BOOL bAlign,
        LPDWORD pdwBytesRead);
    LONGLONG Size(LONGLONG *pSizeAvailable = NULL);

protected:
    CString m_strFileName;
    HANDLE m_hFile;
    ULONGLONG m_llFileSize;
    LONGLONG m_llPosition;
};