// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright 1993 - 2000 Microsoft Corporation.  All Rights Reserved.
//
//  MODULE:   listen.c
//
//  PURPOSE:   Displays the "Listen" dialog box
//
//  FUNCTIONS:
//    CmdListen          - Displays the "Listen" dialog box.
//    Listen             - Processes messages for "Listen" dialog box.
//    MsgListenInit      - Centers dialog and initializes edit controls.
//    MsgListenConnected - Handles Connected message when socket is connected.
//    MsgListenCommand   - Process WM_COMMAND message sent to the listen box.
//    CmdListenDone      - Free the listen box and related data.
//    CmdListenNow       - Sets up listen on specified socket
//
//  COMMENTS:
//
//
#include "globals.h"            // prototypes specific to this application
#include <windows.h>            // required for all Windows applications
#include <windowsx.h>
#include <stdlib.h>
#include <stdio.h>


LRESULT CALLBACK Listen(HWND, UINT, WPARAM, LPARAM);
LRESULT MsgListenInit(HWND, UINT, WPARAM, LPARAM);
LRESULT MsgListenCommand(HWND, UINT, WPARAM, LPARAM);
LRESULT MsgListenConnected(HWND, UINT, WPARAM, LPARAM);
LRESULT CmdListenNow(HWND, WORD, WORD, HWND);
LRESULT CmdListenDone(HWND, WORD, WORD, HWND);

// Listen dialog message table definition.
MSD rgmsdListen[] =
{
    {WM_COMMAND,    MsgListenCommand},
    {WM_INITDIALOG, MsgListenInit},
    {LDM_CONNECTED, MsgListenConnected}
};

MSDI msdiListen =
{
    sizeof(rgmsdListen) / sizeof(MSD),
    rgmsdListen,
    edwpNone
};

// Listen dialog command table definition.
CMD rgcmdListen[] =
{
    {IDOK,     CmdListenNow},
    {IDCANCEL, CmdListenDone}
};

CMDI cmdiListen =
{
    sizeof(rgcmdListen) / sizeof(CMD),
    rgcmdListen,
    edwpNone
};

// Module specific "globals"  Used when a variable needs to be
// accessed in more than one handler function.

HFONT hfontDlg;

//
//  FUNCTION: CmdListen(HWND, WORD, WORD, HWND)
//
//  PURPOSE: Displays the "Listen" dialog box
//
//  PARAMETERS:
//    hwnd      - Window handle
//    wCommand  - IDM_LISTEN          (unused)
//    wNotify   - Notification number (unused)
//    hwndCtrl  - NULL                (unused)
//
//  RETURN VALUE:
//
//    Always returns 0 - Message handled
//
//  COMMENTS:
//    To process the IDM_LISTEN message, call DialogBox() to display the
//    Listen dialog box.

LRESULT CmdListen(HWND hwnd, WORD wCommand, WORD wNotify, HWND hwndCtrl)
{
    HMENU hmenu;

    hwndCtrl;
    wNotify;
    wCommand;

    SetWindowText(hwnd, "IPX Chat Server");     // Change title bar text
    
    // Start Dialog
    if(DialogBox(hInst, "ListenBox", hwnd, (DLGPROC)Listen))
    {

        // Dialog got a connection!  Set Message to indicate
        // when we have data to read, or if the connection is
        // closed on us.
        if (WSAAsyncSelect(sock,
                           hwnd,
                           MW_DATAREADY,
                           FD_READ | FD_CLOSE) == SOCKET_ERROR) 
        {
            MessageBox(hwnd, "WSAAsyncSelect Failed!", NULL, MB_OK);
            CleanUp();
            return 0;
        }

        // Fix menus
        if(NULL == (hmenu = GetMenu(hwnd)))
            return 0;
        EnableMenuItem(hmenu, IDM_CONNECT, MF_GRAYED);
        EnableMenuItem(hmenu, IDM_LISTEN, MF_GRAYED);
        EnableMenuItem(hmenu, IDM_DISCONNECT, MF_ENABLED);
        return 0;
    }

    // Listen Failed
    SetWindowText(hwnd, szTitle);
    return 0;
}


//
//  FUNCTION: Listen(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for "Listen" dialog box.
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
//     Gets port information from user and then listens
//
//     Listen when user clicks on the OK button.  Kill Dialog when connection
//     established.
//

LRESULT CALLBACK Listen(HWND hdlg, UINT uMessage, WPARAM wparam, LPARAM lparam)
{
    return DispMessage(&msdiListen, hdlg, uMessage, wparam, lparam);
}


//
//  FUNCTION: MsgListenInit(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: To center dialog and limit size of edit controls and initialize
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
//           Socket   4  chars (2 2-digit hex numbers)
//

LRESULT MsgListenInit(HWND hdlg, UINT uMessage, WPARAM wparam, LPARAM lparam)
{

    lparam;
    wparam;
    uMessage;
    
    // Create a font to use
    if(NULL == (hfontDlg = CreateFont(14, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 
                          VARIABLE_PITCH | FF_SWISS, "")))
        return FALSE;

    // Center the dialog over the application window
    CenterWindow (hdlg, GetWindow (hdlg, GW_OWNER));
     
    // Initialize Socket Addresses
    SetDlgItemText(hdlg, LD_SOCKET, szListenSocket);

    // Limit input to proper size strings
    SendDlgItemMessage(hdlg, LD_SOCKET, EM_LIMITTEXT, 4, 0);

    return (TRUE);
}

//
//  FUNCTION: MsgListenConnected(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: To handle connected message when socket is connected
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
//    Performs accept() on incoming socket
//

LRESULT MsgListenConnected(HWND hdlg, UINT uMessage, WPARAM wparam, LPARAM lparam)
{
    char outtext[128];
    HRESULT hRet;

    lparam;
    wparam;
    uMessage;

    SetDlgItemText(hdlg,
                   LD_STATUS,
                   "Client Connected!");

    SetDlgItemText(hdlg,
                   LD_STATUS,
                   "Calling accept()");

    pRemAddr = (PSOCKADDR_IPX)&addr;
    addrlen = sizeof(addr);

    // Accept connection
    if ((sock = accept(SrvSock, (struct sockaddr *)pRemAddr, &addrlen)) == INVALID_SOCKET)
    {
        // accept() failed -- show status and clean up dialog
        //sprintf(outtext, "Accept() failed, error %u",WSAGetLastError());
        hRet = StringCchPrintf(outtext,128,"Accept() failed, error %u",WSAGetLastError());
        SetDlgItemText(hdlg,
                       LD_STATUS,
                       outtext);
        closesocket(SrvSock);
        WSACleanup();
        EnableWindow(GetDlgItem(hdlg, IDOK), TRUE);
        EnableWindow(GetDlgItem(hdlg, LD_SOCKET), TRUE);
        SetFocus(GetDlgItem(hdlg, LD_SOCKET));
        return(TRUE);
    }

    // We're connected!
    GetAddrString(pRemAddr, outtext);
    hRet = StringCchCat(outtext,128," connected!");

    SetDlgItemText(hdlg,
                   LD_STATUS,
                   outtext);
          
    EndDialog(hdlg, TRUE);          // Exit the dialog
    DeleteObject (hfontDlg);        // Drop font
    return (TRUE);
}

//
//  FUNCTION: MsgListenCommand(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Process WM_COMMAND message sent to the Listen box.
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
//    with the cmdiListen structure defined in this file to handle
//    the command messages for the Listen dialog box.
//

LRESULT MsgListenCommand(HWND   hwnd, 
                        UINT   uMessage, 
                        WPARAM wparam, 
                        LPARAM lparam)
{
    uMessage;

    return DispCommand(&cmdiListen, hwnd, wparam, lparam);
}

//
//  FUNCTION: CmdListenDone(HWND, WORD, HWND)
//
//  PURPOSE: Free the Listen box and related data.
//
//  PARAMETERS:
//    hdlg - The window handling the command.
//    wCommand - The command to be handled (unused).
//    wNotify   - Notification number      (unused)
//    hwndCtrl - NULL                      (unused).
//
//  RETURN VALUE:
//    Always returns TRUE.
//
//  COMMENTS:
//    Cleans up sockets then calls EndDialog to finish the dialog session.
//

LRESULT CmdListenDone(HWND hdlg, WORD wCommand, WORD wNotify, HWND hwndCtrl)
{

    hwndCtrl;
    wNotify;
    wCommand;
    
    
    if (INVALID_SOCKET != SrvSock)
    {
        closesocket(SrvSock);    // Free any aborted socket resources
        SrvSock = INVALID_SOCKET;
    }
    WSACleanup();
    DeleteObject (hfontDlg); // Drop the font
    EndDialog(hdlg, FALSE);  // Exit Dialog -- rtrn false since no connection
    return TRUE;
}

//
//  FUNCTION: CmdListenNow(HWND, WORD, WORD, HWND)
//
//  PURPOSE: Handles ID_OK message and listens on the specified socket
//
//  PARAMETERS:
//    hwnd - The window handling the command.
//    wCommand - The command to be handled (unused).
//    wNotify   - Notification number (unused)
//    hwndCtrl - NULL (unused).
//
//  RETURN VALUE:
//    Always returns TRUE.
//
//  COMMENTS:
//    Shows Listening address on status bar when listen is successful.
//

LRESULT CmdListenNow(HWND hdlg, WORD wCommand, WORD wNotify, HWND hwndCtrl)
{
    char szXferBuffer[5];
    WORD wVersionRequested;
    WSADATA wsaData;
    char outtext[80];
    HRESULT hRet;

    hwndCtrl;
    wNotify;
    wCommand;

    // Get Socket Address
    GetDlgItemText(hdlg,
         LD_SOCKET,
         szXferBuffer,
         sizeof(szXferBuffer));
    wVersionRequested = MAKEWORD(1, 1);

    SetDlgItemText(hdlg,
                  LD_STATUS,
                  "Calling WSAStartup");

    // Initializes winsock dll
    if(WSAStartup(wVersionRequested, &wsaData) != 0)
    {

        SetDlgItemText(hdlg,
                       LD_STATUS,
                       "WSAStartup failed");
        return TRUE;
    }

    SetDlgItemText(hdlg,
          LD_STATUS,
          "WSAStartup Succeeded");

    SetDlgItemText(hdlg,
          LD_STATUS,
          "Calling socket()");

    // Allocate socket handle
    SrvSock = socket(AF_IPX,         // IPX Family
                     SOCK_SEQPACKET, // Gives message mode transfers
                     NSPROTO_SPX);   // SPX is connection oriented transport

    if(SrvSock == INVALID_SOCKET) {
        SetDlgItemText(hdlg,
                       LD_STATUS,
                       "ERROR on socket()");
        WSACleanup();
        return TRUE;
    }

    SetDlgItemText(hdlg,
                   LD_STATUS,
                   "socket() Succeeded");

    // Set up socket address to bind to
    memset(&addr, 0, sizeof(addr));    // Clear address
    pSockAddr = (PSOCKADDR_IPX)&addr;  // Set pointer
    pSockAddr->sa_family = AF_IPX;     // IPX Family
    // Make sure socket number is in network order
    AtoH(szXferBuffer, (char *)&pSockAddr->sa_socket, 2);

    SetDlgItemText(hdlg,
                   LD_STATUS,
                   "Calling bind()");

    // Bind to socket address
    if(bind(SrvSock, 
            (PSOCKADDR) pSockAddr, 
            sizeof(SOCKADDR_IPX)) == SOCKET_ERROR)
    {
        SetDlgItemText(hdlg,
                       LD_STATUS,
                       "Error on bind()");
        if (INVALID_SOCKET != SrvSock)
        {
            closesocket(SrvSock);
            SrvSock = INVALID_SOCKET;
        }
        WSACleanup();
        return TRUE;
    }

    SetDlgItemText(hdlg,
                   LD_STATUS,
                   "bind() Succeeded");

    SetDlgItemText(hdlg,
                   LD_STATUS,
                   "Calling listen()");

    // Set up listen queue - this app only supports one connection so
    // queue length is set to 1.
    if (listen(SrvSock, 1) == SOCKET_ERROR)
    {
        hRet = StringCchPrintf(outtext,80,"FAILURE: listen() returned %u", WSAGetLastError());
        SetDlgItemText(hdlg,                       
                       LD_STATUS,
                       outtext);
        if (INVALID_SOCKET != SrvSock)
        {
            closesocket(SrvSock);
            SrvSock = INVALID_SOCKET;
        }
        WSACleanup();
        return TRUE;
    }

    SetDlgItemText(hdlg,
                   LD_STATUS,
                   "listen() succeeded");

    SetDlgItemText(hdlg,
                   LD_STATUS,
                   "Calling WSAAsyncSelect()");

    // Specify message to be posted when client connects
    if(WSAAsyncSelect(SrvSock, 
                      hdlg, 
                      LDM_CONNECTED, 
                      FD_ACCEPT) == SOCKET_ERROR)
    {
        SetDlgItemText(hdlg,
                       LD_STATUS,
                       "WSAAsyncSelect() failed");
        if (INVALID_SOCKET != SrvSock)
        {
            closesocket(SrvSock);
            SrvSock = INVALID_SOCKET;
        }
        WSACleanup();
        return TRUE;
    }

    SetDlgItemText(hdlg,
                   LD_STATUS,
                   "WSAAsyncSelect() succeeded");

    addrlen = sizeof(addr);

    // Get full network.number.socket address
    if(getsockname(SrvSock,&addr,&addrlen) == SOCKET_ERROR)
    {
        hRet = StringCchCopy(outtext,80,"ERROR getsocketname()");
    }
    else
    {
        hRet = StringCchCopy(outtext,80,"Listening on ");
        GetAddrString((PSOCKADDR_IPX)&addr, outtext + lstrlen(outtext));
    }

    SetDlgItemText(hdlg,
                   LD_STATUS,
                   outtext);

    
    SetFocus(GetDlgItem(hdlg, IDCANCEL));             // Give Cancel Button focus
    EnableWindow(GetDlgItem(hdlg, IDOK), FALSE);      // Grey OK button
    EnableWindow(GetDlgItem(hdlg, LD_SOCKET), FALSE); // Grey Socket Edit control
    return (TRUE);
}
