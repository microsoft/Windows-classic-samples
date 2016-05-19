/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) 1985-2007 Microsoft Corporation. All Rights Reserved.

Abstract:
    This C file includes sample code for working with contacts using
    the Microsoft Peer-to-Peer Collaboration APIs.

Feedback:
    If you have any questions or feedback, please contact us using
    any of the mechanisms below:

    Email: peerfb@microsoft.com
    Newsgroup: Microsoft.public.win32.programmer.networks
    Website: http://www.microsoft.com/p2p

Note:
    This peer to peer application requires global IPv6 connectivity.

--********************************************************************/

#pragma warning(disable:4201)   // nameless struct/union

#include <p2p.h>
#include <stdio.h>
#include <strsafe.h>
#include "Events.h"
#include "shared.h"

//-----------------------------------------------------------------------------
// Function:    DisplayWatchListChanged
// Purpose:     Called to display a PPEER_EVENT_WATCHLIST_CHANGED_DATA structure
// Parameters:  
//   pWatchlistChangedData    [in] : pointer to a PPEER_EVENT_WATCHLIST_CHANGED_DATA structure
//
void DisplayWatchListChanged(const PEER_EVENT_WATCHLIST_CHANGED_DATA *pWatchlistChangedData)
{
    if (pWatchlistChangedData)
    {
        PrintContact(pWatchlistChangedData->pContact);
        wprintf(L"\tChange Type: %s\n", ChangeType(pWatchlistChangedData->changeType));
    }
}

//-----------------------------------------------------------------------------
// Function:    DisplayPresenceChanged
// Purpose:     Called to display a PPEER_EVENT_PRESENCE_CHANGED_DATA structure
// Parameters:  
//   pPresenceChangedData    [in] : pointer to a PPEER_EVENT_PRESENCE_CHANGED_DATA structure
//
void DisplayPresenceChanged(const PEER_EVENT_PRESENCE_CHANGED_DATA *pPresenceChangedData)
{
    if (pPresenceChangedData)
    { 
        PrintContact(pPresenceChangedData->pContact);
        PrintEndpoint(pPresenceChangedData->pEndpoint);
        wprintf(L"\tChange Type: %s\n", ChangeType(pPresenceChangedData->changeType));
        wprintf(L"\tPresence: ");
        PrintPresenceInformation(pPresenceChangedData->pPresenceInfo);
    }
}

//-----------------------------------------------------------------------------
// Function:    DisplayApplicationChanged
// Purpose:     Called to display a PPEER_EVENT_APPLICATION_CHANGED_DATA structure
// Parameters:  
//   pApplicationChangedData    [in] : pointer to a PPEER_EVENT_APPLICATION_CHANGED_DATA structure
//
void DisplayApplicationChanged(const PEER_EVENT_APPLICATION_CHANGED_DATA *pApplicationChangedData)
{
    if (pApplicationChangedData)
    { 
        PrintContact(pApplicationChangedData->pContact);
        PrintEndpoint(pApplicationChangedData->pEndpoint);
        wprintf(L"\tChange Type: %s\n", ChangeType(pApplicationChangedData->changeType));
        PrintApplication(pApplicationChangedData->pApplication);
    }
}

//-----------------------------------------------------------------------------
// Function:    DisplayObjectChanged
// Purpose:     Called to display a PPEER_EVENT_OBJECT_CHANGED_DATA structure
// Parameters:  
//   pObjectChangedData    [in] : pointer to a PPEER_EVENT_OBJECT_CHANGED_DATA structure
//
void DisplayObjectChanged(const PEER_EVENT_OBJECT_CHANGED_DATA *pObjectChangedData)
{
    if (pObjectChangedData)
    { 
        PrintContact(pObjectChangedData->pContact);
        PrintEndpoint(pObjectChangedData->pEndpoint);
        wprintf(L"\tChange Type: %s\n", ChangeType(pObjectChangedData->changeType));
        PrintObject(pObjectChangedData->pObject);
    }
}


//-----------------------------------------------------------------------------
// Function:    DisplayEndpointChanged
// Purpose:     Called to display a PPEER_EVENT_ENDPOINT_CHANGED_DATA structure
// Parameters:  
//   pEndpointChangedData    [in] : pointer to a PPEER_EVENT_ENDPOINT_CHANGED_DATA structure
//
VOID DisplayEndpointChanged(const PEER_EVENT_ENDPOINT_CHANGED_DATA *pEndpointChangedData)
{
    if (pEndpointChangedData)
    {        
        PrintContact(pEndpointChangedData->pContact);
        PrintEndpoint(pEndpointChangedData->pEndpoint);
    }
}


//-----------------------------------------------------------------------------
// Function:    DisplayPeopleNearMeChanged
// Purpose:     Called to display a PPEER_EVENT_PEOPLE_NEAR_ME_CHANGED_DATA structure
// Parameters:  
//   pPNMChangedData    [in] : pointer to a PPEER_EVENT_PEOPLE_NEAR_ME_CHANGED_DATA structure
//
VOID CALLBACK DisplayPeopleNearMeChanged(const PEER_EVENT_PEOPLE_NEAR_ME_CHANGED_DATA *pPNMChangedData)
{
    if (pPNMChangedData)
    {       
        wprintf(L"\tChange Type: %s\n", ChangeType(pPNMChangedData->changeType));
        if (pPNMChangedData->pPeopleNearMe == NULL)
        {
            wprintf(L"\t  Endpoint: <Me>\n");
        }
        else
        {
            PrintEndpoint(&pPNMChangedData->pPeopleNearMe->endpoint);
            wprintf(L"\t  NickName: %s\n", pPNMChangedData->pPeopleNearMe->pwzNickName);
        }
    }
}

//-----------------------------------------------------------------------------
// Function:    DisplayRequestStatusChanged
// Purpose:     Called to display a PPEER_EVENT_REQUEST_STATUS_CHANGED_DATA structure
// Parameters:  
//   pRequestStatusChangedData    [in] : pointer to a PPEER_EVENT_REQUEST_STATUS_CHANGED_DATA structure
//
VOID CALLBACK DisplayRequestStatusChanged(const PEER_EVENT_REQUEST_STATUS_CHANGED_DATA  *pRequestStatusChangedData)
{
    if (pRequestStatusChangedData)
    {     
        PrintFullEndpoint(pRequestStatusChangedData->pEndpoint, TRUE);
        if (SUCCEEDED(pRequestStatusChangedData->hrChange))
        {
            wprintf(L"Request Status changed event indicates success.\n");
        }
        else
        {
            wprintf(L"Request Status changed event indicates failure. HRESULT=0x%x.\n",
                    pRequestStatusChangedData->hrChange);
            PrintError(pRequestStatusChangedData->hrChange);
        }
    }
}

//-----------------------------------------------------------------------------
// Function:    PNMEventHandler
// Purpose:     Registers for all event types and prints out the event data
// Parameters:  None
//
HRESULT EventHandler()
{
    HPEEREVENT            hPeerEvent = NULL;
    HRESULT               hr = S_OK;
    PEER_COLLAB_EVENT_REGISTRATION  eventReg[11] = {0};
    HANDLE hEvent[2];
    HANDLE hParentHandle = NULL;

    hr = GetParentProcessHandle(&hParentHandle);

    if (FAILED(hr))
    {
        printf("Failed to get parent process handle.  Aborting event printing process\n");
        hr = E_FAIL;
        goto exit;
    }

    hEvent[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
    hEvent[1] = hParentHandle;

    eventReg[0].eventType = PEER_EVENT_WATCHLIST_CHANGED;
    eventReg[1].eventType = PEER_EVENT_ENDPOINT_CHANGED;
    eventReg[2].eventType = PEER_EVENT_ENDPOINT_PRESENCE_CHANGED;
    eventReg[3].eventType = PEER_EVENT_ENDPOINT_APPLICATION_CHANGED;
    eventReg[4].eventType = PEER_EVENT_ENDPOINT_OBJECT_CHANGED;
    eventReg[5].eventType = PEER_EVENT_MY_ENDPOINT_CHANGED;
    eventReg[6].eventType = PEER_EVENT_MY_PRESENCE_CHANGED;
    eventReg[7].eventType = PEER_EVENT_MY_APPLICATION_CHANGED;
    eventReg[8].eventType = PEER_EVENT_MY_OBJECT_CHANGED;
    eventReg[9].eventType = PEER_EVENT_PEOPLE_NEAR_ME_CHANGED;
    eventReg[10].eventType = PEER_EVENT_REQUEST_STATUS_CHANGED;

    // Register to be notified when the request finishes  
    hr = PeerCollabRegisterEvent(hEvent[0], celems(eventReg), eventReg, &hPeerEvent);  

    if (SUCCEEDED(hr))
    {
        while (WaitForMultipleObjects(2, hEvent, FALSE, INFINITE) == WAIT_OBJECT_0)
        {
            PEER_COLLAB_EVENT_DATA *pEventData = NULL;

            // retrieve all event data 
            while (PeerCollabGetEventData(hPeerEvent, &pEventData) == S_OK)
            {
                switch(pEventData->eventType)
                {
                    case PEER_EVENT_WATCHLIST_CHANGED:
                        wprintf(L"PEER_EVENT_WATCHLIST_CHANGED event signalled\n");
                        break;

                    case PEER_EVENT_ENDPOINT_CHANGED:
                        wprintf(L"PEER_EVENT_ENDPOINT_CHANGED\n");
                        break;

                    case PEER_EVENT_ENDPOINT_PRESENCE_CHANGED:
                        wprintf(L"PEER_EVENT_ENDPOINT_PRESENCE_CHANGED event signalled\n");
                        break;

                    case PEER_EVENT_ENDPOINT_APPLICATION_CHANGED:
                        wprintf(L"PEER_EVENT_ENDPOINT_APPLICATION_CHANGED event signalled\n");
                        break;

                    case PEER_EVENT_ENDPOINT_OBJECT_CHANGED:
                        wprintf(L"PEER_EVENT_ENDPOINT_OBJECT_CHANGED event signalled\n");
                        break;

                    case PEER_EVENT_MY_ENDPOINT_CHANGED:
                        wprintf(L"PEER_EVENT_MY_ENDPOINT_CHANGED event signalled\n");
                        break;

                    case PEER_EVENT_MY_PRESENCE_CHANGED:
                        wprintf(L"PEER_EVENT_MY_PRESENCE_CHANGED event signalled\n");
                        break;

                    case PEER_EVENT_MY_APPLICATION_CHANGED:
                        wprintf(L"PEER_EVENT_MY_APPLICATION_CHANGED event signalled\n");
                        break;

                    case PEER_EVENT_MY_OBJECT_CHANGED:
                        wprintf(L"PEER_EVENT_MY_OBJECT_CHANGED event signalled\n");
                        break;

                    case PEER_EVENT_PEOPLE_NEAR_ME_CHANGED:
                        wprintf(L"PEER_EVENT_PEOPLE_NEAR_ME_CHANGED event signalled\n");
                        break;

                    case PEER_EVENT_REQUEST_STATUS_CHANGED:
                        wprintf(L"PEER_EVENT_REQUEST_STATUS_CHANGED event signalled\n");
                        break;
                }

                switch (pEventData->eventType)
                {
                    case PEER_EVENT_WATCHLIST_CHANGED:
                              DisplayWatchListChanged(&pEventData->watchListChangedData);
                              break;
                    case PEER_EVENT_ENDPOINT_PRESENCE_CHANGED:  //intentional fallthrough
                        case PEER_EVENT_MY_PRESENCE_CHANGED:
                              DisplayPresenceChanged(&pEventData->presenceChangedData);
                              break;
                    case PEER_EVENT_ENDPOINT_APPLICATION_CHANGED:  //intentional fallthrough
                    case PEER_EVENT_MY_APPLICATION_CHANGED:
                              DisplayApplicationChanged(&pEventData->applicationChangedData);
                              break;
                    case PEER_EVENT_ENDPOINT_OBJECT_CHANGED:  //intentional fallthrough
                    case PEER_EVENT_MY_OBJECT_CHANGED:
                              DisplayObjectChanged(&pEventData->objectChangedData);
                              break;
                    case PEER_EVENT_ENDPOINT_CHANGED:  //intentional fallthrough
                    case PEER_EVENT_MY_ENDPOINT_CHANGED:
                              DisplayEndpointChanged(&pEventData->endpointChangedData);
                              break;
                    case PEER_EVENT_PEOPLE_NEAR_ME_CHANGED:
                              DisplayPeopleNearMeChanged(&pEventData->peopleNearMeChangedData);
                              break;
                    case PEER_EVENT_REQUEST_STATUS_CHANGED:
                        DisplayRequestStatusChanged(&pEventData->requestStatusChangedData);
                        break;
                    default: //do nothing
                        break;
                }

                //Print a seperator line
                wprintf(L"--------------------------------\n");

                SAFE_PEER_FREE_DATA(pEventData);
            }
        }
    }
    else
    {
        wprintf(L"PeerCollabRegisterEvent failed.\nHRESULT=0x%x\n", hr);
        PrintError(hr);
    }

    hr = S_OK;

exit:

    if (hPeerEvent != NULL)
    {
        (void) PeerCollabUnregisterEvent(hPeerEvent);
        hPeerEvent = NULL;
    }

    if (hEvent[0] != NULL)
    {
        CloseHandle(hEvent[0]);
        hEvent[0] = NULL;
    }

    if (hEvent[1] != NULL)
    {
        CloseHandle(hEvent[1]);
        hEvent[1] = NULL;
    }

    return hr; 
}
