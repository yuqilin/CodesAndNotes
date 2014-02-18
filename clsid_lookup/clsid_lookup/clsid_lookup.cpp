
#include "stdafx.h"
#include "mapGUID.h"

//////////////////////////////////////////////////////////////////////////
const int kMinClsidLength = 36;
const int kMaxTextLength = 1024;

//////////////////////////////////////////////////////////////////////////
void TrimString(LPCTSTR pSrc, LPTSTR pDst);
BOOL CenterWindow(HWND hWnd);
int WINAPI DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

//////////////////////////////////////////////////////////////////////////
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, 
                     LPTSTR lpstrCmdLine, int nCmdShow)
{
    
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, DlgProc);
    return 0;
}

//////////////////////////////////////////////////////////////////////////
int WINAPI DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        {
            HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON));
            if (hIcon)
                SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
            CenterWindow(hDlg);
            KnownGuid::Load();
        }
        break;
    case WM_COMMAND:
        {
            WORD wId = LOWORD(wParam);
            WORD wCode = HIWORD(wParam);
            if (wCode == EN_CHANGE)
            {
                static BOOL bIsLookingUp = FALSE;

                if (bIsLookingUp)
                {
                    break;
                }

                if (wId == IDC_EDIT_CLSID)
                {
                    BOOL bResult = FALSE;
                    TCHAR szClsid[kMaxTextLength] = {0};
                    if (GetDlgItemText(hDlg, IDC_EDIT_CLSID, szClsid, kMaxTextLength-1) >= kMinClsidLength)
                    {
                        TCHAR szFullClsid[kMaxTextLength] = {0};
                        if (_tcslen(szClsid) == kMinClsidLength)
                        {
                            _stprintf_s(szFullClsid, kMaxTextLength-1, _T("{%s}"), szClsid);
                        }
                        else if (_tcslen(szClsid) == kMinClsidLength+1)
                        {
                            if (szClsid[0] == '{')
                                _stprintf_s(szFullClsid, kMaxTextLength-1, _T("{%s"), szClsid);
                            else if (szClsid[kMinClsidLength] == '}')
                                _stprintf_s(szFullClsid, kMaxTextLength-1, _T("%s}"), szClsid);
                        }
                        else
                        {
                            TrimString(szClsid, szFullClsid);
                        }
                        GUID guid = GUID_NULL;
                        if (S_OK == CLSIDFromString(szFullClsid, &guid))
                        {
                            LPCTSTR pName = KnownGuid::LookupGUID(guid);
                            if (pName)
                            {
                                TCHAR szName[kMaxTextLength] = {0};
                                _tcscpy_s(szName, pName);

                                bIsLookingUp = TRUE;
                                if (SetDlgItemText(hDlg, IDC_EDIT_FRIENDLYNAME, szName))
                                    bResult = TRUE;
                                bIsLookingUp = FALSE;
                            }
                        }
                    }
                    
                    if (!bResult)
                    {
                        bIsLookingUp = TRUE;
                        SetDlgItemText(hDlg, IDC_EDIT_FRIENDLYNAME, _T(""));
                        bIsLookingUp = FALSE;
                    }
                }
                else if (wId == IDC_EDIT_FRIENDLYNAME)
                {
                    TCHAR szName[kMaxTextLength] = {0};
                    BOOL bResult = FALSE;
                    if (GetDlgItemText(hDlg, IDC_EDIT_FRIENDLYNAME, szName, kMaxTextLength-1) > 0)
                    {
                        TCHAR szFullName[kMaxTextLength] = {0};

                        TrimString(szName, szFullName);

                        TCHAR szGUID[kMaxTextLength] = {0};
                        const GUID* pGUID = KnownGuid::LookupFriendlyName(szFullName);
                        if (pGUID)
                        {
                            if (StringFromGUID2(*pGUID, szGUID, kMaxTextLength-1))
                            {
                                bIsLookingUp = TRUE;
                                if (SetDlgItemText(hDlg, IDC_EDIT_CLSID, szGUID))
                                    bResult = TRUE;
                                bIsLookingUp = FALSE;
                            }
                        }
                    }
                    if (!bResult)
                    {
                        bIsLookingUp = TRUE;
                        SetDlgItemText(hDlg, IDC_EDIT_CLSID, _T(""));
                        bIsLookingUp = FALSE;
                    }
                }
                
            }
        }
        break;
    case WM_CLOSE:
        {
            KnownGuid::Free();
            ::EndDialog(hDlg, 0);
        }
        break;
    }

    return 0;
}

BOOL CenterWindow(HWND hWnd)
{
    HWND hWndCenter = NULL;
    DWORD dwStyle = (DWORD)::GetWindowLong(hWnd, GWL_STYLE);
    if(dwStyle & WS_CHILD)
        hWndCenter = ::GetParent(hWnd);
    else
        hWndCenter = ::GetWindow(hWnd, GW_OWNER);

    // get coordinates of the window relative to its parent
    RECT rcDlg;
    ::GetWindowRect(hWnd, &rcDlg);
    RECT rcArea;
    RECT rcCenter;
    HWND hWndParent;
    if(!(dwStyle & WS_CHILD))
    {
        // don't center against invisible or minimized windows
        if(hWndCenter != NULL)
        {
            DWORD dwStyleCenter = ::GetWindowLong(hWndCenter, GWL_STYLE);
            if(!(dwStyleCenter & WS_VISIBLE) || (dwStyleCenter & WS_MINIMIZE))
                hWndCenter = NULL;
        }

        // center within screen coordinates
#if WINVER < 0x0500
        ::SystemParametersInfo(SPI_GETWORKAREA, NULL, &rcArea, NULL);
#else
        HMONITOR hMonitor = NULL;
        if(hWndCenter != NULL)
        {
            hMonitor = ::MonitorFromWindow(hWndCenter, MONITOR_DEFAULTTONEAREST);
        }
        else
        {
            hMonitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
        }
        //ATLENSURE_RETURN_VAL(hMonitor != NULL, FALSE);

        MONITORINFO minfo;
        minfo.cbSize = sizeof(MONITORINFO);
        BOOL bResult = ::GetMonitorInfo(hMonitor, &minfo);
        //ATLENSURE_RETURN_VAL(bResult, FALSE);

        rcArea = minfo.rcWork;
#endif
        if(hWndCenter == NULL)
            rcCenter = rcArea;
        else
            ::GetWindowRect(hWndCenter, &rcCenter);
    }
    else
    {
        // center within parent client coordinates
        hWndParent = ::GetParent(hWnd);
        //ATLASSERT(::IsWindow(hWndParent));

        ::GetClientRect(hWndParent, &rcArea);
        //ATLASSERT(::IsWindow(hWndCenter));
        ::GetClientRect(hWndCenter, &rcCenter);
        ::MapWindowPoints(hWndCenter, hWndParent, (POINT*)&rcCenter, 2);
    }

    int DlgWidth = rcDlg.right - rcDlg.left;
    int DlgHeight = rcDlg.bottom - rcDlg.top;

    // find dialog's upper left based on rcCenter
    int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
    int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

    // if the dialog is outside the screen, move it inside
    if(xLeft + DlgWidth > rcArea.right)
        xLeft = rcArea.right - DlgWidth;
    if(xLeft < rcArea.left)
        xLeft = rcArea.left;

    if(yTop + DlgHeight > rcArea.bottom)
        yTop = rcArea.bottom - DlgHeight;
    if(yTop < rcArea.top)
        yTop = rcArea.top;

    // map screen coordinates to child coordinates
    return ::SetWindowPos(hWnd, NULL, xLeft, yTop, -1, -1,
        SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void TrimString(LPCTSTR pSrc, LPTSTR pDst)
{
    LPCTSTR pBeg = pSrc;
    LPCTSTR pEnd = pSrc + _tcslen(pSrc) - 1;
    while (*pBeg == ' ' && pBeg < pEnd) ++pBeg;
    while (*pEnd == ' ' && pEnd > pBeg) --pEnd;
    if (pBeg < pEnd)
        memcpy(pDst, pBeg, (pEnd-pBeg+1)*sizeof(TCHAR));
}