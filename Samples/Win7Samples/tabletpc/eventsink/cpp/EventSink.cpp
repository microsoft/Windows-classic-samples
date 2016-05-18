// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Module:
//      EventSink.cpp
//
// Description:
//      This program demonstrates how to build an application that
//      captures InkCollector events using only C++.  It uses the
//      InkCollectorEvents base class (also included with this sample)
//      to convert the IDispatch event notification into a call to the
//      appropriate event handler.
//
//      This program co-creates an InkCollector object to enable inking
//      in the window. It displays a message box whenever a Stroke event
//      is received.
//
//      The only code specific to the Tablet PC SDK is located in the
//      WndProc method.
//
//      This application is discussed in the Getting Started guide.
//
//      The interfaces used are:
//      IInkCollector
//
//--------------------------------------------------------------------------

// Windows Header Files:
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <assert.h>

// ATL header files
#include <atlbase.h>        // defines CComModule, CComPtr, CComVariant

#include <msinkaut.h>
#include "TpcConpt.h"
#include "EventSink.h"
#define MAX_LOADSTRING 100
#include <msinkaut_i.c>

// Global Variables:
HINSTANCE hInst;                                // current instance
TCHAR szTitle[MAX_LOADSTRING];                    // The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


/////////////////////////////////////////////////////////
//
// WinMain
//
//      The WinMain function is called by the system as the
//      initial entry point for a Win32-based application.
//      It contains typical boilerplate code to create and
//      show the main window, and pump messages.
//
// Parameters:
//      HINSTANCE hInstance,      : [in] handle to current instance
//      HINSTANCE hPrevInstance,  : [in] handle to previous instance
//      LPSTR lpCmdLine,          : [in] command line
//      int nCmdShow              : [in] show state
//
// Return Values:
//      0 : The function terminated before entering the message loop.
//      non zero: Value of the wParam when receiving the WM_QUIT message
//
/////////////////////////////////////////////////////////
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE /* hPrevInstance */,
                     LPSTR     /* lpCmdLine */,
                     int       nCmdShow)
{
     CoInitialize(NULL);
    MSG msg;
    HACCEL hAccelTable;

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_EVENTSINK, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_EVENTSINK);

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    CoUninitialize();
    return (int) msg.wParam;
}

/////////////////////////////////////////////////////////
//
// MyRegisterClass
//
//      This registers the main window class.
//
// Parameters:
//      HINSTANCE hInstance,      : [in] handle to current instance
//
// Return Values:
//      0 : The function failed
//      non zero: Class was registered successfully
//
/////////////////////////////////////////////////////////
ATOM MyRegisterClass(
    HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style            = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = (WNDPROC)WndProc;
    wcex.cbClsExtra        = 0;
    wcex.cbWndExtra        = 0;
    wcex.hInstance        = hInstance;
    wcex.hIcon            = LoadIcon(hInstance, (LPCTSTR)IDI_EVENTSINK);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground    = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName    = (LPCTSTR)IDC_EVENTSINK;
    wcex.lpszClassName    = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

    return RegisterClassEx(&wcex);
}

/////////////////////////////////////////////////////////
//
// MyRegisterClass
//
//      Saves instance handle and creates main window. In this
//      function, we save the instance handle in a global variable and
//      create and display the main program window.
//
// Parameters:
//      HINSTANCE hInstance,      : [in] handle to current instance
//      int nCmdShow              : [in] how window is to be shown
//
// Return Values:
//      FALSE : Could not create window
//      TRUE:   Window successfully created
//
/////////////////////////////////////////////////////////
BOOL InitInstance(
    HINSTANCE hInstance,
    int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

/////////////////////////////////////////////////////////
//
// WndProc
//
//      Processes messages for the main window. The two
//      important messages are WM_CREATE and WM_DESTROY.
//      For WM_CREATE, the object which maintains the
//      InkCollector information is created and attaches
//      an InkCollector to the window. WM_DESTROY frees
//      the object which in turn frees all resources
//      connected with the InkCollector.
//
// Parameters:
//      HWND hWnd                   :   [in] Window
//      UINT message                :   [in] Message id
//      WPARAM wParam               :   [in] wParam for message
//      LPARAM lParam               :   [in] lParam for message
//
// Return Values:
//      0 :     Successfully processed message
//      Non-0:  Error occurred
//
/////////////////////////////////////////////////////////
LRESULT CALLBACK WndProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    static CMyInkCollector *pmic = NULL;
    int wmId, wmEvent;

    switch (message)
    {
    case WM_CREATE:

        // Allocate and initialize memory for object
        pmic = new CMyInkCollector();

        if (pmic != NULL)
        {
            // Real initialization. This consists of creating
            //  an ink collector object and attaching it to
            //  the current window.
            if (SUCCEEDED(pmic->Init(hWnd)))
            {
                return 0;
            }

            // Failure free resources.
            delete pmic;
            pmic = NULL;
        }

        return -1;

    case WM_COMMAND:

        wmId    = LOWORD(wParam);
        wmEvent = HIWORD(wParam);

        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
            break;

        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;

    case WM_DESTROY:
        // The destructor for the object handles releasing the
        //  InkCollector and disconnecting the InkCollector
        //  from the object's event sink.
        delete pmic;
        pmic = NULL;
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

/////////////////////////////////////////////////////////
//
// About
//
//      Message handler for About dialog box.
//
// Parameters:
//      HWND hDlg               :   [in] handle to dialog box
//      UINT message            :   [in] message id
//      WPARAM wParam           :   [in] wParam
//      LPARAM lParam           :   [in] lParam
//
// Return Values:
//      FALSE:  Did not process message
//      TRUE:   Processed message.
//
/////////////////////////////////////////////////////////
LRESULT CALLBACK About(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
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
