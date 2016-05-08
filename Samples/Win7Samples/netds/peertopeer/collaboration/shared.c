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
#include <Tlhelp32.h>
#include "Shared.h"


GUID MESSAGE_GUID = { /* 191312ce-4466-4dc4-a8d6-cb2e9710157f */
    0x191312ce,
    0x4466,
    0x4dc4,
    {0xa8, 0xd6, 0xcb, 0x2e, 0x97, 0x10, 0x15, 0x7f}
  };

#define ROUND_UP_COUNT(Count,Pow2) \
        ( ((Count)+(Pow2)-1) & (~(((LONG)(Pow2))-1)) )

// If Ptr is not already aligned, then round it up until it is.
#define ROUND_UP_POINTER(Ptr,Pow2) \
        ( (LPVOID) ( (((ULONG_PTR)(Ptr))+(Pow2)-1) & (~(((LONG)(Pow2))-1)) ) )
// Usage: myPtr = ROUND_UP_POINTER( unalignedPtr, ALIGN_LPVOID )


#define ALIGN_LPVOID            sizeof(LPVOID)

//-----------------------------------------------------------------------------
// Function:    ChangeType
// Purpose:     Convert the enum PEER_CHANGE_TYPE to a string
// Parameters:  
//   eChangeType   :  [in] enum value to convert to a string
//
const WCHAR *ChangeType(__in const PEER_CHANGE_TYPE eChangeType)
{
    const static PWSTR rgEventType[] = 
    {
        L"Added",
        L"Deleted",
        L"Updated"
    };

    if (eChangeType >= celems(rgEventType))
    {
        return L"";
    }

    return rgEventType[eChangeType];
}

//-----------------------------------------------------------------------------
// Function:    ChangeType
// Purpose:     Convert the enum PEER_PUBLICATION_SCOPE to a string
// Parameters:  
//   ePublicationScope   :  [in] enum value to convert to a string
//
const WCHAR *PublicationScope(__in const PEER_PUBLICATION_SCOPE ePublicationScope)
{
    const static PWSTR rgPublicationScope[] =
    {
        L"None",
        L"Near Me",
        L"Internet",
        L"All"
    };

    if (ePublicationScope >= celems(rgPublicationScope))
    {
        return L"";
    }

    return rgPublicationScope[ePublicationScope];
}

//-----------------------------------------------------------------------------
// Function:    ChangeType
// Purpose:     Convert the enum PEER_PRESENCE_STATUS to a string
// Parameters:  
//   ePresenceStatus   :  [in] enum value to convert to a string
//
const WCHAR *PresenceStatus(__in const PEER_PRESENCE_STATUS ePresenceStatus)
{
    const static PWSTR rgPresence[] =
    {
        L"Offline",
        L"Out to lunch",
        L"Away",
        L"Be Right Back",
        L"Idle",
        L"Busy",
        L"On the phone",
        L"Online"
    };

    if (ePresenceStatus >= celems(rgPresence))
    {
        return L"";
    }

    return rgPresence[ePresenceStatus];
}

//-----------------------------------------------------------------------------
// Function:    PrintError
// Purpose:     Utility routine to print an error and the associated error string
// Parameters:  hr error obtained
//
void PrintError(HRESULT hrError)
{
    DWORD dwCch=0;
    WCHAR wszBuffer[ERRBUFSIZE] = {0};  

    if (HRESULT_FACILITY(hrError) == FACILITY_P2P)
    {
        HMODULE hResDll = GetModuleHandle(L"p2p.dll");
        if (NULL != hResDll)
        {
            dwCch = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE,
                                  hResDll,
                                  HRESULTTOWIN32(hrError),
                                  0,
                                  wszBuffer,
                                  ERRBUFSIZE,
                                  NULL);
            FreeLibrary(hResDll);
           
            if (dwCch > 0)
            {
                wprintf(L"Error Description: %s\n", wszBuffer);
            }
        }
    }
    else
    {
        dwCch = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                              NULL,
                              HRESULTTOWIN32(hrError),
                              0,
                              wszBuffer,
                              ERRBUFSIZE,
                              NULL);

        if (dwCch > 0)
        {
            wprintf(L"Error Description: %s\n", wszBuffer);
        }
    }
}  

//-----------------------------------------------------------------------------
// Function:   PrintInvitation
// Purpose:    Prints out a PEER_INVITATION structure to the console
// Parameters:
//   pInvitation : [in] pointer to PEER_INVITATION structure
//
void PrintInvitation(PCPEER_INVITATION pInvitation)
{
    PWSTR   pwzApplicationId = NULL;

    wprintf(L"\tApplication:\n");

    if (UuidToString(&pInvitation->applicationId, &pwzApplicationId) == RPC_S_OK)
    {
        wprintf(L"\t  Application Id: %s\n", pwzApplicationId);
        RpcStringFree(&pwzApplicationId);
    }

    if (pInvitation->pwzMessage  != NULL)
    {
        wprintf(L"\t  Message: %s\n", pInvitation->pwzMessage );
    }

    wprintf(L"\tApplication Data\n");
    wprintf(L"\t  Application Data Size: %d\n", pInvitation->applicationData.cbData);
    if (pInvitation->applicationData.cbData > 0)
    {
        wprintf(L"\t  Application Data: %.*s\n", pInvitation->applicationData.cbData, (WCHAR *)pInvitation->applicationData.pbData );
    }
}


//-----------------------------------------------------------------------------
// Function:    PrintEndpoint
// Purpose:     Prints out a PEER_ENDPOINT structure to the console
// Parameters:
//   pEndpoint : [in] pointer to PEER_ENDPOINT structure
//

void PrintEndpoint(PCPEER_ENDPOINT pEndpoint)
{
    PrintFullEndpoint(pEndpoint, FALSE);
}

void PrintFullEndpoint(PCPEER_ENDPOINT pEndpoint, BOOL fFull)
{
    HRESULT hr = S_OK;
    WCHAR wzAddr[256] = {0};
    DWORD dwLen = (sizeof(wzAddr) / sizeof(wzAddr[0]));
    PEER_PRESENCE_INFO* pPresInfo = NULL;

    if (pEndpoint != NULL)
    {
        wprintf(L"\tEndpoint:\n");

        // Print out address
        if (WSAAddressToString(
                (LPSOCKADDR) &(pEndpoint->address.sin6),
                sizeof(SOCKADDR_IN6),
                NULL,
                wzAddr,
                &dwLen) != SOCKET_ERROR)
        {
            wprintf(L"\t  IPV6 Endpoint Address: %s\n", wzAddr);
        }
        else
        {
            wprintf(L"\t  Error parsing address\n");
        }

        // Print out endpoint name
        wprintf(L"\t  Endpoint Name: %s\n", pEndpoint->pwzEndpointName);
    }
    else
    {
        wprintf(L"\tEndpoint\n");
        wprintf(L"\t  <Me>\n");
    }

    if (fFull != FALSE)
    {
        //Presence Info
        hr = PeerCollabGetPresenceInfo(pEndpoint, &pPresInfo);
        if (FAILED(hr))
        {
            wprintf(L"PeerCollabGetPresenceInfo failed, hr=0x%x\n", hr);
            PrintError(hr);
            goto exit;
        }

        // print out the presence information
        PrintPresenceInformation(pPresInfo);
    }

exit:
    // free the presence info struct that was allocated by the PeerCollabGetPresenceInfo API
    SAFE_PEER_FREE_DATA(pPresInfo);
}

//-----------------------------------------------------------------------------
// Function:    PrintContact
// Purpose:     Prints out a PEER_CONTACT structure to the console
// Parameters:
//   pContact : [in] pointer to PEER_CONTACT structure
//
void PrintContact(PCPEER_CONTACT pContact)
{
    if (pContact != NULL)
    {
        wprintf(L"\tContact\n");
        wprintf(L"\t  pwzPeerName       : %s\n", pContact->pwzPeerName);
        wprintf(L"\t  pwzNickName       : %s\n", pContact->pwzNickName);
        wprintf(L"\t  pwzDisplayName    : %s\n", pContact->pwzDisplayName);
        wprintf(L"\t  pwzEmailAddress   : %s\n", pContact->pwzEmailAddress);
        wprintf(L"\t  fWatch            : %s\n", (int)pContact->fWatch==0?L"FALSE":L"TRUE");
        wprintf(L"\t  WatcherPermissions: %s\n", (int)pContact->WatcherPermissions==PEER_WATCH_BLOCKED?L"PEER_WATCH_BLOCKED":L"PEER_WATCH_ALLOWED");
        wprintf(L"\t  credentials       : %d bytes\n", pContact->credentials.cbData);
    }
    else
    {
        wprintf(L"\tContact\n");
        wprintf(L"\t  <Me>\n");
    }
}

//-----------------------------------------------------------------------------
// Function:    PrintContacts
// Purpose:     Prints out an array of PEER_CONTACT structures to the console
// Parameters:
//    ppContacts: array of pointers to PEER_CONTACT structures
//    cContacts : count of PEER_CONTACT structures in ppContacts array
//
void PrintContacts(__in_ecount(cContacts) PCPEER_CONTACT *ppContacts, __in const ULONG cContacts)
{
    ULONG i;
    wprintf(L"Contacts:\n");

    for (i = 0; i < cContacts; i++)
    {
        wprintf(L"%d)\n", i+1);
        PrintContact(ppContacts[i]);
    }
}

//-----------------------------------------------------------------------------
// Function:   PrintObject
// Purpose:    Prints out a PEER_OBJECT structure to the console
// Parameters:
//   pObject : [in] pointer to PEER_CONTACT structure
//
void PrintObject(__in PCPEER_OBJECT pObject)
{
    PWSTR   pwzObjectId = NULL;
    
    wprintf(L"\tObject:\n");

    if (UuidToString(&pObject->id, &pwzObjectId) == RPC_S_OK)
    {
        wprintf(L"\t  Id: %s\n", pwzObjectId);
        RpcStringFree(&pwzObjectId);
    }

    wprintf(L"\t  Size: %d\n", pObject->data.cbData);

    // Displays contents of message object when object's GUID equals MESSAGE_GUID
    if (pObject->data.cbData != 0 &&
        memcmp(&(pObject->id), &(MESSAGE_GUID), sizeof(pObject->id)) == 0)
    {
        wprintf(L"\t  Data: %s\n", (PWSTR) pObject->data.pbData);
    }

    wprintf(L"\t  Publication Scope: %s\n", PublicationScope(pObject->dwPublicationScope));
}

//-----------------------------------------------------------------------------
// Function:   PrintApplication
// Purpose:    Prints out a PEER_APPLICATION structure to the console
// Parameters:
//   pApplication : [in] pointer to PEER_APPLICATION structure
//
void PrintApplication(__in PCPEER_APPLICATION pApplication)
{
    PWSTR   pwzApplicationId = NULL;

    wprintf(L"\tApplication:\n");

    if (UuidToString(&pApplication->id, &pwzApplicationId) == RPC_S_OK)
    {
        wprintf(L"\t  Id: %s\n", pwzApplicationId);
        RpcStringFree(&pwzApplicationId);
    }

    if (pApplication->pwzDescription != NULL)
    {
        wprintf(L"\t  Description: %s\n", pApplication->pwzDescription);
    }

    wprintf(L"\t  Data Size: %d\n\n", pApplication->data.cbData);

}

//-----------------------------------------------------------------------------
// Function:   PrintPeopleNearMeInfo
// Purpose:    Prints out an array of PEER_PEOPLE_NEAR_ME structures to the console
// Parameters:
//    ppPeopleNearMe: array of pointers to PEER_PEOPLE_NEAR_ME structures
//    cContacts : count of PEER_PEOPLE_NEAR_ME structures in ppPeopleNearMe array
//
void PrintPeopleNearMeInfo(__in_ecount(cContacts) PCPEER_PEOPLE_NEAR_ME *ppPeopleNearMe, __in const ULONG cPeopleNearMe)
{
    ULONG i = 0;

    wprintf(L"\nPeople Near Me: \n");
    for (i = 0; i < cPeopleNearMe; i++)
    {        
        // Print out nickname
        wprintf(L"%d) %s, ", i+1, ppPeopleNearMe[i]->pwzNickName);

        PrintEndpoint(&ppPeopleNearMe[i]->endpoint);
    }
}

//-----------------------------------------------------------------------------
// Function:    PrintPresenceInformation
// Purpose:     Prints out a PCPEER_PRESENCE_INFO structure to the console
// Parameters:
//   pPresInfo : [in] pointer to PCPEER_PRESENCE_INFO structure
//
void PrintPresenceInformation(__in PCPEER_PRESENCE_INFO pPresInfo)
{

    // first print the text equivalent of the status
    wprintf(L"%s", PresenceStatus(pPresInfo->status));
    
    // now print any descriptive text 
    // (for example, the user might set status to "Playing Halo")
    wprintf(L" -  %s\n", pPresInfo->pwzDescriptiveText == NULL ? L"" :
                       pPresInfo->pwzDescriptiveText);
}

//-----------------------------------------------------------------------------
// Function:    PrintGUID
// Purpose:     Prints out a GUID to the console
// Parameters:
//   pguid : [in] pointer to GUID
//
void PrintGUID(__in const GUID *pguid)
{
    PWSTR   pwzApplicationId = NULL;

    if (pguid != NULL)
    {
        if (UuidToString(pguid, &pwzApplicationId) == RPC_S_OK)
        {
            wprintf(L"%s\n", pwzApplicationId);
            RpcStringFree(&pwzApplicationId);
        }
    }
}

//-----------------------------------------------------------------------------
// Function:    DuplicateContact
//
// Purpose:     Duplicates a PEER_CONTACT structure
//              It allocates a copy, so the caller needs to free
//
// Returns:        HRESULT
//
HRESULT DuplicateContact(PEER_CONTACT ** ppContactDestination, const PEER_CONTACT * pContactSource)
{
    PEER_CONTACT * pTempDestination = NULL;
    size_t cbPeerName = 0;
    size_t cbNickName = 0;
    size_t cbDisplayName = 0;
    size_t cbEmailAddress = 0;
    WCHAR *pPos = NULL;

    if (ppContactDestination == NULL || pContactSource == NULL) return E_INVALIDARG;

    //Get size of the strings pointed to by the PWSTR elements
    //
    if (pContactSource->pwzPeerName)
    {
        cbPeerName = ROUND_UP_COUNT((wcslen(pContactSource->pwzPeerName) * sizeof(WCHAR)) + sizeof(WCHAR), ALIGN_LPVOID);
    }

    if (pContactSource->pwzNickName)
    {
        cbNickName = ROUND_UP_COUNT((wcslen(pContactSource->pwzNickName) * sizeof(WCHAR)) + sizeof(WCHAR), ALIGN_LPVOID);
    }

    if (pContactSource->pwzDisplayName)
    {
        cbDisplayName = ROUND_UP_COUNT((wcslen(pContactSource->pwzDisplayName) * sizeof(WCHAR)) + sizeof(WCHAR), ALIGN_LPVOID);
    }

    if (pContactSource->pwzEmailAddress)
    {
        cbEmailAddress = ROUND_UP_COUNT((wcslen(pContactSource->pwzEmailAddress) * sizeof(WCHAR)) + sizeof(WCHAR), ALIGN_LPVOID);
    }
    
    //Allocate PEER_CONTACT structure
    //
    pTempDestination = (PEER_CONTACT *) malloc(ROUND_UP_COUNT(sizeof(PEER_CONTACT), ALIGN_LPVOID) + cbPeerName + cbNickName + cbDisplayName + cbEmailAddress + pContactSource->credentials.cbData);
    
    if (NULL == pTempDestination)
        return E_OUTOFMEMORY;
    
    ZeroMemory(pTempDestination, sizeof(PEER_CONTACT));

    //Copy the relevant fields into the PEER_PEOPLE_NEAR_ME structure
    //
    pTempDestination->fWatch = pContactSource->fWatch;
    pTempDestination->WatcherPermissions = pContactSource->WatcherPermissions;
    pTempDestination->credentials.cbData = pContactSource->credentials.cbData;
    pPos = (WCHAR *)ROUND_UP_POINTER((pTempDestination + 1), ALIGN_LPVOID);

    if (pContactSource->pwzPeerName)
    {
        pTempDestination->pwzPeerName = pPos;
    
        //Copy the relevant fields into the PEER_PEOPLE_NEAR_ME structure
        //
        memcpy(pTempDestination->pwzPeerName, pContactSource->pwzPeerName, cbPeerName);

        pPos+=ROUND_UP_COUNT(cbPeerName, ALIGN_LPVOID)/sizeof(WCHAR);
    }

    if (pContactSource->pwzNickName)
    {
        pTempDestination->pwzNickName = pPos;
    
        //Copy the relevant fields into the PEER_PEOPLE_NEAR_ME structure
        //
        memcpy(pTempDestination->pwzNickName, pContactSource->pwzNickName, cbNickName);

        pPos+=ROUND_UP_COUNT(cbNickName, ALIGN_LPVOID)/sizeof(WCHAR);
    }

    if (pContactSource->pwzDisplayName)
    {
        pTempDestination->pwzDisplayName = pPos;
    
        //Copy the relevant fields into the Display_PEOPLE_NEAR_ME structure
        //
        memcpy(pTempDestination->pwzDisplayName, pContactSource->pwzDisplayName, cbDisplayName);

        pPos+=ROUND_UP_COUNT(cbDisplayName, ALIGN_LPVOID)/sizeof(WCHAR);
    }

    if (pContactSource->pwzEmailAddress)
    {
        pTempDestination->pwzEmailAddress = pPos;
    
        //Copy the relevant fields into the PEER_PEOPLE_NEAR_ME structure
        //
        memcpy(pTempDestination->pwzEmailAddress, pContactSource->pwzEmailAddress, cbEmailAddress);

        pPos+=ROUND_UP_COUNT(cbEmailAddress, ALIGN_LPVOID)/sizeof(WCHAR);
    }

    if (pContactSource->credentials.pbData)
    {
        pTempDestination->credentials.pbData = (PBYTE)pPos;
    
        memcpy(pTempDestination->credentials.pbData, pContactSource->credentials.pbData, pContactSource->credentials.cbData);
    }

    *ppContactDestination = pTempDestination;

    return S_OK;
}

//-----------------------------------------------------------------------------
// Function:    DuplicateEndpoint
//
// Purpose:     Duplicates a PEER_ENDPOINT structure
//              It allocates a copy, so the caller needs to free
//
// Returns:        HRESULT
//
HRESULT DuplicateEndpoint(PEER_ENDPOINT ** ppEndpointDestination, const PEER_ENDPOINT * pEndpointSource)
{
    size_t cbEndpointName = 0;
    PEER_ENDPOINT * pTempDestination = NULL;
    WCHAR *pPos = NULL;

    if (pEndpointSource->pwzEndpointName)
    {
        cbEndpointName = ROUND_UP_COUNT((wcslen(pEndpointSource->pwzEndpointName) * sizeof(WCHAR)) + sizeof(WCHAR), ALIGN_LPVOID);
    }
    
    //Allocate PEER_PEOPLE_NEAR_ME structure
    //
    pTempDestination = (PEER_ENDPOINT *) malloc(ROUND_UP_COUNT(sizeof(PEER_ENDPOINT), ALIGN_LPVOID) + cbEndpointName);
    
    if (NULL == pTempDestination)
        return E_OUTOFMEMORY;
    
    ZeroMemory(pTempDestination, sizeof(PEER_ENDPOINT));

    //Copy the relevant fields into the PEER_PEOPLE_NEAR_ME structure
    //
    pTempDestination->address = pEndpointSource->address;
    pPos = (WCHAR *)ROUND_UP_POINTER((pTempDestination + 1), ALIGN_LPVOID);

    //Get size of pwzEndpointName
    //
    if (pEndpointSource->pwzEndpointName)
    {
        pTempDestination->pwzEndpointName = pPos;

        //Copy the relevant fields into the PEER_PEOPLE_NEAR_ME structure
        //
        memcpy(pTempDestination->pwzEndpointName, pEndpointSource->pwzEndpointName, cbEndpointName);

        pPos+=ROUND_UP_COUNT(cbEndpointName, ALIGN_LPVOID)/sizeof(WCHAR);
    }

    *ppEndpointDestination = pTempDestination;

    return S_OK;
}

//-----------------------------------------------------------------------------
// Function:   SelectContact
// Purpose:    Retrieves and prints to the console all contacts for the currently
//             logged in user.  Unless fPrompt is FALSE the function also 
//             prompts the user to select a contact from the list.
// Parameters:
//   pppContacts :  [out] pointer to the selected contact.
//   fPrompt     :  [out] If false skips prompting the user to select a contact (only prints the contacts)
//                        
HRESULT SelectContact(__out PPEER_CONTACT *pContact, __in BOOL fPrompt)
{
    HPEERENUM hEnum;
    HRESULT hr = S_OK;
    ULONG Selection = 0;
    WCHAR wzBuff[5];
    PPEER_CONTACT *ppContacts = NULL;
    ULONG cContacts = 0;

    if (pContact == NULL) return E_INVALIDARG;

    // Begin an enumeration of all contacts
    //
    hr = PeerCollabEnumContacts(&hEnum);

    IF_FAILED_PRINT_ERROR_AND_EXIT(hr, L"PeerCollabEnumContacts failed.");

    // Retrieve the number of contacts
    //
    hr = PeerGetItemCount(hEnum, &cContacts);

    IF_FAILED_PRINT_ERROR_AND_EXIT(hr, L"PeerGetItemCount failed.");

    if (cContacts == 0)
    {
        wprintf(L"No contacts exist.\n");
        hr = E_FAIL;
        goto exit;
    }

    // Retrieve all the contacts
    //
    hr = PeerGetNextItem(hEnum, &cContacts, (void***) &ppContacts);

    if (FAILED(hr))
    {
        wprintf(L"Failed to retrieve contacts.  hr=0x%x\n", hr);
        PrintError(hr);
        goto exit;
    }

    // Display the contacts
    //
    PrintContacts(ppContacts, cContacts);

    if (fPrompt != FALSE)
    {
        // Prompt the user to select a contact
        //
        wprintf(L"Select contact: ");
        GET_PROMPT_RESPONSE(hr, wzBuff);

        Selection = _wtoi(wzBuff);

        if (Selection < 1 || (ULONG)Selection > cContacts)
        {
            wprintf(L"Invalid selection.\n");
            hr = E_FAIL;
            goto exit;
        }

        Selection--; //make it 0 based rather than 1 based.

        hr = DuplicateContact(pContact, ppContacts[Selection]);

        IF_FAILED_PRINT_ERROR_AND_EXIT(hr, L"Failed to duplicate contacts.");
    }

exit:
    SAFE_PEER_END_ENUMERATION(hEnum);
    SAFE_PEER_FREE_DATA(ppContacts);

    return hr;
}

//-----------------------------------------------------------------------------
// Function:    SelectEndpointFromContact
// Purpose:     Given a contact allow the user to select an available endpoint of that contact
// Parameters:  
//   pContact     : [in] contact to select endpoint for
//   ppEndpoint   : [out] pointer to an endpoint pointer of the endpoint that the user selected
//
HRESULT SelectEndpointFromContact(__in const PEER_CONTACT *pContact, __out PEER_ENDPOINT **ppEndpoint)
{
    HRESULT hr;
    PEER_ENDPOINT **ppEndpoints = NULL;
    HPEERENUM hEnum = NULL;
    ULONG ulEndpoint = 0;
    int nSelection = 0;
    ULONG cEndpoints = 0;
    WCHAR wzInputBuffer[5] = {0};

    if (pContact == NULL || ppEndpoint == NULL) return E_INVALIDARG;

    *ppEndpoint = NULL;

    // Enumerate all endpoints for this contact
    //
    hr = PeerCollabEnumEndpoints(pContact, &hEnum);

    IF_FAILED_PRINT_ERROR_AND_EXIT(hr, L"PeerCollabEnumEndpoints failed.");

    // Get the count of endpoints for this contact
    //
    hr = PeerGetItemCount(hEnum, &cEndpoints);

    IF_FAILED_PRINT_ERROR_AND_EXIT(hr, L"PeerGetItemCount failed.");

    if (cEndpoints == 0)
    {
        wprintf(L"Contact has no endpoints visible. Are we watching this contact?  Has this contact granted us watch permissions?)\n");
        hr = E_FAIL;
        goto exit;
    }

    // If there are any endpoints, retrieve them all
    //
    hr = PeerGetNextItem(hEnum, &cEndpoints, (void***) &ppEndpoints);

    IF_FAILED_PRINT_ERROR_AND_EXIT(hr, L"PeerGetNextItem failed.");

    // Prompt for which endpoint to send the invite to
    //
    for (ulEndpoint = 0; ulEndpoint < cEndpoints; ulEndpoint++)
    {
        wprintf(L"%d)\n", ulEndpoint+1);
        PrintEndpoint(ppEndpoints[ulEndpoint]);
    }

    wprintf(L"\nSelect Endpoint [1-%d]: ", cEndpoints);
    GET_PROMPT_RESPONSE(hr, wzInputBuffer);

    nSelection = _wtoi(wzInputBuffer);

    if (nSelection < 1 || (ULONG)nSelection > cEndpoints)
    {
        wprintf(L"\nInvalid Selection.\n");
        hr = E_FAIL;
        goto exit;
    }

    nSelection--; //make it 0 based rather than 1 based.

    hr = DuplicateEndpoint(ppEndpoint, ppEndpoints[nSelection]);

    if (FAILED(hr))
    {
        wprintf(L"Failed to duplicate endpoint structure.\n");
        goto exit;
    }

    //Ensure we have the latest information from this endpoint
    //
    hr = RefreshEndpoint(*ppEndpoint);

    if (FAILED(hr))
    {
        wprintf(L"Failed to refresh endpoint.\n");
        goto exit;
    }

exit:
    if (FAILED(hr))
    {
        //If we are returning an error but have already allocated memory for the out
        //paramter it is our responsibillity to clean up
        SAFE_FREE(*ppEndpoint);
    }

    SAFE_PEER_FREE_DATA(ppEndpoints);
    SAFE_PEER_END_ENUMERATION(hEnum);

    return hr;
}

//-----------------------------------------------------------------------------
// Function:    SelectContactEndpoint
// Purpose:     Select a contact and an endpoint of that contact
// Parameters:  
//   pContact     : [out] pointer to a pointer of the selected contact
//   ppEndpoint   : [out] pointer to a pointer of the selected endpoint
//
HRESULT SelectContactEndpoint(__out PPEER_CONTACT *ppContact, PPEER_ENDPOINT *ppEndpoint)
{
    HRESULT hr = S_OK;

    if (ppContact == NULL || ppEndpoint == NULL) return E_INVALIDARG;

    *ppContact = NULL;
    *ppEndpoint = NULL;

    // Retrieve the list of contacts
    //
    hr = SelectContact(ppContact, TRUE);

    if (FAILED(hr))
    {
        goto exit;
    }

    hr = SelectEndpointFromContact(*ppContact, ppEndpoint);
    if (FAILED(hr))
    {
        goto exit;
    }

exit:
    if (FAILED(hr))
    {
        SAFE_FREE(*ppContact);
        SAFE_FREE(*ppEndpoint);
    }

    return hr;
}

//-----------------------------------------------------------------------------
// Function:    SelectPNMEndpoint
// Purpose:     Returns a PNM endpoint selected by the user from a menu of all
//              available PNM endpoints
// Parameters:
//   ppEndpoint     : [out] pointer to a pointer of the selected endpoint
//
HRESULT SelectPNMEndpoint(__out PPEER_ENDPOINT *ppEndpoint)
{
    PEER_PEOPLE_NEAR_ME**    ppPeopleNearMe = NULL;
    HRESULT                  hr = S_OK;
    HPEERENUM                hEnum = NULL;
    ULONG                    count = 0;
    ULONG                    ulEndpoint = 0;   
    WCHAR                    wzInputBuffer[5] = {0};

    if (ppEndpoint == NULL) return E_INVALIDARG;

    *ppEndpoint = NULL;
 
    hr = PeerCollabEnumPeopleNearMe(&hEnum);

    IF_FAILED_PRINT_ERROR_AND_EXIT(hr, L"PeerCollabEnumPeopleNearMe failed.");

    hr = PeerGetItemCount(hEnum, &count);

    IF_FAILED_PRINT_ERROR_AND_EXIT(hr, L"PeerGetItemCount failed.");

    if (count == 0)
    {
        wprintf(L"No people near me found\n");
        hr = E_FAIL;
        return hr;
    }

    hr = PeerGetNextItem(hEnum, &count, (PVOID **) &ppPeopleNearMe);

    IF_FAILED_PRINT_ERROR_AND_EXIT(hr, L"PeerGetNextItem failed.");

    PrintPeopleNearMeInfo(ppPeopleNearMe, count);

    wprintf(L"\nSelect Endpoint [1-%d]: ", count);
    GET_PROMPT_RESPONSE(hr, wzInputBuffer);

    ulEndpoint = _wtoi(wzInputBuffer);
    if (ulEndpoint < 1 || ulEndpoint > count)
    {
        wprintf(L"\nInvalid Selection\n");
        hr = E_FAIL;
        goto exit;
    }

    ulEndpoint--; //make it 0 based

    hr = DuplicateEndpoint(ppEndpoint, &ppPeopleNearMe[ulEndpoint]->endpoint);

    //Ensure we have the latest information from this endpoint
    //
    hr = RefreshEndpoint(*ppEndpoint);

    if (FAILED(hr))
    {
        wprintf(L"Failed to refresh endpoint.\n");
        goto exit;
    }

exit:
    if (FAILED(hr))
    {
        //If we are returning an error but have already allocated memory for the out
        //paramter it is our responsibillity to clean up
        SAFE_FREE(*ppEndpoint);
    }

    SAFE_PEER_FREE_DATA(ppPeopleNearMe);
    SAFE_PEER_END_ENUMERATION(hEnum);

    return hr;
}

//-----------------------------------------------------------------------------
// Function:    SelectMultipleEndpoints
// Purpose:     Select multiple sets of
//                    - a PNM endpoint; and
//                    - a contact and endpoint
// Parameters:  
//   ulNumEndpoints: [in]  The number of endpoints to allocate and populate.
//   pppContact    : [out] Pointer to an array of pointers to PPEER_CONTACT structures.  
//                         The function will allocate an array of size ulNumEndpoints.
//                         In the case where a PNM endpoint was selected that arrary element will be set to NULL.
//   pppEndpoint   : [out] Pointer to an array or pointers to PPEER_ENDPOINT structures.
//                         The function will allocate an array of size ulNumEndpoints
//
// Remarks:
//   If this function returns a successes HRESULT the called is responsible for freeing *pppContacts and *pppEndpoint.
//   The caller must seperatly free each non-null pointer within each array and then free the pointers to the arrays.
//
HRESULT SelectMultipleEndpoints(__in unsigned int uNumEndpoints, __out PPEER_CONTACT **pppContacts, __out PPEER_ENDPOINT **pppEndpoint)
{
    HRESULT hr = S_OK;
    unsigned int uCurrentEndpoint = 0;

    if (pppContacts== NULL || pppEndpoint == NULL || uNumEndpoints == 0) return E_INVALIDARG;

    *pppContacts = (PPEER_CONTACT *) malloc(sizeof (PPEER_CONTACT) * uNumEndpoints);
    if (pppContacts == NULL)
    {
        printf("Failed to allocate memory in SelectMultipleEndpoints\n");
        hr = E_ABORT;
        goto exit;
    }

    *pppEndpoint = (PPEER_ENDPOINT *) malloc(sizeof (PPEER_ENDPOINT) * uNumEndpoints);
    if (pppEndpoint == NULL)
    {
        printf("Failed to allocate memory in SelectMultipleEndpoints\n");
        hr = E_ABORT;
        goto exit;
    }

    ZeroMemory(*pppContacts, sizeof (PPEER_CONTACT) * uNumEndpoints);
    ZeroMemory(*pppEndpoint, sizeof (PPEER_ENDPOINT) * uNumEndpoints);

    uCurrentEndpoint = 0;
    while (uCurrentEndpoint < uNumEndpoints)
    {
        wprintf(L"\nSelecting endpoint %d of %d:\n", uCurrentEndpoint+1, uNumEndpoints);
        hr = SelectEndpoint(&((*pppContacts)[uCurrentEndpoint]), &((*pppEndpoint)[uCurrentEndpoint]));
        if(FAILED(hr) && hr != E_FAIL)
        {
            //for E_FAIL we will retry, otherwise abort the loop
            goto exit;
        }

        if (SUCCEEDED(hr))
        {
            uCurrentEndpoint++;  
        }
    }

exit:
    if (FAILED(hr))
    {
        if (*pppContacts)
        {
            for (uCurrentEndpoint = 0; uCurrentEndpoint < uNumEndpoints; uCurrentEndpoint++)
            {
                if ((*pppContacts)[uCurrentEndpoint] != NULL)
                {
                    SAFE_FREE((*pppContacts)[uCurrentEndpoint]);
                }
            }

            SAFE_FREE(*pppContacts);        
        }

        if (*pppEndpoint)
        {
            for (uCurrentEndpoint = 0; uCurrentEndpoint < uNumEndpoints; uCurrentEndpoint++)
            {
                if ((*pppEndpoint)[uCurrentEndpoint] != NULL)
                {
                    DeleteEndpointData((*pppEndpoint)[uCurrentEndpoint]);
                    SAFE_FREE((*pppEndpoint)[uCurrentEndpoint]);
                }
            }

            SAFE_FREE(*pppEndpoint);        
        }
    }
    return hr;
}


//-----------------------------------------------------------------------------
// Function:    SelectEndpoint
// Purpose:     Select either
//                    - a PNM endpoint; or
//                    - a contact and endpoint
// Parameters:  
//   [pContact    : [out] pointer to a pointer of the selected contact.  *pContact will be
//                        set to NULL if a PNM endpoint is selected
//   ppEndpoint   : [out] pointer to a pointer of the selected endpoint
//
HRESULT SelectEndpoint(__out PPEER_CONTACT *ppContact, __out PPEER_ENDPOINT *ppEndpoint)
{
    HRESULT hr = S_OK;
    BOOL fIsContactEndpoint = FALSE;
    WCHAR wzBuff[5] = {0};
    int nSelection = 0; 

    if (ppContact == NULL || ppEndpoint == NULL) return E_INVALIDARG;

    *ppContact = NULL;
    *ppEndpoint = NULL;

    wprintf(L"What type of endpoint (1) Contact, (2) People Near Me [1-2]: ");
    GET_PROMPT_RESPONSE(hr, wzBuff);

    nSelection = _wtoi(wzBuff);

    if (nSelection < 1 || nSelection > 2)
    {
        wprintf(L"Invalid selection.\n");
        hr = E_ABORT; //let the user exit out without needing to complete the process
        goto exit;
    }

    fIsContactEndpoint = nSelection==1?TRUE:FALSE;

    if (fIsContactEndpoint != FALSE)
    {
        hr = SelectContactEndpoint(ppContact, ppEndpoint);
    }
    else
    {
        hr = SelectPNMEndpoint(ppEndpoint);
    }

exit:
    if (FAILED(hr))
    {
        SAFE_FREE(*ppContact);
        SAFE_FREE(*ppEndpoint);
    }

    return hr;
}

//-----------------------------------------------------------------------------
// Function:    RefreshEndpoint
// Purpose:     Requests a refresh of a specific endpoint's data.
//              Note that if a subscription to the endpoint exists via
//              PeerCollabSubscribeEndpoint, this is not necessary.
// Parameters:  
//   pcEndpoint : [in] endpoint to refresh
//
HRESULT RefreshEndpoint(__in PCPEER_ENDPOINT pcEndpoint)
{
    HPEEREVENT            hPeerEvent = NULL;
    HRESULT               hr = S_OK;
    HANDLE                hEvent = NULL;
    PEER_COLLAB_EVENT_DATA *pEventData = NULL;
    PEER_COLLAB_EVENT_REGISTRATION  eventReg = {0};
    DWORD dwWait = 0;

    if (pcEndpoint == NULL) return E_INVALIDARG;

    hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (hEvent == NULL)
    {
        goto exit;
    }

    eventReg.eventType = PEER_EVENT_REQUEST_STATUS_CHANGED;
    eventReg.pInstance = NULL;

    // Register to be notified when the request finishes  
    hr = PeerCollabRegisterEvent(hEvent, 1, &eventReg, &hPeerEvent);

    IF_FAILED_PRINT_ERROR_AND_EXIT(hr, L"PeerCollabRegisterEvent failed.");

    hr = PeerCollabRefreshEndpointData(pcEndpoint);

    IF_FAILED_PRINT_ERROR_AND_EXIT(hr, L"PeerCollabRefreshEndpointData failed.");

    // Block until an event is set indicating that endpoint data has
    // successfully been refreshed
    dwWait = WaitForSingleObject(hEvent, SHORT_TIMEOUT);
    
    switch (dwWait)
    {
        case WAIT_OBJECT_0:
            // Find out if the refresh request succeeded
            hr = PeerCollabGetEventData(hPeerEvent, &pEventData);

            IF_FAILED_PRINT_ERROR_AND_EXIT(hr, L"PeerCollabGetEventData failed.");

            if (FAILED(pEventData->requestStatusChangedData.hrChange))
            {
                wprintf(L"The data returned by PeerCollabGetEventData indicates failure. HRESULT=0x%x\n", hr);
                PrintError(hr);
                goto exit;
            }

            break;

        case WAIT_TIMEOUT:
            wprintf(L"Endpoint was not refreshed after waiting %d seconds.  Giving up.\n", SHORT_TIMEOUT/1000);
            hr = E_FAIL;
            goto exit;
            break;

        default:
            wprintf(L"WaitForSingleObject returned unexpected valud.  Giving up.\n");
            hr = E_FAIL;
            goto exit;
            break;
    }

exit:
    SAFE_PEER_FREE_DATA(pEventData);

    if (hPeerEvent)
    {
        (void)PeerCollabUnregisterEvent(hPeerEvent);
        hPeerEvent = NULL;
    }

    if (hEvent)
    {
        CloseHandle(hEvent);
        hEvent = NULL;
    }
    return hr;
}

//-----------------------------------------------------------------------------
// Function:    DeleteEndpointData
// Purpose:     Delete cached endpoint data when it is no longer required
// Parameters:  
//   pcEndpoint : [in] endpoint for which to delete cached data
//
HRESULT DeleteEndpointData(__in PCPEER_ENDPOINT pcEndpoint)
{
    HRESULT               hr = S_OK;

    //Ignore failures - best effort to delete cached data
    (void)PeerCollabDeleteEndpointData(pcEndpoint);

    return hr;
}

//-----------------------------------------------------------------------------
// Function:    GetParentProcessHandle
// Purpose:     Returns a handle to the parent process of the calling process
// Parameters:  
//   pHandle : [out] Pointer to a handle to the parent process of the calling process
//
HRESULT GetParentProcessHandle(HANDLE *pHandle)
{
  HANDLE hProcessSnap;
  HANDLE hProcess;
  PROCESSENTRY32 pe32;
  HRESULT hr = S_OK;
  DWORD dwCurrentProcessId = 0;

  dwCurrentProcessId = GetCurrentProcessId();


  // Take a snapshot of all processes in the system.
  hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
  if( hProcessSnap == INVALID_HANDLE_VALUE )
  {
    printf( "CreateToolhelp32Snapshot (of processes) failed" );
    hr = E_ABORT;
    goto exit;
  }

  // Set the size of the structure before using it.
  pe32.dwSize = sizeof( PROCESSENTRY32 );

  // Retrieve information about the first process.
  // We can use this to get hte process Id of our parent processs
  if( !Process32First( hProcessSnap, &pe32 ) )
  {
    printf("Process32First failed" );  // Show cause of failure
    hr = E_ABORT;
    goto exit;
  }

  do
  {
    // Retrieve a handle to the process
    hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID );
    if( hProcess != NULL )
    {
        if (dwCurrentProcessId == pe32.th32ProcessID)
        {
            CloseHandle( hProcess );

            // Retrieve a handle to the process
            hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pe32.th32ParentProcessID );

            *pHandle = hProcess;
            break;
        }

        CloseHandle( hProcess );
    }

  } while( Process32Next( hProcessSnap, &pe32 ) );

    hr = S_OK;



exit:
  CloseHandle( hProcessSnap );
  return( TRUE );
}


//-----------------------------------------------------------------------------
// Function:    PrintError
// Purpose:     Utility routine to print the status of a set of invitations.  Will
//              optionally cancel outstanding invitations if fCancelInvitations == TRUE
//
// Return:
//      HRESULT - TRUE if there outstanding invitations were found.  FALSE otherwise.
//
BOOL PrintInvitationStatus(__in HANDLE *phInvite, __in unsigned int uNumEndpoints, __in BOOL fCancelInvitations)
{
    BOOL fInvitationsOutstanding = FALSE;
    PPEER_INVITATION_RESPONSE pResponse = NULL;
    unsigned int uCurrentEndpoint = 0;
    HRESULT hr = S_OK;

    if (phInvite == NULL || uNumEndpoints == 0) return FALSE;

    wprintf(L"------------------------------\n");
    wprintf(L"      Invitation Status\n");

    for (uCurrentEndpoint=0; uCurrentEndpoint < uNumEndpoints; uCurrentEndpoint++)
    {
        wprintf(L"Invitation #%d: ", uCurrentEndpoint+1);
        hr = PeerCollabGetInvitationResponse(phInvite[uCurrentEndpoint], &pResponse);

        if (hr == PEER_E_INVITE_RESPONSE_NOT_AVAILABLE)
        {
            fInvitationsOutstanding = TRUE;

            if (fCancelInvitations != FALSE)
            {
                wprintf(L"Cancelling invitation.  ");
                hr = PeerCollabCancelInvitation(phInvite[uCurrentEndpoint]);

                if (SUCCEEDED(hr))
                {
                    wprintf(L"Successfully cancelled Invitation.\n");
                }
                else
                {
                    wprintf(L"Failed cancel invitation.  hr=0x%x\n", hr);
                    PrintError(hr);
                }
            }
            else
            {
                wprintf(L"Invitation response pending.\n");
            }
        }
        else
        {
            IF_FAILED_PRINT_ERROR_AND_EXIT(hr, L"PeerCollabGetInvitationResponse failed.");

            if (pResponse->action == PEER_INVITATION_RESPONSE_ACCEPTED)
            {
                wprintf(L"Invitation accepted.\n");
            }
            else
            {
                wprintf(L"Invitation not accepted.\n");
            }
        }

        SAFE_PEER_FREE_DATA(pResponse);
    }

    wprintf(L"------------------------------\n\n", uCurrentEndpoint+1);

exit:

    SAFE_PEER_FREE_DATA(pResponse);

    return fInvitationsOutstanding;
}