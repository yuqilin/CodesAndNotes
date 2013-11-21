#include "stdafx.h"
#include <shellapi.h>
#include "ViewWindow.h"

ViewWindow::ViewWindow()
{

}

ViewWindow::~ViewWindow()
{
    DestroyWindow();
}

LRESULT ViewWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    //ModifyStyle(WS_OVERLAPPEDWINDOW, WS_POPUP);
    //ModifyStyleEx(0, WS_EX_CLIENTEDGE | WS_EX_ACCEPTFILES);
    //ResizeClient(600, 400);
    //CenterWindow();
    return 0;
}

LRESULT ViewWindow::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    DestroyWindow();
    return 0;
}

LRESULT ViewWindow::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    PostQuitMessage(0);
    return 0;
}

LRESULT ViewWindow::OnDropFiles(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    TCHAR szFile[MAX_PATH];
    UINT uNumberOfDrags = DragQueryFile((HDROP)wParam, 0xFFFFFFFF, szFile, MAX_PATH);
    for (int i = 0; i < uNumberOfDrags; i++)
    {
        if (DragQueryFile((HDROP)wParam, i, szFile, MAX_PATH))
        {
            player_log(kLogLevelTrace, _T("File[%d] : %s\n"), i, szFile);
        }
    }
    return 0;
}

LRESULT ViewWindow::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
//     static int count = 0;
//     player_log(kLogLevelTrace, _T("ViewWindow::OnPaint, %d\n"), count++);
    PAINTSTRUCT ps;
    HDC hDC = BeginPaint(&ps);


    EndPaint(&ps);
    //bHandled = FALSE;
    return 0;
}

LRESULT ViewWindow::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
//     static int count = 0;
//     player_log(kLogLevelTrace, _T("ViewWindow::OnEraseBkgnd, %d\n"), count++);

    //     HDC hDC = (HDC)wParam;
    // 
    //     RECT rcClient;
    //     GetClientRect(&rcClient);
    //     FillRect(hDC, &rcClient, (HBRUSH)GetStockObject(GRAY_BRUSH));

    return 0;
}

LRESULT ViewWindow::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    WORD wWidth = LOWORD(lParam);
    WORD wHeight = LOWORD(lParam);

    player_log(kLogLevelTrace, _T("ViewWindow::OnSize, width = %d, height = %d\n"), wWidth, wHeight);

    return 0;
}

LRESULT ViewWindow::OnSizing(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return 0;
}