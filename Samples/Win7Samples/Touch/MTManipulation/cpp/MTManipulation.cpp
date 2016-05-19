// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// MTManipulation application
// Description:
//  This application will initially draw rectangle in the center of client area
//  (including diagonals). The user can manipulate the rectangle using his/her 
//  fingers. The available commands are:
//      - rectangle stretch 
//          By putting two fingers on the screen and modifying distance between 
//          them by moving fingers in the opposite directions or towards each
//          other the user can zoom in/out this rectangle.
//      - panning
//          By touching the screen with two fingers and moving them in the same 
//          direction the user can move the rectangle. Also it's possible to pan
//          object by using single finger (SFP - single finger panning)
//      - rotate
//          By putting one finger in the center of the rotation and then rotating 
//          the other finger around it the user can rotate the rectangle
//
// Purpose:
//  This application demonstrate how to use _IManipulationEvents interface to 
//  handle WM_TOUCH message to manipulate objects.
//
//  Manipulations are used to simplify 2D transformation operations on objects for 
//  touch-enabled applications. By using manipulations, developers can more easily 
//  perform transformations on objects in a way that is consistent with other 
//  applications. 2D transformations resulting from applying system gestures zoom, 
//  pan, and rotate, may be implemented using Manipulations and raw touch input 
//  (WM_TOUCH).
//
// MTManipulation.cpp : Defines the entry point for the application.
//

// Windows header files
#include <windows.h>

// C RunTime header files
#include <assert.h>
#define ASSERT assert

// Application specific header files
#include "Resource.h"
#include "CManipulationEventSink.h" // contains definition of the CManipulationEventSink class  

#define MAX_LOADSTRING 100
    
// Set up a variable to point to the manipulation event sink implementation class    
CManipulationEventSink* g_pManipulationEventSink = NULL;    
       
// Pointer to a global reference of a manipulation processor event sink
IManipulationProcessor* g_pIManipProc = NULL;     

CDrawingObject g_cRect; // CDrawingObject class holds information about the rectangle
                        // and it is responsible for painting the rectangle


///////////////////////////////////////////////////////////////////////////////
// Application framework (wizard code)

// Global Variables:
HINSTANCE g_hInst;                              // current instance
WCHAR g_wszTitle[MAX_LOADSTRING];               // The title bar text
WCHAR g_wszWindowClass[MAX_LOADSTRING];         // the main window class name

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

    // Initialize COM
    HRESULT hr = CoInitialize(0);       
    if (FAILED(hr))
    {
        ASSERT(SUCCEEDED(hr) && L"Failed to execute CoInitialize");
        return FALSE;
    }

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, g_wszTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_MTMANIPULATION, g_wszWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization
    if (!InitInstance (hInstance, nCmdShow))
    {
        // Uninitialize COM
        CoUninitialize();
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

    // Uninitialize COM
    CoUninitialize();

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

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = 0;
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = g_wszWindowClass;
    wcex.hIconSm        = 0;

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

    // Register application window for receiving multi-touch input. Use default settings.
    if (!RegisterTouchWindow(hWnd, 0))
    {
        MessageBox(hWnd, L"Cannot register application window for multi-touch input", L"Error", MB_OK);
        return FALSE;
    }
    ASSERT(IsTouchWindow(hWnd, NULL));

    // Instantiate the ManipulationProcessor object
    HRESULT hr = CoCreateInstance(__uuidof(ManipulationProcessor), NULL, CLSCTX_ALL, IID_PPV_ARGS(&g_pIManipProc));
    if (FAILED(hr))
    {
        ASSERT(SUCCEEDED(hr) && L"InitInstance: failed to instantiate the ManipulationProcessor object");
        return FALSE;
    }

    // Instantiate the event sink with the manipulation processor and pointer to the rectangle object
    g_pManipulationEventSink = new CManipulationEventSink(&g_cRect);
    if (g_pManipulationEventSink == NULL)
    {
        ASSERT(g_pManipulationEventSink && L"InitInstance: failed to instantiate the CManipulationEventSink class");
        g_pIManipProc->Release();
        g_pIManipProc = NULL;
        return FALSE;
    }

    // Establish the link between ManipulationEventSink and ManipulationProcessor
    if (!g_pManipulationEventSink->Connect(g_pIManipProc))
    {
        ASSERT(FALSE && L"InitInstance: failed to connect ManipulationEventSink and ManipulationProcessor");
        g_pIManipProc->Release();
        g_pIManipProc = NULL;
        g_pManipulationEventSink->Release();
        g_pManipulationEventSink = NULL;
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

// Processes messages for the main window:
//      WM_COMMAND  - process the application menu
//      WM_SIZE     - process resizing of the client area
//      WM_PAINT    - paint the main window
//      WM_TOUCH    - multi-touch message
//      WM_DESTROY  - post a quit message and return
// This function is generated by Visual Studio app wizard; added WM_PAINT, WM_TOUCH
// and WM_DESTROY code.
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

    case WM_SIZE:
        // resize rectangle and place it in the center of the new client area
        g_cRect.ResetObject(LOWORD(lParam),HIWORD(lParam));        
        break;

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);

        // Full redraw: background + rectangle
        g_cRect.Paint(hdc);

        EndPaint(hWnd, &ps);
        break;

    // WM_TOUCH message handlers
    case WM_TOUCH:
        {
            // WM_TOUCH message can contain several messages from different contacts
            // packed together.
            // Message parameters need to be decoded:
            UINT  cInputs  = (int) wParam;      // Number of actual per-contact messages
            TOUCHINPUT* pInputs = new TOUCHINPUT[cInputs]; // Allocate the storage for the parameters of the per-contact messages
            if (pInputs == NULL)
            {
                break;
            }
            // Unpack message parameters into the array of TOUCHINPUT structures, each
            // representing a message for one single contact.
            if (GetTouchInputInfo((HTOUCHINPUT)lParam, cInputs, pInputs, sizeof(TOUCHINPUT)))
            {
                // For each contact, dispatch the message to the appropriate message
                // handler.
                for (unsigned int i = 0; i < cInputs; i++)
                {
                    if (pInputs[i].dwFlags & TOUCHEVENTF_DOWN)
                    {
                        g_pIManipProc->ProcessDown(pInputs[i].dwID, (FLOAT)pInputs[i].x, (FLOAT)pInputs[i].y);
                    }
                    else if (pInputs[i].dwFlags & TOUCHEVENTF_MOVE)
                    {
                        g_pIManipProc->ProcessMove(pInputs[i].dwID, (FLOAT)pInputs[i].x, (FLOAT)pInputs[i].y);
                    }
                    else if (pInputs[i].dwFlags & TOUCHEVENTF_UP)
                    {
                        g_pIManipProc->ProcessUp(pInputs[i].dwID, (FLOAT)pInputs[i].x, (FLOAT)pInputs[i].y);
                    }
                }
            }
            else
            {
                // error handling, presumably out of memory
                ASSERT(FALSE && L"Error: failed to execute GetTouchInputInfo");
                delete [] pInputs;
                break;
            }
            if (!CloseTouchInputHandle((HTOUCHINPUT)lParam))
            {
                // error handling, presumably out of memory
                ASSERT(FALSE && L"Error: failed to execute CloseTouchInputHandle");
                delete [] pInputs;
                break;
            }
            delete [] pInputs;

            // Force redraw of the rectangle
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;

    case WM_DESTROY:
        // Clean up of application data: unregister window for multi-touch.
        if (!UnregisterTouchWindow(hWnd))
        {
            MessageBox(NULL, L"Cannot unregister application window for touch input", L"Error", MB_OK);
        }
        ASSERT(!IsTouchWindow(hWnd, NULL));

        // COM objects must be released before CoUninitialize
        // Release ManipulationEventSink object 
        if (g_pManipulationEventSink != NULL) 
        {
            // Terminate the connection between ManipulationEventSink and ManipulationProcessor
            if (!g_pManipulationEventSink->Disconnect())
            {
                ASSERT(FALSE && L"Failed to disconnect ManipulationEventSink from ManipulationProcessor");
            }

            g_pManipulationEventSink->Release();
            g_pManipulationEventSink = NULL;
        }
        // Release ManipulationProcessor object
        if (g_pIManipProc != NULL)
        {
            g_pIManipProc->Release();
            g_pIManipProc = NULL;
        }

        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
