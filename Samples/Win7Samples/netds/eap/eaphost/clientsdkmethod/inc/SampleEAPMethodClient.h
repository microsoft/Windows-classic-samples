// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.


#ifndef _SAMPLE_CLIENT_EAPMETHOD_H_
#define _SAMPLE_CLIENT_EAPMETHOD_H_

#pragma once

#include <windows.h>
#include "eaptypes.h"
#include "eapmethodtypes.h"
#include "eapmethodpeerapis.h"
#include "lmcons.h"     // <UNLEN, PWLEN>

/// UI dialog shown after receiving EAP-Challenge.
const WCHAR STRING_UI_ALLOW_AUTH[] = L"You are being authenticated by a Sample EAP provider.";

/// UI dialog shown after receiving EAP-Challenge.
const size_t STRING_LENGTH_UI_ALLOW_AUTH = sizeof(STRING_UI_ALLOW_AUTH);

/// UI context that we pass back for successful UI interactions.
const WCHAR STRING_UI_SUCCESS[] = L"OK";

/// Buffer length (in bytes) of UI context that we pass back for successful
/// UI interactions.
const size_t STRING_LENGTH_UI_SUCCESS = sizeof(STRING_UI_SUCCESS);

/// The title of the UI displayed for interactive authentication.
const WCHAR STRING_INTERACTIVE_UI_TITLE[] = L"EAP sample";

/// Buffer length (in bytes) of the title of the UI displayed for interactive
/// authentication.
const size_t STRING_LENGTH_INTERACTIVE_UI_TITLE = sizeof(STRING_INTERACTIVE_UI_TITLE);

/// Following are required for registering Eap Peer Method with EapHost.
const wchar_t peerFriendlyName[] = L"PeerFriendlyName";
const wchar_t peerDllPath[] = L"PeerDllPath";
const wchar_t properties[] = L"Properties";
const wchar_t peerInvokeUserNameDialog[] = L"PeerInvokeUsernameDialog";
const wchar_t peerInvokePasswordDialog[] = L"PeerInvokePasswordDialog";
const wchar_t peerFriendlyNameValue[] = L"SdkPeerEapMethod";
const DWORD propertiesValue = 0x280000; ///< eapPropMppeEncryption (0x80000) & eapPropSupportsConfig (0x200000)
const wchar_t peerMethodDllName[] = L"ClientSdkMethodDll.dll";

/// A named constant for the credential input field struct size.
static const DWORD CRED_FIELD_SIZE = sizeof(EAP_CONFIG_INPUT_FIELD_DATA);

/// The credential input fields needed for this method to authenticate users.
static EAP_CONFIG_INPUT_FIELD_DATA defaultCredentialInputFields[] =
{
    {
        CRED_FIELD_SIZE,                       ///< Size of this structure.
        EapConfigInputUsername,                ///< This element's field type.
        EAP_CONFIG_INPUT_FIELD_PROPS_DEFAULT,  ///< Desired EAP_CONFIG_FLAG values.
        L"User name:",                         ///< Label/name for this field.
        L"",                                   ///< Data entered by the user for this field. (When passing this list of fields back to the caller, leave this value blank.)
        1,                                     ///< Minimum length (in bytes) for valid user data.
        UNLEN + 1                              ///< Maximum length (in bytes) for valid user data.
    },

    {
        CRED_FIELD_SIZE,                       ///< Size of this structure.
        EapConfigInputPassword,                ///< This element's field type.
        EAP_CONFIG_INPUT_FIELD_PROPS_DEFAULT,  ///< Desired EAP_CONFIG_FLAG values.
        L"Password:",                          ///< Label/name for this field.
        L"",                                   ///< Data entered by the user for this field. (When passing this list of fields back to the caller, leave this value blank.)
        0,                                     ///< Minimum length (in bytes) for valid user data.
        PWLEN + 1                              ///< Maximum length (in bytes) for valid user data.
    },
};

/// A credential fields array, used when passing the credential fields from
/// this Eap Peer Method <--> EapHost <--> the Eap Supplicant <--> WinLogon.
static const EAP_CONFIG_INPUT_FIELD_ARRAY defaultCredentialInputArray =
{
    1,                            ///< Version
    2,                            ///< Number of fields in the array buffer
    defaultCredentialInputFields  ///< Pointer to the array of input fields needed.
};


/// Enum: States within the EAP state machines used by this EAP Peer Dll.
typedef enum _MYSTATE
{
        MYSTATE_Initial,
        MYSTATE_InteractiveUI,
        MYSTATE_AfterUserOK,
        MYSTATE_Done
} MYSTATE;

/// This structure is used to helps pass the user's username & password
/// between the interactive UI dialog & EAP Method.
typedef struct _EAP_NAME_DIALOG
{
    WCHAR               awszIdentity[ UNLEN + 1 ];      ///< A string buffer which will contain the authenticating user's username.
    WCHAR               awszPassword[ PWLEN + 1 ];      ///< A string buffer which will contain the authenticating user's password.
} EAP_NAME_DIALOG;


/// This structure is used to helps pass the Connection Data between
/// EapMethod and EapHost.
typedef struct _CONN_DATA_BLOB
{
        DWORD eapTypeId;
        WCHAR awszData[PATHLEN + 1];
}CONN_DATA_BLOB;


/// This structure is used to helps pass the User Data between
/// EapMethod and EapHost.
typedef struct _USER_DATA_BLOB
{
        DWORD eapTypeId;
        EAP_NAME_DIALOG eapUserNamePassword;
}USER_DATA_BLOB;

/**
  * EAP working buffer.
  *
  * This structure contains all persistent data that the EAP protocol needs to
  * operate.  The data buffer is created inside EapPeerBeginSession(),
  * persists through the entire authentication session, and is released inside
  * EapPeerEndSession().
  */
typedef struct _EAPCB
{
    LPVOID              pWorkBuffer;              ///< Pointer to self -- a pointer to the memory buffer containing this instance of the structure.
    DWORD              fFlags;                   ///< One or more flags that quantify the authentication process.
    MYSTATE            EapState;                 ///< Current state within EAP state machine.
    DWORD              dwResult;                 ///< The overall result of the authentication attempt.
    BOOL                 ClientOK;

    EapAttributes* pEapAttributes;
    HANDLE hTokenImpersonateUser;
    DWORD dwSizeofConnectionData;
    PBYTE pConnectionData;
    DWORD dwSizeofUserData;
    PBYTE pUserData;
    DWORD dwMaxSendPacketSize;

    CHAR                aszIdentity[ UNLEN + 1 ]; ///< A string buffer which will contain the authenticating user's username, once it is known.
    CHAR                aszPassword[ PWLEN + 1 ]; ///< A string buffer which will contain the authenticating user's password, once it is known.

    BYTE                bRecvPacketId;            ///< The EAP Identifier from the last packet received.

    PBYTE               pDataFromInteractiveUI;   ///< A buffer that will contain data from interactive UI displayed to the user.
    DWORD               dwSizeOfDataFromInteractiveUI; ///< The byte length of the buffer containing interactive UI data.

    DWORD               dwSizeOfUIContext;  ///< The byte length of the buffer containing UI Context.
    PBYTE               pUIContext;               ///< A buffer that will contain UI context data.

    EapAttributes* pMPPEKeyAttributes;   ///< Local pointer to the MPPE key EAP attributes generated by this EAP provider after a successful authentication transaction has been performed.
    EapAttribute *pFakeAttribute;
}EAPCB;


//
// EAP Functions whose function pointer is returned back in EapPeerGetInfo().
//

DWORD WINAPI SdkEapPeerInitialize(OUT EAP_ERROR** pEapError);

DWORD WINAPI SdkEapPeerBeginSession(
         IN DWORD dwFlags,
         IN const EapAttributes* const pAttributeArray,
         IN HANDLE hTokenImpersonateUser,
         IN DWORD dwSizeofConnectionData,
         IN BYTE* pConnectionData,
         IN DWORD dwSizeofUserData,
         IN BYTE* pUserData,
         IN DWORD dwMaxSendPacketSize,
         OUT EAP_SESSION_HANDLE* pSessionHandle,
         OUT EAP_ERROR** ppEapError
         );

DWORD WINAPI SdkEapPeerGetIdentity(
        IN DWORD flags,
        IN DWORD dwSizeofConnectionData,
        IN const BYTE* pConnectionData,
        IN DWORD dwSizeofUserData,
        IN const BYTE* pUserData,
        IN HANDLE hTokenImpersonateUser,
        OUT BOOL* pfInvokeUI,
        IN OUT DWORD* pdwSizeOfUserDataOut,
        OUT BYTE** ppUserDataOut,
        OUT __out LPWSTR* ppwszIdentity,
        OUT EAP_ERROR** ppEapError
         );

// A method exports either EapPeerGetIdentity (and EapPeerInvokeIdentityUI) or
// exports EapPeerSetCredentials (and sets the InvokeUserNameDlg regkey). The
// registry key controls which of the two apis will get called.
DWORD WINAPI SdkEapPeerSetCredentials(
         IN EAP_SESSION_HANDLE sessionHandle,
         IN __in LPWSTR pwszIdentity,
         IN __in LPWSTR pwszPassword,         
         OUT EAP_ERROR** pEapError
         );

DWORD WINAPI SdkEapPeerProcessRequestPacket(
         IN EAP_SESSION_HANDLE sessionHandle,
         IN DWORD cbReceivePacket,
         IN EapPacket* pReceivePacket,
         OUT EapPeerMethodOutput* pEapOutput,
         OUT EAP_ERROR** pEapError
         );


DWORD WINAPI SdkEapPeerGetResponsePacket(
         IN EAP_SESSION_HANDLE sessionHandle,
         IN OUT DWORD *pcbSendPacket,
         OUT EapPacket* pSendPacket,
         OUT EAP_ERROR** pEapError
         );

// This will get called either when a method says that it has completed auth.
// or when the lower layer receives an alternative result.
DWORD WINAPI SdkEapPeerGetResult(
         IN EAP_SESSION_HANDLE sessionHandle,
         IN EapPeerMethodResultReason reason,
         OUT EapPeerMethodResult* ppResult,
         OUT EAP_ERROR** pEapError
         );

DWORD WINAPI SdkEapPeerGetUIContext(
         IN EAP_SESSION_HANDLE sessionHandle,
         OUT DWORD* dwSizeOfUIContextData,
         OUT BYTE** pUIContextData,
         OUT EAP_ERROR** pEapError
      );

DWORD WINAPI SdkEapPeerSetUIContext(
        IN EAP_SESSION_HANDLE sessionHandle,
        IN DWORD dwSizeOfUIContextData,
        IN const BYTE* pUIContextData,
        OUT EapPeerMethodOutput* pEapOutput,
        OUT EAP_ERROR** pEapError
     );

DWORD WINAPI SdkEapPeerGetResponseAttributes(
        IN EAP_SESSION_HANDLE sessionHandle,
        OUT EapAttributes* pAttribs,
        OUT EAP_ERROR** pEapError
     );

DWORD WINAPI SdkEapPeerSetResponseAttributes(
         IN EAP_SESSION_HANDLE sessionHandle,
         IN EapAttributes* pAttribs,
         OUT EapPeerMethodOutput* pEapOutput,
         OUT EAP_ERROR** pEapError
      );

DWORD WINAPI SdkEapPeerEndSession(
         IN EAP_SESSION_HANDLE sessionHandle,
         OUT EAP_ERROR** pEapError
         );

DWORD WINAPI SdkEapPeerShutdown(OUT EAP_ERROR** pEapError);


//
// Helper Functions
//

/**
  * MakeResponseMessage() helper function: Construct a response message
  *
  * This function constructs the EAP provider's response to the received
  * EAP-Challenge packet.
  *
  *
  * @param  pwb          [in]  Pointer to the work buffer.
  *
  * @param  pSendBuf     [out] Pointer to a EapPacket structure.
  *
  * @param  cbSendBuf    [out]  Specifies the size, in bytes, of the buffer
  *                            pointed to by pSendBuf.
  *
  *
  * @return If the function succeeds, the return value is NO_ERROR.
  */
DWORD MakeResponseMessage(
    IN  EAPCB*           pwb,
    IN  DWORD * pcbSendBuf,
    OUT EapPacket * pSendBuf
    );


/**
  * GetIdentity() helper function: Get the username & password for the
  * user to be authenticated.
  *
  * This function obtains credentials for the user being authenticated.
  * It display UI requesting the username & password.
  *
  *
  * @param  hwndParent            [in]  Handle to the parent window for the
  *                                     user interface dialog.
  *
  * @param  pUserDataIn           [in]  Pointer to the user-specific data.
  *
  *
  * @param  dwSizeOfUserDataIn    [in]  Specifies the size of the
  *                                     user-specific data currently stored.
  *
  * @param  ppUserDataOut         [out] Pointer to a pointer that, on
  *                                     successful return, points to the
  *                                     identity data for the user.
  *
  * @param  pdwSizeOfUserDataOut  [out] Pointer to a DWORD variable that
  *                                     receives the size of the data pointed
  *                                     to by the ppUserDataOut parameter.
  *
  *
  * @return If the function succeeds, the return value is NO_ERROR.
  */
DWORD
GetIdentity(
    IN  HWND    hwndParent,
    IN  BYTE*   pUserDataIn,
    IN  DWORD   dwSizeOfUserDataIn,
    OUT BYTE**  ppUserDataOut,
    OUT DWORD*  pdwSizeOfUserDataOut,
    OUT __out LPWSTR* ppwszIdentityOut
);



DWORD GetConfigData(
    IN  HWND    hwndParent,
    IN  BYTE*   pConnectionDataIn,
    IN  DWORD   dwSizeOfConnectionDataIn,
    OUT BYTE**  ppConnectionDataOut,
    OUT DWORD*  dwSizeOfConnectionDataOut
);

//---------------------------------------------------------------------------
//
//    Dialog routines.
//
//---------------------------------------------------------------------------

/**
  * GetUsernameAndPassword() helper function: Display user credentials UI.
  *
  * This function displays an interactive UI (the IDD_IDENTITY_DIALOG dialog box)
  * requesting the user to enter their username and password.
  *
  *
  * @param  hwndParent        [in]  Handle to the parent window for the
  *                                 user interface dialog.
  *
  * @param  pEapNameDialog    [in]  Pointer to an EAP_NAME_DIALOG structure
  *                                 that, on successful return, will be filled
  *                                 in with the user's username & password.
  *                                 The caller is responsible for allocating
  *                                 and freeing this buffer.
  *
  *
  * @return If the function succeeds, the return value is NO_ERROR.
  */

DWORD GetUsernameAndPassword(
    IN  HWND                hwndParent,
    IN  USER_DATA_BLOB*    pEapUserData
    );


/**
  * This function handles Windows messages sent to the username/password UI
  * dialog box.  It is called by the Windows UI subsystem.    *
  *
  * @param  hWnd       [in]  Handle to the dialog box.
  *
  * @param  unMsg      [in]  Specifies the message.  Messages supported:
  *                            \li WM_INITDIALOG -- Initialize the dialog box.
  *                            \li WM_COMMAND -- The user has clicked on a
  *                                menu, control, or has used an accelerator
  *                                key.
  *
  * @param  wParam     [in]  Specifies additional message-specific
  *                          information.
  *
  * @param  lParam     [in]  Specifies additional message-specific
  *                          information.
  *
  *
  * @return The dialog box procedure should return TRUE if it processed the
  *         message, and FALSE if it did not.  If the dialog box procedure
  *         returns FALSE, the dialog manager performs the default dialog
  *         operation in response to the message.
  */

INT_PTR CALLBACK UsernameDialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
    );

/**
  * UsernameDialogProc() helper function: initialize the username dialog.
  *
  * This function handles the WM_INITDIALOG message, by initializing the
  * username dialog.
  *
  *
  * @param  hWnd       [in]  Handle to the dialog box.
  *
  * @param  lParam     [in]  Specifies additional message-specific
  *                          information.
  *
  *
  * @return FALSE, to prevent Windows from setting the default keyboard focus.
  */

BOOL InitUsernameDialog(
    IN  HWND    hWnd,
    IN  LPARAM  lParam
    );

/**
  *
  * This function handles the WM_COMMAND message, by saving any data the user
  * has entered into the text fields of the username dialog.
  *
  * @param  pEapNameDialog  [in]  Pointer to an EAP_NAME_DIALOG structure
  *                               that, on successful return, will be filled
  *                               in with the user's username & password. This
  *                               buffer must be allocated before this
  *                               function is called.
  *
  * @param  wId             [in]  The identifier of the menu item, control,
  *                               or accelerator.  Supported identifiers:
  *                                 \li IDOK     - the dialog's OK button. The
  *                                     user's credentials will be saved.
  *                                 \li IDCANCEL - the dialog's Cancel button.
  *
  * @param  hWndDlg         [in]  Handle to the dialog box.
  *
  *
  * @return TRUE if we processed this message; FALSE if we did not process
  *         this message.
  */

BOOL UsernameCommand(
    IN  USER_DATA_BLOB*    pEapUserData,
    IN  WORD                wId,
    IN  HWND                hWndDlg
    );

DWORD
GetConnectionData(
    IN  HWND                hwndParent,
    IN  CONN_DATA_BLOB*    pEapConfigData
);

INT_PTR CALLBACK
ConnectionDialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
);

BOOL
InitConnectionDialog(
    IN  HWND    hWnd,
    IN  LPARAM  lParam
);

BOOL
ConnectionCommand(
    IN  CONN_DATA_BLOB*    pEapConfigData,
    IN  WORD                wId,
    IN  HWND                hWndDlg
);


/**
  * MakeMPPEKeyAttributes() helper function: Build MPPE Key attributes.
  *
  * @param  pwb           [in]  Pointer to the work buffer.
  *
  * @return If the function succeeds, the return value is NO_ERROR.
  */
DWORD MakeMPPEKeyAttributes(
    IN EAPCB *  pwb
    );


/**
  * FillMppeKeyAttribute() helper function: Construct MPPE Key attributes.
  *
  * This function constructs MPPE encryption key attributes and saves them
  * into the EAPCB work buffer.
  *
  * @return If the function succeeds, the return value is NO_ERROR.
  */
DWORD FillMppeKeyAttribute(IN EAPCB *pwb,
                           IN BYTE *&bBuffer,
                           IN DWORD pattern,
                           IN BYTE bKeyDirection,
                           IN OUT EapAttribute &pAttrib);


/**
  * Construct a MPPE key vendor attribute.
  *
  * @param   bKeyDirection  [in]  Either MS_MPPE_SEND_KEY or MS_MPPE_RECV_KEY.
  *
  * @param   pMppeKeyData   [in]  The MPPE Key data.
  *
  * @param   cbMppeKeyData  [in]  The byte length of the MPPE Key data.
  *
  * @param   pAttrib        [out] The EAP attribute that will contain the
  *                               final MPPE Key vendor attribute.
  *
  *
  * @return A Win32 error code, indicating success or failure.
  */
DWORD ConstructMppeKeyAttribute(
    IN    BYTE  bKeyDirection,
    IN    PBYTE pMppeKeyData,
    IN    DWORD cbMppeKeyData,
    IN OUT EapAttribute *pAttrib
    );


DWORD GetXmlElementValue(
     IN  IXMLDOMDocument2 *pXmlDoc,
     IN  __in LPWSTR pElementName,
     IN  DWORD dwTypeOfDoc,
     OUT BSTR &pElementValue
);



LPWSTR NewString(
    IN const __in LPWSTR pInput
    ) throw();

DWORD CopyCredentialInputArray(
    OUT      EAP_CONFIG_INPUT_FIELD_ARRAY *pDest,
    IN const EAP_CONFIG_INPUT_FIELD_ARRAY *pSrc
    ) throw();


DWORD ConstructUserBlobFromCredentialInputArray(
    IN const EAP_METHOD_TYPE *pEapTypeIn,
    IN const EAP_CONFIG_INPUT_FIELD_ARRAY *pEapConfigFieldsArray,
    IN OUT   DWORD *pdwUserBlobSize,
    OUT      PBYTE *ppbUserBlob
    ) throw();


LPWSTR GetCredentialInputValue(
    IN const EAP_CONFIG_INPUT_FIELD_ARRAY  *pSrc,
    IN const DWORD                          valueType,
    IN const __in LPWSTR                         pValueLabel = NULL
    ) throw();


void FreeCredentialInputArray(
    IN OUT EAP_CONFIG_INPUT_FIELD_ARRAY *pArray
    ) throw();

#endif
