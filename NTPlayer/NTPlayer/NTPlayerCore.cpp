#include "stdafx.h"
#include <atlpath.h>
#include "text.h"
#include "NTPlayerCore.h"

CCodecsManager CNTPlayerCore::m_CodecsManager;

CNTPlayerCore::CNTPlayerCore()
{

}

CNTPlayerCore::~CNTPlayerCore()
{
    m_CodecsManager.UnLoadCodecsInfo();
}

HRESULT CNTPlayerCore::LoadCodecsInfo()
{
    m_CodecsManager.LoadCodecsInfo();

    return S_OK;
}

HRESULT CNTPlayerCore::LoadPlayerSettings()
{
    return S_OK;
}

HRESULT CNTPlayerCore::GetCodecsInfoString(const char** info)
{
    if (info)
    {
        *info = m_CodecsManager.GetCodecsInfoString();
    }
    return S_OK;
}

HRESULT CNTPlayerCore::OpenMedia(const char* url)
{
    HRESULT hr = E_FAIL;

    CStringA strUrl(url);
    m_strUrl = strUrl;

    //HRESULT hr = Kernel_Render();
//     CFGManager* fgmgr = new CFGManager;
// 
//     HRESULT hr = fgmgr->RenderFile(m_strUrl, NULL);

    m_pGB = new CFGManagerPlayer(_T("CFGManagerPlayer"), NULL, m_hVideoWindow);

    if (m_pGB)
        hr = m_pGB->RenderFile(m_strUrl, NULL);

    m_pMC = m_pGB;
    m_pME = m_pGB;
    m_pMS = m_pGB; // general
    m_pVW = m_pGB;
    m_pBV = m_pGB; // video
    m_pBA = m_pGB; // audio
    m_pFS = m_pGB;


    return hr;
}

HRESULT CNTPlayerCore::CloseMedia()
{

    return S_OK;
}

HRESULT CNTPlayerCore::Play()
{

    m_pMS->SetRate(m_dSpeedRate);
    m_pMC->Run();

    return S_OK;
}

HRESULT CNTPlayerCore::Pause()
{

    return S_OK;
}

HRESULT CNTPlayerCore::Stop()
{

    return S_OK;
}


HRESULT CNTPlayerCore::Kernel_Render()
{
    // EnumSourceFilters

    MatchingItemList matching_list;
    EnumSourceFilters(m_strUrl, matching_list);


    // RenderSourceFilter

    return S_OK;
}

HRESULT CNTPlayerCore::EnumSourceFilters(LPCTSTR file_name, MatchingItemList& matching_list)
{
    HRESULT hr = S_OK;

    matching_list.RemoveAll();

    CString fn = CString(file_name).TrimLeft();
    CString protocol = fn.Left(fn.Find(':') + 1).TrimRight(':').MakeLower();
    CString ext = CPath(fn).GetExtension().MakeLower();

    HANDLE hFile = INVALID_HANDLE_VALUE;

    if (protocol.GetLength() <= 1 || protocol == L"file")
    {
        hFile = CreateFile(CString(fn), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile == INVALID_HANDLE_VALUE)
        {
            return VFW_E_NOT_FOUND;
        }
    }

    CodecsInfo* info = NULL;
    if (hFile == INVALID_HANDLE_VALUE)
    {
        // protocol
        POSITION pos = m_CodecsManager.m_listSources.GetHeadPosition();
        while (pos)
        {
            info = m_CodecsManager.m_listSources.GetNext(pos);
            if (info->protocols.Find(protocol))
            {
                MatchingItem item;
                item.info = info;
                matching_list.AddTail(item);
            }
        }
    }
    else
    {
        // check bytes
        POSITION pos = m_CodecsManager.m_listSources.GetHeadPosition();
        while (pos)
        {
            info = m_CodecsManager.m_listSources.GetNext(pos);
            if (info)
            {
                POSITION pos2 = info->checkbytes.GetHeadPosition();
                while (pos2)
                {
                    const CheckByteItem& checkbyte_item = info->checkbytes.GetNext(pos2);
                    if (CheckBytes(hFile, checkbyte_item.checkbyte))
                    {
                        MatchingItem matching_item;
                        matching_item.info = info;
                        matching_item.subtype = checkbyte_item.subtype;
                        matching_list.AddTail(matching_item);
                        break;
                    }
                }
            }
        }
    }

    return hr;
}

BOOL CNTPlayerCore::CheckProtocol(CString protocol, const CodecsInfo* info)
{
    if (info != NULL && info->protocols.Find(protocol))
    {
        return TRUE;
    }
    return FALSE;
}

BOOL CNTPlayerCore::CheckBytes(HANDLE hFile, const CString& chkbytes)
{
    CAtlList<CString> sl;
    Explode(chkbytes, sl, ',');

    if (sl.GetCount() < 4)
    {
        return FALSE;
    }

    ASSERT(!(sl.GetCount() & 3));

    LARGE_INTEGER size = {0, 0};
    GetFileSizeEx(hFile, &size);

    while (sl.GetCount() >= 4) {
        CString offsetstr = sl.RemoveHead();
        CString cbstr = sl.RemoveHead();
        CString maskstr = sl.RemoveHead();
        CString valstr = sl.RemoveHead();

        long cb = _ttol(cbstr);

        if (offsetstr.IsEmpty() || cbstr.IsEmpty()
            || valstr.IsEmpty() || (valstr.GetLength() & 1)
            || cb * 2 != valstr.GetLength()) {
                return FALSE;
        }

        LARGE_INTEGER offset;
        offset.QuadPart = _ttoi64(offsetstr);
        if (offset.QuadPart < 0) {
            offset.QuadPart = size.QuadPart - offset.QuadPart;
        }
        SetFilePointerEx(hFile, offset, &offset, FILE_BEGIN);

        // LAME
        while (maskstr.GetLength() < valstr.GetLength()) {
            maskstr += 'F';
        }

        CAtlArray<BYTE> mask, val;
        CStringToBin(maskstr, mask);
        CStringToBin(valstr, val);

        for (size_t i = 0; i < val.GetCount(); i++) {
            BYTE b;
            DWORD r;
            if (!ReadFile(hFile, &b, 1, &r, NULL) || (b & mask[i]) != val[i]) {
                return FALSE;
            }
        }
    }

    return sl.IsEmpty();
}

void CNTPlayerCore::PlayThread()
{
    // play thread loop
    while true
        if play thread is running
            break
        else
            wait opeartion

            switch opearation



    while (true)
    {
        if (!m_bRunning)
            break;

        CAMMsgEvent evt;
        evt.Wait();
    }
}

void CNTPlayerCore::PostOperation()
{

}