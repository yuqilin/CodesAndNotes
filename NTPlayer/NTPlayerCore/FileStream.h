#pragma once

#include "PlayerBaseStream.h"

class FileStream : public PlayerBaseStream
{
public:
    virtual HRESULT Load(LPCWSTR fn);

    // CAsyncStream
    virtual HRESULT SetPointer(LONGLONG llPos);
    virtual HRESULT Read(PBYTE pbBuffer,
        DWORD dwBytesToRead,
        BOOL bAlign,
        LPDWORD pdwBytesRead);
    virtual LONGLONG Size(LONGLONG *pSizeAvailable);

protected:
    BOOL Open(LPCTSTR path);
    void Close();

protected:
    CString m_strPath;
    HANDLE m_hFile;
    LONGLONG m_llSize;


};