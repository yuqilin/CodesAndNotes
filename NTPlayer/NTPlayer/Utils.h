#ifndef __NTPLAYER_UTILS_H__
#define __NTPLAYER_UTILS_H__

#include <string>


#define SAFE_DELETE(p)       { if (p) { delete (p);     (p) = NULL; } }
#define SAFE_DELETE_ARRAY(p) { if (p) { delete [] (p);  (p) = NULL; } }
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p) = NULL; } }


TCHAR*  LoadResourceText(HINSTANCE hInstance, const TCHAR * pcszType, UINT uResourceId);
void    FreeResourceText(TCHAR * pText);

std::wstring    mbs2wcs(int nCodePage, const char* mbs);
std::string	    wcs2mbs(int nCodePage, const wchar_t* wcs);


void CStringToBin(CString str, CAtlArray<BYTE>& data);

GUID GUIDFromCString(CString str);
HRESULT GUIDFromCString(CString str, GUID& guid);
CString CStringFromGUID(const GUID& guid);

BOOL GetModulePath(HMODULE hModule, TCHAR * pszPath);

#endif

