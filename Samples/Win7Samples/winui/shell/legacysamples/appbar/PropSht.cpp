// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "appbar.h"


//////////////////////////////////////////////////////////////////////////////
// Global variables visible only to this file

static POPTIONS s_pOptions;     // Appbar options used throughout the options property page.
static HWND s_hwndAppbar;       // Appbar window handle


//////////////////////////////////////////////////////////////////////////////
// Local Prototypes

INT_PTR CALLBACK OptionsPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void Options_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
INT_PTR Options_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void Options_OnDestroy(HWND hwnd);
LRESULT Options_OnNotify(HWND hwnd, int idFrom, LPNMHDR pnmhdr);


//
//  FUNCTION:   ShowOptions(HWND)
//
//  PURPOSE:    Creates and displays the appbar's property sheet.
//
//  PARAMETERS:
//      hwndParent - Handle to the parent window for the prop sheet.
//

void ShowOptions(HWND hwndParent)
{
    // Store the window handle of the appbar so we can send it commands
    // when the options change
    s_hwndAppbar = hwndParent;

    // Initialize the memory
    PROPSHEETPAGE psp[2];
    ZeroMemory(psp, sizeof(PROPSHEETPAGE) * 2);

    // Fill out the property sheet pages
    psp[0].dwSize      = sizeof(PROPSHEETPAGE);
    psp[0].dwFlags     = PSP_USETITLE;
    psp[0].hInstance   = g_hInstance;
    psp[0].pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS);
    psp[0].pszTitle    = L"AppBar Options";
    psp[0].pfnDlgProc  = OptionsPageProc;
    psp[0].lParam      = (LPARAM)((POPTIONS)GetAppbarData(hwndParent));

    // Now fill out the property sheet header
    PROPSHEETHEADER psh;
    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_PROPTITLE;
    psh.hwndParent = hwndParent;
    psh.hInstance = g_hInstance;
    psh.pszCaption = L"AppBar";
    psh.nPages = 1;
    psh.ppsp = psp;

    // Finally display the property sheet
    PropertySheet(&psh);
}


//  PURPOSE:    Dispatches messages for the options property page.
//

INT_PTR CALLBACK OptionsPageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            return HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Options_OnInitDialog);

        case WM_COMMAND:
            HANDLE_WM_COMMAND(hwnd, wParam, lParam, Options_OnCommand);
            return TRUE;

        case WM_DESTROY:
            HANDLE_WM_DESTROY(hwnd, wParam, lParam, Options_OnDestroy);
            return TRUE;

        case WM_NOTIFY:
            HANDLE_WM_NOTIFY(hwnd, wParam, lParam, Options_OnNotify);
            return TRUE;
    }

    return FALSE;
}

//  PURPOSE:    Handles initialization of the property page.
//
//  PARAMETERS:
//      hwnd      - Handle of the property page window.
//      hwndFocus - Child window that will receive focus if the function
//                  returns TRUE.
//      lParam    - Will contain a pointer to the appbar OPTIONS struct.
//
//  RETURN VALUE:
//      Returns TRUE to set the focus to the child window contained in the
//      hwndFocus parameter.  Returns FALSE to set the focus ourselves.
//

INT_PTR Options_OnInitDialog(HWND hwnd, HWND /* hwndFocus */, LPARAM lParam)
{
    // Get the pointer to the OPTIONS struct from the lParam
    PROPSHEETPAGE *ppsp = (PROPSHEETPAGE *)lParam;
    s_pOptions = (POPTIONS)ppsp->lParam;

    // Load and set the picture for the base image
    HBITMAP hbmpBase = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_BASE));
    if (!hbmpBase)
    {
        ErrorHandler();
        EndDialog(hwnd, 0);
    }
    SendDlgItemMessage(hwnd, IDC_PICTURE, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hbmpBase);

    // Set the pictures for the rest of the static controls, move them
    // into position, and hide them
    HBITMAP hbmpAppbar = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_APPBAR));
    HBITMAP hbmpWindow = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_WINDOW));
    if (!hbmpAppbar || !hbmpWindow)
    {
        ErrorHandler();
        EndDialog(hwnd, 0);
    }
    SendDlgItemMessage(hwnd, IDC_APPBAR, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hbmpAppbar);
    SendDlgItemMessage(hwnd, IDC_WINDOW, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hbmpWindow);

    // Get the position of the picture and convert to client coordiates
    RECT rc;
    GetWindowRect(GetDlgItem(hwnd, IDC_PICTURE), &rc);
    ScreenToClient(hwnd, (LPPOINT)&rc);

    SetWindowPos(GetDlgItem(hwnd, IDC_APPBAR), GetDlgItem(hwnd, IDC_PICTURE),
                 rc.left + 2, rc.top + 69, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);

    SetWindowPos(GetDlgItem(hwnd, IDC_WINDOW), GetDlgItem(hwnd, IDC_APPBAR),
                 rc.left + 212, rc.top + 69, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);

    // Set the values of the controls based on the values in OPTIONS
    Button_SetCheck(GetDlgItem(hwnd, IDC_AUTOHIDE), s_pOptions->fAutoHide);
    Button_SetCheck(GetDlgItem(hwnd, IDC_ONTOP), s_pOptions->fOnTop);

    if (s_pOptions->fAutoHide)
    {
        ShowWindow(GetDlgItem(hwnd, IDC_APPBAR), SW_HIDE);
    }
    if (s_pOptions->fOnTop)
    {
        ShowWindow(GetDlgItem(hwnd, IDC_WINDOW), SW_HIDE);
    }

    // Check the autohide window for this side.  Since only one appbar can be
    // auto hidden on each side of the screen, we should disable this check box
    // if we can't autohide.
    APPBARDATA abd;
    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = s_hwndAppbar;
    abd.uEdge = s_pOptions->uSide;

    HWND hwndAutohide = (HWND) SHAppBarMessage(ABM_GETAUTOHIDEBAR, &abd);
    if ((hwndAutohide != NULL) && (hwndAutohide != s_hwndAppbar))
    {
        EnableWindow(GetDlgItem(hwnd, IDC_AUTOHIDE), FALSE);
    }

    return FALSE;
}


//
//  FUNCTION:   Options_OnCommand(HWND, int, HWND, UINT)
//
//  PURPOSE:    Handles the command messages for the options page.
//
//  PARAMETERS:
//      hwnd        - handle of the window receiving the message
//      id          - identifier of the command
//      hwndCtl     - handle of the control sending the message)
//      codeNotify  - specifies the notification code if the message is from
//                    a control
//
//  COMMENTS:
//      codeNotify is 1 if from an accelerator, 0 if from a menu.
//      If the message is not from a control hwndCtl is NULL.
//

void Options_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT /* codeNotify */)
{
    switch (id)
    {
        case IDC_AUTOHIDE:
            // First let the property sheet know somethings changed
            PropSheet_Changed(GetParent(hwnd), hwnd);

            // Now update the picture
            if (Button_GetCheck(hwndCtl))
            {
                ShowWindow(GetDlgItem(hwnd, IDC_APPBAR), SW_HIDE);
            }
            else
            {
                ShowWindow(GetDlgItem(hwnd, IDC_APPBAR), SW_SHOW);
            }
            break;

        case IDC_ONTOP:
            // First let the property sheet know somethings changed
            PropSheet_Changed(GetParent(hwnd), hwnd);

            // Now update the picture
            if (Button_GetCheck(hwndCtl))
            {
                ShowWindow(GetDlgItem(hwnd, IDC_WINDOW), SW_HIDE);
            }
            else
            {
                ShowWindow(GetDlgItem(hwnd, IDC_WINDOW), SW_SHOW);
            }
            break;
    }
}


//
//  FUNCTION:   Options_OnDestroy(HWND)
//
//  PURPOSE:    Clean up the GDI objects we created in the OnInitDialog.
//
//  PARAMETERS:
//      hwnd    - Handle of the options property page.
//

void Options_OnDestroy(HWND hwnd)
{
    HBITMAP hbmp = (HBITMAP) SendDlgItemMessage(hwnd, IDC_PICTURE, STM_GETIMAGE, IMAGE_BITMAP, 0L);
    if (hbmp)
    {
        DeleteObject(hbmp);
    }

    hbmp = (HBITMAP) SendDlgItemMessage(hwnd, IDC_APPBAR, STM_GETIMAGE, IMAGE_BITMAP, 0L);
    if (hbmp)
    {
        DeleteObject(hbmp);
    }

    hbmp = (HBITMAP) SendDlgItemMessage(hwnd, IDC_WINDOW, STM_GETIMAGE, IMAGE_BITMAP, 0L);
    if (hbmp)
    {
        DeleteObject(hbmp);
    }
}


//
//  FUNCTION:   Options_OnNotify(HWND, int, LPNMHDR)
//
//  PURPOSE:    Handles updating the appbar states when the user either presses
//              the "Apply" button or the "OK" button.
//
//  PARAMETERS:
//      hwnd    - handle of the window receiving the notification
//      idCtl   - identifies the control sending the notification
//      pnmh    - points to a NMHDR struct with more inforamation regarding the
//                notification
//
//  RETURN VALUE:
//      Always zero.
//

LRESULT Options_OnNotify(HWND hwnd, int /* idFrom */, LPNMHDR pnmhdr)
{
    switch (pnmhdr->code)
    {
        case PSN_APPLY:
        {
            // Check to see if the options have changed.  If so, send
            // the appropriate command to the appbar
            BOOL fCheck = (BOOL) Button_GetCheck(GetDlgItem(hwnd, IDC_AUTOHIDE));
            if (s_pOptions->fAutoHide != fCheck)
            {
                if (AppBar_SetAutoHide(s_hwndAppbar, fCheck))
                {
                    s_pOptions->fAutoHide = fCheck;
                }
            }

            // If the Always-On-Top setting has changed update the appbar state
            fCheck = (BOOL) Button_GetCheck(GetDlgItem(hwnd, IDC_ONTOP));
            if (s_pOptions->fOnTop != fCheck)
            {
                s_pOptions->fOnTop = fCheck;
                AppBar_SetAlwaysOnTop(s_hwndAppbar, fCheck);
            }

            SetDlgMsgResult(hwnd, WM_NOTIFY, PSNRET_NOERROR);
            break;
        }
    }

    return 0;
}
