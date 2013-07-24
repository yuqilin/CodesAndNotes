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
