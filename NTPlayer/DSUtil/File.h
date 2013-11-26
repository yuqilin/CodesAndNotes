#pragma once

#include <atltime.h>

/////////////////////////////////////////////////////////////////////////////
// File status

struct CFileStatus
{
    CTime m_ctime;          // creation date/time of file
    CTime m_mtime;          // last modification date/time of file
    CTime m_atime;          // last access date/time of file
    ULONGLONG m_size;            // logical size of file in bytes
    BYTE m_attribute;       // logical OR of CFile::Attribute enum values
    BYTE _m_padding;        // pad the structure to a WORD
    TCHAR m_szFullName[_MAX_PATH]; // absolute path name
};

//////////////////////////////////////////////////////////////////////////
// CFile

class CFile
{
public:
    enum OpenFlags {
        modeRead =         (int) 0x00000,
        modeWrite =        (int) 0x00001,
        modeReadWrite =    (int) 0x00002,
        shareCompat =      (int) 0x00000,
        shareExclusive =   (int) 0x00010,
        shareDenyWrite =   (int) 0x00020,
        shareDenyRead =    (int) 0x00030,
        shareDenyNone =    (int) 0x00040,
        modeNoInherit =    (int) 0x00080,
        modeCreate =       (int) 0x01000,
        modeNoTruncate =   (int) 0x02000,
        typeText =         (int) 0x04000, // typeText and typeBinary are
        typeBinary =       (int) 0x08000, // used in derived classes only
        osNoBuffer =       (int) 0x10000,
        osWriteThrough =   (int) 0x20000,
        osRandomAccess =   (int) 0x40000,
        osSequentialScan = (int) 0x80000,
    };

    enum Attribute {
        normal =    0x00,
        readOnly =  0x01,
        hidden =    0x02,
        system =    0x04,
        volume =    0x08,
        directory = 0x10,
        archive =   0x20
    };

    enum SeekPosition { begin = 0x0, current = 0x1, end = 0x2 };

    static const HANDLE hFileNull;

    // Constructors
    CFile();
    CFile(HANDLE hFile);
    CFile(LPCTSTR lpszFileName, UINT nOpenFlags, HRESULT& hr);

    // Attributes
    HANDLE m_hFile;
    inline operator HANDLE() const
    { return m_hFile; }

    virtual HRESULT GetPosition(ULONGLONG* pPos) const;
    HRESULT GetStatus(CFileStatus& rStatus) const;
    virtual CString GetFileName() const;
    virtual CString GetFileTitle() const;
    virtual CString GetFilePath() const;
    virtual void SetFilePath(LPCTSTR lpszNewName);

    // Operations
    virtual HRESULT Open(LPCTSTR lpszFileName, UINT nOpenFlags);
    static HRESULT Rename(LPCTSTR lpszOldName, LPCTSTR lpszNewName);
    static HRESULT Remove(LPCTSTR lpszFileName);
    static HRESULT GetStatus(LPCTSTR lpszFileName, CFileStatus& rStatus);
    static HRESULT SetStatus(LPCTSTR lpszFileName, const CFileStatus& status);

    inline HRESULT CFile::SeekToEnd(ULONGLONG* pPos)
    { return Seek(0, CFile::end, pPos); }

    inline HRESULT SeekToBegin(ULONGLONG* pPos)
    { return Seek(0, CFile::begin, pPos); }

    // Overridables
    virtual CFile* Duplicate() const;

    virtual HRESULT Seek(LONGLONG lOff, UINT nFrom, ULONGLONG* pPos);
    virtual HRESULT SetLength(ULONGLONG dwNewLen);
    virtual HRESULT GetLength(ULONGLONG* pLength) const;

    virtual HRESULT Read(void* lpBuf, UINT nCount, DWORD* pdwRead);
    virtual HRESULT Write(const void* lpBuf, UINT nCount, DWORD* pdwWritten);

    virtual HRESULT LockRange(ULONGLONG dwPos, ULONGLONG dwCount);
    virtual HRESULT UnlockRange(ULONGLONG dwPos, ULONGLONG dwCount);

    virtual void Abort();
    virtual HRESULT Flush();
    virtual void Close();

    // Implementation
public:
    virtual ~CFile();

    enum BufferCommand { bufferRead, bufferWrite, bufferCommit, bufferCheck };
    enum BufferFlags 
    { 
        bufferDirect = 0x01,
        bufferBlocking = 0x02
    };
    virtual UINT GetBufferPtr(UINT nCommand, UINT nCount = 0,
        void** ppBufStart = NULL, void** ppBufMax = NULL);

protected:
    BOOL m_bCloseOnDelete;
    CString m_strFileName;
};
