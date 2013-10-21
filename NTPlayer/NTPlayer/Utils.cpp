#include "stdafx.h"
#include "Utils.h"

TCHAR * LoadResourceText(HINSTANCE hInstance, const TCHAR * pcszType, UINT uResourceId)
{
    TCHAR * pResult = NULL;
    HRSRC hRsrc = ::FindResource(hInstance, MAKEINTRESOURCE(uResourceId), pcszType);
    if(hRsrc != NULL)
    {
        HGLOBAL hGlobal = ::LoadResource(hInstance, hRsrc);
        if(hGlobal != NULL)
        {
            DWORD dwSize = ::SizeofResource(hInstance, hRsrc);
            if(dwSize > 0)
            {
                char * pData = (char *)::LockResource(hGlobal);
                if(pData != NULL)
                {
#ifdef UNICODE
                    int nLength = ::MultiByteToWideChar(CP_ACP, 0, pData, dwSize, NULL, 0);
                    TCHAR * pszBuffer = new TCHAR[nLength + 1];
                    if(pszBuffer != NULL)
                    {
                        int nResult = ::MultiByteToWideChar(CP_ACP, 0, pData, dwSize, pszBuffer, nLength);
                        if(nResult > 0)
                        {
                            pszBuffer[nResult] = '\0';
                            pResult = pszBuffer;
                        }
                        else
                            delete pszBuffer;
                    }
#else
                    TCHAR * pszBuffer = new TCHAR[dwSize + 1];
                    if(pszBuffer != NULL)
                    {
                        lstrcpyn(pszBuffer, pData, dwSize);
                        pszBuffer[dwSize] = '\0';
                        pResult = pszBuffer;
                    }
#endif // UNICODE
                    UnlockResource(hGlobal);
                }
            }
            ::FreeResource(hGlobal);
        }
    }
    return pResult;
}


void FreeResourceText(TCHAR * pText)
{
    if(pText != NULL)
        delete pText;
}


BOOL GetModulePath(HMODULE hModule, TCHAR * pszPath)
{
    if(::GetModuleFileName(hModule, pszPath, MAX_PATH) <= 0)
        return FALSE;

    TCHAR * pEnd = pszPath + lstrlen(pszPath) - 1;
    while(pEnd > pszPath && *pEnd != '\\') pEnd--;
    *pEnd = 0;
    return TRUE;
}


/*----------------------------------------------------------------------
|   mbs2wcs
+---------------------------------------------------------------------*/
std::wstring mbs2wcs(int nCodePage, const char* mbs)
{
    int cchWideChar = MultiByteToWideChar(nCodePage,
        0, 
        mbs, 
        -1, 
        NULL, 
        0);  
    wchar_t *pText = new wchar_t[cchWideChar+1];
    memset(pText, 0, (cchWideChar+1)*sizeof(wchar_t));
    MultiByteToWideChar(nCodePage, 
        0, 
        mbs,
        -1, 
        pText, 
        cchWideChar);

    std::wstring wstrText(pText);
    delete[] pText;
    return wstrText;

}

/*----------------------------------------------------------------------
|   wcs2mbs
+---------------------------------------------------------------------*/
std::string	wcs2mbs(int nCodePage, const wchar_t* wcs)
{
    char* pText;
    int cbMultiByte = WideCharToMultiByte(nCodePage,
        0,
        wcs,
        -1,
        NULL,
        0,
        NULL,
        NULL);
    pText=new char[cbMultiByte+1];
    memset(pText, 0, (cbMultiByte+1)*sizeof(char));
    WideCharToMultiByte(nCodePage,
        0,
        wcs,
        -1,
        pText,
        cbMultiByte,
        NULL,
        NULL);

    std::string strText(pText);
    delete[] pText;
    return strText;
}

void CStringToBin(CString str, CAtlArray<BYTE>& data)
{
    str.Trim();
    ASSERT((str.GetLength() & 1) == 0);
    data.SetCount(str.GetLength() / 2);

    BYTE b = 0;

    str.MakeUpper();
    for (int i = 0, j = str.GetLength(); i < j; i++) {
        TCHAR c = str[i];
        if (c >= _T('0') && c <= _T('9')) {
            if (!(i & 1)) {
                b = ((char(c - _T('0')) << 4) & 0xf0) | (b & 0x0f);
            } else {
                b = (char(c - _T('0')) & 0x0f) | (b & 0xf0);
            }
        } else if (c >= _T('A') && c <= _T('F')) {
            if (!(i & 1)) {
                b = ((char(c - _T('A') + 10) << 4) & 0xf0) | (b & 0x0f);
            } else {
                b = (char(c - _T('A') + 10) & 0x0f) | (b & 0xf0);
            }
        } else {
            break;
        }

        if (i & 1) {
            data[i >> 1] = b;
            b = 0;
        }
    }
}

GUID GUIDFromCString(CString str)
{
    GUID guid = GUID_NULL;
    HRESULT hr = CLSIDFromString(CComBSTR(str), &guid);
    ASSERT(SUCCEEDED(hr));
    UNREFERENCED_PARAMETER(hr);
    return guid;
}

HRESULT GUIDFromCString(CString str, GUID& guid)
{
    guid = GUID_NULL;
    return CLSIDFromString(CComBSTR(str), &guid);
}

CString CStringFromGUID(const GUID& guid)
{
    WCHAR null[128] = {0}, buff[128];
    StringFromGUID2(GUID_NULL, null, 127);
    return CString(StringFromGUID2(guid, buff, 127) > 0 ? buff : null);
}
