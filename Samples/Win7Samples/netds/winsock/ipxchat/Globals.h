#ifndef _GLOBAL_H_
#define _GLOBAL_H_

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright 1993 - 2000 Microsoft Corporation.  All Rights Reserved.
//
// PURPOSE:
//    Contains declarations for all globally scoped names in the program.
//

//-------------------------------------------------------------------------
// Product identifier string defines

// #define APPNAME       IPXChat
// #define SZVERSION     "Version 4.0"

#include <winsock2.h>
#include <wsipx.h>
#define STRSAFE_LIB
#include <strsafe.h>

//-------------------------------------------------------------------------
// Functions for handling main window messages.  The message-dispatching
// mechanism expects all message-handling functions to have the following
// prototype:
//
//     LRESULT FunctionName(HWND, UINT, WPARAM, LPARAM);

LRESULT MsgCreate(HWND, UINT, WPARAM, LPARAM);
LRESULT MsgSize(HWND, UINT, WPARAM, LPARAM);
LRESULT MsgSetfocus(HWND, UINT, WPARAM, LPARAM);
LRESULT MsgCommand(HWND, UINT, WPARAM, LPARAM);
LRESULT MsgDataready(HWND, UINT, WPARAM, LPARAM);
LRESULT MsgRefreshdisplay(HWND, UINT, WPARAM, LPARAM);
LRESULT MsgDisconnected(HWND, UINT, WPARAM, LPARAM);
LRESULT MsgDestroy(HWND, UINT, WPARAM, LPARAM);


//-------------------------------------------------------------------------
// Functions for handling main window commands--ie. functions for
// processing WM_COMMAND messages based on the wParam value.
// The message-dispatching mechanism expects all command-handling
// functions to have the following prototype:
//
//     LRESULT FunctionName(HWND, WORD, WORD, HWND);

LRESULT CmdConnect(HWND, WORD, WORD, HWND);
LRESULT CmdListen(HWND, WORD, WORD, HWND);
LRESULT CmdExit(HWND, WORD, WORD, HWND);
LRESULT CmdAbout(HWND, WORD, WORD, HWND);
LRESULT CmdOutbox(HWND, WORD, WORD, HWND);
LRESULT CmdDisconnect(HWND, WORD, WORD, HWND);


//-------------------------------------------------------------------------
// Global function prototypes.

BOOL InitApplication(HINSTANCE, int);
BOOL CenterWindow(HWND, HWND);
BOOL ReceiveInBox(HWND, WPARAM, LPARAM, char *, int);
void SendOutBox(char *, int);
void AtoH(char *, char *, int);
unsigned char BtoH(char);
void CleanUp(void);
void GetAddrString(PSOCKADDR_IPX, char *);
void HtoA(char *, char *, int);
char HtoB(UCHAR);
LPTSTR GetStringRes (int id);

// Callback functions.  These are called by Windows.

// **TODO**  Add new callback function prototypes here.

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


//-------------------------------------------------------------------------
// Command ID definitions.  These definitions are used to associate menu
// items with commands.

// Options menu
#define IDM_CONNECT    1000
#define IDM_LISTEN     1001
#define IDM_DISCONNECT 1002
#define IDM_EXIT       1003

// Help menu
#define IDM_ABOUT      1100

//-------------------------------------------------------------------------
// User Defined Messages.  These definitions are used for indicating 
// network events.

#define MW_DATAREADY       501
#define MW_DISPLAYREFRESH  502
#define MW_DISCONNECTED    503
#define LDM_CONNECTED      504

//-------------------------------------------------------------------------
// String Table ID definitions.

#define IDS_APPNAME     1
#define IDS_DESCRIPTION 2

//-------------------------------------------------------------------------
//  Main Window Edit Control defines.

#define ID_OUTBOX          601
#define ID_INBOX           602

//-------------------------------------------------------------------------
//  About dialog defines.

#define IDD_VERFIRST    100
#define IDD_VERLAST     104

//-------------------------------------------------------------------------
//  Connect dialog defines.

#define CD_STATUS       200
#define CD_NETWORK      201
#define CD_NODE         202
#define CD_SOCKET       203

//-------------------------------------------------------------------------
//  Listen dialog defines.

#define LD_SOCKET       300
#define LD_STATUS       301

//-------------------------------------------------------------------------
// Global variable declarations.

extern HINSTANCE hInst;          // The current instance handle
extern char      szAppName[];    // The name of this application
extern char      szTitle[];      // The title bar text
SOCKET sock, SrvSock;
SOCKADDR_IPX RemAddr;
PSOCKADDR_IPX pSockAddr, pRemAddr;
struct sockaddr addr;
int addrlen;
HWND hOutWnd, hInWnd;
char szRcvBuf[0x8000];
char szSndBuf[0x8000];
extern char szConnectNetwork[];
extern char szConnectNode[];
extern char szConnectSocket[];
extern char szListenSocket[];
extern BOOL i_should_sleep;

// For NON-MDI applications, uncomment line 1 below and comment
// line 2.  For MDI applications, uncomment line 2 below, comment
// line 1, and then define hwndMDIIPXChat as a global variable in
// INIT.C
#define hwndMDIIPXChat NULL        /* (1) Stub for NON-MDI applications. */
// extern HWND hwndMDIIPXChat;     /* (2) For MDI applications.          */


//-------------------------------------------------------------------------
// Message and command dispatch infrastructure.  The following type
// definitions and functions are used by the message and command dispatching
// mechanism and do not need to be changed.

    // Function pointer prototype for message handling functions.
typedef LRESULT (*PFNMSG)(HWND,UINT,WPARAM,LPARAM);

    // Function pointer prototype for command handling functions.
typedef LRESULT (*PFNCMD)(HWND,WORD,WORD,HWND);

    // Enumerated type used to determine which default window procedure
    // should be called by the message- and command-dispatching mechanism
    // if a message or command is not handled explicitly.
typedef enum
{
   edwpNone,            // Do not call any default procedure.
   edwpWindow,          // Call DefWindowProc.
   edwpDialog,          // Call DefDlgProc (This should be used only for
                        // custom dialogs - standard dialog use edwpNone).
   edwpMDIChild,        // Call DefMDIChildProc.
   edwpMDIFrame         // Call DefFrameProc.
} EDWP;                // Enumeration for Default Window Procedures

    // This structure maps messages to message handling functions.
typedef struct _MSD
{
    UINT   uMessage;
    PFNMSG pfnmsg;
} MSD;                 // MeSsage Dispatch structure

    // This structure contains all of the information that a window
    // procedure passes to DispMessage in order to define the message
    // dispatching behavior for the window.
typedef struct _MSDI
{
    int  cmsd;          // Number of message dispatch structs in rgmsd
    MSD *rgmsd;         // Table of message dispatch structures
    EDWP edwp;          // Type of default window handler needed.
} MSDI, FAR *LPMSDI;   // MeSsage Dipatch Information

    // This structure maps command IDs to command handling functions.
typedef struct _CMD
{
    WORD   wCommand;
    PFNCMD pfncmd;
} CMD;                 // CoMmand Dispatch structure

    // This structure contains all of the information that a command
    // message procedure passes to DispCommand in order to define the
    // command dispatching behavior for the window.
typedef struct _CMDI
{
    int  ccmd;          // Number of command dispatch structs in rgcmd
    CMD *rgcmd;         // Table of command dispatch structures
    EDWP edwp;          // Type of default window handler needed.
} CMDI, FAR *LPCMDI;   // CoMmand Dispatch Information

    // Message and command dispatching functions.  They look up messages
    // and commands in the dispatch tables and call the appropriate handler
    // function.
LRESULT DispMessage(LPMSDI, HWND, UINT, WPARAM, LPARAM);
LRESULT DispCommand(LPCMDI, HWND, WPARAM, LPARAM);

    // Message dispatch information for the main window
extern MSDI msdiMain;
    // Command dispatch information for the main window
extern CMDI cmdiMain;



//-------------------------------------------------------------------------
// Version string definitions--Leave these alone.

#define SZRCOMPANYNAME "CompanyName"
#define SZRDESCRIPTION "FileDescription"
#define SZRVERSION     "FileVersion"
#define SZRAPPNAME     "InternalName"
#define SZRCOPYRIGHT   "LegalCopyright"
#define SZRTRADEMARK   "LegalTrademarks"
#define SZRPRODNAME    "ProductName"
#define SZRPRODVER     "ProuctVersion"

#endif
