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
#include "EndpointInfo.h"
#include "Shared.h"


//-----------------------------------------------------------------------------
// Function:    SetEndpointName
// Purpose:     Routine to set the name of the current endpoint used by the peer application
// Parameters:  None.
//
void SetEndpointName()
{
    HRESULT hr = S_OK;
    WCHAR wzBuff[STRING_BUFSIZE];

    //Retrieve the desired endpoint name.
    //
    wprintf(L"Enter Endpoint name: ");
    GET_PROMPT_RESPONSE(hr, wzBuff);

    //Set the endpoint name
    //PeerCollabSetEndpointName imposes a limit of 255 Unicode characters
    //for the endpoint name.  For this sample we intentionally allow more than 255 characters to
    //demonstrate the error that is returned (E_INVALIDARG).
    //
    hr = PeerCollabSetEndpointName(wzBuff);

    if (SUCCEEDED(hr))
    {
        wprintf(L"Endpoint name successfully set\n");
    }
    else
    {
        wprintf(L"PeerCollabSetEndpointName failed. HRESULT=0x%x.\n", hr);
        PrintError(hr);
    }

exit:
    return;
}

//-----------------------------------------------------------------------------
// Function:    GetEndpointName
// Purpose:     Routine to get the name of the current endpoint used by the peer application
// Parameters:  None.
//
void GetEndpointName()
{
    HRESULT hr = S_OK;
    PWSTR pwzEndpointName = NULL;

    //Get the endpoint name
    //
    hr = PeerCollabGetEndpointName(&pwzEndpointName);

    if (SUCCEEDED(hr))
    {
        wprintf(L"Endpoint name is: %s\n", pwzEndpointName);
    }
    else
    {
        wprintf(L"PeerCollabGetEndpointName failed. HRESULT=0x%x.\n", hr);
        PrintError(hr);
    }

    SAFE_PEER_FREE_DATA(pwzEndpointName);
}

//-----------------------------------------------------------------------------
// Function:    DisplayEndpointObjects
// Purpose:     Retrieves and displays object information from an endpoint. 
//              Should call PeerCollabRefreshEndpointData before using this function.
// Parameters:  
//   PCPEER_ENDPOINT : [in] endpoint about which information is displayed
//
HRESULT DisplayEndpointObjects(__in PCPEER_ENDPOINT pcEndpoint)
{
    HPEERENUM           hObjectEnum = NULL;
    ULONG               cObjects = 0;
    ULONG               i = 0;
    PPEER_OBJECT*       ppObjects = NULL;
    HRESULT             hr = S_OK;

    // Get a list of objects published by this endpoint - The NULL parameter
    // indicates that we wish to retrieve all objects
    hr = PeerCollabEnumObjects(pcEndpoint, NULL, &hObjectEnum);
    
    if (FAILED(hr))
    {
        wprintf(L"Error retrieving objects. HRESULT=0x%x\n", hr);
        PrintError(hr);
        goto exit;
    }

    hr = PeerGetItemCount(hObjectEnum, &cObjects);

    if (FAILED(hr))
    {
        wprintf(L"Error retrieving item count. HRESULT=0x%x\n", hr);
        PrintError(hr);
        goto exit;
    }

    if (cObjects == 0)
    {
        wprintf(L"No objects found for this endpoint\n");
        hr = E_FAIL;
        goto exit;
    }

    hr = PeerGetNextItem(hObjectEnum, &cObjects, (PVOID **) &ppObjects);

    if (FAILED(hr))
    {
        wprintf(L"PeerGetNextItem failed. HRESULT=0x%x\n", hr);
        goto exit;
    }

    for (i = 0; i < cObjects; i++)
    {
        PrintObject(ppObjects[i]);
    }


exit:
    SAFE_PEER_FREE_DATA(ppObjects);
    SAFE_PEER_END_ENUMERATION(hObjectEnum);

    return hr;
}

//-----------------------------------------------------------------------------
// Function:    DisplayEndpointApplications
// Purpose:     Retrieves and displays application information from an endpoint
// Parameters:  
//   PCPEER_ENDPOINT : [in] endpoint about which information is displayed
//
HRESULT DisplayEndpointApplications(__in PCPEER_ENDPOINT pcEndpoint)
{
    HPEERENUM           hApplicationEnum = NULL;
    PPEER_APPLICATION*  ppApplications = NULL;
    ULONG               cApplications = 0;
    ULONG               i = 0;
    HRESULT             hr = S_OK;

    // Get a list of applications from the endpoint - The NULL parameter 
    // indicates that we want to retrieve all applications
    hr = PeerCollabEnumApplications(pcEndpoint, NULL, &hApplicationEnum);

    if (FAILED(hr))
    {
        wprintf(L"Error retrieving application info. HRESULT=0x%x\n", hr);
        PrintError(hr);
        goto exit;
    }

    hr = PeerGetItemCount(hApplicationEnum, &cApplications);

    if (FAILED(hr))
    {
        wprintf(L"Error retrieving item count. HRESULT=0x%x\n", hr);
        PrintError(hr);
        goto exit;
    }

    if (cApplications == 0)
    {
        wprintf(L"No applications found for this endpoint.  A PNM endpoint will return an empty list if we are not a trusted contact.  A PNM endpoint may still accept our invitations if it is set to accept invitations from anyone on its subnet.\n");
        hr = E_FAIL;
        goto exit;
    }

    hr = PeerGetNextItem(hApplicationEnum, &cApplications, (PVOID **) &ppApplications);

    if (FAILED(hr))
    {
        wprintf(L"Error retrieving applications. HRESULT=0x%x\n", hr);
        PrintError(hr);
        goto exit;
    }

    for (i = 0; i < cApplications; i++)
    {
        PrintApplication(ppApplications[i]);
    }


exit:
    SAFE_PEER_FREE_DATA(ppApplications);
    SAFE_PEER_END_ENUMERATION(hApplicationEnum);

    return hr;
}

//-----------------------------------------------------------------------------
// Function:    DisplayEndpointInformation
// Purpose:     Routine to display the objects and applications from an endpoint
// Parameters:  None.
//
void DisplayEndpointInformation()
{
    HRESULT hr = S_OK;
    PEER_CONTACT  *pContacts = NULL;
    PEER_ENDPOINT *pEndpoint = NULL;

    //Note:  The pContacts and pEndpoint structures
    //returned by this function have been allocated by the,
    //sample application directly using malloc, not via the p2p library.
    //We must free them by calling free.  Do not called PeerFreeData
    hr = SelectEndpoint(&pContacts, &pEndpoint);

    if (FAILED(hr))
    {
        goto exit;
    }

    DisplayEndpointObjects(pEndpoint);
    DisplayEndpointApplications(pEndpoint);


exit:
    //Releases memory used by the P2P library to cache information regarding this endpoint
    if (pEndpoint)
    {
        DeleteEndpointData(pEndpoint);
    }

    SAFE_FREE(pContacts);
    SAFE_FREE(pEndpoint);

    return;
}