// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#ifndef WINVER                  // Specifies that the minimum required platform is Windows 7.
#define WINVER 0x0601           // Target version is for Windows 7.
#endif

// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <tpcshrd.h>

// Other Headers
#include "ComTouchDriver.h"
#include "resource.h"

#define MAX_LOADSTRING 100

// Global Variables
HINSTANCE   g_hInst;
HWND        g_hWnd;
WCHAR       g_tszWindowClass[MAX_LOADSTRING];
WCHAR       g_tszMainTitle[MAX_LOADSTRING];
int         g_iCWidth;			// The client windows width
int         g_iCHeight;			// The client windows height

CComTouchDriver* g_ctDriver;	// Encapsulates all Touch/Mouse Event handling

// Forward declarations of functions
ATOM                MyRegisterClass(HINSTANCE hInst);
BOOL                InitInstance(HINSTANCE hinst, int nCmdShow);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
VOID                SetTabletInputServiceProperties();
VOID                FillInputData(TOUCHINPUT* inData, DWORD cursor, DWORD eType, DWORD time, int x, int y);

// Program entry point
int APIENTRY wWinMain(__in HINSTANCE hinst, __in_opt HINSTANCE hinstPrev, __in LPWSTR lpCmdLine,__in int nCmdShow)
{
    (void)HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
    UNREFERENCED_PARAMETER(hinstPrev);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);
    BOOL success = TRUE;
    MSG msg;
    
    // Initialize global strings
    LoadString(hinst, IDS_WINDOW, g_tszWindowClass, MAX_LOADSTRING);
    LoadString(hinst, IDS_CAPTION, g_tszMainTitle, MAX_LOADSTRING);

    // D2D automatically handles high DPI settings
    SetProcessDPIAware();

    // Register Class
    MyRegisterClass(hinst);

    // Initialize Application
    if (!InitInstance(hinst, SW_SHOWMAXIMIZED)) 
    {
        wprintf(L"Failed to initialize application");
        success = FALSE;
    }
    
    if(success)
    {
        // Main message loop
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return success;
} 

// Register Window Class
ATOM MyRegisterClass(HINSTANCE hInst)
{
    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszClassName = g_tszWindowClass;

    return RegisterClassEx(&wc);
}

// Initialize Instance
BOOL InitInstance(HINSTANCE hinst, int nCmdShow)
{
    BOOL success = TRUE;

    // Store instance handler in global variable
    g_hInst = hinst; 

    g_hWnd = CreateWindowEx(0, g_tszWindowClass, g_tszMainTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 
        CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, g_hInst, 0);
    

    if(!g_hWnd)
    {
        success = FALSE;
    }

    // Create and initialize touch and mouse handler

    if(success)
    {
        g_ctDriver = new (std::nothrow) CComTouchDriver(g_hWnd);

        if(g_ctDriver == NULL)
        {
            success = FALSE;
        }

        if(success)
        {
            success = g_ctDriver->Initialize();
        }
    }

    if(success)
    {
        // Disable UI feedback for penflicks 
        SetTabletInputServiceProperties();
        
        // Ready for handling WM_TOUCH messages
        RegisterTouchWindow(g_hWnd, 0);
        
        ShowWindow(g_hWnd, nCmdShow);
        UpdateWindow(g_hWnd);
    }
    else
    {
        if(g_ctDriver)
        {
            delete g_ctDriver;
            g_ctDriver = NULL;
        }
    }
    return success;
}

// Processes messages for main Window
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PTOUCHINPUT pInputs;
    TOUCHINPUT tInput;
    HTOUCHINPUT hInput;
    int iNumContacts;
    POINT ptInputs;
    PAINTSTRUCT ps;

    // Handle each type of inData and based on which event we handle continue processing
    // the inData for manipulations by calling the ComTouchDriver
    
    switch (msg)
    {
    case WM_TOUCH:

        iNumContacts = LOWORD(wParam);
        hInput = (HTOUCHINPUT)lParam;

        pInputs = new (std::nothrow) TOUCHINPUT[iNumContacts];
       
        // Get each touch input info and feed each TOUCHINPUT into the process input handler

        if(pInputs != NULL)
        {
            if(GetTouchInputInfo(hInput, iNumContacts, pInputs, sizeof(TOUCHINPUT)))
            {
               for(int i = 0; i < iNumContacts; i++)
               {
                   // Bring touch input info into client coordinates

                   ptInputs.x = pInputs[i].x/100;	
                   ptInputs.y = pInputs[i].y/100;
                   ScreenToClient(g_hWnd, &ptInputs);
                   pInputs[i].x = ptInputs.x;
                   pInputs[i].y = ptInputs.y;
                   g_ctDriver->ProcessInputEvent(&pInputs[i]);
               }
               g_ctDriver->ProcessChanges();
            }
        }
        
        delete [] pInputs;
        CloseTouchInputHandle(hInput);
        break;
    
    // For each Mouse event build a TOUCHINPUT struct and feed into the process input handler

    case WM_LBUTTONDOWN:

        FillInputData(&tInput, MOUSE_CURSOR_ID, TOUCHEVENTF_DOWN, (DWORD)GetMessageTime(),LOWORD(lParam),HIWORD(lParam));
        g_ctDriver->ProcessInputEvent(&tInput);        
        break;

    case WM_MOUSEMOVE:

        if(LOWORD(wParam) == MK_LBUTTON)
        {
            FillInputData(&tInput, MOUSE_CURSOR_ID, TOUCHEVENTF_MOVE, (DWORD)GetMessageTime(),LOWORD(lParam), HIWORD(lParam));
            g_ctDriver->ProcessInputEvent(&tInput);      
            g_ctDriver->ProcessChanges();
        }
      
        break;

    case WM_LBUTTONUP:

        FillInputData(&tInput, MOUSE_CURSOR_ID, TOUCHEVENTF_UP, (DWORD)GetMessageTime(),LOWORD(lParam), HIWORD(lParam));
        g_ctDriver->ProcessInputEvent(&tInput);
        
        break;

    case WM_DESTROY:
        
        if(g_ctDriver)
        {
            delete g_ctDriver;
        }
        PostQuitMessage(0);
        return 1;

    case WM_SIZE:

        g_iCWidth = LOWORD(lParam);
        g_iCHeight = HIWORD(lParam);
        break;

    case WM_PAINT:
        
        BeginPaint(g_hWnd, &ps);
        g_ctDriver->RenderInitialState(g_iCWidth, g_iCHeight);
        EndPaint(g_hWnd, &ps);
        break;

    case WM_TIMER:
        g_ctDriver->ProcessChanges();
        break;
      
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

// Fills the input data received from the mouse and builds the tagTOUCHINPUT struct

VOID FillInputData(TOUCHINPUT* inData, DWORD cursor, DWORD eType, DWORD time, int x, int y)
{
    inData->dwID = cursor;
    inData->dwFlags = eType;
    inData->dwTime = time;
    inData->x = x;
    inData->y = y;
}

VOID SetTabletInputServiceProperties()
{
    DWORD_PTR dwHwndTabletProperty = 
    TABLET_DISABLE_PRESSANDHOLD | // disables press and hold (right-click) gesture
    TABLET_DISABLE_PENTAPFEEDBACK | // disables UI feedback on pen up (waves)
    TABLET_DISABLE_PENBARRELFEEDBACK | // disables UI feedback on pen button down (circle)
    TABLET_DISABLE_FLICKS; // disables pen flicks (back, forward, drag down, drag up)
    
    ATOM atom = ::GlobalAddAtom(MICROSOFT_TABLETPENSERVICE_PROPERTY);  
    SetProp(g_hWnd, MICROSOFT_TABLETPENSERVICE_PROPERTY, reinterpret_cast<HANDLE>(dwHwndTabletProperty));
    GlobalDeleteAtom(atom);

}
