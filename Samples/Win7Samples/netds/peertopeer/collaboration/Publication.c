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
#include "Publication.h"
#include "shared.h"


//-----------------------------------------------------------------------------
// Function:    SubscribeEndpointData
// Purpose:     Subscribe to changes in a specific endpoint's data
// Parameters:  
//   pcEndpoint [in] : pointer to endpoint whose data we wish to subscribe to
//
void SubscribeEndpointData()
{
    HRESULT hr = S_OK;
    PEER_ENDPOINT *pEndpoint = NULL;

    //Note:  The pContacts and pEndpoint structures
    //returned by this function have been allocated by the,
    //sample application directly using malloc, not via the p2p library.
    //We must free them by calling free.  Do not called PeerFreeData
    hr = SelectPNMEndpoint(&pEndpoint);

    if (FAILED(hr))
    {
        goto exit;
    }

    hr = PeerCollabSubscribeEndpointData(pEndpoint);
    if (SUCCEEDED(hr))
    {
        wprintf(L"Subscribing to endpoint data.\n");
    }
    else
    {
        wprintf(L"PeerCollabSubscribeEndpointData failed.\nHRESULT=0x%x\n", hr);
        PrintError(hr);
    }


exit:
    SAFE_FREE(pEndpoint);
}

//-----------------------------------------------------------------------------
// Function:    UnsubscribeEndpointData
// Purpose:     Unsubscrie from changes to a specific endpoints data
// Parameters:  
//   None
//
void UnsubscribeEndpointData()
{
    HRESULT hr = S_OK;
    PEER_ENDPOINT *pEndpoint = NULL;

    //Note:  The pContacts and pEndpoint structures
    //returned by this function have been allocated by the,
    //sample application directly using malloc, not via the p2p library.
    //We must free them by calling free.  Do not called PeerFreeData
    hr = SelectPNMEndpoint(&pEndpoint);

    if (FAILED(hr))
    {
        goto exit;
    }

    hr = PeerCollabUnsubscribeEndpointData(pEndpoint);
    if (SUCCEEDED(hr))
    {
        wprintf(L"Successfully unsubscribed to endpoint data.\n");
    }
    else
    {
        wprintf(L"PeerCollabUnsubscribeEndpointData failed. HRESULT=0x%x\n", hr);
        PrintError(hr);
    }


exit:
    //Releases memory used by the P2P library to cache information regarding this endpoint
    if (pEndpoint)
    {
        DeleteEndpointData(pEndpoint);
    }

    SAFE_FREE(pEndpoint);
}

//-----------------------------------------------------------------------------
// Function:    PublishEndpointObject
// Purpose:     Publishes a sample object (a string message)
// Parameters:  None
//
void PublishEndpointObject()
{
    PEER_OBJECT   object = {0};
    WCHAR         wzBuff[256] = {0};
    HRESULT       hr = S_OK;
    int nPublicationScope = 0;

    wprintf(L"Publication Scope (1) Near Me, (2) Internet, (3) All [1-3]: ");
    GET_PROMPT_RESPONSE(hr, wzBuff);

    // since wtoi can't distinguish between 0 and an error, note that valid values start at 1, not 0    
    nPublicationScope = _wtoi(wzBuff);
    if (nPublicationScope < 1  || nPublicationScope > 3)
    {
        printf ("Invalid input, expected a number between 1 and 3.\n");
        goto exit;
    }

    wprintf(L"\nObject Data [String]: ");
    GET_PROMPT_RESPONSE(hr, wzBuff);

    object.id = MESSAGE_GUID;
    object.data.cbData = (ULONG) (wcslen(wzBuff) + 1) * sizeof(WCHAR);
    object.data.pbData = (PBYTE) wzBuff;
    object.dwPublicationScope = nPublicationScope;
    hr = PeerCollabSetObject(&object);

    if (SUCCEEDED(hr))
    {
        wprintf(L"Object published.\n");
    }
    else
    {
        wprintf(L"PeerCollabSetObject failed\nHRESULT=0x%x\n", hr);
        PrintError(hr);
    }

exit:
    return;
}

//-----------------------------------------------------------------------------
// Function:    DeleteEndpointObject
// Purpose:     Deletes the sample object (a string message) from the this endpoint
// Parameters:  None
//
void DeleteEndpointObject()
{
    HRESULT       hr = S_OK;

    hr = PeerCollabDeleteObject(&MESSAGE_GUID);

    if (SUCCEEDED(hr))
    {
        wprintf(L"Object deleted successfully.\n");
    }
    else
    {
        wprintf(L"PeerCollabDeleteObject failed\nHRESULT=0x%x\n", hr);
        PrintError(hr);
    }

    return;
}
