// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#define MAX_LOADSTRING             100                 // Maximal size of a string used in application

// Windows Header Files
#include <windows.h>

// C RunTime Header Files
#include <stdio.h>
#include <tchar.h>

// Other Headers
#include "Program.h"
#include "resource.h"

// Global Variables
wchar_t   g_windowClass[MAX_LOADSTRING];            // Name of window class
wchar_t   g_mainTitle[MAX_LOADSTRING];              // Application main window title
wchar_t   g_touchHitTestingOnText[MAX_LOADSTRING];  // Touch hit testing button label (enabled)
wchar_t   g_touchHitTestingOffText[MAX_LOADSTRING]; // Touch hit testing button label (disabled)
wchar_t   g_resetText[MAX_LOADSTRING];              // Reset button label
int       g_width;                                  // The client windows width
int       g_height;                                 // The client windows height
CProgram *g_driver;                                 // Encapsulates all Touch/Mouse Event handling

// Forward declaration of the window procedure
LRESULT CALLBACK WndProc(_In_ HWND, _In_ UINT, _In_ WPARAM, _In_ LPARAM);

// Register Window Class
ATOM MyRegisterClass(_In_ HINSTANCE hInst)
{
    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = g_windowClass;

    ATOM atom = RegisterClassEx(&wc);
    if (atom == NULL)
    {
        fprintf(stderr, "RegisterClassEx failed with error code %u\n", GetLastError());
    }
    return atom;
}

// Initialize program instance
bool InitInstance(_In_ HINSTANCE hinst, _In_ int cmdShow)
{
    bool success = TRUE;

    // Create maximized window
    HWND hWnd = CreateWindowEx(0, g_windowClass, g_mainTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                               CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, hinst, 0);

    if (hWnd == nullptr)
    {
        fprintf(stderr, "CreateWindowEx failed with error code %u\n", GetLastError());
        success = false;
    }

    // Create and initialize touch and mouse handler
    if (success)
    {
        g_driver = new(std::nothrow) CProgram();

        if (g_driver == nullptr)
        {
            fprintf(stderr, "Failed to allocate memory for new object.\n");
            success = false;
        }

        if (success)
        {
            success = g_driver->Initialize(hWnd, g_touchHitTestingOnText, g_touchHitTestingOffText, g_resetText);
        }
    }

    if (success)
    {
        ShowWindow(hWnd, cmdShow);
        UpdateWindow(hWnd);
    }
    else if (g_driver != nullptr)
    {
        delete g_driver;
        g_driver = nullptr;
    }

    return success;
}

// Program entry point
int APIENTRY wWinMain(_In_ HINSTANCE hinst, _In_opt_ HINSTANCE /*hinstPrev*/, _In_ LPWSTR /*lpCmdLine*/, _In_ int /*nCmdShow*/)
{
    // Initialize localized global strings
    if (LoadString(hinst, IDS_WINDOW, g_windowClass, MAX_LOADSTRING) == 0)
    {
        fprintf(stderr, "LoadString() failed with error code %u.\n", GetLastError());
        return -1;
    }
    if (LoadString(hinst, IDS_CAPTION, g_mainTitle, MAX_LOADSTRING) == 0)
    {
        fprintf(stderr, "LoadString() failed with error code %u.\n", GetLastError());
        return -1;
    }
    if (LoadString(hinst, IDS_TOUCHHITTESTING_ON, g_touchHitTestingOnText, MAX_LOADSTRING) == 0)
    {
        fprintf(stderr, "LoadString() failed with error code %u.\n", GetLastError());
        return -1;
    }
    if (LoadString(hinst, IDS_TOUCHHITTESTING_OFF, g_touchHitTestingOffText, MAX_LOADSTRING) == 0)
    {
        fprintf(stderr, "LoadString() failed with error code %u.\n", GetLastError());
        return -1;
    }
    if (LoadString(hinst, IDS_RESET, g_resetText, MAX_LOADSTRING) == 0)
    {
        fprintf(stderr, "LoadString() failed with error code %u.\n", GetLastError());
        return -1;
    }

    // D2D automatically handles high DPI settings
    SetProcessDPIAware();

    // Register Class
    if (!MyRegisterClass(hinst))
    {
        return -1;
    }

    // Initialize Application
    if (!InitInstance(hinst, SW_SHOWMAXIMIZED))
    {
        return -1;
    }

    // Main message loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0) != 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    delete g_driver;
    g_driver = nullptr;

    return 0;
}

// Processes messages for main Window
LRESULT CALLBACK WndProc(_In_ HWND hWnd, _In_ UINT msg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    PAINTSTRUCT ps;

    switch (msg)
    {
    case WM_DESTROY:
        if (g_driver != nullptr)
        {
            delete g_driver;
            g_driver = nullptr;
        }

        PostQuitMessage(0);
        return 1;

    case WM_SIZE:
        g_width = LOWORD(lParam);
        g_height = HIWORD(lParam);
        break;

    case WM_PAINT:
        BeginPaint(hWnd, &ps);
        g_driver->RenderInitialState(g_width, g_height);
        EndPaint(hWnd, &ps);
        break;

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MOUSEMOVE:
        {
            if (msg == WM_MOUSEMOVE)
            {
                // Filter left button only
                if  (LOWORD(wParam) != MK_LBUTTON)
                {
                    break;
                }
            }

            // Create and process mouse event
            INPUT_EVENT inputEvent;
            inputEvent.pt.y = HIWORD(lParam);
            inputEvent.pt.x = LOWORD(lParam);
            inputEvent.eventSource = EVENT_SOURCE_MOUSE;
            inputEvent.eventType = (msg == WM_LBUTTONDOWN) ? EVENT_TYPE_DOWN :
                (msg == WM_MOUSEMOVE) ? EVENT_TYPE_MOVE : EVENT_TYPE_UP;
            inputEvent.cursorId = CURSOR_ID_MOUSE;
            g_driver->ProcessInputEvent(&inputEvent, 1);
        }
        break;

    case WM_TOUCHHITTESTING:
        // Do smart Touch hit testing
        return g_driver->OnTouchHitTesting((PTOUCH_HIT_TESTING_INPUT)lParam);

    case WM_POINTERDOWN:
    case WM_POINTERUPDATE:
    case WM_POINTERUP:
        {
            // Create and process pointer events
            INPUT_EVENT inputEvent;
            inputEvent.pt.y = HIWORD(lParam);
            inputEvent.pt.x = LOWORD(lParam);

            // Pointer position is in screen coordinates, convert to client coordinates
            ScreenToClient(hWnd, &inputEvent.pt);

            // Program works with any pointer input (touch, pen).
            // But only touch input can initialize smart Touch hit testing.
            inputEvent.eventSource = EVENT_SOURCE_POINTER;
            inputEvent.eventType = (msg == WM_POINTERDOWN) ? EVENT_TYPE_DOWN :
                (msg == WM_POINTERUPDATE) ? EVENT_TYPE_MOVE : EVENT_TYPE_UP;
            inputEvent.cursorId = GET_POINTERID_WPARAM(wParam);
            g_driver->ProcessInputEvent(&inputEvent, 1);
        }
        break;

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}
