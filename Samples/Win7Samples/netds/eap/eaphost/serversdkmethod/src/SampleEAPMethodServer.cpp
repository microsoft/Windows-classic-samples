// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.


#include "stdafx.h"
#include "SampleEAPMethodServer.h"
#include "SdkCommon.h"
#include "memory.h"
#include "strsafe.h"
#include "EapHostError.h"
#include "Resource.h"

using namespace SDK_METHOD_SAMPLE_COMMON;

extern HINSTANCE  g_hInstance;

//
// EAP Functions
//
void EapMethodAuthenticatorFreeErrorMemory(IN EAP_ERROR* pEapError)
{
	//Sanity Check
	if(!pEapError)
	{
		EapTrace("EapMethodAuthenticatorFreeErrorMemory() --- Input Paramter is NULL");
		goto Cleanup;
	}

        //
        //If RootCauseString in EapError, free it.
        //
        if(pEapError->pRootCauseString)
                FreeMemory((PVOID *)&(pEapError->pRootCauseString));

        //
        //If error string in EapError, free it.
        //
        if(pEapError->pRepairString)
                FreeMemory((PVOID *)&(pEapError->pRepairString));

	//
	//Finally, free the EapError structure.
	//
	FreeMemory((PVOID *)&pEapError);
	
Cleanup:
	return;
}


DWORD EapMethodAuthenticatorGetInfo(
         IN EAP_METHOD_TYPE* pEapType, 
         OUT EAP_AUTHENTICATOR_METHOD_ROUTINES* pEapInfo, 
         OUT EAP_ERROR** ppEapError
         )
{
	DWORD retCode = NO_ERROR;

	//Sanity Check
	if((!pEapType) || (!pEapInfo) || (!ppEapError))
	{
		EapTrace("EapMethodAuthenticatorGetInfo() --- One/Some of the paramters is/are NULL");
		retCode = ERROR_INVALID_PARAMETER;
		goto Cleanup;
	}

	//
	// Verify if pEapType passed by EapHost correctly matches the EapType of this DLL.
	//
	if ((pEapType->eapType.type != EAPTYPE) ||
	     (pEapType->dwAuthorId != AUTHOR_ID))
	{
		EapTrace("EapMethodAuthenticatorGetInfo() --- Input Eap Type Info does not match the supported Eap Type");
		retCode = ERROR_NOT_SUPPORTED;
		goto Cleanup;
	}

	ZeroMemory(pEapInfo, sizeof(EAP_AUTHENTICATOR_METHOD_ROUTINES));

	// Need to fill < dwSizeInBytes, pEapType> (???)

	//
	// Fill the function pointers inside pEapInfo structure.
	//
	pEapInfo->EapMethodAuthenticatorInitialize = SdkEapMethodAuthenticatorInitialize;
	pEapInfo->EapMethodAuthenticatorBeginSession = SdkEapMethodAuthenticatorBeginSession;
	pEapInfo->EapMethodAuthenticatorUpdateInnerMethodParams = SdkEapMethodAuthenticatorUpdateInnerMethodParams;
	pEapInfo->EapMethodAuthenticatorReceivePacket = SdkEapMethodAuthenticatorReceivePacket;
	pEapInfo->EapMethodAuthenticatorSendPacket = SdkEapMethodAuthenticatorSendPacket;
	pEapInfo->EapMethodAuthenticatorGetAttributes = SdkEapMethodAuthenticatorGetAttributes;
	pEapInfo->EapMethodAuthenticatorSetAttributes = SdkEapMethodAuthenticatorSetAttributes;
	pEapInfo->EapMethodAuthenticatorGetResult = SdkEapMethodAuthenticatorGetResult;
	pEapInfo->EapMethodAuthenticatorEndSession = SdkEapMethodAuthenticatorEndSession;
	pEapInfo->EapMethodAuthenticatorShutdown = SdkEapAuthenticatorShutdown;

Cleanup:
	if(retCode != ERROR_SUCCESS)
	{
		// Populate the EapError
		if(ppEapError)
		{
			DWORD errCode = ERROR_SUCCESS;
			errCode = AllocateandFillEapError(ppEapError,
										retCode, 
										0,
										NULL, NULL, NULL,
										NULL, NULL
										);
			if(errCode != ERROR_SUCCESS)
			{
				//Report Error
			}
		}
	}
	return retCode;
}


DWORD SdkEapMethodAuthenticatorInitialize(
         IN EAP_METHOD_TYPE* pEapType, 
         OUT EAP_ERROR** ppEapError
         )
{
	DWORD retCode = NO_ERROR;

	//Sanity Check
	if( (!pEapType) || (!ppEapError))
	{
		EapTrace("EapMethodAuthenticatorInitialize() --- One/Some of the paramters is/are NULL");
		retCode = ERROR_INVALID_PARAMETER;
		goto Cleanup;
	}

	//
	// Verify if pEapType passed by EapHost correctly matches the EapType of this DLL.
	//
	if ((pEapType->eapType.type != EAPTYPE) ||
	     (pEapType->dwAuthorId != AUTHOR_ID))
	{
		EapTrace("EapMethodAuthenticatorInitialize() --- Input Eap Method Type Info does not match the supported Eap Method Type");
		retCode = ERROR_NOT_SUPPORTED;
		goto Cleanup;
	}

	//
	// Initialize any resources which is required as long as this method DLL is loaded.
	//
	

Cleanup:
	if(retCode != ERROR_SUCCESS)
	{
		// Populate the EapError
		if(ppEapError)
		{
			DWORD errCode = ERROR_SUCCESS;
			errCode = AllocateandFillEapError(ppEapError,
										retCode, 
										0,
										NULL, NULL, NULL,
										NULL, NULL
										);
			if(errCode != ERROR_SUCCESS)
			{
				//Report Error
			}
		}
	}
	return retCode;
}


DWORD SdkEapMethodAuthenticatorBeginSession(
      // Flags to qualify the authentication process.
      IN DWORD dwFlags,
      // Identity of the user being authenticated
      IN LPCWSTR pwszIdentity,
      // Pointer to an array of attributes. This array contains attributes that 
      // describe the entity being authenticated. 
      IN const EapAttributes* const pAttributeArray,
      // Specifies the size in bytes of the data pointed to by pConnectionData. 
      // If pConnectionData is NULL, this member is zero. 
      IN DWORD dwSizeofConnectionData,
      // Pointer to connection data received from the authentication protocol's 
      // configuration user interface.
      IN const BYTE* const pConnectionData,
      // This is the maximum size of an eap packet that the authenticator can send.
      IN DWORD dwMaxSendPacketSize,
      // The session handle that identifies the current authentication session.
      OUT EAP_SESSION_HANDLE* pSessionHandle,
      // On an unsuccessful call, this will contain any error information about
      // the failure. This will be null on a successful call.
      OUT EAP_ERROR** ppEapError
      )
{
	DWORD retCode = NO_ERROR;
	EAPCB* pwb    = NULL;
	DWORD attribCount = 0;
	EapAttribute *pEapAttrib = NULL;

	// Sanity Check
	if( !pSessionHandle || !ppEapError)
	{
		EapTrace("EapMethodAuthenticatorBeginSession() --- One/Some of the paramters is/are NULL");
		retCode = ERROR_INVALID_PARAMETER;
		goto Cleanup;
	}

	//
	// Allocate work buffer. This will remain for the entire session. In each API after BeginSession,
	// the buffer will be passed as EAP_SESSION_HANDLE.
	//
	retCode = AllocateMemory(sizeof(EAPCB), (PVOID*)&pwb);
	if (retCode != NO_ERROR)
	{
		// AllocateMemory() already reported the error. Need to fill the EapError.
		goto Cleanup;
	}

	//
	// Save information passed in, will be used later
	//
	pwb->pWorkBuffer      = (PVOID)pwb;
	pwb->fFlags                = dwFlags;
	pwb->EapState           = MYSTATE_Initial;
	pwb->dwMaxSendPacketSize = dwMaxSendPacketSize;

	//
	// Save the identity information.
	//
	if(pwszIdentity != NULL)
	{
		 WideCharToMultiByte(
	        	CP_ACP,
	        	NO_FLAGS,
	        	pwszIdentity,
	        	AUTOMATIC_STRING_LENGTH,
	        	pwb->aszIdentity,
	        	UNLEN + 1,
	        	NULL,
	        	NULL );

		EapTrace("Identity: \"%s\"", pwb->aszIdentity);
	}

	//
	// Save the Connection Data. 
	// For demostration purpose only, the Connection Data passed here is the same data 
	// passed by configuring the Sample Eap Method. The configuration of Sample Eap Method 
	// required passing a UserName and Password. Therefore, this check that 
	// dwSizeofConnectionData == sizeof(USER_DATA_BLOB).
	//
	if(pConnectionData != NULL && (sizeof(USER_DATA_BLOB) == dwSizeofConnectionData))
	{
		pwb->dwSizeofConnectionData = dwSizeofConnectionData;
		retCode = AllocateMemory(dwSizeofConnectionData, (PVOID *)&(pwb->pConnectionData));
		if(retCode != NO_ERROR)
		{
			goto Cleanup;
		}
		CopyMemory(pwb->pConnectionData, pConnectionData, dwSizeofConnectionData);
	}

	//
	// Save EapAttributes if any.
	//
	if(pAttributeArray != NULL)
	{
		retCode = AllocateAttributes(pAttributeArray->dwNumberOfAttributes, &(pwb->pEapAttributes));
		if(retCode != NO_ERROR)
		{
			goto Cleanup;
		}

		for(attribCount = 0; attribCount < pwb->pEapAttributes->dwNumberOfAttributes; attribCount++)
		{
			pEapAttrib = &((pAttributeArray->pAttribs)[attribCount]);
			retCode = AddAttribute(pwb->pEapAttributes, pEapAttrib->eaType, pEapAttrib->dwLength, pEapAttrib->pValue);
			if(retCode != NO_ERROR)
			{
				goto Cleanup;
			}
		}
	}
  
	// Return the Session Handle.
	*pSessionHandle = (VOID *)pwb;

Cleanup:
	if(retCode != NO_ERROR)
	{
		if(pwb)
		{
			FreeMemory((PVOID *)&(pwb->pConnectionData));
			FreeAttributes(&(pwb->pEapAttributes));
			FreeMemory((PVOID*)&pwb);
		}

		// Populate the EapError
		if(ppEapError)
		{
			DWORD errCode = ERROR_SUCCESS;
			errCode = AllocateandFillEapError(ppEapError,
										retCode, 
										0,
										NULL, NULL, NULL,
										NULL, NULL
										);
			if(errCode != ERROR_SUCCESS)
			{
				//Report Error
			}
		}
	}

	return retCode;
}


DWORD SdkEapMethodAuthenticatorUpdateInnerMethodParams(
      // context handle as returned from a successful call to 
      // EapAuthenticatorBeginSession
      IN EAP_SESSION_HANDLE sessionHandle,
      IN DWORD dwFlags,
      IN CONST WCHAR* pwszIdentity,
      // Pointer to an array of attributes. This array contains attributes that 
      // describe the entity being authenticated. 
      IN const EapAttributes* const pAttributeArray,
      // On an unsuccessful call, this will contain any error information about
      // the failure. This will be null on a successful call.
      OUT EAP_ERROR** pEapError         
      )
{
	DWORD retCode = ERROR_SUCCESS;

	UNREFERENCED_PARAMETER(sessionHandle);
	UNREFERENCED_PARAMETER(dwFlags);
	UNREFERENCED_PARAMETER(pwszIdentity);
	UNREFERENCED_PARAMETER(pAttributeArray);
	UNREFERENCED_PARAMETER(pEapError);

	return retCode;
}


// The authenticator calls this any time it receives a packet that the eaphost
// needs to process. This should be called only after a successful call to
// EapAuthenticatorBeginSession.
DWORD SdkEapMethodAuthenticatorReceivePacket(
         // context handle as returned from a successful call to 
         // EapAuthenticatorBeginSession
         IN EAP_SESSION_HANDLE sessionHandle,
         // Specifies the size, in bytes, of the buffer pointed to by 
         // pReceivePacket
         IN DWORD cbReceivePacket,
         // Pointer to a buffer that contains the incoming EAP data received by 
         // the supplicant.
         IN const EapPacket* const pReceivePacket,
         // This enumeration tells the supplicant to take an appropriate action.
         // The supplicant will typically look at this action and either call 
         // another method on eaphost or do something else on its own.
         OUT EAP_METHOD_AUTHENTICATOR_RESPONSE_ACTION* pEapOutput,
         // On an unsuccessful call, this will contain any error information about
         // the failure. This will be null on a successful call.      
         OUT EAP_ERROR** pEapError
         )
{
	DWORD retCode = NO_ERROR;
	EAPCB*  pwb = NULL;
	DWORD   cbPacket = 0;
	EapPacket *pEapReceivePacket = NULL;
	size_t passwordLength = 0;

	// Sanity checks.
	if (!sessionHandle || !pEapOutput || !pEapError)
	{
		EapTrace("EapMethodAuthenticatorReceivePacket() --- One/Some of the paramters is/are NULL");
		retCode = ERROR_INVALID_PARAMETER;
		// Need to fill the EapError
		goto Cleanup;
	}

	pwb = (EAPCB*)sessionHandle;
	pEapReceivePacket = (EapPacket *)pReceivePacket;
	cbPacket = cbReceivePacket;

	//
	// Main state machine.
	//
	switch(pwb->EapState)
	{
		case MYSTATE_ReqSent:

			EapTrace("Authenticator state: RecvPacket  --- MYSTATE_ReqSent");

			if (pEapReceivePacket != NULL)
			{
				//
				// If we received a packet
				//
				if (pEapReceivePacket->Code == EapCodeResponse)
				{
					//
					// If we receive a response to our identity request, 
					// then process it.
					//
					retCode = GetPasswordFromResponse((BYTE *)pEapReceivePacket, 
					                                pwb->aszPassword,
					                                passwordLength);
					if (retCode != NO_ERROR)
					{    
						// Need to fill the EapError
						goto Cleanup;
					}
					else
					{
						//
						// Place UserName and Password in an Attribute.
						//
						retCode = MakeAuthenticationAttributes(
						                                    pwb->aszIdentity, 
						                                    pwb->aszPassword,
						                                    pwb);

						if (retCode != NO_ERROR)
						{
							EapTrace("MakeAuthenticationAttributes failed %d", retCode);
							goto Cleanup;
						}
						else
						{
							//
							// Now we have username and password obtained from the client.
							// The actual method will employ some mechanism to authenticate the client. 
							// Here we compare the username and password obtained from the client with
							// the username and password passed by the administrator while configuring 
							// the Sample Eap Method. The username and password is stored as 
							// ConnectionData in the EAPCB (i.e., Eap Session Handle).
							//

							retCode = VerifyAuthenticationAttributes(pwb);
							if(retCode != NO_ERROR)
							{
								EapTrace("VerifyAuthenticationAttributes failed %d", retCode);
								goto Cleanup;
							}

							*pEapOutput = EAP_METHOD_AUTHENTICATOR_RESPONSE_RESULT;
						}        
					}

					break;
				}
				else
				{
					//
					// Otherwise silently drop the packet. 
					// We should only get response
					//
					*pEapOutput = EAP_METHOD_AUTHENTICATOR_RESPONSE_DISCARD;
					
					break;
				}
			}

		break;

		default:
		//
		// Authenticator has to be in MYSTATE_ReqSent state when SdkEapMethodAuthenticatorReceivePacket
		// is called. Any other state is not acceptable according to the state machine of this Eap Method.
		//
		EapTrace("SdkEapMethodAuthenticatorReceivePacket -- Authenticator state: [default] Present State = %d", 
					pwb->EapState);
		// Need to fill the EapError
		retCode = ERROR_INVALID_STATE;
		break;
	}

 Cleanup:
	return retCode;
}


DWORD SdkEapMethodAuthenticatorSendPacket(
         // context handle as returned from a successful call to 
         // EapHostAuthenticatorBeginSession
         IN EAP_SESSION_HANDLE sessionHandle,
         // Id to use when constructing the SendPacket
         IN BYTE bPacketId,
         // Specifies the limit on the size, in bytes, on the packet generated
         // by eaphost. On a successful return, this will contain the size of the 
         // data added by the eap module.
         OUT DWORD* pcbSendPacket,
         // Pointer to a buffer that is allocated by the client and populated
         // by the eap module. The value of the incoming buffer is ignored and
         // the method populates it from the beginning of the buffer.
         OUT EapPacket* pSendPacket,
         // Timeout option for sending the packet
         OUT EAP_AUTHENTICATOR_SEND_TIMEOUT* pTimeout,
         // On an unsuccessful call, this will contain any error information about
         // the failure. This will be null on a successful call.
         OUT EAP_ERROR** ppEapError
         )
{
	DWORD retCode = NO_ERROR;
	EAPCB*  pwb = NULL;

	// Sanity Check
	if(!sessionHandle || !pcbSendPacket || !pSendPacket || !pTimeout || !ppEapError)
	{
		EapTrace("EapMethodAuthenticatorSendPacket() --- One/Some of the paramters is/are NULL");
		retCode = ERROR_INVALID_PARAMETER;
		// Need to fill the EapError
		goto Cleanup;
	}

	pwb = (EAPCB*)sessionHandle;

	//
	// Main state machine.
	//
	switch(pwb->EapState)
	{
		case MYSTATE_Initial:
			//
			// Create EAP-Challenge packet
			//
			retCode = MakeRequestMessage(pwb, bPacketId, pSendPacket, pcbSendPacket);
			if(retCode != ERROR_SUCCESS)
			{
				// Need to fill the EapError
				goto Cleanup;
			}

			*pTimeout = EAP_AUTHENTICATOR_SEND_TIMEOUT_INTERACTIVE;

			//
			// Since we have sent a request we change to the ReqSent state
			// where we will wait for a response.
			//
			pwb->EapState = MYSTATE_ReqSent;

		break;

		default:
			//
			// Authenticator has to be in MYSTATE_Initial state when SdkEapMethodAuthenticatorSendPacket
			// is called. Any other state is not acceptable according to the state machine of this Eap Method.
			//
			EapTrace("SdkEapMethodAuthenticatorSendPacket -- Authenticator state: [default] Present State = %d", 
					pwb->EapState);
			// Need to fill the EapError
			retCode = ERROR_INVALID_STATE;
		break;
	}

Cleanup:
	return retCode;
}


  // Returns an array of attributes that the caller needs to act on.
  // The supplicant will call this when a call to 
  // EapHostAuthenticatorProcessRequestPacket returns EapHostAuthenticatorResponseRespond. 
DWORD SdkEapMethodAuthenticatorGetAttributes(
      // context handle as returned from a successful call to 
      // EapHostAuthenticatorBeginSession
      IN EAP_SESSION_HANDLE sessionHandle,
      // Array of attributes that the caller needs to act on.
      OUT EapAttributes* pAttribs,
      OUT EAP_ERROR** pEapError
      )
{
	DWORD retCode = NO_ERROR;

	UNREFERENCED_PARAMETER(sessionHandle);
	UNREFERENCED_PARAMETER(pAttribs);
	UNREFERENCED_PARAMETER(pEapError);

	return retCode;
}



// Sets an array of attributes that the caller wants the eap method to act on. 
DWORD SdkEapMethodAuthenticatorSetAttributes(
      // context handle as returned from a successful call to 
      // EapHostAuthenticatorBeginSession
      IN EAP_SESSION_HANDLE sessionHandle,
      IN const EapAttributes* const pAttribs,
      // This enumeration tells the supplicant to take an appropriate action.
      // The supplicant will typically look at this action and either call 
      // another method on eaphost or do something else on its own.
      OUT EAP_METHOD_AUTHENTICATOR_RESPONSE_ACTION* pEapOutput,
      // On an unsuccessful call, this will contain any error information about
      // the failure. This will be null on a successful call.      
      OUT EAP_ERROR** ppEapError
      )
{
	DWORD retCode = NO_ERROR;

	UNREFERENCED_PARAMETER(sessionHandle);
	UNREFERENCED_PARAMETER(pAttribs);
	UNREFERENCED_PARAMETER(pEapOutput);
	UNREFERENCED_PARAMETER(ppEapError);

	return retCode;
}


   // The authenticator will call this on completion of an authentication. This 
   // can happen in any of the following scenarios:
   // 1. A call to EapHostAuthenticatorReceivePacket returned 
   //    EAP_HOST_AUTHENTICATOR_RESPONSE_SUCCESS or EAP_HOST_AUTHENTICATOR_RESPONSE_FAILURE
   //    Even if the action returned above was a success, the authenticator can choose to call
   //    this method with a failure.
   // 2. The server can choose to terminate an authentication with a failure in the middle of
   //    an authentication.
DWORD SdkEapMethodAuthenticatorGetResult(
      // context handle as returned from a successful call to 
      // EapHostPeerBeginSession
      IN EAP_SESSION_HANDLE sessionHandle,
      // A structure that indicates the result and any state that the 
      // supplicant needs to save for future authentications.
      OUT EAP_METHOD_AUTHENTICATOR_RESULT* pResult, 
      // On an unsuccessful call, this will contain any error information about
      // the failure. This will be null on a successful call.      
      OUT EAP_ERROR** pEapError         
      )
{
	DWORD retCode = NO_ERROR;
	EAPCB*  pwb = NULL;

	//Sanity Check
	if(!sessionHandle || !pResult || !pEapError)
	{
		EapTrace("EapMethodAuthenticatorGetResult() --- One/Some of the paramters is/are NULL");
		retCode = ERROR_INVALID_PARAMETER;
		// Need to fill the EapError
		goto Cleanup;
	}

	pwb = (EAPCB*)sessionHandle;

	if(pwb->dwResult == AUTH_SUCCESS)
		pResult->fIsSuccess = TRUE;
	else
		pResult->fIsSuccess = FALSE;

	if(pResult->fIsSuccess == TRUE)
		pResult->dwFailureReason = 0;
	else
	{
		// Check if authentication failed or the server choose to terminate the authentication.
		if(pwb->dwResult == NO_ERROR)
			pResult->dwFailureReason = 1; // ERROR_EAPHOST_SERVER (??)
		else
			pResult->dwFailureReason = pwb->dwResult;
	}

	//
	// If we made a Success packet, create the MPPE Key Attribute
	// and give it to the EAP-Host (finally to the Authenticator).
	//
	if(pResult->fIsSuccess == TRUE)
	{
		retCode = MakeMPPEKeyAttributes(pwb);
		if(retCode != NO_ERROR)
		{
			// MakeMPPEKeyAttributes() reported the error. Need to fill the EapError.
			goto Cleanup;
		}
		pResult->pAuthAttribs = pwb->pMPPEKeyAttributes;
	}

Cleanup:
	return retCode;
}

// Ends the authentication session. This cleans up any state that the eap 
// method or eaphost might be keeping.
DWORD SdkEapMethodAuthenticatorEndSession(
      // context handle as returned from a successful call to 
      // EapHostPeerBeginSession. This will be set to NULL on a successful call.
      IN EAP_SESSION_HANDLE sessionHandle, 
      // On an unsuccessful call, this will contain any error information about
      // the failure. This will be null on a successful call.      
      OUT EAP_ERROR** ppEapError
   )
{
	DWORD  retCode = NO_ERROR;
	EAPCB* pwb = NULL;
	DWORD index = 0;
	EapAttribute *pAttr = NULL;

	// Sanity check.
	if (!sessionHandle || !ppEapError)
	{
		EapTrace("EapMethodAuthenticatorEndSession() --- One/Some of the paramters is/are NULL");
		retCode = ERROR_INVALID_PARAMETER;
		// Need to fill the EapError
		goto Cleanup;
	}

	pwb = (EAPCB *)sessionHandle;

	EapTrace("Identity: \"%s\"", pwb->aszIdentity);

	//
	// Free up User Attributes
	//
	if(pwb->pUserAttributes)
	{
		// Use SecureZeroMemory() here, to ensure that any authentication data is
		// initialized safely, before the memory block is freed back to the system.
		for(index = 0; index < pwb->pUserAttributes->dwNumberOfAttributes; index++)
		{
			pAttr = &((pwb->pUserAttributes->pAttribs)[index]);
			SecureZeroMemory(pAttr->pValue, pAttr->dwLength);
		}
	}
	FreeAttributes(&(pwb->pUserAttributes));

	//
	// Free up the MPPE Key Attributes
	//
	FreeAttributes(&(pwb->pMPPEKeyAttributes));

	//
	// Free the Connection Data.
	//
	SecureZeroMemory(pwb->pConnectionData, pwb->dwSizeofConnectionData);
	FreeMemory((PVOID*)&(pwb->pConnectionData));

	//
	// Free the Input Eap Attributes.
	//
	FreeAttributes(&(pwb->pEapAttributes));

	SecureZeroMemory(pwb, sizeof(EAPCB));
	FreeMemory((PVOID*)&pwb);

Cleanup:
	return retCode;
}


DWORD SdkEapAuthenticatorShutdown(
         IN EAP_METHOD_TYPE* pEapType, 
         OUT EAP_ERROR** ppEapError
         )
{
	DWORD retCode = NO_ERROR;

	//Sanity Check
	if( (!pEapType) || (!ppEapError))
	{
		EapTrace("EapAuthenticatorShutdown() --- One/Some of the paramters is/are NULL");
		retCode = ERROR_INVALID_PARAMETER;
		// Need to fill the EapError
		goto Cleanup;
	}

	//
	// Verify if pEapType passed by EapHost correctly matches the EapType of this DLL.
	//
	if ((pEapType->eapType.type != EAPTYPE) ||
	     (pEapType->dwAuthorId != AUTHOR_ID))
	{
		EapTrace("EapAuthenticatorShutdown() --- Input Eap Method Type Info does not match the supported Eap Method Type");
		retCode = ERROR_NOT_SUPPORTED;
		// Need to fill the EapError.
		goto Cleanup;
	}

	//
	// Clear up any resources as this dll will get unloaded.
	//

Cleanup:
	return retCode;
}


//
// Functions exposed for Configuring EAP Authentication Method.
//

DWORD WINAPI EapMethodAuthenticatorInvokeConfigUI(
               IN EAP_METHOD_TYPE* pEapMethodType,
               IN HWND hwndParent,
               IN DWORD dwFlags,
               IN LPCWSTR pwszMachineName,
               IN DWORD dwSizeOfConfigIn,
               IN BYTE* pConfigIn,
               OUT DWORD* pdwSizeOfConfigOut,
               OUT BYTE** ppConfigOut,
               OUT EAP_ERROR** ppEapError
               )
{
	DWORD retCode = ERROR_SUCCESS;

	//Sanity Check
	if( (!pEapMethodType) || (!ppEapError) || (!pwszMachineName))
	{
		EapTrace("EapMethodAuthenticatorInvokeConfigUI() --- One/Some of the paramters is/are NULL");
		retCode = ERROR_INVALID_PARAMETER;
		// Need to fill the EapError
		goto Cleanup;
	}

	//
	// Verify if pEapType passed by EapHost correctly matches the EapType of this DLL.
	//
	if ((pEapMethodType->eapType.type != EAPTYPE) ||
	     (pEapMethodType->dwAuthorId != AUTHOR_ID))
	{
		EapTrace("EapMethodAuthenticatorInvokeConfigUI() --- Input Eap Method Type Info does not match the supported Eap Method Type");
		retCode = ERROR_NOT_SUPPORTED;
		// Need to fill the EapError.
		goto Cleanup;
	}

	UNREFERENCED_PARAMETER(dwFlags);
	UNREFERENCED_PARAMETER(dwSizeOfConfigIn);
	UNREFERENCED_PARAMETER(pConfigIn);

	//
	// This is for Demonstration use Only!!!
	// We raise an Dialog Box in which the Administrator can configure an username and
	// password. This combination of username and password will only be authenticated 
	// by the Sample Eap Method. For any other combination, authentication will fail.
	// If the Administrator does not configure, this Sample Eap Method will fail all the 
	// authentication attempts.
	//
	retCode = GetIdentityAsConfigData(
		hwndParent,
		ppConfigOut,
		pdwSizeOfConfigOut);

Cleanup:
	return retCode;
}


VOID WINAPI EapMethodAuthenticatorFreeMemory(IN BYTE* pData)
{
	FreeMemory((PVOID *)&pData);
}


//
//  Helper Functions
//

/**
  * MakeRequestMessage() helper function: Construct a request message
  *
  * This function builds the EAP-Challenge message(s) sent to the client.
  *
  * @param  pwb          [in]  Pointer to the work buffer.
  *
  * @param bPacketId [in] PacketId to be used in the EapChallenge Method.
  *
  * @param  pSendPacket     [out] Pointer to a EapPacket structure. The
  *                            authentication protocol can use this structure
  *                            to specify a packet to send.
  *
  * @param  pcbSendPacket    [out]  Specifies the size, in bytes, of the buffer
  *                            pointed to by pSendPacket.
  *
  *
  * @return If the function succeeds, the return value is NO_ERROR.
  */
DWORD
MakeRequestMessage(
    IN  EAPCB*           pwb,
    IN BYTE bPacketId,
    OUT EapPacket *pSendPacket,
    OUT DWORD *  pcbSendPacket
)
{
	BYTE *pcbPeerMessage = NULL;
	CHAR *pchPeerMessage = NULL;
	HRESULT hr    = S_OK;
	DWORD retCode = NO_ERROR;
	EapPacket *pEapRequestPacket = NULL;
	DWORD sendPktSize = 0;

	//
	//  Packet Structure
	//  Code - 1 Byte
	//  Id - 1 Byte
	//  Length - 2 Bytes
	//  Data[0] - EAPTYPE
	//  Data[1] - Length of the Challenge Message (X).
	//  Data[2] to Data[X+1] - Challenge Message
	//

	// Sanity checks.
	if (! pwb || ! pSendPacket || !pcbSendPacket)
	{
		retCode = ERROR_INVALID_PARAMETER;
		EapTrace("Error -- one or more input parameters is NULL!");
		goto Cleanup;
	}

	// Calculate the length of the Request Packet.
	sendPktSize = EAP_PACKET_HDR_LEN +                // Header - Code, Id, Length
					1 +                                                          // Data[0] = Type
					STRING_LENGTH_CHALLENGE_MESSAGE; // Challenge String Length includes NULL which counts for Data[1]

	if(sendPktSize > *pcbSendPacket)
	{
		// Packet is less than the required size.
		retCode = ERROR_INSUFFICIENT_BUFFER; //SdkEapMethodAuthenticatorSendPacket should reply with better error in EAP_ERROR
		goto Cleanup;
	}

	// Assign actual size of the packet.
	*pcbSendPacket = sendPktSize;

	pEapRequestPacket = (EapPacket *)(pSendPacket);

	pcbPeerMessage  = pEapRequestPacket->Data + 1;

	// The packet data doesn't need room for a trailing NULL. 
	*pcbPeerMessage = (BYTE)(STRING_LENGTH_CHALLENGE_MESSAGE - 1);

	pchPeerMessage  = (PCHAR)(pcbPeerMessage + 1);

	// This function needs the length parameter to include room for the NULL.
	hr = StringCbCopyA(pchPeerMessage, STRING_LENGTH_CHALLENGE_MESSAGE,
	                   STRING_CHALLENGE_MESSAGE);
	if (FAILED(hr))
	{
		retCode = HRESULT_CODE(hr);
		EapTrace("Error while copying password into response message! (error %d)",
		         retCode);
		goto Cleanup;
	}

	// Set the Request Code
	pEapRequestPacket->Code = EapCodeRequest;

	// Set the request packet identifier. Start with the Id that was give to us
	pEapRequestPacket->Id = bPacketId;
	
	// Set the length
	HostToWireFormat16((WORD)(*pcbSendPacket), pEapRequestPacket->Length);

	// Set the EAP Type Id
	pEapRequestPacket->Data[0] = EAPTYPE;

Cleanup:
	return retCode;
}


/**
  * GetPasswordFromResponse() helper function: Parse a response packet to extract
  * the password.
  *
  * This function handles reading the EAP client's response packet.  It
  * obtains the user's password, to use when authenticating the user.
  *
  *
  * @param  pReceiveBuf  [in]  Pointer to a EapPacket structure that
  *                                          contains a received packet.
  *
  * @param  pszPassword  [out] Pointer to a buffer, into which the password
  *                                           will be copied.  The caller is responsible for
  *                                           allocating & freeing this buffer.
  *
  * @param size [in,out]             size of the password
  *
  * @return If the function succeeds, the return value is NO_ERROR.
  */
DWORD
GetPasswordFromResponse(
    IN const BYTE* const pReceivePacket,
    OUT __out_bcount(size)  CHAR*           pszPassword,
    size_t &size
)
{
	BYTE* pcbPassword = NULL;
	CHAR* pchPassword = NULL;
	WORD  cbPacket    = 0;
	WORD *pBufLength  = NULL;
	EapPacket* pReceiveBuf = NULL;

	// Start off with the error code returned for most errors.
	DWORD retCode = EAP_METHOD_INVALID_PACKET;

	// Sanity checks.
	if (!pszPassword || !pReceivePacket)
	{
		EapTrace("Error -- input password pointer is NULL!");
		retCode = ERROR_INVALID_PARAMETER;
		goto Cleanup;
	}

	pReceiveBuf = (EapPacket *)pReceivePacket;

	pBufLength = (PWORD)(pReceiveBuf->Length);
	WireToHostFormat16(*pBufLength, (PBYTE)&cbPacket);

	//
	// Extract the password
	//
	if (cbPacket < (EAP_PACKET_HDR_LEN + 1 + 1))
	{
		EapTrace("Packet is too short! (packet length is only %d bytes)", cbPacket);
		goto Cleanup;
	}

	pcbPassword = pReceiveBuf->Data + 1;
	pchPassword = (PCHAR)(pcbPassword + 1);

	//
	// Check the validity about the length if password.
	//
	if ( (*pcbPassword >=  PWLEN) || (*pcbPassword ==  0) )
	{
		EapTrace ("Password length received in the packet is > PWLEN (%d)",
		          PWLEN);
		goto Cleanup;
	}

	if (cbPacket < EAP_PACKET_HDR_LEN + 1 + 1 + *pcbPassword)
	{
		EapTrace("Number of characters in password is %d", *pcbPassword);
		EapTrace("Number of bytes in the EAP packet is %d", cbPacket);
		goto Cleanup;
	}

	CopyMemory(pszPassword, pchPassword, *pcbPassword);

	//
	// NULL terminate the password
	//

	pszPassword[ *pcbPassword ] = '\0';
	size = *pcbPassword;

	//
	// No errors were hit, so return success.
	//
	retCode = NO_ERROR;

Cleanup:
	return retCode;
}


/**
  * MakeAuthenticationAttributes() helper function: Build authentication
  * attributes to be sent to the authentication provider.
  *
  * This function wraps the received username & password into EAP
  * authentication attributes.  The authentication provider will process these
  * attributes, determine whether the authentication succeeded or failed.
  *
  *
  * @param  szUserName    [in]  The username received from the EAP client.
  *
  * @param  szPassword    [in]  The password received from the EAP client.
  *
  * @param  pwb           [in]  Pointer to the work buffer.
  *
  *
  * @return If the function succeeds, the return value is NO_ERROR.
  */
DWORD 
MakeAuthenticationAttributes(
    IN __in CHAR *   szUserName,    
    IN __in CHAR *   szPassword,    
    IN EAPCB *  pwb
)
{
	HRESULT hr = S_OK;
	DWORD   retCode = NO_ERROR;
	DWORD   attribCount = 0;
	size_t  buflen = 0;

	// Sanity checks.
	if (!pwb ||!szUserName || !szPassword)
	{
		EapTrace("Error -- one or more input parameters is NULL!");
		retCode = ERROR_INVALID_PARAMETER;
		goto Cleanup;
	}

	if (pwb->pUserAttributes != NULL)
	{
		// Cleanup any existing user attributes.
		retCode = FreeAttributes(&(pwb->pUserAttributes));
		if (retCode != NO_ERROR)
		{
			EapTrace("Error while freeing existing attributes!");
			goto Cleanup;
		}
	}

	// For authentication, we need 2 attributes.
	attribCount = ATTRIBUTE_COUNT_AUTHENTICATION;
	retCode = AllocateAttributes(attribCount, &(pwb->pUserAttributes));
	if (retCode != NO_ERROR)
	{
		// Comments/Tracing
		goto Cleanup;
	}

	//
	// for user name
	//

	hr = StringCbLengthA(szUserName, UNLEN+1, &buflen);
	if (FAILED(hr))
	{
		retCode = HRESULT_CODE(hr);
		EapTrace("Error while calculating username length! (error %d)", retCode);
		goto Cleanup;
	}

	retCode = AddAttribute(pwb->pUserAttributes, eatUserName,
	                                     (DWORD)buflen , szUserName);
	if (retCode != NO_ERROR)
	{
		goto Cleanup;
	}

	//
	// for password
	//

	hr = StringCbLengthA(szPassword, PWLEN+1, &buflen);
	if (FAILED(hr))
	{
		retCode = HRESULT_CODE(hr);
		EapTrace("Error while calculating password length! (error %d)", retCode);
		goto Cleanup;
	}

	retCode = AddAttribute(pwb->pUserAttributes, eatUserPassword,
	                                          (DWORD)buflen , szPassword);
	if (retCode != NO_ERROR)
	{
		goto Cleanup;
	}


Cleanup:
	if (retCode != NO_ERROR && pwb != NULL) 
	{
		FreeAttributes(&(pwb->pUserAttributes)); 
	}

	return retCode;
}



/**
  * MakeMPPEKeyAttributes() helper function: Build MPPE Key attributes.
  *
  * This function constructs MPPE encryption key attributes and saves them
  * into the EAPCB work buffer.
  *
  *
  * @param  pwb           [in]  Pointer to the work buffer.
  *
  *
  * @return If the function succeeds, the return value is NO_ERROR.
  */
DWORD
MakeMPPEKeyAttributes(
    IN EAPCB *  pwb
)
{
	DWORD   retCode         = NO_ERROR;
	DWORD   dwSendPattern = 0;
	DWORD   dwRecvPattern = 0;
	BYTE*   pByte         = NULL;

	EapAttribute keyAttrib = {eatMinimum, 0, 0};

	// Sanity checks.
	if (! pwb)
	{
		retCode = ERROR_INVALID_PARAMETER;
		goto Cleanup;
	}

	// Free any prior MPPE key attributes.
	retCode = FreeAttributes(&(pwb->pMPPEKeyAttributes));
	if (retCode != NO_ERROR)
	{
		goto Cleanup;
	}

	// Set up a scratch buffer to use when constructing keys.
	retCode = AllocateMemory(MPPE_KEY_LENGTH, (PVOID*)&pByte);
	if (retCode != NO_ERROR)
		goto Cleanup;

	//
	// The "keys" used by this Sample EAP provider are simple values XOR'ed
	// with these values.  When implementing a real EAP protocol, a much
	// stronger key generation mechanism should be used.
	//

	dwSendPattern = 0xAB;
	dwRecvPattern = 0xCD;

	//
	// Construct the MPPE Send key.
	//

	retCode = FillMppeKeyAttribute(pwb, pByte, dwSendPattern, MS_MPPE_SEND_KEY, keyAttrib);
	if (retCode != NO_ERROR)
		goto Cleanup;

	//
	// Construct the MPPE Recv key.
	//

	retCode = FillMppeKeyAttribute(pwb, pByte, dwRecvPattern, MS_MPPE_RECV_KEY, keyAttrib);
	if (retCode != NO_ERROR)
		goto Cleanup;
	
Cleanup:
	FreeMemory((PVOID*)&pByte);

	if (NO_ERROR != retCode)
	{
		if (pwb)
			FreeAttributes(&(pwb->pMPPEKeyAttributes));
	}

	return(retCode);
}


/**
  * FillMppeKeyAttribute() helper function: Construct MPPE Key attributes.
  *
  * This function constructs MPPE encryption key attributes and saves them
  * into the EAPCB work buffer.
  */
DWORD FillMppeKeyAttribute(IN EAPCB *pwb, 
								IN BYTE *&bBuffer, 
								IN DWORD pattern, 
								IN BYTE bKeyDirection,
								IN OUT EapAttribute &pAttrib)
{
	DWORD retCode = ERROR_SUCCESS;
	DWORD dwIndex = 0;
	
	CopyMemory(bBuffer, pwb->aszPassword, MPPE_KEY_LENGTH);
	for (dwIndex = 0; dwIndex < MPPE_KEY_LENGTH; dwIndex++)
	{
		bBuffer[dwIndex] ^= pattern;
	}

	retCode = ConstructMppeKeyAttribute(bKeyDirection,
	                      bBuffer, MPPE_KEY_LENGTH, &pAttrib);
	if (retCode != NO_ERROR)
		goto Cleanup;

	// Add the newly-constructed attribute to the list of attribs.
	retCode = AppendAttributeToList(&(pwb->pMPPEKeyAttributes),
	                  pAttrib.eaType, pAttrib.dwLength,
	                  pAttrib.pValue);
	if (retCode != NO_ERROR)
		goto Cleanup;

	// Cleanup.
	retCode = FreeMemory((PVOID*)&(pAttrib.pValue));
	if (retCode != NO_ERROR)
		goto Cleanup;

Cleanup:
	return retCode;
}




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
DWORD
ConstructMppeKeyAttribute(
    IN     BYTE  bKeyDirection,
    IN     PBYTE pMppeKeyData,
    IN     DWORD cbMppeKeyData,
    IN OUT EapAttribute *pAttrib
)
{
	DWORD retCode = NO_ERROR;

	PBYTE pByte = NULL;
	BYTE  cbAttribValue = 0;

	// Sanity check.
	if (! pAttrib)
	{
		retCode = ERROR_INVALID_PARAMETER;
		goto Cleanup;
	}

	//
	// Bytes needed:
	//      4: Vendor-Id
	//      1: Vendor-Type
	//      1: Vendor-Length
	//      2: Salt
	//      1: Key-Length
	//     NN: Key (default = 32 bytes)
	//     pp: Padding (so Key-Length + Key + Padding = multiple of 16 bytes)
	//     -----------------
	//     tt: Total (i.e. key = 32 bytes, padding = 15, overall total = 56)
	//

	//
	// Choose an appropriate key buffer size.
	//
	cbAttribValue = (BYTE)(cbMppeKeyData + 1);  // Add leading "key-length" byte.

	// If key length isn't a multiple of 16, pad it out accordingly.
	if (cbAttribValue % MPPE_KEY_BLOCK_LENGTH)
	cbAttribValue += MPPE_KEY_BLOCK_LENGTH - ( cbAttribValue %
	                               MPPE_KEY_BLOCK_LENGTH );

	// Include room for the vendor-attribute header & the Salt field (above).
	cbAttribValue = cbAttribValue + VENDOR_ATTRIBUTE_HEADER_LENGTH
	              + MPPE_KEY_SALT_LENGTH;

	retCode = AllocateMemory(cbAttribValue, (PVOID*)&(pAttrib->pValue));
	if (retCode != NO_ERROR)
		goto Cleanup;

	pByte = (PBYTE)(pAttrib->pValue);

	//
	// Populate the RADIUS vendor attribute's values.
	//
	HostToWireFormat32(VENDOR_ID_MICROSOFT, pByte); // Vendor-Id
	pByte[4] = bKeyDirection;                       // Vendor-Type (ie, MS-MPPE-Recv-Key)
	pByte[5] = cbAttribValue - sizeof(DWORD);       // Vendor-Length (all except Vendor-Id)
	// pByte[6-7] is the zero-filled salt field
	pByte[8] = (BYTE)cbMppeKeyData;                 // Key-Length

	//
	// Fill in the MPPE key, starting after the vendor attrib header, the
	// Salt field, and the MPPE key length subfield (1 byte).
	//
	CopyMemory(pByte + VENDOR_ATTRIBUTE_HEADER_LENGTH
	 + MPPE_KEY_SALT_LENGTH + 1,
	pMppeKeyData, cbMppeKeyData);

	//
	// Fill in the rest of the RADIUS attribute data.
	//
	pAttrib->eaType  = eatVendorSpecific;
	pAttrib->dwLength = cbAttribValue;

Cleanup:
	if (retCode != NO_ERROR)
		FreeMemory((PVOID*)&(pAttrib->pValue));

	return retCode;
}



DWORD 
VerifyAuthenticationAttributes(
      IN EAPCB *pwb)
{
	DWORD retCode = NO_ERROR;
	USER_DATA_BLOB *pUserBlob = NULL;
	EapAttribute *pEapAttr = NULL;

	//
	// pwb->pUserAttributes contains the UserName and Password obtained from Client.
	// pwb->pConnectionData contains the UserName and Password obtained from Administrator.
	// After checking if the two matches result is to be set in pwb->dwResult.
	//

	if(pwb->pConnectionData)
	{
		// 1. Get the UserName and Password in Char * format.
		pUserBlob = (USER_DATA_BLOB*)pwb->pConnectionData;

		{
			CHAR  aszIdentity[ UNLEN + 1 ];
			CHAR  aszPassword[ PWLEN + 1 ]; 

			// If username field is present, store it.
			WideCharToMultiByte(
						CP_ACP,
						NO_FLAGS,
						pUserBlob->eapUserNamePassword.awszIdentity,
						AUTOMATIC_STRING_LENGTH,
						aszIdentity,
						UNLEN + 1,
						NULL,
						NULL );

			// If password field is present, store it.
			WideCharToMultiByte(
						CP_ACP,
						NO_FLAGS,
						pUserBlob->eapUserNamePassword.awszPassword,
						AUTOMATIC_STRING_LENGTH,
						aszPassword,
						PWLEN + 1,
						NULL,
						NULL );

			// User Name Attribute
			pEapAttr = &(((pwb->pUserAttributes)->pAttribs)[0]);
			if(0 != memcmp(aszIdentity, pEapAttr->pValue, pEapAttr->dwLength))
			{
				pwb->dwResult = AUTH_FAILURE;
				goto Cleanup;
			}

			// Password Attribute
			pEapAttr = &(((pwb->pUserAttributes)->pAttribs)[1]);
			if(0 != memcmp(aszPassword, pEapAttr->pValue, pEapAttr->dwLength))
			{
				pwb->dwResult = AUTH_FAILURE;
				goto Cleanup;
			}

			pwb->dwResult = AUTH_SUCCESS;
		}
	}
	else
		pwb->dwResult = AUTH_FAILURE;


Cleanup:
	return retCode;
}


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
GetIdentityAsConfigData(
    IN  HWND    hwndParent,
    OUT BYTE**  ppUserDataOut,
    OUT DWORD*  pdwSizeOfUserDataOut
)
{
       USER_DATA_BLOB* pEapUserData = NULL;
	DWORD  retCode  = NO_ERROR;

	// Sanity checks.
	if (! ppUserDataOut || ! pdwSizeOfUserDataOut)
	{
		EapTrace("Error -- one or more output pointers is NULL!");
		retCode = ERROR_INVALID_PARAMETER;
		goto Cleanup;
	}
	
	//
	// Allocate memory for OUT parameters
	//

	retCode = AllocateMemory(sizeof(USER_DATA_BLOB), (PVOID*)&pEapUserData);
	if (retCode != NO_ERROR)
	{
		goto Cleanup;
	}

	//
	// Else prompt for username and password
	//

	retCode = GetUsernameAndPassword(hwndParent, pEapUserData);
	if(retCode != ERROR_SUCCESS)
		goto Cleanup;

	pEapUserData->eapTypeId = EAPTYPE;

	//
	// Set the OUT paramters
	//

	*ppUserDataOut = (BYTE*)pEapUserData;
	*pdwSizeOfUserDataOut = sizeof(USER_DATA_BLOB);

	//
	// We mustn't free OUT parameters.
	//

	pEapUserData = NULL;

Cleanup:
	if(retCode != NO_ERROR)
		FreeMemory((PVOID*)&pEapUserData);

	return retCode;
}


//---------------------------------------------------------------------------
//
//    Dialog routines.
//
//---------------------------------------------------------------------------

/**
  * GetUsernameAndPassword() helper function: Display user credentials UI.
  *
  * This function displays an interactive UI (the IDD_CONFIG_IDENTITY_DIALOG dialog box)
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
DWORD
GetUsernameAndPassword(
    IN  HWND                hwndParent,
    IN  USER_DATA_BLOB*    pEapUserData
)
{
	DWORD dwErr  = NO_ERROR;
	int   result = 0;

	result = (int)DialogBoxParam(
	                 g_hInstance,
	                 MAKEINTRESOURCE(IDD_CONFIG_IDENTITY_DIALOG),
	                 hwndParent,
	                 UsernameDialogProc,
	                 (LPARAM)pEapUserData);

	if (result < 0)
	{
		dwErr = GetLastError();
		EapTrace("Hit error %d while displaying Username/Password dialog!",
		         dwErr);
	}

	return(dwErr);
}


/**
  * This function handles Windows messages sent to the username/password UI
  * dialog box.  It is called by the Windows UI subsystem.
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
INT_PTR CALLBACK
UsernameDialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
)
{
	DWORD dwErr = FALSE;        // By default, return FALSE.
	USER_DATA_BLOB*    pEapUserData = NULL;

	switch (unMsg)
	{
		case WM_INITDIALOG:

			dwErr = InitUsernameDialog(hWnd, lParam);
		break;

		case WM_COMMAND:

			pEapUserData = (USER_DATA_BLOB*)((LONG_PTR)GetWindowLongPtr(hWnd, DWLP_USER));
			dwErr = UsernameCommand(pEapUserData, LOWORD(wParam), hWnd);
		break;
	}

	return(dwErr);
}


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

BOOL
InitUsernameDialog(
    IN  HWND    hWnd,
    IN  LPARAM  lParam
)
{
	SetWindowLongPtr(hWnd, DWLP_USER, (LONG)lParam);

	return(FALSE);
}


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
BOOL
UsernameCommand(
    IN  USER_DATA_BLOB*    pEapUserData,
    IN  WORD                wId,
    IN  HWND                hWndDlg
)
{
	BOOL fOk  = FALSE;
	HWND hWnd = NULL;
	DWORD editFieldLen = 0;

	// Sanity checks.
	if (! pEapUserData)
	{
		EapTrace("Error -- input dialog pointer is NULL!");
		goto LDone;
	}

	switch(wId)
	{
		case IDOK:

			//
			// Save whatever the user typed in as the user name
			//

			hWnd = GetDlgItem(hWndDlg, IDC_EDIT_NAME);
			if (hWnd == NULL)              // GetDlgItem() returns NULL for errors.
			{
				EapTrace("Error -- couldn't get username value! (error %d)",
				         GetLastError());
				goto LDone;
			}

			editFieldLen = GetWindowTextLength(hWnd);
			if(editFieldLen == 0)
			{
				MessageBox(NULL, L"UserName Field Cannot be Empty", L"MisConfiguration...", MB_OK);
				goto LDone;
			}

			GetWindowText(hWnd, pEapUserData->eapUserNamePassword.awszIdentity, UNLEN + 1);

			//
			// Save whatever the user typed in as the password
			//

			hWnd = GetDlgItem(hWndDlg, IDC_EDIT_PASSWD);
			if (hWnd == NULL)              // GetDlgItem() returns NULL for errors.
			{
				EapTrace("Error -- couldn't get password value! (error %d)",
				         GetLastError());
				goto LDone;
			}

			editFieldLen = GetWindowTextLength(hWnd);
			if(editFieldLen == 0)
			{
				MessageBox(NULL, L"Password Field Cannot be Empty", L"MisConfiguration...", MB_OK);
				goto LDone;
			}

			GetWindowText(hWnd, pEapUserData->eapUserNamePassword.awszPassword, PWLEN + 1);

			// Fall through

		case IDCANCEL:

			EndDialog(hWndDlg, wId);
			fOk = TRUE;

		break;
	}

LDone:
	if(fOk == FALSE)
	{
		// Case where UserName id populated but password is left blank.
		// When password is blank, that is an error condition.
		if(pEapUserData)
			ZeroMemory(pEapUserData, sizeof(USER_DATA_BLOB));
	}
	return fOk;
}