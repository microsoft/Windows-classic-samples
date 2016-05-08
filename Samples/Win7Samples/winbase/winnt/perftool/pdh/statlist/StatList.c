//THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// PROGRAM: STATLIST.c
//
// PURPOSE: Illustrates the use of the PDH counter browsing dialog
//          and the statistical functions of the PDH
//
// PLATFORMS:  Windows NT (only)
//
// FUNCTIONS:
//    WinMain() - calls initialization function, processes message loop
//    InitApplication() - Initializes window data nd registers window
//    InitInstance() -saves instance handle and creates main window
//    WindProc() Processes messages
//    About() - Process menssages for "About" dialog box
//    MyRegisterClass() - Registers the application's window class
//    CenterWindow() -  Centers one window over another
//
// SPECIAL INSTRUCTIONS: N/A
//

// Windows Header Files:
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <memory.h>

// Local Header Files
#include "statlist.h"

// Global Variables:

HINSTANCE hInst;      // current instance
char szAppName[100];  // Name of the app
char szTitle[100];    // The title bar text
char APPNAME[] = "Statlist"; // title of the app

// Foward declarations of functions included in this code module:

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK About(HWND, UINT, WPARAM, LPARAM);
BOOL CenterWindow (HWND, HWND);
LPTSTR   GetStringRes (int id);


//
//  FUNCTION: MyRegisterClass(CONST WNDCLASS*)
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
// function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(CONST WNDCLASS *lpwc)
{
   HANDLE  hMod;
   FARPROC proc;
   WNDCLASSEX wcex;

   hMod = GetModuleHandle ("USER32");
   if (hMod != NULL) {

#if defined (UNICODE)
      proc = GetProcAddress (hMod, "RegisterClassExW");
#else
      proc = GetProcAddress (hMod, "RegisterClassExA");
#endif

      if (proc != NULL) {

         wcex.style         = lpwc->style;
         wcex.lpfnWndProc   = lpwc->lpfnWndProc;
         wcex.cbClsExtra    = lpwc->cbClsExtra;
         wcex.cbWndExtra    = lpwc->cbWndExtra;
         wcex.hInstance     = lpwc->hInstance;
         wcex.hIcon         = lpwc->hIcon;
         wcex.hCursor       = lpwc->hCursor;
         wcex.hbrBackground = lpwc->hbrBackground;
         wcex.lpszMenuName  = lpwc->lpszMenuName;
         wcex.lpszClassName = lpwc->lpszClassName;

         // Added elements for Windows 95:
         wcex.cbSize = sizeof(WNDCLASSEX);
         wcex.hIconSm = LoadIcon(wcex.hInstance, "SMALL");

         return (*proc)(&wcex);//return RegisterClassEx(&wcex) with type ATOM;
      }
   }
   return (RegisterClass(lpwc));
}

//
//  FUNCTION: InitApplication(HANDLE)
//
//  PURPOSE: Initializes window data and registers window class
//
//  COMMENTS:
//
//       In this function, we initialize a window class by filling out a data
//       structure of type WNDCLASS and calling either RegisterClass or
//       the internal MyRegisterClass.
//
BOOL InitApplication(HINSTANCE hInstance)
{
    WNDCLASS  wc;
    HWND      hwnd;

    // Win32 will always set hPrevInstance to NULL, so lets check
    // things a little closer. This is because we only want a single
    // version of this app to run at a time
    hwnd = FindWindow (szAppName, NULL);
    if (hwnd) {
        // We found another version of ourself. Lets defer to it:
        if (IsIconic(hwnd)) {
            ShowWindow(hwnd, SW_RESTORE);
        }
        SetForegroundWindow (hwnd);

        // If this app actually had any functionality, we would
        // also want to communicate any action that our 'twin'
        // should now perform based on how the user tried to
        // execute us.
        return FALSE;
        }

        // Fill in window class structure with parameters that describe
        // the main window.
        wc.style         = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = (WNDPROC)WndProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = 0;
        wc.hInstance     = hInstance;
        wc.hIcon         = LoadIcon (hInstance, szAppName);
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);

        wc.lpszMenuName  = szAppName;
        wc.lpszClassName = szAppName;

        // Register the window class and return success/failure code.
        return RegisterClass(&wc);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
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
   DWORD dwStatus;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szAppName, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
      NULL, NULL, hInstance, NULL);

   if (!hWnd) {
      dwStatus = GetLastError();
      return (FALSE);
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return (TRUE);
}

//
//  FUNCTION: WinMain(HANDLE, HANDLE, LPSTR, int)
//
//  PURPOSE: Entry point for the application.
//
//  COMMENTS:
//
// This function initializes the application and processes the
// message loop.
//
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
   MSG msg;
   HANDLE hAccelTable;

   // Initialize global strings
   strncpy_s(szAppName, 99, APPNAME, _TRUNCATE);
   szAppName[99] = '\0'; // ensure that szAppName is NULL terminated
   LoadString (hInstance, IDS_APP_TITLE, szTitle, 100);


   if (!hPrevInstance) {
      // Perform instance initialization:
      if (!InitApplication(hInstance)) {
         return (FALSE);
      }
   }

   // Perform application initialization:
   if (!InitInstance(hInstance, nCmdShow)) {
      return (FALSE);
   }

   hAccelTable = LoadAccelerators (hInstance, szAppName);

   // Main message loop:
   while (GetMessage(&msg, NULL, 0, 0)) {
      if (!TranslateAccelerator (msg.hwnd, hAccelTable, &msg)) {
         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }
   }

   return (int) (msg.wParam);

   lpCmdLine; // This will prevent 'unused formal parameter' warnings
}

