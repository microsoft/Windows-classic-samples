/*****************************************************************************
 *
 *  Obscure.c
 *
 *  Copyright (c) 1997 Microsoft Corporation.
 *
 *  Abstract:
 *
 *      Sample program to demonstrate how a program can detect
 *      whether its client area is totally obscured.
 *
 *      A program may wish to do this in order to suppress
 *      computationally expensive graphics operations when
 *      there is no way for the user to see the output.
 *
 *  Details:
 *
 *      This program displays the time in its client area.
 *      When the client area is completely obscured, there is no
 *      point in drawing the time (since the user can't see it),
 *      so the application goes to sleep until the client area
 *      is again visible.
 *
 *      To see this program in action, run this application and
 *      Notepad simultaneously.  Drag the Notepad window so that
 *      it covers this sample program.  Observe that the title
 *      changes to "Covered".  Move the Notepad window away, and
 *      observe that the title changes to "Uncovered" or "Partially
 *      Covered", as appropriate.
 *
 *****************************************************************************/

#define STRICT
#include <windows.h>
#include <strsafe.h>

/*****************************************************************************
 *
 *      Overview
 *
 *      To determine whether the client area is obscured, determine
 *      whether it has an empty clip region.
 *
 *      To be notified when the client area is again unobscured,
 *      invalidate the entire client rectangle and wait for
 *      Windows to send a WM_PAINT message.  (Explicit invalidation
 *      is required, for if you were obscured by a menu or a window
 *      with the CS_SAVEBITS class style, Windows will restore your
 *      client area automatically instead of sending a WM_PAINT message.)
 *
 *      So that you can see the behavior of the application, we will
 *      put information in the title of the window.  Watch the taskbar
 *      to see the application change its state.
 *
 *****************************************************************************/

BOOL g_fTimerActive = FALSE;

/*****************************************************************************
 *
 *      GetClientObscuredness
 *
 *      Returns one of the OBS_* values.
 *
 *****************************************************************************/

#define OBS_COMPLETELYCOVERED       0
#define OBS_PARTIALLYVISIBLE        1
#define OBS_COMPLETELYVISIBLE       2

int
GetClientObscuredness(HWND hwnd)
{
    HDC hdc;
    RECT rc, rcClient;
    int iType;

    hdc = GetDC(hwnd);
    iType = GetClipBox(hdc, &rc);

    ReleaseDC(hwnd, hdc);

    if (iType == NULLREGION)
        return OBS_COMPLETELYCOVERED;
    if (iType == COMPLEXREGION)
        return OBS_PARTIALLYVISIBLE;

    GetClientRect(hwnd, &rcClient);
    if (EqualRect(&rc, &rcClient))
        return OBS_COMPLETELYVISIBLE;

    return OBS_PARTIALLYVISIBLE;
}

/*****************************************************************************
 *
 *  Obscure_EnsureTimerStopped
 *
 *      If the timer that causes us to do work is running, then stop it.
 *
 *****************************************************************************/

void
Obscure_EnsureTimerStopped(HWND hwnd)
{
    if (g_fTimerActive) {
        KillTimer(hwnd, 1);
        g_fTimerActive = FALSE;
        InvalidateRect(hwnd, NULL, FALSE);
        SetWindowText(hwnd, TEXT("Covered (Paused)"));
    }
}

/*****************************************************************************
 *
 *  Obscure_TimerProc
 *
 *      If we have become obscured, then stop the timer.
 *
 *      Otherwise, invalidate our rectangle so we will redraw with
 *      the new time.
 *
 *****************************************************************************/

void CALLBACK
Obscure_TimerProc(HWND hwnd, UINT uiMsg, UINT_PTR idTimer, DWORD tm)
{
    /*
     *  If the client area is totally obscured, then stop the timer
     *  so we don't waste any more CPU.
     */
    int iState = GetClientObscuredness(hwnd);
    if (iState == OBS_COMPLETELYCOVERED) {
        Obscure_EnsureTimerStopped(hwnd);
    } else {
        InvalidateRect(hwnd, NULL, FALSE);
        if (iState == OBS_PARTIALLYVISIBLE) {
            SetWindowText(hwnd, TEXT("Partially Visible"));
        } else if (iState == OBS_COMPLETELYVISIBLE) {
            SetWindowText(hwnd, TEXT("Completely Visible"));
        }
    }
}

/*****************************************************************************
 *
 *  Obscure_EnsureTimerRunning
 *
 *      If the timer that causes us to do work is not running, then start it.
 *
 *****************************************************************************/

void
Obscure_EnsureTimerRunning(HWND hwnd)
{
    if (!g_fTimerActive) {
        SetTimer(hwnd, 1, 100, Obscure_TimerProc);
        g_fTimerActive = TRUE;
    }
}

/*****************************************************************************
 *
 *  Obscure_OnPaint
 *
 *      Draw the current time and restart the timer if needed.
 *
 *****************************************************************************/

void
Obscure_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc;

    hdc = BeginPaint(hwnd, &ps);
    if (hdc) {
        TCHAR sz[64];
        RECT rc;
        HFONT hfOld;
        COLORREF crTextOld, crBkOld;
        int cch;

        if (IsRectEmpty(&ps.rcPaint)) {
            /*
             *  Nothing to do.  Don't wake up either.
             */
        } else {
            hfOld = SelectObject(hdc, GetStockObject(ANSI_FIXED_FONT));
            crTextOld = SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
            crBkOld = SetBkColor(hdc, GetSysColor(COLOR_WINDOW));

            GetClientRect(hwnd, &rc);
            cch = StringCchPrintf(sz, 64, TEXT("%u"), GetTickCount());
            ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, sz, cch, NULL);

            SetBkColor(hdc, crBkOld);
            SetTextColor(hdc, crTextOld);
            SelectObject(hdc, hfOld);

            Obscure_EnsureTimerRunning(hwnd);
        }

        EndPaint(hwnd, &ps);
    }
}

/*****************************************************************************
 *
 *  Obscure_WndProc
 *
 *      Window procedure for obscured window demo.
 *
 *****************************************************************************/

LRESULT CALLBACK
Obscure_WndProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uiMsg) {

    case WM_CREATE:
        Obscure_EnsureTimerRunning(hwnd);
        break;

    case WM_PAINT:
        Obscure_OnPaint(hwnd);
        break;

    /*
     *  The app exits when this window is destroyed.
     */
    case WM_DESTROY:
        Obscure_EnsureTimerStopped(hwnd);
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, uiMsg, wParam, lParam);
}

/*****************************************************************************
 *
 *  WinMain
 *
 *      Program entry point.
 *
 *****************************************************************************/

int WINAPI
WinMain(HINSTANCE hinst, HINSTANCE hinstPrev, LPSTR pszCmdLine, int nCmdShow)
{
    WNDCLASS wc;
    MSG msg;
    HWND hwnd;

    wc.style = 0;
    wc.lpfnWndProc = Obscure_WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hinst;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = TEXT("Obscure");

    RegisterClass(&wc);

    hwnd = CreateWindow(
        TEXT("Obscure"),                /* Class Name */
        TEXT("Obscure"),                /* Title */
        WS_OVERLAPPEDWINDOW,            /* Style */
        CW_USEDEFAULT, CW_USEDEFAULT,   /* Position */
        CW_USEDEFAULT, CW_USEDEFAULT,   /* Size */
        NULL,                           /* Parent */
        NULL,                           /* Use class menu */
        hinst,                          /* Instance */
        0);                             /* No special parameters */

    ShowWindow(hwnd, nCmdShow);

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
