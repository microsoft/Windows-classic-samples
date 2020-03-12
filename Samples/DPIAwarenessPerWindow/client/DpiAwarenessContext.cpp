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

#include <windows.h>
#include <WinUser.h>
#include <windowsx.h>
#include <Strsafe.h>
#include <commdlg.h>
#include <stdio.h>
#include "Resource.h"
#include "PluginHeader.h"

#define DEFAULT_PADDING96          20
#define WINDOW_WIDTH96             500
#define WINDOW_HEIGHT96            700
#define DEFAULT_CHAR_BUFFER        150
#define DEFAULT_BUTTON_HEIGHT96    25
#define DEFAULT_BUTTON_WIDTH96     100
#define SAMPLE_STATIC_HEIGHT96     50
#define EXTERNAL_CONTENT_WIDTH96   400
#define EXTERNAL_CONTENT_HEIGHT96  400
#define WINDOWCLASSNAME            L"SetThreadDpiAwarenessContextSample"
#define HWND_NAME_RADIO            L"RADIO"
#define HWND_NAME_CHECKBOX         L"CHECKBOX"
#define HWND_NAME_DIALOG           L"Open a System Dialog"
#define HWND_NAME_STATIC           L"Static"
#define PROP_DPIISOLATION          L"PROP_ISOLATION"

// Globals
HINSTANCE g_hInst;

// Forward declarations of functions included in this code module:
BOOL                GetParentRelativeWindowRect(HWND hWnd, LPRECT lpRect);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    HostDialogProc(HWND, UINT, WPARAM, LPARAM);
LRESULT             DoInitialWindowSetup(HWND hWnd);
UINT                HandleDpiChange(HWND hWnd, WPARAM wParam, LPARAM lParam);
void                CreateSampleWindow(HWND hWndDlg, DPI_AWARENESS_CONTEXT context, BOOL bEnableNonClientDpiScaling, BOOL bChildWindowDpiIsolation);
void                DeleteWindowFont(HWND hWnd);
void                ShowFileOpenDialog(HWND hWnd);
void                UpdateAndDpiScaleChildWindows(HWND hWnd, UINT uDpi);
void                UpdateDpiString(HWND hWnd, UINT uDpi);

struct CreateParams
{
    BOOL bEnableNonClientDpiScaling;
    BOOL bChildWindowDpiIsolation;
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int nCmdShow)
{
    WNDCLASSEXW wcex = {};

    wcex.cbSize = sizeof(wcex);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = WINDOWCLASSNAME;

    RegisterClassExW(&wcex);

    g_hInst = hInstance; // Store instance handle in our global variable

    // Create the host window
    HWND hHostDlg = CreateDialogParamW(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), nullptr, HostDialogProc, 0);
    if (!hHostDlg)
    {
        return FALSE;
    }

    ShowWindow(hHostDlg, nCmdShow);

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}

// Create a string that shows the current thread's DPI context and DPI,
// then send this string to the provided static control
void UpdateDpiString(HWND hWnd, UINT uDpi)
{
    WCHAR result[DEFAULT_CHAR_BUFFER];
    WCHAR awareness[DEFAULT_CHAR_BUFFER];
    WCHAR awarenessContext[DEFAULT_CHAR_BUFFER];

    // Get the DPI awareness of the window from the DPI-awareness context of the thread
    DPI_AWARENESS_CONTEXT dpiAwarenessContext = GetThreadDpiAwarenessContext();
    DPI_AWARENESS dpiAwareness = GetAwarenessFromDpiAwarenessContext(dpiAwarenessContext);

    // Convert DPI awareness to a string
    switch (dpiAwareness)
    {
        case DPI_AWARENESS_SYSTEM_AWARE:
            StringCchCopy(awareness, ARRAYSIZE(awareness), L"DPI_AWARENESS_SYSTEM_AWARE");
            StringCchCopy(awarenessContext, ARRAYSIZE(awarenessContext), L"DPI_AWARENESS_CONTEXT_SYSTEM_AWARE");

            break;
        case DPI_AWARENESS_PER_MONITOR_AWARE:
            StringCchCopy(awareness, ARRAYSIZE(awareness), L"DPI_AWARENESS_PER_MONITOR_AWARE");
            if (AreDpiAwarenessContextsEqual(dpiAwarenessContext, DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
            {
                StringCchCopy(awarenessContext, ARRAYSIZE(awarenessContext), L"DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2");
            }
            else
            {
                StringCchCopy(awarenessContext, ARRAYSIZE(awarenessContext), L"DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE");
            }
            break;
        case DPI_AWARENESS_UNAWARE:
        // intentional fallthrough
        default:
            StringCchCopy(awareness, ARRAYSIZE(awareness), L"DPI_AWARENESS_UNAWARE");
            StringCchCopy(awarenessContext, ARRAYSIZE(awarenessContext), L"DPI_AWARENESS_CONTEXT_UNAWARE");
    }

    StringCchPrintf(result, ARRAYSIZE(result), L"DPI Awareness: %s\rDPI Awareness Context: %s\rGetDpiForWindow(...): %d", awareness, awarenessContext, uDpi);
    SetWindowText(hWnd, result);
}

// Resize and reposition child controls for DPI
void UpdateAndDpiScaleChildWindows(HWND hWnd, UINT uDpi)
{
    HWND hWndRadio;
    HWND hWndDialog;

    // Resize the static control
    UINT uPadding = MulDiv(DEFAULT_PADDING96, uDpi, 96);
    RECT rcClient = {};
    GetClientRect(hWnd, &rcClient);

    // Size and position the static control
    HWND hWndStatic = FindWindowEx(hWnd, nullptr, L"STATIC", nullptr);
    if (hWndStatic == nullptr)
    {
        return;
    }
    UINT uWidth = (rcClient.right - rcClient.left) - 2 * uPadding;
    UINT uHeight = MulDiv(SAMPLE_STATIC_HEIGHT96, uDpi, 96);
    SetWindowPos(
        hWndStatic,
        nullptr,
        uPadding,
        uPadding,
        uWidth,
        uHeight,
        SWP_NOZORDER | SWP_NOACTIVATE);

    UpdateDpiString(hWndStatic, uDpi);

    // Size and position the checkbox
    HWND hWndCheckbox = FindWindowEx(hWnd, nullptr, L"BUTTON", HWND_NAME_CHECKBOX);
    if (hWndCheckbox == nullptr)
    {
        return;
    }
    GetParentRelativeWindowRect(hWndStatic, &rcClient);
    SetWindowPos(
        hWndCheckbox,
        nullptr,
        uPadding,
        rcClient.bottom + uPadding,
        MulDiv(DEFAULT_BUTTON_WIDTH96, uDpi, 96),
        MulDiv(DEFAULT_BUTTON_HEIGHT96, uDpi, 96), SWP_NOZORDER | SWP_NOACTIVATE);

    // Size and position the radio button
    hWndRadio = FindWindowEx(hWnd, nullptr, L"BUTTON", HWND_NAME_RADIO);
    if (hWndCheckbox == nullptr)
    {
        return;
    }
    GetParentRelativeWindowRect(hWndCheckbox, &rcClient);
    SetWindowPos(hWndRadio, nullptr, rcClient.right + uPadding, rcClient.top,
        MulDiv(DEFAULT_BUTTON_WIDTH96, uDpi, 96),
        MulDiv(DEFAULT_BUTTON_HEIGHT96, uDpi, 96),
        SWP_NOZORDER | SWP_NOACTIVATE);

    // Size and position the dialog button
    hWndDialog = FindWindowEx(hWnd, nullptr, L"BUTTON", HWND_NAME_DIALOG);
    GetParentRelativeWindowRect(hWndCheckbox, &rcClient);
    SetWindowPos(hWndDialog, nullptr, uPadding, rcClient.bottom + uPadding,
        MulDiv(DEFAULT_BUTTON_WIDTH96 * 2, uDpi, 96), // Make this one twice as wide as the others
        MulDiv(DEFAULT_BUTTON_HEIGHT96, uDpi, 96),
        SWP_NOZORDER | SWP_NOACTIVATE);

    // Size and position the external content HWND
    HWND hWndExternal = FindWindowEx(hWnd, nullptr, PLUGINWINDOWCLASSNAME, HWND_NAME_EXTERNAL);
    GetParentRelativeWindowRect(hWndDialog, &rcClient);
    SetWindowPos(hWndExternal, hWndDialog, uPadding, rcClient.bottom + uPadding,
        MulDiv(EXTERNAL_CONTENT_WIDTH96, uDpi, 96),
        MulDiv(EXTERNAL_CONTENT_HEIGHT96, uDpi, 96),
        SWP_NOZORDER | SWP_NOACTIVATE);

    // Send a new font to all child controls (the 'plugin' content is subclassed to ignore WM_SETFONT)
    auto hFontOld = GetWindowFont(hWndStatic);
    LOGFONT lfText = {};
    SystemParametersInfoForDpi(SPI_GETICONTITLELOGFONT, sizeof(lfText), &lfText, FALSE, uDpi);
    HFONT hFontNew = CreateFontIndirect(&lfText);
    if (hFontNew)
    {
        DeleteObject(hFontOld);
        EnumChildWindows(hWnd, [](HWND hWnd, LPARAM lParam) -> BOOL
        {
            SendMessage(hWnd, WM_SETFONT, (WPARAM)lParam, MAKELPARAM(TRUE, 0));
            return TRUE;
        }, (LPARAM)hFontNew);
    }
}

BOOL GetParentRelativeWindowRect(HWND hWnd, RECT* childBounds)
{
    if (!GetWindowRect(hWnd, childBounds))
    {
        return FALSE;
    }
    MapWindowRect(HWND_DESKTOP, GetAncestor(hWnd, GA_PARENT), childBounds);

    return TRUE;
}

// Perform initial Window setup and DPI scaling when the window is created
LRESULT DoInitialWindowSetup(HWND hWnd)
{
    // Resize the window to account for DPI. The window might have been created
    // on a monitor that has > 96 DPI. Windows does not send a window a DPI change
    // when it is created, even if it is created on a monitor with a DPI > 96
    RECT rcWindow = {};
    UINT uDpi = 96;

    // Determine the DPI to use, according to the DPI awareness mode
    DPI_AWARENESS dpiAwareness = GetAwarenessFromDpiAwarenessContext(GetThreadDpiAwarenessContext());
    switch (dpiAwareness)
    {
        // Scale the window to the system DPI
        case DPI_AWARENESS_SYSTEM_AWARE:
            uDpi = GetDpiForSystem();
            break;

        // Scale the window to the monitor DPI
        case DPI_AWARENESS_PER_MONITOR_AWARE:
            uDpi = GetDpiForWindow(hWnd);
            break;
    }

    GetWindowRect(hWnd, &rcWindow);
    rcWindow.right = rcWindow.left + MulDiv(WINDOW_WIDTH96, uDpi, 96);
    rcWindow.bottom = rcWindow.top + MulDiv(WINDOW_HEIGHT96, uDpi, 96);
    SetWindowPos(hWnd, nullptr, rcWindow.right, rcWindow.top, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, SWP_NOZORDER | SWP_NOACTIVATE);

    // Create a static control for use displaying DPI-related information.
    // Initially the static control will not be sized, but we will next DPI
    // scale it with a helper function.
    HWND hWndStatic = CreateWindowExW(WS_EX_LEFT, L"STATIC", HWND_NAME_STATIC, SS_LEFT | WS_CHILD | WS_VISIBLE,
        0, 0, 0, 0, hWnd, nullptr, g_hInst, nullptr);
    if (hWndStatic == nullptr)
    {
        return -1;
    }

    // Create some buttons
    HWND hWndCheckbox = CreateWindow(L"BUTTON", HWND_NAME_CHECKBOX, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_CHECKBOX, 0, 0, 0, 0, hWnd, nullptr, g_hInst, nullptr);
    HWND hWndRadio = CreateWindow(L"BUTTON", HWND_NAME_RADIO, BS_PUSHBUTTON | BS_TEXT | BS_DEFPUSHBUTTON | BS_USERBUTTON | BS_AUTORADIOBUTTON | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, 0, 0, 0, 0, hWnd, nullptr, g_hInst, nullptr);
    HWND hWndDialog = CreateWindow(L"BUTTON", HWND_NAME_DIALOG, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hWnd, (HMENU)IDM_SHOWDIALOG, g_hInst, nullptr);

    // Load an HWND from an external source (a DLL in this example)
    //
    // HWNDs from external sources might not support Per-Monitor V2 awareness. Hosting HWNDs that
    // don't support the same DPI awareness mode as their host can lead to rendering problems.
    // When child-HWND DPI isolation is enabled, Windows will try to let that HWND run in its native
    // DPI scaling mode (which might or might not have been defined explicitly). 

    // First, determine if we are in the correct mode to use this feature
    BOOL bDpiIsolation = PtrToInt(GetProp(hWnd, PROP_DPIISOLATION));

    DPI_AWARENESS_CONTEXT previousDpiContext = {};
    DPI_HOSTING_BEHAVIOR previousDpiHostingBehavior = {};

    if (bDpiIsolation)
    {
        previousDpiHostingBehavior = SetThreadDpiHostingBehavior(DPI_HOSTING_BEHAVIOR_MIXED);

        // For this example, we'll have the external content run with System-DPI awareness
		previousDpiContext = SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);
    }

    HWND hWndExternal = PlugInDll::PlugInDll::CreateContentHwnd(g_hInst, EXTERNAL_CONTENT_WIDTH96, EXTERNAL_CONTENT_HEIGHT96);

    // Return the thread context and hosting behavior to its previous value, if using DPI-isolation
	if (bDpiIsolation)
	{
		SetThreadDpiAwarenessContext(previousDpiContext);
		SetThreadDpiHostingBehavior(previousDpiHostingBehavior);
	}

    // After the external content HWND was create with a system-DPI awareness context, reparent it
    HWND hWndResult = SetParent(hWndExternal, hWnd);

    // DPI scale child-windows
    UpdateAndDpiScaleChildWindows(hWnd, uDpi);

    return 0;
}


// DPI Change handler. on WM_DPICHANGE resize the window and
// then call a function to redo layout for the child controls
UINT HandleDpiChange(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    HWND hWndStatic = FindWindowEx(hWnd, nullptr, L"STATIC", nullptr);

    if (hWndStatic != nullptr)
    {
        UINT uDpi = HIWORD(wParam);

        // Resize the window
        auto lprcNewScale = reinterpret_cast<RECT*>(lParam);

        SetWindowPos(hWnd, nullptr, lprcNewScale->left, lprcNewScale->top,
            lprcNewScale->right - lprcNewScale->left, lprcNewScale->bottom - lprcNewScale->top,
            SWP_NOZORDER | SWP_NOACTIVATE);

        // Redo layout of the child controls
        UpdateAndDpiScaleChildWindows(hWnd, uDpi);
    }

    return 0;
}

// Create the sample window and set its initial size, based off of the
// DPI awareness mode that it's running under
void CreateSampleWindow(HWND hWndDlg, DPI_AWARENESS_CONTEXT context, BOOL bEnableNonClientDpiScaling, BOOL bChildWindowDpiIsolation)
{
    // Store the current thread's DPI-awareness context
    DPI_AWARENESS_CONTEXT previousDpiContext = SetThreadDpiAwarenessContext(context);

    // Create the window. Initially create it using unscaled (96 DPI)
    // sizes. We'll resize the window after it's created

    CreateParams createParams;
    createParams.bEnableNonClientDpiScaling = bEnableNonClientDpiScaling;
    createParams.bChildWindowDpiIsolation = bChildWindowDpiIsolation;

    // Windows 10 (1803) supports child-HWND DPI-mode isolation. This enables
    // child HWNDs to run in DPI-scaling modes that are isolated from that of 
    // their parent (or host) HWND. Without child-HWND DPI isolation, all HWNDs 
    // in an HWND tree must have the same DPI-scaling mode.
	DPI_HOSTING_BEHAVIOR previousDpiHostingBehavior = {};
	if (bChildWindowDpiIsolation)
	{
		previousDpiHostingBehavior = SetThreadDpiHostingBehavior(DPI_HOSTING_BEHAVIOR_MIXED);
	}

    HWND hWnd = CreateWindowExW(0L, WINDOWCLASSNAME, L"", WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
        CW_USEDEFAULT, 0, WINDOW_WIDTH96, WINDOW_HEIGHT96, hWndDlg, LoadMenuW(g_hInst, MAKEINTRESOURCEW(IDC_MAINMENU)),
        g_hInst, &createParams);

    ShowWindow(hWnd, SW_SHOWNORMAL);

    // Restore the current thread's DPI awareness context
    SetThreadDpiAwarenessContext(previousDpiContext);

	// Restore the current thread DPI hosting behavior, if we changed it.
	if (bChildWindowDpiIsolation)
	{
		SetThreadDpiHostingBehavior(previousDpiHostingBehavior);
	}
}

// The window procedure for the sample windows
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_NCCREATE:
        {
            // Enable per-monitor DPI scaling for caption, menu, and top-level
            // scroll bars.
            //
            // Non-client area (scroll bars, caption bar, etc.) does not DPI scale
            // automatically on Windows 8.1. In Windows 10 (1607) support was added
            // for this via a call to EnableNonClientDpiScaling. Windows 10 (1703)
            // supports this automatically when the DPI_AWARENESS_CONTEXT is
            // DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2.
            //
            // Here we are detecting if a BOOL was set to enable non-client DPI scaling
            // via the call to CreateWindow that resulted in this window. Doing this
            // detection is only necessary in the context of this sample.
            auto createStruct = reinterpret_cast<const CREATESTRUCT*>(lParam);
            auto createParams = static_cast<const CreateParams*>(createStruct->lpCreateParams);
            if (createParams->bEnableNonClientDpiScaling)
            {
                EnableNonClientDpiScaling(hWnd);
            }

            // Store a flag on the window to note that it'll run its child in a different awareness
			if (createParams->bChildWindowDpiIsolation)
			{
				SetProp(hWnd, PROP_DPIISOLATION, (HANDLE)TRUE);
			}

            return DefWindowProc(hWnd, message, wParam, lParam);
        }

        // Set static text background to white.
        case WM_CTLCOLORSTATIC:
            return (INT_PTR)GetStockBrush(WHITE_BRUSH);

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

        case WM_CLOSE:
        {
            DestroyWindow(hWnd);
            return 0;
        }

        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
                case IDM_SHOWDIALOG:
                    ShowFileOpenDialog(hWnd);
                    return 0;
                    break;

                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }

        case WM_DESTROY:
        {
            DeleteWindowFont(hWnd);

            return 0;
        }
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

void ShowFileOpenDialog(HWND hWnd)
{
    OPENFILENAME ofn{ sizeof(ofn) };       // common dialog box structure
    WCHAR szFile[MAX_PATH]{};      // buffer for file name

    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"All\0*.*\0Text\0*.TXT\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // Display the Open dialog box.
    GetOpenFileName(&ofn);
}

// The dialog procedure for the sample host window
LRESULT CALLBACK HostDialogProc(HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSTATIC:
        return (INT_PTR)GetStockBrush(WHITE_BRUSH);

    case WM_INITDIALOG:
    {
        const PCWSTR appDescription =
        {
            L"This sample app lets you create windows with different DPI Awareness modes so "
            L"that you can observe how Win32 windows behave under these modes. "
            L"Each window will show different behaviors depending on the mode (will be blurry or "
            L"crisp, non-client area will scale differently, etc.)."
            L"\r\n\r\n"
            L"The best way to observe these differences is to move each window to a display with a "
            L"different display scaling (DPI) value. On single-display devices you can simulate "
            L"this by changing the display scaling value of your display (the \"Change the size "
            L"of text, apps, and other items\" setting in the Display settings page of the Settings "
            L"app, as of Windows 10, 1703). Make these settings changes while the app is still "
            L"running to observe the different DPI-scaling behavior."
        };
        SetDlgItemText(hWndDlg, IDC_EDIT1, appDescription);
        return 0;
    }
    break;
        case WM_COMMAND:
        {
            DPI_AWARENESS_CONTEXT context = nullptr;
            BOOL bNonClientScaling = false;
			BOOL bChildWindowDpiIsolation = false;
            switch (LOWORD(wParam))
            {
                case IDC_BUTTON_UNAWARE:
                    context = DPI_AWARENESS_CONTEXT_UNAWARE;
                    break;
                case IDC_BUTTON_SYSTEM:
                    context = DPI_AWARENESS_CONTEXT_SYSTEM_AWARE;
                    break;
                case IDC_BUTTON_81:
                    context = DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE;
                    break;
                case IDC_BUTTON_1607:
                    context = DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE;
                    bNonClientScaling = true;
                    break;
                case IDC_BUTTON_1703:
                    context = DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2;
                    break;
                case IDC_BUTTON_1803:
                    context = DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2;
                    bChildWindowDpiIsolation = true;
                    break;
                case IDM_EXIT:
                    DestroyWindow(hWndDlg);
                    return 0;
            }

            if (context != nullptr)
            {
                CreateSampleWindow(hWndDlg, context, bNonClientScaling, bChildWindowDpiIsolation);
            }
            return TRUE;

        }

        case WM_CLOSE:
            DestroyWindow(hWndDlg);
            return 0;
            break;

        case WM_DESTROY:
        {
            DeleteWindowFont(hWndDlg);
            PostQuitMessage(0);
            return FALSE;
        }
    }
    return FALSE;
}

// Find the child static control, get the font for the control, then
// delete it
void DeleteWindowFont(HWND hWnd)
{
    HWND hWndStatic = GetWindow(hWnd, GW_CHILD);
    if (hWndStatic == nullptr)
    {
        return;
    }

    // Get a handle to the font
    HFONT hFont = GetWindowFont(hWndStatic);
    if (hFont == nullptr)
    {
        return;
    }

    SetWindowFont(hWndStatic, nullptr, FALSE);
    DeleteObject(hFont);
}