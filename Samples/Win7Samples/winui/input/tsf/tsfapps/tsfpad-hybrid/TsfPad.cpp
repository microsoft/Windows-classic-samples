// TsfPad.cpp : Defines the entry point for the application.
//

#include "private.h"
#include "TsfPad.h"
#include "TextInputCtrl.h"
//
// MSIME.H can be found in http://msdn2.microsoft.com/en-us/library/ms970233.aspx
//
#include "msime.h"
#include "DisplayAttribute.h"
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

ITfThreadMgr *g_pThreadMgr = NULL;
TfClientId g_TfClientId = TF_CLIENTID_NULL;

ITfKeystrokeMgr *g_pKeystrokeMgr = NULL;

#ifdef USE_MESSAGEPUMP
ITfMessagePump *g_pMessagePump = NULL;
#endif

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
	MSG msg = {0};

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    if (FAILED(CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER, 
                                IID_ITfThreadMgr, (void**)&g_pThreadMgr)))
    {
        goto Exit;
    }

    if (FAILED(g_pThreadMgr->Activate(&g_TfClientId)))
    {
        goto Exit;
    }

    if (g_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&g_pKeystrokeMgr) != S_OK)
    {
        goto Exit;
    }

#ifdef USE_MESSAGEPUMP
    if (g_pThreadMgr->QueryInterface(IID_ITfMessagePump, (void **)&g_pMessagePump) != S_OK)
    {
        goto Exit;
    }
#endif

    InitDisplayAttrbute();
    
	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_TSFPAD, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	CTextInputCtrl::RegisterClass(hInstance);


    WM_MSIME_MOUSE = RegisterWindowMessage( RWM_MOUSE );


	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		goto Exit;
	}

#ifdef USE_MESSAGEPUMP
    //
    // This is a sameple to use ITfMessagePump inenterface.
    // Using ITfMessagePump, the application can optimize the timing to let TextService to process key event.
    //
    while(TRUE)
    {
        BOOL fEaten;
        int fResult;
        //
        // TextServiceFramework does not ask TextService to process this key event.
        //
        if (g_pMessagePump->GetMessage(&msg, NULL, 0, 0, &fResult) != S_OK)
        {
            fResult = -1;
        }
        else if (msg.message == WM_KEYDOWN)
        {
            // does an ime want it?
            if (g_pKeystrokeMgr->TestKeyDown(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten &&
                g_pKeystrokeMgr->KeyDown(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten)
            {
                continue;
            }
        }
        else if (msg.message == WM_KEYUP)
        {
        // does an ime want it?
            if (g_pKeystrokeMgr->TestKeyUp(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten &&
                g_pKeystrokeMgr->KeyUp(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten)
            {
                continue;
            }
        }

        if (fResult <= 0)
            break;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
    }
#else
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
 		TranslateMessage(&msg);
	 	DispatchMessage(&msg);
	}
#endif

    g_pThreadMgr->Deactivate();

Exit:
    UninitDisplayAttrbute();

#ifdef USE_MESSAGEPUMP
    if (g_pMessagePump)
    {
        g_pMessagePump->Release();
    }
#endif

    if (g_pThreadMgr)
    {
        g_pThreadMgr->Release();
    }
    CoUninitialize();

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
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_TSFPAD);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCTSTR)IDC_TSFPAD;
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

