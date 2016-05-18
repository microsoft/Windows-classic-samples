// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "SampleEAPMethodClient.h"
#include "memory.h"
#include "SdkCommon.h"
#include "Resource.h"
#include "strsafe.h"
#include "EapHostError.h"

#define CONNECTION_PROPERTIES    1
#define USER_PROPERTIES                 2
using namespace SDK_METHOD_SAMPLE_COMMON;

extern HINSTANCE  g_hInstance;

VOID WINAPI EapPeerFreeErrorMemory(IN EAP_ERROR* pEapError)
{
        //Sanity Check
        if(!pEapError)
        {
                // Nothing to do; exit cleanly.
                EapTrace("EapPeerFreeErrorMemory() --- Input Parameter is NULL, exiting.");
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

DWORD WINAPI EapPeerGetInfo(
         IN EAP_TYPE* pEapType, 
         OUT EAP_PEER_METHOD_ROUTINES* pEapInfo, 
         OUT EAP_ERROR** ppEapError
         )
{
        DWORD retCode = NO_ERROR;

        //Sanity Check
        if((!pEapType) || (!pEapInfo) || (!ppEapError))
        {
                EapTrace("EapPeerGetInfo() --- One/Some of the paramters is/are NULL");
                retCode = ERROR_INVALID_PARAMETER;
                goto Cleanup;
        }

        //
        // Verify if pEapType passed by EapHost correctly matches the EapType of this DLL.
        //
        if (pEapType->type != EAPTYPE)
        {
                EapTrace("EapPeerGetInfo() --- Input Eap Type Info does not match the supported Eap Type");
                retCode = ERROR_NOT_SUPPORTED;
                goto Cleanup;
        }

        ZeroMemory(pEapInfo, sizeof(EAP_PEER_METHOD_ROUTINES));

        //
        // Fill the function pointers inside pEapInfo structure.
        //
        pEapInfo->dwVersion = VERSION;
        //pEapInfo->pEapType = ;
        pEapInfo->EapPeerInitialize = SdkEapPeerInitialize;
        pEapInfo->EapPeerBeginSession = SdkEapPeerBeginSession;
        
        // A method exports either EapPeerGetIdentity or EapPeerSetCredentials (either one of them).
        // If EapPeerGetIdentity is exported, then EapPeerInvokeIdentityUI should also be implemented.
        pEapInfo->EapPeerGetIdentity = SdkEapPeerGetIdentity;
        // If EapPeerSetCredentials is exported, InvokeUserNameDlg regkey should be set.
        pEapInfo->EapPeerSetCredentials = NULL;
        
        pEapInfo->EapPeerProcessRequestPacket = SdkEapPeerProcessRequestPacket;
        pEapInfo->EapPeerGetResponsePacket = SdkEapPeerGetResponsePacket;
        pEapInfo->EapPeerGetResult = SdkEapPeerGetResult;
        pEapInfo->EapPeerGetUIContext = SdkEapPeerGetUIContext;
        pEapInfo->EapPeerSetUIContext = SdkEapPeerSetUIContext;
        pEapInfo->EapPeerGetResponseAttributes = SdkEapPeerGetResponseAttributes;
        pEapInfo->EapPeerSetResponseAttributes = SdkEapPeerSetResponseAttributes;
        pEapInfo->EapPeerEndSession = SdkEapPeerEndSession;
        pEapInfo->EapPeerShutdown = SdkEapPeerShutdown;

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


DWORD WINAPI SdkEapPeerInitialize(OUT EAP_ERROR** ppEapError)
{
        DWORD retCode = NO_ERROR;

        //Sanity Check
        if( !ppEapError)
        {
                EapTrace("SdkEapPeerInitialize() --- One/Some of the paramters is/are NULL");
                retCode = ERROR_INVALID_PARAMETER;
                goto Cleanup;
        }

        //
        // Initialize any resources which is required as long as this method DLL is loaded.
        //
Cleanup:
        return retCode;
}

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
         )
{
        DWORD retCode = NO_ERROR;
        EAPCB* pwb    = NULL;
        DWORD attribCount = 0;
        EapAttribute *pEapAttrib = NULL;
        USER_DATA_BLOB *pUserBlob = NULL;

        // The user data should contain User Name and Password.
        WCHAR* pPassword = NULL;
        WCHAR* pUserIdentity = NULL;

        //Sanity Check
        if( !pSessionHandle || !ppEapError)
        {
                EapTrace("EapPeerBeginSession() --- One/Some of the paramters is/are NULL");
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
                goto Cleanup;
        }

        //
        // Save information passed in, will be used later
        //
        pwb->pWorkBuffer      = (PVOID)pwb;
        pwb->fFlags                = dwFlags;
        pwb->EapState           = MYSTATE_Initial;
        pwb->hTokenImpersonateUser = hTokenImpersonateUser;
        pwb->dwMaxSendPacketSize = dwMaxSendPacketSize;
        pwb->ClientOK = FALSE;

        //
        // Save the Connection Data.
        //
        if(pConnectionData != NULL)
        {
                pwb->dwSizeofConnectionData = dwSizeofConnectionData;
                retCode = AllocateMemory(dwSizeofConnectionData, (PVOID *)&(pwb->pConnectionData));
                if(retCode != NO_ERROR)
                {
                        goto Cleanup;
                }
                CopyMemory(pwb->pConnectionData, pConnectionData, dwSizeofConnectionData);
        }

        // Save the User Data in 
        if ((NULL != pUserData) && (sizeof(USER_DATA_BLOB) == dwSizeofUserData))
        {
                pUserBlob = (USER_DATA_BLOB*)pUserData;
                pUserIdentity = pUserBlob->eapUserNamePassword.awszIdentity;
                pPassword = pUserBlob->eapUserNamePassword.awszPassword;

                pwb->dwSizeofUserData = dwSizeofUserData;
                retCode = AllocateMemory(dwSizeofUserData, (PVOID *)&(pwb->pUserData));
                if(retCode != NO_ERROR)
                {
                        // allocate memory failed. Need to fill the EapError.
                        goto Cleanup;
                }
                CopyMemory(pwb->pUserData, pUserData, dwSizeofUserData);
        }

        // If username field is present, store it.
        if (pUserIdentity)
        {
                WideCharToMultiByte(
                                        CP_ACP,
                                        NO_FLAGS,
                                        pUserIdentity,
                                        AUTOMATIC_STRING_LENGTH,
                                        pwb->aszIdentity,
                                        UNLEN + 1,
                                        NULL,
                                        NULL );
        }

        // If password field is present, store it.
        if (pPassword)
        {
                WideCharToMultiByte(
                                        CP_ACP,
                                        NO_FLAGS,
                                        pPassword,
                                        AUTOMATIC_STRING_LENGTH,
                                        pwb->aszPassword,
                                        PWLEN + 1,
                                        NULL,
                                        NULL );
        }

        if(pAttributeArray != NULL)
        {
                retCode = AllocateAttributes(pAttributeArray->dwNumberOfAttributes, &(pwb->pEapAttributes));
                if(retCode != NO_ERROR)
                {
                        // AllocateAttributes() reported the error. Need to fill the EapError.
                        goto Cleanup;
                }

                for(attribCount = 0; attribCount < pwb->pEapAttributes->dwNumberOfAttributes; attribCount++)
                {
                        pEapAttrib = &((pAttributeArray->pAttribs)[attribCount]);
                        retCode = AddAttribute(pwb->pEapAttributes, pEapAttrib->eaType, pEapAttrib->dwLength, pEapAttrib->pValue);
                        if(retCode != NO_ERROR)
                        {
                                // AddAttribute() reported the error. Need to fill the EapError.
                                goto Cleanup;
                        }
                }
        }
  
        // Return the Session Handle.
        *pSessionHandle = (VOID *)pwb;

Cleanup:
        if(retCode != ERROR_SUCCESS)
	{
		if(pwb)
		{
			FreeMemory((PVOID *)&(pwb->pConnectionData));
			FreeMemory((PVOID *)&(pwb->pUserData));
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

DWORD WINAPI SdkEapPeerGetIdentity(
        IN    DWORD flags,
        IN    DWORD dwSizeofConnectionData,
        IN    const BYTE* pConnectionData,
        IN    DWORD dwSizeofUserData,
        IN    const BYTE* pUserData,
        IN HANDLE hTokenImpersonateUser,
        OUT BOOL* pfInvokeUI,
        IN OUT DWORD* pdwSizeOfUserDataOut,
        OUT BYTE** ppUserDataOut,
        OUT __out LPWSTR* ppwszIdentity,
        OUT EAP_ERROR** ppEapError
         )
{
        DWORD retCode = NO_ERROR;
        USER_DATA_BLOB* pUserBlob = NULL;
        size_t  lenIdentity = 0;
        WCHAR* pwszIdentity    = NULL;
        HRESULT hr = S_OK;

        if(!pfInvokeUI || !ppwszIdentity || !ppEapError)
        {
                EapTrace("SdkEapPeerGetIdentity() --- One/Some of the paramters is/are NULL");
                retCode = ERROR_INVALID_PARAMETER;
                // Need to fill the EapError
                goto Cleanup;
        }

        UNREFERENCED_PARAMETER(flags);
        UNREFERENCED_PARAMETER(dwSizeofConnectionData);
        UNREFERENCED_PARAMETER(pConnectionData);
	    //
	    // hTokenImpersonateUser can be used for retreiving the user related information.
	    // Sample Eap Method does not use this parameter as it does not require any impersonation.
	    //        
        UNREFERENCED_PARAMETER(hTokenImpersonateUser);

        //
        // If UserData is not present, need to ask EapHost to raise the IdentityUI.
        // Therefore, set pfInvokeUI = true.
        //
        if(dwSizeofUserData == 0)
        {
            *pfInvokeUI = TRUE;
            goto Cleanup;
        }

        //
        // Size of UserData should be equal to sizeof(USER_DATA_BLOB), else incorrect
        // user data.
        //
        if((pUserData == NULL) || (dwSizeofUserData != sizeof(USER_DATA_BLOB)))
        {
                EapTrace("SdkEapPeerGetIdentity() --- Incorrect Input User Blob");
                retCode = ERROR_INVALID_PARAMETER;
                // Need to fill the EapError
                goto Cleanup;
        }

        //
        // Allocate Memory for OUT parameters.
        //
        retCode = AllocateMemory(sizeof(USER_DATA_BLOB), (PVOID*)&pUserBlob);
        if (retCode != NO_ERROR)
        {
                // Need to fill the EapError
                goto Cleanup;
        }

        lenIdentity = (UNLEN+1) * sizeof(WCHAR);
        retCode = AllocateMemory((DWORD)lenIdentity, (PVOID*)&pwszIdentity);
        if (retCode != NO_ERROR)
        {
                // Need to fill the EapError
                goto Cleanup;
        }

        //
        // Assign the output parameters their respective values.
        // 
        CopyMemory(pUserBlob, pUserData, sizeof(USER_DATA_BLOB));
        hr = StringCbCopyW(pwszIdentity,
                           lenIdentity,
                           pUserBlob->eapUserNamePassword.awszIdentity);
        if (FAILED(hr))
        {
                retCode = HRESULT_CODE(hr);
                // Need to fill the EapError
                goto Cleanup;
        }

        //
        // Set the OUT paramters
        //
        *ppUserDataOut = (BYTE *)pUserBlob;
        *pdwSizeOfUserDataOut = sizeof(USER_DATA_BLOB);
        *ppwszIdentity = pwszIdentity;

        //
        // We mustn't free OUT parameters.
        //
        pUserBlob = NULL;
        pwszIdentity = NULL;
        
Cleanup:
        if(retCode != NO_ERROR)
        {
                FreeMemory((PVOID*)&pUserBlob);
                FreeMemory((PVOID*)&pwszIdentity);
        }
        return retCode;
}


DWORD WINAPI SdkEapPeerSetCredentials(
         IN EAP_SESSION_HANDLE sessionHandle,
         IN __in LPWSTR pwszIdentity,
         IN __in LPWSTR pwszPassword,         
         OUT EAP_ERROR** pEapError
         )
{
        DWORD retCode = NO_ERROR;

        UNREFERENCED_PARAMETER(sessionHandle);
        UNREFERENCED_PARAMETER(pwszIdentity);
        UNREFERENCED_PARAMETER(pwszPassword);
        UNREFERENCED_PARAMETER(pEapError);

        return retCode;
}

DWORD WINAPI SdkEapPeerProcessRequestPacket(
         IN EAP_SESSION_HANDLE sessionHandle,
         IN DWORD cbReceivePacket,
         IN EapPacket* pReceivePacket,
         OUT EapPeerMethodOutput* pEapOutput,
         OUT EAP_ERROR** pEapError
         )
{
        DWORD retCode = NO_ERROR;
        EAPCB*  pwb = NULL;
        DWORD   cbPacket = 0;

        if(!sessionHandle || !pReceivePacket || !pEapOutput  || !pEapError)
        {
                EapTrace("EapPeerProcessRequestPacket() --- One/Some of the paramters is/are NULL");
                retCode = ERROR_INVALID_PARAMETER;
                // Need to fill the EapError
                goto Cleanup;
        }

        pwb = (EAPCB*)sessionHandle;
        cbPacket = cbReceivePacket;

        pwb->bRecvPacketId = pReceivePacket->Id;
        
        //
        // Main state machine.
        //
        switch(pwb->EapState)
        {
                case MYSTATE_Initial:

                        EapTrace("Authenticatee state: MYSTATE_Initial");

                        //
                        // The received packet can contain certain Attributes that the method cannot process.
                        // Either those attributes need to be consumed by EapHost (SoH Attribute) or Supplicant.
                        //

                        //
                        // Following is just an example. It creates an EapAttributes (assuming it receives from the 
                        // EapPacket) and set the action = EapPeerMethodResponseActionRespond so that supplicant
                        // on receiving the action can call GetResponseAttributes.
                        //

                        //
                        // Allocate Memory for pwb->pFakeAttribute attribute.
                        //
                        retCode = AllocateMemory(sizeof(EapAttribute), (PVOID *)&(pwb->pFakeAttribute));
                        if(retCode != ERROR_SUCCESS)
                        {
                                EapTrace("SdkEapPeerGetResponseAttributes() --- AllocateMemory (pwb->pFakeAttribute) failed");
                                // Need to fill the EapError
                                goto Cleanup;
                        }

                        //
                        // Populate the pwb->pFakeAttribute attribute.
                        //
                        pwb->pFakeAttribute->eaType = (EapAttributeType)1;
                        pwb->pFakeAttribute->dwLength = 4;
                        retCode = AllocateMemory(pwb->pFakeAttribute->dwLength, (PVOID *)&pwb->pFakeAttribute->pValue);
                        if(retCode != ERROR_SUCCESS)
                        {
                                EapTrace("SdkEapPeerGetResponseAttributes() --- AllocateMemory (pwb->pFakeAttribute->pValue) failed");
                                // Free the earlier allocated memory for pwb->pFakeAttribute.
                                FreeMemory((PVOID *)&(pwb->pFakeAttribute));
                                // Need to fill the EapError
                                goto Cleanup;
                        }
                        {
                                DWORD eapType = EAPTYPE;
                                CopyMemory(pwb->pFakeAttribute->pValue, (BYTE *)&eapType, pwb->pFakeAttribute->dwLength);
                        }

                        //
                        // Set the action that supplicant needs to take.
                        //
                        pEapOutput->action = EapPeerMethodResponseActionRespond;
                        pEapOutput->fAllowNotifications = TRUE; //??
                        
                        break;

                default:
                        //
                        // Peer has to be in MYSTATE_Initial when SdkEapPeerProcessRequestPacket is called. 
                        // Any other state is not acceptable according to the state machine of this Eap Method.
                        //
                        EapTrace("SdkEapPeerProcessRequestPacket --- Peer state: [default] Present State = %d", 
                                        pwb->EapState);
                        // Need to fill the EapError
                        retCode = ERROR_INVALID_STATE;
                break;
        }

Cleanup:
        return retCode;
}
         
DWORD WINAPI SdkEapPeerGetResponsePacket(
         IN EAP_SESSION_HANDLE sessionHandle,
         IN OUT DWORD* pcbSendPacket,
         OUT EapPacket* pSendPacket,
         OUT EAP_ERROR** ppEapError
         )
{
        DWORD retCode = NO_ERROR;
        EAPCB*  pwb = NULL;

        //Sanity Check
        if(!sessionHandle || !pcbSendPacket  || !pSendPacket || !ppEapError)
        {
                EapTrace("EapPeerGetResponsePacket() --- One/Some of the paramters is/are NULL");
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
                case MYSTATE_AfterUserOK:
                        //
                        // Make the EAP-Challenge Response Message.
                        //
                        retCode = MakeResponseMessage(pwb, pcbSendPacket, pSendPacket);
                        if(retCode != NO_ERROR)
                        {
                                // Report Error
                                goto Cleanup;
                        }
                        //
                        // We are done (i.e., no more processing is required on Peer side).
                        // Peer Side just waits for the authentication result after sending this message to the authenticator.
                        // So EapState is changed to MYSTATE_Done.
                        //
                        pwb->EapState = MYSTATE_Done;
                        pwb->ClientOK = TRUE;
                        
                        break;

                default:
                        //
                        // Peer Method has to be in MYSTATE_AfterUserOK state when SdkEapPeerGetResponsePacket
                        // is called. Any other state is not acceptable according to the state machine of this Eap Method.
                        //
                        EapTrace("SdkEapPeerGetResponsePacket --- Peer state: [default] Present State = %d", 
                                        pwb->EapState);
                        // Need to fill the EapError
                        retCode = ERROR_INVALID_STATE;
                        break;
        }

Cleanup:
        return retCode;
}

// This will get called either when a method says that it has completed auth.
// or when the lower layer receives an alternative result.
DWORD WINAPI SdkEapPeerGetResult(
         IN EAP_SESSION_HANDLE sessionHandle,
         IN EapPeerMethodResultReason reason,
         OUT EapPeerMethodResult* pResult, 
         OUT EAP_ERROR** pEapError         
         )
{
        DWORD retCode = NO_ERROR;
        EAPCB*  pwb = NULL;

        //Sanity Check
        if(!sessionHandle || !pResult || !pEapError)
        {
                EapTrace("EapPeerGetInfo() --- One/Some of the paramters is/are NULL");
                retCode = ERROR_INVALID_PARAMETER;
                // Need to fill the EapError
                goto Cleanup;
        }

        pwb = (EAPCB*)sessionHandle;

        //
        // Copy the data store in Working Buffer to the output parameter --- pResult
        //

        //
        // The final result decision should be based on both the result it obtains from authenticator
        // as well as its internal state machine.
        // 
        if(reason == EapPeerMethodResultSuccess)
        {
                if(pwb->ClientOK == TRUE)
                        pResult->fIsSuccess = TRUE;
                else
                        pResult->fIsSuccess = FALSE;
        }
        else
                pResult->fIsSuccess = FALSE;

	if(pResult->fIsSuccess == TRUE)
	{
		//
		// Save the Connection Data.
		//
		if(pwb->pConnectionData != NULL)
		{
			pResult->fSaveConnectionData = TRUE;
			pResult->dwSizeofConnectionData = pwb->dwSizeofConnectionData;
			pResult->pConnectionData = pwb->pConnectionData;
		}

		//
		// Save the User Data.
		//
		if(pwb->pUserData != NULL)
		{
			pResult->fSaveUserData = TRUE;
			pResult->dwSizeofUserData = pwb->dwSizeofUserData;
			pResult->pUserData = pwb->pUserData;
		}

		//
		// Fill MPPE Attributes.
		//
		retCode = MakeMPPEKeyAttributes(pwb);
		if(retCode != NO_ERROR)
		{
			// Need to fill the EapError.
			goto Cleanup;
		}
		pResult->pAttribArray = pwb->pMPPEKeyAttributes;
	}
	else
	{
		//
		// Fill the EAP_ERROR
		//
		retCode = AllocateandFillEapError(&(pResult->pEapError),
									1,  ///> Exact Cause of Authentication Failure
									0,
									NULL, NULL, NULL,
									NULL, NULL 
									);
		if(retCode != ERROR_SUCCESS)
		{
			//Report Error
		}

	}

Cleanup:
        if(retCode != NO_ERROR)
        {
                FreeMemory((PVOID*)&(pResult->pConnectionData));
                FreeMemory((PVOID*)&(pResult->pUserData));
        }
        return retCode;
}

DWORD WINAPI SdkEapPeerGetUIContext(
         IN EAP_SESSION_HANDLE sessionHandle,
         OUT DWORD* dwSizeOfUIContextData,         
         OUT BYTE** pUIContextData,
         OUT EAP_ERROR** ppEapError
      )
{
        DWORD retCode = NO_ERROR;
        EAPCB*  pwb = NULL;

        //Sanity Check
        if(!sessionHandle || !dwSizeOfUIContextData || !pUIContextData || !ppEapError)
        {
                EapTrace("EapPeerGetUIContext() --- One/Some of the paramters is/are NULL");
                retCode = ERROR_INVALID_PARAMETER;
                // Need to fill the EapError
                goto Cleanup;
        }

        pwb = (EAPCB*)sessionHandle;

        if(pwb->pUIContext != NULL)
        {
                *dwSizeOfUIContextData = pwb->dwSizeOfUIContext;
                *pUIContextData = pwb->pUIContext;
        }
        else
        {
                *dwSizeOfUIContextData = 0;
        }

Cleanup:
        return retCode;
}

DWORD WINAPI SdkEapPeerSetUIContext(
        IN EAP_SESSION_HANDLE sessionHandle,
        IN DWORD dwSizeOfUIContextData,
        IN const BYTE* pUIContextData,
        OUT EapPeerMethodOutput* pEapOutput,
        OUT EAP_ERROR** ppEapError
     )
{
        DWORD retCode = NO_ERROR;
        EAPCB*  pwb = NULL;

        //Sanity Check
        if(!sessionHandle || !pEapOutput || !pUIContextData || !ppEapError)
        {
                EapTrace("EapPeerSetUIContext() --- One/Some of the paramters is/are NULL");
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
                //
                // if the function is called after InvokeInteractiveUI.
                //
                case MYSTATE_InteractiveUI:
                        // Free the Interactive UI if already stored.
                        retCode = FreeMemory((PVOID*)&(pwb->pDataFromInteractiveUI));
                        if (retCode != NO_ERROR)
                        {
                                goto Cleanup;
                        }
                        //
                        // Look if the answer is "OK".
                        //
                        if(dwSizeOfUIContextData != STRING_LENGTH_UI_SUCCESS)
                        {
                                //Error
                                goto Cleanup;
                        }
                        else
                        {
                                if(memcmp((void *)STRING_UI_SUCCESS, pUIContextData, 
                                        STRING_LENGTH_UI_SUCCESS)  != 0)
                                {
                                        //Error
                                        goto Cleanup;
                                }
                                //
                                // If the user has agreed to use the Sample Eap, then the supplicant should send 
                                // response to the EAP-Challenge (i.e., the password). The peer already has the 
                                // password stored (from InvokeIdentityUI). Everything is ready for sending the 
                                // EAP-Challenge response. Therefore, the action = ResponseActionSend.
                                //
                                pEapOutput->action = EapPeerMethodResponseActionSend;
                                pEapOutput->fAllowNotifications = FALSE; //??
                                //
                                // EapState changes to WaitForRequest signifying user agreed to use Sample Eap.
                                //
                                pwb->EapState = MYSTATE_AfterUserOK;
                        }
                break;

                default:
                        //
                        // Peer Method has to be in MYSTATE_InteractiveUI state when SdkEapPeerSetUIContext
                        // is called. Any other state is not acceptable according to the state machine of this Eap Method.
                        //
                        EapTrace("SdkEapPeerSetUIContext --- Peer state: [default] Present State = %d", 
                                        pwb->EapState);
                        // Need to fill the EapError
                        retCode = ERROR_INVALID_STATE;
                break;
        }
        
Cleanup:
        return retCode;
}

DWORD WINAPI SdkEapPeerGetResponseAttributes(
        IN EAP_SESSION_HANDLE sessionHandle,
        OUT EapAttributes* pAttribs,
        OUT EAP_ERROR** ppEapError         
     )
{
        DWORD retCode = ERROR_SUCCESS;
        EAPCB*  pwb = NULL;

        if(!sessionHandle || !pAttribs || !ppEapError)
        {
                EapTrace("SdkEapPeerGetResponseAttributes() --- One/Some of the paramters is/are NULL");
                retCode = ERROR_INVALID_PARAMETER;
                // Need to fill the EapError
                goto Cleanup;
        }

        pwb = (EAPCB*)sessionHandle;

        //
        // Fill the pAttribs Structure
        //
        pAttribs->dwNumberOfAttributes = 1;
        pAttribs->pAttribs = pwb->pFakeAttribute;

Cleanup:
        return retCode;
}

DWORD WINAPI SdkEapPeerSetResponseAttributes(
         IN EAP_SESSION_HANDLE sessionHandle,
         IN EapAttributes* pAttribs,
         OUT EapPeerMethodOutput* pEapOutput,
         OUT EAP_ERROR** ppEapError
      )
{
        DWORD retCode = NO_ERROR;
        HRESULT hr = S_OK;
        EAPCB*  pwb = NULL;
        DWORD numAttributes = 0;
        DWORD numAttrCtr = 0;
        EapAttribute *pEapAttribute = NULL;

        if(!sessionHandle || !pAttribs || !pEapOutput || !ppEapError)
        {
                EapTrace("SdkEapPeerSetResponseAttributes() --- One/Some of the paramters is/are NULL");
                retCode = ERROR_INVALID_PARAMETER;
                // Need to fill the EapError
                goto Cleanup;
        }

        pwb = (EAPCB*)sessionHandle;

        //
        // We need to process the attributes that is received.
        //
        
        numAttributes = pAttribs->dwNumberOfAttributes;

        for(numAttrCtr = 0; numAttrCtr < numAttributes; numAttrCtr++)
        {
                pEapAttribute = (pAttribs->pAttribs) + numAttrCtr;

                //
                // Except only AttributeType = 2 in response.
                //
                switch(pEapAttribute->eaType)
                {
                        case 2:
                                //
                                // Expected Attribute ---- eaType = 2; dwLength = 4; pValue = 0 or 1 
                                //
                                {
                                        DWORD attrLength = 0;
                                        DWORD attrValue = 0;
                                        attrLength = pEapAttribute->dwLength;
                                        
                                        if(attrLength != sizeof(DWORD))
                                        {
                                                EapTrace("Sdk Peer Method expects length = 4.");
                                        }

                                        attrValue = *((DWORD *)pEapAttribute->pValue);

                                        if(attrValue != 1 && attrValue != 0)
                                        {
                                                EapTrace("Sdk Peer Method expects attrValue to be either 1 or 0");
                                        }
                                }       
                        break;

                        default:
                                EapTrace("Sdk Peer Method does not process attributes except EapAttrType = 2");
                        break;
                }
        }

        //
        // Fill the output
        //

        //
        // Check for EAP_FLAG_SUPRESS_UI flag.
        // If this flag is set, we should not raise an interactive UI and go striaght to 
        // sending the packet that contains user password.
        // This is used in SSO(Single Sign On) scenario where MS supplicant does not 
        // expect any UI to raised.
        //
	if(pwb->fFlags & EAP_FLAG_SUPRESS_UI)
	{
	        pEapOutput->action = EapPeerMethodResponseActionSend;
	        pEapOutput->fAllowNotifications = FALSE;
	        //
	        // EapState changes to MYSTATE_AfterUserOK signifying user agreed to use Sample Eap.
	        //
	        pwb->EapState = MYSTATE_AfterUserOK;
	}
	else
	{
	        //
	        // Bring up interactive UI to notify user that he/she is being
	        // authenticated via the sample EAP
	        //
	        pEapOutput->action = EapPeerMethodResponseActionInvokeUI;
	        pEapOutput->fAllowNotifications = TRUE;

	        if (NULL == pwb->pDataFromInteractiveUI)
	        {
	                pwb->dwSizeOfUIContext = (DWORD)STRING_LENGTH_UI_ALLOW_AUTH;

	                retCode = AllocateMemory(pwb->dwSizeOfUIContext, (PVOID*)&(pwb->pUIContext));
	                if (retCode != NO_ERROR)
	                {
	                        goto Cleanup;
	                }

	                hr = StringCbCopyW((WCHAR*)pwb->pUIContext, pwb->dwSizeOfUIContext, STRING_UI_ALLOW_AUTH);
	                if (FAILED(hr))
	                {
	                        retCode = HRESULT_CODE(hr);
	                        EapTrace("Error while copying UI data to be displayed! (error %d)",retCode);
	                        goto Cleanup;
	                }

	                pwb->EapState = MYSTATE_InteractiveUI;
	        }
	}
	
Cleanup:
        if(retCode != NO_ERROR)
                FreeMemory((PVOID*)&(pwb->pUIContext));
        
        return retCode;
}

DWORD WINAPI SdkEapPeerEndSession(
        IN EAP_SESSION_HANDLE sessionHandle, 
         OUT EAP_ERROR** ppEapError
         )
{
        DWORD  retCode = NO_ERROR;
        EAPCB* pwb = NULL;

        // Sanity check.
        if (!sessionHandle || !ppEapError)
        {
                EapTrace("SdkEapPeerEndSession() --- One/Some of the paramters is/are NULL");
                retCode = ERROR_INVALID_PARAMETER;
                // Need to fill the EapError
                goto Cleanup;
        }

        pwb = (EAPCB *)sessionHandle;

        //
        // Free up the MPPE Key Attributes
        //
        FreeAttributes(&(pwb->pMPPEKeyAttributes));

        //
        // Free the Connection Data.
        //
        FreeMemory((PVOID*)&(pwb->pConnectionData));

        //
        // Free the User Data.
        //
        SecureZeroMemory(pwb->pUserData, pwb->dwSizeofUserData);
        FreeMemory((PVOID*)&(pwb->pUserData));

        //
        // Free the UI Context Data.
        //
        FreeMemory((PVOID*)&(pwb->pUIContext));

        //
        // Free the Input Attributes.
        //
        FreeAttributes(&(pwb->pEapAttributes));

        // Free the Fake Attribute.
        FreeMemory((PVOID *)&(pwb->pFakeAttribute->pValue));
        FreeMemory((PVOID *)&(pwb->pFakeAttribute));

        // Use SecureZeroMemory() here, to ensure that any authentication data is
        // initialized safely, before the memory block is freed back to the system.
        SecureZeroMemory(pwb, sizeof(EAPCB));
        FreeMemory((PVOID*)&pwb);

Cleanup:
        return retCode;
}

DWORD WINAPI SdkEapPeerShutdown(OUT EAP_ERROR** ppEapError)
{
        DWORD retCode = NO_ERROR;

        //Sanity Check
        if( !ppEapError)
        {
                EapTrace("SdkEapPeerShutdown() --- One/Some of the paramters is/are NULL");
                retCode = ERROR_INVALID_PARAMETER;
                // Need to fill the EapError
                goto Cleanup;
        }
        
        //
        // Clear up any resources as this dll will get unloaded.
        //

Cleanup:
        return retCode;
}


//
// Functions exposed for Configuring EAP Peer Method.
//

DWORD WINAPI EapPeerInvokeConfigUI(
         IN EAP_METHOD_TYPE* pEapType,
         IN HWND hwndParent,
         IN DWORD dwFlags,
         IN DWORD dwSizeOfConnectionDataIn,
         IN BYTE* pConnectionDataIn,
         OUT DWORD* dwSizeOfConnectionDataOut,
         OUT BYTE** ppConnectionDataOut,
         OUT EAP_ERROR** pEapError
         )
{
        DWORD retCode = NO_ERROR;

        UNREFERENCED_PARAMETER(pEapType);
        if(!pEapType || !dwSizeOfConnectionDataOut || !ppConnectionDataOut || !pEapError)
        {
                EapTrace("EapPeerInvokeConfigUI() --- One/Some of the paramters is/are NULL");
                retCode = ERROR_INVALID_PARAMETER;
                // Need to fill the EapError
                goto Cleanup;
        }

        if ((pEapType->eapType.type != EAPTYPE) ||  
             (pEapType->dwAuthorId != AUTHOR_ID))
        {
                EapTrace("EapPeerInvokeConfigUI() --- Input Eap Type Info does not match the supported Eap Type");
                retCode = ERROR_NOT_SUPPORTED;
                // Need to fill the EapError.
                goto Cleanup;
        }

        UNREFERENCED_PARAMETER(dwFlags);

        retCode = GetConfigData(hwndParent,
                                              pConnectionDataIn,
                                              dwSizeOfConnectionDataIn,
                                              ppConnectionDataOut,
                                              dwSizeOfConnectionDataOut);
        if(retCode != ERROR_SUCCESS)
        {
                EapTrace("EapPeerInvokeConfigUI() --- GetConfigData returned with retCode = %d", retCode);
                // Need to fill the EapError.
                goto Cleanup;
        }                    

Cleanup:
        return retCode;
}


DWORD WINAPI EapPeerConfigXml2Blob(
                    IN DWORD dwFlags,
                    IN EAP_METHOD_TYPE eapMethodType,
                    IN IXMLDOMDocument2* pConfigDoc,
                    OUT __out_ecount(*pdwSizeOfConfigOut) BYTE** ppConfigOut,
                    OUT DWORD* pdwSizeOfConfigOut,
                    OUT EAP_ERROR** ppEapError
                    )
{
        DWORD retCode = NO_ERROR;
        BSTR elementValue = NULL;
        CONN_DATA_BLOB*  pEapConfigData  = NULL;

        if(!ppConfigOut || !pdwSizeOfConfigOut || !pConfigDoc || !ppEapError)
        {
                EapTrace("EapPeerConfigXml2Blob() --- One/Some of the paramters is/are NULL");
                retCode = ERROR_INVALID_PARAMETER;
                // Need to fill the EapError, if ppEapError != NULL else just return
                goto Cleanup;
        }

        if ((eapMethodType.eapType.type != EAPTYPE) ||  
             (eapMethodType.dwAuthorId != AUTHOR_ID))
        {
                EapTrace("EapPeerConfigXml2Blob() --- Input Eap Type Info does not match the supported Eap Type");
                retCode = ERROR_NOT_SUPPORTED;
                // Need to fill the EapError.
                goto Cleanup;
        }

        UNREFERENCED_PARAMETER(dwFlags);

        //
        // Allocate memory for OUT parameters
        //
        retCode = AllocateMemory(sizeof(CONN_DATA_BLOB), (PVOID*)&pEapConfigData);
        if (retCode != NO_ERROR)
        {
                goto Cleanup;
        }

        //
        // Process the pConfigDoc to get the Connection BLOB.
        //
        //
        // Sample Eap Method expects --- 
        // <EapType>40</EapType><ConfigData>MethodSpecificConfigData</ConfigData>
        // Therefore, reads the two elements -- EapType and ConfigData
        //

        // Read the EapType element from XML
        retCode = GetXmlElementValue(pConfigDoc, L"EapType", CONNECTION_PROPERTIES, elementValue);
        if(retCode != ERROR_SUCCESS)
        {
                //Need to fill EapError
                goto Cleanup;
        }
        
        // Set the EapTypeId of CONN_DATA_BLOB
        pEapConfigData->eapTypeId = _wtol((WCHAR*)elementValue);
        SysFreeString(elementValue);

        // Read the ConfigData element from XML
        retCode = GetXmlElementValue(pConfigDoc, L"ConfigData", CONNECTION_PROPERTIES, elementValue);
        if(retCode != ERROR_SUCCESS)
        {
                //Need to fill EapError
                goto Cleanup;
        }
        
        // Set the awszData of CONN_DATA_BLOB
        CopyMemory(pEapConfigData->awszData, (BYTE *)elementValue, 
                                         ((SysStringLen(elementValue) + 1) * sizeof(wchar_t)));
        SysFreeString(elementValue);

        //
        // Set the output parameters.
        // 
        *ppConfigOut = (BYTE *)pEapConfigData;
        *pdwSizeOfConfigOut = sizeof(CONN_DATA_BLOB);

        pEapConfigData = NULL;

Cleanup:
        if(retCode != ERROR_SUCCESS)
                FreeMemory((PVOID*)&pEapConfigData);

        return retCode;
}



DWORD WINAPI EapPeerCredentialsXml2Blob(
                    IN DWORD dwFlags,
                    IN EAP_METHOD_TYPE eapMethodType,
                    IN IXMLDOMDocument2* pCredentialsDoc,
                    IN __in_ecount(dwSizeOfConfigIn) const BYTE* pConfigIn,
                    IN DWORD dwSizeOfConfigIn,
                    OUT __out_ecount(*pdwSizeOfCredentialsOut) BYTE** ppCredentialsOut,
                    OUT DWORD* pdwSizeOfCredentialsOut,
                    OUT EAP_ERROR** ppEapError
                    )
{
        DWORD retCode = NO_ERROR;
        BSTR elementValue = NULL;
        USER_DATA_BLOB*  pEapUserData  = NULL;

        if(!ppCredentialsOut || !pdwSizeOfCredentialsOut || !pCredentialsDoc || !ppEapError)
        {
                EapTrace("EapPeerCredentialsXml2Blob() --- One/Some of the paramters is/are NULL");
                retCode = ERROR_INVALID_PARAMETER;
                // Need to fill the EapError, if ppEapError != NULL else just return
                goto Cleanup;
        }

        if ((eapMethodType.eapType.type != EAPTYPE) ||  
             (eapMethodType.dwAuthorId != AUTHOR_ID))
        {
                EapTrace("EapPeerCredentialsXml2Blob() --- Input Eap Type Info does not match the supported Eap Type");
                retCode = ERROR_NOT_SUPPORTED;
                // Need to fill the EapError.
                goto Cleanup;
        }

        UNREFERENCED_PARAMETER(dwFlags);
        UNREFERENCED_PARAMETER(pConfigIn);
        UNREFERENCED_PARAMETER(dwSizeOfConfigIn);

        //
        // Allocate memory for OUT parameters
        //
        retCode = AllocateMemory(sizeof(USER_DATA_BLOB), (PVOID*)&pEapUserData);
        if (retCode != NO_ERROR)
        {
                goto Cleanup;
        }

        //
        // Process the pCredentialsDoc to get the User BLOB.
        //
        // Sample Eap Method expects --- 
        // <EapType>40</EapType><Identity>IdentityInformation</Identity><Password>PasswordInformation</Password>
        // Therefore, reads the three elements -- EapType, Identity and Password
        //

        // Read the EapType element from XML
        retCode = GetXmlElementValue(pCredentialsDoc, L"EapType", USER_PROPERTIES, elementValue);
        if(retCode != ERROR_SUCCESS)
        {
                //Need to fill EapError
                goto Cleanup;
        }
        
        // Set the EapTypeId of USER_DATA_BLOB
        pEapUserData->eapTypeId = _wtol((WCHAR*)elementValue);
        SysFreeString(elementValue);

        // Read the Identity element from XML
        retCode = GetXmlElementValue(pCredentialsDoc, L"Identity", USER_PROPERTIES, elementValue);
        if(retCode != ERROR_SUCCESS)
        {
                //Need to fill EapError
                goto Cleanup;
        }
        
        // Set the awszIdentity of USER_DATA_BLOB
        CopyMemory(pEapUserData->eapUserNamePassword.awszIdentity, 
                              (BYTE *)elementValue, 
                              ((SysStringLen(elementValue) + 1) * sizeof(wchar_t)));
        SysFreeString(elementValue);

        // Read the Password element from XML
        retCode = GetXmlElementValue(pCredentialsDoc, L"Password", USER_PROPERTIES, elementValue);
        if(retCode != ERROR_SUCCESS)
        {
                //Need to fill EapError
                goto Cleanup;
        }
        
        // Set the awszPassword of USER_DATA_BLOB
        CopyMemory(pEapUserData->eapUserNamePassword.awszPassword, 
                               (BYTE *)elementValue, 
                              ((SysStringLen(elementValue) + 1) * sizeof(wchar_t)));
        SysFreeString(elementValue);


        //
        // Set the output parameters.
        // 
        *ppCredentialsOut = (BYTE *)pEapUserData;
        *pdwSizeOfCredentialsOut = sizeof(USER_DATA_BLOB);

        pEapUserData = NULL;

Cleanup:
        if(retCode != ERROR_SUCCESS)
                FreeMemory((PVOID*)&pEapUserData);
        
        return retCode;
}



DWORD WINAPI EapPeerConfigBlob2Xml(
                 IN DWORD dwFlags,
                 IN EAP_METHOD_TYPE eapMethodType,
                 IN __in_ecount(dwSizeOfConfigIn) const BYTE* pConfigIn,
                 IN DWORD dwSizeOfConfigIn,
                 OUT IXMLDOMDocument2** ppConfigDoc,
                 OUT EAP_ERROR** ppEapError
                 )
{
       DWORD retCode = ERROR_SUCCESS;
       IXMLDOMDocument2 *pXmlDoc = NULL;

       // pConfigIn can be NULL. If it is, use the default configuration of the method.
       if(!ppConfigDoc || !ppEapError)
       {
                EapTrace("EapPeerConfigBlob2Xml() --- One/Some of the paramters is/are NULL");
                retCode = ERROR_INVALID_PARAMETER;
                // Need to fill the EapError, if ppEapError != NULL else just return
                goto Cleanup;
       }

        if ((eapMethodType.eapType.type != EAPTYPE) ||  
             (eapMethodType.dwAuthorId != AUTHOR_ID))
        {
                EapTrace("EapPeerConfigBlob2Xml() --- Input Eap Type Info does not match the supported Eap Type");
                retCode = ERROR_NOT_SUPPORTED;
                // Need to fill the EapError.
                goto Cleanup;
        }

        UNREFERENCED_PARAMETER(dwFlags);
        UNREFERENCED_PARAMETER(pConfigIn);
        UNREFERENCED_PARAMETER(dwSizeOfConfigIn);

        HRESULT hr = CoCreateInstance(CLSID_DOMDocument60,
                                      NULL,
                                      CLSCTX_INPROC_SERVER,
                                      IID_IXMLDOMDocument2,
                                      reinterpret_cast<void**>(&pXmlDoc)
                                      );
        if(FAILED(hr))
        {
			  retCode = HRESULT_CODE(hr);
              EapTrace("EapPeerConfigBlob2Xml() --- Unable to CoCreate XMLDOMDocument2: retCode = %d", retCode);
              // Need to fill EapError
              goto Cleanup;
        }

        hr = pXmlDoc->put_async(VARIANT_FALSE);
        if (FAILED(hr))
        {
			 retCode = HRESULT_CODE(hr);		
             EapTrace("EapPeerConfigBlob2Xml() --- put_async failed: retCode = %d", retCode);
             // Need to fill EapError
             goto Cleanup;
        }

        //
        // One is expected to read the pConfigIn and then decide what should be the content of XML Doc.
        // For simplicity, we have hardcoded the XML Doc that is sent in response.
        //
        BSTR xml = L"<ConfigBlob xmlns='http://www.microsoft.com/provisioning/EapHostConfig'>2800000061006E0075006A006B000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000</ConfigBlob>";
        VARIANT_BOOL isSuccess = VARIANT_FALSE;
        hr = pXmlDoc->loadXML(xml, &isSuccess);
        if (FAILED(hr) || (isSuccess != VARIANT_TRUE))
        {
			 retCode = HRESULT_CODE(hr);
             EapTrace("EapPeerConfigBlob2Xml() --- loadXML failed: retCode = %d", retCode);
             // Need to fill EapError
             goto Cleanup;
        }

        //
        // Set the output parameters.
        // 
        *ppConfigDoc = pXmlDoc;

		pXmlDoc = NULL;

Cleanup:

        if(pXmlDoc)
            pXmlDoc->Release();

        return retCode;
}


DWORD WINAPI EapPeerInvokeInteractiveUI(
         IN EAP_METHOD_TYPE* pEapType,
         IN HWND hwndParent,
         IN DWORD dwSizeofUIContextData,
         IN BYTE* pUIContextData,
         OUT DWORD* pdwSizeOfDataFromInteractiveUI,
         OUT BYTE** ppDataFromInteractiveUI,
         OUT EAP_ERROR** ppEapError
         )
{
        DWORD retCode = NO_ERROR;
        HRESULT hr = S_OK;

        if (! pUIContextData || !ppDataFromInteractiveUI || !pdwSizeOfDataFromInteractiveUI || !ppEapError)
        {
                EapTrace("EapPeerInvokeInteractiveUI() --- One/Some of the paramters is/are NULL");
                retCode = ERROR_INVALID_PARAMETER;
                // Need to fill the EapError
               goto Cleanup;
        }

        if ((pEapType->eapType.type != EAPTYPE) ||  
             (pEapType->dwAuthorId != AUTHOR_ID))
        {
                EapTrace("EapPeerInvokeInteractiveUI() --- Input Eap Type Info does not match the supported Eap Type");
                retCode = ERROR_NOT_SUPPORTED;
                // Need to fill the EapError.
                goto Cleanup;
        }

        UNREFERENCED_PARAMETER(dwSizeofUIContextData);
        UNREFERENCED_PARAMETER(hwndParent);

        if (MessageBox(hwndParent, (WCHAR*)pUIContextData, 
                                 STRING_INTERACTIVE_UI_TITLE, 
                                 MB_OKCANCEL) == IDOK)
        {
                //
                // If the user presses OK on the Message Box, then he/she agrees to use the EAP Sample.
                //
                //
                // Populate the output parameters -- pdwSizeOfDataFromInteractiveUI & ppDataFromInteractiveUI
                //
                *pdwSizeOfDataFromInteractiveUI = (DWORD)STRING_LENGTH_UI_SUCCESS;

                retCode = AllocateMemory(*pdwSizeOfDataFromInteractiveUI,
                               (PVOID*)ppDataFromInteractiveUI);
                if (retCode != NO_ERROR)
                {
                        goto Cleanup;
                }

                //
                // Safely copy the UI data into our output buffer.
                //
                hr = StringCbCopyW((WCHAR*)*ppDataFromInteractiveUI,
                           *pdwSizeOfDataFromInteractiveUI,
                           STRING_UI_SUCCESS);
                if (FAILED(hr))
                {
                        retCode = HRESULT_CODE(hr);
                        EapTrace("Error while copying UI data! (error %d)",retCode);
                        goto Cleanup;
                }
        }
        else
        {
                *ppDataFromInteractiveUI         = NULL;
                *pdwSizeOfDataFromInteractiveUI = 0;
        }

Cleanup:
        if (retCode != NO_ERROR && ppDataFromInteractiveUI != NULL)
        {
                FreeMemory((PVOID*)ppDataFromInteractiveUI);

                if (pdwSizeOfDataFromInteractiveUI)
                        *pdwSizeOfDataFromInteractiveUI = 0;
        }

        return retCode;
}

DWORD WINAPI EapPeerInvokeIdentityUI(
         IN EAP_METHOD_TYPE* pEapType,
         IN DWORD dwFlags,
         IN HWND hwndParent,
         IN DWORD dwSizeOfConnectionData,
         IN const BYTE* pConnectionData,
         IN DWORD dwSizeOfUserData,
         IN const BYTE* pUserData,
         OUT DWORD* pdwSizeOfUserDataOut,
         OUT BYTE** ppUserDataOut,
         OUT __out LPWSTR* ppwszIdentity,
         OUT EAP_ERROR** pEapError
         )
{
        DWORD retCode = NO_ERROR;

        if(!pEapType || !pdwSizeOfUserDataOut || !ppUserDataOut || !ppwszIdentity || !pEapError)
        {
                EapTrace("EapPeerInvokeIdentityUI() --- One/Some of the paramters is/are NULL");
                retCode = ERROR_INVALID_PARAMETER;
                // Need to fill the EapError
                goto Cleanup;
        }

        if ((pEapType->eapType.type != EAPTYPE) ||  
             (pEapType->dwAuthorId != AUTHOR_ID))
        {
                EapTrace("EapPeerInvokeIdentityUI() --- Input Eap Type Info does not match the supported Eap Type");
                retCode = ERROR_NOT_SUPPORTED;
                // Need to fill the EapError.
                goto Cleanup;
        }

        UNREFERENCED_PARAMETER(dwFlags);
        UNREFERENCED_PARAMETER(dwSizeOfConnectionData);
        UNREFERENCED_PARAMETER(pConnectionData);

        retCode = GetIdentity(
                hwndParent,
                (BYTE *)pUserData,
                dwSizeOfUserData,
                ppUserDataOut,
                pdwSizeOfUserDataOut,
                ppwszIdentity);

Cleanup:

        return retCode;
}


DWORD WINAPI EapPeerQueryCredentialInputFields(
         IN  HANDLE                        hUserToken,
         IN  EAP_METHOD_TYPE               eapType,
         IN  DWORD                         dwFlags,
         IN  DWORD                         dwEapConnDataSize,
         IN  PBYTE                         pbEapConnData,
         OUT EAP_CONFIG_INPUT_FIELD_ARRAY *pEapConfigFieldsArray,
         OUT EAP_ERROR                   **ppEapError
         ) throw()
{
        DWORD retval = NO_ERROR;

        UNREFERENCED_PARAMETER(hUserToken);
        UNREFERENCED_PARAMETER(dwFlags);
        UNREFERENCED_PARAMETER(dwEapConnDataSize);
        UNREFERENCED_PARAMETER(pbEapConnData);

        if (! pEapConfigFieldsArray ||
            ! ppEapError)
        {
                retval = ERROR_INVALID_PARAMETER;
                EapTrace("QueryCredentalInputFields(): Error: Output pointer was NULL!");
                // Need to fill the EapError.
                goto Cleanup;
        }

        //
        // Verify if eapType passed by EapHost correctly matches the EapType of this DLL.
        //
        if ((eapType.eapType.type != EAPTYPE) ||  
             (eapType.dwAuthorId != AUTHOR_ID))
        {
                EapTrace("EapPeerQueryCredentialInputFields() --- Input Eap Type Info does not match the supported Eap Type");
                retval = ERROR_NOT_SUPPORTED;
                // Need to fill the EapError.
                goto Cleanup;
        }

        // Copy this method's default credential input field array, and pass it
        // back to the caller.
        retval = CopyCredentialInputArray(pEapConfigFieldsArray,
                                          &defaultCredentialInputArray);
        if (retval != NO_ERROR)
        {
                EapTrace("QueryCredentalInputFields(): Hit error %d while copying credential field array!",
                         retval);
                // Need to fill the EapError.
                goto Cleanup;
        }


 Cleanup:
        return retval;
}


DWORD WINAPI EapPeerQueryUserBlobFromCredentialInputFields(
         IN  HANDLE                        hUserToken,
         IN  EAP_METHOD_TYPE               eapType,
         IN  DWORD                         dwFlags,
         IN  DWORD                         dwEapConnDataSize,
         IN  PBYTE                         pbEapConnData,
         IN  EAP_CONFIG_INPUT_FIELD_ARRAY *pEapConfigFieldsArray,
         OUT DWORD                        *pdwUserBlobSize,
         OUT PBYTE                        *ppbUserBlob,
         OUT EAP_ERROR                   **ppEapError
         ) throw()
{
        DWORD retval = NO_ERROR;

        UNREFERENCED_PARAMETER(hUserToken);
        UNREFERENCED_PARAMETER(dwFlags);
        UNREFERENCED_PARAMETER(dwEapConnDataSize);
        UNREFERENCED_PARAMETER(pbEapConnData);


        if (! pEapConfigFieldsArray ||
            ! pdwUserBlobSize ||
            ! ppbUserBlob ||
            ! ppEapError)
        {
                retval = ERROR_INVALID_PARAMETER;
                EapTrace("QueryUserBlobFromCredentialInputFields(): Error: One or more input/output pointers were NULL!");
                // Need to fill the EapError.
                goto Cleanup;
        }


        //
        // Verify if eapType passed by EapHost correctly matches the EapType of this DLL.
        //
        if ((eapType.eapType.type != EAPTYPE) ||  
             (eapType.dwAuthorId != AUTHOR_ID))
        {
                retval = ERROR_NOT_SUPPORTED;
                EapTrace("EapPeerQueryUserBlobFromCredentialInputFields() --- Input Eap Type Info does not match the supported Eap Type");
                // Need to fill the EapError.
                goto Cleanup;
        }

        // Generate a User Data blob that corresponds to the credential input field
        // data received.
        retval = ConstructUserBlobFromCredentialInputArray(
                                         &eapType,
                                         pEapConfigFieldsArray,
                                         pdwUserBlobSize,
                                         ppbUserBlob);
        if (retval != NO_ERROR)
        {
                EapTrace("QueryUserBlobFromCredentialInputFields(): Hit error %d while copying credential field array!",
                         retval);
                // Need to fill the EapError.
                goto Cleanup;
        }


 Cleanup:
        return retval;
}


VOID WINAPI EapPeerFreeMemory(
        IN void* pUIContextData
        )
{
        FreeMemory((PVOID*)&pUIContextData);   
}



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
DWORD
MakeResponseMessage(
    IN  EAPCB*           pwb,
    IN  DWORD * pcbSendBuf,
    OUT EapPacket * pSendBuf
)
{
        DWORD retCode = ERROR_SUCCESS;
        BYTE*   pcbPassword  = NULL;
        CHAR*   pszPassword  = NULL;
        size_t  sizePassword = 0;
        HRESULT hr           = S_OK;
        DWORD packetSize = 0;

        if(!pwb || !pcbSendBuf || !pSendBuf)
        {
                EapTrace("MakeResponseMessage() --- One/Some of the paramters is/are NULL");
                retCode = ERROR_INVALID_PARAMETER;
                // Need to fill the EapError
                goto Cleanup;
        }

        //
        //  Packet Structure
        //  Code - 1 Byte
        //  Id - 1 Byte
        //  Length - 2 Bytes
        //  Data[0] - EAPTYPE
        //  Data[1] - Length of Password (X).
        //  Data[2] to Data[X+1] - Password
        //

        //
        // Fill in the password.
        //

        pcbPassword = pSendBuf->Data + 1; 

        // Note: StringCbLength() does not include room for the trailing NULL. This
        // is valid for the packet (which doesn't need the NULL), but calls to any
        // length-based string copy functions will need to add room for the NULL.
        hr = StringCbLengthA(pwb->aszPassword, (size_t)(PWLEN + 1), &sizePassword);
        if (FAILED(hr))
        {
            retCode = HRESULT_CODE(hr);
            EapTrace("Error while calculating password length! (error %d)", retCode);
            goto Cleanup;
        }

        *pcbPassword = (BYTE)sizePassword;

        pszPassword = (PCHAR)(pcbPassword + 1);

        // This function needs the length parameter to include room for the NULL.
        hr = StringCbCopyA(pszPassword, (*pcbPassword)+1, pwb->aszPassword);
        if (FAILED(hr))
        {
                retCode = HRESULT_CODE(hr);
                EapTrace("Error while copying password into response message! (error %d)",
                         retCode);
                goto Cleanup;
        }

        packetSize = EAP_PACKET_HDR_LEN+1+*pcbPassword+1;

        if(packetSize > *pcbSendBuf)
        {
                // Error
                goto Cleanup;
        }
        else
                *pcbSendBuf = packetSize;

        //
        // Set the response code
        //
        pSendBuf->Code = (BYTE)EapCodeResponse;

        //
        // The Reponse packet Id MUST match the Request packet Id.
        //
        pSendBuf->Id = pwb->bRecvPacketId;

        //
        // Set the EAP type ID
        //
        pSendBuf->Data[0] = (BYTE)EAPTYPE;

        //
        // Set the length of the packet
        //
        HostToWireFormat16((WORD)packetSize, pSendBuf->Length);

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
)
{
       USER_DATA_BLOB* pEapUserData = NULL;
        DWORD  retCode  = NO_ERROR;
        size_t  lenIdentity     = 0;
        WCHAR* pwszIdentity    = NULL;
        HRESULT hr           = S_OK;


        // Sanity checks.
        if (! ppUserDataOut || ! pdwSizeOfUserDataOut || !ppwszIdentityOut)
        {
                EapTrace("Error -- one or more output pointers is NULL!");
                retCode = ERROR_INVALID_PARAMETER;
                goto Cleanup;
        }

        UNREFERENCED_PARAMETER(dwSizeOfUserDataIn);
        UNREFERENCED_PARAMETER(hwndParent);
        
        //
        // Allocate memory for OUT parameters
        //

        retCode = AllocateMemory(sizeof(USER_DATA_BLOB), (PVOID*)&pEapUserData);
        if (retCode != NO_ERROR)
        {
                goto Cleanup;
        }

        lenIdentity = (UNLEN+1) * sizeof(WCHAR);
        retCode = AllocateMemory((DWORD)lenIdentity, (PVOID*)&pwszIdentity);
        if (retCode != NO_ERROR)
        {
                goto Cleanup;
        }

        if (NULL != pUserDataIn)
        {
                //
                // Use the saved credentials if they exist
                //

                CopyMemory(pEapUserData, pUserDataIn, sizeof(USER_DATA_BLOB));
        }
        else
        {
                //
                // Else prompt for username and password
                //

                GetUsernameAndPassword(hwndParent, pEapUserData);
        }

        pEapUserData->eapTypeId = EAPTYPE;

       // Safely copy the UI data into our output buffer.
	hr = StringCbCopy(pwszIdentity,
                           lenIdentity,
                           pEapUserData->eapUserNamePassword.awszIdentity);
        if (FAILED(hr))
        {
                retCode = HRESULT_CODE(hr);
                goto Cleanup;
        }

        //
        // Set the OUT paramters
        //

        *ppUserDataOut = (BYTE*)pEapUserData;
        *pdwSizeOfUserDataOut = sizeof(USER_DATA_BLOB);
        *ppwszIdentityOut = pwszIdentity;

        //
        // We mustn't free OUT parameters.
        //

        pEapUserData = NULL;
        pwszIdentity = NULL;

Cleanup:
        if(retCode != NO_ERROR)
        {
                FreeMemory((PVOID*)&pEapUserData);
                FreeMemory((PVOID*)&pwszIdentity);
        }

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
                     MAKEINTRESOURCE(IDD_IDENTITY_DIALOG),
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

        GetWindowText(hWnd, pEapUserData->eapUserNamePassword.awszPassword, PWLEN + 1);

        // Fall through

    case IDCANCEL:

        EndDialog(hWndDlg, wId);
        fOk = TRUE;

        break;
    }

 LDone:
    return fOk;
}


DWORD GetConfigData(
    IN  HWND    hwndParent,
    IN  BYTE*   pConnectionDataIn,
    IN  DWORD   dwSizeOfConnectionDataIn,
    OUT BYTE**  ppConnectionDataOut,
    OUT DWORD*  dwSizeOfConnectionDataOut
)
{
        CONN_DATA_BLOB*  pEapConfigData  = NULL;
        DWORD  retCode  = NO_ERROR;


        // Sanity checks.
        if (! ppConnectionDataOut || ! dwSizeOfConnectionDataOut)
        {
                EapTrace("Error -- one or more output pointers is NULL!");
                retCode = ERROR_INVALID_PARAMETER;
                goto Cleanup;
        }

        UNREFERENCED_PARAMETER(pConnectionDataIn);
        UNREFERENCED_PARAMETER(dwSizeOfConnectionDataIn);
        
        //
        // Allocate memory for OUT parameters
        //

        retCode = AllocateMemory(sizeof(CONN_DATA_BLOB), (PVOID*)&pEapConfigData);
        if (retCode != NO_ERROR)
        {
                goto Cleanup;
        }

        //
        // Prompt for config data from UI. Sets the awszData within CONN_DATA_BLOB
        //
        GetConnectionData(hwndParent, pEapConfigData);

        //
        // Set the EapType within CONN_DATA_BLOB.
        //
        pEapConfigData->eapTypeId = EAPTYPE;

        //
        // Set the OUT paramters
        //
        
        *ppConnectionDataOut = (BYTE*)pEapConfigData;
        *dwSizeOfConnectionDataOut = sizeof(CONN_DATA_BLOB);

        //
        // We mustn't free OUT parameters.
        //

        pEapConfigData = NULL;

Cleanup:
        return retCode;
}

DWORD
GetConnectionData(
    IN  HWND                hwndParent,
    IN  CONN_DATA_BLOB*    pEapConfigData
)
{
        DWORD dwErr  = NO_ERROR;
        int   result = 0;

        result = (int)DialogBoxParam(
                                 g_hInstance,
                                 MAKEINTRESOURCE(IDD_CONFIG_DIALOG),
                                 hwndParent,
                                 ConnectionDialogProc,
                                 (LPARAM)pEapConfigData);
        if (result < 0)
        {
                dwErr = GetLastError();
                EapTrace("Hit error %d while displaying Connection dialog!",
                     dwErr);
        }

        return(dwErr);
}

INT_PTR CALLBACK
ConnectionDialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
)
{
        DWORD dwErr = FALSE;        // By default, return FALSE.
        CONN_DATA_BLOB*    pEapConfigData = NULL;

        switch (unMsg)
        {
                case WM_INITDIALOG:
                        
                dwErr = InitConnectionDialog(hWnd, lParam);
                
                break;

                case WM_COMMAND:
                        
                pEapConfigData = (CONN_DATA_BLOB*)((LONG_PTR)GetWindowLongPtr(hWnd, DWLP_USER));
                dwErr = ConnectionCommand(pEapConfigData, LOWORD(wParam), hWnd);
                
                break;
        }

        return(dwErr);
}

BOOL
InitConnectionDialog(
    IN  HWND    hWnd,
    IN  LPARAM  lParam
)
{
        SetWindowLongPtr(hWnd, DWLP_USER, (LONG)lParam);

        return(FALSE);
}


BOOL
ConnectionCommand(
    IN  CONN_DATA_BLOB*    pEapConfigData,
    IN  WORD                wId,
    IN  HWND                hWndDlg
)
{
        BOOL fOk  = FALSE;
        HWND hWnd = NULL;


        // Sanity checks.
        if (! pEapConfigData)
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

                        hWnd = GetDlgItem(hWndDlg, IDC_EDIT_CONFIG_NAME);
                        if (hWnd == NULL)              // GetDlgItem() returns NULL for errors.
                        {
                                EapTrace("Error -- couldn't get config value! (error %d)",
                                         GetLastError());
                                goto LDone;
                        }

                        GetWindowText(hWnd, pEapConfigData->awszData, PATHLEN + 1);

                        // Fall through

                case IDCANCEL:

                        EndDialog(hWndDlg, wId);
                        fOk = TRUE;

                break;
        }

LDone:
        return fOk;
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

        //
        // Free any prior MPPE key attributes.
        //
        retCode = FreeAttributes(&(pwb->pMPPEKeyAttributes));
        if (retCode != NO_ERROR)
        {
                goto Cleanup;
        }

        //
        // Set up a scratch buffer to use when constructing keys.
        //
        retCode = AllocateMemory(MPPE_KEY_LENGTH, (PVOID*)&pByte);
        if (retCode != NO_ERROR)
                goto Cleanup;

        //
        // The "keys" used by this Sample EAP provider are simple values XOR'ed
        // with these values.  When implementing a real EAP protocol, a much
        // stronger key generation mechanism should be used.
        //

        dwSendPattern = 0xCD;
        dwRecvPattern = 0xAB;
        
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

        //
        // Add the newly-constructed attribute to the list of attribs.
        //
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
    IN    BYTE  bKeyDirection,
    IN    PBYTE pMppeKeyData,
    IN    DWORD cbMppeKeyData,
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
        // Populate the EAP vendor attribute's values.
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
        // Fill in the rest of the EAP attribute data.
        //
        pAttrib->eaType  = eatVendorSpecific;
        pAttrib->dwLength = cbAttribValue;

Cleanup:
        if (retCode != NO_ERROR && pAttrib != NULL)
                FreeMemory((PVOID*)&(pAttrib->pValue));

        return retCode;
}


DWORD GetXmlElementValue(
        IXMLDOMDocument2 *pXmlDoc, 
        __in LPWSTR pElementName, 
        DWORD dwTypeOfDoc,
        BSTR &pElementValue
)
{
        DWORD retCode = ERROR_SUCCESS;
        IXMLDOMNode      *pDOMNode   = NULL;
        HRESULT hr = S_OK;
        WCHAR  *fullNodeName = NULL;
        VARIANT var = {0};

        var.vt = VT_EMPTY;
        if(dwTypeOfDoc == CONNECTION_PROPERTIES)
            var.bstrVal = SysAllocString(L"xmlns:SDKEapClientMethod='http://www.microsoft.com/provisioning/EapHostConfig'");
        else
            var.bstrVal = SysAllocString(L"xmlns:SDKEapClientMethod='http://www.microsoft.com/provisioning/EapHostUserCredentials'");
        if (var.bstrVal == NULL)
        {
             retCode = ERROR_NOT_ENOUGH_MEMORY;
             goto Cleanup;
        }
        var.vt = VT_BSTR;

       hr = pXmlDoc->setProperty((BSTR)L"SelectionNamespaces", var);
       if(FAILED(hr))
       {
            retCode = HRESULT_CODE(hr);
            EapTrace("GetXmlElementValue --- setProperty returned error (for Element = %s) = %d \n", 
                                       pElementName,
                                       retCode);
            goto Cleanup;
       }

        //
        // Get the size of ElementName.
        //
        WCHAR initialNodeName[] = L"//SDKEapClientMethod:";
        DWORD initailNodeNameSize = sizeof(initialNodeName);    // length includes null terminated character

        DWORD elementNameSize = 0;
        hr = StringCbLengthW(pElementName, (size_t)(STRSAFE_MAX_CCH * sizeof(wchar_t)), (size_t *)&elementNameSize);
        if (FAILED(hr))
        {
            retCode = HRESULT_CODE(hr);
            EapTrace("CopyPCWSTR -- StringCbLengthW :  retCode = %d", retCode);
            goto Cleanup;
        }


        DWORD fullNodeNameSize = elementNameSize + initailNodeNameSize;

        retCode = AllocateMemory(fullNodeNameSize, (PVOID*)&fullNodeName);
        if (retCode != NO_ERROR)
        {
                goto Cleanup;
        }

        CopyMemory(fullNodeName, initialNodeName, initailNodeNameSize);
        CopyMemory(((BYTE *)fullNodeName) + (initailNodeNameSize - sizeof(wchar_t)), pElementName, elementNameSize);

       //
       // Selecting the node we are interested in.
       //
       hr = pXmlDoc->selectSingleNode((BSTR)fullNodeName, &pDOMNode);
       if(FAILED(hr))
       {
            retCode = HRESULT_CODE(hr);
            EapTrace("GetXmlElementValue --- Invalid Node returned error (for Element = %s) = %d \n", 
                                       pElementName,
                                       retCode);
            goto Cleanup;
       }

       if ( pDOMNode != NULL )
       {
            // 
            // Get the content of the node as BSTR.
            //
            hr = pDOMNode->get_text(&pElementValue);
            if(FAILED(hr))
            {
                  retCode = HRESULT_CODE(hr);
                  EapTrace("GetXmlElementValue --- get_text returned error (for Element = %s) = %d \n", 
                                             pElementName,
                                             retCode);
                  goto Cleanup;
            } 
       }


Cleanup:
        if(pDOMNode != NULL)
        {
            pDOMNode->Release();
            pDOMNode = NULL;
        }
        if(fullNodeName != NULL)
            FreeMemory((PVOID *)&fullNodeName);
        if(var.bstrVal)
            SysFreeString(var.bstrVal);

        return retCode;
}


/**
  * Allocate a new, independent copy of an input string.
  *
  *
  * @param  pInput     [in] A string to copy.
  *
  *
  * @return A pointer to the new string. If the input string is NULL, or the
  *         memory allocation fails, this returns NULL.
  */

LPWSTR NewString(IN const __in LPWSTR pInput)
{
    DWORD   result = NO_ERROR;
    HRESULT hr     = S_OK;

    LPWSTR  pOutput    = NULL;
    size_t  cbOutput = 0;

    // If NULL, just return.
    if (! pInput)
        goto Cleanup;

    // Get the input string's length.
    hr = StringCbLengthW(pInput, MAX_PATH, &cbOutput);
    if ( FAILED(hr) )
    {
        result = HRESULT_CODE(hr);
        goto Cleanup;
    }

    // Include room for the trailing NULL.
    cbOutput += sizeof(WCHAR);

    // Allocate a new string buffer for it.
    result = AllocateMemory((DWORD)cbOutput, (PVOID*)&pOutput);
    if (result != NO_ERROR)
    {
        goto Cleanup;
    }

    // Copy the string into it.
    hr = StringCbCopyW(pOutput, cbOutput, pInput);
    if ( FAILED(hr) )
    {
        result = HRESULT_CODE(hr);
        goto Cleanup;
    }

 Cleanup:
    if (result != NO_ERROR)
    {
        EapTrace("NewString(): Hit error %d (hr = %#x)!",
                 result, hr);

        FreeMemory((PVOID*)&pOutput);
        pOutput = NULL;
    }

    return pOutput;
}


DWORD CopyCredentialInputArray(
    OUT      EAP_CONFIG_INPUT_FIELD_ARRAY  *pDest,
    IN const EAP_CONFIG_INPUT_FIELD_ARRAY  *pSrc
    ) throw()
{
    DWORD retval = NO_ERROR;
    DWORD i      = 0;

    EAP_CONFIG_INPUT_FIELD_DATA *pWalkerSrc  = NULL;
    EAP_CONFIG_INPUT_FIELD_DATA *pWalkerDest = NULL;


    // The input array pointer is optional; if NULL, ignore it and exit cleanly.
    if (pSrc == NULL)
    {
        goto Cleanup;
    }

    // The output array pointer is required; if NULL, exit with an error.
    if (pDest == NULL)
    {
        retval = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    
    retval = AllocateMemory(pSrc->dwNumberOfFields
                               * sizeof(EAP_CONFIG_INPUT_FIELD_DATA),
                            (PVOID*)&(pDest->pFields));
    if (retval != NO_ERROR)
    {
        goto Cleanup;
    }

    pDest->dwNumberOfFields  = pSrc->dwNumberOfFields;

    // Sanity check.
    if (pSrc->dwNumberOfFields > 0 &&
        pSrc->pFields == NULL)
    {
        retval = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }


    // Copy over the individual array elements.
    pWalkerSrc  = pSrc->pFields;
    pWalkerDest = pDest->pFields;

    for (i = 0; i < pDest->dwNumberOfFields; i++)
    {
        // First, copy members that require allocating new memory.
        pWalkerDest->pwszLabel = NewString(pWalkerSrc->pwszLabel);
        pWalkerDest->pwszData  = NewString(pWalkerSrc->pwszData);

        if (! pWalkerDest->pwszLabel ||
            ! pWalkerDest->pwszData    )
        {
            retval = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        // Next, copy simple members.
        pWalkerDest->dwSize          = sizeof(EAP_CONFIG_INPUT_FIELD_DATA);
        pWalkerDest->Type            = pWalkerSrc->Type;
        pWalkerDest->dwFlagProps     = pWalkerSrc->dwFlagProps;
        pWalkerDest->dwMinDataLength = pWalkerSrc->dwMinDataLength;
        pWalkerDest->dwMaxDataLength = pWalkerSrc->dwMaxDataLength;

        // Finally, move along to the next pair of elements.
        pWalkerDest++;
        pWalkerSrc++;
    }

Cleanup:
    if (retval != NO_ERROR &&
        pDest  != NULL)
    {
        FreeCredentialInputArray(pDest);
    }

    return retval;
}


void FreeCredentialInputArray(IN OUT EAP_CONFIG_INPUT_FIELD_ARRAY *pArray) throw()
{
    DWORD i = 0;
    DWORD numElements = 0;

    EAP_CONFIG_INPUT_FIELD_DATA *pWalker = NULL;

    if (! pArray)
        goto Cleanup;

    // Walk the array, freeing each element's memory allocations.
    numElements = pArray->dwNumberOfFields;

    for (i = 0, pWalker = pArray->pFields;
         i < numElements;
         i++, pWalker++)
    {
        FreeMemory((PVOID*)&(pWalker->pwszLabel));
        FreeMemory((PVOID*)&(pWalker->pwszData));
    }

    // Last, free the array buffer itself. 
    FreeMemory((PVOID*)&(pArray->pFields));

Cleanup:
    return;
}


DWORD ConstructUserBlobFromCredentialInputArray(
    IN const EAP_METHOD_TYPE *pEapTypeIn,
    IN const EAP_CONFIG_INPUT_FIELD_ARRAY *pEapConfigFieldsArray,
    IN OUT   DWORD *pdwUserBlobSize,
    OUT      PBYTE *ppbUserBlob
    ) throw()
{
    DWORD   retval    = NO_ERROR;
    HRESULT hr        = S_OK;
    LPWSTR  pIdentity = NULL;
    LPWSTR  pPassword = NULL;

    DWORD eapTypeId   = 0;

    USER_DATA_BLOB *pBlob = NULL;


    if (! pEapConfigFieldsArray ||
        ! pdwUserBlobSize ||
        ! ppbUserBlob)
    {
        retval = ERROR_INVALID_PARAMETER;
        EapTrace("ConstructUserBlobFromCredentialInputArray(): Error: One or more input/output pointers were NULL!");
        goto Cleanup;
    }


    // First, find the credential input data fields we care about.

    pIdentity = GetCredentialInputValue(pEapConfigFieldsArray,
                                        EapConfigInputUsername);

    pPassword = GetCredentialInputValue(pEapConfigFieldsArray,
                                        EapConfigInputPassword);

    if (! pIdentity ||
        ! pPassword   )
    {
        retval = ERROR_INVALID_DATA;
        goto Cleanup;
    }


    // Next, create the output user blob buffer, and fill in the values.

    retval = AllocateMemory(sizeof(USER_DATA_BLOB), (PVOID*)&pBlob);

    if (retval != NO_ERROR)
        goto Cleanup;


    eapTypeId = EAPTYPE;

    if (pEapTypeIn)
        eapTypeId = pEapTypeIn->eapType.type;

    pBlob->eapTypeId = eapTypeId;


    hr = StringCbCopyW(pBlob->eapUserNamePassword.awszIdentity,
                       (UNLEN + 1),
                       pIdentity);
    if (FAILED(hr))
        goto Cleanup;

    hr = StringCbCopyW(pBlob->eapUserNamePassword.awszPassword,
                       (PWLEN + 1),
                       pPassword);
    if (FAILED(hr))
        goto Cleanup;


    // Finally, pass the blob back to the caller.
    *ppbUserBlob     = (PBYTE)pBlob;
    *pdwUserBlobSize = sizeof(USER_DATA_BLOB);

Cleanup:
    if (FAILED(hr))
        retval = HRESULT_CODE(hr);

    if (retval != NO_ERROR)
    {
        FreeMemory((PVOID*)&pBlob);
        *ppbUserBlob = NULL;
        *pdwUserBlobSize = 0;
    }

    return retval;
}


LPWSTR GetCredentialInputValue(
    IN const EAP_CONFIG_INPUT_FIELD_ARRAY  *pSrc,
    IN const DWORD                          valueType,
    IN const __in LPWSTR                         pValueLabel
    ) throw()
{
    DWORD   result      = ERROR_NOT_FOUND;
    HRESULT hr          = S_OK;
    DWORD   i           = 0;
    DWORD   numElements = 0;
    size_t  sizeLabel   = 0;
    LPWSTR  pData       = NULL;

    EAP_CONFIG_INPUT_FIELD_DATA *pWalker = 0;


    if (pSrc == NULL)
    {
        result = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    // The Label parameter is optional; if specified, get its length (used below).
    if (pValueLabel != NULL)
    {
        hr = StringCbLengthW(pValueLabel, MAX_PATH, &sizeLabel);
        if (FAILED(hr))
        {
            result = HRESULT_CODE(hr);
            goto Cleanup;
        }
    }

    // Walk the array, checking for mismatched data.
    numElements = pSrc->dwNumberOfFields;
    pWalker     = pSrc->pFields;

    for (i = 0, pWalker = pSrc->pFields;
         i < numElements;
         i++, pWalker++)
    {
        if (pWalker->Type != (int)valueType)
            continue;

        if (pValueLabel != NULL &&
            memcmp(pWalker->pwszLabel, pValueLabel, sizeLabel) != 0)
            continue;

        // All filters passed; we've found it!
        pData  = pWalker->pwszData;
        result = NO_ERROR;
        break;
    }

Cleanup:
    return pData;
}
