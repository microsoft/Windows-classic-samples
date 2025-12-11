
#define USE_VIRTUAL_DESKTOP_APIS
#define USE_WINDOW_ACTION_APIS
#include "User32Utils.h"
#include "resource.h"
#include "windowsx.h"

//
// This sample app uses PlacementEx.h to implement 'remembering positions'. When
// the window closes, its position and other information (a PlacementEx) is
// stored in the registry. When launching, this stored position is used to pick
// an initial position.
//
// This information includes Minimized state and virtual desktop information.
// Typically, this information is unused (the app launches restored/Maximized
// and on the active desktop). But, if the app is restarted, like after a system
// reboot, we use that info to correctly relaunch the window Minimized and on
// its original desktop (if closed in that state).
//

PCWSTR appRegKeyName =          L"SOFTWARE\\Microsoft\\Win32Samples\\SaveRestoreSample";
PCWSTR lastCloseRegKeyName =    L"LastClosePosition";
PCWSTR wndClassName =           L"SaveRestoreSampleWindow";
constexpr SIZE defaultSize =    { 600, 400 };
constexpr SIZE minSize =        { 300, 200 };

// Monitor Hint
//
// When launched from UI (like the Taskbar, desktop icons, file explorer, etc),
// apps receive a 'monitor hint', the monitor of the UI the user interacted
// with, in their `STARTUPINFO`. By default, CreateWindow uses this hint to
// place the window, so by default PlacementEx also uses this hint to override
// any stored position (where the window was last closed). This is generally the
// correct behavior: users will see apps on the monitor they interacted with.
// However, some users prefer to hide taskbars on their non-primary displays and
// may be frustrated that apps always relaunch on their primary display (instead
// of where they last closed).
//
// If `true`: consumes the monitor hint & launches on that monitor.
// If `false`: ignores the monitor hint & launches on monitor it was last-closed on.
bool UseMonitorHint =           true;
PCWSTR UseMonitorHintKeyName =  L"UseMonitorHint";
RECT rcUseMonitorHintTxt =      {};

// Allow Partially Off-Screen
//
// Users can move and resize apps to any positions and dimensions, including
// hard-to-recover ones (e.g. extremely large and in the bottom-right corner).
// In most cases, users don't want apps to restart in those positions (e.g. the
// move was accidental or they forgot they did such an absurd move or they
// relaunch to recover a more sensible state). However, rare apps may
// deliberately want to be saved & relaunched partially off-screen.
//
// If `true`: sets the flag `PlacementFlags::AllowPartiallyOffScreen`, which
// allows a window to relaunch partially off-screen (outside the work area).
// If `false`: the relaunch position is moved as needed to be 100% inside the
// work area.
//
// If the new position is more than 50% outside the work area, PlacementEx
// assumes this was accidental and repositions the window anyway.
bool AllowOffScreen =  true;
PCWSTR AllowOffScreenKeyName = L"AllowPartiallyOffscreen";
RECT rcAllowOffScreenTxt = {};

// Creates the window, picks an initial position, and shows/activates it.
bool CreateMainWindow(HINSTANCE hInstance, PWSTR cmdLine)
{
    // PlacementParams allows us to pick the initial window position.
    // Here we pick the default size (to use the very first time launching on
    // a machine), and a registry key to use to store the last close position,
    // which is used by default when launching. (The app normally launches
    // where it was when it closed.)
    PlacementParams pp(defaultSize, appRegKeyName, lastCloseRegKeyName);

    // Look for existing windows with the same class name.
    // If one exists (the top/last activated one) the new window will be
    // placed above the previous window, shifted down/right a bit to keep the
    // previous window's title bar visible (aka 'cascading').
    pp.FindPrevWindow(wndClassName);

    // Create the new window using default (CW_USEDEFAULT) position and size.
    // Do not set WS_VISIBLE (keep the window hidden).
    HWND hwnd = CreateWindowEx(
        0,
        wndClassName,
        L"SaveRestore Sample",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        nullptr,
        nullptr,
        hInstance,
        nullptr);

    if (!hwnd)
    {
        return false;
    }

    // Register to be restarted if the system reboots while this app is running.
    // Also check if this is a restart (using a command line string). This
    // modifies how we initially position the window (for example, if closed
    // while Minimized or on a background virtual desktop).
    PCWSTR restartCmdLine = L"/restart";
    RegisterApplicationRestart(restartCmdLine, 0);
    if (wcsstr(cmdLine, restartCmdLine) != nullptr)
    {
        pp.SetIsRestart();
    }

    // Turning off UseMonitorHint disables the default StartupInfo::MonitorHint flag.
    if (!UseMonitorHint)
    {
        pp.ClearStartupInfoFlag(StartupInfoFlags::MonitorHint);
    }

    // AllowPartiallyOffScreen is enabled by default, but the user setting can
    // disable it.
    if (AllowOffScreen)
    {
        pp.SetAllowPartiallyOffscreen();
    }

    // Move the window to the initial position and show it.
    pp.PositionAndShow(hwnd);

    return true;
}

// Called when the window is closing.
// This updates the last close position, stored in the registry. We'll use
// this position by default when launching a new instance.
void SaveLastClosePosition(HWND hwnd)
{
    PlacementEx::StorePlacementInRegistry(
        hwnd,
        appRegKeyName,
        lastCloseRegKeyName);
}

// Read registry key for MonitorHint user setting. This is called on first
// launch and after changing the settings (clicking text toggles the setting).
void ReadUserSettings()
{
    // UseMonitorHint default is 1 (enabled)
    DWORD dw = ReadDwordRegKey(appRegKeyName, UseMonitorHintKeyName, 1);
    UseMonitorHint = (dw == 1);

    // AllowOffScreen default is 0 (disabled)
    dw = ReadDwordRegKey(appRegKeyName, AllowOffScreenKeyName, 0);
    AllowOffScreen = (dw == 1);
}

// Handle WM_LBUTTONDOWN: Clicking on settings text toggles setting
void OnWmLButtonDown(HWND hwnd, LPARAM lParam)
{
    POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

    if (PtInRect(&rcUseMonitorHintTxt, pt))
    {
        // Toggle monitor hint setting
        WriteDwordRegKey(appRegKeyName, UseMonitorHintKeyName, !UseMonitorHint);
        ReadUserSettings();
        InvalidateRect(hwnd, nullptr, true);
    }

    if (PtInRect(&rcAllowOffScreenTxt, pt))
    {
        // Toggle allow offscreen setting
        WriteDwordRegKey(appRegKeyName, AllowOffScreenKeyName, !AllowOffScreen);
        ReadUserSettings();
        InvalidateRect(hwnd, nullptr, true);
    }

}

// Handle WM_GETMINMAXINFO: Enforce a minimum logical size
void OnWmGetMinMaxInfo(HWND hwnd, LPARAM lParam)
{
    MINMAXINFO* pmmi = reinterpret_cast<MINMAXINFO*>(lParam);
    UINT dpi = GetDpiForWindow(hwnd);
    pmmi->ptMinTrackSize.x = MulDiv(minSize.cx, dpi, 96);
    pmmi->ptMinTrackSize.y = MulDiv(minSize.cy, dpi, 96);
}

// Handle WM_GETMINMAXINFO:
// - escape closes the window
// - 1 key sizes window to 500x500
void OnWmChar(HWND hwnd, WPARAM wParam)
{
    switch(wParam)
    {
    case VK_ESCAPE:
        SendMessage(hwnd, WM_CLOSE, 0, 0);
        return;

    case 0x31: /* VK_1 */
    {
        PlacementEx pex;
        if (PlacementEx::GetPlacement(hwnd, &pex))
        {
            pex.showCmd = SW_SHOWNORMAL;
            WI_ClearFlag(pex.flags, PlacementFlags::Arranged);
            pex.SetLogicalSize(defaultSize);
            PlacementEx::SetPlacement(hwnd, &pex);
        }
        return;
    }
    }
}

void OnWmPaint(HWND hwnd, HDC hdc)
{
    const COLORREF rgbMaize = RGB(255, 203, 5);
    const COLORREF rgbBlue = RGB(0, 39, 76);
    const COLORREF rgbRed = RGB(122, 18, 28);
    static HBRUSH hbrMaize = CreateSolidBrush(rgbMaize);
    static HBRUSH hbrBlue = CreateSolidBrush(rgbBlue);
    static HBRUSH hbrRed = CreateSolidBrush(rgbRed);
    const UINT dpi = GetDpiForWindow(hwnd);
    const UINT textSize = 25;
    static HFONT hfont = nullptr;
    static UINT dpiLast = 0;

    // Create a font and store it until the window changes DPI (scale).
    if (dpiLast != dpi)
    {
        if (hfont)
        {
            DeleteObject(hfont);
        }

        hfont = CreateFont(MulDiv(textSize, dpi, 96),
                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                           L"Courier New");

        dpiLast = dpi;
    }

    SetBkMode(hdc, TRANSPARENT);
    SelectObject(hdc, hfont);

    // If maximized - Blue (yellow text)
    // If arranged - Red (yellow text)
    // Restored - Yellow (blue text)
    COLORREF rgbText;
    HBRUSH hbrBackground;
    if (IsZoomed(hwnd))
    {
        rgbText = rgbMaize;
        hbrBackground = hbrBlue;
    }
    else if (IsWindowArranged(hwnd))
    {
        rgbText = rgbMaize;
        hbrBackground = hbrRed;
    }
    else
    {
        rgbText = rgbBlue;
        hbrBackground = hbrMaize;
    }
    SetTextColor(hdc, rgbText);

    RECT rc;
    GetClientRect(hwnd, &rc);
    FillRect(hdc, &rc, hbrBackground);
    UINT nudge = MulDiv(10, dpi, 96);
    InflateRect(&rc, -1 * nudge, -1 * nudge);
    RECT rcTxt = rc;

    const UINT lineHeight = nudge + MulDiv(textSize, dpi, 96);

    // If Maximized or Arranged
    if (IsZoomed(hwnd) || IsWindowArranged(hwnd))
    {
        PCWSTR maxTxt = IsZoomed(hwnd) ? L"Maximized" : L"Arranged";
        DrawText(hdc, maxTxt, (int)wcslen(maxTxt), &rc, DT_LEFT);
        rc.top += lineHeight;
    }

    RECT rcWindow;
    GetWindowRect(hwnd, &rcWindow);

    // Current window size, then rect.
    std::wstring sizeStr = wil::str_printf<std::wstring>(L"(%d x %d)",
        RECTWIDTH(rcWindow), RECTHEIGHT(rcWindow));
    DrawText(hdc, sizeStr.c_str(), (int)wcslen(sizeStr.c_str()), &rc, DT_LEFT);
    rc.top += lineHeight;

    std::wstring rectStr = wil::str_printf<std::wstring>(L"[%d, %d, %d, %d]",
        rcWindow.left, rcWindow.top, rcWindow.right, rcWindow.bottom);
    DrawText(hdc, rectStr.c_str(), (int)wcslen(rectStr.c_str()), &rc, DT_LEFT);

    // Reset the rect to the full window (with nudge) and move to the bottom
    // row (the user settings).
    rc = rcTxt;
    rc.top = rc.bottom - lineHeight;

    // UseMonitorHint -> Launch Monitor: Last/Best (where 'use hint' == best)
    std::wstring lastMonSettingTxt = wil::str_printf<std::wstring>(
        L"Launch Monitor: %ws", UseMonitorHint ? L"Best" : L"Last");
    DrawText(hdc, lastMonSettingTxt.c_str(), (int)wcslen(lastMonSettingTxt.c_str()),
        &rc, DT_SINGLELINE | DT_LEFT | DT_BOTTOM);

    // Remember the rect for the monitor hint rect; clicking toggles the setting.
    rcUseMonitorHintTxt = rc;

    // AllowOffScreen. If no, this sets PlacementFlags::AllowPartiallyOffScreen.
    rc.top -= lineHeight;
    rc.bottom -= lineHeight;
    std::wstring offscreenSettingTxt = wil::str_printf<std::wstring>(
        L"Allow OffScreen: %ws", AllowOffScreen ? L"Yes" : L"No");
    DrawText(hdc, offscreenSettingTxt.c_str(), (int)wcslen(offscreenSettingTxt.c_str()),
        &rc, DT_SINGLELINE | DT_LEFT | DT_BOTTOM);

    // Remember the rect for the offscreen text; clicking toggles the setting.
    rcAllowOffScreenTxt = rc;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        OnWmPaint(hwnd, BeginPaint(hwnd, &ps));
        EndPaint(hwnd, &ps);
        break;
    }

    case WM_WINDOWPOSCHANGED:
    {
        // Repaint when the window rect changes
        auto pwp = reinterpret_cast<WINDOWPOS*>(lParam);
        RECT rc = { pwp->x, pwp->y, pwp->x + pwp->cx, pwp->y + pwp->cy };
        static RECT rcWindowLast = {};
        if (!EqualRect(&rc, &rcWindowLast))
        {
            rcWindowLast = rc;
            InvalidateRect(hwnd, nullptr, true);
        }
        break;
    }

    case WM_LBUTTONDOWN:
        OnWmLButtonDown(hwnd, lParam);
        break;

    case WM_GETMINMAXINFO:
        OnWmGetMinMaxInfo(hwnd, lParam);
        break;

    case WM_DPICHANGED:
    {
        RECT* prc = reinterpret_cast<RECT*>(lParam);
        SetWindowPos(hwnd,
                     nullptr,
                     prc->left,
                     prc->top,
                     RECTWIDTH(*prc),
                     RECTHEIGHT(*prc),
                     SWP_NOZORDER | SWP_NOACTIVATE);
        break;
    }

    case WM_CHAR:
        OnWmChar(hwnd, wParam);
        break;

    case WM_DESTROY:
        SaveLastClosePosition(hwnd);
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR cmdLine, int)
{
    // Run as Per-Monitor DPI Aware, or Unaware if 'u' in the command line.
    SetThreadDpiAwarenessContext(
        (wcsstr(cmdLine, L"u") != nullptr) ?
            DPI_AWARENESS_CONTEXT_UNAWARE :
            DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    // Initialize COM, needed for virtual desktop APIs (USE_VIRTUAL_DESKTOP_APIS).
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    // Read settings stored in registry.
    ReadUserSettings();

    WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hInstance = hInstance;
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = wndClassName;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDC_SAVE));

    // Create the window and show it.
    if (!RegisterClassEx(&wc) || !CreateMainWindow(hInstance, cmdLine))
    {
        return 1;
    }

    // Pump messages until exit.
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
