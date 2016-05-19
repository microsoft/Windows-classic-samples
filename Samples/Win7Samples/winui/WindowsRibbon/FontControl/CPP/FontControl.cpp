// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// RibbonApp.cpp : Defines the entry point for the application.
//

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
// Windows Header Files.
#include <windows.h>

#include "RibbonFramework.h"
#include "resource.h"
#include "RichEditMng.h"
#include "ids.h"
#include "MiniToolbar.h"

// Global Variables:
HINSTANCE hInst;                                         // current instance.
WCHAR wszTitle[MAX_LOADSTRING];                          // The title bar text.
WCHAR wszWindowClass[MAX_LOADSTRING];                    // the main window class name.
CFCSampleAppRichEditManager *g_pFCSampleAppManager = NULL;       // Object to manage the RichEdit control.

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        return FALSE;
    }


    // Initialize global strings.
    LoadString(hInstance, IDS_APP_TITLE, wszTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_FONTCONTROL, wszWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization.
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    // Main message loop.
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CoUninitialize();

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
//    so that the application has 'well formed' small icons associated
//    with it.
//
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FONTCONTROL));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = wszWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window.
//
//   COMMENTS:
//
//        In this function, an instance handle is saved in a global variable and
//        create and display the main program window.
//
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;

    hInst = hInstance; // Store instance handle in our global variable.

    hWnd = CreateWindow(wszWindowClass, wszTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

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
//  WM_COMMAND	- process the application menu.
//  WM_PAINT	- Paint the main window.
//  WM_DESTROY	- post a quit message and return.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    bool initSuccess = false;

    switch (message)
    {
    case WM_CREATE:
        // Initialize the RichEdit manager object.
        g_pFCSampleAppManager = new CFCSampleAppRichEditManager(hWnd, hInst);
        if (g_pFCSampleAppManager)
        {
            // Initializes the Ribbon framework.
            initSuccess = InitializeFramework(hWnd);
        }
        if (!initSuccess) 
        {
            return -1;
        }
        break;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        // Tears down the Ribbon framework.
        DestroyFramework();
        // Delete the RichEdit manager object.
        if (g_pFCSampleAppManager)
        {
            delete g_pFCSampleAppManager;
            g_pFCSampleAppManager = NULL;
        }
        PostQuitMessage(0);
        break;
    case WM_SIZE:
        // Resize the RichEdit control.
        g_pFCSampleAppManager->Resize();
        break;
    case WM_NOTIFY:
    {
        LPNMHDR phdr = (LPNMHDR) lParam;
        if (phdr->code == EN_SELCHANGE && phdr->hwndFrom == g_pFCSampleAppManager->GetRichEditWnd())
        {
            // Selection changed in RichEdit control, invalidate the ribbon.
            return (SUCCEEDED(g_pFramework->InvalidateUICommand(IDC_CMD_FONTCONTROL, UI_INVALIDATIONS_ALLPROPERTIES, 0)));
        }
        break;
    }
    case WM_CONTEXTMENU:
    {
        POINT pt;
        POINTSTOPOINT(pt, lParam);
        // Show the context menu at given point.
        if (ShowMiniToolbar(pt))
        {
            // Handled the message.
            break;
        }
        // Did not handle the message so call the default window procedure.
        __fallthrough;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
