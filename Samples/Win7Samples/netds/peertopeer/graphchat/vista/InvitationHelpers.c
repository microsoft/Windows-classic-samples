/**********************************************************************
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All Rights Reserved.

Module Name:

    InvitationHelpers.c

Abstract:

    This C file includes sample code inviting people near me to an 
    application.

Feedback:
    If you have any questions or feedback, please contact us using 
    any of the mechanisms below:

    Email: peerfb@microsoft.com 
    Newsgroup: Microsoft.public.win32.programmer.networks 
    Website: http://www.microsoft.com/p2p 

--********************************************************************/

#pragma warning(disable:4201)   // nameless struct/union

#include "InvitationHelpers.h"

// Variable declarations
//
static HANDLE               g_hInviteEvent = NULL;
static HANDLE              g_hInviteWaitObject = NULL;
static HANDLE               g_hInvite = NULL;


//-----------------------------------------------------------------------------
// Function:   RefreshEndpointData
//
// Purpose:    Refreshes the data associated with an endpoint
//
// Returns:    HRESULT
//
HRESULT RefreshEndpointData(PCPEER_ENDPOINT pEndpoint)
{
    HPEEREVENT            hPeerEvent = NULL;
    HRESULT               hr = S_OK;
    HANDLE                hEvent = NULL;
    PEER_COLLAB_EVENT_DATA *pEventData = NULL;
    PEER_COLLAB_EVENT_REGISTRATION  eventReg = {0};

    hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (hEvent == NULL)
    {
        return E_OUTOFMEMORY;
    }

    eventReg.eventType = PEER_EVENT_REQUEST_STATUS_CHANGED;
    eventReg.pInstance = NULL;

    // Register to be notified when the request finishes  
    hr = PeerCollabRegisterEvent(hEvent, 1, &eventReg, &hPeerEvent);

    if (SUCCEEDED(hr))
    {
        hr = PeerCollabRefreshEndpointData(pEndpoint);

        if (FAILED(hr))
        {
            PeerCollabUnregisterEvent(hPeerEvent);
            CloseHandle(hEvent);
            return hr;
        }   

        // Block until an event is set indicating that endpoint data has
        // successfully been refreshed
        if (WaitForSingleObject(hEvent, INFINITE) == WAIT_OBJECT_0)
        {
            // Find out if refresh request succeeded
            hr = PeerCollabGetEventData(hPeerEvent, &pEventData);

            if (SUCCEEDED(hr) && 
                SUCCEEDED(pEventData->requestStatusChangedData.hrChange))
            {
                PeerFreeData(pEventData);
            }
        }
        PeerCollabUnregisterEvent(hPeerEvent);
    }
    CloseHandle(hEvent);

    return hr;
}


//-----------------------------------------------------------------------------
// Function:   ProcessInviteResponse
//
// Purpose:    Utility routine to check whether a contact/endpoint pair has
//             the desired application
//
// Returns:    DWORD
//        
DWORD CALLBACK ProcessInviteResponse(LPVOID lpContext)
{
    PPEER_INVITATION_RESPONSE pResponse = NULL;
    HRESULT hr = S_OK;
    ENDPOINT_INVITATION_CONTEXT * pInvitationContext = (ENDPOINT_INVITATION_CONTEXT *) lpContext;

    // Get the invitation response
    //
    hr = PeerCollabInviteEndpoint(pInvitationContext->pEndpoint, pInvitationContext->pInvite, &pResponse);

    if (SUCCEEDED(hr))
    {
        if (pResponse->action == PEER_INVITATION_RESPONSE_DECLINED)
        {
            MessageBox(NULL, L"Your invitation was declined.", L"Invitation Declined", MB_OK);
        }

        if (pResponse->action == PEER_INVITATION_RESPONSE_ERROR)
        {
            MessageBox(NULL, L"An error occured sending the invitation.", L"Invitation Error", MB_OK);
        }

        if (pResponse->action == PEER_INVITATION_RESPONSE_EXPIRED)
        {
            MessageBox(NULL, L"The invitation expired without being accepted.", L"Invitation Error", MB_OK);
        }     
    }

    // Free the used PEER resources
    //
    PeerFreeData(pResponse); 

    FreeInvitationContext(pInvitationContext);

    return 0;
}

//-----------------------------------------------------------------------------
// Function:   DuplicateInvitationContext
//
// Purpose:    Utility routine to duplicate the invitation context
//
// Returns:    HRESULT
//
HRESULT DuplicateInvitationContext(ENDPOINT_INVITATION_CONTEXT ** pDestination, ENDPOINT_INVITATION_CONTEXT * pSource)
{
    HRESULT hr = S_OK;
    ENDPOINT_INVITATION_CONTEXT * pTempDestination = NULL;
    size_t cbMessage = 0;

    cbMessage = (wcslen(pSource->pInvite->pwzMessage) * sizeof(WCHAR)) + sizeof(WCHAR);

    // Copy the base structure
    //
    pTempDestination = malloc(sizeof(ENDPOINT_INVITATION_CONTEXT));
    
    if (NULL == pTempDestination)
    {
        return E_OUTOFMEMORY;
    }

    ZeroMemory(pTempDestination, sizeof(ENDPOINT_INVITATION_CONTEXT));

    // Copy the endpoint structure
    //
    pTempDestination->pEndpoint = malloc(sizeof(PEER_ENDPOINT));

    if (NULL == pTempDestination->pEndpoint)
    {
        FreeInvitationContext(pTempDestination);
        return E_OUTOFMEMORY;
    }
    
    ZeroMemory(pTempDestination->pEndpoint, sizeof(PEER_ENDPOINT));
    memcpy(&pTempDestination->pEndpoint->address, &pSource->pEndpoint->address, sizeof(PEER_ADDRESS));

    // Copy the PEER_INVITATION structure
    //
    pTempDestination->pInvite = malloc(sizeof(PEER_INVITATION));

    if (NULL == pTempDestination->pInvite)
    {
        FreeInvitationContext(pTempDestination);
        return E_OUTOFMEMORY;
    }
    
    ZeroMemory(pTempDestination->pInvite, sizeof(PEER_INVITATION));
    
    // Copy the application data
    //
    pTempDestination->pInvite->applicationData.cbData = pSource->pInvite->applicationData.cbData;
    pTempDestination->pInvite->applicationData.pbData = malloc(pSource->pInvite->applicationData.cbData);

    if (NULL == pTempDestination->pInvite->applicationData.pbData)
    {
        FreeInvitationContext(pTempDestination);
        return E_OUTOFMEMORY;
    }
    
    memcpy(pTempDestination->pInvite->applicationData.pbData, pSource->pInvite->applicationData.pbData, pSource->pInvite->applicationData.cbData);

    // Copy the invitation message
    //
    pTempDestination->pInvite->pwzMessage = malloc(cbMessage);

    if (NULL == pTempDestination->pInvite->pwzMessage)
    {
        FreeInvitationContext(pTempDestination);
        return E_OUTOFMEMORY;
    }
    
    memcpy(pTempDestination->pInvite->pwzMessage, pSource->pInvite->pwzMessage, cbMessage);

    // Copy the application ID
    //
    pTempDestination->pInvite->applicationId = pSource->pInvite->applicationId;

    *pDestination = pTempDestination;

    return hr;
}

//-----------------------------------------------------------------------------
// Function:    Free Invitation Context
//
// Purpose:     Frees an ENDPOINT_INVITATION_CONTEXT structure allocated by the model
//              
// Returns:        VOID
//
VOID FreeInvitationContext(ENDPOINT_INVITATION_CONTEXT * pInvitationContext)
{
    if (NULL != pInvitationContext)
    {
        free(pInvitationContext->pEndpoint);
        if (NULL != pInvitationContext->pInvite)
        {
            free(pInvitationContext->pInvite->applicationData.pbData);
            free(pInvitationContext->pInvite->pwzMessage);
            free(pInvitationContext->pInvite);
        }
        free(pInvitationContext);
    }
}
//-----------------------------------------------------------------------------
// Function:   SendInviteOnThread
//
// Purpose:    Utility routine to send an invitation
//
// Returns:    HRESULT
//
HRESULT SendInviteOnThread(ENDPOINT_INVITATION_CONTEXT * pIncomingInvitationContext)
{
    HRESULT hr = S_OK;
    ENDPOINT_INVITATION_CONTEXT * pInvitationContext = NULL;

    hr = DuplicateInvitationContext(&pInvitationContext, pIncomingInvitationContext);
    
    if (SUCCEEDED(hr))
    {
        if (!QueueUserWorkItem(ProcessInviteResponse, (PVOID) pInvitationContext, 0))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            MessageBox(NULL, L"Error", L"QueueUserWorkItem failed", 1);
        }
    }

    return hr;
}
