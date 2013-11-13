#pragma once

class SHP2PService;

class QvodStream : public PlayerBaseStream
{
public:
    QvodStream();
    ~QvodStream();

    void SetSavedPath(LPCTSTR path);

    virtual HRESULT Load(LPCWSTR fn);

    // CAsyncStream
    virtual HRESULT SetPointer(LONGLONG llPos);
    virtual HRESULT Read(PBYTE pbBuffer,
        DWORD dwBytesToRead,
        BOOL bAlign,
        LPDWORD pdwBytesRead);
    virtual LONGLONG Size(LONGLONG *pSizeAvailable);

    static int MessageNotify(const void* pUser, int msg, void* wParam, void* lParam);

protected:
    LONGLONG m_llSize;
    LONGLONG m_llPos;
    CString m_strUrl;
    CString m_strSavedPath;
    SHP2PService* m_shp2p;

    CAMEvent m_evtLoad;
    BOOL m_fLoaded;


};