// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.


#ifndef SAMPLE_SUPPLICANT
#define SAMPLE_SUPPLICANT

#pragma once

#include <windows.h>
#include "EapTypes.h"
#include "eapmethodtypes.h"
#include "EapHostPeerTypes.h"
#include "EapHostPeerConfigApis.h"

//
// Demonstrate how to get function pointers to EAP Host Peer APIs.
//
DWORD GetEapHostPeerAPIsFunctionPointers();

//
// Demonstrate usage of EAPHostGetMethods, showing supplicant-specific
// method selection and then using any of the given installed EAP Method
// depending on what properties are supported by that EAP Method.
//
DWORD GetEapMethodFromListOfInstalledEapMethods(EAP_METHOD_TYPE *pEapMethodType);

//
// Initialize the resources used for authentication.
//
DWORD Initialize();

//
// Demonstrate the sequence of entire EAP Authentication Process.
//
DWORD BeginAuthentication();

//
// Demonstrate how UI calls need to be executed in a different thread.
// This function is the threadhandler that executes UI calls to EapHost.
//
DWORD HandleUICalls(LPVOID lpParameter);

//
// Demonstrate the handling of EapHostPeerResponseAction returned from EAP Host.
//
DWORD HandleEapHostPeerResponseAction(EapHostPeerResponseAction action);

//
// Demonstrate what to do when EAP Host returns GetResult as the action to take.
//
DWORD HandleEapHostPeerResponseAction_GetResult(EapHostPeerMethodResultReason reason);

//
// Demonstrate what to do when EAP Host returns InvokeUI as the action to take.
//
DWORD HandleEapHostPeerResponseAction_InvokeUI();

//
// Demonstrate retrieval and use of EapAttributes.
//
DWORD HandleEapHostPeerResponseAction_Respond();

//
// Demonstrate how the supplicant gets a response from EAP Host and sends to the authenticator.
//
DWORD HandleEapHostPeerResponseAction_Send();

//
//Clean up the resources used for authentication.
//
void CleanUp();

//
// Demonstrate how to send and receive packet to/from supplicant to the authenticator.
//
DWORD SendEapPackets(BYTE *pEapPacketToSend, DWORD dwPacketSize);
DWORD ReceiveEapPackets(LPVOID lpParameter);

//
// Demonstrate how to create IXMLDOMDocument2 interface pointer and load an xml file using it.
//
DWORD CreateDOMDocumentFromXML(DWORD type, IXMLDOMDocument2** pXMLDOMDocument2);

//
// Generates the Identity Request EAP Packet.
//
DWORD MakeIdentityRequestMessage(BYTE **pIdentityReq, DWORD *pIdentityReqLen);

//
// Do logon user and then impersonate the user.
//
DWORD Impersonate(HANDLE &pToken, LPCWSTR user, 
                                      LPCWSTR domain, LPCWSTR pswd);

//
// Stores the BLOB (either UserData or ConnectionData) into registry. The supplicant does not care about the 
// content of the BLOB.
//
DWORD StoreBLOBInRegistry(DWORD type, DWORD sizeOfBLOB, BYTE *pBLOB);

//
// Retrieves the BLOB (either UserData or ConnectionData) from registry. The supplicant does not care about the 
// content of the BLOB.
//
DWORD ReadBLOBFromRegistry(DWORD type, DWORD &sizeOfBLOB, BYTE *&pBLOB);

//
// Delete the registry entries for Config and User Blob if present when authentication fails
// using those values.
//
DWORD DeleteBLOBFromRegistry();


//---------------------------------------------------------------------------
//
//    Dialog routines.
//
//---------------------------------------------------------------------------


//
//  Display and Configure Eap Method() helper function
//  This function displays an UI (the IDD_DIALOG dialog box) that displays the list of 
//  installed EapMethods and Eap Properties supported by each of them.
//

DWORD DisplayInstalledEapMethods(IN  HWND hwndParent, IN  EAP_METHOD_INFO_ARRAY*  pEapMethodsData);

INT_PTR CALLBACK DialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
);

BOOL InitDialog(
    IN  HWND    hWndDlg,
    IN  LPARAM  lParam
);

BOOL
CommandProc(
    IN  EAP_METHOD_INFO_ARRAY*    pEapMethodsData,
    IN  WORD                wId,
    IN  HWND                hWndDlg
);

//
// Declaration of Callback function pointer that gets called when the client
// needs to re-authenticate due to change in state of health. (related to NAP)
//
typedef void (CALLBACK *NOTIFICATIONHANDLER) (
                 IN GUID connectionId,
                 IN void* pContextData
                 );

//
// Function Pointers Declaration of EAP Host Peer APIs.
//

typedef DWORD (APIENTRY* EapHostPeerInitialize)();

typedef void (APIENTRY* EapHostPeerUninitialize)();

typedef DWORD (APIENTRY* EapHostPeerBeginSession)(
   IN DWORD dwFlags,
   IN EAP_METHOD_TYPE eapType,
   IN const EapAttributes* const pAttributeArray,
   IN HANDLE hTokenImpersonateUser,
   IN DWORD dwSizeofConnectionData,
   IN const BYTE* const pConnectionData,
   IN DWORD dwSizeofUserData,
   IN const BYTE* const pUserData,
   IN DWORD dwMaxSendPacketSize,
   // If the supplicant is intrested in re-auth caused by SoH chagne,
   // it should provide a unique GUID.
   // When this function is called by PEAP inner method, it will be NULL.
   // 
   // When pConnectionId is NULL, func and pContextData will be ignored.
   IN const GUID* const pConnectionId,
   // if the function handler is NULL, pContextData will be ignored,
   // and it means the caller is not interested in SoH change notification
   // from EapQec.
   IN NOTIFICATIONHANDLER func,
   // a pointer to some data that the supplicant want to associate with
   // the connection when NotificationHandler call back is called.
   // When NotificationHandler is called, it will be called as:
   // func(*pCOnnectionId, pContextData).
   IN void* pContextData,
   OUT EAP_SESSIONID* pSessionId,
   OUT EAP_ERROR** ppEapError
   );


typedef DWORD (APIENTRY* EapHostPeerProcessReceivedPacket)(
   IN EAP_SESSIONID sessionHandle,
   IN DWORD cbReceivePacket,
   IN const BYTE* const pReceivePacket,
   OUT EapHostPeerResponseAction* pEapOutput,
   OUT EAP_ERROR** ppEapError
   );


typedef DWORD (APIENTRY* EapHostPeerGetSendPacket)(
   IN EAP_SESSIONID sessionHandle,
	OUT DWORD* pcbSendPacket,
	OUT BYTE** ppSendPacket,
	OUT EAP_ERROR** ppEapError
	);
   

typedef DWORD (APIENTRY* EapHostPeerGetResult)(
   IN EAP_SESSIONID sessionHandle,
	IN EapHostPeerMethodResultReason reason,
	OUT EapHostPeerMethodResult* ppResult, 
	OUT EAP_ERROR** ppEapError         
	   );


typedef DWORD (APIENTRY* EapHostPeerGetUIContext)(
   IN EAP_SESSIONID sessionHandle,
   OUT DWORD* pdwSizeOfUIContextData,
   OUT BYTE** ppUIContextData,
	OUT EAP_ERROR** ppEapError
   );


typedef DWORD (APIENTRY* EapHostPeerSetUIContext)(
   IN EAP_SESSIONID sessionHandle,
   IN DWORD dwSizeOfUIContextData,
   IN const BYTE* const pUIContextData,
   OUT EapHostPeerResponseAction* pEapOutput,
	OUT EAP_ERROR** ppEapError
	);


typedef DWORD (APIENTRY* EapHostPeerGetResponseAttributes)(
   IN EAP_SESSIONID sessionHandle,
   OUT EapAttributes* pAttribs,
	OUT EAP_ERROR** ppEapError
   );


typedef DWORD (APIENTRY* EapHostPeerSetResponseAttributes)(
   IN EAP_SESSIONID sessionHandle,
   IN const EapAttributes* const pAttribs,
   OUT EapHostPeerResponseAction* pEapOutput,
	OUT EAP_ERROR** ppEapError
   );


typedef DWORD (APIENTRY* EapHostPeerGetAuthStatus)(
   IN EAP_SESSIONID sessionHandle,
   IN EapHostPeerAuthParams authParam,
   OUT DWORD* pcbAuthData,
   OUT BYTE** ppAuthData,
   OUT EAP_ERROR** ppEapError   
   );


typedef DWORD (APIENTRY* EapHostPeerEndSession)(
   IN EAP_SESSIONID sessionHandle,
   OUT EAP_ERROR** ppEapError
   );


typedef DWORD (APIENTRY* EapHostPeerClearConnection)(
   IN GUID *connectionId,
   OUT EAP_ERROR** ppEapError
   );

typedef void (APIENTRY* EapHostPeerFreeEapError)(IN EAP_ERROR* pEapError); 

#endif
