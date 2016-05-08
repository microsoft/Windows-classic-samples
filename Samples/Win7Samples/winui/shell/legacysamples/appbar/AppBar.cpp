// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#include <windows.h>
#include <windowsx.h>
#include <strsafe.h>
#include "resource.h"
#include "appbar.h"

#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

/////////////////////////////////////////////////////////////////////////////
// Local Prototypes

static BOOL AppBar_AutoHide(HWND hwnd);
static BOOL AppBar_NoAutoHide(HWND hwnd);
static void SlideWindow(HWND hwnd, LPRECT prc);

/////////////////////////////////////////////////////////////////////////////
// Global Variables

HINSTANCE g_hInstance = NULL;
BOOL g_fAppRegistered = FALSE;      // TRUE if the appbar is registered
RECT g_rcAppBar;                    // Current rect of the appbar

DWORD g_cxWidth, g_cyHeight;
const int g_dtSlideHide = 400;
const int g_dtSlideShow = 200;

//
//  FUNCTION:   AppBar_Size(HWND)
//
//  PURPOSE:    Handles updating the appbar's size and position.
//
//  PARAMETERS:
//      hwnd    - handle of the appbar
//
//  COMMENTS:
//

void AppBar_Size(HWND hwnd)
{
    if (g_fAppRegistered)
    {
        POPTIONS pOpt = GetAppbarData(hwnd);

        APPBARDATA abd;
        abd.cbSize = sizeof(APPBARDATA);
        abd.hWnd = hwnd;

        RECT rc;
        GetWindowRect(hwnd, &rc);
        AppBar_QuerySetPos(pOpt->uSide, &rc, &abd, TRUE);
    }
}


//
//  FUNCTION:   AppBar_QueryPos
//
//  PURPOSE:    Asks the system if the AppBar can occupy the rectangle specified
//              in lprc.  The system will change the lprc rectangle to make
//              it a valid rectangle on the desktop.
//
//  PARAMETERS:
//      hwnd - Handle to the AppBar window.
//      lprc - Rectange that the AppBar is requesting to occupy.
//

void AppBar_QueryPos(HWND hwnd, LPRECT lprc)
{
    POPTIONS pOpt = GetAppbarData(hwnd);

    // Fill out the APPBARDATA struct and save the edge we're moving to
    // in the appbar OPTIONS struct.
    APPBARDATA abd;
    abd.hWnd = hwnd;
    abd.cbSize = sizeof(APPBARDATA);
    abd.rc = *lprc;
    abd.uEdge = pOpt->uSide;

    // Calculate the part we want to occupy.  We only figure out the top
    // and bottom coordinates if we're on the top or bottom of the screen.
    // Likewise for the left and right.  We will always try to occupy the
    // full height or width of the screen edge.
    int iWidth = 0;
    int iHeight = 0;
    if ((ABE_LEFT == abd.uEdge) || (ABE_RIGHT == abd.uEdge))
    {
        iWidth = abd.rc.right - abd.rc.left;
        abd.rc.top = 0;
        abd.rc.bottom = GetSystemMetrics(SM_CYSCREEN);
    }
    else
    {
        iHeight = abd.rc.bottom - abd.rc.top;
        abd.rc.left = 0;
        abd.rc.right = GetSystemMetrics(SM_CXSCREEN);
    }

    // Ask the system for the screen space
    SHAppBarMessage(ABM_QUERYPOS, &abd);

    // The system will return an approved position along the edge we're asking
    // for.  However, if we can't get the exact position requested, the system
    // only updates the edge that's incorrect.  For example, if we want to
    // attach to the bottom of the screen and the taskbar is already there,
    // we'll pass in a rect like 0, 964, 1280, 1024 and the system will return
    // 0, 964, 1280, 996.  Since the appbar has to be above the taskbar, the
    // bottom of the rect was adjusted to 996.  We need to adjust the opposite
    // edge of the rectangle to preserve the height we want.

    switch (abd.uEdge)
    {
        case ABE_LEFT:
            abd.rc.right = abd.rc.left + iWidth;
            break;

        case ABE_RIGHT:
            abd.rc.left = abd.rc.right - iWidth;
            break;

        case ABE_TOP:
            abd.rc.bottom = abd.rc.top + iHeight;
            break;

        case ABE_BOTTOM:
            abd.rc.top = abd.rc.bottom - iHeight;
            break;
    }

    *lprc = abd.rc;
}


//
//  FUNCTION:   AppBar_QuerySetPos(UINT, LPRECT, PAPPBARDATA, BOOL)
//
//  PURPOSE:    Asks the system if the appbar can move itself to a particular
//              side of the screen and then does move the appbar.
//
//  PARAMETERS:
//      uEdge   - Side of the screen to move to.  Can be ABE_TOP, ABE_BOTTOM,
//                ABE_LEFT, or ABE_RIGHT.
//      lprc    - Screen rect the appbar wishes to occupy.  This will be
//                modified and will return the area the system will let the
//                appbar occupy.
//      pabd    - Pointer to the APPBARDATA struct used in all appbar system
//                calls.
//      fMove   - TRUE if the function should move the appbar, FALSE if the
//                caller will move the AppBar.
//

void AppBar_QuerySetPos(UINT uEdge, LPRECT lprc, PAPPBARDATA pabd, BOOL fMove)
{
    POPTIONS pOpt = GetAppbarData(pabd->hWnd);

    // Fill out the APPBARDATA struct and save the edge we're moving to
    // in the appbar OPTIONS struct.
    pabd->rc = *lprc;
    pabd->uEdge = uEdge;
    pOpt->uSide = uEdge;

    AppBar_QueryPos(pabd->hWnd, &(pabd->rc));

    // Tell the system we're moving to this new approved position.
    SHAppBarMessage(ABM_SETPOS, pabd);

    if (fMove)
    {
        // Move the appbar window to the new position
        MoveWindow(pabd->hWnd, pabd->rc.left, pabd->rc.top,
                   pabd->rc.right - pabd->rc.left,
                   pabd->rc.bottom - pabd->rc.top, TRUE);
    }

    // Save the appbar rect.  We use this later when we autohide.  If we're
    // currently hidden, then don't mess with this.
    if (!pOpt->fAutoHide)
    {
        g_rcAppBar = pabd->rc;
    }
}


//
//  FUNCTION:   AppBar_PosChanged(PAPPBARDATA)
//
//  PURPOSE:    The system has changed our position for some reason.  We need
//              to recalculate the position on the screen we want to occupy
//              by determining how wide or thick we are and the update the
//              screen position.
//
//
//  PARAMETERS:
//      pabd    - Pointer to the APPBARDATA structure used in all AppBar calls
//                to the system.
//

void AppBar_PosChanged(PAPPBARDATA pabd)
{
    // Start by getting the size of the screen.
    RECT rc;
    rc.top = 0;
    rc.left = 0;
    rc.right = GetSystemMetrics(SM_CXSCREEN);
    rc.bottom = GetSystemMetrics(SM_CYSCREEN);

    // Update the g_rcAppbar so when we slide (if hidden) we slide to the
    // right place.
    POPTIONS pOpt = GetAppbarData(pabd->hWnd);
    if (pOpt->fAutoHide)
    {
        g_rcAppBar = rc;
        switch (pOpt->uSide)
        {
            case ABE_TOP:
                g_rcAppBar.bottom = g_rcAppBar.top + g_cyHeight;
                break;

            case ABE_BOTTOM:
                g_rcAppBar.top = g_rcAppBar.bottom - g_cyHeight;
                break;

            case ABE_LEFT:
                g_rcAppBar.right = g_rcAppBar.left + g_cxWidth;
                break;

            case ABE_RIGHT:
                g_rcAppBar.left = g_rcAppBar.right - g_cxWidth;
                break;
        }
    }

    // Now get the current window rectangle and find the height and width
    RECT rcWindow;
    GetWindowRect(pabd->hWnd, &rcWindow);
    int iHeight = rcWindow.bottom - rcWindow.top;
    int iWidth = rcWindow.right - rcWindow.left;

    // Depending on which side we're on, try to preserve the thickness of
    // the window
    switch (pOpt->uSide)
    {
        case ABE_TOP:
            rc.bottom = rc.top + iHeight;
            break;

        case ABE_BOTTOM:
            rc.top = rc.bottom - iHeight;
            break;

        case ABE_LEFT:
            rc.right = rc.left + iWidth;
            break;

        case ABE_RIGHT:
            rc.left = rc.right - iWidth;
            break;
    }

    // Move the appbar.
    AppBar_QuerySetPos(pOpt->uSide, &rc, pabd, TRUE);
}


//
//  FUNCTION:   AppBar_Callback(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:    Handles notification messages sent from the system to our
//              appbar.
//
//  PARAMETERS:
//      hwnd    - Handle of the appbar window receiving the notification.
//      uMsg    - The appbar notification message that we registered.
//      wParam  - Contains the specific notification code.
//      lParam  - Extra information dependant on the notification code.
//

void AppBar_Callback(HWND hwnd, UINT /* uMsg */, WPARAM wParam, LPARAM lParam)
{
    static HWND hwndZOrder = NULL;

    APPBARDATA abd;
    abd.cbSize = sizeof(abd);
    abd.hWnd = hwnd;

    switch (wParam)
    {
        // Notifies the appbar that the taskbar's autohide or always-on-top
        // state has changed.  The appbar can use this to conform to the settings
        // of the system taskbar.
        case ABN_STATECHANGE:
            break;

        // Notifies the appbar when a full screen application is opening or
        // closing.  When a full screen app is opening, the appbar must drop
        // to the bottom of the Z-Order.  When the app is closing, we should
        // restore our Z-order position.
        case ABN_FULLSCREENAPP:
            if (lParam)
            {
                // A full screen app is opening.  Move us to the bottom of the
                // Z-Order.

                // First get the window that we're underneath so we can correctly
                // restore our position
                hwndZOrder = GetWindow(hwnd, GW_HWNDPREV);

                // Now move ourselves to the bottom of the Z-Order
                SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0,
                             SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            }
            else
            {
                // The app is closing.  Restore the Z-order
                POPTIONS pOpt = GetAppbarData(hwnd);
                SetWindowPos(hwnd, pOpt->fOnTop ? HWND_TOPMOST : hwndZOrder,
                             0, 0, 0, 0,
                             SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

                hwndZOrder = NULL;
            }
            break;

        // Notifies the appbar when an event has occured that may effect the
        // appbar's size and position.  These events include changes in the
        // taskbar's size, position, and visiblity as well as adding, removing,
        // or resizing another appbar on the same side of the screen.
        case ABN_POSCHANGED:
            // Update our position in response to the system change
            AppBar_PosChanged(&abd);
            break;
    }
}


//
//  FUNCTION:   AppBar_Register(HWND)
//
//  PURPOSE:    Registers the appbar with the system.
//
//  PARAMETERS:
//      hwnd    - handle of the appbar to register.
//
//  RETURN VALUE:
//      Returns TRUE if successful, FALSE otherwise.
//
//  COMMENTS:
//      Sets the system wide g_fAppRegistered variable.
//

BOOL AppBar_Register(HWND hwnd)
{
    APPBARDATA abd;
    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = hwnd;
    abd.uCallbackMessage = APPBAR_CALLBACK;

    g_fAppRegistered = (BOOL)SHAppBarMessage(ABM_NEW, &abd);
    return g_fAppRegistered;
}


//
//  FUNCTION:   AppBar_UnRegister(HWND)
//
//  PARAMETERS:
//      hwnd    - handle of the appbar to register.
//
//  RETURN VALUE:
//      Returns TRUE if successful, FALSE otherwise.
//
//  COMMENTS:
//      Sets the system wide g_fAppRegistered variable.
//

BOOL AppBar_UnRegister(HWND hwnd)
{
    APPBARDATA abd;
    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = hwnd;

    g_fAppRegistered = !SHAppBarMessage(ABM_REMOVE, &abd);

    return !g_fAppRegistered;
}


//
//  FUNCTION:   AppBar_SetAutoHide(HWND, BOOL)
//
//  PURPOSE:    Causes the appbar window to either auto hide or stop auto
//              hiding.
//
//  PARAMETERS:
//      hwnd    - Handle of the appbar window to change the auto hide state.
//      fHide   - TRUE if we want the window to autohide, FALSE to stop.
//
//  RETURN VALUE:
//      Returns TRUE if successful, FALSE otherwise.
//

BOOL AppBar_SetAutoHide(HWND hwnd, BOOL fHide)
{
    if (fHide)
    {
        return AppBar_AutoHide(hwnd);
    }
    else
    {
        return AppBar_NoAutoHide(hwnd);
    }
}


//
//  FUNCTION:   AppBar_AutoHide(HWND)
//
//  PURPOSE:    Does the work of changing the appbar to autohide.  We check
//              to see if we can autohide, and if so unregister and tell
//              the system we are autohiding.
//
//  PARAMETERS:
//      hwnd    - Window handle of the appbar.
//
//  RETURN VALUE:
//      TRUE if successful, FALSE otherwise.
//

BOOL AppBar_AutoHide(HWND hwnd)
{
    POPTIONS pOpt = GetAppbarData(hwnd);
    APPBARDATA abd;
    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = hwnd;
    abd.uEdge = pOpt->uSide;

    // First figure out if someone already has this side for
    // autohiding
    HWND hwndAutoHide = (HWND) SHAppBarMessage(ABM_GETAUTOHIDEBAR, &abd);
    if (hwndAutoHide != NULL)
    {
        return FALSE;
    }

    // We can autohide on this edge.  Set the autohide style.
    abd.lParam = TRUE;

    BOOL fSuccess = (BOOL) SHAppBarMessage(ABM_SETAUTOHIDEBAR, &abd);
    if (!fSuccess)
    {
        ErrorHandler();
        return FALSE;
    }
    else
    {
        // Since we're allowed to autohide, we need to adjust our screen
        // rectangle to the autohidden position.  This will allow the system
        // to resize the desktop.
        pOpt->fAutoHide = TRUE;
        g_cxWidth = pOpt->cxWidth;
        g_cyHeight = pOpt->cyHeight;

        RECT rc;
        rc = g_rcAppBar;
        switch (pOpt->uSide)
        {
            case ABE_TOP:
                rc.bottom = rc.top + 2;
                break;
            case ABE_BOTTOM:
                rc.top = rc.bottom - 2;
                break;
            case ABE_LEFT:
                rc.right = rc.left + 2;
                break;
            case ABE_RIGHT:
                rc.left = rc.right - 2;
                break;
        }

        AppBar_QuerySetPos(pOpt->uSide, &rc, &abd, TRUE);
    }

    return TRUE;
}


//
//  FUNCTION:   AppBar_NoAutoHide(HWND)
//
//  PURPOSE:    Does the work of changing the appbar to no-autohide.  We
//              check to make sure we are actually auto hiding, and if so
//              re-register the appbar to get our desktop scren space back.
//
//  PARAMETERS:
//      hwnd    - Window handle of the appbar.
//
//  RETURN VALUE:
//      TRUE if successful, FALSE otherwise.
//

BOOL AppBar_NoAutoHide(HWND hwnd)
{
    POPTIONS pOpt = GetAppbarData(hwnd);
    APPBARDATA abd;
    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = hwnd;
    abd.uEdge = pOpt->uSide;

    // First let's check to see if we're the appbar attached to the
    // side of the screen
    abd.uEdge = pOpt->uSide;
    HWND hwndAutoHide = (HWND) SHAppBarMessage(ABM_GETAUTOHIDEBAR, &abd);
    if (hwndAutoHide != hwnd)
    {
        return FALSE;
    }

    // We can autohide or stop autohide on this edge.  Set the autohide style.
    abd.lParam = FALSE;

    BOOL fSuccess = (BOOL) SHAppBarMessage(ABM_SETAUTOHIDEBAR, &abd);
    if (!fSuccess)
    {
        ErrorHandler();
        return FALSE;
    }
    else
    {
        // Need to change the appbar to get the screen desktop space
        // back.  Also need to reattach the appbar to that edge of the
        // screen.
        pOpt->fAutoHide = FALSE;
        pOpt->cxWidth = g_cxWidth;
        pOpt->cyHeight = g_cyHeight;

        AppBar_SetSide(hwnd, pOpt->uSide);
    }

    return TRUE;
}


//
//  FUNCTION:   AppBar_SetSide(HWND, UINT)
//
//  PURPOSE:    Sets the side the AppBar is currently attached to.
//
//  PARAMETERS:
//      hwnd    - Window handle of the appbar.
//      uSide   - Side of the screen to attach to.  Can be ABE_TOP, ABE_BOTTOM,
//                ABE_LEFT, or ABE_RIGHT.
//
//  RETURN VALUE:
//      TRUE if successful, FALSE otherwise.
//

BOOL AppBar_SetSide(HWND hwnd, UINT uSide)
{
    // Calculate the size of the screen so we can occupy the full width or
    // height of the screen on the edge we request.
    RECT rc;
    rc.top = 0;
    rc.left = 0;
    rc.right = GetSystemMetrics(SM_CXSCREEN);
    rc.bottom = GetSystemMetrics(SM_CYSCREEN);

    // Fill out the APPBARDATA struct with the basic information
    APPBARDATA abd;
    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = hwnd;

    // If the appbar is autohidden, turn that off so we can move the bar
    BOOL fAutoHide = FALSE;
    POPTIONS pOpt = GetAppbarData(hwnd);
    if (pOpt->fAutoHide)
    {
        fAutoHide = pOpt->fAutoHide;

        // Turn off the redrawing of the desktop while we move things around.
        // If you put any breakpoints in this area you will lock up the desktop
        // Since turning off the desktop repaints turns it off for all the apps
        // in the system
        SetWindowRedraw(GetDesktopWindow(), FALSE);
        AppBar_SetAutoHide(hwnd, FALSE);
        pOpt->fHiding = FALSE;
    }

    // Adjust the rectangle to set our height or width depending on the
    // side we want.
    switch (uSide)
    {
        case ABE_TOP:
            rc.bottom = rc.top + pOpt->cyHeight;
            break;
        case ABE_BOTTOM:
            rc.top = rc.bottom - pOpt->cyHeight;
            break;
        case ABE_LEFT:
            rc.right = rc.left + pOpt->cxWidth;
            break;
        case ABE_RIGHT:
            rc.left = rc.right - pOpt->cxWidth;
            break;
    }

    // Move the appbar to the new screen space.
    AppBar_QuerySetPos(uSide, &rc, &abd, TRUE);

    // If the appbar was hidden, rehide it now
    if (fAutoHide)
    {
        AppBar_SetAutoHide(hwnd, TRUE);
        pOpt->fHiding = TRUE;

        SetWindowRedraw(GetDesktopWindow(), TRUE);
        RedrawWindow(GetDesktopWindow(), NULL, NULL,
                     RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    }

    return TRUE;
}


//
//  FUNCTION:   AppBar_SetAlwaysOnTop(HWND, BOOL)
//
//  PURPOSE:    Sets the Always-On-Top state for the appbar.
//
//  PARAMETERS:
//      hwnd    - Window handle of the AppBar to set the state for.
//      fOnTop  - TRUE to set the AppBar to Always-On-Top, FALSE otherwise
//

void AppBar_SetAlwaysOnTop(HWND hwnd, BOOL fOnTop)
{
    POPTIONS pOpt = GetAppbarData(hwnd);

    // Update the window position to HWND_TOPMOST if we're to be always
    // on top, or HWND_NOTOPMOST if we're not.
    SetWindowPos(hwnd, (fOnTop) ? HWND_TOPMOST : HWND_NOTOPMOST,
                 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    // Store the setting in the appbar OPTIONS struct.
    pOpt->fOnTop = fOnTop;
}


//
//  FUNCTION:   AppBar_Hide(HWND)
//
//  PURPOSE:    Causes the appbar to hide itself.
//
//  PARAMETERS:
//      hwnd    - Handle of the AppBar to hide.
//

void AppBar_Hide(HWND hwnd)
{
    POPTIONS pOpt = GetAppbarData(hwnd);

    // Don't want to hide if AutoHide not set
    if (!pOpt->fAutoHide)
    {
        return;
    }

    // Calculate a hidden rectangle to use
    RECT rc = g_rcAppBar;
    switch (pOpt->uSide)
    {
        case ABE_TOP:
            rc.bottom = rc.top + 2;
            break;
        case ABE_BOTTOM:
            rc.top = rc.bottom - 2;
            break;
        case ABE_LEFT:
            rc.right = rc.left + 2;
            break;
        case ABE_RIGHT:
            rc.left = rc.right - 2;
            break;
    }

    pOpt->fHiding = TRUE;
    SlideWindow(hwnd, &rc);
}


//
//  FUNCTION:   AppBar_UnHide(HWND)
//
//  PURPOSE:    Causes a hidden appbar to unhide itself.
//
//  PARAMETERS:
//      hwnd    - Handle of the appbar to unhide.
//

void AppBar_UnHide(HWND hwnd)
{
    POPTIONS pOpt = GetAppbarData(hwnd);

    SlideWindow(hwnd, &g_rcAppBar);
    pOpt->fHiding = FALSE;

    AppBar_SetAutoHideTimer(hwnd);
}


//
//  FUNCTION:   AppBar_SetAutoHideTimer(HWND)
//
//  PURPOSE:    Starts the auto hide timer.
//
//  PARAMETERS:
//      hwnd    - Handle of the appbar to set the timer for.
//
//  COMMENTS:
//      This is called to cause a delay between when the user's mouse leaves
//      the appbar area and the appbar is slid off the screen.
//

void AppBar_SetAutoHideTimer(HWND hwnd)
{
    POPTIONS pOpt = GetAppbarData(hwnd);

    if (pOpt->fAutoHide)
    {
        SetTimer(hwnd, IDT_AUTOHIDE, 500, NULL);
    }
}


//
//  FUNCTION:   AppBar_SetAutoUnhideTimer(HWND)
//
//  PURPOSE:    Starts the auto-UNhide timer.
//
//  PARAMETERS:
//      hwnd    - Handle of the appbar to set the timer for.
//
//  COMMENTS:
//      This is called to cause a delay between the time where the mouse enters
//      the appbar window and the time where the appbar displays itself.
//

void AppBar_SetAutoUnhideTimer(HWND hwnd)
{
    POPTIONS pOpt = GetAppbarData(hwnd);

    if (pOpt->fAutoHide && pOpt->fHiding)
    {
        SetTimer(hwnd, IDT_AUTOUNHIDE, 50, NULL);
    }
}


//
//  FUNCTION:   GetAppbarData(HWND)
//
//  PURPOSE:    Retrieves a pointer to the OPTIONS structure stored in the
//              window's extra bytes.
//
//  PARAMETERS:
//      hwnd    - Handle of the window to retrieve the pointer from.
//
//  RETURN VALUE:
//      Returns a pointer to an OPTIONS struct
//

POPTIONS GetAppbarData(HWND hwnd)
{
    return (POPTIONS) GetWindowLongPtr(hwnd, 0);
}


//
//  FUNCTION:   SlideWindow(HWND, LPRECT)
//
//  PURPOSE:    Slides the AppBar off the edge of the screen when the AppBar
//              has the AutoHide state set.
//
//  PARAMETERS:
//      hwnd    - handle of the window to scroll off the screen
//      prc     - rectangle you wish the appbar to occupy
//
//  COMMENTS:
//

void SlideWindow(HWND hwnd, LPRECT prc)
{
    RECT rcNew = *prc;

    RECT rcOld;
    GetWindowRect(hwnd, &rcOld);

    BOOL fShow = (rcNew.bottom - rcNew.top) > (rcOld.bottom - rcOld.top) ||
                 (rcNew.right - rcNew.left) > (rcOld.right - rcOld.left);

    int dx = (rcNew.right - rcOld.right) + (rcNew.left - rcOld.left);
    int dy = (rcNew.bottom - rcOld.bottom) + (rcNew.top - rcOld.top);

    int dt;
    if (fShow)
    {
        rcOld = rcNew;
        OffsetRect(&rcOld, -dx, -dy);
        SetWindowPos(hwnd, NULL, rcOld.left, rcOld.top,
                rcOld.right - rcOld.left, rcOld.bottom - rcOld.top,
                SWP_NOZORDER | SWP_NOACTIVATE | SWP_DRAWFRAME);

        dt = g_dtSlideShow;
    }
    else
    {
        dt = g_dtSlideHide;
    }

    HANDLE hThreadMe = GetCurrentThread();
    int priority = GetThreadPriority(hThreadMe);
    SetThreadPriority(hThreadMe, THREAD_PRIORITY_HIGHEST);

    int t, t0 = GetTickCount();
    while ((t = GetTickCount()) < t0 + dt)
    {
        int x = rcOld.left + dx * (t - t0) / dt;
        int y = rcOld.top + dy * (t - t0) / dt;

        SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        if (fShow)
        {
            UpdateWindow(hwnd);
        }
        else
        {
            UpdateWindow(GetDesktopWindow());
        }
    }

    SetThreadPriority(hThreadMe, priority);

    SetWindowPos(hwnd, NULL, rcNew.left, rcNew.top,
                 rcNew.right - rcNew.left, rcNew.bottom - rcNew.top,
                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_DRAWFRAME);
}

//
//  FUNCTION:   ErrorHandlerEx(INT, LPSTR)
//
//  PURPOSE:    Reports that an error has occurred.
//
//  PARAMETERS:
//      wLine   - Source file line number
//      pszFile - Source file path
//

void ErrorHandlerEx( INT wLine, LPSTR pszFile )
{
    // Get the text of the error message
    LPWSTR pszMessage;
    DWORD dwError = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                            FORMAT_MESSAGE_FROM_SYSTEM,
                            NULL,
                            GetLastError(),
                            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                            (LPWSTR)&pszMessage,
                            0,
                            NULL);

    // Check to see if an error occured calling FormatMessage()
    WCHAR szBuffer[256];
    if (0 == dwError)
    {
        StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), L"An error occured calling FormatMessage(). Error Code %d", GetLastError());
        MessageBoxW(NULL, szBuffer, L"Generic", MB_ICONSTOP | MB_ICONEXCLAMATION);
    }
    else
    {
        // Display the error message
        StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), L"Generic, Line=%d, File=%s", wLine, pszFile);
        MessageBox(NULL, pszMessage, szBuffer, MB_ICONEXCLAMATION | MB_OK);
    }
}

//
//  FUNCTION:   wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
//
//  PURPOSE:    Main entry point of the AppBar sample application. Creates
//              the appbar window and pumps messages.
//
//  PARAMETERS:
//      hInstance       - Instance handle of the application
//      (others are unused)
//
//  RETURN VALUE:
//      Process exit code.
//

int PASCAL wWinMain(HINSTANCE hInstance,
                    HINSTANCE /* hPrevInstance */,
                    LPWSTR /* lpszCmdLine */,
                    int /* nCmdShow */)
{
    g_hInstance = hInstance;

    WCHAR const szWindowClass[] = L"MSSampleAppbar";    // The main window class name

    WNDCLASSEX wcex = { sizeof(wcex) };
    wcex.lpfnWndProc = MainWndProc;
    wcex.hInstance = g_hInstance;
    wcex.cbWndExtra = sizeof(LPVOID);
    wcex.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_APPICON));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_APPICON),
                                  IMAGE_ICON,
                                  GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
                                  0);

    RegisterClassEx(&wcex);

    // Create a main window for this application instance
    HWND hwnd = CreateWindowEx(WS_EX_CLIENTEDGE | WS_EX_TOOLWINDOW,
                            szWindowClass,
                            L"Win32 Sample AppBar",
                            WS_POPUP | WS_THICKFRAME | WS_CLIPCHILDREN,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            400,
                            200,
                            NULL,
                            NULL,
                            hInstance,
                            NULL
                            );

    // If the window was successfully created, make the window visible,
    // update its client area, and return "success".  If the window
    // was not created, return "failure"
    int iResult = -1;

    if (hwnd)
    {
        ShowWindow(hwnd, SW_SHOWDEFAULT); // Set to visible & paint non-client area
        UpdateWindow(hwnd);         // Tell window to paint client area

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);   // Translates virtual key codes
            DispatchMessage(&msg);    // Dispatches message to window procedure
        }
        iResult = ((int)msg.wParam);
    }

    return iResult;
}
