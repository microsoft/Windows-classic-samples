//////////////////////////////////////////////////////////////////////
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////

#include "DXVAHD_Sample.h"

/* 

Command line options:

    [ -hh | -hs | -ss ] 

        -hh : Hardware Direct3D device and hardware DXVA-HD device.
        -hs : Hardware Direct3D device and software DXVA-HD device.
        -ss : Software Direct3D device and software DXVA-HD device.

    -u [0 | 1 | 2]

        Specifies the DXVA-HD device usage. 

        0 = Normal video playback
        1 = Optimal speed
        2 = Optimal quality


Modes

    The user can toggle between different modes by pressing the
    F1 through F9 keys.

    The arrow keys control different settings in each mode:

    F1 : Alpha values

        UP/DOWN:    Main video planar alpha
        LEFT/RIGHT: Substream pixel alpha

    F2 : Resize the main video source rectangle.

    F3 : Move the main video source rectangle.

    F4 : Resize the main video destination rectangle.

    F5 : Move the main video destination rectangle.

    F6 : Change the background color or extended color information.

        UP/DOWN:    Change YCbCr standard and RGB color range. (See EX_COLOR_INFO[])
        LEFT/RIGHT: Cycle through background colors.

    F7 : Ajust brightness and contrast.

        UP/DOWN:    Brightness
        LEFT/RIGHT: Contrast

    F8 : Adjust hue and saturation.

        UP/DOWN:    Hue
        LEFT/RIGHT: Saturation

    F9: Resize the target rectangle.

    HOME : Resets all mode settings to their default values.

    ALT + ENTER: Switch between windowed and full-screen mode.


*/

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Constants

const WCHAR CLASS_NAME[]  = L"DXVA2 HD Sample Class";
const WCHAR WINDOW_NAME[] = L"DXVA2 HD Sample Application";


//-------------------------------------------------------------------
// WinMain
//
// Application entry-point.
//-------------------------------------------------------------------

INT WINAPI wWinMain(HINSTANCE,HINSTANCE,LPWSTR,INT)
{
    (void)HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    Application application;    // Manages the application logic.

    if (application.Initialize())
    {
        application.MessageLoop();
    }

    return 0;
}


//-------------------------------------------------------------------
// WindowProc
//
// Main window procedure.
//-------------------------------------------------------------------

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static Application *pApp = NULL;

    switch (uMsg)
    {
    case WM_CREATE:
        pApp = (Application*) (((LPCREATESTRUCT) lParam)->lpCreateParams);
        break;

    case WM_DESTROY:    
        PostQuitMessage(0);
        break;

    default:
        if (pApp)
        {
            return pApp->HandleMessage(hwnd, uMsg, wParam, lParam);
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

//-------------------------------------------------------------------
// InitializeWindow
//
// Creates the application window.
//-------------------------------------------------------------------

HWND InitializeWindow(LPVOID lpParam)
{
    WNDCLASS wc = {0};

    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = GetModuleHandle(NULL);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClass(&wc))
    {
        DBGMSG(L"RegisterClass failed with error %d.\n", GetLastError());
        return NULL;
    }

    HWND hwnd = CreateWindow(
        CLASS_NAME,
        WINDOW_NAME,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        lpParam
        );

    if (!hwnd)
    {
        DBGMSG(L"CreateWindow failed with error %d.\n", GetLastError());
        return NULL;
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    return hwnd;
}


void DBGMSG(PCWSTR format, ...)
{
    va_list args;
    va_start(args, format);

    WCHAR string[MAX_PATH];

    if (SUCCEEDED(StringCbVPrintf(string, sizeof(string), format, args)))
    {
        OutputDebugString(string);
    }
    else
    {
        DebugBreak();
    }
}

