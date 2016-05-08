/**********************************************************************
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All Rights Reserved.

Module Name:

    PeoplePickerModel.c

Abstract:

    This C file includes sample code for the model associated with the
    People Picker dialog.

Feedback:
    If you have any questions or feedback, please contact us using 
    any of the mechanisms below:

    Email: peerfb@microsoft.com 
    Newsgroup: Microsoft.public.win32.programmer.networks 
    Website: http://www.microsoft.com/p2p 

--********************************************************************/

#pragma warning(disable:4201)   // nameless struct/union

#include "PeoplePickerModel.h"

// Forward Declarations
VOID CALLBACK ProcessUpdateCallBack(LPVOID lpContext, BOOLEAN fTimer);
HRESULT MonitorPeopleNearMe();
HRESULT EnumPeopleNearMe();

// Variables
static HPEEREVENT          g_hModelPeerEvent = NULL;
static HANDLE              g_hModelWaitObject = NULL;
static HANDLE              g_hModelEvent = NULL;
static HWND                   g_hModelDlgOwner = NULL;

//-----------------------------------------------------------------------------------
// Function: InitPeoplePickerDataModel
//
// Purpose:  Initializes the people picker data model and associates with a dialog
//
// Returns:  HRESULT
//
HRESULT InitPeoplePickerModel(HWND hDlg)
{
    HRESULT hr = S_OK;

    g_hModelDlgOwner = hDlg;

    // Start to monitor people near me
    //
    hr = MonitorPeopleNearMe();

    if (SUCCEEDED(hr))
    {
        // Enumerate the people near me 
        //
        hr = EnumPeopleNearMe();
        return hr;
    }
    
    MessageBox(g_hModelDlgOwner, L"Error", L"Error monitoring PNM", MB_OK);

    return hr;
}

//-----------------------------------------------------------------------------
// Function:    DuplicatePeerPeopleNearMe
//
// Purpose:     Duplicates a PEER_PEOPLE_NEAR_ME structure
//              It allocates a copy, so the caller needs to free
//
// Returns:        HRESULT
//
HRESULT DuplicatePeerPeopleNearMe(PEER_PEOPLE_NEAR_ME ** ppPersonDestination, PEER_PEOPLE_NEAR_ME * pPersonSource)
{
    size_t cbNickName = 0;
    size_t cbEndpointName = 0;
    PEER_PEOPLE_NEAR_ME * pTempDestination = NULL;
    
    //Get size of NickName
    //
    cbNickName = (wcslen(pPersonSource->pwzNickName) * sizeof(WCHAR)) + sizeof(WCHAR);
    cbEndpointName = (wcslen(pPersonSource->endpoint.pwzEndpointName) * sizeof(WCHAR)) + sizeof(WCHAR);

    //Allocate nick name and PEER_PEOPLE_NEAR_ME structure
    //
    pTempDestination = (PEER_PEOPLE_NEAR_ME *) malloc(sizeof(PEER_PEOPLE_NEAR_ME));
    
    if (NULL == pTempDestination)
        return E_OUTOFMEMORY;
    
    ZeroMemory(pTempDestination, sizeof(PEER_PEOPLE_NEAR_ME));

    pTempDestination->pwzNickName = (PWSTR) malloc(cbNickName);

    if (NULL == pTempDestination->pwzNickName)
    {
        // Free the allocated memory if a Nickname cannot be allocated
        //
        PeoplePickerModelFreePerson(pTempDestination);
        return E_OUTOFMEMORY;
    }

    // Assigning the structure, then pointing to newly allocated memory
    // Need to do the structure copy first before the allocation, otherwise we overwrite our pointer
    //
    pTempDestination->endpoint = pPersonSource->endpoint;
    pTempDestination->endpoint.pwzEndpointName = (PWSTR) malloc(cbEndpointName);

    if (NULL == pTempDestination->endpoint.pwzEndpointName)
    {
        // Free the allocated memory if a endpoint name cannot be allocated
        //
        PeoplePickerModelFreePerson(pTempDestination);
        return E_OUTOFMEMORY;
    }
    
    //Copy the relevant fields into the PEER_PEOPLE_NEAR_ME structure
    //
    memcpy(pTempDestination->endpoint.pwzEndpointName, pPersonSource->endpoint.pwzEndpointName, cbEndpointName);
    pTempDestination->id = pPersonSource->id;
    memcpy(pTempDestination->pwzNickName, pPersonSource->pwzNickName, cbNickName);

    *ppPersonDestination = pTempDestination;

    return S_OK;
}

//-----------------------------------------------------------------------------
// Function:    PeoplePickerModelFreePerson
//
// Purpose:     Frees a PEER_PEOPLE_NEAR_ME structure allocated by the model
//              
// Returns:        VOID
//
VOID PeoplePickerModelFreePerson(PEER_PEOPLE_NEAR_ME * pPersonNearMe)
{
    if (NULL != pPersonNearMe)
    {
        free(pPersonNearMe->pwzNickName);
        free(pPersonNearMe->endpoint.pwzEndpointName);
        free(pPersonNearMe);
    }
}

//-----------------------------------------------------------------------------
// Function:    PeoplePickerModelDestroy
//
// Purpose:     Frees up all the resources needed by the model
//              
// Returns:        VOID
//
VOID PeoplePickerModelDestroy()
{
    UnregisterWaitEx(g_hModelWaitObject,INVALID_HANDLE_VALUE);
    PeerCollabUnregisterEvent(g_hModelPeerEvent);
    CloseHandle(g_hModelEvent);
}

//-----------------------------------------------------------------------------
// Function:    ProcessUpdateCallBack
//
// Purpose:     Called when PEER_EVENT_PEOPLE_NEAR_ME_CHANGED event is received
//              Reads in event data and displays the type of change
//
// Returns:     VOID
//
VOID CALLBACK ProcessUpdateCallBack(LPVOID lpContext, BOOLEAN fTimer)
{
    PEER_COLLAB_EVENT_DATA * pEventData = NULL;
    PEER_PEOPLE_NEAR_ME * pPersonNearMe = NULL;

    //Unreferenced parameters
    lpContext;
    fTimer;

    while (SUCCEEDED(PeerCollabGetEventData(g_hModelPeerEvent, &pEventData)) && pEventData)
    {   
        // NULL for the PPEER_PEOPLE_NEAR_ME means this event is for the local person
        //
        if (pEventData->peopleNearMeChangedData.pPeopleNearMe == NULL)
        {
            // PEER_CHANGED_ADDED on the local person means they have signed into people near me
            //
            if (pEventData->peopleNearMeChangedData.changeType == PEER_CHANGE_ADDED)
            {
                InitPeoplePickerModel(g_hModelDlgOwner);
            }

            // PEER_CHANGED_ADDED on the local person means they have signed out of people near me
            //
            if (pEventData->peopleNearMeChangedData.changeType == PEER_CHANGE_DELETED)
            {
                PostMessage(g_hModelDlgOwner, WM_CLEARPEOPLE, 0, 0);
            }
        }
        else
        {
            // Copy the person to give to dialog
            //
            if (SUCCEEDED(DuplicatePeerPeopleNearMe(&pPersonNearMe, pEventData->peopleNearMeChangedData.pPeopleNearMe)))
            {
                // The person was added (signed-in), so tell the dialog to add the person
                //
                if (pEventData->peopleNearMeChangedData.changeType == PEER_CHANGE_ADDED)
                {
                    PostMessage(g_hModelDlgOwner, WM_ADDPERSON, 0, (LPARAM) pPersonNearMe);
                }

                // The person was deleted (signed-out), so tell dialog to remove the person
                //
                if (pEventData->peopleNearMeChangedData.changeType == PEER_CHANGE_DELETED)
                {
                    PostMessage(g_hModelDlgOwner, WM_REMOVEPERSON, 0, (LPARAM) pPersonNearMe);
                }
            }
        }
        // Free the used PEER resources
        //
        PeerFreeData(pEventData);
    }
}


//-----------------------------------------------------------------------------
// Function:    MonitorPeopleNearMe
//
// Purpose:     Watches for change events in PeopleNearMe data
//
// Returns:     HRESULT
//
HRESULT MonitorPeopleNearMe()
{
    HRESULT hr = S_OK;
    PEER_COLLAB_EVENT_REGISTRATION  eventReg = {0};
   
    // Create the event handle for the model
    //
    g_hModelEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (NULL == g_hModelEvent)
    {
        return E_OUTOFMEMORY;
    }

    // Setup the event registration specifying the type of event to be notified about
    //
    eventReg.eventType = PEER_EVENT_PEOPLE_NEAR_ME_CHANGED;
    eventReg.pInstance = NULL;

    // Register to be notified when the People Near Me change
    //
    hr = PeerCollabRegisterEvent(g_hModelEvent, 1, &eventReg, &g_hModelPeerEvent);

    if (SUCCEEDED(hr))
    {
        // Registers a wait object that will call ProcessUpdateCallBack
        // whenever a PEER_EVENT_PEOPLE_NEAR_ME_CHANGED is received.
        //
        if (!RegisterWaitForSingleObject(&g_hModelWaitObject,
                                    g_hModelEvent, 
                                    ProcessUpdateCallBack,
                                    NULL,
                                    INFINITE,
                                    0))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if (FAILED(hr))
    {
        MessageBox(g_hModelDlgOwner, L"Error", L"Could not initialize People Near Me monitoring.", MB_OK);
    }

    return hr;
}

//-----------------------------------------------------------------------------
// Function:    EnumPeopleNearMe
//
// Purpose:     Simple wrapper function that calls PeerCollabEnumPeopleNearMe
//              and displays the results.  Setting fPrompt flag prompts user
//              to select an endpoint for further information.
//
// Returns:     HRESULT
//
HRESULT EnumPeopleNearMe()
{
    PEER_PEOPLE_NEAR_ME **   ppPeopleNearMe = NULL;
    PEER_PEOPLE_NEAR_ME *    pPersonNearMe = NULL;
    HRESULT                  hr = S_OK;
    HPEERENUM                hEnum = NULL;
    ULONG                    count = 0;
    ULONG                     index = 0;
 
    // Enumerate the people near me and setup the enumeration
    //
    hr = PeerCollabEnumPeopleNearMe(&hEnum);

    if (SUCCEEDED(hr))
    {
        hr = PeerGetItemCount(hEnum, &count);

        if (SUCCEEDED(hr))
        {
            if (count == 0)
            {
                PeerEndEnumeration(hEnum);
                return hr;
            }
            hr = PeerGetNextItem(hEnum, &count, (PVOID **) &ppPeopleNearMe);
        }

        if (SUCCEEDED(hr))
        {    
            // Add people to dialog
            //
            while (index < count)
            {
                if (SUCCEEDED(DuplicatePeerPeopleNearMe(&pPersonNearMe, ppPeopleNearMe[index])))
                {
                    //Add the person to the Listview
                    //
                    PostMessage(g_hModelDlgOwner, WM_ADDPERSON, 0, (LPARAM) pPersonNearMe);
                }
                index++;
            }
            // Free the used PEER resources
            //
            PeerFreeData(ppPeopleNearMe);
        }

        // End the PEER enumeration
        //
        PeerEndEnumeration(hEnum);     
    }
    return hr;
}

