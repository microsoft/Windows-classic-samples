// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#include <windows.h>

#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

//
//      Overview
//
//      Normally, pop-up windows receive activation, resulting in the
//      owner window being de-activated.  To prevent the owner from
//      being de-activated, the pop-up window should not receive
//      activation.
//
//      Since the pop-up window is not active, input messages are not
//      delivered to the pop-up.  Instead, the input messages must be
//      explicitly inspected by the message loop.
//
//      Our sample program illustrates how you can create a pop-up
//      window that contains a selection of colors.
//
//      Right-click in the window to change its background color
//      via the fake menu popup.  Observe
//
//      -   The caption of the main application window remains
//          highlighted even though the fake-menu is "active".
//
//      -   The current fake-menu item highlight follows the mouse.
//
//      -   The keyboard arrows can be used to move the highlight,
//          ESC cancels the fake-menu, Enter accepts the fake-menu.
//
//      -   The fake-menu appears on the correct monitor (for
//          multiple-monitor systems).
//

HINSTANCE g_hInstance = NULL;
HBRUSH g_hbrColor = NULL;   // The selected color

// This is the array of predefined colors we put into the color picker.
const COLORREF c_rgclrPredef[] =
{
    RGB(0x00, 0x00, 0x00),          // 0 = black
    RGB(0x80, 0x00, 0x00),          // 1 = maroon
    RGB(0x00, 0x80, 0x00),          // 2 = green
    RGB(0x80, 0x80, 0x00),          // 3 = olive
    RGB(0x00, 0x00, 0x80),          // 4 = navy
    RGB(0x80, 0x00, 0x80),          // 5 = purple
    RGB(0x00, 0x80, 0x80),          // 6 = teal
    RGB(0x80, 0x80, 0x80),          // 7 = gray
    RGB(0xC0, 0xC0, 0xC0),          // 8 = silver
    RGB(0xFF, 0x00, 0x00),          // 9 = red
    RGB(0x00, 0xFF, 0x00),          // A = lime
    RGB(0xFF, 0xFF, 0x00),          // B = yellow
    RGB(0x00, 0x00, 0xFF),          // C = blue
    RGB(0xFF, 0x00, 0xFF),          // D = fuchsia
    RGB(0x00, 0xFF, 0xFF),          // E = cyan
    RGB(0xFF, 0xFF, 0xFF),          // F = white
};

//
//  COLORPICKSTATE
//
//      Structure that records the state of a color-picker pop-up.
//
//      A pointer to this state information is kept in the GWLP_USERDATA
//      window long.
//
//      The iSel field is the index of the selected color, or the
//      special value -1 to mean that no item is highlighted.
//
typedef struct COLORPICKSTATE
{
    BOOL fDone;             // Set when we should get out
    int iSel;               // Which color is selected?
    int iResult;            // Which color should be returned?
    HWND hwndOwner;         // Our owner window

} COLORPICKSTATE, *PCOLORPICKSTATE;

#define CYCOLOR         16      // Height of a single color pick
#define CXFAKEMENU      100     // Width of our fake menu

//
//  ColorPick_GetColorRect
//
//      Returns the rectangle that encloses the specified color.
//
void ColorPick_GetColorRect(RECT *prc, int iColor)
{
    // Build the "menu" item rect.
    prc->left = 0;
    prc->right = CXFAKEMENU;
    prc->top = iColor * CYCOLOR;
    prc->bottom = prc->top + CYCOLOR;
}

//
//  ColorPick_OnCreate
//
//      Stash away our state.
//
LRESULT ColorPick_OnCreate(HWND hwnd, LPCREATESTRUCT pcs)
{
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)(pcs->lpCreateParams));
    return 0;
}

//
//  ColorPick_OnPaint
//
//      Draw the color bars, and put a border around the selected color.
//
void ColorPick_OnPaint(PCOLORPICKSTATE pcps, HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    if (hdc)
    {
        RECT rcClient;
        GetClientRect(hwnd, &rcClient);

        // For each of our predefined colors, draw it in a little
        // rectangular region, leaving some border so the user can
        // see if the item is highlighted or not.
        for (int iColor = 0; iColor < ARRAYSIZE(c_rgclrPredef); iColor++)
        {
            // Build the "menu" item rect.
            RECT rc;
            ColorPick_GetColorRect(&rc, iColor);

            // If the item is highlighted, then draw a highlighted background.
            if (iColor == pcps->iSel)
            {
                FillRect(hdc, &rc, GetSysColorBrush(COLOR_HIGHLIGHT));
            }

            // Now shrink the rectangle by an edge and fill the rest with the
            // color of the item itself.
            InflateRect(&rc, -GetSystemMetrics(SM_CXEDGE),
                             -GetSystemMetrics(SM_CYEDGE));

            HBRUSH hbr = CreateSolidBrush(c_rgclrPredef[iColor]);
            FillRect(hdc, &rc, hbr);
            DeleteObject(hbr);
        }

        EndPaint(hwnd, &ps);
    }
}

//
//  ColorPick_ChangeSel
//
//      Change the selection to the specified item.
//
void ColorPick_ChangeSel(PCOLORPICKSTATE pcps, HWND hwnd, int iSel)
{
    // If the selection changed, then repaint the items that need repainting.
    if (pcps->iSel != iSel)
    {
        if (pcps->iSel >= 0)
        {
            RECT rc;
            ColorPick_GetColorRect(&rc, pcps->iSel);
            InvalidateRect(hwnd, &rc, TRUE);
        }

        pcps->iSel = iSel;
        if (pcps->iSel >= 0)
        {
            RECT rc;
            ColorPick_GetColorRect(&rc, pcps->iSel);
            InvalidateRect(hwnd, &rc, TRUE);
        }
    }
}

//
//  ColorPick_OnMouseMove
//
//      Track the mouse to see if it is over any of our colors.
//
void ColorPick_OnMouseMove(PCOLORPICKSTATE pcps, HWND hwnd, int x, int y)
{
    int iSel;

    if (x >= 0 && x < CXFAKEMENU &&
        y >= 0 && y < ARRAYSIZE(c_rgclrPredef) * CYCOLOR)
    {
        iSel = y / CYCOLOR;
    }
    else
    {
        iSel = -1;
    }

    ColorPick_ChangeSel(pcps, hwnd, iSel);
}

//
//  ColorPick_OnLButtonUp
//
//      When the button comes up, we are done.
//
void ColorPick_OnLButtonUp(PCOLORPICKSTATE pcps, HWND hwnd, int x, int y)
{
    // First track to the final location, in case the user moves the mouse
    // REALLY FAST and immediately lets go.
    ColorPick_OnMouseMove(pcps, hwnd, x, y);

    // Set the result to the current selection.
    pcps->iResult = pcps->iSel;

    // And tell the message loop that we're done.
    pcps->fDone = TRUE;
}

//
//  ColorPick_OnKeyDown
//
//      If the ESC key is pressed, then abandon the fake menu.
//      If the Enter key is pressed, then accept the current selection.
//      If an arrow key is pressed, the move the selection.
//
void ColorPick_OnKeyDown(PCOLORPICKSTATE pcps, HWND hwnd, WPARAM vk)
{
    switch (vk)
    {
        case VK_ESCAPE:
            pcps->fDone = TRUE;         // Abandoned
            break;

        case VK_RETURN:
            pcps->iResult = pcps->iSel; // Accept current selection
            pcps->fDone = TRUE;
            break;

        case VK_UP:
            if (pcps->iSel > 0)         // Decrement selection
            {
                ColorPick_ChangeSel(pcps, hwnd, pcps->iSel - 1);
            }
            else
            {
                ColorPick_ChangeSel(pcps, hwnd, ARRAYSIZE(c_rgclrPredef) - 1);
            }
            break;

        case VK_DOWN:                   // Increment selection
            if (pcps->iSel + 1 < ARRAYSIZE(c_rgclrPredef))
            {
                ColorPick_ChangeSel(pcps, hwnd, pcps->iSel + 1);
            }
            else
            {
                ColorPick_ChangeSel(pcps, hwnd, 0);
            }
            break;
    }
}

//
//  ColorPick_WndProc
//
//      Window procedure for the color picker popup.
//
LRESULT CALLBACK ColorPick_WndProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    PCOLORPICKSTATE pcps = (PCOLORPICKSTATE)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (uiMsg)
    {
        case WM_CREATE:
            return ColorPick_OnCreate(hwnd, (LPCREATESTRUCT)lParam);

        case WM_MOUSEMOVE:
            ColorPick_OnMouseMove(pcps, hwnd, (short)LOWORD(lParam),
                                              (short)HIWORD(lParam));
            break;

        case WM_LBUTTONUP:
            ColorPick_OnLButtonUp(pcps, hwnd, (short)LOWORD(lParam),
                                              (short)HIWORD(lParam));
            break;

        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
            ColorPick_OnKeyDown(pcps, hwnd, wParam);
            break;

        // Do not activate when somebody clicks the window.
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;

        case WM_PAINT:
            ColorPick_OnPaint(pcps, hwnd);
            return 0;
    }

    return DefWindowProc(hwnd, uiMsg, wParam, lParam);
}

//
//  ColorPick_ChooseLocation
//
//      Find a place to put the window so it won't go off the screen
//      or straddle two monitors.
//
//      x, y = location of mouse click (preferred upper-left corner)
//      cx, cy = size of window being created
//
//      We use the same logic that real menus use.
//
//      -   If (x, y) is too high or too far left, then slide onto screen.
//      -   Use (x, y) if all fits on the monitor.
//      -   If too low, then slide up.
//      -   If too far right, then flip left.
//
void ColorPick_ChooseLocation(HWND hwnd, int x, int y, int cx, int cy, LPPOINT ppt)
{
    // First get the dimensions of the monitor that contains (x, y).
    ppt->x = x;
    ppt->y = y;
    HMONITOR hmon = MonitorFromPoint(*ppt, MONITOR_DEFAULTTONULL);

    // If (x, y) is not on any monitor, then use the monitor that the owner
    // window is on.
    if (hmon == NULL)
    {
        hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    }

    MONITORINFO minf;
    minf.cbSize = sizeof(minf);
    GetMonitorInfo(hmon, &minf);

    // Now slide things around until they fit.

    // If too high, then slide down.
    if (ppt->y < minf.rcMonitor.top)
    {
        ppt->y = minf.rcMonitor.top;
    }

    // If too far left, then slide right.
    if (ppt->x < minf.rcMonitor.left)
    {
        ppt->x = minf.rcMonitor.left;
    }

    // If too low, then slide up.
    if (ppt->y > minf.rcMonitor.bottom - cy)
    {
        ppt->y = minf.rcMonitor.bottom - cy;
    }

    // If too far right, then flip left.
    if (ppt->x > minf.rcMonitor.right - cx)
    {
        ppt->x = ppt->x - cx;
    }
}

//
//  ColorPick_Popup
//
//      Display a fake menu to allow the user to select the color.
//
//      Return the color index the user selected, or -1 if no color
//      was selected.
//
int ColorPick_Popup(HWND hwndOwner, int x, int y)
{
    // Early check:  We must be on same thread as the owner so we can see
    // its mouse and keyboard messages when we set capture to it.
    if (GetCurrentThreadId() != GetWindowThreadProcessId(hwndOwner, NULL))
    {
        // Error: Menu must be on same thread as parent window.
        return -1;
    }

    COLORPICKSTATE cps;
    cps.fDone = FALSE;          // Not done yet
    cps.iSel = -1;              // No initial selection
    cps.iResult = -1;           // No result
    cps.hwndOwner = hwndOwner;  // Owner window

    // Set up the style and extended style we want to use.
    DWORD dwStyle = WS_POPUP | WS_BORDER;
    DWORD dwExStyle = WS_EX_TOOLWINDOW |      // So it doesn't show up in taskbar
                      WS_EX_DLGMODALFRAME |   // Get the edges right
                      WS_EX_WINDOWEDGE |
                      WS_EX_TOPMOST;          // So it isn't obscured

    // We want a client area of size (CXFAKEMENU, ARRAYSIZE(c_rgclrPredef) * CYCOLOR),
    // so use AdjustWindowRectEx to figure out what window rect will give us a
    // client rect of that size.
    RECT rc;
    rc.left = 0;
    rc.top = 0;
    rc.right = CXFAKEMENU;
    rc.bottom = ARRAYSIZE(c_rgclrPredef) * CYCOLOR;
    AdjustWindowRectEx(&rc, dwStyle, FALSE, dwExStyle);

    // Now find a proper home for the window that won't go off the screen or
    // straddle two monitors.
    int cx = rc.right - rc.left;
    int cy = rc.bottom - rc.top;
    POINT pt;
    ColorPick_ChooseLocation(hwndOwner, x, y, cx, cy, &pt);

    HWND hwndPopup = CreateWindowEx(
        dwExStyle,
        L"ColorPick",
        L"",
        dwStyle,
        pt.x, pt.y,
        cx, cy,
        hwndOwner,
        NULL,
        g_hInstance,
        &cps);

    // Show the window but don't activate it!
    ShowWindow(hwndPopup, SW_SHOWNOACTIVATE);

    // We want to receive all mouse messages, but since only the active
    // window can capture the mouse, we have to set the capture to our
    // owner window, and then steal the mouse messages out from under it.
    SetCapture(hwndOwner);

    // Go into a message loop that filters all the messages it receives
    // and route the interesting ones to the color picker window.
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        // Something may have happened that caused us to stop.
        if (cps.fDone)
        {
            break;
        }

        // If our owner stopped being the active window (e.g. the user
        // Alt+Tab'd to another window in the meantime), then stop.
        HWND hwndActive = GetActiveWindow();
        if (hwndActive != hwndOwner && !IsChild(hwndActive, hwndOwner) ||
            GetCapture() != hwndOwner)
        {
            break;
        }

        // At this point, we get to snoop at all input messages before
        // they get dispatched.  This allows us to route all input to our
        // popup window even if really belongs to somebody else.

        // All mouse messages are remunged and directed at our popup
        // menu. If the mouse message arrives as client coordinates, then
        // we have to convert it from the client coordinates of the original
        // target to the client coordinates of the new target.
        switch (msg.message)
        {
            // These mouse messages arrive in client coordinates, so in
            // addition to stealing the message, we also need to convert the
            // coordinates.
            case WM_MOUSEMOVE:
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_LBUTTONDBLCLK:
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_RBUTTONDBLCLK:
            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
            case WM_MBUTTONDBLCLK:
                pt.x = (short)LOWORD(msg.lParam);
                pt.y = (short)HIWORD(msg.lParam);
                MapWindowPoints(msg.hwnd, hwndPopup, &pt, 1);
                msg.lParam = MAKELPARAM(pt.x, pt.y);
                msg.hwnd = hwndPopup;
                break;

            // These mouse messages arrive in screen coordinates, so we just
            // need to steal the message.
            case WM_NCMOUSEMOVE:
            case WM_NCLBUTTONDOWN:
            case WM_NCLBUTTONUP:
            case WM_NCLBUTTONDBLCLK:
            case WM_NCRBUTTONDOWN:
            case WM_NCRBUTTONUP:
            case WM_NCRBUTTONDBLCLK:
            case WM_NCMBUTTONDOWN:
            case WM_NCMBUTTONUP:
            case WM_NCMBUTTONDBLCLK:
                msg.hwnd = hwndPopup;
                break;

            // We need to steal all keyboard messages, too.
            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_CHAR:
            case WM_DEADCHAR:
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_SYSCHAR:
            case WM_SYSDEADCHAR:
                msg.hwnd = hwndPopup;
                break;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);

        // Something may have happened that caused us to stop.
        if (cps.fDone)
        {
            break;
        }

        // If our owner stopped being the active window (e.g. the user
        // Alt+Tab'd to another window in the meantime), then stop.
        hwndActive = GetActiveWindow();
        if (hwndActive != hwndOwner && !IsChild(hwndActive, hwndOwner) ||
            GetCapture() != hwndOwner)
        {
            break;
        }
    }

    // Clean up the capture we created.
    ReleaseCapture();

    DestroyWindow(hwndPopup);

    // If we got a WM_QUIT message, then re-post it so the caller's message
    // loop will see it.
    if (msg.message == WM_QUIT)
    {
        PostQuitMessage((int)msg.wParam);
    }

    return cps.iResult;
}

//
//  FakeMenuDemo_OnEraseBkgnd
//
//      Erase the background in the selected color.
//
void FakeMenuDemo_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    FillRect(hdc, &rc, g_hbrColor);
}

//
//  FakeMenuDemo_OnContextMenu
//
//      Display the color-picker pseudo-menu so the user can change
//      the color.
//
void FakeMenuDemo_OnContextMenu(HWND hwnd, int x, int y)
{
    // If the coordinates are (-1, -1), then the user used the keyboard -
    // we'll pretend the user clicked at client (0, 0).
    if (x == -1 && y == -1)
    {
        POINT pt;
        pt.x = 0;
        pt.y = 0;
        ClientToScreen(hwnd, &pt);
        x = pt.x;
        y = pt.y;
    }

    int iColor = ColorPick_Popup(hwnd, x, y);

    // If the user picked a color, then change to that color.
    if (iColor >= 0)
    {
        DeleteObject(g_hbrColor);
        g_hbrColor = CreateSolidBrush(c_rgclrPredef[iColor]);
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

//
//  FakeMenuDemo_WndProc
//
//      Window procedure for the fake menu demo.
//
LRESULT CALLBACK FakeMenuDemo_WndProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uiMsg)
    {
        case WM_ERASEBKGND:
            FakeMenuDemo_OnEraseBkgnd(hwnd, (HDC)wParam);
            return TRUE;

        case WM_CONTEXTMENU:
            FakeMenuDemo_OnContextMenu(hwnd, (short)LOWORD(lParam),
                                             (short)HIWORD(lParam));
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }

    return DefWindowProc(hwnd, uiMsg, wParam, lParam);
}

//
//      Program entry point - demonstrate pseudo-menus.
//
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE /* hInstPrev */, LPWSTR /* pszCmdLine */, int nCmdShow)
{
    g_hInstance = hInstance;

    WNDCLASSEX wc = { sizeof(wc) };
    wc.lpfnWndProc = ColorPick_WndProc;
    wc.hInstance = g_hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wc.lpszClassName = L"ColorPick";
    RegisterClassEx(&wc);

    wc.style = 0;
    wc.lpfnWndProc = FakeMenuDemo_WndProc;
    wc.hInstance = g_hInstance;
    wc.hbrBackground = NULL;            // Background color is dynamic
    wc.lpszClassName = L"FakeMenuDemo";
    RegisterClassEx(&wc);

    g_hbrColor = CreateSolidBrush(RGB(0xFF, 0xFF, 0xFF));

    HWND hwnd = CreateWindow(
        L"FakeMenuDemo",
        L"Fake Menu Demo - Right-click in window to change color",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        g_hInstance,
        0);

    ShowWindow(hwnd, nCmdShow);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
