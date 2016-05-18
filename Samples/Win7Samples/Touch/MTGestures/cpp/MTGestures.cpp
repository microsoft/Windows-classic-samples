// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// MTGestures application
// Description:
//  This application demonstrate how to handle multi-touch gesture commands. This
//  application will initially draw rectangle in the middle of client area. By 
//  using gestures a user can manipulate the rectangle. The available 
//  commands are:
//      - rectangle stretch 
//          By putting two fingers on the screen and modifying distance between 
//          them by moving fingers in the opposite directions or towards each
//          other the user can zoom in/out this rectangle.
//      - panning
//          By touching the screen with two fingers and moving them in the same 
//          direction the user can move the rectangle. 
//      - rotate
//          By putting one finger in the center of the rotation and then rotating 
//          the other finger around it the user can rotate the rectangle
//      - two finger tap
//          By tapping the screen with two fingers the user can toggle drawing of 
//          the diagonals
//      - finger press and tap
//          This gesture involves movements of two fingers. It consists first of
//          putting one finger down. Then putting the second finger down and then
//          lifting it up. Finally the first finger is lifted up. This gesture 
//          will change the color of the rectangle.
//
// Purpose:
//  This sample demonstrates handling of the multi-touch gestures. 
//
// MTGestures.cpp : Defines the entry point for the application.

// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <assert.h>

// Application specific header files
#include "resource.h"
#include "DrawingObject.h"   // contains definition of the CDrawingRectangle class
#include "MyGestureEngine.h" // contains definition of the CMyGestureEngine class

#define ASSERT assert

#define MAX_LOADSTRING 100

CDrawingObject g_cRect; // a class that manipulate the rectangle

// class that reads and processes multi-touch gesture commands
CMyGestureEngine g_cGestureEngine(&g_cRect); 

///////////////////////////////////////////////////////////////////////////////
// Application framework (wizard code)

// Global Variables:
HINSTANCE g_hInst;                      // current instance
WCHAR g_wszTitle[MAX_LOADSTRING];       // the title bar text
WCHAR g_wszWindowClass[MAX_LOADSTRING]; // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

// Win32 application main entry point function.
// in:
//      hInstance       handle of the application instance
//      hPrevInstance   not used, always NULL
//      lpCmdLine       command line for the application, null-terminated string
//      nCmdShow        how to show the window
int APIENTRY wWinMain(HINSTANCE hInstance,
                     HINSTANCE /* hPrevInstance */,
                     LPWSTR    /* lpCmdLine */,
                     int       nCmdShow)
{
    MSG msg;

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, g_wszTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_MTGESTURES, g_wszWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    // Main message loop
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, NULL, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



// Registers the window class of the application.
// This function and its usage are only necessary if you want this code
// to be compatible with Win32 systems prior to the 'RegisterClassEx'
// function that was added to Windows 95. It is important to call this function
// so that the application will get 'well formed' small icons associated
// with it.
// in:
//      hInstance       handle to the instance of the application
// returns:
//      class atom that uniquely identifies the window class
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style            = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc      = WndProc;
    wcex.cbClsExtra       = 0;
    wcex.cbWndExtra       = 0;
    wcex.hInstance        = hInstance;
    wcex.hIcon            = 0;
    wcex.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground    = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName     = NULL;
    wcex.lpszClassName    = g_wszWindowClass;
    wcex.hIconSm          = 0;

    return RegisterClassEx(&wcex);
}

// Saves instance handle and creates main window
// In this function, we save the instance handle in a global variable and
// create and display the main program window.
// in:
//      hInstance       handle to the instance of the application
//      nCmdShow        how to show the window
// returns:
//      flag, succeeded or failed to create the window
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   g_hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(g_wszWindowClass, g_wszTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

// End of the wizard code. 


// The code below is from the wizard but it has been customized for this 
// application. There is a custom code to handle WM_SIZE, WM_GESTURENOTIFY
// and WM_GESTURE messages.
//
// Processes messages for the main window:
//      WM_COMMAND        - process the application menu
//      WM_SIZE           - process resizing of client window
//      WM_PAINT          - paint the main window
//      WM_DESTROY        - post a quit message and return
//      WM_GESTURENOTIFY  - process a gesture notification message 
//      WM_GESTURE        - process the gesture command
// in:
//      hWnd        window handle
//      message     message code
//      wParam      message parameter (message-specific)
//      lParam      message parameter (message-specific)
// returns:
//      the result of the message processing and depends on the message sent
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId;
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_GESTURENOTIFY:
        {
            // This is the right place to define the list of gestures that this
            // application will support. By populating GESTURECONFIG structure 
            // and calling SetGestureConfig function. We can choose gestures 
            // that we want to handle in our application. In this app we
            // decide to handle all gestures.
            GESTURECONFIG gc = {
                0,              // gesture ID
                GC_ALLGESTURES, // settings related to gesture ID that are to be 
                                // turned on
                0               // settings related to gesture ID that are to be 
                                // turned off
            };

            BOOL bResult = SetGestureConfig(
                hWnd,                 // window for which configuration is specified  
                0,                    // reserved, must be 0
                1,                    // count of GESTURECONFIG structures
                &gc,                  // array of GESTURECONFIG structures, dwIDs will be processed in the
                                      // order specified and repeated occurances will overwrite previous ones
                sizeof(GESTURECONFIG) // sizeof(GESTURECONFIG)
            );                        
            
            if (!bResult)
            {
                ASSERT(L"Error in execution of SetGestureConfig" && 0);
            }
        }
        break;

    case WM_GESTURE:
        // The gesture processing code is implemented in the CGestureEngine 
        // class.
        return g_cGestureEngine.WndProc(hWnd,wParam,lParam);
        break;

    case WM_SIZE:
        // resize rectangle and place it in the middle of the new client area
        g_cRect.ResetObject(LOWORD(lParam),HIWORD(lParam));
        break;

    case WM_COMMAND:
        wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);       

        // Full redraw: background + rectangle
        g_cRect.Paint(hdc);

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
