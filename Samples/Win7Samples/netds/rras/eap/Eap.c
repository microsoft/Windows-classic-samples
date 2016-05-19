/*

Copyright (c) 1997, Microsoft Corporation, all rights reserved

Description:
    Sample Extensible Authentication Protocol. Here is a graphic of the EAP
    sample protocol:

                Authenticator                       Authenticatee
                -------------                       -------------

                                "Send Password"
                            ---------------------->
                                  EAP Request

                                  <password>
                            <----------------------
                                  EAP Response

                            ----------------------->
                                 Success/Failure

History:

*/

#include <windows.h>
#include <winuser.h>
#include <lmcons.h>
#include <string.h>
#include <stdlib.h>
#include <raseapif.h>
#include <raserror.h>
#include <rtutils.h>
#include <stdio.h>
#define SDEBUGGLOBALS
#define RASEAPGLOBALS
#include "eap.h"
#include "resource.h"
#include <strsafe.h>

/*---------------------------------------------------------------------------
    External entry points
---------------------------------------------------------------------------*/
/*

Notes:
    RasEapGetInfo entry point called by the EAP-PPP engine by name.
    
*/

DWORD APIENTRY
RasEapGetInfo(
    IN  DWORD         dwEapTypeId,
    OUT PPP_EAP_INFO* pInfo 
)
{
    EapTrace("RasEapGetInfo");

    if (dwEapTypeId != PPP_EAP_PROTOCOL_ID)
    {
        //
        // We only support PPP_EAP_PROTOCOL_ID eap type
        //

        EapTrace("Type ID %d is not supported", dwEapTypeId);

        return(ERROR_NOT_SUPPORTED);
    }

    ZeroMemory(pInfo, sizeof(PPP_EAP_INFO));

    //
    // Fill in the required information
    //

    pInfo->dwEapTypeId       = PPP_EAP_PROTOCOL_ID;
    pInfo->RasEapBegin       = EapBegin;
    pInfo->RasEapEnd         = EapEnd;
    pInfo->RasEapMakeMessage = EapMakeMessage;

    return(NO_ERROR);
}

/*

Notes:
    EapBegin entry point called by the EAP PPP engine thru the passed address.
    
*/

DWORD APIENTRY
EapBegin(
    OUT VOID** ppWorkBuf,
    IN  VOID*  pInfo 
)
{
    PPP_EAP_INPUT* pInput = (PPP_EAP_INPUT*)pInfo;
    EAPCB*         pwb;

    EapTrace("EapBegin(%ws)", pInput->pwszIdentity);

    //
    // Allocate work buffer.
    //

    if ((pwb = (EAPCB*)LocalAlloc(LPTR, sizeof(EAPCB))) == NULL)
    {
        EapTrace("Not enough memory");
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    //
    // Save information passed in, will be used later
    //

    pwb->fFlags             = pInput->fFlags;
    pwb->fAuthenticator     = pInput->fAuthenticator;
    pwb->EapState           = MYSTATE_Initial;
    pwb->dwInitialPacketId  = pInput->bInitialId;

    if (pInput->pDataFromInteractiveUI != NULL)
    {
        pwb->dwSizeOfDataFromInteractiveUI = 
            pInput->dwSizeOfDataFromInteractiveUI;
        pwb->pDataFromInteractiveUI =
            LocalAlloc(LPTR, pwb->dwSizeOfDataFromInteractiveUI);

        if (NULL != pwb->pDataFromInteractiveUI)
        {
            CopyMemory(pwb->pDataFromInteractiveUI, 
                pInput->pDataFromInteractiveUI,
                pwb->dwSizeOfDataFromInteractiveUI);
        }
    }

    //
    // Save the identity. On the authenticatee side, this is obtained by user
    // input; on the authenticator side this was obtained by the Identity
    // request message.
    //

    WideCharToMultiByte(
        CP_ACP,
        0,
        pInput->pwszIdentity,
        -1,
        pwb->aszIdentity,
        UNLEN + 1,
        NULL,
        NULL );

    //
    // If we are an authenticatee, then use the password passed in
    //

    if (!pwb->fAuthenticator)
    {
        if (   (NULL != pInput->pUserData)
            && (sizeof(EAP_NAME_DIALOG) == pInput->dwSizeOfUserData))
        {
            WideCharToMultiByte(
                CP_ACP,
                0,
                ((EAP_NAME_DIALOG*)(pInput->pUserData))->awszPassword,
                -1,
                pwb->aszPassword,
                PWLEN + 1,
                NULL,
                NULL );
        }
    }

    //
    // Register work buffer with engine.
    //

    *ppWorkBuf = pwb;

    return(NO_ERROR);
}

/*

Notes:
    EapEnd entry point called by the PPP engine thru the passed address.
    See EAP interface documentation.
    
*/

DWORD APIENTRY
EapEnd(
    IN VOID* pWorkBuf 
)
{
    EAPCB* pwb = (EAPCB *)pWorkBuf;

    if (pwb == NULL)
    {
        return(NO_ERROR);
    }

    //
    // Release all resources used by this authentication session.
    //

    EapTrace("EapEnd(%s)", pwb->aszIdentity);

    LocalFree(pwb->pUIContext);
    LocalFree(pwb->pDataFromInteractiveUI);

    if (pwb->pUserAttributes != NULL)
    {
        //
        // Free up Attributes
        //

        LocalFree(pwb->pUserAttributes[0].Value);
        LocalFree(pwb->pUserAttributes[1].Value);
        LocalFree(pwb->pUserAttributes);
    }

    if (pwb->pMPPEKeyAttributes != NULL)
    {
        //
        // Free up the MPPE Key Attributes
        //

        LocalFree(pwb->pMPPEKeyAttributes[0].Value);
        LocalFree(pwb->pMPPEKeyAttributes);
    }

    ZeroMemory(pwb, sizeof(EAPCB));

    LocalFree(pwb);

    return(NO_ERROR);
}

/*

Notes:
    RasEapMakeMessage entry point called by the PPP engine thru the passed
    address.
    
*/

DWORD APIENTRY
EapMakeMessage(
    IN  VOID*               pWorkBuf,
    IN  PPP_EAP_PACKET*     pReceiveBuf,
    OUT PPP_EAP_PACKET*     pSendBuf,
    IN  DWORD               cbSendBuf,
    OUT PPP_EAP_OUTPUT*     pResult,
    IN  PPP_EAP_INPUT*      pInput 
)
{
    EAPCB*  pwb = (EAPCB*)pWorkBuf;

    EapTrace("EapMakeMessage(%s)", pwb->aszIdentity);

    //
    // Call the appropriate routine to process the event.
    //

    if (pwb->fAuthenticator)
    {
        return(AuthenticatorMakeMessage(pwb, 
                                        pReceiveBuf,
                                        pSendBuf, 
                                        cbSendBuf, 
                                        pInput, 
                                        pResult));
    }
    else
    {
        return(AuthenticateeMakeMessage(pwb, 
                                        pReceiveBuf, 
                                        pSendBuf, 
                                        cbSendBuf, 
                                        pInput, 
                                        pResult));
    }
}

/*

Notes:
    RasEapGetIdentity entry point called by the EAP-PPP engine by name.

*/

DWORD APIENTRY
RasEapGetIdentity(
    IN  DWORD           dwEapTypeId,
    IN  HWND            hwndParent,
    IN  DWORD           dwFlags,
    IN  const WCHAR*    pwszPhonebook,
    IN  const WCHAR*    pwszEntry,
    IN  BYTE*           pConnectionDataIn,
    IN  DWORD           dwSizeOfConnectionDataIn,
    IN  BYTE*           pUserDataIn,
    IN  DWORD           dwSizeOfUserDataIn,
    OUT BYTE**          ppUserDataOut,
    OUT DWORD*          pdwSizeOfUserDataOut,
    OUT WCHAR**         ppwszIdentity
)
{
    DWORD               dwErr           = NO_ERROR;

    if ( dwFlags & RAS_EAP_FLAG_NON_INTERACTIVE )
    {
        dwErr = ERROR_INTERACTIVE_MODE;
        goto LDone;
    }

    if ( dwFlags & RAS_EAP_FLAG_MACHINE_AUTH )
    {
        dwErr = ERROR_NOT_SUPPORTED;
        goto LDone;
    }

    if (dwFlags & RAS_EAP_FLAG_ROUTER)
    {
        //
        // A routing interface must have its credentials set beforehand
        //

        if (   (NULL == pUserDataIn)
            || (sizeof(EAP_NAME_DIALOG) != dwSizeOfUserDataIn))
        {
            //
            // Bad saved credentials
            //

            EapTrace("Credentials for this interface have not been set");
            dwErr = E_FAIL;
            goto LDone;
        }
    }
    else
    {
        //
        // Ignore old values. Prompt for username and password.
        //

        pUserDataIn = NULL;
    }

    dwErr = GetIdentity(
                    hwndParent,
                    pUserDataIn,
                    dwSizeOfUserDataIn,
                    ppUserDataOut,
                    pdwSizeOfUserDataOut,
                    ppwszIdentity);

LDone:

    return(dwErr);
}

/*

Notes:
    RasEapInvokeConfigUI entry point called by the EAP-PPP engine by name.

*/

DWORD APIENTRY
RasEapInvokeConfigUI(
    IN  DWORD       dwEapTypeId,
    IN  HWND        hwndParent,
    IN  DWORD       dwFlags,
    IN  BYTE*       pConnectionDataIn,
    IN  DWORD       dwSizeOfConnectionDataIn,
    OUT BYTE**      ppConnectionDataOut,
    OUT DWORD*      pdwSizeOfConnectionDataOut
)
{
    DWORD       dwDisplayedNumber;
    WCHAR       awszMessage[100];
    DWORD       dwErr               = NO_ERROR;

    *ppConnectionDataOut = NULL;
    *pdwSizeOfConnectionDataOut = 0;

    if (   (NULL == pConnectionDataIn)
        || (0 == dwSizeOfConnectionDataIn))
    {
        //
        // We are configuring for the first time
        //

        dwDisplayedNumber = 1;
    }
    else
    {
        //
        // How many times has this been configured?
        //

        dwDisplayedNumber = *(DWORD*)pConnectionDataIn;
    }

    StringCchPrintf((LPTSTR)awszMessage, sizeof(awszMessage),(LPTSTR)L"%5d times",dwDisplayedNumber);

    MessageBox(hwndParent, awszMessage,
        L"You have configured this interface...", MB_OK | MB_ICONINFORMATION);

    //
    // Allocate memory for the OUT parameter
    //

    *ppConnectionDataOut = (BYTE*)LocalAlloc(LPTR, sizeof(DWORD));

    if (NULL == *ppConnectionDataOut)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto LDone;
    }

    //
    // This has been configured one more time
    //

    dwDisplayedNumber += 1;

    //
    // Set the OUT parameters
    //

    CopyMemory(*ppConnectionDataOut, (BYTE*)&dwDisplayedNumber,
        sizeof(DWORD));
    *pdwSizeOfConnectionDataOut = sizeof(DWORD);

LDone:

    return(dwErr);
}

/*

Notes:
    RasEapInvokeInteractiveUI entry point called by the EAP-PPP engine by name.

*/

DWORD APIENTRY
RasEapInvokeInteractiveUI(
    IN  DWORD           dwEapTypeId,
    IN  HWND            hWndParent,
    IN  PBYTE           pUIContextData,
    IN  DWORD           dwSizeofUIContextData,
    OUT PBYTE *         ppDataFromInteractiveUI,
    OUT DWORD *         lpdwSizeOfDataFromInteractiveUI
)
{
    EapTrace("RasEapInvokeInteractiveUI");

    if (MessageBox(hWndParent, 
                      (WCHAR*)pUIContextData, 
                      L"EAP sample", 
                      MB_OKCANCEL) == IDOK)
    {
        *lpdwSizeOfDataFromInteractiveUI = (wcslen(L"OK") + 1) * sizeof(WCHAR);

        if ((*ppDataFromInteractiveUI = 
                    LocalAlloc(LPTR, *lpdwSizeOfDataFromInteractiveUI)) == NULL)
        {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        StringCchCopy((LPTSTR)*ppDataFromInteractiveUI,(*lpdwSizeOfDataFromInteractiveUI),(LPTSTR)L"OK");
    }
    else
    {
        *ppDataFromInteractiveUI         = NULL;
        *lpdwSizeOfDataFromInteractiveUI = 0;
    }
    
    return(NO_ERROR);
}

/*

Notes:
    RasEapFreeMemory entry point called by the EAP-PPP engine by name.

*/

DWORD APIENTRY
RasEapFreeMemory(
    IN  BYTE*   pMemory
)
{
    EapTrace("RasEapFreeMemory");
    LocalFree(pMemory);
    return(NO_ERROR);
}

/*---------------------------------------------------------------------------
    Internal routines 
---------------------------------------------------------------------------*/

/*

Notes:
    Print debug information.
    
*/

VOID   
EapTrace(
    IN  CHAR*   Format, 
    ... 
) 
{
    va_list arglist;

    va_start(arglist, Format);

    TraceVprintfExA(g_dwEapTraceId, 
        0x00010000 | TRACE_USE_MASK | TRACE_USE_MSEC,
        Format,
        arglist);

    va_end(arglist);
}

/*

Notes:
    Will convert a 32 bit integer from host format to wire format.
    
*/

VOID
HostToWireFormat32(
    IN     DWORD dwHostFormat,
    IN OUT PBYTE pWireFormat
)
{
    *((PBYTE)(pWireFormat)+0) = (BYTE) ((DWORD)(dwHostFormat) >> 24);
    *((PBYTE)(pWireFormat)+1) = (BYTE) ((DWORD)(dwHostFormat) >> 16);
    *((PBYTE)(pWireFormat)+2) = (BYTE) ((DWORD)(dwHostFormat) >>  8);
    *((PBYTE)(pWireFormat)+3) = (BYTE) (dwHostFormat);
}

/*

Notes:
    Will convert a 16 bit integer from host format to wire format.
    
*/

VOID
HostToWireFormat16(
    IN     WORD  wHostFormat,
    IN OUT PBYTE pWireFormat
)
{
    *((PBYTE)(pWireFormat)+0) = (BYTE) ((DWORD)(wHostFormat) >>  8);
    *((PBYTE)(pWireFormat)+1) = (BYTE) (wHostFormat);
}

/*

Notes:
    Will convert a 16 bit integer from wire format to host format.
    
*/

WORD
WireToHostFormat16(
    IN PBYTE pWireFormat
)
{
    WORD wHostFormat = ((*((PBYTE)(pWireFormat)+0) << 8) +
                        (*((PBYTE)(pWireFormat)+1)));

    return(wHostFormat);
}

/*

Notes:
    Authenticatee side event handler.
    
*/

DWORD
AuthenticateeMakeMessage(
    IN  EAPCB*            pwb,
    IN  PPP_EAP_PACKET*   pReceiveBuf,
    OUT PPP_EAP_PACKET*   pSendBuf,
    IN  DWORD             cbSendBuf,
    IN  PPP_EAP_INPUT*    pInput,
    OUT PPP_EAP_OUTPUT*   pResult
)
{
    DWORD dwRetCode = NO_ERROR;
    BYTE* pDataFromInteractiveUI;
    DWORD dwSizeOfDataFromInteractiveUI;

    EapTrace("AuthenticateeMakeMessage");

    switch(pwb->EapState)
    {
        case MYSTATE_Initial:

            if (pwb->fFlags & RAS_EAP_FLAG_ROUTER)
            {
                pwb->EapState = MYSTATE_WaitForRequest;

                break;
            }
            pwb->bRecvPacketId = pReceiveBuf->Id;
            if (NULL == pwb->pDataFromInteractiveUI)
            {
                WCHAR * pUIContextData = L"You are being authenticated by a Sample EAP";
                //
                // Bring up interactive UI to notify user that he/she is being
                // authenticated via the sample EAP
                //

                pResult->fInvokeInteractiveUI = TRUE;

                pResult->dwSizeOfUIContextData =
                   (wcslen(pUIContextData)+1) *
                       sizeof(WCHAR);

                pResult->pUIContextData = LocalAlloc(
                                              LPTR,
                                              pResult->dwSizeOfUIContextData);

                if (pResult->pUIContextData == NULL)
                {
                    EapTrace("OUt of memory");
                    return(ERROR_NOT_ENOUGH_MEMORY);
                }

                StringCchCopy((LPTSTR)pResult->pUIContextData,
                             wcslen(pUIContextData) + 1,
                             (LPTSTR)pUIContextData);

                pwb->pUIContext = pResult->pUIContextData;

                pwb->EapState = MYSTATE_WaitForUserOK;

                break;
            }

            //
            // Else, fall through
            //

        case MYSTATE_WaitForUserOK:

            //
            // Wait for response from user
            //

            if (   pInput->fDataReceivedFromInteractiveUI
                || (NULL != pwb->pDataFromInteractiveUI))
            {
                if (pInput->fDataReceivedFromInteractiveUI)
                {
                    pDataFromInteractiveUI =
                            pInput->pDataFromInteractiveUI;
                    dwSizeOfDataFromInteractiveUI =
                            pInput->dwSizeOfDataFromInteractiveUI;
                }
                else
                {
                    pDataFromInteractiveUI =
                            pwb->pDataFromInteractiveUI;
                    dwSizeOfDataFromInteractiveUI =
                            pwb->dwSizeOfDataFromInteractiveUI;
                }

                LocalFree(pwb->pUIContext);
                pwb->pUIContext = NULL;

                //
                // If user doesn't like this, then we hangup the line
                //

                if (dwSizeOfDataFromInteractiveUI !=
                            (wcslen(L"OK")+1) * sizeof(WCHAR))
                {
                    EapTrace("User chose to cancel");
                    dwRetCode = ERROR_ACCESS_DENIED;
                    break;
                }

                if (wcscmp((WCHAR*)pDataFromInteractiveUI, L"OK") != 0)
                {
                    EapTrace("User chose to cancel");
                    dwRetCode = ERROR_ACCESS_DENIED;
                    break;
                }

                
                pwb->EapState = MYSTATE_WaitForRequest;
            }
            else
            {
                //
                // Ignore all other events.
                //

                pResult->Action = EAPACTION_NoAction;
            }

            if ( !(pwb->fFlags & RAS_EAP_FLAG_8021X_AUTH ) )
            {
                //
                // if this is a VPN client, we can rely on 
                // retransmission.  But with wireless client
                // we cannot do that.
                //
                break;
            }
            //fall thru

        case MYSTATE_WaitForRequest:

            if ( (pwb->fFlags & RAS_EAP_FLAG_8021X_AUTH ) )
            {
                //
                // Build the response packet
                //
                MakeResponseMessage1(pwb, pSendBuf, cbSendBuf);

                //
                // Response packets should not be sent with any timeout
                //

                pResult->Action = EAPACTION_Send;

                //
                // We are done so we change to MYSTATE_Done
                //

                pwb->EapState = MYSTATE_Done;

                break;

            }
            else if (pReceiveBuf != NULL )
            {
                //
                // If we received a request packet from the server then we
                // process it.
                //

                if (pReceiveBuf->Code == EAPCODE_Request)
                {
                    //
                    // Build the response packet
                    //

                    MakeResponseMessage(pwb, pReceiveBuf, pSendBuf, cbSendBuf);

                    //
                    // Response packets should not be sent with any timeout
                    //

                    pResult->Action = EAPACTION_Send;

                    //
                    // We are done so we change to MYSTATE_Done
                    //

                    pwb->EapState = MYSTATE_Done;

                    break;
                }
                else
                {
                    //
                    // We shouldn't get any other packet in this state so
                    // we simply drop this invalid packet
                    //

                    pResult->Action = EAPACTION_NoAction;
                    dwRetCode = ERROR_PPP_INVALID_PACKET;
                    break;
                }
            }

            break;

        case MYSTATE_Done:
        {
            if (pReceiveBuf == NULL)
            {
                //
                // If we did not receive a packet then we check to see if
                // the fSuccessPacketReceived flag is set
                //

                if ((pInput != NULL) && (pInput->fSuccessPacketReceived))
                {
                    //
                    // We are done
                    //

                    //
                    // Create the MPPE Key Attribute and give it to the EAP-PPP 
                    // engine.
                    //

                    dwRetCode = MakeMPPEKeyAttributes(pwb);

                    if (NO_ERROR == dwRetCode)
                    {
                        pResult->pUserAttributes = pwb->pMPPEKeyAttributes;
                    }
                    
                    pResult->Action = EAPACTION_Done;
                    pwb->EapState   = MYSTATE_Done;
                }
                else
                {
                    //
                    // Otherwise we ignore this event
                    //

                    pResult->Action = EAPACTION_NoAction;
                }

                break;
            }

            if ((pReceiveBuf->Code == EAPCODE_Success) ||
                (pReceiveBuf->Code == EAPCODE_Failure))
            {
                if (pReceiveBuf->Code == EAPCODE_Success)
                {
                    //
                    // If we received success or failure, we are done, but first
                    // make sure the ID's match
                    //

                    //
                    // Create the MPPE Key Attribute and give it to the EAP-PPP 
                    // engine.
                    //

                    dwRetCode = MakeMPPEKeyAttributes(pwb);

                    if (NO_ERROR == dwRetCode)
                    {
                        pResult->pUserAttributes = pwb->pMPPEKeyAttributes;
                    }

                    pResult->Action = EAPACTION_Done;
                    pwb->EapState   = MYSTATE_Done;
                }
                else
                {
                    //
                    // Otherwise drop the packet
                    //

                    pResult->Action = EAPACTION_NoAction;
                    dwRetCode       = ERROR_PPP_INVALID_PACKET;
                }

                break;
            }
            else if (pReceiveBuf->Code == EAPCODE_Request)  
            {
                //
                // We must always respond to requests
                //

                MakeResponseMessage(pwb, pReceiveBuf, pSendBuf, cbSendBuf);

                //
                // Response packets should not be sent with any timeout
                //

                pResult->Action = EAPACTION_Send;
            }
            else
            {
                //
                // Otherwise we received an illegal packet, wrong code set
                // So simply drop the packet.
                //

                pResult->Action = EAPACTION_NoAction;
                dwRetCode       = ERROR_PPP_INVALID_PACKET;
            }
        }
    }

    return(dwRetCode);
}


VOID
MakeResponseMessage1(
    IN  EAPCB*           pwb,    
    OUT PPP_EAP_PACKET * pSendBuf,
    IN  DWORD            cbSendBuf
)
{
    BYTE* pcbPassword;
    CHAR* pchPassword;

    EapTrace("MakeResponseMessage1");

    (void)cbSendBuf;

    //
    // Fill in the password.
    //

    pcbPassword = pSendBuf->Data + 1; 

    *pcbPassword = (BYTE)strlen(pwb->aszPassword);

    pchPassword = pcbPassword + 1;

    StringCbCopy((LPTSTR)pchPassword,strlen(pwb->aszPassword)+1, (LPTSTR)pwb->aszPassword);

    //
    // Set the response code
    //

    pSendBuf->Code = (BYTE)EAPCODE_Response;

    //
    // The Reponse packet Id MUST match the Request packet Id.
    //

    pSendBuf->Id = pwb->bRecvPacketId;

    //
    // The Success/Failure packet that we get must match the ID of the last 
    // response sent
    //

    pwb->dwIdExpected = pSendBuf->Id;

    //
    // Set the EAP type ID
    //

    pSendBuf->Data[0] = (BYTE)PPP_EAP_PROTOCOL_ID;

    //
    // Set the length of the packet
    //

    HostToWireFormat16((WORD)(PPP_EAP_PACKET_HDR_LEN+1+*pcbPassword+1),
                       pSendBuf->Length);
}
/*

Notes:
    Builds a response packet. 'pwb' is the address of the work
    buffer associated with the port.
    
*/

VOID
MakeResponseMessage(
    IN  EAPCB*           pwb,
    IN  PPP_EAP_PACKET * pReceiveBuf,
    OUT PPP_EAP_PACKET * pSendBuf,
    IN  DWORD            cbSendBuf
)
{
    BYTE* pcbPassword;
    CHAR* pchPassword;

    EapTrace("MakeResponseMessage");

    (void)cbSendBuf;

    //
    // Fill in the password.
    //

    pcbPassword = pSendBuf->Data + 1; 

    *pcbPassword = (BYTE)strlen(pwb->aszPassword);

    pchPassword = pcbPassword + 1;

    StringCbCopy((LPTSTR)pchPassword, strlen(pwb->aszPassword)+1,(LPTSTR)pwb->aszPassword);

    //
    // Set the response code
    //

    pSendBuf->Code = (BYTE)EAPCODE_Response;

    //
    // The Reponse packet Id MUST match the Request packet Id.
    //

    pSendBuf->Id = pReceiveBuf->Id;

    //
    // The Success/Failure packet that we get must match the ID of the last 
    // response sent
    //

    pwb->dwIdExpected = pSendBuf->Id;

    //
    // Set the EAP type ID
    //

    pSendBuf->Data[0] = (BYTE)PPP_EAP_PROTOCOL_ID;

    //
    // Set the length of the packet
    //

    HostToWireFormat16((WORD)(PPP_EAP_PACKET_HDR_LEN+1+*pcbPassword+1),
                       pSendBuf->Length);
}

/*

Notes:
    Builds a result packet (Success or Failure) in caller's 'pSendBuf' 
    buffer. 'cbSendBuf' is the length of caller's buffer.  
    'dwError' indicates whether an Success or Failure should be generated, 
    'bId' is the Id of the Success of Failure packet.
    
*/

VOID
MakeResultMessage(
    IN  EAPCB *         pwb,
    IN  DWORD           dwError,
    OUT PPP_EAP_PACKET* pSendBuf,
    IN  DWORD           cbSendBuf 
)
{
    EapTrace("MakeResultMessage");

    (void)cbSendBuf;

    //
    // If there was no error then we send a Success packet, otherwise we send
    // a failure message
    //

    if (dwError == NO_ERROR)
    {
        pSendBuf->Code = EAPCODE_Success;
    }
    else
    {
        pSendBuf->Code = EAPCODE_Failure;
    }

    //
    // The Id of a success or failure message MUST match the Id of the last
    // response received from the client according to the EAP spec.
    //

    pSendBuf->Id = (BYTE)pwb->dwInitialPacketId;

    //
    // Set the length
    //

    HostToWireFormat16((WORD)PPP_EAP_PACKET_HDR_LEN, (PBYTE)pSendBuf->Length);
}

/*

Notes:
    Will build a request packet.
    
*/

VOID
MakeRequestMessage(
    IN  EAPCB*           pwb,
    OUT PPP_EAP_PACKET * pSendBuf,
    IN DWORD             cbSendBuf
)
{
    BYTE *pcbPeerMessage;
    CHAR *pchPeerMessage;

    EapTrace("MakeRequestMessage");

    pcbPeerMessage  = pSendBuf->Data + 1;

    *pcbPeerMessage = (BYTE)strlen("send password");

    pchPeerMessage  = pcbPeerMessage + 1;

    StringCbCopy((LPTSTR)pchPeerMessage, strlen("send password")+1,(LPTSTR)"send password");

    //
    // Set the Request Code
    // 

    pSendBuf->Code = EAPCODE_Request;

    //
    // Set the request packet identifier. Start with the Id that was give to us
    //

    pSendBuf->Id = (BYTE)pwb->dwInitialPacketId;

    //
    // Set the length
    //

    HostToWireFormat16((WORD)(PPP_EAP_PACKET_HDR_LEN+1+*pcbPeerMessage+1),  
                              pSendBuf->Length);

    //
    // Set the EAP Type Id
    //

    pSendBuf->Data[0] = PPP_EAP_PROTOCOL_ID;

}

/*

Notes:
    Authenticator side event handler.
    
*/

DWORD
AuthenticatorMakeMessage(
    IN  EAPCB*              pwb,
    IN  PPP_EAP_PACKET*     pReceiveBuf,
    OUT PPP_EAP_PACKET*     pSendBuf,
    IN  DWORD               cbSendBuf,
    IN  PPP_EAP_INPUT*      pInput,
    OUT PPP_EAP_OUTPUT*     pResult 
)
{
    DWORD dwRetCode = NO_ERROR;

    EapTrace("AuthenticatorMakeMessage");

    switch(pwb->EapState)
    {
        case MYSTATE_ReqSent:

            if (pReceiveBuf != NULL)
            {
                //
                // If we received a packet
                //

                if (pReceiveBuf->Code == EAPCODE_Response)
                {
                    //
                    // If we received a response to our identity request, 
                    // then process it. There is no need to check the Id    
                    // here since the PPP engine will only pass on packets
                    // whose Id matches those set with the 
                    // EAPACTION_SendWithTimeout action.
                    //

                    dwRetCode = GetPasswordFromResponse(pReceiveBuf, 
                                                         pwb->aszPassword);

                    if (dwRetCode != NO_ERROR)
                    {    
                        if (dwRetCode != ERROR_PPP_INVALID_PACKET)
                        {
                            //
                            // Fatal error, we fail the connection. 
                            //

                            return(dwRetCode);
                        }
                    }
                    else
                    {
                        //
                        // Request authentication provider to authenticate 
                        // this user.
                        //
            
                        dwRetCode = MakeAuthenticationAttributes(
                                                            pwb->aszIdentity, 
                                                            pwb->aszPassword,   
                                                            pwb);

                        if (dwRetCode != NO_ERROR)
                        {
                            return(dwRetCode);
                        }
                        else
                        {
                            //
                            // Authentication request completed successfully.
                            // This is an asynchronous call so we change state
                            // and wait for the provider to complete the 
                            // authentication.  
                            //

                            pResult->pUserAttributes = pwb->pUserAttributes;

                            pResult->Action = EAPACTION_Authenticate;

                            // 
                            // Save Id so that we can send the correct one
                            // in the success/failure packet
                            //

                            pwb->dwIdExpected = pReceiveBuf->Id; 

                            pwb->EapState = 
                                    MYSTATE_WaitForAuthenticationToComplete;
                        }        
                    }

                    break;
                }
                else
                {
                    //
                    // Otherwise silently drop the packet. 
                    // We should only get requests
                    //

                    pResult->Action = EAPACTION_NoAction;

                    break;
                }
            }

            break;

        case MYSTATE_Initial:

            //
            // Create Request packet
            //

            MakeRequestMessage(pwb, pSendBuf, cbSendBuf);

            //
            // Request messages must be sent with a timeout
            //

            pResult->Action = EAPACTION_SendWithTimeoutInteractive;

            //
            // Since we have sent a request we change to the ReqSent state
            // where we will wait for a response.
            //

            pwb->EapState = MYSTATE_ReqSent;

            break;

        case MYSTATE_WaitForAuthenticationToComplete:
        {
            if (pInput != NULL)
            {
                //
                // Did the authentication provider complete the authentication?
                //

                if (pInput->fAuthenticationComplete)
                {
                    //
                    // If the user failed to authenticate, save the failure 
                    // code.
                    //

                    if (pInput->dwAuthResultCode != NO_ERROR)
                    {
                        pwb->dwResult = pInput->dwAuthResultCode;
                    }

                    pResult->Action = EAPACTION_SendAndDone;
                    pwb->EapState   = MYSTATE_Done;

                    //
                    // fall thru to the MYSTATE_Done state where we will
                    // send a Success or Failure packet
                    //
                }
            }

            if ((pInput == NULL) || (!pInput->fAuthenticationComplete))
            {
                //
                // Ignore everything if authentication is not complete
                //

                pResult->Action = EAPACTION_NoAction;

                break;
            }

            //
            // ...fall thru to the MYSTATE_Done state where we will
            // send a Success or Failure packet
            //
        }

        case MYSTATE_Done:
        {
            //
            // Make Success or Failure packet.  
            //

            MakeResultMessage(pwb, pwb->dwResult, pSendBuf, cbSendBuf);

            if (NO_ERROR == pwb->dwResult)
            {
                //
                // If we made a Success packet, create the MPPE Key Attribute
                // and give it to the EAP-PPP engine.
                //

                dwRetCode = MakeMPPEKeyAttributes(pwb);

                if (NO_ERROR == dwRetCode)
                {
                    pResult->pUserAttributes = pwb->pMPPEKeyAttributes;
                }
            }

            pResult->Action = EAPACTION_SendAndDone;

            pResult->dwAuthResultCode = pwb->dwResult;

            break;
        }

        default:

            break;
    }

    return(dwRetCode);

}

/*

Notes:
    Fill caller's pszPassword' buffer with the password, in the request 
    packet.

    Returns NO_ERROR if successful., or ERROR_PPP_INVALID_PACKET if the 
    packet is misformatted in any way.

*/

DWORD
GetPasswordFromResponse(
    IN  PPP_EAP_PACKET* pReceiveBuf,
    OUT CHAR*           pszPassword
)
{
    BYTE* pcbPassword;
    CHAR* pchPassword;
    WORD  cbPacket;

    EapTrace("GetPasswordFromResponse");

    cbPacket = WireToHostFormat16(pReceiveBuf->Length);

    //
    // Extract the password
    //

    if (cbPacket < (PPP_EAP_PACKET_HDR_LEN + 1 + 1))
    {
        EapTrace("Number of bytes in the EAP packet is only %d", cbPacket);

        return(ERROR_PPP_INVALID_PACKET);
    }

    pcbPassword = pReceiveBuf->Data + 1;
    pchPassword = pcbPassword + 1;

    if (cbPacket < PPP_EAP_PACKET_HDR_LEN + 1 + 1 + *pcbPassword)
    {
        EapTrace("Number of characters in password is %d", *pcbPassword);
        EapTrace("Number of bytes in the EAP packet is only %d", cbPacket);

        return ERROR_PPP_INVALID_PACKET;
    }
    if ( *pcbPassword >  PWLEN )
    {
        EapTrace ("Password length received in the packet is > PWLEN" );
        return ERROR_PPP_INVALID_PACKET;
    }
    CopyMemory(pszPassword, pchPassword, *pcbPassword);

    //
    // NULL terminate the password
    //

    pszPassword[ *pcbPassword ] = '\0';

    return(NO_ERROR);
}

/*

Notes:
    Will build user attributes and send them to the authentication provider
    for authentication.

*/

DWORD 
MakeAuthenticationAttributes(
    IN CHAR *   szUserName,    
    IN CHAR *   szPassword,    
    IN EAPCB *  pwb
)
{
    EapTrace("MakeAuthenticationAttributes");

    if (pwb->pUserAttributes != NULL)
    {
        LocalFree(pwb->pUserAttributes[0].Value);
        LocalFree(pwb->pUserAttributes[1].Value);
        LocalFree(pwb->pUserAttributes);

        pwb->pUserAttributes = NULL;
    }

    pwb->pUserAttributes = (RAS_AUTH_ATTRIBUTE *)
                           LocalAlloc(LPTR, sizeof (RAS_AUTH_ATTRIBUTE) * 3);

    if (pwb->pUserAttributes == NULL) 
    {
        return(GetLastError());
    }

    //
    // for user name
    //

    pwb->pUserAttributes[0].raaType  = raatUserName;
    pwb->pUserAttributes[0].dwLength = strlen(szUserName);
    pwb->pUserAttributes[0].Value    = LocalAlloc(LPTR, (strlen(szUserName)+1));

    if (pwb->pUserAttributes[0].Value == NULL)
    { 
        LocalFree(pwb->pUserAttributes); 

        pwb->pUserAttributes = NULL;

        return(GetLastError());
    }

    CopyMemory(pwb->pUserAttributes[0].Value,szUserName, strlen(szUserName));

    //
    // for password
    //

    pwb->pUserAttributes[1].raaType  = raatUserPassword;
    pwb->pUserAttributes[1].dwLength = strlen(szPassword);
    pwb->pUserAttributes[1].Value    = LocalAlloc(LPTR, (strlen(szPassword)+1));

    if (pwb->pUserAttributes[1].Value == NULL) 
    {
        LocalFree(pwb->pUserAttributes[0].Value);

        LocalFree(pwb->pUserAttributes); 

        pwb->pUserAttributes = NULL;

        return(GetLastError());
    }

    CopyMemory(pwb->pUserAttributes[1].Value,szPassword, strlen(szPassword));
  
    //
    // For Termination
    //

    pwb->pUserAttributes[2].raaType  = raatMinimum;
    pwb->pUserAttributes[2].dwLength = 0;
    pwb->pUserAttributes[2].Value    = NULL;

    return(NO_ERROR);
}

/*

Notes:

*/

DWORD
MakeMPPEKeyAttributes(
    IN EAPCB *  pwb
)
{
    DWORD   dwErr           = NO_ERROR;
    DWORD   dwIndex;
    DWORD   dwSendPattern;
    DWORD   dwRecvPattern;
    BYTE*   pByte;

    EapTrace("MakeMPPEKeyAttributes");

    if (NULL != pwb->pMPPEKeyAttributes)
    {
        //
        // Free up the MPPE Key Attributes if they exist
        //

        LocalFree(pwb->pMPPEKeyAttributes[0].Value);
        LocalFree(pwb->pMPPEKeyAttributes[1].Value);
        LocalFree(pwb->pMPPEKeyAttributes);
        pwb->pMPPEKeyAttributes = NULL;
    }

    //
    // We need 3 RAS_AUTH_ATTRIBUTE structs: for MS-MPPE-Send-Key, 
    // MS-MPPE-Recv-Key, and termination
    //

    pwb->pMPPEKeyAttributes = (RAS_AUTH_ATTRIBUTE *) LocalAlloc(
                LPTR, sizeof(RAS_AUTH_ATTRIBUTE) * 3);

    if (NULL == pwb->pMPPEKeyAttributes)
    {
        dwErr = GetLastError();
        goto LDone;
    }

    if (pwb->fAuthenticator)
    {
        dwSendPattern = 0xAB;
        dwRecvPattern = 0xCD;
    }
    else
    {
        dwSendPattern = 0xCD;
        dwRecvPattern = 0xAB;
    }

    //
    // Bytes needed:
    //      4: Vendor-Id
    //      1: Vendor-Type
    //      1: Vendor-Length
    //      2: Salt
    //      1: Key-Length
    //     32: Key
    //     15: Padding
    //     -----------------
    //     56: Total
    //

    //
    // Copy MS-MPPE-Send-Key
    //

    pwb->pMPPEKeyAttributes[0].Value = LocalAlloc(LPTR, 56);

    if (pwb->pMPPEKeyAttributes[0].Value == NULL)
    {
        dwErr = GetLastError();
        goto LDone;
    }

    pByte = pwb->pMPPEKeyAttributes[0].Value;

    HostToWireFormat32(311, pByte); // Vendor-Id
    pByte[4] = 16;                  // Vendor-Type (MS-MPPE-Send-Key)
    pByte[5] = 56 - 4;              // Vendor-Length (all except Vendor-Id)
    // pByte[6-7] is the zero-filled salt field
    pByte[8] = 32;                  // Key-Length

    {
        //
        // This is just an example. Copy a real key here.
        //

        CopyMemory(pByte + 9, pwb->aszPassword, 32);

        for (dwIndex = 0; dwIndex < 32; dwIndex++)
        {
            pByte[9 + dwIndex] ^= dwSendPattern;
        }
    }

    // pByte[41-55] is the Padding (zero octets)

    pwb->pMPPEKeyAttributes[0].dwLength = 56;
    pwb->pMPPEKeyAttributes[0].raaType  = raatVendorSpecific;

    //
    // Copy MS-MPPE-Recv-Key
    //

    pwb->pMPPEKeyAttributes[1].Value = LocalAlloc(LPTR, 56);

    if (pwb->pMPPEKeyAttributes[1].Value == NULL)
    {
        dwErr = GetLastError();
        goto LDone;
    }

    pByte = pwb->pMPPEKeyAttributes[1].Value;

    HostToWireFormat32(311, pByte); // Vendor-Id
    pByte[4] = 17;                  // Vendor-Type (MS-MPPE-Recv-Key)
    pByte[5] = 56 - 4;              // Vendor-Length (all except Vendor-Id)
    // pByte[6-7] is the zero-filled salt field
    pByte[8] = 32;                  // Key-Length

    {
        //
        // This is just an example. Copy a real key here.
        //

        CopyMemory(pByte + 9, pwb->aszPassword, 32);

        for (dwIndex = 0; dwIndex < 32; dwIndex++)
        {
            pByte[9 + dwIndex] ^= dwRecvPattern;
        }
    }

    // pByte[41-55] is the Padding (zero octets)

    pwb->pMPPEKeyAttributes[1].dwLength = 56;
    pwb->pMPPEKeyAttributes[1].raaType  = raatVendorSpecific;

    //
    // For Termination
    //

    pwb->pMPPEKeyAttributes[2].raaType  = raatMinimum;
    pwb->pMPPEKeyAttributes[2].dwLength = 0;
    pwb->pMPPEKeyAttributes[2].Value    = NULL;

LDone:

    if (NO_ERROR != dwErr)
    {
        //
        // If something failed, free the allocated memory
        //

        if (pwb->pMPPEKeyAttributes != NULL)
        {
            LocalFree(pwb->pMPPEKeyAttributes[0].Value);
            LocalFree(pwb->pMPPEKeyAttributes[1].Value);
            LocalFree(pwb->pMPPEKeyAttributes);
            pwb->pMPPEKeyAttributes = NULL;
        }
    }

    return(dwErr);
}

/*

Notes:

*/

DWORD
GetIdentity(
    IN  HWND    hwndParent,
    IN  BYTE*   pUserDataIn,
    IN  DWORD   dwSizeOfUserDataIn,
    OUT BYTE**  ppUserDataOut,
    OUT DWORD*  pdwSizeOfUserDataOut,
    OUT WCHAR** ppwszIdentity
)
{
    EAP_NAME_DIALOG*    pEapNameDialog  = NULL;
    WCHAR*              pwszIdentity     = NULL;
    DWORD               dwErr           = NO_ERROR;


    //
    // Allocate memory for OUT parameters
    //

    pEapNameDialog = LocalAlloc(LPTR, sizeof(EAP_NAME_DIALOG));

    if (NULL == pEapNameDialog)
    {
        EapTrace("Out of memory");
        dwErr = GetLastError();
        goto LDone;
    }

    pwszIdentity = LocalAlloc(LPTR, (UNLEN + 1) * sizeof(WCHAR));

    if (NULL == pwszIdentity)
    {
        EapTrace("Out of memory");
        dwErr = GetLastError();
        goto LDone;
    }

    if (NULL != pUserDataIn)
    {
        //
        // Use the saved credentials if they exist
        //

        CopyMemory(pEapNameDialog, pUserDataIn, sizeof(EAP_NAME_DIALOG));
    }
    else
    {
        //
        // Else prompt for username and password
        //

        GetUsernameAndPassword(hwndParent, pEapNameDialog);
    }

    StringCchCopy((LPTSTR)pwszIdentity,(UNLEN + 1), (LPTSTR)pEapNameDialog->awszIdentity);

    //
    // Set the OUT paramters
    //

    *ppUserDataOut = (BYTE*)pEapNameDialog;
    *pdwSizeOfUserDataOut = sizeof(EAP_NAME_DIALOG);
    *ppwszIdentity = pwszIdentity;

    //
    // We mustn't LocalFree OUT parameters
    //

    pEapNameDialog = NULL;
    pwszIdentity = NULL;

LDone:

    LocalFree(pEapNameDialog);
    LocalFree(pwszIdentity);

    return(dwErr);
}

/*---------------------------------------------------------------------------
    Dialog routines 
---------------------------------------------------------------------------*/

/*

Notes:
    Displays the IDD_DIALOG dialog, and fills up pEapNameDialog with the 
    username and password.

*/

VOID
GetUsernameAndPassword(
    IN  HWND                hwndParent,
    IN  EAP_NAME_DIALOG*    pEapNameDialog
)
{
    DialogBoxParam(
        g_hInstance,
        MAKEINTRESOURCE(IDD_DIALOG),
        hwndParent,
        UsernameDialogProc,
        (LPARAM)pEapNameDialog);
}

/*

Notes:
    Callback function used with the DialogBoxParam function. It processes 
    messages sent to the dialog box. See the DialogProc documentation in MSDN.

*/

INT_PTR CALLBACK
UsernameDialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
)
{
    EAP_NAME_DIALOG*    pEapNameDialog;

    switch (unMsg)
    {
    case WM_INITDIALOG:
        
        return(InitUsernameDialog(hWnd, lParam));

    case WM_COMMAND:

        pEapNameDialog = (EAP_NAME_DIALOG*)GetWindowLongPtr(hWnd, DWLP_USER);

        return(UsernameCommand(pEapNameDialog, LOWORD(wParam), hWnd));
    }

    return(FALSE);
}

/*

Returns:
    FALSE (prevent Windows from setting the default keyboard focus).

Notes:
    Response to the WM_INITDIALOG message.

*/

BOOL
InitUsernameDialog(
    IN  HWND    hWnd,
    IN  LPARAM  lParam
)
{
    HWND    hWndEdit;

    SetWindowLongPtr(hWnd, DWLP_USER, lParam);

    return(FALSE);
}

/*

Returns:
    TRUE: We prrocessed this message.
    FALSE: We did not prrocess this message.

Notes:
    Response to the WM_COMMAND message.

*/

BOOL
UsernameCommand(
    IN  EAP_NAME_DIALOG*    pEapNameDialog,
    IN  WORD                wId,
    IN  HWND                hWndDlg
)
{
    HWND    hWnd;

    switch(wId)
    {
    case IDOK:

        //
        // Save whatever the user typed in as the user name
        //

        hWnd = GetDlgItem(hWndDlg, IDC_EDIT_NAME);
        GetWindowText(hWnd, pEapNameDialog->awszIdentity, UNLEN + 1);

        //
        // Save whatever the user typed in as the password
        //

        hWnd = GetDlgItem(hWndDlg, IDC_EDIT_PASSWD);
        GetWindowText(hWnd, pEapNameDialog->awszPassword, PWLEN + 1);

        // Fall through

    case IDCANCEL:

        EndDialog(hWndDlg, wId);
        return(TRUE);

    default:

        return(FALSE);
    }
}


