/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) 1985-2007 Microsoft Corporation. All Rights Reserved.

Abstract:
    This C file includes sample code for enumerating people near me
    with the Microsoft Peer-to-Peer Collaboration APIs.

Feedback:
    If you have any questions or feedback, please contact us using
    any of the mechanisms below:

    Email: peerfb@microsoft.com
    Newsgroup: Microsoft.public.win32.programmer.networks
    Website: http://www.microsoft.com/p2p

--********************************************************************/

#pragma warning(disable:4201)   // nameless struct/union

#include <p2p.h>
#include <stdio.h>
#include <strsafe.h>
#include <string.h>
#include <rpc.h>
#include "PNM.h"
#include "shared.h"


//-----------------------------------------------------------------------------
// Function:    EnumeratePeopleNearMe
// Purpose:     Simple wrapper function that calls PeerCollabEnumPeopleNearMe
//              and displays the results.
// Parameters:  (none)
//
void EnumeratePeopleNearMe()
{
    PEER_PEOPLE_NEAR_ME**    ppPeopleNearMe = NULL;
    HRESULT                  hr = S_OK;
    HPEERENUM                hEnum = NULL;
    ULONG                    count = 0;

    //Open an enumeration of all PNM endpoints
    //
    hr = PeerCollabEnumPeopleNearMe(&hEnum);

    if (FAILED(hr))
    {
        wprintf(L"PeerCollabEnumPeopleNearMe failed.\nHRESULT=0x%x\n", hr);
        PrintError(hr);
    }

    hr = PeerGetItemCount(hEnum, &count);

    if (FAILED(hr))
    {
        wprintf(L"Error retrieving item count. HRESULT=0x%x\n", hr);
        PrintError(hr);
        goto exit;
    }

    if (count == 0)
    {
        wprintf(L"No PNM endpoints found.  PNM is not started by default.  On another Vista machine on your IPv6 subnet start PNM via this sample app or the PNM control panel icon.\n");
        hr = E_FAIL;
        goto exit;
    }

    //Obtain all endpoints in the enumeration
    //
    hr = PeerGetNextItem(hEnum, &count, (PVOID **) &ppPeopleNearMe);

    //Display a list of all the endpoints
    //
    if (SUCCEEDED(hr))
    {
        PrintPeopleNearMeInfo(ppPeopleNearMe, count);
    } 
    else
    {
        wprintf(L"Error retrieving endpoints, HRESULT=0x%x", hr);
        PrintError(hr);
    }


exit:
    SAFE_PEER_FREE_DATA(ppPeopleNearMe);

    //Close the enumeratration
    //
    SAFE_PEER_END_ENUMERATION(hEnum);
}

//-----------------------------------------------------------------------------
// Function:    AddEndpointAsContact
// Purpose:     Add an endpoints data to the contact store
// Parameters:  
//   None
//
void AddEndpointAsContact()
{
    HRESULT hr = S_OK;
    PEER_ENDPOINT *pEndpoint = NULL;
    PWSTR           pwzContactData = NULL;


    //Note:  The pEndpoint structures
    //returned by this function has been allocated by the,
    //sample application directly using malloc, not via the p2p library.
    //We must free it by calling free.  Do not called PeerFreeData
    hr = SelectPNMEndpoint(&pEndpoint);

    if (FAILED(hr))
    {
        goto exit;
    }

    hr = PeerCollabQueryContactData(pEndpoint, &pwzContactData);

    IF_FAILED_PRINT_ERROR_AND_EXIT(hr, L"PeerCollabQueryContactData failed.");

    hr = PeerCollabAddContact(pwzContactData, NULL);
    if (SUCCEEDED(hr))
    {
        wprintf(L"Successfully added endpoint as contact.\n");
    }
    else
    {
        wprintf(L"PeerCollabAddContact failed.\nHRESULT=0x%x\n", hr);
        PrintError(hr);
    }
    

exit:
    SAFE_PEER_FREE_DATA(pwzContactData);

    //Releases memory used by the P2P library to cache information regarding this endpoint
    if (pEndpoint)
    {
        DeleteEndpointData(pEndpoint);
    }

    SAFE_FREE(pEndpoint);
}
