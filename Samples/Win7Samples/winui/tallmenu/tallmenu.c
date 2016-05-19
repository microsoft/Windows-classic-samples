/*****************************************************************************
 *
 *  TallMenu.c
 *
 *  Copyright (c) 1997-2002 Microsoft Corporation.
 *
 *  All rights reserved.
 *
 *       This source code is only intended as a supplement to 
 *       Development Tools and/or SDK documentation.
 *       See these sources for detailed information regarding the 
 *       Microsoft samples programs.
 *
 *   THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
 *   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 *   PARTICULAR PURPOSE.
 *
 *  Abstract:
 *
 *      Sample program to demonstrate how a program can display a
 *      tall menu without the menu going off the top or bottom of
 *      the work area.
 *
 *****************************************************************************/

#include "tallmenu.h"

/*****************************************************************************
 *
 *      Overview
 *
 *      To split a tall menu into multiple columns, add the
 *      MF_MENUBREAK attribute on each item that should start
 *      in a new column.
 *
 *      To apply this dynamically to a menu, walk through the items
 *      on the menu, tallying their cumulative heights.  When the
 *      height exceeds the height of the screen, place the next
 *      item in a new column.
 *
 *****************************************************************************/

#define COMPILE_MULTIMON_STUBS
#include <multimon.h>

/*****************************************************************************
 *
 *      GetWorkArea
 *
 *      This function takes a window handle and returns the dimensions
 *      of the work area associated with that window.
 *
 *      Ask the system which monitor the window is on and use the
 *      work area of that monitor.
 *
 *****************************************************************************/

void
GetWorkArea(HWND hwnd, LPRECT prcWork)
{
    HMONITOR hmon;
    MONITORINFO moni;

    hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);

    moni.cbSize = sizeof(moni);
    GetMonitorInfo(hmon, &moni);

    *prcWork = moni.rcWork;
}

/*****************************************************************************
 *
 *      AdjustTallMenu
 *
 *      The function takes a tall menu and adds/removes MF_MENUBREAK
 *      attributes so that the menu is not taller than the screen.
 *
 *      The job is made trickier by multiple monitor support.  Each
 *      monitor may have a different height, and we must make sure
 *      to use the height appropriate to the monitor associated with
 *      the menu.
 *
 *      hwnd - The window associated with the menu
 *      hmenuPopup - The popup menu to adjust
 *
 *****************************************************************************/

void
AdjustTallMenu(HWND hwnd, HMENU hmenuPopup)
{
    int cyScreen, cyItem, cy;
    int iItem, cItem;
    RECT rc;

    /*
     *  Early-out: If the menu is empty, then there is nothing to adjust.
     *  This saves us from boundary conditions in the rest of the code.
     */
    cItem = GetMenuItemCount(hmenuPopup);
    if (cItem == 0) return;

    GetWorkArea(hwnd, &rc);

    /*
     *  Convert rectangle boundaries to height.
     */
    cyScreen = rc.bottom - rc.top;

    /*
     *  Subtract off some extra to account for the border around the menu.
     *  Note that this doesn't need to be exact, since this entire process
     *  of adjusting a tall menu is a heuristic anyway.
     */

    cyScreen -= 2 * 3 * GetSystemMetrics(SM_CYEDGE);

    /*
     *  Warning!  We assume that the menu is not owner-draw!
     *
     *  Extending this code to support owner-draw menu items is left
     *  as an exercise for the reader.  (Hint: WM_MEASUREITEM is your
     *  friend.)
     */
    GetMenuItemRect(hwnd, hmenuPopup, 0, &rc);
    cyItem = rc.bottom - rc.top;

    /*
     *  cyItem is zero if the menu item has never been shown, in which
     *  case we cheat and use the menu bar height, which is a reasonable
     *  approximation to the menu item height.
     */
    if (cyItem == 0) {
        cyItem = GetSystemMetrics(SM_CYMENU);
    }

    /*
     *  Now walk through the menu tallying up the height of each item.
     *  When it gets too tall, insert a break.
     *
     *  An annoying feature of SetMenuItemInfo is that you can't change
     *  the MFT_MENUBARBREAK flag without resetting the text, so we
     *  have to get the text, flip the flag, and set the text back
     *  with the new flag.
     */

    cy = 0;
    for (iItem = 0; iItem < cItem; iItem++) {
        MENUITEMINFO mii;
        TCHAR buf[80];

        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE | MIIM_DATA;
        mii.dwTypeData = buf;
        mii.cch = 80;

        if (GetMenuItemInfo(hmenuPopup, iItem, MF_BYPOSITION, &mii)) {
            if (cy > cyScreen) {
                cy = 0;
                mii.fType |= MFT_MENUBARBREAK;
            } else {
                mii.fType &= ~MFT_MENUBARBREAK;
            }

            SetMenuItemInfo(hmenuPopup, iItem, MF_BYPOSITION, &mii);
        }
        cy += cyItem;
    }

}

/*****************************************************************************
 *
 *  TallMenu_WndProc
 *
 *      Window procedure for tall menu demo.
 *
 *****************************************************************************/

LRESULT CALLBACK
TallMenu_WndProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uiMsg) {

    /*
     *  If a popup menu is about to be displayed, first make sure
     *  it isn't too tall for the screen.
     */
    case WM_INITMENUPOPUP:
        AdjustTallMenu(hwnd, (HMENU)wParam);
        break;

    /*
     *  The app exits when this window is destroyed.
     */
    case WM_DESTROY:
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
 *      Demonstrate tall menus and how to prevent them from going off
 *      the screen.
 *
 *****************************************************************************/

int WINAPI
WinMain(HINSTANCE hinst, HINSTANCE hinstPrev, LPSTR pszCmdLine, int nCmdShow)
{
    WNDCLASS wc;
    MSG msg;
    HWND hwnd;

    wc.style = 0;
    wc.lpfnWndProc = TallMenu_WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hinst;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = TEXT("TallMenu");
    wc.lpszClassName = TEXT("TallMenu");

    RegisterClass(&wc);

    hwnd = CreateWindow(
        TEXT("TallMenu"),               /* Class Name */
        TEXT("Tall Menu Sample"),       /* Title */
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
