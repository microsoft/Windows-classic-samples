/*************************************************************************************************
*
* File: MagnifierSample.cpp
*
* Description: Implements a simple control that magnifies the screen, using the 
* Magnification API.
*
* The magnification window is quarter-screen by default but can be resized.
* To make it full-screen, use the Maximize button or double-click the caption
* bar. To return to partial-screen mode, click on the application icon in the 
* taskbar and press ESC. 
*
* In full-screen mode, all keystrokes and mouse clicks are passed through to the
* underlying focused application. In partial-screen mode, the window can receive the 
* focus. 
*
* Multiple monitors are not supported.
*
* 
* Requirements: To compile, link to Magnification.lib. The sample must be run with 
* elevated privileges.
*
* The sample is not designed for multimonitor setups.
* 
*  This file is part of the Microsoft WinfFX SDK Code Samples.
* 
*  Copyright (C) Microsoft Corporation.  All rights reserved.
* 
* This source code is intended only as a supplement to Microsoft
* Development Tools and/or on-line documentation.  See these other
* materials for detailed information regarding Microsoft code samples.
* 
* THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
* 
*************************************************************************************************/

// Ensure that the following definition is in effect before winuser.h is included.
#define _WIN32_WINNT 0x0501    

#include <windows.h>
#include <wincodec.h>
#include <magnification.h>

// For simplicity, the sample uses a constant magnification factor.
#define MAGFACTOR  2.0f
#define RESTOREDWINDOWSTYLES WS_SIZEBOX | WS_SYSMENU | WS_CLIPCHILDREN | WS_CAPTION | WS_MAXIMIZEBOX

// Global variables and strings.
HINSTANCE           hInst;
const TCHAR         WindowClassName[]= TEXT("MagnifierWindow");
const TCHAR         WindowTitle[]= TEXT("Screen Magnifier Sample");
const UINT          timerInterval = 16; // close to the refresh rate @60hz
HWND                hwndMag;
HWND                hwndHost;
RECT                magWindowRect;
RECT                hostWindowRect;

// Forward declarations.
ATOM                RegisterHostWindowClass(HINSTANCE hInstance);
BOOL                SetupMagnifier(HINSTANCE hinst);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void CALLBACK       UpdateMagWindow(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
void                GoFullScreen();
void                GoPartialScreen();
BOOL                isFullScreen = FALSE;

//
// FUNCTION: WinMain()
//
// PURPOSE: Entry point for the application.
//
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE /*hPrevInstance*/,
                     LPSTR     /*lpCmdLine*/,
                     int       nCmdShow)
{
    if (FALSE == MagInitialize())
    {
        return 0;
    }
    if (FALSE == SetupMagnifier(hInstance))
    {
        return 0;
    }

    ShowWindow(hwndHost, nCmdShow);
    UpdateWindow(hwndHost);

    // Create a timer to update the control.
    UINT_PTR timerId = SetTimer(hwndHost, 0, timerInterval, UpdateMagWindow);

    // Main message loop.
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Shut down.
    KillTimer(NULL, timerId);
    MagUninitialize();
    return (int) msg.wParam;
}

//
// FUNCTION: HostWndProc()
//
// PURPOSE: Window procedure for the window that hosts the magnifier control.
//
LRESULT CALLBACK HostWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) 
    {
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            if (isFullScreen) 
            {
                GoPartialScreen();
            }
        }
        break;

    case WM_SYSCOMMAND:
        if (GET_SC_WPARAM(wParam) == SC_MAXIMIZE)
        {
            GoFullScreen();
        }
        else
        {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_SIZE:
        if ( hwndMag != NULL )
        {
            GetClientRect(hWnd, &magWindowRect);
            // Resize the control to fill the window.
            SetWindowPos(hwndMag, NULL, 
                magWindowRect.left, magWindowRect.top, magWindowRect.right, magWindowRect.bottom, 0);
        }
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;  
}

//
//  FUNCTION: RegisterHostWindowClass()
//
//  PURPOSE: Registers the window class for the window that contains the magnification control.
//
ATOM RegisterHostWindowClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex = {};

    wcex.cbSize = sizeof(WNDCLASSEX); 
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = HostWndProc;
    wcex.hInstance      = hInstance;
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(1 + COLOR_BTNFACE);
    wcex.lpszClassName  = WindowClassName;

    return RegisterClassEx(&wcex);
}

//
// FUNCTION: SetupMagnifier
//
// PURPOSE: Creates the windows and initializes magnification.
//
BOOL SetupMagnifier(HINSTANCE hinst)
{
    // Set bounds of host window according to screen size.
    hostWindowRect.top = 0;
    hostWindowRect.bottom = GetSystemMetrics(SM_CYSCREEN) / 4;  // top quarter of screen
    hostWindowRect.left = 0;
    hostWindowRect.right = GetSystemMetrics(SM_CXSCREEN);

    // Create the host window.
    RegisterHostWindowClass(hinst);
    hwndHost = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED, 
        WindowClassName, WindowTitle, 
        RESTOREDWINDOWSTYLES,
        0, 0, hostWindowRect.right, hostWindowRect.bottom, NULL, NULL, hInst, NULL);
    if (!hwndHost)
    {
        return FALSE;
    }

    // Make the window opaque.
    SetLayeredWindowAttributes(hwndHost, 0, 255, LWA_ALPHA);

    // Create a magnifier control that fills the client area.
    GetClientRect(hwndHost, &magWindowRect);
    hwndMag = CreateWindow(WC_MAGNIFIER, TEXT("MagnifierWindow"), 
        WS_CHILD | MS_SHOWMAGNIFIEDCURSOR | WS_VISIBLE,
        magWindowRect.left, magWindowRect.top, magWindowRect.right, magWindowRect.bottom, hwndHost, NULL, hInst, NULL );
    if (!hwndMag)
    {
        return FALSE;
    }

    // Set the magnification factor.
    MAGTRANSFORM matrix;
    memset(&matrix, 0, sizeof(matrix));
    matrix.v[0][0] = MAGFACTOR;
    matrix.v[1][1] = MAGFACTOR;
    matrix.v[2][2] = 1.0f;

    BOOL ret = MagSetWindowTransform(hwndMag, &matrix);

    if (ret)
    {
        MAGCOLOREFFECT magEffectInvert = 
        {{ // MagEffectInvert
            { -1.0f,  0.0f,  0.0f,  0.0f,  0.0f },
            {  0.0f, -1.0f,  0.0f,  0.0f,  0.0f },
            {  0.0f,  0.0f, -1.0f,  0.0f,  0.0f },
            {  0.0f,  0.0f,  0.0f,  1.0f,  0.0f },
            {  1.0f,  1.0f,  1.0f,  0.0f,  1.0f } 
        }};

        ret = MagSetColorEffect(hwndMag,&magEffectInvert);
    }

    return ret;  
}


//
// FUNCTION: UpdateMagWindow()
//
// PURPOSE: Sets the source rectangle and updates the window. Called by a timer.
//
void CALLBACK UpdateMagWindow(HWND /*hwnd*/, UINT /*uMsg*/, UINT_PTR /*idEvent*/, DWORD /*dwTime*/)
{
    POINT mousePoint;
    GetCursorPos(&mousePoint);

    int width = (int)((magWindowRect.right - magWindowRect.left) / MAGFACTOR);
    int height = (int)((magWindowRect.bottom - magWindowRect.top) / MAGFACTOR);
    RECT sourceRect;
    sourceRect.left = mousePoint.x - width / 2;
    sourceRect.top = mousePoint.y -  height / 2;

    // Don't scroll outside desktop area.
    if (sourceRect.left < 0)
    {
        sourceRect.left = 0;
    }
    if (sourceRect.left > GetSystemMetrics(SM_CXSCREEN) - width)
    {
        sourceRect.left = GetSystemMetrics(SM_CXSCREEN) - width;
    }
    sourceRect.right = sourceRect.left + width;

    if (sourceRect.top < 0)
    {
        sourceRect.top = 0;
    }
    if (sourceRect.top > GetSystemMetrics(SM_CYSCREEN) - height)
    {
        sourceRect.top = GetSystemMetrics(SM_CYSCREEN) - height;
    }
    sourceRect.bottom = sourceRect.top + height;

    // Set the source rectangle for the magnifier control.
    MagSetWindowSource(hwndMag, sourceRect);

    // Reclaim topmost status, to prevent unmagnified menus from remaining in view. 
    SetWindowPos(hwndHost, HWND_TOPMOST, 0, 0, 0, 0, 
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );

    // Force redraw.
    InvalidateRect(hwndMag, NULL, TRUE);
}


//
// FUNCTION: GoFullScreen()
//
// PURPOSE: Makes the host window full-screen by placing non-client elements outside the display.
//
void GoFullScreen()
{
    isFullScreen = TRUE;
    // The window must be styled as layered for proper rendering. 
    // It is styled as transparent so that it does not capture mouse clicks.
    SetWindowLong(hwndHost, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT);
    // Give the window a system menu so it can be closed on the taskbar.
    SetWindowLong(hwndHost, GWL_STYLE,  WS_CAPTION | WS_SYSMENU);

    // Calculate the span of the display area.
    HDC hDC = GetDC(NULL);
    int xSpan = GetSystemMetrics(SM_CXSCREEN);
    int ySpan = GetSystemMetrics(SM_CYSCREEN);
    ReleaseDC(NULL, hDC);

    // Calculate the size of system elements.
    int xBorder = GetSystemMetrics(SM_CXFRAME);
    int yCaption = GetSystemMetrics(SM_CYCAPTION);
    int yBorder = GetSystemMetrics(SM_CYFRAME);

    // Calculate the window origin and span for full-screen mode.
    int xOrigin = -xBorder;
    int yOrigin = -yBorder - yCaption;
    xSpan += 2 * xBorder;
    ySpan += 2 * yBorder + yCaption;

    SetWindowPos(hwndHost, HWND_TOPMOST, xOrigin, yOrigin, xSpan, ySpan, 
        SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOACTIVATE);
}

//
// FUNCTION: GoPartialScreen()
//
// PURPOSE: Makes the host window resizable and focusable.
//
void GoPartialScreen()
{
    isFullScreen = FALSE;

    SetWindowLong(hwndHost, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_LAYERED);
    SetWindowLong(hwndHost, GWL_STYLE, RESTOREDWINDOWSTYLES);
    SetWindowPos(hwndHost, HWND_TOPMOST, 
        hostWindowRect.left, hostWindowRect.top, hostWindowRect.right, hostWindowRect.bottom, 
        SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOACTIVATE);
}