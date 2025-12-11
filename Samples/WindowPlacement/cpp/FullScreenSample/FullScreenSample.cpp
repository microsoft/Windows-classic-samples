
#define USE_VIRTUAL_DESKTOP_APIS
#include "User32Utils.h"
#include "resource.h"

// A FullScreen window is sized to the monitor and doesn't have a caption
// bar or resize borders (in other words, no WS_CAPTION or WS_THICKFRAME).
//
// When exiting FullScreen, users expect the window to move back to its previous 
// position, stored when entering FullScreen. This is similar to Maximize/Minimize, 
// except apps must track this restore position manually; the system does not.
//
// This sample demonstrates entering/exiting FullScreen, using PlacementEx to
// implement this memory. PlacementEx remembers the window position (when
// entering) and uses that position to move the window at a later time (exiting).
PlacementEx fsPlacement;

// The last close position is stored in the registry, using a string that is
// unique to this app.
PCWSTR registryPath = L"SOFTWARE\\Microsoft\\Win32Samples\\FullScreenSample";
PCWSTR lastCloseRegKeyName = L"LastClosePosition";

// Called after creating the window (hidden). This picks a good starting
// position for the window and shows it.
//
//  hwndPrev        Another instance of this app, opened prior to this
//                  instance opening. If this is not null, this window will
//                  launch over the other instance, in the same position but
//                  cascaded (moved down/right a bit to keep both windows visible).
//
//  isRestart       If true, this is a restart (not a normal launch). The
//                  system restarted while this app was running, and we're
//                  being relaunched.
//                  This allows the window to launch minimized, or cloaked (on
//                  a background virtual desktop).
//
void SetInitialPosition(HWND hwnd, HWND hwndPrev, bool isRestart)
{
    PlacementParams pp({ 600, 400 }, registryPath, lastCloseRegKeyName);

    if (isRestart)
    {
        pp.SetIsRestart();
    }
    else if (hwndPrev)
    {
        pp.SetPrevWindow(hwndPrev);
    }

    fsPlacement = pp.PositionAndShow(hwnd);

    // Repaint now that fsPlacement is set (checked for when painting to pick
    // background color).
    InvalidateRect(hwnd, nullptr, true);
}

// Called before destroying the window. This stores the current window placement
// in the registry, as the last close position.
//
// If closed while FullScreen, make sure our stored position is on the window's
// current monitor (but do not refresh it). We want to launch next time as
// FullScreen, with the restore position that we stored when entering FullScreen.
void SaveLastClosePosition(HWND hwnd)
{
    if (fsPlacement.IsFullScreen())
    {
        fsPlacement.MoveToWindowMonitor(hwnd);
    }
    else if (!PlacementEx::GetPlacement(hwnd, &fsPlacement))
    {
        return;
    }

    fsPlacement.StoreInRegistry(
        registryPath,
        lastCloseRegKeyName);
}

// Handle keyboard input.
void OnWmChar(HWND hwnd, WPARAM wParam)
{
    switch (wParam)
    {
    // Enter key toggles FullScreen.
    case VK_RETURN:
        fsPlacement.ToggleFullScreen(hwnd);
        return;

    // Space toggles Maximize.
    // This is not done while FullScreen.
    case VK_SPACE:
        if (!fsPlacement.IsFullScreen())
        {
            ShowWindow(hwnd, IsZoomed(hwnd) ? SW_RESTORE : SW_MAXIMIZE);
        }
        return;

    // M key minimizes the window.
    case 0x4D: // M key
    case 0x6D: // m key
        ShowWindow(hwnd, SW_MINIMIZE);
        return;

    // C key moves the cursor to the center of the window.
    case 0x43: // C key
    case 0x63: // c key
    {
        RECT rc;
        GetWindowRect(hwnd, &rc);
        SetCursorPos(rc.left + ((RECTWIDTH(rc)) / 2), rc.top + ((RECTHEIGHT(rc)) / 2));
        return;
    }

    // Escape key exits.
    case VK_ESCAPE:
        DestroyWindow(hwnd);
        return;
    }
}

void OnWmPaint(HWND hwnd, HDC hdc)
{
    const COLORREF rgbMaize = RGB(255, 203, 5);
    const COLORREF rgbBlue = RGB(0, 39, 76);
    static HBRUSH hbrMaize = CreateSolidBrush(rgbMaize);
    static HBRUSH hbrBlue = CreateSolidBrush(rgbBlue);
    const UINT dpi = GetDpiForWindow(hwnd);

    // Create a font of size 30 (* DPI scale) and store it until DPI changes.
    static HFONT hfont = nullptr;
    static UINT dpiLast = 0;
    if (dpiLast != dpi)
    {
        if (hfont)
        {
            DeleteObject(hfont);
        }

        hfont = CreateFont(MulDiv(30, dpi, 96),
                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                           L"Courier New");

        dpiLast = dpi;
    }
    SelectObject(hdc, hfont);

    // Background in one color, text in the other.
    const bool fFullScreen = fsPlacement.IsFullScreen();
    HBRUSH hbrBackground = fFullScreen ? hbrMaize : hbrBlue;
    COLORREF rgbText = fFullScreen ? rgbBlue : rgbMaize;

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, rgbText);

    RECT rc;
    GetClientRect(hwnd, &rc);

    // Draw background
    FillRect(hdc, &rc, hbrBackground);

    const UINT nudge = MulDiv(30, dpi, 96);

    // Draw text

    rc.top += (3 * nudge);
    rc.left += nudge;

    PCWSTR toggleFullTxt = fFullScreen ?
        L"ENTER to exit Fullscreen" : L"ENTER to enter Fullscreen";
    DrawText(hdc, toggleFullTxt, (int)wcslen(toggleFullTxt), &rc, DT_LEFT);

    if (!fFullScreen)
    {
        rc.top += nudge;
        PCWSTR toggleMaxTxt = IsZoomed(hwnd) ?
            L"SPACE to restore from Maximize" : L"SPACE to Maximize";
        DrawText(hdc, toggleMaxTxt, (int)wcslen(toggleMaxTxt), &rc, DT_LEFT);
    }

    rc.top += nudge;
    PCWSTR minTxt = L"M to minimize";
    DrawText(hdc, minTxt, (int)wcslen(minTxt), &rc, DT_LEFT);

    rc.top += nudge;
    PCWSTR escTxt = L"ESC to close";
    DrawText(hdc, escTxt, (int)wcslen(escTxt), &rc, DT_LEFT);

    rc.top += nudge;
    PCWSTR cTxt = L"C to move cursor to center";
    DrawText(hdc, cTxt, (int)wcslen(cTxt), &rc, DT_LEFT);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CHAR:
        OnWmChar(hwnd, wParam);
        break;

    case WM_SYSCOMMAND:

        // If the window is maximized, FullScreen, and someone is trying to
        // restore the window (for example, Win+Down hotkey), exit FullScreen.
        // (We do NOT want to remain FullScreen but restore from Maximize and
        // move to the restore position...)
        if ((wParam == SC_RESTORE) &&
            IsZoomed(hwnd) &&
            fsPlacement.IsFullScreen())
        {
            fsPlacement.ExitFullScreen(hwnd);
            return 0;
        }
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        OnWmPaint(hwnd, BeginPaint(hwnd, &ps));
        EndPaint(hwnd, &ps);
        break;
    }

    case WM_DPICHANGED:
    {
        RECT* prc = (RECT*)lParam;

        SetWindowPos(hwnd,
                     nullptr,
                     prc->left,
                     prc->top,
                     prc->right - prc->left,
                     prc->bottom - prc->top,
                     SWP_NOZORDER | SWP_NOACTIVATE);
        break;
    }

    case WM_ENDSESSION:
    case WM_DESTROY:
        SaveLastClosePosition(hwnd);
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool InitWindow(HINSTANCE hInst, bool isRestart)
{
    PCWSTR windowTitle = L"FullScreen Sample";
    PCWSTR wndClassName = L"FullScreenSampleWindow";

    WNDCLASSEX wc = { sizeof(wc) };
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = wndClassName;
    wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDC_FULLSCREEN));

    if (!RegisterClassEx(&wc))
    {
        return false;
    }

    HWND hwndPrev = FindWindow(wndClassName, nullptr);

    // Create the window with the default position and not visible.
    HWND hwnd = CreateWindowEx(
        0,
        wndClassName,
        windowTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        nullptr,
        nullptr,
        hInst,
        nullptr);

    if (!hwnd)
    {
        return false;
    }

    SetInitialPosition(hwnd, hwndPrev, isRestart);

    return true;
}

int wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR cmdLine, int)
{
    // If 'u' in the command line, run as DPI Unaware. Otherwise run as
    // Per-Monitor DPI Aware.
    SetThreadDpiAwarenessContext((wcsstr(cmdLine, L"u") != nullptr) ?
        DPI_AWARENESS_CONTEXT_UNAWARE :
        DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    // Initialize COM, needed for virtual desktop APIs (USE_VIRTUAL_DESKTOP_APIS).
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    PCWSTR restartCmdLine = L"restart";
    const bool isRestart = (wcsstr(cmdLine, restartCmdLine) != nullptr);
    RegisterApplicationRestart(restartCmdLine, 0);

    // Create the main window.
    if (!InitWindow(hInst, isRestart))
    {
        MessageBox(NULL,
            L"Failed to create Main Window.",
            L"ERROR", MB_ICONEXCLAMATION | MB_OK);
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
