#pragma once

#include "File.h"

class CStdioFile : public CFile
{
public:
    // Constructors
    CStdioFile();
    CStdioFile(FILE* pOpenStream, HRESULT& hr);
    CStdioFile(LPCTSTR lpszFileName, UINT nOpenFlags, HRESULT& hr);

    // Attributes
    FILE* m_pStream;    // stdio FILE
    // m_hFile from base class is _fileno(m_pStream)

    // Operations
    // reading and writing strings
    virtual HRESULT WriteString(LPCTSTR lpsz);
    virtual LPTSTR ReadString(_Out_z_cap_(nMax) LPTSTR lpsz, _In_ UINT nMax);
    virtual BOOL ReadString(CString& rString);

    // Implementation
public:
    virtual ~CStdioFile();

    virtual HRESULT GetPosition(ULONGLONG* pPos) const;
    virtual HRESULT GetLength(ULONGLONG* pLength) const;
    virtual HRESULT Open(LPCTSTR lpszFileName, UINT nOpenFlags);
    virtual HRESULT Read(void* lpBuf, UINT nCount, DWORD* pdwRead);
    virtual HRESULT Write(const void* lpBuf, UINT nCount, DWORD* pdwWritten);
    virtual HRESULT Seek(LONGLONG lOff, UINT nFrom, ULONGLONG* pPos);
    virtual void Abort();
    virtual HRESULT Flush();
    virtual void Close();

    // Unsupported APIs
    virtual CFile* Duplicate() const;
    virtual HRESULT LockRange(ULONGLONG dwPos, ULONGLONG dwCount);
    virtual HRESULT UnlockRange(ULONGLONG dwPos, ULONGLONG dwCount);
};