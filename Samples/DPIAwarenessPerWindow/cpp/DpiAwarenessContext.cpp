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
#include <commdlg.h>

#define DEFAULT_PADDING96        20
#define WINDOW_WIDTH96           600
#define WINDOW_HEIGHT96          350
#define DEFAULT_CHAR_BUFFER      150
#define DIALOG_EDIT_BUFFER       728
#define DEFAULT_BUTTON_HEIGHT96  25
#define DEFAULT_BUTTON_WIDTH96   100
#define SAMPLE_STATIC_HEIGHT96   50
#define WINDOWCLASSNAME          L"SetThreadDpiAwarenessContextSample"
#define HWND_NAME_RADIO          L"RADIO"
#define HWND_NAME_CHECKBOX       L"CHECKBOX"
#define HWND_NAME_DIALOG         L"Open a System Dialog"

// Global Variables:
HINSTANCE g_hInst;

// Forward declarations of functions included in this code module:
BOOL                GetWindowRectEx(HWND hWnd, LPRECT lpRect);
BOOL                InitInstance(HINSTANCE, int);
BOOL CALLBACK       UpdateFont(HWND hWnd, LPARAM lParam);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    HostDialogProc(HWND, UINT, WPARAM, LPARAM);
LRESULT             DoInitialWindowSetup(HWND hWnd);
UINT                HandleDpiChange(HWND hWnd, WPARAM wParam, LPARAM lParam);
VOID                CreateSampleWindow(HWND hWndDlg, DPI_AWARENESS_CONTEXT context, BOOL bEnableNonClientDpiScaling);
VOID                DeleteFont(HWND hWnd);
VOID                ShowFileOpenDialog(HWND hWnd);
VOID                UpdateAndDpiScaleChildWindows(HWND hWnd, UINT uDpi);
VOID                UpdateDpiString(HWND hWnd, UINT uDpi);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE,
                      _In_ LPWSTR,
                      _In_ int       nCmdShow)
{
    // Register the sample window class
    WNDCLASSEXW wcex = {};

    wcex.cbSize = sizeof(wcex);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = WINDOWCLASSNAME;

    RegisterClassExW(&wcex);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
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

    // Create the host window
    HWND hHostDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, HostDialogProc);

    if (!hHostDlg)
    {
        return FALSE;
    }

    ShowWindow(hHostDlg, nCmdShow);

    return TRUE;
}

// Create a string that shows the current thread's DPI context and DPI,
// then send this string to the provided static control
VOID UpdateDpiString(HWND hWnd, UINT uDpi)
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


    StringCchPrintf(result, DEFAULT_CHAR_BUFFER, L"DPI Awareness: %s\rDPI Awareness Context: %s\rGetDpiForWindow(...): %d", awareness, awarenessContext, uDpi);
    SetWindowText(hWnd, result);
}

// Resize and reposition child controls for DPI
VOID UpdateAndDpiScaleChildWindows(HWND hWnd, UINT uDpi)
{
    HWND hWndRadio;
    HWND hWndDialog;

    // Resize the static control
    UINT uPadding = MulDiv(DEFAULT_PADDING96, uDpi, 96);
    RECT rcClient = {};
    GetClientRect(hWnd, &rcClient);
    
    // Size and position the static control
    HWND hWndStatic = GetWindow(hWnd, GW_CHILD);
    if (hWndStatic == NULL)
    {
        return;
    }
    UINT uWidth = (rcClient.right - rcClient.left) - 2 * uPadding;
    UINT uHeight = MulDiv(SAMPLE_STATIC_HEIGHT96, uDpi, 96);
    SetWindowPos(
        hWndStatic, 
        NULL, 
        uPadding, 
        uPadding, 
        uWidth, 
        uHeight, 
        SWP_NOZORDER | SWP_NOACTIVATE);

    UpdateDpiString(hWndStatic, uDpi);

    // Size and position the checkbox
    HWND hWndCheckbox = FindWindowEx(hWnd, NULL, L"BUTTON", HWND_NAME_CHECKBOX);
    if (hWndCheckbox == NULL)
    {
        return;
    }
    GetWindowRectEx(hWndStatic, &rcClient);
    SetWindowPos(
        hWndCheckbox, 
        NULL, 
        uPadding, 
        rcClient.bottom + uPadding, 
        MulDiv(DEFAULT_BUTTON_WIDTH96, uDpi, 96), 
        MulDiv(DEFAULT_BUTTON_HEIGHT96, uDpi, 96), SWP_NOZORDER | SWP_NOACTIVATE);

    // Size and position the radio button
    hWndRadio = FindWindowEx(hWnd, NULL, L"BUTTON", HWND_NAME_RADIO);
    if (hWndCheckbox == NULL)
    {
        return;
    }
    GetWindowRectEx(hWndCheckbox, &rcClient);
    SetWindowPos(
        hWndRadio,
        NULL,
        rcClient.right + uPadding,
        rcClient.top,
        MulDiv(DEFAULT_BUTTON_WIDTH96, uDpi, 96),
        MulDiv(DEFAULT_BUTTON_HEIGHT96, uDpi, 96), 
        SWP_NOZORDER | SWP_NOACTIVATE);

    // Size and position the dialog button
    hWndDialog = FindWindowEx(hWnd, NULL, L"BUTTON", HWND_NAME_DIALOG);
    GetWindowRectEx(hWndCheckbox, &rcClient);
    SetWindowPos(
        hWndDialog,
        NULL,
        uPadding,
        rcClient.bottom + uPadding,
        MulDiv(DEFAULT_BUTTON_WIDTH96*2, uDpi, 96),
        MulDiv(DEFAULT_BUTTON_HEIGHT96, uDpi, 96),
        SWP_NOZORDER | SWP_NOACTIVATE);

    // Send a new font to all child controls
    HFONT hFontOld;
    hFontOld = (HFONT)(SendMessage(hWndStatic, WM_GETFONT, NULL, NULL));
    LOGFONT lfText = {};
    SystemParametersInfoForDpi(SPI_GETICONTITLELOGFONT, sizeof(lfText), &lfText, FALSE, uDpi);
    HFONT hFontNew = CreateFontIndirect(&lfText);
    if (hFontNew)
    {
        DeleteObject(hFontOld);
        EnumChildWindows(hWnd, UpdateFont, (LPARAM)hFontNew);
    }    
}

// Callback function that will send a new font to child controls
BOOL CALLBACK UpdateFont(HWND hWnd, LPARAM lParam)
{
    SendMessage(hWnd, WM_SETFONT, (WPARAM)lParam, MAKELPARAM(TRUE, 0));
    return TRUE;
}

// Sets the position of a child HWND rect, in client coordinates,
// relative to the top level window
BOOL GetWindowRectEx(HWND hWnd, LPRECT lpRect)
{
    RECT rcHwnd = {};
    HWND hWndTopLevel;

    if (!GetWindowRect(hWnd, &rcHwnd))
    {
        return FALSE;
    }
    POINT ptChildTopLeft = {};
    POINT ptChildBottomRight = {};
    ptChildTopLeft.x = rcHwnd.left;
    ptChildTopLeft.y = rcHwnd.top;
    ptChildBottomRight.x = rcHwnd.right;
    ptChildBottomRight.y = rcHwnd.bottom;
    hWndTopLevel = GetAncestor(hWnd, GA_ROOT);
    if (!ScreenToClient(hWndTopLevel, &ptChildTopLeft) || !ScreenToClient(hWndTopLevel, &ptChildBottomRight))
    {
        return FALSE;
    }

    lpRect->left = ptChildTopLeft.x;
    lpRect->top = ptChildTopLeft.y;
    lpRect->right = ptChildBottomRight.x;
    lpRect->bottom = ptChildBottomRight.y;

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

    // Determine the DPI to use, accoridng to the DPI awareness mode
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
    SetWindowPos(hWnd, NULL, rcWindow.right, rcWindow.top, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, SWP_NOZORDER | SWP_NOACTIVATE);

    // Create a static control for use displaying DPI-related information. 
    // Initially the static control will not be sized, but we will next DPI
    // scale it with a helper function.
    GetClientRect(hWnd, &rcWindow);
    HWND hWndStatic = CreateWindowEx(WS_EX_LEFT, L"STATIC", NULL, SS_LEFT | WS_CHILD | WS_VISIBLE,
        0,
        0,
        0,
        0,
        hWnd,
        NULL,
        g_hInst,
        NULL);

    if (hWndStatic == NULL)
    {
        return -1;
    }

    // Create some buttons
    HWND hWndCheckbox = CreateWindow(L"BUTTON", HWND_NAME_CHECKBOX, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_CHECKBOX, 0, 0, 0, 0, hWnd, NULL, g_hInst, NULL);
    HWND hWndRadio = CreateWindow(L"BUTTON", HWND_NAME_RADIO, BS_PUSHBUTTON | BS_TEXT | BS_DEFPUSHBUTTON | BS_USERBUTTON | BS_AUTORADIOBUTTON | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, 0, 0, 0, 0, hWnd, NULL, g_hInst, NULL);
    HWND hWndDialog = CreateWindow(L"BUTTON", HWND_NAME_DIALOG, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hWnd, (HMENU)IDM_SHOWDIALOG, g_hInst, NULL);

    // Set a font for the static control, then DPI scale it.
    LOGFONT lfText = {};
    SystemParametersInfoForDpi(SPI_GETICONTITLELOGFONT, sizeof(lfText), &lfText, FALSE, uDpi);
    HFONT hFontNew = CreateFontIndirect(&lfText);
    SendMessage(hWnd, WM_SETFONT, (WPARAM)hFontNew, MAKELPARAM(TRUE, 0));

    // Resize the child controls
    UpdateAndDpiScaleChildWindows(hWnd, uDpi);

    return 0;
}


// DPI Change handler. on WM_DPICHANGE resize the window and
// then call a function to redo layout for the child controls
UINT HandleDpiChange(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    HWND hWndStatic = GetWindow(hWnd, GW_CHILD);
    if (hWndStatic != NULL)
    {
        UINT uDpi = HIWORD(wParam);

        // Resize the window
        RECT *lprcNewScale = (RECT*)lParam;

        SetWindowPos(hWnd,
            NULL,
            lprcNewScale->left,
            lprcNewScale->top,
            lprcNewScale->right - lprcNewScale->left,
            lprcNewScale->bottom - lprcNewScale->top,
            SWP_NOZORDER | SWP_NOACTIVATE);

        // Redo layout of the child controls
        UpdateAndDpiScaleChildWindows(hWnd, uDpi);
    }
    return 0;
}

// Create the sample window and set its initial size, based off of the
// DPI awareness mode that it's running under
VOID CreateSampleWindow(HWND hWndDlg, DPI_AWARENESS_CONTEXT context, BOOL bEnableNonClientDpiScaling)
{
    // Store the current thread's DPI-awareness context
    DPI_AWARENESS_CONTEXT previousDpiContext = SetThreadDpiAwarenessContext(context);

    // Create the window. Initially create it using unscaled (96 DPI)
    // sizes. We'll resize the window when it's created

    HWND hWnd = CreateWindow(
        WINDOWCLASSNAME,
        L"",
        WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
        CW_USEDEFAULT,
        0,
        WINDOW_WIDTH96,
        WINDOW_HEIGHT96,
        hWndDlg,
        (HMENU)LoadMenu(g_hInst, MAKEINTRESOURCEW(IDC_MAINMENU)),
        g_hInst,
        (LPVOID)bEnableNonClientDpiScaling);

    ShowWindow(hWnd, SW_SHOWNORMAL);

    // Restore the current thread's DPI awareness context
    SetThreadDpiAwarenessContext(previousDpiContext);
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
            if((BOOL)(((LPCREATESTRUCT)lParam)->lpCreateParams))
                EnableNonClientDpiScaling(hWnd);

            return DefWindowProc(hWnd, message, wParam, lParam);
        }

        // Set static text background to white.
        case WM_CTLCOLORSTATIC:
            return (INT_PTR)CreateSolidBrush(RGB(255, 255, 255));

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
            DestroyWindow(hWnd);
            return 0;
            break;

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
            DeleteFont(hWnd);
            return 0;
        }
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

VOID ShowFileOpenDialog(HWND hWnd)
{
    OPENFILENAME ofn;       // common dialog box structure
    WCHAR szFile[260];      // buffer for file name
    HANDLE hf;              // file handle

                            // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
    // use the contents of szFile to initialize itself.
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"All\0*.*\0Text\0*.TXT\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // Display the Open dialog box. 

    if (GetOpenFileName(&ofn) == TRUE)
        hf = CreateFile(ofn.lpstrFile,
            GENERIC_READ,
            0,
            (LPSECURITY_ATTRIBUTES)NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            (HANDLE)NULL);
}

// The dialog procedure for the sample host window
LRESULT CALLBACK HostDialogProc(HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {

    // White background
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSTATIC:
        return (INT_PTR)CreateSolidBrush(RGB(255, 255, 255));

    case WM_INITDIALOG:
    {
        WCHAR *appDescription[DIALOG_EDIT_BUFFER] = 
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
        SetDlgItemText(hWndDlg, IDC_EDIT1, *appDescription);
        return 0;
    }
    break;
        case WM_COMMAND:
        {
            DPI_AWARENESS_CONTEXT context = NULL;
            BOOL bNonClientScaling = false;
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
                case IDM_EXIT:
                    DestroyWindow(hWndDlg);
                    return 0;
            }

            if (context != NULL)
            {
                CreateSampleWindow(hWndDlg, context, bNonClientScaling);
            }
            return TRUE;

        }

        case WM_CLOSE:
            DestroyWindow(hWndDlg);
            return 0;
            break;

        case WM_DESTROY:
        {
            DeleteFont(hWndDlg);
            PostQuitMessage(0);
            return FALSE;
        }
    }
    return FALSE;
}


// Find the child static control, get the font for the control, then
// delete it
VOID DeleteFont(HWND hWnd)
{
    HWND hWndStatic = GetWindow(hWnd, GW_CHILD);
    if (hWndStatic == NULL)
    {
        return;
    }
    
    // Get a handle to the font
    HFONT hFont = (HFONT)SendMessage(hWndStatic, WM_GETFONT, 0, 0);
    if (hFont == NULL)
    {
        return;
    }

    SendMessage(hWndStatic, WM_SETFONT, (WPARAM)NULL, 0);
    DeleteObject(hFont);
}