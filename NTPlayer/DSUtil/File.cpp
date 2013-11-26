#include "stdafx.h"
#include <commdlg.h>
#include "File.h"

const HANDLE CFile::hFileNull = INVALID_HANDLE_VALUE;

BOOL _FullPath(LPTSTR lpszPathOut, LPCTSTR lpszFileIn);
UINT _GetFileName(LPCTSTR lpszPathName, _Out_opt_cap_(nMax) LPTSTR lpszTitle, UINT nMax);
UINT _GetFileTitle(LPCTSTR lpszPathName, _Out_cap_(nMax) LPTSTR lpszTitle, UINT nMax);
void _GetRoot(LPCTSTR lpszPath, CString& strRoot);
HRESULT _TimeToFileTime(const CTime& time, LPFILETIME pFileTime);

CFile::CFile()
{
    m_hFile = INVALID_HANDLE_VALUE;
    m_bCloseOnDelete = FALSE;
}

CFile::CFile(HANDLE hFile)
{
    ASSERT(hFile != INVALID_HANDLE_VALUE);
    m_hFile = hFile;
    m_bCloseOnDelete = FALSE;
}

CFile::CFile(LPCTSTR lpszFileName, UINT nOpenFlags, HRESULT& hr)
{
    ASSERT(lpszFileName != NULL);
    m_hFile = INVALID_HANDLE_VALUE;

    if (FAILED(hr = Open(lpszFileName, nOpenFlags)))
    {
        //AfxThrowFileException(e.m_cause, e.m_lOsError, e.m_strFileName);
        //player_log(kLogLevelError, _T("Open failed,%s"), lpszFileName);
    }
}

CFile::~CFile()
{
    if (m_hFile != INVALID_HANDLE_VALUE && m_bCloseOnDelete)
        Close();
}

CFile* CFile::Duplicate() const
{
    ASSERT(m_hFile != INVALID_HANDLE_VALUE);

    CFile* pFile = new CFile();
    HANDLE hFile;
    if (!::DuplicateHandle(::GetCurrentProcess(), m_hFile,
        ::GetCurrentProcess(), &hFile, 0, FALSE, DUPLICATE_SAME_ACCESS))
    {
        delete pFile;
        //CFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
        ////player_log(_T("DuplicateHandle got error, LastError=0x%08x"), ::GetLastError());
        return NULL;
    }
    pFile->m_hFile = hFile;
    ASSERT(pFile->m_hFile != INVALID_HANDLE_VALUE);
    pFile->m_bCloseOnDelete = m_bCloseOnDelete;
    return pFile;
}

HRESULT CFile::Open(LPCTSTR lpszFileName, UINT nOpenFlags)
{
    ASSERT(lpszFileName != NULL);
    ASSERT((nOpenFlags & typeText) == 0);   // text mode not supported

    // shouldn't open an already open file (it will leak)
    ASSERT(m_hFile == INVALID_HANDLE_VALUE);

    // CFile objects are always binary and CreateFile does not need flag
    nOpenFlags &= ~(UINT)typeBinary;

    m_bCloseOnDelete = FALSE;

    m_hFile = INVALID_HANDLE_VALUE;
    m_strFileName.Empty();

    TCHAR szTemp[_MAX_PATH];
    if (lpszFileName != NULL && SUCCEEDED(StringCchLength(lpszFileName, _MAX_PATH, NULL)) )
    {
        if( _FullPath(szTemp, lpszFileName) == FALSE )
            return E_FAIL;
    }
    else
    {
        // user passed in a buffer greater then _MAX_PATH
        return E_FAIL; // path is too long
    }

    m_strFileName = szTemp;
    ASSERT(shareCompat == 0);

    // map read/write mode
    ASSERT((modeRead|modeWrite|modeReadWrite) == 3);
    DWORD dwAccess = 0;
    switch (nOpenFlags & 3)
    {
    case modeRead:
        dwAccess = GENERIC_READ;
        break;
    case modeWrite:
        dwAccess = GENERIC_WRITE;
        break;
    case modeReadWrite:
        dwAccess = GENERIC_READ | GENERIC_WRITE;
        break;
    default:
        ASSERT(FALSE);  // invalid share mode
    }

    // map share mode
    DWORD dwShareMode = 0;
    switch (nOpenFlags & 0x70)    // map compatibility mode to exclusive
    {
    default:
        ASSERT(FALSE);  // invalid share mode?
    case shareCompat:
    case shareExclusive:
        dwShareMode = 0;
        break;
    case shareDenyWrite:
        dwShareMode = FILE_SHARE_READ;
        break;
    case shareDenyRead:
        dwShareMode = FILE_SHARE_WRITE;
        break;
    case shareDenyNone:
        dwShareMode = FILE_SHARE_WRITE | FILE_SHARE_READ;
        break;
    }

    // Note: typeText and typeBinary are used in derived classes only.

    // map modeNoInherit flag
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = (nOpenFlags & modeNoInherit) == 0;

    // map creation flags
    DWORD dwCreateFlag;
    if (nOpenFlags & modeCreate)
    {
        if (nOpenFlags & modeNoTruncate)
            dwCreateFlag = OPEN_ALWAYS;
        else
            dwCreateFlag = CREATE_ALWAYS;
    }
    else
        dwCreateFlag = OPEN_EXISTING;

    // special system-level access flags

    // Random access and sequential scan should be mutually exclusive
    ASSERT((nOpenFlags&(osRandomAccess|osSequentialScan)) != (osRandomAccess|
        osSequentialScan) );

    DWORD dwFlags = FILE_ATTRIBUTE_NORMAL;
    if (nOpenFlags & osNoBuffer)
        dwFlags |= FILE_FLAG_NO_BUFFERING;
    if (nOpenFlags & osWriteThrough)
        dwFlags |= FILE_FLAG_WRITE_THROUGH;
    if (nOpenFlags & osRandomAccess)
        dwFlags |= FILE_FLAG_RANDOM_ACCESS;
    if (nOpenFlags & osSequentialScan)
        dwFlags |= FILE_FLAG_SEQUENTIAL_SCAN;

    // attempt file creation
    HANDLE hFile = ::CreateFile(lpszFileName, dwAccess, dwShareMode, &sa,
        dwCreateFlag, dwFlags, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        //player_log(kLogLevelError, _T("CFile::Open, CreateFile LastError = 0x%08x"), ::GetLastError());
        return E_FAIL;
    }
    m_hFile = hFile;
    m_bCloseOnDelete = TRUE;

    return S_OK;
}

HRESULT CFile::Read(void* lpBuf, UINT nCount, DWORD* pdwRead)
{
    ASSERT(m_hFile != INVALID_HANDLE_VALUE);

    if (nCount == 0)
        return 0;   // avoid Win32 "null-read"

    ASSERT(lpBuf != NULL);
    ASSERT(ATL::AtlIsValidAddress(lpBuf, nCount, TRUE));

    DWORD dwRead;
    if (!::ReadFile(m_hFile, lpBuf, nCount, &dwRead, NULL))
    {
        //player_log(kLogLevelError, _T("CFile::Read, ReadFile LastError = 0x%08x"), ::GetLastError());
        if (pdwRead)
            *pdwRead = 0;
        return E_FAIL;
    }

    if (pdwRead)
        *pdwRead = dwRead;
    return S_OK;
}

HRESULT CFile::Write(const void* lpBuf, UINT nCount, DWORD* pdwWritten)
{
    ASSERT(m_hFile != INVALID_HANDLE_VALUE);

    if (nCount == 0)
        return S_OK;     // avoid Win32 "null-write" option

    ASSERT(lpBuf != NULL);
    ASSERT(ATL::AtlIsValidAddress(lpBuf, nCount, FALSE));

    DWORD nWritten;
    if (!::WriteFile(m_hFile, lpBuf, nCount, &nWritten, NULL))
    {
        //CFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
        //player_log(kLogLevelError, _T("CFile::Write, WriteFile LastError = 0x%08x"), ::GetLastError());
        if (pdwWritten)
            *pdwWritten = nWritten;
        return E_FAIL;
    }

    if (pdwWritten)
        *pdwWritten = nWritten;

    if (nWritten != nCount)
    {
        //AfxThrowFileException(CFileException::diskFull, -1, m_strFileName);
        //player_log(kLogLevelError, _T("CFile::Write, WriteFile diskFull?"));
        return S_FALSE;
    }
    return S_OK;
}

HRESULT CFile::Seek(LONGLONG lOff, UINT nFrom, ULONGLONG* pPos)
{
    ASSERT(m_hFile != INVALID_HANDLE_VALUE);
    ASSERT(nFrom == begin || nFrom == end || nFrom == current);
    ASSERT(begin == FILE_BEGIN && end == FILE_END && current == FILE_CURRENT);

    LARGE_INTEGER liOff;
    liOff.QuadPart = lOff;
    liOff.LowPart = ::SetFilePointer(m_hFile, liOff.LowPart, &liOff.HighPart, (DWORD)nFrom);
    if (liOff.LowPart  == (DWORD)-1)
    {
        if (::GetLastError() != NO_ERROR)
        {
            //CFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
            if (pPos)
                *pPos = INVALID_SET_FILE_POINTER;
            //player_log(kLogLevelError, _T("CFile::Seek, SetFilePointer LastError = 0x%08x"), ::GetLastError());
            return E_FAIL;
        }
    }

    if (pPos)
        *pPos = liOff.QuadPart;
    return S_OK;
}

HRESULT CFile::GetPosition(ULONGLONG* pPos) const
{
    ASSERT(m_hFile != INVALID_HANDLE_VALUE);

    LARGE_INTEGER liPos;
    liPos.QuadPart = 0;
    liPos.LowPart = ::SetFilePointer(m_hFile, liPos.LowPart, &liPos.HighPart , FILE_CURRENT);
    if (liPos.LowPart == (DWORD)-1)
    {
        if (::GetLastError() != NO_ERROR)
        {
            //CFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
            if (pPos)
                *pPos = INVALID_SET_FILE_POINTER;
            //player_log(kLogLevelError, _T("CFile::GetPosition, SetFilePointer LastError = 0x%08x"), ::GetLastError());
            return E_FAIL;
        }
    }
    if (pPos)
        *pPos = liPos.QuadPart;
    return S_OK;
}

HRESULT CFile::Flush()
{
    if (m_hFile == INVALID_HANDLE_VALUE)
        return S_FALSE;

    if (!::FlushFileBuffers(m_hFile))
    {
        //CFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
        //player_log(kLogLevelError, _T("CFile::Flush, FlushFileBuffers LastError = 0x%08x"), ::GetLastError());
        return E_FAIL;
    }
    return S_OK;
}

void CFile::Close()
{
    ASSERT(m_hFile != INVALID_HANDLE_VALUE);
    BOOL bError = FALSE;
    if (m_hFile != INVALID_HANDLE_VALUE)
        bError = !::CloseHandle(m_hFile);

    m_hFile = INVALID_HANDLE_VALUE;
    m_bCloseOnDelete = FALSE;
    m_strFileName.Empty();

    if (bError)
    {
        //CFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
        //player_log(kLogLevelError, _T("CFile::Close, CloseHandle LastError = 0x%08x"), ::GetLastError());
    }
}

void CFile::Abort()
{
    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        // close but ignore errors
        ::CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }
    m_strFileName.Empty();
}

HRESULT CFile::LockRange(ULONGLONG dwPos, ULONGLONG dwCount)
{
    ASSERT(m_hFile != INVALID_HANDLE_VALUE);

    ULARGE_INTEGER liPos;
    ULARGE_INTEGER liCount;

    liPos.QuadPart = dwPos;
    liCount.QuadPart = dwCount;
    if (!::LockFile(m_hFile, liPos.LowPart, liPos.HighPart, liCount.LowPart, 
        liCount.HighPart))
    {
        //CFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
        //player_log(kLogLevelError, _T("CFile::LockRange %d - %d, LockFile LastError = 0x%08x"), dwPos, dwCount, ::GetLastError());
        return E_FAIL;
    }
    return S_OK;
}

HRESULT CFile::UnlockRange(ULONGLONG dwPos, ULONGLONG dwCount)
{
    ASSERT(m_hFile != INVALID_HANDLE_VALUE);

    ULARGE_INTEGER liPos;
    ULARGE_INTEGER liCount;

    liPos.QuadPart = dwPos;
    liCount.QuadPart = dwCount;
    if (!::UnlockFile(m_hFile, liPos.LowPart, liPos.HighPart, liCount.LowPart,
        liCount.HighPart))
    {
        //CFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
        //player_log(kLogLevelError, _T("CFile::UnlockRange %I64u - %I64u, UnlockFile LastError = 0x%08x"), dwPos, dwCount, ::GetLastError());
        return E_FAIL;
    }
    return S_OK;
}

HRESULT CFile::SetLength(ULONGLONG dwNewLen)
{
    ASSERT(m_hFile != INVALID_HANDLE_VALUE);

    ULONGLONG pPos;
    if (FAILED(Seek(dwNewLen, (UINT)begin, &pPos)))
    {
        return E_FAIL;
    }

    if (!::SetEndOfFile(m_hFile))
    {
        //CFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
        //player_log(kLogLevelError, _T("CFile::SetLength %I64u, SetEndOfFile LastError = 0x%08x"), ssdwNewLen, ::GetLastError());
        return E_FAIL;
    }
    return S_OK;
}

HRESULT CFile::GetLength(ULONGLONG* pLength) const
{
    ULARGE_INTEGER liSize;
    liSize.LowPart = ::GetFileSize(m_hFile, &liSize.HighPart);
    if (liSize.LowPart == INVALID_FILE_SIZE)
    {
        if (::GetLastError() != NO_ERROR)
        {
            //CFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
            if (pLength)
                *pLength = INVALID_FILE_SIZE;
            //player_log(kLogLevelError, _T("GetFileSize got error, LastError=0x%08x"), ::GetLastError());
            return E_FAIL;
        }
    }
    if (pLength)
        *pLength = liSize.QuadPart;
    return S_OK;
}

// CFile does not support direct buffering (CMemFile does)
UINT CFile::GetBufferPtr(UINT nCommand, UINT /*nCount*/,
                         void** /*ppBufStart*/, void** /*ppBufMax*/)
{
    ASSERT(nCommand == bufferCheck);
    //UNUSED(nCommand);    // not used in retail build

    return 0;   // no support
}

HRESULT CFile::Rename(LPCTSTR lpszOldName, LPCTSTR lpszNewName)
{
    if (!::MoveFile((LPTSTR)lpszOldName, (LPTSTR)lpszNewName))
    {
        //CFileException::ThrowOsError((LONG)::GetLastError(), lpszOldName);
        return E_FAIL;
    }
    return S_OK;
}

HRESULT CFile::Remove(LPCTSTR lpszFileName)
{
    if (!::DeleteFile((LPTSTR)lpszFileName))
    {
        //CFileException::ThrowOsError((LONG)::GetLastError(), lpszFileName);
        return E_FAIL;
    }
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
CString CFile::GetFileName() const
{
    CFileStatus status;
    GetStatus(status);
    CString strResult;
    _GetFileName(status.m_szFullName, strResult.GetBuffer(_MAX_FNAME),
        _MAX_FNAME);
    strResult.ReleaseBuffer();
    return strResult;
}

CString CFile::GetFileTitle() const
{
    CFileStatus status;
    GetStatus(status);
    CString strResult;
    _GetFileTitle(status.m_szFullName, strResult.GetBuffer(_MAX_FNAME),
        _MAX_FNAME);
    strResult.ReleaseBuffer();
    return strResult;
}

CString CFile::GetFilePath() const
{
    CFileStatus status;
    GetStatus(status);
    return status.m_szFullName;
}

HRESULT CFile::GetStatus(CFileStatus& rStatus) const
{
    memset(&rStatus, 0, sizeof(CFileStatus));

    // copy file name from cached m_strFileName
    Checked::tcsncpy_s(rStatus.m_szFullName, _countof(rStatus.m_szFullName), m_strFileName, _TRUNCATE);

    if (m_hFile != hFileNull)
    {
        // get time current file size
        FILETIME ftCreate, ftAccess, ftModify;
        if (!::GetFileTime(m_hFile, &ftCreate, &ftAccess, &ftModify))
            return E_FAIL;

        LARGE_INTEGER li;

        if (::GetFileSizeEx(m_hFile, &li) == 0)
            return E_FAIL;

        rStatus.m_size = li.QuadPart;

        if (m_strFileName.IsEmpty())
            rStatus.m_attribute = 0;
        else
        {
            DWORD dwAttribute = ::GetFileAttributes(m_strFileName);

            // don't return an error for this because previous versions of MFC didn't
            if (dwAttribute == 0xFFFFFFFF)
                rStatus.m_attribute = 0;
            else
            {
                rStatus.m_attribute = (BYTE) dwAttribute;
            }
        }

        // convert times as appropriate
        // some file systems may not record file creation time, file access time etc
        if (CTime::IsValidFILETIME(ftCreate))
        {
            rStatus.m_ctime = CTime(ftCreate);
        }
        else
        {
            rStatus.m_ctime = CTime();
        }

        if (CTime::IsValidFILETIME(ftAccess))
        {
            rStatus.m_atime = CTime(ftAccess);
        }
        else
        {
            rStatus.m_atime = CTime();
        }

        if (CTime::IsValidFILETIME(ftModify))
        {
            rStatus.m_mtime = CTime(ftModify);
        }
        else
        {
            rStatus.m_mtime = CTime();
        }

        if (rStatus.m_ctime.GetTime() == 0)
            rStatus.m_ctime = rStatus.m_mtime;

        if (rStatus.m_atime.GetTime() == 0)
            rStatus.m_atime = rStatus.m_mtime;
    }
    return S_OK;
}

HRESULT CFile::GetStatus(LPCTSTR lpszFileName, CFileStatus& rStatus)
{
    ASSERT( lpszFileName != NULL );

    if ( lpszFileName == NULL ) 
    {
        return E_FAIL;
    }

    if ( lstrlen(lpszFileName) >= _MAX_PATH )
    {
        return E_FAIL;
    }

    // attempt to fully qualify path first
    if (!_FullPath(rStatus.m_szFullName, lpszFileName))
    {
        rStatus.m_szFullName[0] = '\0';
        return E_FAIL;
    }

    WIN32_FILE_ATTRIBUTE_DATA fileAttributeData;
    if (!GetFileAttributesEx(lpszFileName, GetFileExInfoStandard, &fileAttributeData))
        return E_FAIL;

    // strip attribute of NORMAL bit, our API doesn't have a "normal" bit.
    rStatus.m_attribute = (BYTE)
        (fileAttributeData.dwFileAttributes & ~FILE_ATTRIBUTE_NORMAL);

    rStatus.m_size = fileAttributeData.nFileSizeHigh;
    rStatus.m_size <<= 32;
    rStatus.m_size |= fileAttributeData.nFileSizeLow;

    // convert times as appropriate
    if (CTime::IsValidFILETIME(fileAttributeData.ftCreationTime))
    {
        rStatus.m_ctime = CTime(fileAttributeData.ftCreationTime);
    }
    else
    {
        rStatus.m_ctime = CTime();
    }

    if (CTime::IsValidFILETIME(fileAttributeData.ftLastAccessTime))
    {
        rStatus.m_atime = CTime(fileAttributeData.ftLastAccessTime);
    }
    else
    {
        rStatus.m_atime = CTime();
    }

    if (CTime::IsValidFILETIME(fileAttributeData.ftLastWriteTime))
    {
        rStatus.m_mtime = CTime(fileAttributeData.ftLastWriteTime);
    }
    else
    {
        rStatus.m_mtime = CTime();
    }

    if (rStatus.m_ctime.GetTime() == 0)
        rStatus.m_ctime = rStatus.m_mtime;

    if (rStatus.m_atime.GetTime() == 0)
        rStatus.m_atime = rStatus.m_mtime;

    return S_OK;
}

HRESULT CFile::SetStatus(LPCTSTR lpszFileName, const CFileStatus& status)
{
    DWORD wAttr;
    FILETIME creationTime;
    FILETIME lastAccessTime;
    FILETIME lastWriteTime;
    LPFILETIME lpCreationTime = NULL;
    LPFILETIME lpLastAccessTime = NULL;
    LPFILETIME lpLastWriteTime = NULL;

    if ((wAttr = GetFileAttributes((LPTSTR)lpszFileName)) == (DWORD)-1L)
    {
        //CFileException::ThrowOsError((LONG)GetLastError(), lpszFileName);
        return E_FAIL;
    }

    if ((DWORD)status.m_attribute != wAttr && (wAttr & readOnly))
    {
        // Set file attribute, only if currently readonly.
        // This way we will be able to modify the time assuming the
        // caller changed the file from readonly.

        if (!SetFileAttributes((LPTSTR)lpszFileName, (DWORD)status.m_attribute))
        {
            //CFileException::ThrowOsError((LONG)GetLastError(), lpszFileName);
            return E_FAIL;
        }
    }

    // last modification time
    if (status.m_mtime.GetTime() != 0)
    {
        _TimeToFileTime(status.m_mtime, &lastWriteTime);
        lpLastWriteTime = &lastWriteTime;

        // last access time
        if (status.m_atime.GetTime() != 0)
        {
            _TimeToFileTime(status.m_atime, &lastAccessTime);
            lpLastAccessTime = &lastAccessTime;
        }

        // create time
        if (status.m_ctime.GetTime() != 0)
        {
            _TimeToFileTime(status.m_ctime, &creationTime);
            lpCreationTime = &creationTime;
        }

        HANDLE hFile = ::CreateFile(lpszFileName, GENERIC_READ|GENERIC_WRITE,
            FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
            NULL);

        if (hFile == INVALID_HANDLE_VALUE)
        {
            //CFileException::ThrowOsError((LONG)::GetLastError(), lpszFileName);
            return E_FAIL;
        }

        if (!SetFileTime((HANDLE)hFile, lpCreationTime, lpLastAccessTime, lpLastWriteTime))
        {
            LONG sc = (LONG)::GetLastError();
            ::CloseHandle(hFile);
            //CFileException::ThrowOsError(sc, lpszFileName);
            return E_FAIL;
        }

        if (!::CloseHandle(hFile))
        {
            //CFileException::ThrowOsError((LONG)::GetLastError(), lpszFileName);
            return E_FAIL;
        }
    }

    if ((DWORD)status.m_attribute != wAttr && !(wAttr & readOnly))
    {
        if (!SetFileAttributes((LPTSTR)lpszFileName, (DWORD)status.m_attribute))
        {
            //CFileException::ThrowOsError((LONG)GetLastError(), lpszFileName);
            return E_FAIL;
        }
    }
    return S_OK;
}

//////////////////////////////////////////////////////////////////////////
UINT _GetFileName(LPCTSTR lpszPathName, _Out_opt_cap_(nMax) LPTSTR lpszTitle, UINT nMax)
{
    ASSERT(lpszTitle == NULL ||
        ATL::AtlIsValidAddress(lpszTitle, nMax, TRUE));
    ASSERT(ATL::AtlIsValidString(lpszPathName));

    // always capture the complete file name including extension (if present)
    LPTSTR lpszTemp = ::PathFindFileName(lpszPathName);

    // lpszTitle can be NULL which just returns the number of bytes
    if (lpszTitle == NULL)
        return lstrlen(lpszTemp)+1;

    // otherwise copy it into the buffer provided
    Checked::tcsncpy_s(lpszTitle, nMax, lpszTemp, _TRUNCATE);
    return 0;
}

// turn a file, relative path or other into an absolute path
BOOL _FullPath(LPTSTR lpszPathOut, LPCTSTR lpszFileIn)
// lpszPathOut = buffer of _MAX_PATH
// lpszFileIn = file, relative path or absolute path
// (both in ANSI character set)
{
    ASSERT(ATL::AtlIsValidAddress(lpszPathOut, _MAX_PATH, TRUE));

    // first, fully qualify the path name
    LPTSTR lpszFilePart;
    DWORD dwRet = GetFullPathName(lpszFileIn, _MAX_PATH, lpszPathOut, &lpszFilePart);
    if (dwRet == 0)
    {
        if (lpszFileIn != NULL && lpszFileIn[0] != '\0')
        {
            //player_log(kLogLevelWarn, _T("Warning: could not parse the path '%s'.\n"), lpszFileIn);
        }
        Checked::tcsncpy_s(lpszPathOut, _MAX_PATH, lpszFileIn, _TRUNCATE); // take it literally
        return FALSE;
    }
    else if (dwRet >= _MAX_PATH)
    {
        // GetFullPathName() returned a path greater than _MAX_PATH
        if (lpszFileIn[0] != '\0')
        {
            //player_log(kLogLevelWarn, "Warning: could not parse the path '%s'. Path is too long.\n", lpszFileIn);
        }
        return FALSE; // long path won't fit in buffer
    }

    CString strRoot;
    // determine the root name of the volume
    _GetRoot(lpszPathOut, strRoot);

    if (!::PathIsUNC( strRoot ))
    {
        // get file system information for the volume
        DWORD dwFlags, dwDummy;
        if (!GetVolumeInformation(strRoot, NULL, 0, NULL, &dwDummy, &dwFlags,
            NULL, 0))
        {
            //player_log(kLogLevelWarn, _T("Warning: could not get volume information '%s'.\n"), (LPCTSTR)strRoot);
            return FALSE;   // preserving case may not be correct
        }

        // not all characters have complete uppercase/lowercase
        if (!(dwFlags & FS_CASE_IS_PRESERVED))
            CharUpper(lpszPathOut);

        // assume non-UNICODE file systems, use OEM character set
        if (!(dwFlags & FS_UNICODE_STORED_ON_DISK))
        {
            WIN32_FIND_DATA data;
            HANDLE h = FindFirstFile(lpszFileIn, &data);
            if (h != INVALID_HANDLE_VALUE)
            {
                FindClose(h);
                if(lpszFilePart != NULL && lpszFilePart > lpszPathOut)
                {
                    int nFileNameLen = lstrlen(data.cFileName);
                    int nIndexOfPart = (int)(lpszFilePart - lpszPathOut);
                    if ((nFileNameLen + nIndexOfPart) < _MAX_PATH)
                    {
                        Checked::tcscpy_s(lpszFilePart, _MAX_PATH - nIndexOfPart, data.cFileName);
                    }
                    else
                    {
                        // the path+filename of the file is too long
                        return FALSE; // Path doesn't fit in the buffer.
                    }
                }
                else
                {
                    return FALSE;
                }
            }
        }
    }

    return TRUE;
}

void _GetRoot(LPCTSTR lpszPath, CString& strRoot)
{
    ASSERT(lpszPath != NULL);

    LPTSTR lpszRoot = strRoot.GetBuffer(_MAX_PATH);
    memset(lpszRoot, 0, _MAX_PATH);
    Checked::tcsncpy_s(lpszRoot, _MAX_PATH, lpszPath, _TRUNCATE);
    PathStripToRoot(lpszRoot);
    strRoot.ReleaseBuffer();
}

HRESULT _TimeToFileTime(const CTime& time, LPFILETIME pFileTime)
{
    ASSERT(pFileTime != NULL);

    if (pFileTime == NULL) 
    {
        //AfxThrowInvalidArgException();
        return E_INVALIDARG;
    }

    SYSTEMTIME sysTime;
    sysTime.wYear = (WORD)time.GetYear();
    sysTime.wMonth = (WORD)time.GetMonth();
    sysTime.wDay = (WORD)time.GetDay();
    sysTime.wHour = (WORD)time.GetHour();
    sysTime.wMinute = (WORD)time.GetMinute();
    sysTime.wSecond = (WORD)time.GetSecond();
    sysTime.wMilliseconds = 0;

    // convert system time to local file time
    FILETIME localTime;
    if (!SystemTimeToFileTime((LPSYSTEMTIME)&sysTime, &localTime))
    {
        //CFileException::ThrowOsError((LONG)::GetLastError());
        return E_FAIL;
    }

    // convert local file time to UTC file time
    if (!LocalFileTimeToFileTime(&localTime, pFileTime))
    {
        //CFileException::ThrowOsError((LONG)::GetLastError());
        return E_FAIL;
    }

    return S_OK;
}

UINT _GetFileTitle(LPCTSTR lpszPathName, _Out_cap_(nMax) LPTSTR lpszTitle, UINT nMax)
{
    ASSERT(lpszTitle == NULL ||
        ATL::AtlIsValidAddress(lpszTitle, _MAX_FNAME));
    ASSERT(ATL::AtlIsValidString(lpszPathName));

    // use a temporary to avoid bugs in ::GetFileTitle when lpszTitle is NULL
    TCHAR szTemp[_MAX_PATH];
    LPTSTR lpszTemp = lpszTitle;
    if (lpszTemp == NULL)
    {
        lpszTemp = szTemp;
        nMax = _countof(szTemp);
    }
    if (::GetFileTitle(lpszPathName, lpszTemp, (WORD)nMax) != 0)
    {
        // when ::GetFileTitle fails, use cheap imitation
        return _GetFileName(lpszPathName, lpszTitle, nMax);
    }
    return lpszTitle == NULL ? lstrlen(lpszTemp)+1 : 0;
}
