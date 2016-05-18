// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright 1993 - 2000 Microsoft Corporation.  All Rights Reserved.
//
//  MODULE:   connect.c
//
//  PURPOSE:   Displays the "Connect" dialog box
//
//  FUNCTIONS:
//    CmdConnect        - Displays the "Connect" dialog box
//    Connect           - Processes messages for "Connect" dialog box.
//    MsgConnectInit    - Initializes edit controls
//    MsgConnectCommand - Process WM_COMMAND message sent to the connect box.
//    CmdConnectDone    - Free the connect box and related data.
//    CmdConnectNow     - Establishes connection to specified address. Kills
//                        dialog if successful.
//
//  COMMENTS:
//
//
#include "globals.h"            // prototypes specific to this application
#include <windows.h>            // required for all Windows applications
#include <windowsx.h>
#include <stdlib.h>
#include <stdio.h>


//   Function Definitions


LRESULT CALLBACK Connect(HWND, UINT, WPARAM, LPARAM);
LRESULT MsgConnectInit(HWND, UINT, WPARAM, LPARAM);
LRESULT MsgConnectCommand(HWND, UINT, WPARAM, LPARAM);
LRESULT CmdConnectNow(HWND, WORD, WORD, HWND);
LRESULT CmdConnectDone(HWND, WORD, WORD, HWND);

// Connect dialog message table definition.
MSD rgmsdConnect[] =
{
    {WM_COMMAND,    MsgConnectCommand},
    {WM_INITDIALOG, MsgConnectInit}
};

MSDI msdiConnect =
{
    sizeof(rgmsdConnect) / sizeof(MSD),
    rgmsdConnect,
    edwpNone
};

// Connect dialog command table definition.
CMD rgcmdConnect[] =
{
    {IDOK,     CmdConnectNow},
    {IDCANCEL, CmdConnectDone}
};

CMDI cmdiConnect =
{
    sizeof(rgcmdConnect) / sizeof(CMD),
    rgcmdConnect,
    edwpNone
};

// Module specific "globals"  Used when a variable needs to be
// accessed in more than one handler function.

HFONT hfontDlg;

//
//  FUNCTION: CmdConnect(HWND, WORD, WORD, HWND)
//
//  PURPOSE: Displays the "Connect" dialog box
//
//  PARAMETERS:
//    hwnd      - Window handle
//    wCommand  - IDM_CONNECT (unused)
//    wNotify   - Notification number (unused)
//    hwndCtrl  - NULL (unused)
//
//  RETURN VALUE:
//
//    Always returns 0 - Message handled
//
//  COMMENTS:
//    To process the IDM_CONNECT message, call DialogBox() to display the
//    Connect dialog box.

LRESULT CmdConnect(HWND hwnd, WORD wCommand, WORD wNotify, HWND hwndCtrl)
{
    HMENU hmenu = NULL;

    hwndCtrl;
    wNotify;
    wCommand;


    if(FALSE == SetWindowText(hwnd, "IPX Chat Client"))    // Change Window Title
        return 1;

    // Start dialog box
    if(DialogBox(hInst, "ConnectBox", hwnd, (DLGPROC)Connect))
    {
        
        // We have a connection!  Set up message notification if
        // data ready to be read or if connection is closed on us
        if (WSAAsyncSelect(sock,
                           hwnd,
                           MW_DATAREADY,
                           FD_READ | FD_CLOSE) == SOCKET_ERROR) 
        {
            MessageBox(hwnd, "WSAAsyncSelect Failed!", NULL, MB_OK);
            CleanUp();
            return 0;
        }

        // Connect has succeeded!  Fix menus
        if(NULL == (hmenu = GetMenu(hwnd)))
            return 1;
        if(FALSE == EnableMenuItem(hmenu, IDM_CONNECT, MF_GRAYED))
            return 1;
        if(FALSE == EnableMenuItem(hmenu, IDM_LISTEN, MF_GRAYED))
            return 1;
        if(FALSE == EnableMenuItem(hmenu, IDM_DISCONNECT, MF_ENABLED))
            return 1;

        return 0;
    }

    // Connection failed - reset window title
    if(FALSE == SetWindowText(hwnd, szTitle))
        return 1;

    return 0;
}


//
//  FUNCTION: Connect(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for "Connect" dialog box.
//
//  PARAMETERS:
//    hdlg - window handle of the dialog box
//    wMessage - type of message
//    wparam - message-specific information
//    lparam - message-specific information
//
//  RETURN VALUE:
//    TRUE - message handled
//    FALSE - message not handled
//
//  COMMENTS:
//
//     Gets connection information from user and then establishes a connection.
//
//     Connect when user clicks on the OK button.  Kill Dialog when connection
//     established.
//

LRESULT CALLBACK Connect(HWND hdlg, UINT uMessage, WPARAM wparam, LPARAM lparam)
{
    return DispMessage(&msdiConnect, hdlg, uMessage, wparam, lparam);
}


//
//  FUNCTION: MsgConnectInit(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: To center dialog and limit size of edit controls
//
//  PARAMETERS:
//    hdlg - The window handing the message.
//    uMessage - The message number. (unused).
//    wparam - Message specific data (unused).
//    lparam - Message specific data (unused).
//
//  RETURN VALUE:
//    Always returns 0 - message handled.
//
//  COMMENTS:
//    Set size of edit controls for the following
//           Network  8  chars (4 2-digit hex numbers)
//           Node     12 chars (6 2-digit hex numbers)
//           Socket   4  chars (2 2-digit hex numbers)
//

LRESULT MsgConnectInit(HWND hdlg, UINT uMessage, WPARAM wparam, LPARAM lparam)
{

    lparam;
    wparam;
    uMessage;

    // Create a font to use
    if(NULL == (hfontDlg = CreateFont(14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          VARIABLE_PITCH | FF_SWISS, "")))
        return FALSE;

    // Center the dialog over the application window
    CenterWindow (hdlg, GetWindow (hdlg, GW_OWNER));

    // Limit size of edit controls
    SendDlgItemMessage(hdlg, CD_NETWORK, EM_LIMITTEXT, 8, 0);
    SendDlgItemMessage(hdlg, CD_NODE, EM_LIMITTEXT, 12, 0);
    SendDlgItemMessage(hdlg, CD_SOCKET, EM_LIMITTEXT, 4, 0);

    // Initialize Edit Controls
    if(FALSE == SetDlgItemText(hdlg, CD_NETWORK, szConnectNetwork))
        return FALSE;
    if(FALSE == SetDlgItemText(hdlg, CD_NODE, szConnectNode))
        return FALSE;
    if(FALSE == SetDlgItemText(hdlg, CD_SOCKET, szConnectSocket))
        return FALSE;
     
    return TRUE;
}

//
//  FUNCTION: MsgConnectCommand(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Process WM_COMMAND message sent to the Connect box.
//
//  PARAMETERS:
//    hwnd - The window handing the message.
//    uMessage - The message number. (unused).
//    wparam - Message specific data (unused).
//    lparam - Message specific data (unused).
//
//  RETURN VALUE:
//    Always returns 0 - message handled.
//
//  COMMENTS:
//    Uses this DispCommand function defined in wndproc.c combined
//    with the cmdiConnect structure defined in this file to handle
//    the command messages for the Connect dialog box.
//

LRESULT MsgConnectCommand(HWND   hwnd, 
                        UINT   uMessage, 
                        WPARAM wparam, 
                        LPARAM lparam)
{
    uMessage;
    
    return DispCommand(&cmdiConnect, hwnd, wparam, lparam);
}

//
//  FUNCTION: CmdConnectDone(HWND, WORD, HWND)
//
//  PURPOSE: Free the Connect box and related data.
//
//  PARAMETERS:
//    hdlg - The window handling the command.
//    wCommand - The command to be handled (unused).
//    wNotify   - Notification number (unused)
//    hwndCtrl - NULL (unused).
//
//  RETURN VALUE:
//    Always returns TRUE.
//
//  COMMENTS:
//    Cleans up sockets then calls EndDialog to finish the dialog session.
//

LRESULT CmdConnectDone(HWND hdlg, WORD wCommand, WORD wNotify, HWND hwndCtrl)
{

    hwndCtrl;
    wNotify;
    wCommand;

    CleanUp();               // Free any aborted socket resources
    EndDialog(hdlg, FALSE);  // Exit Dialog -- rtrn false since no connection
    return TRUE;
}

//
//  FUNCTION: CmdConnectNow(HWND, WORD, HWND)
//
//  PURPOSE: Establish the connection
//
//  PARAMETERS:
//    hdlg - The window handling the command.
//    wCommand - The command to be handled (unused).
//    wNotify   - Notification number (unused)
//    hwndCtrl - NULL (unused).
//
//  RETURN VALUE:
//    Always returns TRUE.
//
//  COMMENTS:
//    Makes Connection calls
//

LRESULT CmdConnectNow(HWND hdlg, WORD wCommand, WORD wNotify, HWND hwndCtrl)
{
    char outtext[128];
    WORD wVersionRequested;
    WSADATA wsaData;
    HRESULT hRet;

    hwndCtrl;
    wNotify;
    wCommand;

    GetDlgItemText(hdlg, CD_NETWORK, szConnectNetwork, 9);        // Get Network Number
    AtoH(szConnectNetwork, (char *)&RemAddr.sa_netnum, 4);        // Network order

    GetDlgItemText(hdlg, CD_NODE, szConnectNode, 13);             // Get Node Number
    AtoH((char *)szConnectNode, (char *)&RemAddr.sa_nodenum, 6);  // Network order

    GetDlgItemText(hdlg, CD_SOCKET, szConnectSocket, 5);          // Get Socket Number
    AtoH((char *)szConnectSocket, (char *)&RemAddr.sa_socket, 2); // Network order                  

    RemAddr.sa_family = AF_IPX;                                   // IPX Family
    wVersionRequested = MAKEWORD(1, 1);

    SetDlgItemText(hdlg, CD_STATUS, "Calling WSAStartup");

    // Initialize winsock dll
    if(WSAStartup(wVersionRequested, &wsaData) != 0)
    {
        SetDlgItemText(hdlg, CD_STATUS, "WSAStartup failed");
        return TRUE;
    }

    SetDlgItemText(hdlg, CD_STATUS, "WSAStartup Succeeded");

    SetDlgItemText(hdlg, CD_STATUS, "Calling socket()");

    // get socket handle
    sock = socket(AF_IPX,            // IPX family
                  SOCK_SEQPACKET,    // Message mode data xfers
                  NSPROTO_SPX);      // SPX = Connection Oriented

    if(sock == INVALID_SOCKET) {
        SetDlgItemText(hdlg, CD_STATUS, "ERROR on socket()");
        WSACleanup();
        return TRUE;
    }

    SetDlgItemText(hdlg, CD_STATUS, "socket() Succeeded");

    SetDlgItemText(hdlg, CD_STATUS, "Calling connect()");

    // Call specified address (we block here since I don't have anything better
    // to do).
    if(connect(sock, (struct sockaddr *)&RemAddr, sizeof(RemAddr)) == SOCKET_ERROR)
    {
        // Failed!  Post status and cleanup
        hRet = StringCchPrintf(outtext,128,"connect() failed, error %u", WSAGetLastError());
        SetDlgItemText(hdlg, CD_STATUS, outtext);
        closesocket(sock);
        WSACleanup();
        return TRUE;
    }

    // We've got a connection!  Kill the dialog
    SetDlgItemText(hdlg, CD_STATUS, "connect() succeeded!");

    EndDialog(hdlg, TRUE);          // Exit the dialog
    DeleteObject (hfontDlg);
    return (TRUE);
}
