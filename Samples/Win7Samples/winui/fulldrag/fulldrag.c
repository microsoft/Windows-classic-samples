/*****************************************************************************
 *
 *  Fulldrag.c
 *
 *  Copyright (c) 1997 Microsoft Corporation.
 *
 *  Abstract:
 *
 *      Sample program to demonstrate how a program can override
 *      Full Drag.
 *
 *      This program is to be read in conjunction with KB article Q121541.
 *
 *****************************************************************************/

#include "fulldrag.h"

/*****************************************************************************
 *
 *      Globals
 *
 *      g_fMoveSize
 *
 *          Set while we are in a move/size loop.  The WM_ENTERSIZEMOVE
 *          and WM_EXITSIZEMOVE messages tell us when these things happen.
 *
 *      g_fPaintDeferred
 *
 *          Set if we deferred a paint that occured while we were
 *          busy doing a Move/Size.  When the Move/Size completes
 *          and we discover that a paint was deferred, we force
 *          a full repaint to complete the deferred paint.
 *
 *
 *      Note:
 *
 *      In a real (i.e., non-sample) program, these would not be global
 *      variables.  They would be per-window variables.
 *
 *****************************************************************************/

BOOL g_fDragging;
BOOL g_fPaintDeferred;

/*****************************************************************************
 *
 *  Hilbert
 *
 *      Draws a segment of the hilbert curve.
 *
 *      The math is not important.  What is important is that drawing
 *      a hilbert curve takes a long time.
 *
 *****************************************************************************/

#define MAXDEPTH 8      /* Bigger depth takes longer to draw */

void
Hilbert(HDC hdc, int x, int y, int vx, int vy, int wx, int wy, int n)
{
    if (n >= MAXDEPTH) {
        LineTo(hdc, x + (vx+wx)/2, y + (vy+wy)/2);
    } else {
        n++;
        Hilbert(hdc, x, y, wx/2, wy/2, vx/2, vy/2, n);
        Hilbert(hdc, x+vx/2, y+vy/2, vx/2, vy/2, wx/2, wy/2, n);
        Hilbert(hdc, x+vx/2+wx/2, y+vy/2+wy/2, vx/2, vy/2, wx/2, wy/2, n);
        Hilbert(hdc, x+vx/2+wx, y+vy/2+wy, -wx/2, -wy/2, -vx/2, -vy/2, n);
    }
}

/*****************************************************************************
 *
 *  Hilbert_OnPaint
 *
 *      Handle the WM_PAINT message.
 *
 *      If the user is dragging the window, then don't do our painting,
 *      because that would make the dragging very jerky.  Instead, just
 *      remember that there was a paint message that we ignored.  After
 *      the size/move is complete, we will perform one big paint to do
 *      the things that we had ignored.
 *
 *****************************************************************************/

void
Hilbert_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc;

    hdc = BeginPaint(hwnd, &ps);
    if (hdc) {
        if (g_fDragging) {
            g_fPaintDeferred = TRUE;
        } else {

            HBRUSH hbrOld;
            HPEN hpenOld;
            HCURSOR hcurOld;

            hcurOld = SetCursor(LoadCursor(0, IDC_WAIT));

            hbrOld = SelectObject(hdc, GetStockObject(BLACK_BRUSH));
            hpenOld = SelectObject(hdc, GetStockObject(BLACK_PEN));

            MoveToEx(hdc, 0, 0, 0);
            Hilbert(hdc, 0, 0, GetSystemMetrics(SM_CXFULLSCREEN), 0,
                               0, GetSystemMetrics(SM_CYFULLSCREEN), 0);
            SelectObject(hdc, hpenOld);
            SelectObject(hdc, hbrOld);

            SetCursor(hcurOld);
        }
        EndPaint(hwnd, &ps);
    }
}

/*****************************************************************************
 *
 *  Hilbert_WndProc
 *
 *      Window procedure.
 *
 *****************************************************************************/

LRESULT CALLBACK
Hilbert_WndProc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp)
{
    switch (wm) {
    case WM_PAINT:
        Hilbert_OnPaint(hwnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    /*
     *  When we begin a size/move operation, remember the fact
     *  so we won't paint during the size/move.
     */
    case WM_ENTERSIZEMOVE:
        g_fDragging = TRUE;
        break;

    /*
     *  When we finish a size/move operation, remember the fact
     *  so we will resume painting, and if there were any deferred
     *  paint messages, re-invalidate ourselves so they will be
     *  regenerated.
     */
    case WM_EXITSIZEMOVE:
        g_fDragging = FALSE;
        if (g_fPaintDeferred) {
            g_fPaintDeferred = FALSE;
            InvalidateRect(hwnd, 0, TRUE);
        }
        break;

    }

    return DefWindowProc(hwnd, wm, wp, lp);
}

/*****************************************************************************
 *
 *  WinMain
 *
 *      Program entry point.
 *
 *      Register the class, create the window, and go into a message loop.
 *
 *****************************************************************************/

int WINAPI
WinMain(HINSTANCE hinst, HINSTANCE hinstPrev, LPSTR pszCmdLine, int nCmdShow)
{
    HWND hwnd;
    MSG msg;
    WNDCLASS wc = {
        0,
        Hilbert_WndProc,
        0,
        0,
        hinst,
        LoadIcon(0, IDI_APPLICATION),
        LoadCursor(0, IDC_ARROW),
        (HBRUSH)(COLOR_WINDOW+1),
        0,
        TEXT("Hilbert")
    };

    RegisterClass(&wc);
    hwnd = CreateWindow(TEXT("Hilbert"),
		                TEXT("Hilbert"), WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, CW_USEDEFAULT,
                        CW_USEDEFAULT, CW_USEDEFAULT,
                        0, 0, hinst, 0);
    ShowWindow(hwnd, nCmdShow);
    while (GetMessage(&msg, 0, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
