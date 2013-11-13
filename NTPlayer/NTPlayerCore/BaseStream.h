#pragma once

class BaseStream
{
public:
    BaseStream();
    virtual ~BaseStream();

public:
    virtual HRESULT Open(LPCTSTR pcszUrl, BOOL bRead) = 0;
    virtual HRESULT Close(void) = 0;
    virtual HRESULT GetSize(UINT64* pnSize) = 0;
    virtual HRESULT SetSize(UINT64 nSize) = 0;
    virtual HRESULT GetPosition(UINT64* pnPos) = 0;
    virtual HRESULT SetPosition(UINT64 nPos) = 0;
    virtual HRESULT Read(BYTE* pBuffer, DWORD dwBytesToRead, DWORD* pdwBytesRead) = 0;
    virtual HRESULT Abort(BOOL bStop) = 0;

public:
    void				Lock(void) { m_csLock.Lock(); }
    void				Unlock(void) { m_csLock.Unlock(); }

protected:
    CCritSec			m_csLock;
};
