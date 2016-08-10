//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH 
// THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//*********************************************************

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdio.h>
#include "Resource.h"
#include "Strsafe.h"

#define DEFAULT_PADDING96    20
#define WINDOW_WIDTH96       600
#define WINDOW_HEIGHT96      200
#define DEFAULT_CHAR_BUFFER  100
#define WINDOWCLASSNAME      L"SetThreadDpiAwarenessContextSample"

// Global Variables:
HINSTANCE g_hInst;
HFONT     g_hFont;

// Forward declarations of functions included in this code module:
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT             DoInitialWindowSetup(HWND hWnd);
UINT                HandleDpiChange(HWND hWnd, WPARAM wParam, LPARAM lParam);
VOID                CreateDpiUnawareWindow(HWND hWnd);
VOID                DeleteFont(HWND hWnd);
VOID                DpiScaleFont(HWND hWnd, UINT uDpi);
VOID                UpdateAndDpiScaleStaticControl(HWND hWnd, HWND hWndStatic, UINT uDpi);
VOID                UpdateDpiString(HWND hWnd, UINT uDpi);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE,
                      _In_ LPWSTR,
                      _In_ int       nCmdShow)
{
    // Register the window class
    WNDCLASSEXW wcex = {};

    wcex.cbSize = sizeof(wcex);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SETTHREADDPIAWARENESSCONTEXTSAMPLE));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = WINDOWCLASSNAME;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    RegisterClassExW(&wcex);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}

// Save instance handle and create the main window
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    g_hInst = hInstance; // Store instance handle in our global variable
    HMENU hmenu = LoadMenu(hInstance, MAKEINTRESOURCEW(IDC_SETTHREADDPIAWARENESSCONTEXTSAMPLE));
    if (hmenu == nullptr)
    {
        return FALSE;
    }

    // Here we create the primary top-level window but before the window is created it 
    // doesn't have a DPI associated with it, so we don't know what pixel dimensions
    // it should have. We'll create the window with zero width and height and then 
    // resize it for DPI in the WM_CREATE handler in the winproc.
    HWND hWnd = CreateWindowW(
        WINDOWCLASSNAME,
        L"SetThreadDpiAwarenessContext Sample",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        0,
        0,
        0,
        nullptr,
        hmenu,
        hInstance,
        nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

// Replace the font for the static control with a DPI-scaled font
VOID DpiScaleFont(HWND hWnd, UINT uDpi)
{
    LOGFONT lfText = {};
    SystemParametersInfoForDpi(SPI_GETICONTITLELOGFONT, sizeof(lfText), &lfText, FALSE, uDpi);
    HFONT hFontNew = CreateFontIndirect(&lfText);
    if (hFontNew)
    {
        DeleteObject(g_hFont);
        g_hFont = hFontNew;
        SendMessage(hWnd, WM_SETFONT, (WPARAM)hFontNew, MAKELPARAM(TRUE, 0));
    }
}

// Create a string that shows the current thread's DPI context and DPI,
// then send this string to the provided static control
VOID UpdateDpiString(HWND hWnd, UINT uDpi)
{
    WCHAR result[DEFAULT_CHAR_BUFFER];
    WCHAR awareness[DEFAULT_CHAR_BUFFER];

    // Get the DPI awareness of the window, from the DPI-awareness context of the thread
    DPI_AWARENESS_CONTEXT dpiAwarenessContext = GetThreadDpiAwarenessContext();
    DPI_AWARENESS dpiAwareness = GetAwarenessFromDpiAwarenessContext(dpiAwarenessContext);

    // Convert DPI awareness to a string
    switch (dpiAwareness)
    {
    case DPI_AWARENESS_SYSTEM_AWARE:
        StringCchCopy(awareness, ARRAYSIZE(awareness), L"DPI_AWARENESS_SYSTEM_AWARE");
        break;
    case DPI_AWARENESS_PER_MONITOR_AWARE:
        StringCchCopy(awareness, ARRAYSIZE(awareness), L"DPI_AWARENESS_PER_MONITOR_AWARE");
        break;
    case DPI_AWARENESS_UNAWARE:
    // intentional fallthrough
    default:
        StringCchCopy(awareness, ARRAYSIZE(awareness), L"DPI_AWARENESS_UNAWARE");
    }

    StringCchPrintf(result, DEFAULT_CHAR_BUFFER, L"DPI Awareness: %s\rGetDpiForWindow(...): %d", awareness, uDpi);
    SetWindowText(hWnd, result);
}

// Resize the static control for DPI, scale its font and then
// update its string to reflect the parent window's DPI
VOID UpdateAndDpiScaleStaticControl(HWND hWnd, HWND hWndStatic, UINT uDpi)
{
    // Resize the static control
    UINT uDpiScaledPadding = MulDiv(DEFAULT_PADDING96, uDpi, 96);
    UINT uPadding = MulDiv(DEFAULT_PADDING96, uDpi, 96);
    RECT rcClient = {};
    GetClientRect(hWnd, &rcClient);
    UINT uWidth = (rcClient.right - rcClient.left) - 2 * uPadding;
    UINT uHeight = (rcClient.bottom - rcClient.top) - 2 * uPadding;
    SetWindowPos(hWndStatic, nullptr, uPadding, uPadding, uWidth, uHeight, SWP_NOZORDER | SWP_NOACTIVATE);

    DpiScaleFont(hWndStatic, uDpi);
    UpdateDpiString(hWndStatic, uDpi);
}

// Perform initial Window setup and DPI scaling when the window is created:
// 1) Resize the window for DPI
// 2) Create a static control
// 3) Set a font on the static control
// 4) DPI scale the font for the static control and update the
//    text in the static control
LRESULT DoInitialWindowSetup(HWND hWnd)
{
    // Resize the window to account for DPI. The window might have been created
    // on a monitor that has > 96 DPI. In this case, we'll resize the window
    // for the DPI manually
    RECT rcWindow = {};
    UINT uDpi = GetDpiForWindow(hWnd);
    UINT uDpiScaledPadding = MulDiv(DEFAULT_PADDING96, uDpi, 96);
    GetWindowRect(hWnd, &rcWindow);
    rcWindow.right = rcWindow.left + MulDiv(WINDOW_WIDTH96, uDpi, 96);
    rcWindow.bottom = rcWindow.top + MulDiv(WINDOW_HEIGHT96, uDpi, 96);
    SetWindowPos(hWnd, nullptr, rcWindow.right, rcWindow.top, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, SWP_NOZORDER | SWP_NOACTIVATE);

    // Create a static control for use displaying DPI-related information. 
    // Initially the static control will not be sized, but we will immediately scale
    // it with a helper function.
    GetClientRect(hWnd, &rcWindow);
    HWND hWndStatic = CreateWindowEx(WS_EX_LEFT, L"STATIC", nullptr, SS_LEFT | WS_CHILD | WS_VISIBLE,
        0,
        0,
        0,
        0,
        hWnd,
        nullptr,
        g_hInst,
        nullptr);

    if (hWndStatic == nullptr)
    {
        return -1;
    }

    // Set a font for the static control, then DPI scale it.
    LOGFONT lfText = {};
    SystemParametersInfoForDpi(SPI_GETICONTITLELOGFONT, sizeof(lfText), &lfText, FALSE, uDpi);
    HFONT hFontNew = CreateFontIndirect(&lfText);
    SendMessage(hWnd, WM_SETFONT, (WPARAM)hFontNew, MAKELPARAM(TRUE, 0));

    // Resize the child static control
    UpdateAndDpiScaleStaticControl(hWnd, hWndStatic, uDpi);

    return 0;
}

// DPI Change handler
// 1) Update the static control's text to reflect the DPI change
// 2) Resize the top-level window for DPI
// 3) Resize the child static control
// 4) Scale the child control's font for DPI
UINT HandleDpiChange(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    HWND hWndStatic = GetWindow(hWnd, GW_CHILD);
    if (hWndStatic != nullptr)
    {
        UINT uDpi = HIWORD(wParam);

        // Resize the window
        RECT *lprcNewScale = (RECT*)lParam;

        SetWindowPos(hWnd,
            nullptr,
            lprcNewScale->left,
            lprcNewScale->top,
            lprcNewScale->right - lprcNewScale->left,
            lprcNewScale->bottom - lprcNewScale->top,
            SWP_NOZORDER | SWP_NOACTIVATE);

        // Resize the Static control
        HWND hWndStatic = GetWindow(hWnd, GW_CHILD);
        if (hWndStatic != nullptr)
        {
            UpdateAndDpiScaleStaticControl(hWnd, hWndStatic, uDpi);
        }
    }
    return 0;
}

// Create a DPI-unaware instance of the window class. Use SetThreadDpiAwareness to 
// temporarily change the current thread's DPI awareness to DPI_AWARENESS_CONTEXT_UNAWARE 
// before creating the new window. This will mark the new window as DPI unaware which will 
// result in the DWM scaling the window for DPI whenever the DPI != 96
VOID CreateDpiUnawareWindow(HWND hWnd)
{
    // Store the current thread's DPI-awareness context
    DPI_AWARENESS_CONTEXT previousDpiContext = SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_UNAWARE);

    HWND hWndUnaware = CreateWindow(
        L"SetThreadDpiAwarenessContextSample",
        L"DPI Unaware Top-Level Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        0,
        WINDOW_WIDTH96, // Because the DWM will scale this window, we don't need to DPI scale these values ourself
        WINDOW_HEIGHT96,
        nullptr,
        nullptr,
        g_hInst,
        nullptr);
    ShowWindow(hWndUnaware, SW_SHOWNORMAL);

    // Restore the current thread's DPI awareness context
    SetThreadDpiAwarenessContext(previousDpiContext);
}

// The main window procedure.
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_NCCREATE:
        {
            // Enable per-monitor DPI scaling for caption, menu, and top-level
            // scroll bars for top-level windows that are running as per-monitor
            // DPI aware (This will have no effect if the thread's DPI context is
            // not per-monitor-DPI-aware). This API should be called while 
            // processing WM_NCCREATE.
            EnableNonClientDpiScaling(hWnd);
            return TRUE;
        }

        case WM_CREATE:
        {
            return DoInitialWindowSetup(hWnd);
        }

        // On DPI change resize the window, scale the font, and update
        // the DPI-info string
        case WM_DPICHANGED:
        {
            return HandleDpiChange(hWnd, wParam, lParam);
        }

        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
                case IDM_EXIT:
                    DestroyWindow(hWnd);
                    return 0;

                case IDM_DPIUNAWARE:
                    // Create a new top-level window that is running with a DPI-unaware context.
                    // This window will be scaled by the DWM whenever the DPI != 96
                    CreateDpiUnawareWindow(hWnd);

                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }

        case WM_DESTROY:
        {
            DeleteFont(hWnd);
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

// Find the child static control, get the font for the control, then
// delete it
VOID DeleteFont(HWND hWnd)
{
    HWND hWndStatic = GetWindow(hWnd, GW_CHILD);
    if (hWndStatic == nullptr)
    {
        return;
    }
    
    // Get a handle to the font
    HFONT hFont = (HFONT)SendMessage(hWndStatic, WM_GETFONT, 0, 0);
    if (hFont == nullptr)
    {
        return;
    }

    SendMessage(hWndStatic, WM_SETFONT, (WPARAM)nullptr, 0);
    DeleteObject(hFont);
}