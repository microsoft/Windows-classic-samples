/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) 1985-2007 Microsoft Corporation. All Rights Reserved.

Abstract:

    This C file includes sample code for sending an invitation
    with the Microsoft Peer-to-Peer Collaboration APIs.

Feedback:
    If you have any questions or feedback, please contact us using
    any of the mechanisms below:

    Email: peerfb@microsoft.com
    Newsgroup: Microsoft.public.win32.programmer.networks
    Website: http://www.microsoft.com/p2p

Notes:
    * This peer to peer application requires global IPv6 connectivity.

    * This sample relies on having existing contacts who are actively
      signed in.  This sample does not offer the ability to add new
      contacts.  To do this, the Contacts samples may be run.

--********************************************************************/

#pragma warning(disable:4201)   // nameless struct/union

#include <p2p.h>
#include <stdio.h>
#include <strsafe.h>
#include <stdlib.h>
#include "Invite.h"
#include "Shared.h"


// Note: Every application should create a new guid for their applications.
//       Never copy and paste the guid below for use in your own application.
//
GUID SampleAppGuid = {0x12341234, 0x1234, 0x1234, {0x12,0x34,0x12,0x34,0x12,0x34,0x12,0x34}};


//-----------------------------------------------------------------------------
// Function:   RegisterApplication
// Purpose:    Register the sample application for the current user
// Parameters:
//
void RegisterApplication()
{
    WCHAR wzPath[MAX_PATH];
    HRESULT hr = S_OK;
    PEER_APPLICATION_REGISTRATION_INFO RegInfo = {0};

    // Prompt for which application to register
    //
    hr = GetModuleFileName(NULL, wzPath, MAX_PATH);

    if (FAILED(hr))
    {
        //abort
        goto exit;
    }

    // Setup the application registration
    //
    RegInfo.application.id = SampleAppGuid;
    RegInfo.application.pwzDescription = L"Collaboration SDK Application";
    RegInfo.pwzApplicationArguments = L"/invite"; //we want to be passed this command line argument when launched
    RegInfo.pwzApplicationToLaunch = wzPath;
    RegInfo.dwPublicationScope = PEER_PUBLICATION_SCOPE_INTERNET;  //Internet scope is the only valid value for this member

    // Register the app as a new application for all users
    //
    hr = PeerCollabRegisterApplication(&RegInfo, PEER_APPLICATION_CURRENT_USER);

    if (SUCCEEDED(hr))
    {
        wprintf(L"Successfully registered application\n");
    }
    else
    {
        wprintf(L"Failed to register application, likely already registered. HRESULT=0x%x\n", hr);
        PrintError(hr);
    }

    hr = S_OK;

exit:
    return;
}


//-----------------------------------------------------------------------------
// Function:   UnregisterApplication
// Purpose:    Unregister the sample application for the current user
// Parameters: None
//
void UnregisterApplication()
{
    HRESULT hr = S_OK;

    // Unregister the sample app as a new application for all users
    //
    hr = PeerCollabUnregisterApplication(&SampleAppGuid, PEER_APPLICATION_CURRENT_USER);

    if (SUCCEEDED(hr))
    {
        wprintf(L"Successfully unregistered application\n");
    }
    else
    {
        wprintf(L"Failed to unregister application. HRESULT=0x%x\n", hr);
        PrintError(hr);
    }
}

//----------------------------------------------------------------------------
// Function:   DisplayLocalApplicationInfo
// Purpose:    Prints out locally registered applications
// Parameters:
//   pApplication : [in] pointer to PEER_APPLICATION_REGISTRATION_INFO object
//
void DisplayLocalApplicationInfo(__in PCPEER_APPLICATION_REGISTRATION_INFO pApplication)
{
    wprintf(L"Application:\n");

    wprintf(L"\tId: ");
    PrintGUID(&pApplication->application.id);

    if (pApplication->application.pwzDescription != NULL)
    {
        wprintf(L"\tDescription: %s\n", pApplication->application.pwzDescription);
    }

    if (pApplication->pwzApplicationToLaunch != NULL)
    {
        wprintf(L"\tApplication Path: %s\n", pApplication->pwzApplicationToLaunch);
    }

    if (pApplication->pwzApplicationArguments != NULL)
    {
        wprintf(L"\tApplication Arguments: %s\n", pApplication->pwzApplicationArguments);
    }

    wprintf(L"\tPublication Scope: %s\n", PublicationScope(pApplication->dwPublicationScope));

    
}

//-----------------------------------------------------------------------------
// Function:    DisplayApplicationRegistrations
// Purpose:     Retrieves and displays locally registered applications
// Parameters:  None
//
void DisplayApplicationRegistrations()
{
    PPEER_APPLICATION_REGISTRATION_INFO*  ppApplications = NULL;
    HPEERENUM           hApplicationEnum = NULL;
    ULONG               cApplications = 0;
    ULONG               i = 0;
    HRESULT             hr = S_OK;
    PPEER_APPLICATION_REGISTRATION_INFO pRegInfo = NULL;


    // Get a list of locally registered applications for the current user
    //
    wprintf(L"Enumerating all locally registered applications for the current user:\n");

    hr = PeerCollabEnumApplicationRegistrationInfo(PEER_APPLICATION_CURRENT_USER, &hApplicationEnum);

    IF_FAILED_PRINT_ERROR_AND_EXIT(hr, L"Error retrieving application info.");

    hr = PeerGetItemCount(hApplicationEnum, &cApplications);

    IF_FAILED_PRINT_ERROR_AND_EXIT(hr, L"Error retrieving item count.");

    if (cApplications == 0)
    {
        wprintf(L"There are no locally registered applications for the current user.\n");
    }
    else
    {
        hr = PeerGetNextItem(hApplicationEnum, &cApplications, (PVOID **) &ppApplications);

        IF_FAILED_PRINT_ERROR_AND_EXIT(hr, L"Error retrieving application info.");

        for (i = 0; i < cApplications; i++)
        {
            DisplayLocalApplicationInfo(ppApplications[i]);
            printf("\n");
        }
    }
    SAFE_PEER_END_ENUMERATION(hApplicationEnum);

    // Get a list of locally registgered applications for all users
    //
    wprintf(L"\nEnumerating all locally registered applications for all users:\n");

    hr = PeerCollabEnumApplicationRegistrationInfo(PEER_APPLICATION_ALL_USERS, &hApplicationEnum);

    IF_FAILED_PRINT_ERROR_AND_EXIT(hr, L"Error retrieving application info.");

    hr = PeerGetItemCount(hApplicationEnum, &cApplications);

    IF_FAILED_PRINT_ERROR_AND_EXIT(hr, L"Error retrieving item count.");

    if (cApplications == 0)
    {
        wprintf(L"There are no locally registered applications for all users.\n");
    }
    else
    {
        hr = PeerGetNextItem(hApplicationEnum, &cApplications, (PVOID **) &ppApplications);

        IF_FAILED_PRINT_ERROR_AND_EXIT(hr, L"Error retrieving application info.");

        for (i = 0; i < cApplications; i++)
        {
            DisplayLocalApplicationInfo(ppApplications[i]);
            printf("\n");
        }
    }

    SAFE_PEER_END_ENUMERATION(hApplicationEnum);

    wprintf(L"\nDisplaying registration for Collaboration Application:\n");

    hr = PeerCollabGetApplicationRegistrationInfo(&SampleAppGuid, PEER_APPLICATION_CURRENT_USER, &pRegInfo);

    if (SUCCEEDED(hr))
    {
        DisplayLocalApplicationInfo(pRegInfo);
    }
    else if (hr == PEER_E_NOT_FOUND)
    {
        wprintf(L"  Collaboration SDK Sample application is not registered\n");
    }
    else
    {
        wprintf(L"  Failed to get Application Registration Info. HRESULT=0x%x\n", hr);
        PrintError(hr);
    }

exit:
    SAFE_PEER_END_ENUMERATION(hApplicationEnum);
    SAFE_PEER_FREE_DATA(pRegInfo);
}

//-----------------------------------------------------------------------------
// Function:   SendAsyncInvite
// Purpose:    Utility routine to send an asynchronous invitation.
//             The routine will block on an event for LONG_TIMEOUT ms.  If the invitation is not
//             responded to (event signalled) within that time the invitation will be cancelled
// Parameters:
//   ppContact     :  [in] An array of pointers to PEER_CONTACT structures to send inviations to.
//                         The length of the array is uNumEndpoints.  Any element
//                         of the array may be NULL, if so that invite is directed
//                         to the endpoint only and there is no guarantee  that the recipient of the
//                         invite is the specific contact that the user intended.  If an elements is non-
//                         null the invitation is sent to a specific contact on a specific endpoint.
//   ppEndpoint    :  [in] An array of pointers to PEER_ENDPOINT structures to send an invitations to.
//                         The length of the array is uNumEndpoints.  
//   pInvite       :  [in] The invitation
//   uNumEndpoints :  [in] The number of endpoints in the arrays
//
HRESULT SendAsyncInvite(__in const PEER_CONTACT **ppContact,
                        __in const PEER_ENDPOINT **ppEndpoint,
                        __in const PEER_INVITATION *pInvite,
                        __in const unsigned int uNumEndpoints)
{
    HANDLE *phInvite = NULL;
    HANDLE hEvent = NULL;
    HRESULT hr = S_OK;
    DWORD dwWait = 0;
    unsigned int uCurrentEndpoint = 0;
    BOOL fInvitationsOutstanding = FALSE;
    DWORD dwWaitMS = 0;
    ULONGLONG ullWaitStartTickCount = 0;

    if (ppContact == NULL || ppContact == NULL || pInvite == NULL || uNumEndpoints == 0)
    {
        return E_INVALIDARG;
    }

    //Although we many be sending multiple invitations we will only use
    //the one notification handle (indicating that at least one
    //invitation has changed state).
    hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (hEvent == NULL)
        return E_OUTOFMEMORY;

    //Allocate memory for as many invitation handles as we need
    phInvite = malloc(sizeof(HANDLE) * uNumEndpoints);
    if (phInvite == NULL)
    {
        printf("Error alocating memory in SendAsyncInvite.\n");
        goto exit;
    }

    //Loop through and asynchronously send invitations to either a specific contant/endpoint pair or
    //just to an endpoint.
    for (uCurrentEndpoint=0; uCurrentEndpoint < uNumEndpoints; uCurrentEndpoint++)
    {
        if (ppContact[uCurrentEndpoint] != NULL)
        {
            // Send an async invite to a specific trusted contact on a specific endpoint
            //
            hr = PeerCollabAsyncInviteContact(ppContact[uCurrentEndpoint], ppEndpoint[uCurrentEndpoint], pInvite, hEvent, &(phInvite[uCurrentEndpoint]));
        }
        else
        {
            // Send an async invite to whoever is on a specific endpoint
            //
            hr = PeerCollabAsyncInviteEndpoint(ppEndpoint[uCurrentEndpoint], pInvite, hEvent, &(phInvite[uCurrentEndpoint]));
        }

        IF_FAILED_PRINT_ERROR_AND_EXIT(hr, L"Sending invite failed.");
    }

    // For simplicity we block and wait on the handle.  More sophisticated applications
    // would continue processing their message loops, and/or do this work in another thread.
    //
    // In this example we have used one handle, hEvent, to signal that at least one of the invitations
    // has changed state.  You could also use seperate handles and WaitForMultpleObjects to wait on
    // up to MAXIMUM_WAIT_OBJECTS handles (MAXIMUM_WAIT_OBJECTS == 64).
    //
    fInvitationsOutstanding = TRUE;
    ullWaitStartTickCount = GetTickCount64();
    while (fInvitationsOutstanding != FALSE)
    {
        //Calculate hwo much longer we want to wait for invitation
        //responses and wait on hEvent
        ULONGLONG ullCurrentTickCount = GetTickCount64();
        if (ullCurrentTickCount >= ullWaitStartTickCount+LONG_TIMEOUT)
        {
            dwWait = 0;
        }
        else
        {
            dwWaitMS = (DWORD)(LONG_TIMEOUT - (ullCurrentTickCount - ullWaitStartTickCount));
        }

        wprintf(L"Waiting for %d seconds on hEvent for Invitation response.\n", dwWaitMS/1000);
        wprintf(L"Invitation will be cancelled if not responded to within %d seconds.\n", dwWaitMS/1000);
        dwWait = WaitForSingleObject(hEvent, dwWaitMS);

        //Enumerate through the phInvite array
        //and get the status of each invitation
        //Set fInvitationsOutstanding to TRUE if PeerCollabGetInvitationResponse
        //returns PEER_E_INVITE_RESPONSE_NOT_AVAILABLE for any invitation
        //

        if (dwWait == WAIT_OBJECT_0)
        {
            // Event has been signalled indicating that at least one of the invitiations
            // now has a resposne.
            //
            fInvitationsOutstanding = PrintInvitationStatus(phInvite, uNumEndpoints, FALSE);
        }
        else if (dwWait == WAIT_TIMEOUT)
        {
            //Print the final invitation state and cancel any that are still outstanding
            (void)PrintInvitationStatus(phInvite, uNumEndpoints, TRUE);
            fInvitationsOutstanding = FALSE; //unconditionally exit the loop
        }
        else
        {
            wprintf(L"Wait returned unexpected value.");
            fInvitationsOutstanding = FALSE; //unconditionally exit the loop
        }
    }

exit:
    if (phInvite)
    {
        for (uCurrentEndpoint=0; uCurrentEndpoint < uNumEndpoints; uCurrentEndpoint++)
        {
            PeerCollabCloseHandle(phInvite[uCurrentEndpoint]);
        }

        SAFE_FREE(phInvite);
    }

    if (hEvent)
    {
        CloseHandle(hEvent);
    }
    return hr;
}

//-----------------------------------------------------------------------------
// Function:   SendSyncInvite
// Purpose:    Utility routine to send a synchronous invitation.
//             The routine will block until the API call returns.
// Parameters:
//   pContact   :  [in] contact to send an invite to, may be NULL, if so the invite is directed
//                      to the enpoint only and there is no guarantee  that the recipient of the
//                      invite is the specific contact that the user intended to send the invite to
//   pEndpoint  :  [in] endpoint to send an invite to
//   pInvite    :  [in] the invitation
//
HRESULT SendSyncInvite(__in const PEER_CONTACT *pContact,
                       __in const PEER_ENDPOINT *pEndpoint,
                       __in const PEER_INVITATION *pInvite)
{
    HRESULT hr = S_OK;
    PPEER_INVITATION_RESPONSE pResponse = NULL;

    if (pContact)
    {
        // Send an async invite to a specific trusted contact on a specific endpoint
        //
        hr = PeerCollabInviteContact(pContact, pEndpoint, pInvite, &pResponse);
    }
    else
    {
        // Send an async invite to whoever is on a specific endpoint
        //
        hr = PeerCollabInviteEndpoint(pEndpoint, pInvite, &pResponse);
    }

    IF_FAILED_PRINT_ERROR_AND_EXIT(hr, L"Sending invite failed.");

    if (pResponse->action == PEER_INVITATION_RESPONSE_ACCEPTED)
        wprintf(L"Invitation accepted.\n");
    else
        wprintf(L"Invitation not accepted.\n");

exit:
    SAFE_PEER_FREE_DATA(pResponse);

    return hr;
}


//-----------------------------------------------------------------------------
// Function:   PromptIsSynchronous
// Purpose:    Prompts if the invite should be sent synchronously
// Parameters:
//   None
//
BOOL PromptIsSynchronous()
{
    HRESULT hr;
    WCHAR wzBuff[INPUT_BUFSIZE] = {0};

    wprintf(L"Send (1) Synchronous Invite, (2) Asynchronous Invite [1-2]: ");
    GET_PROMPT_RESPONSE(hr, wzBuff);

    if  (wcsncmp(wzBuff, L"1", INPUT_BUFSIZE) == 0)
    {
        return TRUE;
    }

exit:
    return FALSE;
}


//-----------------------------------------------------------------------------
// Function:   SendEndpointInvite
// Purpose:    Sends an invite to given contact and endpoint.  Will prompt the user
//             to determine if the invite should be sent synchronously or asynchronously
// Parameters:
//   ppContact     :  [in] An array of pointers to PEER_CONTACT structures to send inviations to.
//                         The length of the array is uNumEndpoints.  Any element
//                         of the array may be NULL, if so that invite is directed
//                         to the endpoint only and there is no guarantee  that the recipient of the
//                         invite is the specific contact that the user intended.  If an elements is non-
//                         null the invitation is sent to a specific contact on a specific endpoint.
//   ppEndpoint    :  [in] An array of pointers to PEER_ENDPOINT structures to send an invitations to.
//                         The length of the array is uNumEndpoints.  
//   fSynchronous  :  [in] FALSE to send asynchronous invitations.  TRUE to send synchronous invitations.
//                         If sending synchronous invitations uNumEndpoints must equal 1.
//   uNumEndpoints :  [in] The number of endpoints in the arrays
//
HRESULT SendEndpointInvite(__in const PEER_CONTACT **ppContact, __in const PEER_ENDPOINT **ppEndpoint, BOOL fSynchronous, unsigned int uNumEndpoints)
{
    HRESULT hr;
    BOOL fInvitationSent = FALSE;
    PEER_INVITATION InviteRequest = {0};
    WCHAR AppDataExample[] = L"I am extra data to send as part of invite";

    if (ppContact == NULL || ppEndpoint == NULL || uNumEndpoints == 0 || (fSynchronous != FALSE && uNumEndpoints != 1))
        return E_INVALIDARG;

    //Build the invitation
    InviteRequest.applicationId = SampleAppGuid;
    InviteRequest.applicationData.cbData = sizeof(AppDataExample);
    InviteRequest.applicationData.pbData = (PBYTE) AppDataExample;
    InviteRequest.pwzMessage = L"Let's run the sample app together!";

    // Send the invite
    //
    if (fSynchronous != FALSE)
    {
        wprintf(L"Sending synchronous invite\n");
        hr = SendSyncInvite(ppContact[0], ppEndpoint[0], &InviteRequest);
    }
    else
    {
        wprintf(L"Sending asynchronous invite\n");
        hr = SendAsyncInvite(ppContact, ppEndpoint, &InviteRequest, uNumEndpoints);
    }

    if (SUCCEEDED(hr))
    {
        fInvitationSent = TRUE;
    }


    if (!fInvitationSent)
    {
        wprintf(L"No invitations sent.\n");
    }

    return hr;
}

//-----------------------------------------------------------------------------
// Function:    SendInvite
// Purpose:     Allows the user to select an endpoint, with a trusted contact endpoint or
//              a PNM endoint and sends an invitation
// Parameters:  None
//
void SendInvite()
{
    BOOL fSynchronous = FALSE;
    HRESULT hr = S_OK;
    PEER_CONTACT  **ppContacts = NULL;
    PEER_ENDPOINT **ppEndpoint = NULL;
    WCHAR wzBuff[4];
    unsigned int uNumEndpoints = 0;
    unsigned int uCurrentEndpoint = 0;

    // Prompt for if we send the invite synchronously
    fSynchronous = PromptIsSynchronous();

    if (fSynchronous == FALSE)
    {
        //Since we are sending asynchronous invites
        //we can send multiple invites from the one thread.
        //
        //Prompt the user for how many people they want to invite
        wprintf(L"How many contacts or endpoints do you wish to invite [1-100]: ");
        GET_PROMPT_RESPONSE(hr, wzBuff);

        uNumEndpoints = _wtoi(wzBuff);
        if (uNumEndpoints < 1 || uNumEndpoints > 100)
        {
            wprintf(L"\nInvalid Selection\n");
            hr = E_FAIL;
            goto exit;
        }
    }
    else
    {
        //Only allow one invitatation to be sent
        uNumEndpoints = 1;
    }

    //Note:  The pointers written to ppContacts and ppEndpoint
    //by this function have been allocated by the,
    //sample application directly using malloc, not via the p2p library.
    //We must free them by calling free.  Do not called PeerFreeData
    hr = SelectMultipleEndpoints(uNumEndpoints, &ppContacts, &ppEndpoint);

    if (FAILED(hr))
    {
        goto exit;
    }

    //We have collected all the inputs from the user.  Send the invitations/
    hr = SendEndpointInvite(ppContacts, ppEndpoint, fSynchronous, uNumEndpoints);

exit:
    //Free ppContacts and ppEndpoint as requried by SelectMultipleEndpoints
    if (ppContacts)
    {
        for (uCurrentEndpoint = 0; uCurrentEndpoint < uNumEndpoints; uCurrentEndpoint++)
        {
            if (ppContacts[uCurrentEndpoint] != NULL)
            {
                SAFE_FREE(ppContacts[uCurrentEndpoint]);
            }
        }

        SAFE_FREE(ppContacts);        
    }

    if (ppEndpoint)
    {
        for (uCurrentEndpoint = 0; uCurrentEndpoint < uNumEndpoints; uCurrentEndpoint++)
        {
            if (ppEndpoint[uCurrentEndpoint] != NULL)
            {
                DeleteEndpointData(ppEndpoint[uCurrentEndpoint]);
                SAFE_FREE(ppEndpoint[uCurrentEndpoint]);
            }
        }

        SAFE_FREE(ppEndpoint);        
    }

    return;
}

//-----------------------------------------------------------------------------
// Function:    DisplayAppLauchInfo
// Purpose:     Displays the app launch info returned by PeerCollabGetAppLaunchInfo.
//              Intended to be called when this application is launched by the user accepting
//              a received peer collaboration invite
// Parameters:  None
//
void DisplayAppLauchInfo()
{
    HRESULT hr;
    PPEER_APP_LAUNCH_INFO pLaunchInfo;
    WCHAR wzBuff[INPUT_BUFSIZE];

    hr = PeerCollabGetAppLaunchInfo(&pLaunchInfo);

    IF_FAILED_PRINT_ERROR_AND_EXIT(hr, L"PeerCollabGetAppLaunchInfo failed.");

    wprintf(L"Application launched with /invite argument.  Displaying App Launch Info returned from PeerCollabGetAppLaunchInfo\n");
    PrintContact(pLaunchInfo->pContact);
    PrintEndpoint(pLaunchInfo->pEndpoint);
    PrintInvitation(pLaunchInfo->pInvitation);
    wprintf(L"\n");

    wprintf(L"Press <ENTER> to continue.\n");
    fflush(stdin);
    (void)StringCbGets(wzBuff, sizeof(wzBuff));

exit:

    SAFE_PEER_FREE_DATA(pLaunchInfo);
}
