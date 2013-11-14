#pragma once

#include "PlayerBaseStream.h"

class PlayerFileStream : public PlayerBaseStream
{
public:
    PlayerFileStream();
    ~PlayerFileStream();

    // PlayerBaseStream
    HRESULT Open(LPCTSTR lpszFileName);
    void Close();

    // CAyncStream
    HRESULT SetPointer(LONGLONG llPos);
    HRESULT Read(PBYTE pbBuffer,
        DWORD dwBytesToRead,
        BOOL bAlign,
        LPDWORD pdwBytesRead);
    LONGLONG Size(LONGLONG *pSizeAvailable = NULL);

protected:
    HANDLE m_hFile;
};