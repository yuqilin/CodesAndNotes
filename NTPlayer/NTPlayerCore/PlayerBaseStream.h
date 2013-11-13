#pragma once

#include "../AsyncReader/asyncio.h"

class PlayerBaseStream : public CAsyncStream
{
public:
    PlayerBaseStream()
        : m_subtype(GUID_NULL)
        , m_fileheader(NULL)
    {}
    virtual ~PlayerBaseStream()
    {
        Unload();
        SAFE_DELETE(m_fileheader);
    }

    virtual HRESULT Load(LPCWSTR fn) { return E_NOTIMPL; }
    virtual void Unload() { }

    virtual DWORD Alignment() { return 1; }

    virtual void Lock(void) { m_csLock.Lock(); }
    virtual void Unlock(void) { m_csLock.Unlock(); }

    MEMORY_DATA* GetFileHeader() { return m_fileheader; }

protected:
    CCritSec			m_csLock;
    GUID                m_subtype;
    MEMORY_DATA*        m_fileheader;
};


class PlayerAsyncStream : public PlayerBaseStream
{
public:
    PlayerAsyncStream() : m_pDelegate (NULL)
    {
    }

    virtual ~PlayerAsyncStream() {
        SAFE_DELETE(m_pDelegate);
    }

    HRESULT SetPlayerStream(PlayerBaseStream* pStream) {
        CheckPointer(pStream, E_POINTER);
        m_pDelegate = pStream;
        return S_OK;
    }

//     HRESULT Load(LPCWSTR fn) {
//         if (m_pDelegate)
//             return m_pDelegate->Load(fn);
//         return E_NOTIMPL;
//     }

    HRESULT SetPointer(LONGLONG llPos) {
        if (m_pDelegate)
            return m_pDelegate->SetPointer(llPos);
        return E_NOTIMPL;
    }

    HRESULT Read(PBYTE pbBuffer,
        DWORD dwBytesToRead,
        BOOL bAlign,
        LPDWORD pdwBytesRead) {
        if (m_pDelegate)
            return m_pDelegate->Read(pbBuffer, dwBytesToRead, bAlign, pdwBytesRead);
        return E_NOTIMPL;
    }

    LONGLONG Size(LONGLONG *pSizeAvailable = NULL) {
        if (m_pDelegate)
            return m_pDelegate->Size(pSizeAvailable);
        return E_NOTIMPL;
    }

    DWORD Alignment() {
        if (m_pDelegate)
            return m_pDelegate->Alignment();
        return E_NOTIMPL;
    }
protected:
    PlayerBaseStream* m_pDelegate;
};
