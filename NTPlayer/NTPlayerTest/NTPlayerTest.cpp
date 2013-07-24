// NTPlayerTest.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "NTPlayerTest.h"

#define MAX_LOADSTRING 100

// Global Variables:
HWND g_hMainWnd;
HWND g_hEditWnd;
HINSTANCE g_hInstance;							// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

void                ShowCodecsInfo();

BOOL                CreateMainWindow();
BOOL                CreateEditWindow();

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
	LoadString(hInstance, IDC_NTPLAYERTEST, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_NTPLAYERTEST));

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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NTPLAYERTEST));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_NTPLAYERTEST);
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

   g_hInstance = hInstance; // Store instance handle in our global variable

   if (!CreateMainWindow())
   {
       return FALSE;
   }

   if (!CreateEditWindow())
   {
       return FALSE;
   }

   ShowWindow(g_hMainWnd, nCmdShow);
   UpdateWindow(g_hMainWnd);

   ntplayer_init();

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

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
        case ID_CODECS_CODECSINFO:
            ShowCodecsInfo();
            break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
        
        {
           
        }
		EndPaint(hWnd, &ps);
		break;
    case WM_SIZE:
        {
            MoveWindow(g_hEditWnd, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
        }
        break;
	case WM_DESTROY:

        ntplayer_uninit();

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

void ShowCodecsInfo()
{
    const char* info = NULL;
    ntplayer_get_codecs_info_string(&info);

    SendMessageA(g_hEditWnd, EM_REPLACESEL, TRUE, (LPARAM)info);
}

BOOL CreateMainWindow()
{
    HWND hWnd = NULL;
    int nShowX = GetSystemMetrics(SM_CXSCREEN);
    int nShowY = GetSystemMetrics(SM_CYSCREEN);
    int nWidth = 600;
    int nHeight = 400;

    hWnd = CreateWindowEx(
        WS_EX_CLIENTEDGE  | WS_EX_ACCEPTFILES,
        szWindowClass, 
        szTitle,
        WS_OVERLAPPEDWINDOW,
        (nShowX-nWidth)/2, (nShowY-nHeight)/2, 600, 400, 
        NULL, 
        NULL, 
        g_hInstance, 
        NULL);

    if (!hWnd)
    {
        return FALSE;
    }

    g_hMainWnd = hWnd;
    return TRUE;
}

BOOL CreateEditWindow()
{
    HWND hWnd = NULL;
    RECT ClientRect;
    GetClientRect(g_hMainWnd, &ClientRect);
    hWnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        _T("EDIT"),		// We are creating an Edit control
        NULL,			// Leave the control empty
        WS_CHILD | WS_HSCROLL | WS_VSCROLL | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY,
        ClientRect.left, ClientRect.top, ClientRect.right-ClientRect.left, ClientRect.bottom-ClientRect.top, 
        g_hMainWnd, NULL, g_hInstance, NULL);
    if (!hWnd)
    {
        return FALSE;
    }
    g_hEditWnd = hWnd;
    return TRUE;
}
