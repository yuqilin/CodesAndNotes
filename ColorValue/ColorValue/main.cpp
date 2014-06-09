#include <windows.h>
#include <tchar.h>
#include "resource.h"

int WINAPI DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);


int WINAPI _tWinMain( __in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in_opt LPTSTR lpCmdLine, __in int nShowCmd )
{
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG_COLORVALUE), NULL, (DLGPROC)DlgProc);
    return 0;
}

int WINAPI DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        ::SetFocus(::GetDlgItem(hDlg, IDC_EDIT_COLORVALUE));
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDC_BTN_SHOWCOLOR:
            {
                TCHAR szValue[MAX_PATH] = {0};
                GetDlgItemText(hDlg, IDC_EDIT_COLORVALUE, szValue, MAX_PATH);
                if (_tcslen(szValue) > 0)
                {
                    int color = _tcstol(szValue, NULL, 16);
                    int b = GetRValue(color);
                    int g = GetGValue(color);
                    int r = GetBValue(color);

                    RECT rcColorView;
                    HWND hwndColorView = GetDlgItem(hDlg, IDC_STATIC_COLORVIEW);
                    GetClientRect(hwndColorView, &rcColorView);
                    FillRect(GetDC(hwndColorView), &rcColorView, CreateSolidBrush(RGB(r, g, b)));
                }
            }
            break;
        }
        break;
    case WM_CLOSE:
        EndDialog(hDlg, 0);
        break;
    }
    return 0;
}