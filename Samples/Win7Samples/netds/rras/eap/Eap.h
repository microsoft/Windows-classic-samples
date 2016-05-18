/*

Copyright (c) 1997, Microsoft Corporation, all rights reserved

Description:
    Sample Extensible Authentication Protocol header file.

History:

*/

#ifndef _EAP_H_
#define _EAP_H_

#define PPP_EAP_PROTOCOL_ID 20  // This protocols Type Id

// Defines states within the this EAP protocol.

typedef enum _MYSTATE 
{
    MYSTATE_Initial,
    MYSTATE_WaitForUserOK,
    MYSTATE_WaitForRequest,
    MYSTATE_ReqSent,
    MYSTATE_WaitForAuthenticationToComplete,
    MYSTATE_Done

} MYSTATE;

typedef struct _EAPCB 
{
    MYSTATE             EapState;
    DWORD               fFlags;
    BOOL                fAuthenticator;
    LPVOID              pWorkBuffer;
    CHAR                aszIdentity[ UNLEN + 1 ];
    DWORD               dwIdExpected;
    CHAR                aszPassword[ PWLEN + 1 ];
    DWORD               dwResult;
    DWORD               dwInitialPacketId;
    BYTE*               pDataFromInteractiveUI;
    BYTE                bRecvPacketId;          //Special Id for Wireless case 
                                                //because it does not retransmit packets
    DWORD               dwSizeOfDataFromInteractiveUI;
    PBYTE               pUIContext;
    RAS_AUTH_ATTRIBUTE* pUserAttributes;
    RAS_AUTH_ATTRIBUTE* pMPPEKeyAttributes;    // MPPE key

} EAPCB;

typedef struct _EAP_NAME_DIALOG
{
    WCHAR               awszIdentity[ UNLEN + 1 ];
    WCHAR               awszPassword[ PWLEN + 1 ];

} EAP_NAME_DIALOG;

// Globals

#ifdef RASEAPGLOBALS

DWORD        g_dwEapTraceId  = INVALID_TRACEID;
HINSTANCE    g_hInstance     = NULL;

#else

extern DWORD        g_dwEapTraceId;
extern HINSTANCE    g_hInstance;

#endif // RASEAPGLOBALS

// Function Prototypes

VOID   
EapTrace(
    IN  CHAR*   Format, 
    ... 
);

DWORD APIENTRY
EapBegin(
    OUT VOID** ppWorkBuf,
    IN  VOID*  pInfo 
);

DWORD APIENTRY
EapEnd(
    IN VOID* pWorkBuf 
);

DWORD APIENTRY
EapMakeMessage(
    IN  VOID*               pWorkBuf,
    IN  PPP_EAP_PACKET*     pReceiveBuf,
    OUT PPP_EAP_PACKET*     pSendBuf,
    IN  DWORD               cbSendBuf,
    OUT PPP_EAP_OUTPUT*     pResult,
    IN  PPP_EAP_INPUT*      pInput 
);

DWORD
AuthenticateeMakeMessage(
    IN  EAPCB*              pwb,
    IN  PPP_EAP_PACKET*     pReceiveBuf,
    OUT PPP_EAP_PACKET*     pSendBuf,
    IN  DWORD               cbSendBuf,
    IN PPP_EAP_INPUT*       pInput,
    OUT  PPP_EAP_OUTPUT*    pResult
);

DWORD
AuthenticatorMakeMessage(
    IN  EAPCB*              pwb,
    IN  PPP_EAP_PACKET*     pReceiveBuf,
    OUT PPP_EAP_PACKET*     pSendBuf,
    IN  DWORD               cbSendBuf,
    IN PPP_EAP_INPUT*       pInput,
    OUT  PPP_EAP_OUTPUT*    pResult
);

VOID
MakeResponseMessage(
    IN  EAPCB*              pwb,
    IN  PPP_EAP_PACKET*     pReceiveBuf,
    OUT PPP_EAP_PACKET *    pSendBuf,
    IN DWORD                cbSendBuf
);

VOID
MakeResponseMessage1(
    IN  EAPCB*              pwb,
    OUT PPP_EAP_PACKET *    pSendBuf,
    IN DWORD                cbSendBuf
);

VOID
MakeResultMessage(
    IN  EAPCB *             pwb,
    IN  DWORD               dwError,
    OUT PPP_EAP_PACKET*     pSendBuf,
    IN  DWORD               cbSendBuf
);

DWORD
GetPasswordFromResponse(
    IN  PPP_EAP_PACKET*     pReceiveBuf,
    OUT CHAR*               pszUserName
);

DWORD
MakeAuthenticationAttributes(
    IN CHAR *   szUserName,
    IN CHAR *   szPassword,
    IN EAPCB *  pwb
);

DWORD
MakeMPPEKeyAttributes(
    IN EAPCB *  pwb
);

DWORD
GetIdentity(
    IN  HWND    hwndParent,
    IN  BYTE*   pUserDataIn,
    IN  DWORD   dwSizeOfUserDataIn,
    OUT BYTE**  ppUserDataOut,
    OUT DWORD*  pdwSizeOfUserDataOut,
    OUT WCHAR** ppwszIdentityOut
);

VOID
GetUsernameAndPassword(
    IN  HWND                hwndParent,
    IN  EAP_NAME_DIALOG*    pEapNameDialog
);

INT_PTR CALLBACK
UsernameDialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
);

BOOL
InitUsernameDialog(
    IN  HWND    hWnd,
    IN  LPARAM  lParam
);

BOOL
UsernameCommand(
    IN  EAP_NAME_DIALOG*    pEapNameDialog,
    IN  WORD                wId,
    IN  HWND                hWndDlg
);

#endif // _EAP_H_
