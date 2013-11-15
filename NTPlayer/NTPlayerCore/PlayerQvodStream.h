#pragma once

#include "PlayerBaseStream.h"

class SHP2PService;

class PlayerQvodStream : public PlayerBaseStream
{
public:
    PlayerQvodStream();
    ~PlayerQvodStream();

    void SetSavedPath(LPCTSTR path);

    virtual HRESULT Open(LPCTSTR lpUrl);
    virtual void Close();

    // CAsyncStream
    virtual HRESULT SetPointer(LONGLONG llPos);
    virtual HRESULT Read(PBYTE pbBuffer,
        DWORD dwBytesToRead,
        BOOL bAlign,
        LPDWORD pdwBytesRead);
    virtual LONGLONG Size(LONGLONG *pSizeAvailable);

    static int MessageNotify(const void* pUser, int msg, void* wParam, void* lParam);

protected:
    CString m_strSavedPath;
    CString m_strTitle;
    SHP2PService* m_shp2p;

    CAMEvent m_evtLoad;
    BOOL m_fLoaded;


};