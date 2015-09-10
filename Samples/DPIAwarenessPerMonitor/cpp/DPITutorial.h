//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) Microsoft Corporation.  All rights reserved.

#pragma once
#include "resource.h"

// DPI Helper Class - functions to manage the state of DPI awareness, and scale values used for window size and rendering.
//
// Scale, GetScale, SetScale
// GetAwareness, SetAwareness
// SetDPIChanged, GetDPIChanged

// Constants for UI size
#define MAX_LOADSTRING     100
#define PADDING            10
#define WINDOW_HEIGHT      400 // Default size of window for a standard display (96 DPI, 100% scaling)
#define WINDOW_WIDTH       600
#define BUTTON_HEIGHT      (WINDOW_HEIGHT / 8)
#define BUTTON_WIDTH       (WINDOW_WIDTH  / 5)
#define BUTTON_FONT_HEIGHT (BUTTON_HEIGHT / 2)

// Forward declarations
ATOM             MyRegisterClass(HINSTANCE hInstance);
BOOL             InitInstance(HINSTANCE, int);
HFONT            CreateScaledFont(int g_nFontHeight);
void             CreateFonts(HWND hWnd);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Rectangle helper functions
int RectHeight(RECT Rect)
{
    return Rect.bottom - Rect.top;
}

int RectWidth(RECT Rect)
{
    return Rect.right - Rect.left;
}

class CDPI
{
public:
    CDPI()
    {
        m_nScaleFactor = 0;
        m_nScaleFactorSDA = 0;
        m_Awareness = PROCESS_DPI_UNAWARE;
    }

    int  Scale(int x)
    {
        // DPI Unaware:  Return the input value with no scaling.
        // These apps are always virtualized to 96 DPI and scaled by the system for the DPI of the monitor where shown.
        if (m_Awareness == PROCESS_DPI_UNAWARE)
        {
            return x;
        }

        // System DPI Aware:  Return the input value scaled by the factor determined by the system DPI when the app was launched.
        // These apps render themselves according to the DPI of the display where they are launched, and they expect that scaling
        // to remain constant for all displays on the system.
        // These apps are scaled up or down when moved to a display with a different DPI from the system DPI.
        if (m_Awareness == PROCESS_SYSTEM_DPI_AWARE)
        {
            return MulDiv(x, m_nScaleFactorSDA, 100);
        }

        // Per-Monitor DPI Aware:  Return the input value scaled by the factor for the display which contains most of the window.
        // These apps render themselves for any DPI, and re-render when the DPI changes (as indicated by the WM_DPICHANGED window message).
        return MulDiv(x, m_nScaleFactor, 100);
    }

    UINT GetScale()
    {
        if (m_Awareness == PROCESS_DPI_UNAWARE)
        {
            return 100;
        }
        
        if (m_Awareness == PROCESS_SYSTEM_DPI_AWARE)
        {
            return m_nScaleFactorSDA;
        }

        return m_nScaleFactor;
    }

    void SetScale(__in UINT iDPI)
    {
        m_nScaleFactor = MulDiv(iDPI, 100, 96);
        if (m_nScaleFactorSDA == 0)
        {
            m_nScaleFactorSDA = m_nScaleFactor;  // Save the first scale factor, which is all that SDA apps know about
        }
        return;
    }

    PROCESS_DPI_AWARENESS GetAwareness()
    {
        HANDLE hProcess;
        hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, GetCurrentProcessId());
        GetProcessDpiAwareness(hProcess, &m_Awareness);
        return m_Awareness;
    }

    void SetAwareness(PROCESS_DPI_AWARENESS awareness)
    {
        HRESULT hr = E_FAIL;
        hr = SetProcessDpiAwareness(awareness);
        if (hr == S_OK)
        {
            m_Awareness = awareness;
        }
        else
        {
            MessageBox(NULL, (LPCWSTR) L"SetProcessDpiAwareness Error", (LPCWSTR) L"Error", MB_OK);
        }
        return;
    }

    // Scale rectangle from raw pixels to relative pixels.
    void ScaleRect(__inout RECT *pRect)
    {
        pRect->left = Scale(pRect->left);
        pRect->right = Scale(pRect->right);
        pRect->top = Scale(pRect->top);
        pRect->bottom = Scale(pRect->bottom);
    }

    // Scale Point from raw pixels to relative pixels.
    void ScalePoint(__inout POINT *pPoint)
    {
        pPoint->x = Scale(pPoint->x);
        pPoint->y = Scale(pPoint->y);
    }

private:
    UINT m_nScaleFactor;
    UINT m_nScaleFactorSDA;
    PROCESS_DPI_AWARENESS m_Awareness;
};