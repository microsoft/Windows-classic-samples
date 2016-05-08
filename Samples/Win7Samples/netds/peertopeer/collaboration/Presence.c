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
#include "Presence.h"
#include "Shared.h"


//-----------------------------------------------------------------------------
// Function:    GetPresenceInformation
// Purpose:     Demonstrate the use of PeerCollabGetPresenceInfo() API
// Parameters:  None
//
void GetPresenceInformation()
{
    HRESULT hr = S_OK;
    PEER_CONTACT  *pContacts = NULL;
    PEER_ENDPOINT *pEndpoint = NULL;
    PEER_PRESENCE_INFO* pPresInfo = NULL;

    //Note:  The pContacts and pEndpoint structures
    //returned by this function have been allocated by the,
    //sample application directly using malloc, not via the p2p library.
    //We must free them by calling free.  Do not called PeerFreeData
    hr = SelectEndpoint(&pContacts, &pEndpoint);

    if (FAILED(hr))
    {
        goto exit;
    }

    hr = PeerCollabGetPresenceInfo(pEndpoint, &pPresInfo);
    if (FAILED(hr))
    {
        wprintf(L"PeerCollabGetPresenceInfo failed, hr=0x%x\n", hr);
        PrintError(hr);
        goto exit;
    }

    // print out the presence information
    PrintPresenceInformation(pPresInfo);

exit:
    // free the presence info struct that was allocated by the PeerCollabGetPresenceInfo API
    SAFE_PEER_FREE_DATA(pPresInfo);

    //Releases memory used by the P2P library to cache information regarding this endpoint
    if (pEndpoint)
    {
        DeleteEndpointData(pEndpoint);
    }

    SAFE_FREE(pContacts);
    SAFE_FREE(pEndpoint);
}

//-----------------------------------------------------------------------------
// Function:    SetPresenceInformation
// Purpose:     Demonstrate use of the PeerCollabSetPresenceInfo() API
// Parameters:  None
//
void SetPresenceInformation()
{
    HRESULT             hr = S_OK;
    PEER_PRESENCE_INFO  presInfo = {0};
    WCHAR               wzNumBuf[INPUT_BUFSIZE];
    WCHAR               wzStringBuf[STRING_BUFSIZE];
    int                 nInput = 0;
 
    // get input from user to fill out presence info
    wprintf(L"Enter a number for what status you want to set for your current presence status\n"
            L"1. Out to lunch\n"
            L"2. Away\n"
            L"3. Be right back\n"
            L"4. Idle\n"
            L"5. Busy\n"
            L"6. On the phone\n"
            L"7. Online\n"
            L" [1-7]: ");

    GET_PROMPT_RESPONSE(hr, wzNumBuf);

    // since wtoi can't distinguish between 0 and an error, note that valid values start at 1, not 0    
    nInput = _wtoi(wzNumBuf);
    if (nInput < 1  || nInput > 7)
    {
        printf ("Invalid input, expected a number between 1 and 7.\n");
        hr = E_FAIL;
        goto exit;
    }

    // Convert the input value to a status value
    presInfo.status = (PEER_PRESENCE_STATUS) nInput;
    
    // enter presence status to set for yourself
    wprintf(L"Now input your detailed status as freeform text [none]: ");
    GET_PROMPT_RESPONSE(hr, wzStringBuf);

    // now set the string into the presence struct
    presInfo.pwzDescriptiveText = (LPWSTR) wzStringBuf;

    // now that we have the information we need in the struct, call the presence info api (which will make its own copy of the string)
    hr = PeerCollabSetPresenceInfo(&presInfo);

    if (SUCCEEDED(hr))
    {
        wprintf(L"PeerCollabSetPresenceInfo succeeded\n");
    }
    else
    {
        wprintf(L"Call to PeerCollabSetPresenceInfo failed. HRESULT=0x%x\n", hr);
        PrintError(hr);
    }

exit:
    return;
}
