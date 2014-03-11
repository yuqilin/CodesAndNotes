// test.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "test.h"
#include "ximage.h"
#include "ColorIt.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

void LoadImage(HWND hWnd);
void DrawImage(HDC hdc, const RECT& rect);
int GetPixelColor(int x, int y);
void DrawPixelColor(int x, int y, int color);

CxImage g_Image;

COLORREF g_NewColor = RGB(255, 0, 0);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_TEST, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TEST));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TEST));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_TEST);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW |WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
    //TCHAR szMsg[MAX_PATH];

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
        {
            hdc = BeginPaint(hWnd, &ps);
		    // TODO: Add any drawing code here...
            RECT rcClient;
            GetClientRect(hWnd, &rcClient);
            DrawImage(hdc, rcClient);
		    EndPaint(hWnd, &ps);
        }
		break;
    case WM_ERASEBKGND:
        break;

    case WM_LBUTTONDOWN:
        {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            RECT rcClient;
            GetClientRect(hWnd, &rcClient);
            int calc_x = x * g_Image.GetWidth() / (rcClient.right - rcClient.left);
            int calc_y = g_Image.GetHeight() - y * g_Image.GetHeight() / (rcClient.bottom - rcClient.top);

            //int old_color = g_Image.GetPixelColor(calc_x, calc_y);
            int new_color = RGB(0xff, 0, 0);
            int boundary_color = RGB(0, 0, 0);

            SetImageInfo(&g_Image, g_Image.GetWidth(), g_Image.GetHeight(),
                &GetPixelColor, &DrawPixelColor);
            ScanLineSeedFill(calc_y, calc_x, new_color, boundary_color);

            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;
    case WM_CREATE:
        {
            LoadImage(hWnd);
        }
        break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void LoadImage(HWND hWnd)
{
    HRSRC hRsrc = FindResource(NULL, MAKEINTRESOURCE(IDB_SIMPSON001), _T("JPG"));
    if (g_Image.LoadResource(hRsrc, CXIMAGE_FORMAT_JPG))
    {
        RECT rcClient;
        GetClientRect(hWnd, &rcClient);
        
        RECT rec1;
        GetWindowRect(hWnd,&rec1);          //取得窗口的大小;
        RECT rec2;
        GetClientRect(hWnd,&rec2);          //取得窗口客户区大小;
        int newWidth = g_Image.GetWidth() + (rec1.right-rec1.left)-(rec2.right-rec2.left);      //新窗口的宽度;
        int newHeight = g_Image.GetHeight() + (rec1.bottom-rec1.top)-(rec2.bottom-rec2.top);    //新窗口的高度;

        SetWindowPos(hWnd, NULL, 0, 0, newWidth, newHeight, 0);
        //SetWindowPos(hwnd, NULL, 0, 0, g_Image.GetWidth(), g_Image.GetHeight(), 0);
    }
}

void DrawImage(HDC hdc, const RECT& rect)
{
    g_Image.Draw(hdc, rect);
}

int GetPixelColor(int x, int y) {
    RGBQUAD clr = g_Image.GetPixelColor(y, x);
    int color = RGB(clr.rgbRed, clr.rgbGreen, clr.rgbBlue);
    return color;
}

void DrawPixelColor(int x, int y, int color) {
    g_Image.SetPixelColor(y, x, (COLORREF)color);
}