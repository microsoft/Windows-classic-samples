// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright 1993 - 2000 Microsoft Corporation.  All Rights Reserved.
//
//  MODULE:   ipxchat.c
//
//  PURPOSE:   Implement the windows procedure for the main application
//    windows.  Also implement the generic message and command dispatchers.
//
//  FUNCTIONS:
//    WndProc           - Processes messages for the main window.
//    MsgCreate         - Initializes Edit Controls for text input/output.
//    MsgSize           - Adjusts size of Edit Controls when window is resized.
//    MsgSetfocus       - Keeps window focus on edit control instead of parent.
//    MsgDataready      - Reads data from incoming IPC mechanism.
//    MsgRefreshdisplay - Refills Inbox edit control text contents
//    MsgDisconnected   - Cleans up connection killed by other side
//    MsgCommand        - Handle the WM_COMMAND messages for the main window.
//    MsgDestroy        - Handles the WM_DESTROY message by calling 
//                        PostQuitMessage().
//    CmdOutbox         - Handles messages from Outbox edit control.
//    CmdDisconnected   - Disconnects current connection
//    CmdExit           - Handles the file exit command by calling destroy 
//                        window on the main window.
//
//  COMMENTS:
//

#include "globals.h"            // prototypes specific to this application
#include <windows.h>            // required for all Windows applications
#include <windowsx.h>




// Main window message table definition.
MSD rgmsd[] =
{
    {WM_CREATE,         MsgCreate},
    {WM_SIZE,           MsgSize},
    {WM_SETFOCUS,       MsgSetfocus},
    {WM_COMMAND,        MsgCommand},
    {WM_DESTROY,        MsgDestroy},
    {MW_DATAREADY,      MsgDataready},
    {MW_DISCONNECTED,   MsgDisconnected},
    {MW_DISPLAYREFRESH, MsgRefreshdisplay}
};

MSDI msdiMain =
{
    sizeof(rgmsd) / sizeof(MSD),
    rgmsd,
    edwpWindow
};


// Main window command table definition.
CMD rgcmd[] =
{
    {IDM_CONNECT,    CmdConnect},
    {IDM_LISTEN,     CmdListen},
    {IDM_EXIT,       CmdExit},
    {IDM_ABOUT,      CmdAbout},
    {ID_OUTBOX,      CmdOutbox},
    {IDM_DISCONNECT, CmdDisconnect}
};

CMDI cmdiMain =
{
    sizeof(rgcmd) / sizeof(CMD),
    rgcmd,
    edwpWindow
};


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  PARAMETERS:
//    hwnd     - window handle
//    uMessage - message number
//    wparam   - additional information (dependant on message number)
//    lparam   - additional information (dependant on message number)
//
//  RETURN VALUE:
//    The return value depends on the message number.  If the message
//    is implemented in the message dispatch table, the return value is
//    the value returned by the message handling function.  Otherwise,
//    the return value is the value returned by the default window procedure.
//
//  COMMENTS:
//    Call the DispMessage() function with the main window's message dispatch
//    information (msdiMain) and the message specific information.
//

LRESULT CALLBACK WndProc(HWND   hwnd, 
                         UINT   uMessage, 
                         WPARAM wparam, 
                         LPARAM lparam)
{
    return DispMessage(&msdiMain, hwnd, uMessage, wparam, lparam);
}

//
//  FUNCTION: MsgCreate(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Creates Inbox and Outbox Edit controls
//
//  PARAMETERS:
//
//    hwnd      - Window handle
//    uMessage  - Message number (Unused)
//    wparam    - Extra data     (Unused)
//    lparam    - Extra data     (Unused)
//
//  RETURN VALUE:
//
//    Always returns 0 - Message handled
//
//  COMMENTS:
//
//

#pragma warning (push)
#pragma warning (disable:4312)

LRESULT MsgCreate(HWND hwnd, UINT uMessage, WPARAM wparam, LPARAM lparam)
{
    lparam;
    wparam;
    uMessage;

    // Create Edit control for typing to be sent to server
    if (NULL == (hOutWnd = CreateWindow("EDIT",
                           NULL,
                           WS_BORDER | WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | 
                           ES_MULTILINE | ES_AUTOVSCROLL,
                           0,0,0,0,
                           hwnd,
                           (HMENU) ID_OUTBOX,
                           (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                           NULL)))
        return FALSE;
    // Create Edit control for typing to be received from server
    if (NULL == (hInWnd = CreateWindow("EDIT",
                          NULL,
                          WS_BORDER | WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | 
                          ES_MULTILINE | ES_AUTOVSCROLL,
                          0,0,0,0,
                          hwnd,
                          (HMENU) ID_INBOX,
                          (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                          NULL)))
        return FALSE;

    return TRUE;
}
#pragma warning (pop)

//
//  FUNCTION: MsgSize(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Adjust Size of Inbox and Outbox Edit controls
//
//  PARAMETERS:
//
//    hwnd      - Window handle
//    uMessage  - Message number (Unused)
//    wparam    - Extra data     (Unused)
//    lparam    - Parent Window Size
//
//  RETURN VALUE:
//
//    Always returns 0 - Message handled
//
//  COMMENTS:
//
//

LRESULT MsgSize(HWND hwnd, UINT uMessage, WPARAM wparam, LPARAM lparam)
{
    wparam;
    uMessage;
    hwnd;
    // Size OutBox Edit Control
    if (FALSE == MoveWindow(hOutWnd,
               1,1,                    // Upper Left Corner
               LOWORD(lparam) - 2,         // Width of Parent Window
               (HIWORD(lparam) / 2) - 2 ,     // Half the height of Parent
               TRUE))                  // repaint
        return FALSE;

    // Size Inbox Edit Control
    if(FALSE == MoveWindow(hInWnd,
               1, (HIWORD(lparam) / 2) + 1,  // Half Way down right side
               LOWORD(lparam) - 2,         // Width of Parent Window
               (HIWORD(lparam) / 2) - 2,      // Half the height of Parent
               TRUE))                  // repaint
        return FALSE;

    return TRUE;
}

//
//  FUNCTION: MsgSetfocus(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Keeps Window focus on edit control
//
//  PARAMETERS:
//
//    hwnd      - Window handle
//    uMessage  - Message number (Unused)
//    wparam    - Extra data     (Unused)
//    lparam    - Extra data     (Unused)
//
//  RETURN VALUE:
//
//    Always returns 0 - Message handled
//
//  COMMENTS:
//
//

LRESULT MsgSetfocus(HWND hwnd, UINT uMessage, WPARAM wparam, LPARAM lparam)
{
    lparam;
    wparam;
    uMessage;
    hwnd;

    if(NULL == SetFocus(hOutWnd))  // Don't let main window have focus
        return FALSE;

    return TRUE;
}

//
//  FUNCTION: MsgDataready(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Read data and post message to display data
//
//  PARAMETERS:
//
//    hwnd      - Window handle
//    uMessage  - Message number (Unused)
//    wparam    - forwarded to ReceiveInBox
//    lparam    - forwarded to ReceiveInBox
//
//  RETURN VALUE:
//
//    Always returns 0 - Message handled
//
//  COMMENTS:
//
//

LRESULT MsgDataready(HWND hwnd, UINT uMessage, WPARAM wparam, LPARAM lparam)
{
    uMessage;

    // Call protocol specific program to Read data and put in szRcvBuf
    if(ReceiveInBox(hwnd, wparam, lparam, szRcvBuf, sizeof(szRcvBuf)))
    {
        PostMessage(hwnd, MW_DISPLAYREFRESH, 0, 0);
    }
    return TRUE;
}
//
//  FUNCTION: MsgRefreshdisplay(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Display socket data in inbox
//
//  PARAMETERS:
//
//    hwnd      - Window handle
//    uMessage  - Message number (Unused)
//    wparam    - Extra Data     (Unused)
//    lparam    - Extra Data     (Unused)
//
//  RETURN VALUE:
//
//    Always returns 0 - Message handled
//
//  COMMENTS:
//
//

LRESULT MsgRefreshdisplay(HWND hwnd, UINT uMessage, WPARAM wparam, LPARAM lparam)
{
    MSG peekmsg;

    lparam;
    wparam;
    uMessage;

    // Don't bother displaying if there is already another update queued
    if(PeekMessage(&peekmsg, hwnd, MW_DATAREADY, MW_DISPLAYREFRESH, PM_NOREMOVE))
    {
       return TRUE;                          
    }
    // Put data in szRcvBuf in InBox
    SendDlgItemMessage(hwnd,
                       ID_INBOX,
                       EM_SETSEL,
                       0,-1);
    SendDlgItemMessage(hwnd,
                       ID_INBOX,
                       EM_REPLACESEL,
                       0,
                       (LPARAM)szRcvBuf);
    return TRUE;
}

//
//  FUNCTION: MsgDisconnected(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Cleanup connection dropped from other side
//
//  PARAMETERS:
//
//    hwnd      - Window handle
//    uMessage  - Message number (Unused)
//    wparam    - Extra data     (Unused)
//    lparam    - Extra data     (Unused)
//
//  RETURN VALUE:
//
//    Always returns 0 - Message handled
//
//  COMMENTS:
//
//

LRESULT MsgDisconnected(HWND hwnd, UINT uMessage, WPARAM wparam, LPARAM lparam)
{
    HMENU hmenu;

    lparam;
    wparam;
    uMessage;

    // Let the user know
    MessageBox(hwnd,
               "Connection Disconnected",
               "Chat Dialog Stopped",
               MB_OK);
    // Protocol Specific Cleanup
    CleanUp();
    // Fix menus
    if(NULL == (hmenu = GetMenu(hwnd)))
        return FALSE;
    if(FALSE == EnableMenuItem(hmenu, IDM_CONNECT, MF_ENABLED))
        return FALSE;
    if(FALSE == EnableMenuItem(hmenu, IDM_LISTEN, MF_ENABLED))
        return FALSE;
    if(FALSE == EnableMenuItem(hmenu, IDM_DISCONNECT, MF_GRAYED))
        return FALSE;
    if(FALSE == SetWindowText(hwnd, szTitle))
        return FALSE;

    return TRUE;
}

//
//  FUNCTION: MsgCommand(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Handle the WM_COMMAND messages for the main window.
//
//  PARAMETERS:
//    hwnd     - window handle
//    uMessage - WM_COMMAND (Unused)
//    GET_WM_COMMAND_ID(wparam, lparam)   - Command identifier
//    GET_WM_COMMAND_HWND(wparam, lparam) - Control handle
//
//  RETURN VALUE:
//    The return value depends on the message number.  If the message
//    is implemented in the message dispatch table, the return value is
//    the value returned by the message handling function.  Otherwise,
//    the return value is the value returned by the default window procedure.
//
//  COMMENTS:
//    Call the DispCommand() function with the main window's command dispatch
//    information (cmdiMain) and the command specific information.
//

LRESULT MsgCommand(HWND hwnd, UINT uMessage, WPARAM wparam, LPARAM lparam)
{
    uMessage;

    return DispCommand(&cmdiMain, hwnd, wparam, lparam);
}


//
//  FUNCTION: MsgDestroy(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Calls PostQuitMessage().
//
//  PARAMETERS:
//
//    hwnd      - Window handle  (Unused)
//    uMessage  - Message number (Unused)
//    wparam    - Extra data     (Unused)
//    lparam    - Extra data     (Unused)
//
//  RETURN VALUE:
//
//    Always returns 0 - Message handled
//
//  COMMENTS:
//
//

LRESULT MsgDestroy(HWND hwnd, UINT uMessage, WPARAM wparam, LPARAM lparam)
{
    lparam;
    wparam;
    uMessage;
    hwnd;

    PostQuitMessage(0);
    return 0;
}

//
//  FUNCTION: CmdOutbox(HWND, WORD, WORD, HWND)
//
//  PURPOSE: Handle messages from Outbox--Send data if EN_CHANGE
//
//  PARAMETERS:
//    hwnd     - The window.
//    wCommand - ID_OUTBOX (unused)
//    wNotify  - Notification number (unused)
//    hwndCtrl - NULL (unused)
//
//  RETURN VALUE:
//    Always returns 0 - command handled.
//
//  COMMENTS:
//
//

LRESULT CmdOutbox(HWND hwnd, WORD wCommand, WORD wNotify, HWND hwndCtrl)
{
    int cSendLen;

    hwndCtrl;
    wCommand;

    if (wNotify != EN_CHANGE)  // We only care if text has changed
    {
        return 0;
    }
    // Text has changed!  Put OutBox's text in szSndBuf
    cSendLen = GetDlgItemText(hwnd,
                              ID_OUTBOX,
                              szSndBuf,
                              sizeof(szSndBuf));

    // Do protocol specific send
    SendOutBox(szSndBuf, cSendLen);
    return 0;
}

//
//  FUNCTION: CmdDisconnect(HWND, WORD, WORD, HWND)
//
//  PURPOSE: Cut off session and fix menus appropriately
//
//  PARAMETERS:
//    hwnd     - The window.
//    wCommand - ID_OUTBOX (unused)
//    wNotify  - Notification number (unused)
//    hwndCtrl - NULL (unused)
//
//  RETURN VALUE:
//    Always returns 0 - command handled.
//
//  COMMENTS:
//
//

LRESULT CmdDisconnect(HWND hwnd, WORD wCommand, WORD wNotify, HWND hwndCtrl)
{
    HMENU hmenu;

    hwndCtrl;
    wNotify;
    wCommand;


    // Protocol Specific cleanup
    CleanUp();

    // Fix menus
    if(NULL == (hmenu = GetMenu(hwnd)))
        return 1;
    if(FALSE == EnableMenuItem(hmenu, IDM_CONNECT, MF_ENABLED))
        return 1;
    if(FALSE == EnableMenuItem(hmenu, IDM_LISTEN, MF_ENABLED))
        return 1;
    if(FALSE == EnableMenuItem(hmenu, IDM_DISCONNECT, MF_GRAYED))
        return 1;
    if(FALSE == SetWindowText(hwnd, szTitle))
        return 1;

    return 0;
}

//
//  FUNCTION: CmdExit(HWND, WORD, WORD, HWND)
//
//  PURPOSE: Exit the application.
//
//  PARAMETERS:
//    hwnd     - The window.
//    wCommand - IDM_EXIT            (unused)
//    wNotify  - Notification number (unused)
//    hwndCtrl - NULL                (unused)
//
//  RETURN VALUE:
//    Always returns 0 - command handled.
//
//  COMMENTS:
//
//

LRESULT CmdExit(HWND hwnd, WORD wCommand, WORD wNotify, HWND hwndCtrl)
{
    
    hwndCtrl;
    wNotify;
    wCommand;
    
    if(FALSE == DestroyWindow(hwnd))
        return 1;

    return 0;
}
