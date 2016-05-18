// ImmPad.cpp : Defines the entry point for the application.
//

#include "private.h"
#include "ImmPad.h"
#include "TextInputCtrl.h"
//
// MSIME.H can be found in http://msdn2.microsoft.com/en-us/library/ms970233.aspx
//
#include "msime.h"
#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE g_hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

HWND g_hwnd;
CTextInputCtrl *g_pTextInputCtrl;

UINT WM_MSIME_MOUSE = 0;

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

int APIENTRY MyWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_IMMPAD, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	CTextInputCtrl::RegisterClass(hInstance);

    WM_MSIME_MOUSE = RegisterWindowMessage( RWM_MOUSE );

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int) msg.wParam;
}



//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_IMMPAD);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCTSTR)IDC_IMMPAD;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   
   g_hInst = hInstance; // Store instance handle in our global variable

   g_hwnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!g_hwnd)
   {
      return FALSE;
   }

   g_pTextInputCtrl = new CTextInputCtrl();
   if (!g_pTextInputCtrl)
   {
       return FALSE;
   }
   if (!g_pTextInputCtrl->Create(g_hwnd))
   {
       return FALSE;
   }

   ShowWindow(g_hwnd, nCmdShow);
   UpdateWindow(g_hwnd);
   ShowWindow(g_pTextInputCtrl->GetWnd(), SW_SHOW);
   SetFocus(g_pTextInputCtrl->GetWnd());

   return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

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
			    DialogBox(g_hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
			    break;

            case IDM_FONT:
                if (g_pTextInputCtrl->GetWnd())
                    g_pTextInputCtrl->SetFont(hWnd);
			    break;

		    case IDM_EXIT:
			    DestroyWindow(hWnd);
			    break;
		}
		break;
	
    case WM_SETFOCUS:
        if (g_pTextInputCtrl->GetWnd())
            SetFocus(g_pTextInputCtrl->GetWnd());
		break;
		
    case WM_SIZE:
		RECT rc;
		GetClientRect(g_hwnd, &rc);
		if (g_pTextInputCtrl)
		    g_pTextInputCtrl->Move(0, 0, rc.right, rc.bottom);
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}



//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
        case WM_INITDIALOG:
		    return TRUE;

	    case WM_COMMAND:
		    if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		    {
			    EndDialog(hDlg, LOWORD(wParam));
			    return TRUE;
		    }
		    break;
	}
	return FALSE;
}


//+---------------------------------------------------------------------------
//
// ModuleEntry
//
//----------------------------------------------------------------------------


int
WINAPI 
WinMain(
    HINSTANCE ,
    HINSTANCE ,
    LPSTR ,
    int 
    )
{
    STARTUPINFO si;

    si.dwFlags = 0;
    GetStartupInfo(&si);

    return MyWinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);
}

