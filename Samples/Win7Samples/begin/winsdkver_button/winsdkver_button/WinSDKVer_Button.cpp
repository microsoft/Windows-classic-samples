// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#include "stdafx.h"
#include "WinSDKVer_Button.h"

#define MAX_LOADSTRING 100

// Split button
const int IDC_BTN_BUTTONA = 201;
const int IDC_BTN_BUTTONB = 202;

// Global Variables:
HINSTANCE hInst;								// Current instance
HWND hWnd, hWndButtonMain, hWndButtonDropDown;	// Split-buttons controls
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WINSDKVER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINSDKVER));

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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINSDKVER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WINSDKVER);
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
   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 300, 300, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   // Create a split button control
   CreateSplitButton(hWnd);
   
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

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		
		switch (wmId)
		{
		case IDC_BTN_BUTTONA:
			if (wmEvent == BN_CLICKED)
			{
				// Notify user that they clicked the button
				MessageBox(hWnd, L"Option A", L"Caption", MB_OK);
			}
			break;
#if WINVER < 0x0600
		case IDC_BTN_BUTTONB:
			if (wmEvent == BN_CLICKED)
			{
				// Display drop down menu for downlevel button
				DisplayDropDownMenu();
			}
			break;
#endif
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case ID_OPTION1_OPTIONB:
			// Notify user that they clicked one down-down menu options
			MessageBox(hWnd, L"Option B", L"Caption", MB_OK);
			break;
		case ID_OPTION1_OPTIONC:
			// Notify user that they clicked one down-down menu options
			MessageBox(hWnd, L"Option C", L"Caption", MB_OK);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_NOTIFY:
#if WINVER >= 0x0600
		switch (((LPNMHDR)lParam)->code)
		{
			// Display drop-down for Vista style split button
			case BCN_DROPDOWN:
				DisplayDropDownMenu();
				break;
		}
#endif
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

VOID CreateSplitButton(HWND hParent)
{
	// The BS_SPLITBUTTON style is supported for on Vista and later.
	// We can use the value of WINVER to determine how to create the
	// split button control.
#if WINVER >= 0x0600
	// Create a split button using BS_SPLITBUTTON
	hWndButtonMain = CreateWindow(L"Button", L"Vista",
		WS_CHILD | WS_VISIBLE | BS_SPLITBUTTON,
		10, 10, 120, 50,
		hParent, (HMENU)IDC_BTN_BUTTONA, hInst, NULL);
#else
	// Create a split button using two separate button controls
	hWndButtonMain = CreateWindow(L"Button", L"Downlevel",
		WS_CHILD | WS_VISIBLE,
		10, 10, 100, 50,
		hParent, (HMENU)IDC_BTN_BUTTONA, hInst, NULL);
	
	hWndButtonDropDown = CreateWindow(L"Button", L".", WS_CHILD | WS_VISIBLE,
	   105, 10, 20, 50,
	   hParent, (HMENU)IDC_BTN_BUTTONB, hInst, NULL);
#endif
}

VOID DisplayDropDownMenu()
{
	RECT rc;
	
	// Get the bounding rectangle of the client area 
	GetWindowRect(hWndButtonMain, &rc);
	
	HMENU hmenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1));
	HMENU hmenuTrackPopup;
				
	hmenuTrackPopup = GetSubMenu(hmenu, 0);
	TrackPopupMenuEx(hmenuTrackPopup, TPM_RIGHTBUTTON, rc.left, rc.bottom, hWnd, NULL);
	DestroyMenu(hmenu);
}