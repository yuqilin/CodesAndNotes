#include "stdafx.h"
#include "DSGraph.h"

HRESULT CDSGraph::EnumSourceFilters(LPCWSTR lpcwstrFileName, CCodecsInfoList& fl)
{
    // TODO: use overrides

    CheckPointer(lpcwstrFileName, E_POINTER);

    fl.RemoveAll();

    CStringW fn = CStringW(lpcwstrFileName).TrimLeft();
    CStringW protocol = fn.Left(fn.Find(':') + 1).TrimRight(':').MakeLower();
    CStringW ext = CPathW(fn).GetExtension().MakeLower();

    HANDLE hFile = INVALID_HANDLE_VALUE;

    if (protocol.GetLength() <= 1 || protocol == L"file") {
        hFile = CreateFile(CString(fn), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);

        if (hFile == INVALID_HANDLE_VALUE) {
            return VFW_E_NOT_FOUND;
        }
    }

    // exceptions first

    //     if (ext == _T(".dvr-ms") || ext == _T(".wtv")) { // doh, this is stupid
    //         fl.Insert(LookupFilterRegistry(CLSID_StreamBufferSource, m_override, MERIT64_PREFERRED), 0);
    //     }

    TCHAR buff[256];
    ULONG len;

    CodecsInfoList& sources = CNTPlayer::m_CodecsManager.m_listSources;

    if (hFile == INVALID_HANDLE_VALUE)
    {
        // internal / protocol

        POSITION pos = sources.GetHeadPosition();
        while (pos)
        {
            CodecsInfo* pInfo = sources.GetNext(pos);
            if (pInfo->protocols.Find(CString(protocol)))
            {
                fl.Insert(pInfo, 0, false, false);
            }
        }
    }
    else
    {
        // internal / check bytes

        POSITION pos = sources.GetHeadPosition();
        while (pos)
        {
            CodecsInfo* pInfo = sources.GetNext(pos);

            if (pInfo == NULL)
                continue;

            POSITION pos2 = pInfo->checkbytes.GetHeadPosition();
            while (pos2)
            {
                if (CheckBytes(hFile, pInfo->checkbytes.GetNext(pos2)))
                {
                    fl.Insert(pInfo, 1, false, false);
                    break;
                }
            }
        }
    }

    if (!ext.IsEmpty())
    {
        // internal / file extension

        POSITION pos = sources.GetHeadPosition();
        while (pos)
        {
            CodecsInfo* pInfo = sources.GetNext(pos);
            if (!pInfo)
                continue;
            if (pInfo->extensions.Find(CString(ext)))
            {
                fl.Insert(pInfo, 2, false, false);
            }
        }
    }

    {
        // internal / the rest

        POSITION pos = sources.GetHeadPosition();
        while (pos)
        {
            CodecsInfo* pInfo = sources.GetNext(pos);
            if (pInfo->protocols.IsEmpty() && pInfo->checkbytes.IsEmpty() && pFGF->m_extensions.IsEmpty())
            {
                fl.Insert(pFGF, 3, false, false);
            }
        }
    }

    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
    }

//     CFGFilter* pFGF = LookupFilterRegistry(CLSID_AsyncReader, m_override);
//     pFGF->AddType(MEDIATYPE_Stream, MEDIASUBTYPE_NULL);
//     fl.Insert(pFGF, 9);

    return S_OK;
}