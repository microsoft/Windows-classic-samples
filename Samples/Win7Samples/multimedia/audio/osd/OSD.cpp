// Abstract:    Defines the entry point for the application.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
// --------------------------------------------------------------------------------

#include "stdafx.h"
#include "OSD.h"
#include "endpointMonitor.h"
#include <new>

// Global Variables:
HINSTANCE g_hInstance = NULL;
wchar_t const g_szWindowClass[] = L"csl_osd";

CVolumeMonitor*     g_pVolumeMonitor = NULL;
HWND                g_hwndOSD = NULL;
BOOL                g_bDblBuffered = FALSE;
VOLUME_INFO         g_currentVolume = {0};

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//
//  DPI Scaling logic - helper functions to allow us to respect DPI settings.
//
float g_fDPIScale = 1.0f;

void InitializeDPIScale(HWND hWnd)
{
    //
    //  Create the font to be used in the OSD, making sure that
    //  the font size is scaled system DPI setting.
    //
    HDC hdc = GetDC(hWnd);
    g_fDPIScale = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
    ReleaseDC(hWnd, hdc);
}

int DPIScale(int iValue)
{
    return static_cast<int>(iValue * g_fDPIScale);
}

//  Entry to the app
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
    g_hInstance = hInstance;

    //  Mark that this process is DPI aware.
    SetProcessDPIAware();

    // Init COM and double-buffered painting
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        hr = BufferedPaintInit();
        g_bDblBuffered = SUCCEEDED(hr);

        // Init volume monitor
        g_pVolumeMonitor = new (std::nothrow) CVolumeMonitor();
        if (g_pVolumeMonitor)
        {
            hr = g_pVolumeMonitor->Initialize();
            if (SUCCEEDED(hr))
            {
                // Get initial volume level so that we can figure out a good window size
                g_pVolumeMonitor->GetLevelInfo(&g_currentVolume);

                WNDCLASSEX wcex = { sizeof(wcex) };
                wcex.style          = CS_HREDRAW | CS_VREDRAW;
                wcex.lpfnWndProc    = WndProc;
                wcex.hInstance      = g_hInstance;
                wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
                wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
                wcex.lpszClassName  = g_szWindowClass;

                RegisterClassEx(&wcex);

                // Create the (only) window
                DWORD const dwStyle = WS_POPUP;     // no border or title bar
                DWORD const dwStyleEx = WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_NOACTIVATE;   // transparent, topmost, with no taskbar item
                g_hwndOSD = CreateWindowEx(dwStyleEx, g_szWindowClass, NULL, dwStyle, 0, 0, 0, 0, NULL, NULL, g_hInstance, NULL);
                if (g_hwndOSD)
                {
                    // Hide the window
                    ShowWindow(g_hwndOSD, SW_HIDE);

                    // Main message loop
                    MSG msg;
                    while (GetMessage(&msg, NULL, 0, 0))
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }

                if (g_bDblBuffered)
                    BufferedPaintUnInit();

                g_pVolumeMonitor->Dispose();
                g_pVolumeMonitor->Release();
            }
        }
        CoUninitialize();
    }

    return 0;
}

// ----------------------------------------------------------------------
//  Windows proc for the one and only window in this app.
//
// ----------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HBRUSH   hbrLit = NULL;
    static HBRUSH   hbrUnlit = NULL;
    static HFONT    hFont = NULL;
    static UINT_PTR nTimerId = 101;

    switch (message)
    {
        case WM_CREATE:
        {
            // Make BLACK the transparency color
            SetLayeredWindowAttributes(hWnd, RGB(0,0,0), 0, LWA_COLORKEY);

            // Initialize the DPI scalar.
            InitializeDPIScale(hWnd);

            // Create brushes and font that will be used in WM_PAINT
            hbrLit = CreateSolidBrush(RGB(0,128,255));
            hbrUnlit = CreateSolidBrush(RGB(0,64,128));
            hFont = CreateFont(DPIScale(64), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, 0, 0, 0, 0, 0, L"Segoe UI");

            // Position at the center of the primary monitor
            POINT const ptZeroZero = {};
            HMONITOR hMonitor = MonitorFromPoint(ptZeroZero, MONITOR_DEFAULTTOPRIMARY);
            MONITORINFO mi = {sizeof(mi)};
            GetMonitorInfo(hMonitor, &mi);

            SIZE const size = { g_currentVolume.cSteps * DPIScale(10), DPIScale(60) };

            POINT const pt = 
            {
                mi.rcMonitor.left + (mi.rcMonitor.left + mi.rcMonitor.right - size.cx) / 2, 
                mi.rcMonitor.top + (mi.rcMonitor.top + mi.rcMonitor.bottom - size.cy) / 2
            };

            SetWindowPos(hWnd, HWND_TOPMOST, pt.x, pt.y, size.cx, size.cy, SWP_SHOWWINDOW);

            break;
        }

        case WM_DESTROY:
        {
            DeleteObject(hbrLit);
            DeleteObject(hbrUnlit);
            DeleteObject(hFont);

            PostQuitMessage(0);
            return 0;
        }

        case WM_ERASEBKGND:
        {
            // Don't do any erasing here.  It's done in WM_PAINT to avoid flicker.
            return 1;
        }

        case WM_VOLUMECHANGE:
        {
            // get the new volume level
            g_pVolumeMonitor->GetLevelInfo(&g_currentVolume);

            // make window visible for 2 seconds
            ShowWindow(hWnd, SW_SHOW);
            InvalidateRect(hWnd, NULL, TRUE);

            nTimerId = SetTimer(hWnd, 101, 2000, NULL);

            return 0;
        }

        case WM_ENDPOINTCHANGE:
        {
            g_pVolumeMonitor->ChangeEndpoint();
            return 0;
        }

        case WM_TIMER:
        {
            // make the window go away
            ShowWindow(hWnd, SW_HIDE);
            KillTimer(hWnd, nTimerId);

            return 0;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT     ps;
            HPAINTBUFFER    hBufferedPaint = NULL;
            RECT            rc;

            GetClientRect(hWnd, &rc);
            HDC hdc = BeginPaint(hWnd, &ps);

            if (g_bDblBuffered)
            {
                // Get doublebuffered DC
                HDC hdcMem;
                hBufferedPaint = BeginBufferedPaint(hdc, &rc, BPBF_COMPOSITED, NULL, &hdcMem);
                if (hBufferedPaint)
                {
                    hdc = hdcMem;
                }
            }

            // black background (transparency color)
            FillRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));

            // Draw LEDs
            for (UINT i = 0; i < (g_currentVolume.cSteps-1); i++)
            {
                RECT const rcLed = { DPIScale(i * 10), DPIScale(10), DPIScale(i * 10 + 8), rc.bottom-DPIScale(15) };

                if ((i < g_currentVolume.nStep) && (!g_currentVolume.bMuted))
                    FillRect(hdc, &rcLed, hbrLit);
                else
                    FillRect(hdc, &rcLed, hbrUnlit);
            }

            if (g_currentVolume.bMuted)
            {
                HGDIOBJ hof = SelectObject(hdc, hFont);
                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, RGB(255, 64, 64));
                RECT rcText = rc;
                rcText.bottom -= DPIScale(11);
                DrawText(hdc, L"MUTED", -1, &rcText, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
                SelectObject(hdc, hof);
            }

            if (hBufferedPaint)
            {
                // end painting
                BufferedPaintMakeOpaque(hBufferedPaint, NULL);
                EndBufferedPaint(hBufferedPaint, TRUE);
            }

            EndPaint(hWnd, &ps);
            return 0;
        }
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}
