#include "stdafx.h"
#include <io.h>
#include <fcntl.h>
#include "StdioFile.h"

CStdioFile::CStdioFile()
{
    m_pStream = NULL;
}

CStdioFile::CStdioFile(FILE* pOpenStream, HRESULT& hr)
{
    ASSERT(pOpenStream != NULL);

    if (pOpenStream == NULL)
    {
        //AfxThrowInvalidArgException();
        hr = E_INVALIDARG;
        return;
    }

    m_pStream = pOpenStream;
    m_hFile = (HANDLE) _get_osfhandle(_fileno(pOpenStream));
    ASSERT(!m_bCloseOnDelete);
}

CStdioFile::CStdioFile(LPCTSTR lpszFileName, UINT nOpenFlags, HRESULT& hr)
{
    ASSERT(lpszFileName != NULL);
    ASSERT(ATL::AtlIsValidString(lpszFileName));

    if (lpszFileName == NULL)
    {
        //AfxThrowInvalidArgException();
        hr = E_INVALIDARG;
        return;
    }

    hr = Open(lpszFileName, nOpenFlags);
}

CStdioFile::~CStdioFile()
{
    if (m_pStream != NULL && m_bCloseOnDelete)
        Close();
}

HRESULT CStdioFile::Open(LPCTSTR lpszFileName, UINT nOpenFlags)
{
    ASSERT(lpszFileName != NULL);
    ASSERT(ATL::AtlIsValidString(lpszFileName));

    if (lpszFileName == NULL)
    {
        return E_INVALIDARG;
    }

    m_pStream = NULL;
    if (FAILED(CFile::Open(lpszFileName, (nOpenFlags & ~typeText))))
    {
        return E_FAIL;
    }

    ASSERT(m_hFile != hFileNull);
    ASSERT(m_bCloseOnDelete);

    char szMode[4]; // C-runtime open string
    int nMode = 0;

    // determine read/write mode depending on CFile mode
    if (nOpenFlags & modeCreate)
    {
        if (nOpenFlags & modeNoTruncate)
            szMode[nMode++] = 'a';
        else
            szMode[nMode++] = 'w';
    }
    else if (nOpenFlags & modeWrite)
        szMode[nMode++] = 'a';
    else
        szMode[nMode++] = 'r';

    // add '+' if necessary (when read/write modes mismatched)
    if (szMode[0] == 'r' && (nOpenFlags & modeReadWrite) ||
        szMode[0] != 'r' && !(nOpenFlags & modeWrite))
    {
        // current szMode mismatched, need to add '+' to fix
        szMode[nMode++] = '+';
    }

    // will be inverted if not necessary
    int nFlags = _O_RDONLY|_O_TEXT;
    if (nOpenFlags & (modeWrite|modeReadWrite))
        nFlags ^= _O_RDONLY;

    if (nOpenFlags & typeBinary)
        szMode[nMode++] = 'b', nFlags ^= _O_TEXT;
    else
        szMode[nMode++] = 't';
    szMode[nMode++] = '\0';

    // open a C-runtime low-level file handle
    int nHandle = _open_osfhandle((UINT_PTR) m_hFile, nFlags);

    // open a C-runtime stream from that handle
    if (nHandle != -1)
        m_pStream = _fdopen(nHandle, szMode);

    if (m_pStream == NULL)
    {
        CFile::Abort(); // close m_hFile
        return E_FAIL;
    }

    return S_OK;
}

HRESULT CStdioFile::Read(void* lpBuf, UINT nCount, DWORD* pdwRead)
{
    ASSERT(m_pStream != NULL);

    if (nCount == 0)
        return S_OK;   // avoid Win32 "null-read"

    ASSERT(ATL::AtlIsValidAddress(lpBuf, nCount));

    if (lpBuf == NULL)
    {
        return E_INVALIDARG;
    }

    UINT nRead = 0;

    if ((nRead = (UINT)fread(lpBuf, sizeof(BYTE), nCount, m_pStream)) == 0 && !feof(m_pStream))
    {
        //AfxThrowFileException(CFileException::genericException, _doserrno, m_strFileName);
        return E_FAIL;
    }
    if (ferror(m_pStream))
    {
        ::clearerr_s(m_pStream);
        //AfxThrowFileException(CFileException::genericException, _doserrno, m_strFileName);
        return E_FAIL;
    }
    if (pdwRead)
        *pdwRead = nRead;
    return S_OK;
}

HRESULT CStdioFile::Write(const void* lpBuf, UINT nCount, DWORD* pdwWritten)
{
    ASSERT(m_pStream != NULL);
    ASSERT(ATL::AtlIsValidAddress(lpBuf, nCount, FALSE));

    if (lpBuf == NULL)
    {
        return E_INVALIDARG;
    }

    if (fwrite(lpBuf, sizeof(BYTE), nCount, m_pStream) != nCount)
    {
        //AfxThrowFileException(CFileException::genericException, _doserrno, m_strFileName);
        return E_FAIL;
    }
    return S_OK;
}

HRESULT CStdioFile::WriteString(LPCTSTR lpsz)
{
    ASSERT(lpsz != NULL);
    ASSERT(m_pStream != NULL);

    if (lpsz == NULL)
    {
        //AfxThrowInvalidArgException();
        return E_INVALIDARG;
    }

    if (_fputts(lpsz, m_pStream) == _TEOF)
    {
        //AfxThrowFileException(CFileException::diskFull, _doserrno, m_strFileName);
        return E_FAIL;
    }
    return S_OK;
}

LPTSTR CStdioFile::ReadString(_Out_z_cap_(nMax) LPTSTR lpsz, _In_ UINT nMax)
{
    ASSERT(lpsz != NULL);
    ASSERT(ATL::AtlIsValidAddress(lpsz, nMax));
    ASSERT(m_pStream != NULL);

    if (lpsz == NULL)
    {
        return NULL;
    }

    LPTSTR lpszResult = _fgetts(lpsz, nMax, m_pStream);
    if (lpszResult == NULL && !feof(m_pStream))
    {
        ::clearerr_s(m_pStream);
        //AfxThrowFileException(CFileException::genericException, _doserrno, m_strFileName);
        return NULL;
    }
    return lpszResult;
}

BOOL CStdioFile::ReadString(CString& rString)
{
    rString = _T("");    // empty string without deallocating
    const int nMaxSize = 128;
    LPTSTR lpsz = rString.GetBuffer(nMaxSize);
    LPTSTR lpszResult;
    int nLen = 0;
    for (;;)
    {
        lpszResult = _fgetts(lpsz, nMaxSize+1, m_pStream);
        rString.ReleaseBuffer();

        // handle error/eof case
        if (lpszResult == NULL && !feof(m_pStream))
        {
            ::clearerr_s(m_pStream);
            //AfxThrowFileException(CFileException::genericException, _doserrno, m_strFileName);
            return FALSE;
        }

        // if string is read completely or EOF
        if (lpszResult == NULL ||
            (nLen = (int)lstrlen(lpsz)) < nMaxSize ||
            lpsz[nLen-1] == '\n')
            break;

        nLen = rString.GetLength();
        lpsz = rString.GetBuffer(nMaxSize + nLen) + nLen;
    }

    // remove '\n' from end of string if present
    lpsz = rString.GetBuffer(0);
    nLen = rString.GetLength();
    if (nLen != 0 && lpsz[nLen-1] == '\n')
        rString.GetBufferSetLength(nLen-1);

    return nLen != 0;
}

HRESULT CStdioFile::Seek(LONGLONG lOff, UINT nFrom, ULONGLONG* pPos)
{
    ASSERT(nFrom == begin || nFrom == end || nFrom == current);
    ASSERT(m_pStream != NULL);

    LONG lOff32;

    if ((lOff < LONG_MIN) || (lOff > LONG_MAX))
    {
        //AfxThrowFileException(CFileException::badSeek, -1, m_strFileName);
        return E_INVALIDARG;
    }

    lOff32 = (LONG)lOff;
    if (fseek(m_pStream, lOff32, nFrom) != 0)
    {
        //AfxThrowFileException(CFileException::badSeek, _doserrno, m_strFileName);
        return E_FAIL;
    }

    long pos = ftell(m_pStream);
    if (pPos)
        *pPos = (ULONGLONG)pos;
    return S_OK;
}

HRESULT CStdioFile::GetLength(ULONGLONG* pLength) const
{
    LONG nCurrent;
    LONG nLength;
    LONG nResult;

    nCurrent = ftell(m_pStream);
    if (nCurrent == -1)
    {
        //AfxThrowFileException(CFileException::invalidFile, _doserrno, m_strFileName);
        return E_FAIL;
    }

    nResult = fseek(m_pStream, 0, SEEK_END);
    if (nResult != 0)
    {
        //AfxThrowFileException(CFileException::badSeek, _doserrno, m_strFileName);
        return E_FAIL;
    }

    nLength = ftell(m_pStream);
    if (nLength == -1)
    {
        //AfxThrowFileException(CFileException::invalidFile, _doserrno, m_strFileName);
        return E_FAIL;
    }
    nResult = fseek(m_pStream, nCurrent, SEEK_SET);
    if (nResult != 0)
    {
        //AfxThrowFileException(CFileException::badSeek, _doserrno, m_strFileName);
        return E_FAIL;
    }
    
    if (pLength)
        *pLength = nLength;

    return S_OK;
}

HRESULT CStdioFile::GetPosition(ULONGLONG* pPos) const
{
    ASSERT(m_pStream != NULL);

    long pos = ftell(m_pStream);
    if (pos == -1)
    {
        //AfxThrowFileException(CFileException::invalidFile, _doserrno, m_strFileName);
        return E_FAIL;
    }
    if (pPos)
    {
        *pPos = pos;
    }
    return S_OK;
}

HRESULT CStdioFile::Flush()
{
    if (m_pStream != NULL && fflush(m_pStream) != 0)
    {
        //AfxThrowFileException(CFileException::diskFull, _doserrno, m_strFileName);
        return E_FAIL;
    }
    return S_OK;
}

void CStdioFile::Close()
{
    ASSERT(m_pStream != NULL);

    int nErr = 0;

    if (m_pStream != NULL)
        nErr = fclose(m_pStream);

    m_hFile = hFileNull;
    m_bCloseOnDelete = FALSE;
    m_pStream = NULL;

    if (nErr != 0)
    {
        //AfxThrowFileException(CFileException::diskFull, _doserrno, m_strFileName);
    }
}

void CStdioFile::Abort()
{
    if (m_pStream != NULL && m_bCloseOnDelete)
        fclose(m_pStream);  // close but ignore errors
    m_hFile = hFileNull;
    m_pStream = NULL;
    m_bCloseOnDelete = FALSE;
}

CFile* CStdioFile::Duplicate() const
{
    ASSERT(m_pStream != NULL);

    //AfxThrowNotSupportedException();
    return NULL;
}

HRESULT CStdioFile::LockRange(ULONGLONG /* dwPos */, ULONGLONG /* dwCount */)
{
    ASSERT(m_pStream != NULL);

    //AfxThrowNotSupportedException();
    return E_FAIL;
}

HRESULT CStdioFile::UnlockRange(ULONGLONG /* dwPos */, ULONGLONG /* dwCount */)
{
    ASSERT(m_pStream != NULL);

    //AfxThrowNotSupportedException();
    return E_FAIL;
}

