#pragma once

#include "../AsyncReader/asyncio.h"

class PlayerBaseStream : public CAsyncStream
{
public:
    PlayerBaseStream();
    virtual ~PlayerBaseStream();

    virtual HRESULT Open(LPCTSTR pUrl) { return E_NOTIMPL; }
    virtual void Close();
    virtual void Abort() {  }
    MEMORY_DATA* GetHeader() { return &m_Header; }


    // CAsyncStream
    virtual DWORD Alignment() { return 1; }
    virtual void Lock(void) { m_csLock.Lock(); }
    virtual void Unlock(void) { m_csLock.Unlock(); }


protected:
    virtual HRESULT LoadHeader();

protected:
    CString             m_strUrl;
    LONGLONG            m_llSize;
    LONGLONG            m_llPosition;
    MEMORY_DATA         m_Header;
    GUID                m_subtype;
    volatile BOOL       m_bAbort;
    CCritSec			m_csLock;

};


// 
// 
// class PlayerAsyncStream : public PlayerBaseStream
// {
// public:
//     PlayerAsyncStream() : m_pDelegate (NULL)
//     {
//     }
// 
//     virtual ~PlayerAsyncStream() {
//         SAFE_DELETE(m_pDelegate);
//     }
// 
//     HRESULT SetPlayerStream(PlayerBaseStream* pStream) {
//         CheckPointer(pStream, E_POINTER);
//         m_pDelegate = pStream;
//         return S_OK;
//     }
// 
//     HRESULT SetPointer(LONGLONG llPos) {
//         if (m_pDelegate)
//             return m_pDelegate->SetPointer(llPos);
//         return E_NOTIMPL;
//     }
// 
//     HRESULT Read(PBYTE pbBuffer,
//         DWORD dwBytesToRead,
//         BOOL bAlign,
//         LPDWORD pdwBytesRead) {
//         if (m_pDelegate)
//             return m_pDelegate->Read(pbBuffer, dwBytesToRead, bAlign, pdwBytesRead);
//         return E_NOTIMPL;
//     }
// 
//     LONGLONG Size(LONGLONG *pSizeAvailable = NULL) {
//         if (m_pDelegate)
//             return m_pDelegate->Size(pSizeAvailable);
//         return E_NOTIMPL;
//     }
// 
//     DWORD Alignment() {
//         if (m_pDelegate)
//             return m_pDelegate->Alignment();
//         return E_NOTIMPL;
//     }
// protected:
//     PlayerBaseStream* m_pDelegate;
// };
