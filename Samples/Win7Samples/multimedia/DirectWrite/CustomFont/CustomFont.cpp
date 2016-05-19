// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//----------------------------------------------------------------------------

#include "Common.h"
#include "CustomFont.h"
#include "Layout.h"
#include "resource.h"

// Misc. global Variables
HINSTANCE g_instance;
int g_dpiX;
int g_dpiY;

// Factory objects.
IDWriteFactory* g_dwriteFactory = NULL;
ID2D1Factory* g_d2dFactory = NULL;

// Remember the current monitor so we can change the text rendering params if the
// window moves to a different monitor.
HMONITOR g_monitor;

// Render target and associated resources.
ID2D1HwndRenderTarget* g_renderTarget = NULL;
ID2D1SolidColorBrush* g_textBrush = NULL;

// Layout object which contains the text to be rendered.
Layout* g_layout = NULL;

// Forward declarations of functions included in this module.
LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void OnPaint(HWND hwnd);
HRESULT RenderFrame(HWND hwnd);
HRESULT UpdateRenderingParams(HWND hwnd);

int APIENTRY wWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR    lpCmdLine,
    int       nCmdShow
    )
{
    // The Microsoft Security Development Lifecycle recommends that all
    // applications include the following call to ensure that heap corruptions
    // do not go unnoticed and therefore do not introduce opportunities
    // for security exploits.
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    HRESULT hr = S_OK;

    g_instance = hInstance;

    // Get the DPI.
    HDC hdc = GetDC(NULL);
    g_dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
    g_dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(NULL, hdc);

    // Create the factory objects.
    if (SUCCEEDED(hr))
    {
        hr = DWriteCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED, 
                __uuidof(IDWriteFactory), 
                reinterpret_cast<IUnknown**>(&g_dwriteFactory)
                );
    }

    if (SUCCEEDED(hr))
    {
        hr = D2D1CreateFactory(
                D2D1_FACTORY_TYPE_SINGLE_THREADED, 
                __uuidof(ID2D1Factory), 
                NULL, 
                (IID_PPV_ARGS(&g_d2dFactory))
                );
    }

    if (SUCCEEDED(hr))
    {
        g_layout = new(std::nothrow) Layout;

        if (g_layout == NULL)
            hr = E_FAIL;
    }

    ATOM classAtom = 0;
    if (SUCCEEDED(hr))
    {
        // Register the window class.
        WNDCLASSEX wcex     = { sizeof(wcex), 0 };
        wcex.lpfnWndProc    = WindowProc;
        wcex.hInstance      = hInstance;
        wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
        wcex.lpszClassName  = L"DWriteCustomFont";

        classAtom = RegisterClassEx(&wcex);
        if (classAtom == 0)
            hr = HRESULT_FROM_WIN32(GetLastError());
    }

    HWND hwnd = NULL;
    if (SUCCEEDED(hr))
    {
        // Compute a window size that will give us our desired initial client size.
        RECT windowRect = { 0, 0, 7 * g_dpiX, 3 * g_dpiY };
        AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, TRUE);

        // Load the window title.
        wchar_t appTitle[100];
        LoadString(g_instance, IDS_APP_TITLE, appTitle, sizeof(appTitle) / sizeof(appTitle[0]));

        // Create the window.
        hwnd = CreateWindow(
                MAKEINTATOM(classAtom), 
                appTitle,
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT, CW_USEDEFAULT, 
                windowRect.right  - windowRect.left, 
                windowRect.bottom - windowRect.top, 
                NULL, 
                NULL, 
                hInstance, 
                NULL
                );

        if (hwnd == NULL)
            hr = HRESULT_FROM_WIN32(GetLastError());
    }

    MSG msg = {};
    if (SUCCEEDED(hr))
    {
        ShowWindow(hwnd, nCmdShow);
        UpdateWindow(hwnd);

        // Main message loop:
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        hr = static_cast<HRESULT>(msg.wParam);
    }

    delete g_layout; g_layout = NULL;
    SafeRelease(&g_textBrush);
    SafeRelease(&g_renderTarget);
    SafeRelease(&g_d2dFactory);
    SafeRelease(&g_dwriteFactory);

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
        OnPaint(hwnd);
        break;

    case WM_ERASEBKGND:
        return true;

    case WM_SIZE:
        SafeRelease(&g_renderTarget);
        InvalidateRect(hwnd, NULL, TRUE);
        UpdateRenderingParams(hwnd);
        break;

    case WM_MOVE:
        UpdateRenderingParams(hwnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

void OnPaint(HWND hwnd)
{
    // Validate the dirty region of the window before presentation.
    ValidateRect(hwnd, NULL);

    // Render the frame.
    HRESULT hr = RenderFrame(hwnd);

    // If it fails with D2DERR_RECREATE_TARGET then we recreate the render target and try again.
    if (hr == D2DERR_RECREATE_TARGET)
    {
        SafeRelease(&g_renderTarget);
        hr = RenderFrame(hwnd);
    }

    if (FAILED(hr))
        PostQuitMessage(hr);
}

HRESULT RenderFrame(HWND hwnd)
{
    HRESULT hr = S_OK;

    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    float clientWidthInDips = clientRect.right * (96.0f / g_dpiX);

    // Create the render target if we haven't already.
    if (SUCCEEDED(hr) && g_renderTarget == NULL)
    {
        // Make sure we'll also recreate device-dependent resources.
        SafeRelease(&g_textBrush);

        // Create the render target.
        hr = g_d2dFactory->CreateHwndRenderTarget(
                D2D1::RenderTargetProperties(), 
                D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU(clientRect.right, clientRect.bottom)), 
                &g_renderTarget
                );
    }

    // Create the text brush if we haven't already.
    if (SUCCEEDED(hr) && g_textBrush == NULL)
    {
        hr = g_renderTarget->CreateSolidColorBrush(
            D2D1::ColorF(GetSysColor(COLOR_WINDOWTEXT)), 
            D2D1::BrushProperties(), 
            &g_textBrush
            );
    }

    if (SUCCEEDED(hr))
    {
        g_renderTarget->BeginDraw();

        g_renderTarget->Clear(D2D1::ColorF(GetSysColor(COLOR_WINDOW)));

        hr = g_layout->Draw(
                clientWidthInDips,
                g_renderTarget,
                g_textBrush
                );
    }

    if (SUCCEEDED(hr))
    {
        hr = g_renderTarget->EndDraw();
    }

    return hr;
}

// Call this on WM_MOVE and WM_SIZE.
HRESULT UpdateRenderingParams(HWND hwnd)
{
    HRESULT hr = S_OK;

    // Get the current monitor for the window.
    HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONULL); 

    // Only update if monitor is different from last time.
    if (monitor == g_monitor) 
        return hr;

    // An ID2D1HwndRenderTarget is automatically created with the correct text rendering
    // parameters for the specified window. However, if the render target already exists
    // and the window is moved to a different monitor then we need to change the text 
    // rendering parameters.
    if (g_renderTarget != NULL)
    {
        // Create a new rendering params object for this monitor.
        IDWriteRenderingParams* renderingParams = NULL;
        hr = g_dwriteFactory->CreateMonitorRenderingParams(monitor, &renderingParams);

        if (SUCCEEDED(hr))
        {
            // Set the new rendering params and make sure we repaint.
            g_renderTarget->SetTextRenderingParams(renderingParams);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        SafeRelease(&renderingParams);
    }

    // Remember the current monitor so we only set the rendering params when the window
    // moves to a different monitor.
    g_monitor = monitor;

    return hr;
}
