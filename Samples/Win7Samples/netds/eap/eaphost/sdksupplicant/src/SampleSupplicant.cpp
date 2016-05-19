// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "SampleSupplicant.h"
#include "memory.h"
#include "SdkCommon.h"
#include "resource.h"
#include "msxml6.h"

using namespace SDK_METHOD_SAMPLE_COMMON;

#define CONNECTION_PROPERTIES    1
#define USER_PROPERTIES                 2

/// Store the SessionId for the entire Authentication Session.
EAP_SESSIONID eapSessionId = 0;
/// Handle to the library that implements EapHost Peer APIs.
HMODULE handleLibrary = NULL;
/// Stores the current action that suppliacnt has to take.
EapHostPeerResponseAction action = EapHostPeerResponseNone;

/// GUID that supplicant-eaphost use for NAP Callback function.
const GUID connectionId = {0x7128fce8,0x3ca0,0x4708,{0x9c, 0x8a, 0xad, 0x76, 0xc1, 0xb2, 0xfc, 0x53}};

/// Stores the connection data.
BYTE *connectionData = NULL;
/// Stores  the size of the connection data.
DWORD sizeConnectionData = 0;
/// Stores the user data.
BYTE *userData = NULL;
/// Stores the size of the user data.
DWORD sizeUserData = 0;

/// The execution of runtime APIs and UI APIs should be processed by two different threads.

/// The event used by Runtime thread to wait for a signal from UI thread.
HANDLE event_WaitforUIThread = NULL;

/// The event used by UI thread to wait for a signal from Runtime thread.
HANDLE event_WaitforRuntimeThread = NULL;

/// Handle to a thread that handles UI calls (API - EapHostPeerInvokeInteractiveUI) to EapHost.
HANDLE UIThreadHandle = NULL;

/// Data that is shared between the Runtime thread and UI thread.
DWORD dwSizeofUIContextData = 0;
BYTE* pUIContextData = NULL;
DWORD dwSizeOfDataFromInteractiveUI = 0;
BYTE* pDataFromInteractiveUI = NULL;
EAP_METHOD_TYPE eapType = {0};

/// Stores the most recent packet received from Authenticator.
BYTE *pReceivedPacket = NULL;
/// Stores the size of the most recent packet received from Authenticator.
DWORD gdwReceivedPacketLen = 0;

/// Handle to an event that is used to signal between the receiving thread and 
/// authentication thread.
HANDLE event_ReceivePacket = NULL;
BOOL needToWaitForEvent = TRUE;

/// Global variables are defined in supplicantMain.cpp
extern BYTE *g_pIdentityRequest;
extern DWORD g_dwIdentityRequestPacketLen;

//
// Declaration to EAP Host Peer APIs function pointers.
//
EapHostPeerInitialize funcEapHostPeerInitialize = NULL;
EapHostPeerUninitialize funcEapHostPeerUninitialize = NULL;
EapHostPeerBeginSession funcBeginSession = NULL;
EapHostPeerProcessReceivedPacket funcProcessReceivedPacket = NULL;
EapHostPeerGetSendPacket funcGetSendPacket = NULL;
EapHostPeerGetResult funcGetResult = NULL;
EapHostPeerGetUIContext funcGetUIContext = NULL;
EapHostPeerSetUIContext funcSetUIContext = NULL;
EapHostPeerGetResponseAttributes funcGetRespAttributes = NULL;
EapHostPeerSetResponseAttributes funcSetRespAttributes = NULL;
EapHostPeerGetAuthStatus funcGetAuthStatus = NULL;
EapHostPeerEndSession funcEndSession = NULL;
EapHostPeerClearConnection funcClearConnection = NULL;
EapHostPeerFreeEapError funcFreeEapError = NULL;

//
// Demonstrate how to get function pointers to EAP Host Peer APIs.
//
DWORD GetEapHostPeerAPIsFunctionPointers()
{
	DWORD retCode = NO_ERROR;

	//
	//Load the library -- eappprxy.dll that implemenst the Peer Side EapHost Runtime APIs.
	//
	handleLibrary = LoadLibrary(L"eappprxy.dll");
	if (handleLibrary == NULL)
	{
		retCode = GetLastError();
		EapTrace("Loading eappprxy.dll failed: %d\n", retCode);
		goto Cleanup;
	}

	//
	//Get the function pointer to all the functions implemented inside the dll.
	//
	funcEapHostPeerInitialize = (EapHostPeerInitialize)
			GetProcAddress(handleLibrary, (LPCSTR)"EapHostPeerInitialize");
	funcEapHostPeerUninitialize = (EapHostPeerUninitialize)
			GetProcAddress(handleLibrary, (LPCSTR)"EapHostPeerUninitialize");
	funcBeginSession = (EapHostPeerBeginSession)
			GetProcAddress(handleLibrary, (LPCSTR)"EapHostPeerBeginSession");
	funcProcessReceivedPacket = (EapHostPeerProcessReceivedPacket)
			GetProcAddress(handleLibrary, (LPCSTR)"EapHostPeerProcessReceivedPacket");
	funcGetSendPacket = (EapHostPeerGetSendPacket)
			GetProcAddress(handleLibrary, (LPCSTR)"EapHostPeerGetSendPacket");
	funcGetResult = (EapHostPeerGetResult)
			GetProcAddress(handleLibrary, (LPCSTR)"EapHostPeerGetResult");
	funcGetUIContext = (EapHostPeerGetUIContext)
			GetProcAddress(handleLibrary, (LPCSTR)"EapHostPeerGetUIContext");
	funcSetUIContext = (EapHostPeerSetUIContext)
			GetProcAddress(handleLibrary, (LPCSTR)"EapHostPeerSetUIContext");
	funcGetRespAttributes = (EapHostPeerGetResponseAttributes)
			GetProcAddress(handleLibrary, (LPCSTR)"EapHostPeerGetResponseAttributes");
	funcSetRespAttributes = (EapHostPeerSetResponseAttributes)
			GetProcAddress(handleLibrary, (LPCSTR)"EapHostPeerSetResponseAttributes");
	funcGetAuthStatus = (EapHostPeerGetAuthStatus)
			GetProcAddress(handleLibrary, (LPCSTR)"EapHostPeerGetAuthStatus");
	funcEndSession = (EapHostPeerEndSession)
			GetProcAddress(handleLibrary, (LPCSTR)"EapHostPeerEndSession");
	funcClearConnection = (EapHostPeerClearConnection)
			GetProcAddress(handleLibrary, (LPCSTR)"EapHostPeerClearConnection");
	funcFreeEapError = (EapHostPeerFreeEapError)
			GetProcAddress(handleLibrary, (LPCSTR)"EapHostPeerFreeEapError");

	if (funcBeginSession ==  NULL ||
		funcProcessReceivedPacket == NULL ||
		funcGetSendPacket == NULL ||
		funcGetResult == NULL ||
		funcGetUIContext == NULL ||
		funcSetUIContext == NULL ||
		funcGetRespAttributes == NULL ||
		funcSetRespAttributes == NULL ||
		funcGetAuthStatus == NULL ||
		funcEndSession == NULL ||
		funcClearConnection == NULL ||
		funcFreeEapError == NULL
		)
	{
		retCode = HRESULT_CODE(E_POINTER);
	}

Cleanup:
	return retCode;
}

//
// Demonstrate usage of EAPHostGetMethods, showing supplicant-specific
// method selection and then using any of the given installed EAP Method
// depending on what properties are supported by that EAP Method.
//
DWORD GetEapMethodFromListOfInstalledEapMethods(EAP_METHOD_TYPE *pEapMethodType)
{
	DWORD retCode = ERROR_SUCCESS;
	EAP_METHOD_INFO_ARRAY eapMethodsInfo = {0};
	EAP_ERROR *pEapError = NULL;
	HWND hwndParent = {0};

	// Sanity Check
	if(!pEapMethodType)
	{
		//Report Error
		retCode = ERROR_INVALID_PARAMETER;
		goto Cleanup;
	}

	// Call the EapHost Config API to get the list of Eap Methods installed and the properties each
	// one supports.
	retCode = EapHostPeerGetMethods(&eapMethodsInfo,
	                                      &pEapError);
	if(retCode != ERROR_SUCCESS)
	{
		//Report Error
		EapHostPeerFreeErrorMemory(pEapError);
		goto Cleanup;
	}

	//
	// Raises a GUI to display the information that is received from the EapHostPeerGetMethods API.
	// One can also configure EapMethod from the GUI.
	retCode = DisplayInstalledEapMethods(hwndParent, &eapMethodsInfo);
	if(retCode != ERROR_SUCCESS)
	{
		//Report Error
	}

	//
	// For demonstration purpose, we select that we will be using EapMethod - EAPTYPE for authentication.
	//
	pEapMethodType->eapType.type = EAPTYPE;
	pEapMethodType->eapType.dwVendorId = 0;
	pEapMethodType->eapType.dwVendorType = 0;
	pEapMethodType->dwAuthorId = AUTHOR_ID;

	//
	// Free the Memory that EapHost has allocated.
	//
	EapHostPeerFreeMemory((BYTE *)eapMethodsInfo.pEapMethods);

Cleanup:
	return retCode;
}


//
// Demonstrate how does supplicant find out from EAPHost that
// it needs to re-authenticate when the health state changes.
//
void SdkEapNotificationHandler(
                 IN GUID connectionId,
                 IN void* pContextData
                 )
{
	DWORD retCode = ERROR_SUCCESS;
	
	//
	// Verify if the input parameters --- connectionId and ContextData 
	// are the same as passed to the BeginSession API.
	//
	UNREFERENCED_PARAMETER(connectionId);
	UNREFERENCED_PARAMETER(pContextData);

	//
	// Check if any active authentication session is in progress.
	// If yes, we need to end the active session and start a fresh authentication session.
	// In this Sample code, we assume that the global variable "eapSessionId" is set 
	// when an active authentication is taking place and is reset to NULL when authentication
	// complete.
	//
	if(eapSessionId) // An Authentication Session is in Progress.
	{
		// End the current authentication session.
		EAP_ERROR *pEapError = NULL;
		retCode = funcEndSession(eapSessionId, &pEapError);
		if(retCode != ERROR_SUCCESS)
		{
			//Report Error
			funcFreeEapError(pEapError);
			return;
		}
	}
	
	//
	// Re-Auth since there is a change in the health state of the supplicant.
	//
	retCode = BeginAuthentication();
	if(retCode != ERROR_SUCCESS)
	{
		//Report Error
	}
	
}

//
// Initialize the resources used for authentication.
//
DWORD Initialize()
{
	DWORD retCode = ERROR_SUCCESS;

	//
	// Get function pointers to EAP Host Peer APIs
	// 
	retCode = GetEapHostPeerAPIsFunctionPointers();
	if(retCode != ERROR_SUCCESS)
	{
		//Report Error
		goto Cleanup;
	}

	//
	//Create an event which Runtime thread uses to listen for UI thread.
	//
	event_WaitforUIThread = CreateEvent(NULL,        // no security attributes
								FALSE,              // auto-reset enabled
								FALSE,              // non-signaled state initially
								NULL);              // event without a name
	if(event_WaitforUIThread == NULL)
	{
		// Report Error
		retCode = GetLastError();
		goto Cleanup;
	}

	//
	// Create an event which UI thread uses to listen to Runtime thread.
	//
	event_WaitforRuntimeThread = CreateEvent(NULL, //no security attributes
								FALSE,               // auto-reset enabled
								FALSE,               // non-signaled state initially
								NULL);               // event without a name
	if(event_WaitforRuntimeThread == NULL)
	{
		// Report Error
		retCode = GetLastError();
		goto Cleanup;
	}

	//
	// Create a thread that handles UI calls (API - EapHostPeerInvokeInteractiveUI) to EapHost.
	//
	UIThreadHandle = (HANDLE) CreateThread(
	                            NULL,
	                            0, 
	                            HandleUICalls,
	                            0,
	                            0, 
	                            NULL);
	if(UIThreadHandle == NULL)
	{
		retCode = GetLastError();
		goto Cleanup;
	}

	//
	// The following comment is just a suggestion.
	// 1. Create a thread (Listener thread) with function "ReceiveEapPackets" to be 
	//     executed by the thread which would listen for EapData from Authenticator. 
	//     Ofcourse, EapData will be contained in 802.1x or PPP packet.
	// 2. Create an event "event_ReceivePacket" that listener thread will fire when it 
	//     receives a EapData. The authenticating thread will pass the data to EapHost 
	//     to process through the API -- EapHostPeerProcessReceivedPacket().

Cleanup:
	return retCode;
}


//
// Demonstrate the handling of EapHostPeerResponseAction returned from EAP Host.
//
DWORD HandleEapHostPeerResponseAction(EapHostPeerResponseAction action)
{
	DWORD retCode = ERROR_SUCCESS;
	
	switch(action)
	{
		//
		// Suggests that authentication result packet has come from the authenticator.
		//
		case EapHostPeerResponseResult:
			retCode = HandleEapHostPeerResponseAction_GetResult(EapHostPeerMethodResultFromMethod);

			break;

		//
		// Suggests that UI needs to be invoked.
		//
		case EapHostPeerResponseInvokeUi:
			retCode = HandleEapHostPeerResponseAction_InvokeUI();

			break;

		//
		// Suggests that there are EapAttributes with EapHost/EapMethod that needs to be processed by supplicant.
		//
		case EapHostPeerResponseRespond:
			retCode = HandleEapHostPeerResponseAction_Respond();

			break;

		//
		// Suggests that some internal problem occured which cannot be fixed. Therefore close the current 
		// authentication session and begin authentication from start.
		//
		case EapHostPeerResponseStartAuthentication:
			{
				//End the existing Session.
				EAP_ERROR *pEapError = NULL;
				retCode = funcEndSession(eapSessionId, &pEapError);
				if(retCode != ERROR_SUCCESS)
				{
					//Report Error
					funcFreeEapError(pEapError);
					goto Cleanup;
				}

				//Start a new Session.
				retCode = BeginAuthentication();
				if(retCode != ERROR_SUCCESS)
				{
					//Report Error
					goto Cleanup;
				}
				
			}
			break;

		//
		// Suggests that there is a packet that EapMethod wants to send to Authenticator. Therefore,
		// supplicant should get the packet from EapMethod via EapHost and transport it to the authenticator.
		//
		case EapHostPeerResponseSend:
			retCode = HandleEapHostPeerResponseAction_Send();

			break;

		//
		// No action needs to be taken.
		//
		case EapHostPeerResponseNone:

			break;

		//
		// Suggests that no action needs to be taken.
		//
		case EapHostPeerResponseDiscard:

			break;

		default:

			break;
	}

	if(retCode != ERROR_SUCCESS)
	{
		// Report Error
		goto Cleanup;
	}

	//
	// If action is ResponseResult, and after we process the result, we are done and 
	// should end the authentication session.
	//
	if(action == EapHostPeerResponseResult)
	{
		EAP_ERROR *pEapError = NULL;
		retCode = funcEndSession(eapSessionId, &pEapError);
		if(retCode != ERROR_SUCCESS)
		{
			//Report Error
			funcFreeEapError(pEapError);
			goto Cleanup;
		}
		//
		// Listener thread should no longer expect more packet.
		//
		needToWaitForEvent = FALSE;
	}

Cleanup:
	return retCode;
}


//
// Demonstrate the sequence of entire EAP Authentication Process.
//
DWORD BeginAuthentication()
{
	DWORD retCode = ERROR_SUCCESS;
	HRESULT hr = S_OK;
	DWORD dwFlags = 0;
	size_t threadId = 0;
	EAP_ERROR *pEapError = NULL;
	HANDLE hTokenImpersonateUser = NULL;

	//Connection Data obtained from EapHost
	IXMLDOMDocument2 *pXMLConnectionProperties = NULL;
	BYTE *pConnectionDataOut = NULL;
	DWORD dwSizeOfConnectionDataOut = 0;

	//
	// Wait till the UI thread to complete the configuration.
	//
	retCode = WaitForSingleObject(event_WaitforUIThread, INFINITE);
	if(retCode != WAIT_OBJECT_0)   /// WAIT_FAILED, WAIT_ABANDONED, WAIT_TIMEOUT all error.
	{
		//Report Error.
		goto Cleanup;
	}

	//
	// There are two main ways to generate Config Blob for authentication.
	// One is through UI configuration.
	// Other one is using XML files and XML APIs of EapHost to generate Config Blob.
	// Below is demosntration of how to do that using XML files/APIs.
	//

	//
	// Get the Config Data and User Data from registry. 
	// Note: Registry is just an example of a place where to store such data!!!
	// If this is the first time user is authenticating, no such registry will be present.
	// These data are present in the registry if the EAP method wishes to save them for
	// future authentications.
	//
	retCode = ReadBLOBFromRegistry(CONNECTION_PROPERTIES, 
								sizeConnectionData,
								connectionData);
	if((retCode != ERROR_SUCCESS) && (retCode != ERROR_FILE_NOT_FOUND))
	{
		EapTrace("ReadBLOBFromRegistry returned error = %d",retCode);
		goto Cleanup;
	}

	if(retCode == ERROR_FILE_NOT_FOUND)
	{
		//
		// No registry present that stores Config Data.
		// Most common scenario -- Administrator pushes Config Data in XML file format
		// through group policy to each machines. These XML files are stored at some location
		// or plumbed to some registry.
		//
		// Here we assume that the Config XML file is stored on hard drive.
		//

		//
		// Demonstrates how to get Config blob from Config XML file using EAPHost APIs.
		// 

		// Convert XML File to XMLDomDocument
		retCode = CreateDOMDocumentFromXML(CONNECTION_PROPERTIES, &pXMLConnectionProperties);
		if(retCode != ERROR_SUCCESS)
		{
			//Report Error
			goto Cleanup;
		}

		// Get XMLDOMElement from XMLDOMDocument.
		IXMLDOMElement  *pEapConnNode;
      		hr = pXMLConnectionProperties->get_documentElement(&pEapConnNode);

		// Call EapHost Config API to convert XMLDomElement to Config BLOB
		retCode = EapHostPeerConfigXml2Blob(
					dwFlags,
					pEapConnNode,
					&dwSizeOfConnectionDataOut,
					&pConnectionDataOut,
					&eapType,
					&pEapError
					);
		if(retCode != ERROR_SUCCESS)
		{
			//Report Error
			EapHostPeerFreeErrorMemory(pEapError);
			goto Cleanup;
		}

		if(pConnectionDataOut)
		{
			//
			// Copy it to the global variable connectiondata.
			//
			sizeConnectionData = dwSizeOfConnectionDataOut;
			retCode = AllocateMemory(sizeConnectionData, (PVOID*)&connectionData);
			if (retCode != NO_ERROR)
				goto Cleanup;
			CopyMemory(connectionData, pConnectionDataOut, dwSizeOfConnectionDataOut);
			
			//
			// Finally free the ConnectionData Buffer obtained from EapHost.
			//
			EapHostPeerFreeMemory(pConnectionDataOut);
		}
	}

       //
       // Check to see if user blob is saved from previous authentication which can be used
       // for this authentication.
       //
	retCode = ReadBLOBFromRegistry(USER_PROPERTIES, 
								sizeUserData,
								userData);
	if((retCode != ERROR_SUCCESS) && (retCode != ERROR_FILE_NOT_FOUND))
	{
		EapTrace("ReadBLOBFromRegistry returned error = %d",retCode);
		goto Cleanup;
	}

	if(retCode == ERROR_FILE_NOT_FOUND)
	{
		//
		// In certain cases like Guest Access at some location (e.g. an airport), one may 
		// receive an user XML file to use for authentication.
		// If such is the case, one needs to follow the same sequence demonstrated above
		// for getting the ConfigBlob from Config XML file.
		// So if required to generate UserBlob from User XML file ---
		// 1. Create a IXMLDOMElement from User XML file.
		// 2. Call the EapHost API -- EapHostPeerCredentailsXml2Blob that returns UserBlob
		// 3. Save it and finally call EapHostPeerFreeMemory when done with the user blob.
		//
	}


	//
	// This is just a demonstration of how to create an Impersonation Token.
	// Required by methods to access resources (user cert...).
	// One will have to pass VALID UserName, Domain and Password.
	// Below all the three parameters are passed as NULL.
	//
	retCode = Impersonate(hTokenImpersonateUser,
						NULL,
						NULL,
						NULL);

	//
	// Must call EapHostPeerInitialize() before calling any EapHostPeerXXX API.
	//
	retCode = funcEapHostPeerInitialize();
	if(retCode != ERROR_SUCCESS)
	{
		//Error Reporting
		goto Cleanup;
	}
	

	//
	// Call BeginSession() in the EAP Host. This starts the authentication process.
	//
	retCode = funcBeginSession(
				dwFlags,						//Flags
				eapType,						//EAP_METHOD_TYPE
				NULL,						//EapAttributes
				hTokenImpersonateUser,		//HANDLE
				sizeConnectionData,			//Connection Data Size
				connectionData,				//Connection Data
				sizeUserData,				//User Data Size
				userData, 					//User Data
				MAX_PACKET_SIZE,			//Max Packet
				&connectionId,				//ConnectionId
				SdkEapNotificationHandler, 	//Notification Call Back Handler
				(void*)threadId,				//Context Data (Thread Identifier)
				&eapSessionId,   			// Session Id
				&pEapError
				);
	if(retCode != ERROR_SUCCESS)
	{
		//Error Reporting
		funcFreeEapError(pEapError);
		goto Cleanup;
	}

	// Call EAPHost to process the received packet.

	//
	// First time this is called, the packet is received from NAS(Network Access Server)/Access Point.
	// The packet is EAP-RequestIdentity.
	// From then, the packet that is received is from the Authentication Server.
	//

	// Copy to the global variable.
	retCode = AllocateMemory(g_dwIdentityRequestPacketLen, (PVOID *)&pReceivedPacket);
	if(retCode != ERROR_SUCCESS)
	{
		//Report Error.
		goto Cleanup;
	}

	CopyMemory(pReceivedPacket, g_pIdentityRequest, g_dwIdentityRequestPacketLen);
	gdwReceivedPacketLen = g_dwIdentityRequestPacketLen;

	//
	// In a loop, take actions and and wait for next packet from authenticator until we receive
	// the Result packet.
	//
	while(action != EapHostPeerResponseResult)
	{
		//
		// Pass the received packet to EapMethod to process it.
		//
		retCode = funcProcessReceivedPacket(
				eapSessionId,						//Session Id	
				gdwReceivedPacketLen,				//Length of the Packet
				pReceivedPacket,					//Packet
				&action,								//EapHostPeerResponseAction
				&pEapError
				);
		if(retCode != ERROR_SUCCESS)
		{
			// Error Reporting
			funcFreeEapError(pEapError);
			goto Cleanup;
		}

		//
		// EapMethod returns an action after processing the received packet.
		// Supplicant should take appropriate action.
		//
		retCode = HandleEapHostPeerResponseAction(action);
		if(retCode != ERROR_SUCCESS)
		{
			// Error Reporting
			goto Cleanup;
		}

		retCode = FreeMemory((PVOID *)&pReceivedPacket);
		if(retCode != ERROR_SUCCESS)
		{
			// Error Reporting
			goto Cleanup;
		}
		gdwReceivedPacketLen = 0;

		//
		// Till we receive the Result packet, we should wait for an event which the listener thread
		// will set when it receives a packet from authenticator.
		//

	}

Cleanup:
	//
	// userData, connectionData and pReceivedPacket all get freed in Cleanup().
	//
	return retCode;
}

//
// Demonstrate how UI calls need to be executed in a different thread.
// This function is the threadhandler that executes UI calls to EapHost.
//
DWORD HandleUICalls(LPVOID lpParameter)
{
	DWORD retCode = ERROR_SUCCESS;
	EAP_ERROR *pEapError = NULL;
	HWND hwndParent = {0};

	UNREFERENCED_PARAMETER(lpParameter);

	retCode = GetEapMethodFromListOfInstalledEapMethods(&eapType);
	if(retCode != ERROR_SUCCESS)
	{
		// Either EapHostPeerGetMethods reported an error or
		// the required EapMethod is not installed.
		goto Cleanup;
	}

	retCode = SetEvent(event_WaitforUIThread);
	if(retCode == 0)
	{
		retCode = GetLastError();
		goto Cleanup;
	}
	
	while(needToWaitForEvent == TRUE)
	{
		retCode = WaitForSingleObject(event_WaitforRuntimeThread, INFINITE);
		if(retCode != WAIT_OBJECT_0)   /// WAIT_FAILED, WAIT_ABANDONED, WAIT_TIMEOUT all error.
		{
			//Report Error.
			goto Cleanup;
		}

		//
		// Always remember to free the memory that is allocated by EapHost CONFIG APIs.
		// While those received from RUNTIME APIs will be freed automatically by EapHost
		// when session ends.
		//
		dwSizeOfDataFromInteractiveUI = 0;
		if(pDataFromInteractiveUI != NULL)
			EapHostPeerFreeMemory(pDataFromInteractiveUI);
		
		//
		// Supplicant will always call InvokeInteractiveUI and EAPHost will take 
		// care of calling Identity and Interactive UI depending on the context.
		//
		retCode = EapHostPeerInvokeInteractiveUI(
					hwndParent,
					dwSizeofUIContextData,
					pUIContextData,
					&dwSizeOfDataFromInteractiveUI,
					&pDataFromInteractiveUI,
					&pEapError
					);
		if(retCode != ERROR_SUCCESS)
		{
			// Error Reporting
			//Config APIs error should be freed using EapHostPeerFreeErrorMemory API.
			EapHostPeerFreeErrorMemory(pEapError);
			goto Cleanup;
		}

		retCode = SetEvent(event_WaitforUIThread);
		if(retCode == 0)
		{
			retCode = GetLastError();
			goto Cleanup;
		}
		
	}

Cleanup:
	return retCode;
}


//
// Demonstrate retrieval and processing of EapAttributes.
//
DWORD HandleEapHostPeerResponseAction_Respond()
{
	DWORD retCode = ERROR_SUCCESS;
	EapAttributes in_attributes = {0};
	EapAttributes *out_attributes = NULL;
	EAP_ERROR *pEapError = NULL;
	DWORD numInAttributes = 0;
	DWORD numAttrCtr = 0;
	DWORD isCorrectAttr = 0;
	EapAttribute *pEapAttribute = NULL;

	//
	// Get the attributes from EapHost/EapMethod that needs to be processed by the supplicant.
	//
	retCode = funcGetRespAttributes(
					eapSessionId, 
					&in_attributes, 
					&pEapError);
	if(retCode != ERROR_SUCCESS)
	{
		// Error Reporting
		funcFreeEapError(pEapError);
		goto Cleanup;
	}

	//
	//Process the EapAtributes
	//
	numInAttributes = in_attributes.dwNumberOfAttributes;
	for(numAttrCtr = 0; numAttrCtr < numInAttributes; numAttrCtr++)
	{
		pEapAttribute = (in_attributes.pAttribs) + numAttrCtr;

		switch(pEapAttribute->eaType)
		{
			case 1:

				isCorrectAttr = 1;
				
				//
				// Expected Attribute ---- eaType = 1; dwLength = 4; pValue = 40 
				//
				{
					DWORD attrLength = 0;
					DWORD attrValue = 0;
					attrLength = pEapAttribute->dwLength;
					
					if(attrLength != sizeof(DWORD))
						isCorrectAttr = 0;

					attrValue = *((DWORD *)pEapAttribute->pValue);

					if(attrValue != EAPTYPE)
						isCorrectAttr = 0;

				}

			break;

			default:
				EapTrace("Supplicant does not process attributes except EapAttrType = 1");
			break;
		}
	}

	//
	// Create the Response attribute. We need to call EapHostPeerSetResponseAttributes even if
	// supplicant has no attributes to set. In that case call EapHostPeerSetResponseAttributes
	// with EapAttributes having 0 eap attributes.
	//

	retCode = AllocateAttributes(1, &out_attributes);
	if(retCode != ERROR_SUCCESS)
	{
		EapTrace("HandleEapHostPeerResponseAction_Respond : AllocateAttributes returned error = %d", retCode);
		goto Cleanup;
	}
	
	//
	// Response Attribute --- eaType = 2, dwLength = 4, pValue = 1 (if isCorrectAttr == TRUE) else 0.
	//

	retCode = AddAttribute(out_attributes, (EapAttributeType)2, sizeof(DWORD), (PVOID)&isCorrectAttr);
	if(retCode != ERROR_SUCCESS)
	{
		EapTrace("HandleEapHostPeerResponseAction_Respond : AddAttribute returned error = %d", retCode);
		goto Cleanup;
	}

	//
	// Set the response attributes to EapMethod via EapHost.
	//
	retCode = funcSetRespAttributes(
               			eapSessionId,
               			out_attributes,
               			&action,
               			&pEapError
               			);
	if(retCode != ERROR_SUCCESS)
	{
		// Error Reporting
		funcFreeEapError(pEapError);
		goto Cleanup;
	}

	retCode = HandleEapHostPeerResponseAction(action);
	if(retCode != ERROR_SUCCESS)
	{
		// Error Reporting
		goto Cleanup;
	}

Cleanup:
	//
	// Free the out_attributes.
	//
	FreeAttributes(&out_attributes);
	
	return retCode;
}


//
// Demonstrate what to do when EAP Host returns InvokeUI as the action to take.
//
DWORD HandleEapHostPeerResponseAction_InvokeUI()
{
	DWORD retCode = ERROR_SUCCESS;
	EAP_ERROR *pEapError = NULL;

	//
	// Get the current context of the EapMethod so that it can be passed to the 
	// EapHostPeerInvokeInteractiveUI API.
	//
	retCode = funcGetUIContext(
				eapSessionId, 
				&dwSizeofUIContextData, 
				&pUIContextData, 
				&pEapError
				);
	if(retCode != ERROR_SUCCESS)
	{
		// Error Reporting
		funcFreeEapError(pEapError);
		goto Cleanup;
	}

	//
	// Invoke the UI thread as it needs to make a call to EapHostPeerInvokeInteractiveUI API.
	//
	retCode = SetEvent(event_WaitforRuntimeThread);
	if(retCode == 0)
	{
		retCode = GetLastError();
		goto Cleanup;
	}

	//
	// Wait till the UI thread completes.
	//
	retCode = WaitForSingleObject(event_WaitforUIThread, INFINITE);
	if(retCode != WAIT_OBJECT_0)   /// WAIT_FAILED, WAIT_ABANDONED, WAIT_TIMEOUT all error.
	{
		//Report Error.
		goto Cleanup;
	}

	//
	// Set the context data received from the above call of EapHostPeerInvokeInteractiveUI.
	//
	retCode = funcSetUIContext(
				eapSessionId,
				dwSizeOfDataFromInteractiveUI,
				pDataFromInteractiveUI,
				&action,
				&pEapError
				);
	if(retCode != ERROR_SUCCESS)
	{
		// Error Reporting
		funcFreeEapError(pEapError);
		goto Cleanup;
	}

	//
	// EapHost returns an action to take depending on the context data passed to it.
	//
	retCode = HandleEapHostPeerResponseAction(action);
	if(retCode != ERROR_SUCCESS)
	{
		// Error Reporting
		goto Cleanup;
	}

Cleanup:
	return retCode;
}


//
// Demonstrate what to do when EAP Host returns GetResult as the action to take.
//
DWORD HandleEapHostPeerResponseAction_GetResult(EapHostPeerMethodResultReason reason)
{
	DWORD retCode = ERROR_SUCCESS;
	EAP_ERROR *pEapError = NULL;
	EapHostPeerMethodResult result = {0};
	
	//
	// Get the final result of authentication from EapHost.
	//
	retCode = funcGetResult(
				eapSessionId,
				reason,
				&result,
				&pEapError
				);
	if(retCode != ERROR_SUCCESS)
	{
		// Error Reporting
		funcFreeEapError(pEapError);
		goto Cleanup;
	}

	if(result.fIsSuccess == TRUE)
	{
		//
		// Save User and Connection Blob if indicated in the EapHostPeerMethodResult structure.
		//
		if(result.fSaveConnectionData == TRUE)
		{
			//
			// Store the Connection Data into Registry. This BLOB will be used next time when 
			// authentication is done.
			// Registry is just an example of a place to store it. 
			// In this example we store raw data in regsitry. One may wish to encrypt it.
			//
			retCode = StoreBLOBInRegistry(CONNECTION_PROPERTIES, 
										result.dwSizeofConnectionData, 
										result.pConnectionData);
			if(retCode != ERROR_SUCCESS)
			{
				EapTrace("StoreBLOBInRegistry returned error = %d",retCode);
				goto Cleanup;
			}
		}

		if(result.fSaveUserData == TRUE)
		{
			//
			// Store the User Data into Registry. This BLOB will be used next time when 
			// authentication is done.
			// Registry is just an example of a place to store it.
			// In this example we store raw data in regsitry. One may wish to encrypt it.
			//
			retCode = StoreBLOBInRegistry(USER_PROPERTIES, 
										result.dwSizeofUserData, 
										result.pUserData);
			if(retCode != ERROR_SUCCESS)
			{
				EapTrace("StoreBLOBInRegistry returned error = %d",retCode);
				goto Cleanup;
			}
		}
	}
	else
	{
		//
		// Delete the registry entries for Config and User Blob if present when authentication fails
		// using those values.
		//
		retCode = DeleteBLOBFromRegistry();
		if(retCode != ERROR_SUCCESS)
		{
			EapTrace("DeleteBLOBFromRegistry returned error = %d",retCode);
			goto Cleanup;
		}
	}

	//
	// Process the result and accordingly take further steps.
	//
	

Cleanup:
	return retCode;
}


//
// Demonstrate how the supplicant gets a response from EAP Host and sends to the authenticator.
//
DWORD HandleEapHostPeerResponseAction_Send()
{
	DWORD retCode = ERROR_SUCCESS;
	DWORD size = 0;
	BYTE* pSendPacket = NULL;
	EAP_ERROR *pEapError = NULL;

	//
	// Get the response packet from EapHost.
	//
	retCode = funcGetSendPacket(eapSessionId, &size, &pSendPacket, &pEapError);
	if(retCode != ERROR_SUCCESS)
	{
		//Report Error
		funcFreeEapError(pEapError);
		goto Cleanup;
	}

	retCode = SendEapPackets(pSendPacket, size);
	if(retCode != ERROR_SUCCESS)
	{
		//Report Error
		goto Cleanup;
	}

Cleanup:
	return retCode;
}


//
//Clean up the resources used for authentication.
//
void CleanUp()
{
	DWORD retCode = ERROR_SUCCESS;
	EAP_ERROR *pEapError = NULL;
	
	//
	// This function needs to be called only if BeginSession succeeds.
	// Call Clear Connection. Same connection can be used across multiple connections.
	//
	if(eapSessionId)
	{
		retCode = funcClearConnection((GUID *)&connectionId, &pEapError);
		if(retCode != ERROR_SUCCESS)
		{
			// Report Error
			funcFreeEapError(pEapError);
		}
	}

	//
	// Last thing to do with the EapHost is to call PeerUninitialize.
	//
	funcEapHostPeerUninitialize();

	//
	// Finally, Unload the eapprxy.dll
	//
	FreeLibrary(handleLibrary);

	//
	// Free the resources.
	//
	if(event_ReceivePacket)
		CloseHandle(event_ReceivePacket);
	if(event_WaitforRuntimeThread)
		CloseHandle(event_WaitforRuntimeThread);
	if(event_WaitforUIThread)
		CloseHandle(event_WaitforUIThread);
	if(UIThreadHandle)
		CloseHandle(UIThreadHandle);

	if(userData)
		FreeMemory((PVOID *)&userData);
	if(connectionData)
		FreeMemory((PVOID *)&connectionData);
	if(pReceivedPacket)
		FreeMemory((PVOID *)&pReceivedPacket);

	eapSessionId = 0;
}

//
// Demonstrate how to send and receive packet to/from supplicant to the authenticator.
//

DWORD SendEapPackets(BYTE *pEapPacketToSend, DWORD dwPacketSize)
{
	DWORD retCode = ERROR_SUCCESS;

	UNREFERENCED_PARAMETER(pEapPacketToSend);
	UNREFERENCED_PARAMETER(dwPacketSize);

	//
	// Code that wrap the EapPacket within 802.1x (in case of Access Point) or PPP (in case of Remote Access Server).
	// and sends it to the respective NAS.
	//
	
	return retCode;
}

DWORD ReceiveEapPackets(LPVOID lpParameter)
{
	DWORD retCode = ERROR_SUCCESS;

	UNREFERENCED_PARAMETER(lpParameter);

	while(needToWaitForEvent == TRUE)
	{
		//
		// Code that will listen for packets from the NAS (Access Point or Remote Access Server).
		// 

		//
		// Once a packet is received, copy the received packet to variable pReceivedPacket and 
		// update gdwReceivedPacketLen.
		//

		//
		// Set the event "event_ReceivePacket" so that authenticator thread wakes up and process 
		// the recived packet by passing it to EapHost.
		//

	}

	return retCode;
}


//
// Demonstrate how to create IXMLDOMDocument2 interface pointer and load an xml file using it.
//
DWORD CreateDOMDocumentFromXML(DWORD type, IXMLDOMDocument2** pXMLDOMDocument2)
{
	DWORD retCode = ERROR_SUCCESS;
	HRESULT hr = S_OK;
	VARIANT xmlFileNameVariant = {0};
	VARIANT_BOOL isSuccessful = {0};

	hr = CoInitialize(NULL);
	if(FAILED(hr))
	{
		//Report Error
		retCode = HRESULT_CODE(hr);
		goto Cleanup;
	}
	
	hr = CoCreateInstance(CLSID_DOMDocument60, NULL, CLSCTX_INPROC_SERVER, 
	       IID_IXMLDOMDocument2, (void**)&(*pXMLDOMDocument2));
	if(FAILED(hr))
	{
		//Report Error
		retCode = HRESULT_CODE(hr);
		goto Cleanup;
	}

	xmlFileNameVariant.vt = VT_BSTR;

	switch(type)
	{
		case CONNECTION_PROPERTIES:
			xmlFileNameVariant.bstrVal = SysAllocString(L"SdkEapConn.xml");
			if(xmlFileNameVariant.bstrVal == NULL)
			{
				retCode = ERROR_OUTOFMEMORY;
				goto Cleanup;
			}
			break;

		case USER_PROPERTIES:
			xmlFileNameVariant.bstrVal = SysAllocString(L"SdkEapUser.xml");
			if(xmlFileNameVariant.bstrVal == NULL)
			{
				retCode = ERROR_OUTOFMEMORY;
				goto Cleanup;
			}
			break;

		default:
			//Report Error
			break;
	}

	hr = (*pXMLDOMDocument2)->load(xmlFileNameVariant, &isSuccessful);
	if(FAILED(hr)) //Fails if isSuccessful is VARIANT_FALSE, no need to especially check isSuccessful
	{
		//Report Error
		retCode = HRESULT_CODE(hr);
		goto Cleanup;
	}

Cleanup:
	SysFreeString(xmlFileNameVariant.bstrVal);
	
	return retCode;
}

//
// Generates the Identity Request EAP Packet.
//
DWORD MakeIdentityRequestMessage(BYTE **pIdentityReq, DWORD *pIdentityReqLen)
{
	DWORD retCode = ERROR_SUCCESS;
	EapPacket *pIdentityReqEapPacket = NULL;
	WORD tempPacketLen = 0;

	*pIdentityReqLen = 5;  //Code = 1, Id = 1, Length = 2, Type = 1

	//
	//Allocate the buffer to store Identity Request packet.
	//
	retCode = AllocateMemory(*pIdentityReqLen, (PVOID *)&pIdentityReqEapPacket);
	if(retCode != ERROR_SUCCESS)
	{
		//Report Error
		goto Cleanup;
	}

	//
	// Fill the elments of the Identity Request Packet.
	//
	pIdentityReqEapPacket->Code = EapCodeRequest;
	pIdentityReqEapPacket->Id = 0;
	HostToWireFormat16((WORD)*pIdentityReqLen, (PBYTE)&tempPacketLen);
	CopyMemory(&(pIdentityReqEapPacket->Length), &tempPacketLen, sizeof(WORD));
	pIdentityReqEapPacket->Data[0] = 0x01; //Identity Request Type

	*pIdentityReq = (BYTE *)pIdentityReqEapPacket;

Cleanup:
	return retCode;
}


//
// Do logon user and then impersonate the user.
// pToken is the handle returned after impersonation.
// UserName, Password and Domain are required for Logon.
//
DWORD Impersonate(HANDLE &pToken, 
                                      LPCWSTR user, 
                                      LPCWSTR domain,
                                      LPCWSTR pswd)
{
	DWORD retCode = ERROR_SUCCESS;
	BOOL  fOk = FALSE;

	fOk = LogonUserW( user,
	              domain,
	              pswd,
	              LOGON32_LOGON_INTERACTIVE,
	              LOGON32_PROVIDER_DEFAULT,
	              &pToken );

	if ( fOk == FALSE )
	{
		retCode = GetLastError( );
		//Report Error
		goto Cleanup;
	}

	fOk = ImpersonateLoggedOnUser(pToken);
	if ( fOk == FALSE )
	{
		retCode = GetLastError( );
		//Report Error
		goto Cleanup;
	}

Cleanup:
	return retCode;
}




//
// Stores the BLOB (either UserData or ConnectionData) into registry. The supplicant does not care about the 
// content of the BLOB.
//
DWORD StoreBLOBInRegistry(DWORD type, DWORD sizeOfBLOB, BYTE *pBLOB)
{
	DWORD retCode = ERROR_SUCCESS;
	HKEY hKeyEap = NULL;
	bool regkeyopen = false;
	LPDWORD lpdwDisposition = NULL;

	//
	// Create the Registry Key --- HKLM\Software\SdkEap if not created before.
	// Location is picked randomly.
	//
	retCode = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
							L"SOFTWARE\\SdkEap", 
							0, 
							KEY_ALL_ACCESS,
						       &hKeyEap);
	if (retCode != ERROR_SUCCESS)
	{
		if (retCode == ERROR_FILE_NOT_FOUND)
		{
			// Create a new key.
			retCode = RegCreateKeyEx(HKEY_LOCAL_MACHINE,     
						L"SOFTWARE\\SdkEap", 
						0,
						NULL,
						REG_OPTION_VOLATILE,
						KEY_ALL_ACCESS, 
						NULL,
						&hKeyEap,
						lpdwDisposition);
			if (retCode != ERROR_SUCCESS)
			{
				EapTrace("StoreBLOBInRegistry: Could not open reg key");
				goto Cleanup;
			} 
			regkeyopen = true;
		}
		else
		{
			goto Cleanup;
		}
	}

	//
	// Set Values in to the above Registry Key.
	//
	wchar_t wszKey[128] = { L'\0' };
	if(type == CONNECTION_PROPERTIES)
		swprintf_s( wszKey, 128, L"ConnectionBlob_%d", EAPTYPE );
	else
		swprintf_s( wszKey, 128, L"UserBlob_%d", EAPTYPE );

	retCode = RegSetValueEx(
	                hKeyEap,
	                wszKey,
	                0,
	                REG_BINARY,
	                pBLOB,
	                sizeOfBLOB
	                );
	if ( retCode != ERROR_SUCCESS )
	{
		EapTrace("StoreBLOBInRegistry: Cannot set regkey %d", retCode);
		goto Cleanup;
	}

	RegCloseKey(hKeyEap); 

Cleanup:
	return retCode;
}


//
// Retrieves the BLOB (either UserData or ConnectionData) from registry. The supplicant does not care about the 
// content of the BLOB.
//
DWORD ReadBLOBFromRegistry(DWORD type, DWORD &sizeOfBLOB, BYTE *&pBLOB)
{
	DWORD retCode = ERROR_SUCCESS;
	HKEY hKeyEap = NULL;
	DWORD dwRegValueType = REG_BINARY;
	DWORD cbRegValueData = 0;

	//
	// Open the registry value.
	//
	retCode = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
	                                 L"SOFTWARE\\SdkEap", 
	                                 0, 
	                                 KEY_READ,
	                                 &hKeyEap);
	if (retCode != ERROR_SUCCESS)
	{
		goto Cleanup;
	}
	else
	{
		wchar_t wszKey[128] = { L'\0' };
		if(type == CONNECTION_PROPERTIES)
			swprintf_s( wszKey, 128, L"ConnectionBlob_%d", EAPTYPE );
		else
			swprintf_s( wszKey, 128, L"UserBlob_%d", EAPTYPE );
		
		//
		// Read the specified regvalue.
		//
		retCode = RegQueryValueExW(hKeyEap, wszKey, NULL, &dwRegValueType,
		                 				NULL, &cbRegValueData);
		if (retCode != ERROR_SUCCESS)
		{
			goto Cleanup;
		}
		else if (retCode == ERROR_SUCCESS && cbRegValueData > 0)
		{
			//
			// Make sure it's the right type.
			//
			if (dwRegValueType != REG_BINARY)
			{
				EapTrace("ReadBLOBFromRegistry(): Error -- registry value was not a string! Ignoring it.");
				retCode = NO_ERROR;
				goto Cleanup;
			}

			// Allocate a buffer to hold the value.
			retCode = AllocateMemory(cbRegValueData, (PVOID*)&pBLOB);
			if (retCode != NO_ERROR)
				goto Cleanup;

			// Get the real value. (Errors handled by code outside if() clause.)
			retCode = RegQueryValueExW(hKeyEap, wszKey, NULL, &dwRegValueType,
			                     (PBYTE)pBLOB, &cbRegValueData);
			if (retCode != ERROR_SUCCESS)
			{
				EapTrace("ReadBLOBFromRegistry(): Error -- couldn't get registry value! (error %d)", retCode);
				goto Cleanup;
			}
			
			sizeOfBLOB = cbRegValueData;
		}
	}

Cleanup:
	if (hKeyEap)
		RegCloseKey(hKeyEap);

	// If returning error, free any memory we allocated.
	if (retCode != NO_ERROR)
		FreeMemory((PVOID*)&pBLOB);

	return retCode;
}


//
// Delete the registry entries for Config and User Blob if present when authentication fails
// using those values.
//
DWORD DeleteBLOBFromRegistry()
{
	DWORD retCode = ERROR_SUCCESS;
	HKEY hkEapBlob = 0;
	const wchar_t eapRootKeyName[] = L"Software";
	const wchar_t eapKeyName[] = L"SdkEap";

	// Check if the key -- "HKLM\Software" exist.
	retCode = RegOpenKeyEx(HKEY_LOCAL_MACHINE, eapRootKeyName,
					0, KEY_ALL_ACCESS, &hkEapBlob);
	if(retCode != ERROR_SUCCESS)
	{
		// Trace Error
		goto Cleanup;
	}

	// Delete the subkey - "SdkEap" which 
	retCode = RegDeleteKeyW(hkEapBlob, eapKeyName);
	if(retCode != ERROR_SUCCESS)
	{
		// Trace Error
		goto Cleanup;
	}

Cleanup:
	if(hkEapBlob)
		RegCloseKey(hkEapBlob);

	return retCode;
}

//---------------------------------------------------------------------------
//
//    Dialog routines.
//
//---------------------------------------------------------------------------

/**
  * Display and Configure Eap Method() helper function
  *
  * This function displays an UI (the IDD_DIALOG dialog box) that displays the list of
  * installed EapMethods and Eap Properties supported by each of them.
  *
  * @param  hwndParent        [in]  Handle to the parent window for the
  *                                 user interface dialog.
  *
  *
  *
  * @return If the function succeeds, the return value is NO_ERROR.
  */
DWORD
DisplayInstalledEapMethods(
    IN  HWND                hwndParent,
    IN  EAP_METHOD_INFO_ARRAY*    pEapMethodsData
)
{
    DWORD dwErr  = NO_ERROR;
    int   result = 0;

    result = (int)DialogBoxParam(
                     GetModuleHandle(NULL),
                     MAKEINTRESOURCE(IDD_DIALOG),
                     hwndParent,
                     DialogProc,
                     (LPARAM)pEapMethodsData);

    if (result < 0)
    {
        dwErr = GetLastError();
        EapTrace("Hit error %d while displaying DisplayInstalledEapMethods dialog!",
                 dwErr);
    }

    return(dwErr);
}


INT_PTR CALLBACK DialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
)
{
	DWORD dwErr = FALSE;        // By default, return FALSE.
	EAP_METHOD_INFO_ARRAY*    pEapMethodsData = NULL;

	switch (unMsg)
	{
		case WM_INITDIALOG:
			dwErr = InitDialog(hWnd, lParam);
		break;

		case WM_COMMAND:
			pEapMethodsData = (EAP_METHOD_INFO_ARRAY *)((LONG_PTR)GetWindowLongPtr(hWnd, DWLP_USER));
			dwErr = CommandProc(pEapMethodsData, LOWORD(wParam), hWnd);
		break;
	}

	return(dwErr);
}


BOOL InitDialog(
    IN  HWND    hWndDlg,
    IN  LPARAM  lParam
)
{
	DWORD dwNumberOfEapMethods = 0;
	DWORD dwMethodCount = 0;
	EAP_METHOD_INFO *pLocalEapMethodInfo = NULL;
	HWND hWnd = NULL;
	EAP_METHOD_INFO_ARRAY  *eapMethodsInfo = NULL;
	LRESULT  lResult = 0;

	SetWindowLongPtr(hWndDlg, DWLP_USER, (LONG)lParam);

	eapMethodsInfo = (EAP_METHOD_INFO_ARRAY *)lParam;

 	hWnd = GetDlgItem(hWndDlg, IDC_METHODLIST);
	if (hWnd == NULL)              // GetDlgItem() returns NULL for errors.
	{
		EapTrace("Error -- InitDialog (error %d)",GetLastError());
		goto Cleanup;
	}

	//
	// Display the list of Eap Methods Installed with EapHost.
	//
	dwNumberOfEapMethods = eapMethodsInfo->dwNumberOfMethods;

	for(dwMethodCount = 0; dwMethodCount < dwNumberOfEapMethods; dwMethodCount++)
	{
		pLocalEapMethodInfo = &((eapMethodsInfo->pEapMethods)[dwMethodCount]);

		lResult = SendMessage(hWnd,     
			                           (UINT) LB_ADDSTRING,      
			                           (WPARAM) 0,     
			                           (LPARAM)pLocalEapMethodInfo->pwszFriendlyName);
		if(lResult == LB_ERR)
			goto Cleanup;
		
	}

Cleanup:
	return(FALSE);
}

BOOL
CommandProc(
    IN  EAP_METHOD_INFO_ARRAY*    pEapMethodsData,
    IN  WORD                wId,
    IN  HWND                hWndDlg
)
{
	BOOL fOk  = FALSE;
	HWND hWnd = NULL;
	DWORD retCode = ERROR_SUCCESS;
	DWORD selIndex = 0;
	EAP_ERROR *pEapError = NULL;
	EAP_METHOD_INFO *pLocalEapMethodInfo = NULL;
	
	WCHAR eapPropText[1000] = {L'\0'};
	DWORD eapPropTextCtr = 0;

	// Sanity checks.
	if (! pEapMethodsData)
	{
		EapTrace("CommandProc: input dialog pointer is NULL!");
		goto Cleanup;
	}

	//
	// Get the index of the selected EapMethod. Done above the switch block as
	// this step is required for both IDC_PROPERTIES and IDC_CONFIGURE.
	//
	hWnd = GetDlgItem(hWndDlg, IDC_METHODLIST);
	if (hWnd == NULL)              // GetDlgItem() returns NULL for errors.
	{
		EapTrace("Error -- CommandProc -- IDC_METHODLIST -- (error %d)",GetLastError());
		goto Cleanup;
	}
	
	selIndex =  (DWORD)SendMessage(hWnd,     
	                           (UINT) LB_GETCURSEL,      
	                           (WPARAM) 0,     
	                           (LPARAM) 0);
	if(selIndex == LB_ERR)
		goto Cleanup;
	
	pLocalEapMethodInfo = &((pEapMethodsData->pEapMethods)[selIndex]);

	switch(wId)
	{
		case IDC_METHODLIST:
			//
			// We already know about the index that is selected.
			// 1. Remove the Eap Properties corresponding to the earlier selected text.
			// 2. Check if the new selected Eap suppports configuration. Accordingly, enable/disable the 
			//     the Configure button.
			
			// 1.
			hWnd = GetDlgItem(hWndDlg, IDC_PROPERTYTEXT);
			if (hWnd == NULL)              // GetDlgItem() returns NULL for errors.
			{
				EapTrace("Error -- CommandProc -- IDC_PROPERTYTEXT -- (error %d)",GetLastError());
				goto Cleanup;
			}
			// eapPropText = NULL
			fOk = SetWindowText(hWnd, eapPropText);
			if(fOk == 0)
				goto Cleanup;

			// 2.
			hWnd = GetDlgItem(hWndDlg, IDC_CONFIGURE);
			if (hWnd == NULL)              // GetDlgItem() returns NULL for errors.
			{
				EapTrace("Error -- CommandProc -- IDC_CONFIGURE -- (error %d)",GetLastError());
				goto Cleanup;
			}
			{
				BOOL  isHighlighted = FALSE;
				if(!(pLocalEapMethodInfo->eapProperties & eapPropSupportsConfig))
				{
					isHighlighted = TRUE;
				}
				SendMessage(hWnd,     
						(UINT) BM_SETSTATE,      
						(WPARAM) isHighlighted,     
						(LPARAM) 0);
			}

			break;
			
		case IDC_PROPERTIES:
			//
			// Display the Properties that the selected Eap Method supports
			//
			
			hWnd = GetDlgItem(hWndDlg, IDC_PROPERTYTEXT);
			if (hWnd == NULL)              // GetDlgItem() returns NULL for errors.
			{
				EapTrace("Error -- CommandProc -- IDC_PROPERTYTEXT -- (error %d)",GetLastError());
				goto Cleanup;
			}

			if(pLocalEapMethodInfo->eapProperties & eapPropCipherSuiteNegotiation)
				eapPropTextCtr = swprintf_s(eapPropText, sizeof(eapPropText)/sizeof(wchar_t), L"CipherSuiteNegotiation \n");
			if(pLocalEapMethodInfo->eapProperties & eapPropMutualAuth)
				eapPropTextCtr += swprintf_s(eapPropText + eapPropTextCtr, sizeof(eapPropText)/sizeof(wchar_t), L"Mutual Authentication \n");
			if(pLocalEapMethodInfo->eapProperties & eapPropIntegrity)
				eapPropTextCtr += swprintf_s(eapPropText + eapPropTextCtr, sizeof(eapPropText)/sizeof(wchar_t), L"Integrity \n");
			if(pLocalEapMethodInfo->eapProperties & eapPropReplayProtection)
				eapPropTextCtr += swprintf_s(eapPropText + eapPropTextCtr, sizeof(eapPropText)/sizeof(wchar_t), L"Replay Protection \n");
			if(pLocalEapMethodInfo->eapProperties & eapPropConfidentiality)
				eapPropTextCtr += swprintf_s(eapPropText + eapPropTextCtr, sizeof(eapPropText)/sizeof(wchar_t), L"Confidentiality \n");
			if(pLocalEapMethodInfo->eapProperties & eapPropKeyDerivation)
				eapPropTextCtr += swprintf_s(eapPropText + eapPropTextCtr, sizeof(eapPropText)/sizeof(wchar_t), L"Key Derivation \n");
			if(pLocalEapMethodInfo->eapProperties & eapPropKeyStrength64)
				eapPropTextCtr += swprintf_s(eapPropText + eapPropTextCtr, sizeof(eapPropText)/sizeof(wchar_t), L"KeyStrength64 \n");
			if(pLocalEapMethodInfo->eapProperties & eapPropKeyStrength128)
				eapPropTextCtr += swprintf_s(eapPropText + eapPropTextCtr, sizeof(eapPropText)/sizeof(wchar_t), L"KeyStrength128 \n");
			if(pLocalEapMethodInfo->eapProperties & eapPropKeyStrength256)
				eapPropTextCtr += swprintf_s(eapPropText + eapPropTextCtr, sizeof(eapPropText)/sizeof(wchar_t), L"KeyStrength256 \n");
			if(pLocalEapMethodInfo->eapProperties & eapPropKeyStrength512)
				eapPropTextCtr += swprintf_s(eapPropText + eapPropTextCtr, sizeof(eapPropText)/sizeof(wchar_t), L"KeyStrength512 \n");
			if(pLocalEapMethodInfo->eapProperties & eapPropKeyStrength1024)
				eapPropTextCtr += swprintf_s(eapPropText + eapPropTextCtr, sizeof(eapPropText)/sizeof(wchar_t), L"KeyStrength1024 \n");
			if(pLocalEapMethodInfo->eapProperties & eapPropDictionaryAttackResistance)
				eapPropTextCtr += swprintf_s(eapPropText + eapPropTextCtr, sizeof(eapPropText)/sizeof(wchar_t), L"DictionaryAttackResistance \n");
			if(pLocalEapMethodInfo->eapProperties & eapPropFastReconnect)
				eapPropTextCtr += swprintf_s(eapPropText + eapPropTextCtr, sizeof(eapPropText)/sizeof(wchar_t), L"FastReconnect \n");
			if(pLocalEapMethodInfo->eapProperties & eapPropCryptoBinding)
				eapPropTextCtr += swprintf_s(eapPropText + eapPropTextCtr, sizeof(eapPropText)/sizeof(wchar_t), L"CryptoBinding \n");
			if(pLocalEapMethodInfo->eapProperties & eapPropSessionIndependence)
				eapPropTextCtr += swprintf_s(eapPropText + eapPropTextCtr, sizeof(eapPropText)/sizeof(wchar_t), L"SessionIndependence \n");
			if(pLocalEapMethodInfo->eapProperties & eapPropFragmentation)
				eapPropTextCtr += swprintf_s(eapPropText + eapPropTextCtr, sizeof(eapPropText)/sizeof(wchar_t), L"Fragmentation \n");
			if(pLocalEapMethodInfo->eapProperties & eapPropChannelBinding)
				eapPropTextCtr += swprintf_s(eapPropText + eapPropTextCtr, sizeof(eapPropText)/sizeof(wchar_t), L"ChannelBinding \n");
			if(pLocalEapMethodInfo->eapProperties & eapPropNap)
				eapPropTextCtr += swprintf_s(eapPropText + eapPropTextCtr, sizeof(eapPropText)/sizeof(wchar_t), L"Supports Nap \n");
			if(pLocalEapMethodInfo->eapProperties & eapPropStandalone)
				eapPropTextCtr += swprintf_s(eapPropText + eapPropTextCtr, sizeof(eapPropText)/sizeof(wchar_t), L"Standalone \n");
			if(pLocalEapMethodInfo->eapProperties & eapPropMppeEncryption)
				eapPropTextCtr += swprintf_s(eapPropText + eapPropTextCtr, sizeof(eapPropText)/sizeof(wchar_t), L"MppeEncryption \n");
			if(pLocalEapMethodInfo->eapProperties & eapPropTunnelMethod)
				eapPropTextCtr += swprintf_s(eapPropText + eapPropTextCtr, sizeof(eapPropText)/sizeof(wchar_t), L"TunnelMethod \n");
			if(pLocalEapMethodInfo->eapProperties & eapPropSupportsConfig)
				eapPropTextCtr += swprintf_s(eapPropText + eapPropTextCtr, sizeof(eapPropText)/sizeof(wchar_t), L"SupportsConfig \n");

			fOk = SetWindowText(hWnd, eapPropText);
			if(fOk == 0)
				goto Cleanup;

			break;

		case IDC_CONFIGURE:
			//
			// Invoke the ConfigUI for the selected Eap Method.
			//
			retCode = EapHostPeerInvokeConfigUI(
							hWndDlg,
							0,
							pLocalEapMethodInfo->eaptype,
							0,
							NULL,
							&sizeConnectionData,
							&connectionData,
							&pEapError);
			if(retCode != ERROR_SUCCESS)
			{
				//Report Error
				EapHostPeerFreeErrorMemory(pEapError);
				goto Cleanup;
			}
			break;

		case IDC_EXITDIALOG:
			EndDialog(hWndDlg, wId);
				
			break;
			
		default:

			break;
	}

	fOk = TRUE;
	
Cleanup:
	return fOk;
}


