/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) 1985-2005 Microsoft Corporation. All Rights Reserved.

Abstract:
    This C file includes sample code for publishing and resolving
    a peer name using the Peer Name Resolution Protocol APIs

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
#include "pnrp.h"

#define ERRBUFSIZE 1024
#define HRESULTTOWIN32(hres)                                \
            ((HRESULT_FACILITY(hres) == FACILITY_WIN32)     \
                ? HRESULT_CODE(hres)                        \
                : (hres))

#define FLUSH_AND_GET_RESPONSE(hr, wzBuff) \
    fflush(stdin); \
    hr = StringCbGets(wzBuff, sizeof(wzBuff));


// If Count is not already aligned, then round it up until it is.
//

// Global Handles
HANDLE          g_hAsyncResolve = NULL;
HANDLE          g_hRegistration = NULL;

// Global registration info - Since updating a peer name registration will
// overwrite all previous registration info, registration information
// must be stored so that a user does not have to reenter item that he
// or she does not wish to change during an update
PEER_PNRP_REGISTRATION_INFO     g_RegInfo = {0};
SOCKADDR_IN6    g_saRegAddrsV6[NUM_USER_DEFINED_ADDRESSES] = {0};
SOCKADDR_IN     g_saRegAddrsV4[NUM_USER_DEFINED_ADDRESSES] = {0};

WCHAR           g_wzCloudName[MAX_CLOUD_NAME] = {0};
WCHAR           g_wzComment[MAX_COMMENT_LENGTH] = {0};
WCHAR           g_PayloadData[MAX_PAYLOAD_LENGTH] = {0};
BOOL            g_fUseV6 = TRUE;


// Custom menu variables, types, and structs
// -------------------------------------

// This defines the signature for functions
// executed by menu selections
typedef HRESULT (*COMMAND)();

// This struct groups command titles (e.g. "Register Peer Name")
// with their corresponding function (e.g. RegisterPeerName)
typedef struct _MENU_COMMAND
{
    PWSTR       pwzTitle;
    COMMAND     pfnCommand;
} MENU_COMMAND, *PMENU_COMMAND;

// This menu lists commands (and their corresponding functions)
// when no peer name is currently registered
MENU_COMMAND g_MenuUnregistered[] =
{
    {L"Register Peer Name", RegisterPeerNameCommand},
    {L"Resolve Peer Name", ResolvePeerNameCommand},
    {L"Show DNS encoded name", ShowDNSEncodedNameCommand},
    {L"List Clouds", EnumCloudsCommand},
    {L"Exit", NULL}
};

// This menu lists commands (and their corresponding functions)
// when a peer name is currently registered
MENU_COMMAND g_MenuRegistered[] =
{
    {L"Update Registration", UpdateRegistrationCommand},
    {L"Delete Registration", UnregisterPeerNameCommand},
    {L"Resolve Peer Name", ResolvePeerNameCommand},
    {L"Show DNS encoded name", ShowDNSEncodedNameCommand},
    {L"List Clouds", EnumCloudsCommand},
    {L"Exit", NULL}
};

//-------------------------
// Registration Functions
//-------------------------
// The following functions involve filling out the PEER_PNRP_REGISTRATION_INFO
// struct, which has the following members:
//
//  PWSTR pwzCloudName     :  Name of cloud in which to register.  A value of
//                            NULL will register the peer name in all clouds.
//                            (PWSTR) PEER_PNRP_ALL_LINK_CLOUDS can be used to
//                            register in all LinkLocal clouds
//
//  PWSTR pwzPublishingIdentity : name of identity used to register (optional)
//
//  ULONG cAddresses       : Number of addresses to be registered.  Can also
//                           use value PEER_PNRP_AUTO_ADDRESSES to have the
//                           addresses automatically selected
//
//  SOCKADDR **ppAddresses : Array of pointers to SOCKADDR or SOCKADDR_IN6
//                           structs which contain information about the
//                           addresses used to register
//
//  WORD wPort             : Port number on which to register (optional)
//
//  PWSTR pwzComment       : User specified string that is retrievable by the
//                           resolver (optional)
//
//  PEER_DATA payload      : Exteneded info about peer (optional) that is also
//                           retrievable by the resolver.
//       ULONG cbdata          : Length of payload
//       PBYTE pbData          : Pointer to data
//
// NOTE: You may also call PeerPnrpRegister with NULL in place of this struct,
// in which case the peer name will be registered in all clouds, with default
// addresses and port, and no comment or payload.  You may not update a
// registration will a NULL registration info struct.  You do not need a
// PEER_PNRP_REGISTRATION_INFO struct to unregister a peer name.
//-------------------------

//-----------------------------------------------------------------------------
// Function:    PrintError
// Purpose:     Prints the error and the associated error string
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
// Function:    RegisterPeerNameCommand
// Purpose:     Prompts user for registration information, fills out a
//              PEER_PNRP_REGISTRATION_INFO struct, creates a peer name,
//              and calls PeerPnrpRegister to register the peer name
// Parameters:  None
//
// NOTE: The signature of this function must adhere to the MENU_COMMAND typedef
HRESULT RegisterPeerNameCommand()
{
    WCHAR                       wzInputBuffer[256] = {0};
    WCHAR                       wzIdentity[MAX_PEERNAME_LENGTH] = {0};
    WCHAR                       wzClassifier[MAX_CLASSIFIER_LENGTH] = {0};
    HRESULT                     hr = S_OK;
    PWSTR                       pwzPeerName = NULL;
    ULONG                       cAddresses = 0;
    //ppRegAddrs is a pointer to an array of pointers to SOCKADDR structures
    SOCKADDR**                  ppRegAddrs = NULL;

    ZeroMemory(&g_RegInfo, sizeof(PEER_PNRP_REGISTRATION_INFO));

    //Collect Registration Information
    //---------------------------------

    // Addresses
    hr = GetAddress(TRUE, &cAddresses, &ppRegAddrs);
    if (SUCCEEDED(hr))
    {
        // Cloud name
        hr = GetCloudName(cAddresses != 1, celems(g_wzCloudName), g_wzCloudName);
    }

    if (SUCCEEDED(hr))
    {
        // Comment
        hr = GetComment(celems(g_wzComment), g_wzComment);
    }

    if (SUCCEEDED(hr))
    {
        // Payload
        hr = GetPayload(sizeof(g_PayloadData), (PBYTE)g_PayloadData);

        if (cAddresses == 0)
        {
            // If no addresses were specified, the payload must be non-null.
            while (PayloadSize(g_PayloadData) == 0 && SUCCEEDED(hr))
            {
                wprintf(L"Payload must be non-empty if no addresses are specified.\n");
                hr = GetPayload(sizeof(g_PayloadData), (PBYTE)g_PayloadData);
            }
        }
    }

    // Create Peer Name
    //------------------

    // If secure, get identity
    if (SUCCEEDED(hr))
    {
        wprintf(L"Secured peer name [no]: ");
        FLUSH_AND_GET_RESPONSE(hr, wzInputBuffer);

        if (SUCCEEDED(hr) && _wcsicmp(wzInputBuffer, L"yes") == 0)
        {
            hr = GetIdentity(celems(wzIdentity), wzIdentity);
        }
    }
    // Classifier
    if (SUCCEEDED(hr))
    {
        do
        {
            wprintf(L"Classifier: ");
            FLUSH_AND_GET_RESPONSE(hr, wzClassifier);

            // Classifier must be non-null.
            if (SUCCEEDED(hr) && wzClassifier[0] == L'\0')
            {
                wprintf(L"Classifier must be non-empty.\n");
            }
        } while (SUCCEEDED(hr) && wzClassifier[0] == L'\0');
    }

    // Create peer name
    if (SUCCEEDED(hr))
    {
        if (wcslen(wzIdentity) != 0)
        {
            // Create secured peer name
            hr = PeerCreatePeerName(wzIdentity, wzClassifier, &pwzPeerName);
        }
        else
        {
            // Create unsecured peer name
            hr = PeerCreatePeerName(NULL, wzClassifier, &pwzPeerName);
        }
    }

    // Fill out g_RegInfo struct
    // -------------------------
    if (wcslen(g_wzCloudName) == 0)
    {
        g_RegInfo.pwzCloudName = NULL;
    }
    else
    {
        g_RegInfo.pwzCloudName = g_wzCloudName;
    }

    //Copy in the address count and pointer to the array of pointers
    g_RegInfo.cAddresses = cAddresses;
    g_RegInfo.ppAddresses = ppRegAddrs;

    g_RegInfo.pwzComment = g_wzComment;

    if (PayloadSize(g_PayloadData) == 0)
    {
        g_RegInfo.payload.cbData = 0;
        g_RegInfo.payload.pbData = NULL;
    }
    else
    {
        g_RegInfo.payload.cbData = PayloadSize(g_PayloadData);
        g_RegInfo.payload.pbData = (PBYTE) g_PayloadData;
    }

    // Perform registration
    if (SUCCEEDED(hr))
    {
        hr = PeerPnrpRegister(pwzPeerName, &g_RegInfo, &g_hRegistration);
    }

    // Display result
    if (SUCCEEDED(hr))
    {
        wprintf(L"\nSuccessfully registered name: %s\n", pwzPeerName);
    }
    else
    {
        wprintf(L"\nError while registering name.  HRESULT=0x%x\n", hr);
        PrintError(hr);
    }

    if (pwzPeerName != NULL)
    {
        PeerFreeData(pwzPeerName);
    }

    if (ppRegAddrs != NULL)
    {
        free(ppRegAddrs);
    }
    return hr;
}

//-----------------------------------------------------------------------------
// Function:    ResolvePnrpNameCommand
// Purpose:     Asynchronously or synchronously resolves a user given peer name
//              using PNRP in a user selected cloud (or clouds).
// Parameters:  None
//
// NOTE: The signature of this function must adhere to the MENU_COMMAND typedef
HRESULT ResolvePeerNameCommand()
{
    HRESULT                 hr = S_OK;
    WCHAR                   wzInputBuffer[MAX_PEERNAME_LENGTH];
    BOOL                    fSynch = TRUE;
    WCHAR                   wzCloudName[MAX_CLOUD_NAME];

    wprintf(L"\nNOTE:  You cannot successfully resolve names registered by this process.\n");
    wprintf(L"       If you wish to test PNRP locally start another instance of this sample\n");
    wprintf(L"       application.\n");

    // Prompts user to select a cloud in which to resolve
    hr = GetCloudName(TRUE, celems(wzCloudName), wzCloudName);

    // Asks user to select a synchronous or asynchronous resolve
    if (SUCCEEDED(hr))
    {
        wprintf(L"Synchronous resolve [no]: ");
        FLUSH_AND_GET_RESPONSE(hr, wzInputBuffer);

        if (_wcsicmp(wzInputBuffer, L"yes") != 0)
        {
            fSynch = FALSE;
        }

        do
        {
            wprintf(L"Peer Name (e.g. 0.myclassifier): ");
            FLUSH_AND_GET_RESPONSE(hr, wzInputBuffer);

            // Peer Name must be non-null.
            if (SUCCEEDED(hr) && wzInputBuffer[0] == L'\0')
            {
                wprintf(L"Peer Name must be non-empty.\n");
            }
        } while (SUCCEEDED(hr) && wzInputBuffer[0] == L'\0');

    }

    if (SUCCEEDED(hr))
    {
        if (fSynch)
        {
            hr = SyncPeerNameResolve(wzInputBuffer, wzCloudName);
        }
        else
        {
            hr = AsyncPeerNameResolve(wzInputBuffer, wzCloudName);
        }
    }
    return hr;
}

//-----------------------------------------------------------------------------
// Function:    ShowDNSEncodedNameCommand
// Purpose:     Given a PNRP encoded name show the equivalant DNS encoded name to
//              pass to GetAddrInfo to resolve this name.  The DNS encoded name
//              can be used to lookup a PNRP name from ANY application that 
//              calls GetAddrInfo (which means almost every application that
//              converts a DNS name to an IP address).  Try it from ping.exe or
//              Internet Explorer.
// Parameters:  None
//
// NOTE: The signature of this function must adhere to the MENU_COMMAND typedef
HRESULT ShowDNSEncodedNameCommand()
{
    HRESULT                 hr = S_OK;
    WCHAR                   wzInputBuffer[MAX_PEERNAME_LENGTH];
    PWSTR                   pwzHostName = NULL;

    do
    {
        wprintf(L"Peer Name (e.g. 0.myclassifier): ");
        FLUSH_AND_GET_RESPONSE(hr, wzInputBuffer);

        // Peer Name must be non-null.
        if (SUCCEEDED(hr) && wzInputBuffer[0] == L'\0')
        {
            wprintf(L"Peer Name must be non-empty.\n");
        }
    } while (SUCCEEDED(hr) && wzInputBuffer[0] == L'\0');

    hr = PeerNameToPeerHostName(wzInputBuffer, &pwzHostName);

    if(FAILED(hr))
    {
        goto exit;
    }

    wprintf(L"\n");
    wprintf(L"DNS encoded name is: %s\n", pwzHostName);
    wprintf(L"\n");
    wprintf(L"The DNS encoded name allows PNRP names to be used in ANY application\n");
    wprintf(L"that calls GetAddrInfo to perform name resolution (and that is just about\n");
    wprintf(L"every application).\n");
    wprintf(L"\n");
    wprintf(L"If this name is registered try using the DNS encoded form of the name in\n");
    wprintf(L"ping or Internet Explorer.");
    wprintf(L"\n\n");

exit:
    PeerFreeData(pwzHostName);
    pwzHostName = NULL;
    return hr;
}

//-----------------------------------------------------------------------------
// Function:    UpdateRegistrationCommand
// Purpose:     Updates peer name registration information
// Parameters:  None
//
// NOTE: The signature of this function must adhere to the MENU_COMMAND typedef
HRESULT UpdateRegistrationCommand()
{
    HRESULT     hr = S_OK;
    //ppRegAddrs is a pointer to an array of pointers to SOCKADDR structures
    SOCKADDR**  ppRegAddrs = NULL;
    ULONG       cAddresses = 0;

    // NOTE: You may not update cAddresses to or from
    // PEER_PNRP_AUTO_ADDRESSES - this can only be specified at the
    // initial registration
    if (g_RegInfo.cAddresses != PEER_PNRP_AUTO_ADDRESSES)
    {
        // Get addresses
        hr = GetAddress(FALSE, &cAddresses, &ppRegAddrs);

        if (SUCCEEDED(hr))
        {
            //Copy in the address count and pointer to the array of pointers
            g_RegInfo.cAddresses = cAddresses;
            g_RegInfo.ppAddresses = ppRegAddrs;
        }
    }

    // Update the comment - since the pointers in the g_RegInfo struct
    // already point to g_wzComment, the comment in g_RegInfo will be
    // implicitly updated
    if (SUCCEEDED(hr))
    {
        hr = GetComment(celems(g_wzComment), g_wzComment);
    }

    // Update the payload.  If no addresses are specified, ensure that the
    // payload is non-empty
    if (SUCCEEDED(hr))
    {
        hr = GetPayload(sizeof(g_PayloadData), (PBYTE)g_PayloadData);
        if ((g_RegInfo.cAddresses == 0) && SUCCEEDED(hr))
        {
            while(PayloadSize(g_PayloadData) == 0 && SUCCEEDED(hr))
            {
                wprintf(L"Payload must be non-empty if no addresses are specified.\n");
                hr = GetPayload(sizeof(g_PayloadData), (PBYTE)g_PayloadData);
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        if (PayloadSize(g_PayloadData) == 0)
        {
            g_RegInfo.payload.cbData = 0;
            g_RegInfo.payload.pbData = NULL;
        }
        else
        {
            g_RegInfo.payload.cbData = PayloadSize(g_PayloadData);
            g_RegInfo.payload.pbData = (PBYTE) g_PayloadData;
        }
    }

    // Perform the update and return the result
    if (SUCCEEDED(hr))
    {
        hr = PeerPnrpUpdateRegistration(g_hRegistration, &g_RegInfo);
    }

    if (SUCCEEDED(hr))
    {
        wprintf(L"Registration successfully updated\n");
    }
    else
    {
        wprintf(L"Error while updating registration.\n");
    }

    if (ppRegAddrs != NULL)
    {
        free(ppRegAddrs);
    }
    return hr;
}

//-----------------------------------------------------------------------------
// Function:    UnregisterPeerNameCommand
// Purpose:     Unregisters the pnrp name
// Parameters:  None
//
// NOTE: The signature of this function must adhere to the MENU_COMMAND typedef
HRESULT UnregisterPeerNameCommand()
{
    HRESULT hr = S_OK;

    hr = PeerPnrpUnregister(g_hRegistration);
    if (SUCCEEDED(hr))
    {
        wprintf(L"Registration successfully deleted\n");
        g_hRegistration = NULL;
    }
    else
    {
        wprintf(L"Registration could not be deleted\n");
    }
    return hr;
}

//--------------------------------------------------
// Helper Functions
//--------------------------------------------------

//-----------------------------------------------------------------------------
// Function:    SyncPeerNameResolve
// Purpose:     Wrapper function for PeerPnrpResolve that perform the
//              resolution synchronously
// Parameters:
//   pwzPeerName [in]    : Peer Name to resolve
//   pwzCloudName [in]   : Cloud name in which to resolve
//
HRESULT SyncPeerNameResolve(PCWSTR pwzPeerName, PCWSTR pwzCloudName)
{
    ULONG                    i = 0;
    HRESULT                  hr = S_OK;
    PPEER_PNRP_ENDPOINT_INFO pEndpointInfo = NULL;
    ULONG                    cEndpoints = MAX_ENDPOINTS_TO_RESOLVE;

    wprintf(L"Resolving (this may take a few seconds)...\n");
    if (wcslen(pwzCloudName) != 0)
    {
        // Perform a synchronous resolve
        hr = PeerPnrpResolve(pwzPeerName, pwzCloudName, &cEndpoints,
                              &pEndpointInfo);
    }
    else
    {
        // If no cloud name is given, search in all clouds (NULL argument)
        hr = PeerPnrpResolve(pwzPeerName, NULL, &cEndpoints, &pEndpointInfo);
    }

    if (SUCCEEDED(hr))
    {
        // Display results
        if (cEndpoints == 0)
        {
            wprintf(L"No endpoints found for peer name %s\n", pwzPeerName);
        }
        else
        {
            for (i = 0; i < cEndpoints; i++)
            {
                DisplayPNRPEndpoint(&pEndpointInfo[i]);
            }
        }
    }

    if (pEndpointInfo != NULL)
    {
        PeerFreeData(pEndpointInfo);
    }
    wprintf(L"Synchronous resolve complete.\n");
    return hr;
}

//-----------------------------------------------------------------------------
// Function:    AsyncPeerNameResolve
// Purpose:     Wrapper function for PeerPnrpResolve that perform the
//              resolution asynchronously
// Parameters:
//   pwzPeerName [in]    : Peer Name to resolve
//   pwzCloudName [in]   : Cloud name in which to resolve
//
HRESULT AsyncPeerNameResolve(PCWSTR pwzPeerName, PCWSTR pwzCloudName)
{
    HRESULT                  hr = S_OK;
    PPEER_PNRP_ENDPOINT_INFO pEndpointInfo = NULL;
    ULONG                    cEndpoints = MAX_ENDPOINTS_TO_RESOLVE;
    HANDLE                   hEvent;
    HANDLE                   hResolve;

    wprintf(L"Resolving (this may take a few seconds)...\n");

    hEvent = CreateEvent(0, FALSE, FALSE, 0);
    if (hEvent == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }
    if (wcslen(pwzCloudName) != 0)
    {
        // Start an asynchronous resolve
        hr = PeerPnrpStartResolve(pwzPeerName, pwzCloudName, cEndpoints,
                                            hEvent, &hResolve);
    }
    else
    {
        // If no cloud name is given, search in all clouds (NULL argument)
        hr = PeerPnrpStartResolve(pwzPeerName, NULL, cEndpoints,
                                            hEvent, &hResolve);
    }

    cEndpoints = 0;
    if (SUCCEEDED(hr))
    {
        // Waits for up to 10 seconds for an endpoint to return
        while (WaitForSingleObject(hEvent, INFINITE) == WAIT_OBJECT_0)
        {
            hr = PeerPnrpGetEndpoint(hResolve, &pEndpointInfo);
            if (hr == PEER_E_NO_MORE)
            {
                // Exit if all endpoints have been retrieved
                break;
            }
            else if (SUCCEEDED(hr))
            {
                // Display results as they are received
                hr = DisplayPNRPEndpoint(pEndpointInfo);
                if (FAILED(hr))
                {
                    wprintf(L"Error in DisplayEndpointInfo\n");
                }
                cEndpoints++;

                if (pEndpointInfo != NULL)
                {
                    PeerFreeData(pEndpointInfo);
                }
            }    
            else
            {
                break;
            }
        }
        if (cEndpoints == 0)
        {
            wprintf(L"No endpoints found for peer name %s\n", pwzPeerName);
        }

        // End Asynchronous Resolve
        hr = PeerPnrpEndResolve(hResolve);
        wprintf(L"Asynchronous resolve complete.\n");
    }

    CloseHandle(hEvent);
    return hr;
}

//-----------------------------------------------------------------------------
// Function:    GetAddress
// Purpose:     Prompts the user for a series of addresses
//              Each address is converted to a SOCKADDR_IN6
//              struct, and if that fails to a SOCKADDR_IN struct.
//
// Parameters:
//   fAllowDefault [in]  : controls whether we allow the user to use AUTO_ADDRESSES
//   pcAddresses [out]   : pointer to ULONG where the number of addresses will
//                         be returned.
//   ppRegAddrOut [out]  : pointer to where to write a pointer to an array of SOCKADDR
//                         pointers.  The caller is responsible for freeing this pointer
//                         (the array of pointers and the structures they point to
//                          are all allocated in a single heap block.  Only call
//                          free on the ppRegAddrOut pointer).
//
HRESULT GetAddress(BOOL fAllowDefault, __out ULONG* pcAddresses, __out_ecount(pcAddresses) SOCKADDR ***ppRegAddrOut)
{
    INT       iAddrSizeV6 = sizeof(SOCKADDR_IN6);
    INT       iAddrSizeV4 = sizeof(SOCKADDR_IN);
    WCHAR     wzInputBuffer[256] = {0};
    INT       cNumAddresses = 0;
    INT       nCurrentAddress = 0;
    //We need to construct an array of pointers to SOCKADDR structures.
    //ppRegAddrs will be a pointer to the array of pointers
    SOCKADDR **ppRegAddrs = NULL;

    HRESULT     hr = S_OK;

    // Prompt user to enter an address - although PNRP supports IPv4 addresses,
    // this sample only supported IPv6 addresses.  It is possible to create a
    // registration with no addresses, but this requires specifying a non-null
    // payload

    if (fAllowDefault)
    {
        wprintf(L"Use default address [yes]: ");
        FLUSH_AND_GET_RESPONSE(hr, wzInputBuffer);
        if (_wcsicmp(wzInputBuffer, L"no") != 0)
        {
            wprintf(L"Default address(es) selected.\n");
            cNumAddresses =  PEER_PNRP_AUTO_ADDRESSES;
        }
    }

    if (cNumAddresses != PEER_PNRP_AUTO_ADDRESSES)
    {
        do
        {
            wprintf(L"Enter Number of Addresses to register: ");
            FLUSH_AND_GET_RESPONSE(hr, wzInputBuffer);
            if (FAILED(hr))
            {
                goto exit;
            }
            
            cNumAddresses = _wtoi(wzInputBuffer);

            if (cNumAddresses<0)
            {
                wprintf(L"Number of Addresses must be >= 0.\n");
            }
            else if (cNumAddresses==0)
            {
                wprintf(L"Registering 0 addresses.  Payload is mandatory.\n");
            }
        } while (cNumAddresses < 0);

        if (cNumAddresses == 0)
        {
        }
        else
        {
            //Alocate enough space for our array of pointers and then
            //the following array of structures
            ppRegAddrs = (SOCKADDR**) malloc( sizeof(SOCKADDR*)*cNumAddresses + sizeof(SOCKADDR_IN6)*cNumAddresses);

            for (nCurrentAddress = 0; nCurrentAddress < cNumAddresses;)
            {
                //Set the pointer ppRegAddrs[nCurrentAddress] to point to the address of the
                //nCurrentAddress structure
                ppRegAddrs[nCurrentAddress] = (SOCKADDR *)(((SOCKADDR_IN6 *)(ppRegAddrs + cNumAddresses))+ nCurrentAddress);

                wprintf(L"Enter Address #%d\n", nCurrentAddress+1);
                wprintf(L"  IPV6 notation: e.g. 2001:0db8::3428:23ce\n");
                wprintf(L"  IPV4 notation: e.g. 192.168.1.50\n");
                wprintf(L"  > ");
                FLUSH_AND_GET_RESPONSE(hr, wzInputBuffer);

                if (FAILED(hr))
                {
                    goto exit;
                }

                if (wcscmp(wzInputBuffer, L"") == 0)
                {
                    wprintf(L"Address may not be empty.");
                    continue;
                }

                // Try parsing string as V6 address first
                hr = WSAStringToAddressW(wzInputBuffer, AF_INET6,
                       NULL, (LPSOCKADDR) ppRegAddrs[nCurrentAddress],
                       (LPINT) &iAddrSizeV6);

                if (FAILED(hr))
                {
                    // Now try parsing as V4 address
                    hr = WSAStringToAddressW(wzInputBuffer, AF_INET,
                           NULL, (LPSOCKADDR) ppRegAddrs[nCurrentAddress],
                           (LPINT) &iAddrSizeV4);
                    
                    if (SUCCEEDED(hr))
                    {
                        g_fUseV6 = FALSE;
                    }
                    else
                    {
                        hr = HRESULT_FROM_WIN32(WSAGetLastError());
                        wprintf(L"WSAStringToAddressW failed\n");
                    }
                }
                else
                {
                    g_fUseV6 = TRUE;
                }


                //Move onto the next address
                nCurrentAddress++;
            }
        }
    }

exit:
    if (SUCCEEDED(hr))
    {
        *pcAddresses = cNumAddresses;
        *ppRegAddrOut = ppRegAddrs;
        ppRegAddrs = NULL;
    }
    return hr;
}

//-----------------------------------------------------------------------------
// Function:    GetComment
// Purpose:     Takes a string from the user and stores it at the location
//              given by pwzComment.  Returns HRESULT of the result
// Parameters:
//   pwzComment [out] : pointer to location where comment should be copied
//
HRESULT GetComment(__in ULONG cchComment, __out_ecount(cchComment) PWSTR pwzComment)
{
    HRESULT     hr = S_OK;

    wprintf(L"Comment [none]: ");
    fflush(stdin);
    hr = StringCbGets(pwzComment, cchComment);

    return hr;
}

//-----------------------------------------------------------------------------
// Function:    GetPayload
// Purpose:     Copies user input into the location specified by pbPayloadData
// Parameters:
//   pPayloadData [out]:pointer to location where payload data should be copied
//
HRESULT GetPayload(ULONG cbPayloadDataSize,
                   __out_ecount(cbPayloadDataSize) PBYTE pbPayloadData)
{
    HRESULT hr = S_OK;

    wprintf(L"Payload data [none]: ");

    fflush(stdin);
    hr = StringCbGets((WCHAR*) pbPayloadData, cbPayloadDataSize);
    if (FAILED(hr))
    {
        ZeroMemory(pbPayloadData, cbPayloadDataSize);
    } 

    return hr;
}

//-----------------------------------------------------------------------------
// Function:    GetIdentity
// Purpose:     Displays a list of available identities to the user, prompts
//              for selection, and copies the selected identity's name to
//              the parameter pwzIdentity
// Parameters:
//   pwzIdentity [out] : pointer to string where retrieved identity's peer
//                       name will be stored
//
HRESULT GetIdentity(ULONG cchIdentity, __out_ecount(cchIdentity) PWSTR pwzIdentity)
{
    HPEERENUM                   hIdentityEnum = NULL;
    ULONG                       cIdentities = 0;
    ULONG                       ulRegisterIdentity = 0;
    PEER_NAME_PAIR**            ppNamePairs = NULL;
    WCHAR                       wzInputBuffer[256] = {0};
    HRESULT                     hr = S_OK;
    PWSTR                       pwzDefaultIdentity = NULL;

    printf("Use default identity? [yes]: ");
    FLUSH_AND_GET_RESPONSE(hr, wzInputBuffer);
    if (SUCCEEDED(hr) && _wcsicmp(wzInputBuffer, L"no") == 0)
    {
        // Retreieve list of available identities
        hr = PeerEnumIdentities(&hIdentityEnum);
        if (SUCCEEDED(hr))
        {
            hr = PeerGetItemCount(hIdentityEnum, &cIdentities);

            if (SUCCEEDED(hr))
            {
                if (cIdentities == 0)
                {
                    wprintf(L"No identities found on this machine\n");
                    hr = E_FAIL;
                }
                else
                {
                    hr = PeerGetNextItem(hIdentityEnum, &cIdentities, (PVOID **) &ppNamePairs);
                }
            }
            PeerEndEnumeration(hIdentityEnum);
        }

        // Display list of available identities
        if (SUCCEEDED(hr))
        {
            ULONG i;

            wprintf(L"\n");
            for (i = 0; i < cIdentities; i++)
            {
                wprintf(L"%d) %s\n", i, ppNamePairs[i]->pwzFriendlyName);
            }

            // Retrieve user selection
            wprintf(L"Identity to register: ");
            FLUSH_AND_GET_RESPONSE(hr, wzInputBuffer);

            if (SUCCEEDED(hr))
            {
                ulRegisterIdentity = _wtoi(wzInputBuffer);

                while (ulRegisterIdentity >= cIdentities)
                {
                    wprintf(L"Invalid Selection\nSelect Identity: ");
                    FLUSH_AND_GET_RESPONSE(hr, wzInputBuffer);
                    if (SUCCEEDED(hr))
                    {
                        ulRegisterIdentity = _wtoi(wzInputBuffer);
                    }
                    else
                    {
                        break;
                    }
                }

                if (SUCCEEDED(hr))
                {
                    hr = StringCchCopy(pwzIdentity, cchIdentity,
                                       ppNamePairs[ulRegisterIdentity]->pwzPeerName);
                }
            }
        }
    }
    else
    {
        // Retrieve the Default Identity for this user
        hr = PeerIdentityGetDefault(&pwzDefaultIdentity);

        if (SUCCEEDED(hr))
        {
            hr = StringCchCopy(pwzIdentity, cchIdentity, pwzDefaultIdentity);
        }
        else
        {
            printf("Failed to get default identity.  HRESULT=0x%x\n", hr);
            PrintError(hr);
        }
    }

    PeerFreeData(ppNamePairs);
    PeerFreeData(pwzDefaultIdentity);

    return hr;
}

//-----------------------------------------------------------------------------
// Function:    GetCloudName
// Purpose:     Prompts users to select a cloud (or set of clouds) from a list
//              of available clouds and returns the name of the selected cloud
// Parameters:
//  pwzCloudName [out] : pointer to string buffer to which cloud name is copied
//
// Returns:     HRESULT
//
HRESULT GetCloudName(BOOL fAllowAll, __in ULONG cchCloudName, __out_ecount(cchCloudName) PWSTR pwzCloudName)
{
    HRESULT                 hr = S_OK;
    PPEER_PNRP_CLOUD_INFO   pCloudInfo = {0};
    ULONG                   ulCloud = 0;
    ULONG                   cClouds = 0;
    WCHAR                   wzInputBuffer[256] = {0};

    hr = PeerPnrpGetCloudInfo(&cClouds, &pCloudInfo);
    if (SUCCEEDED(hr))
    {
        ULONG i;

        // Options include being able to select all available clouds,
        // all Link Local clouds, or any individual cloud
        wprintf(L"\nClouds:\n0) All Clouds\n1) All Link Local Clouds\n");
        for (i = 0; i < cClouds; i++)
        {
            wprintf(L"%d) %s\n", i+2, pCloudInfo[i].pwzCloudName);
        }

        // Retrieve and verify user selection
        wprintf(L"Select cloud [all clouds]: ");
        FLUSH_AND_GET_RESPONSE(hr, wzInputBuffer);
        if (SUCCEEDED(hr))
        {
            ulCloud = _wtoi(wzInputBuffer);

            while(ulCloud > cClouds + 1)
            {
                wprintf(L"Invalid Selection\nSelect Cloud [all clouds]: ");
                FLUSH_AND_GET_RESPONSE(hr, wzInputBuffer);
                if (SUCCEEDED(hr))
                {
                    ulCloud = _wtoi(wzInputBuffer);
                }
                else
                {
                    break;
                }
            }
        }

        if (!fAllowAll && ulCloud < 2)
        {
            printf("When an address is manually specified, you must also explicitly say which\n"
                   "cloud you wish to register in.\n");
            PeerFreeData(pCloudInfo);
            return E_INVALIDARG;
        }

        if (ulCloud == 0)       // All Clouds
        {
            pwzCloudName[0] = (WCHAR) 0;
        }
        else if (ulCloud == 1)     // All Link Local clouds
        {
            hr = StringCchCopy(pwzCloudName, cchCloudName,
                     PEER_PNRP_ALL_LINK_CLOUDS);
        }
        else
        {
            hr = StringCchCopy(pwzCloudName, cchCloudName,
                     pCloudInfo[ulCloud-2].pwzCloudName);
        }
        PeerFreeData(pCloudInfo);
    }
    return hr;
}

//-----------------------------------------------------------------------------
// Function:    PayloadSize
// Purpose:     Examines a WCHAR object and returns its length.  NOTE: In this
//              sample, payload data is implemented as a string.  To use other
//              types of data, you will need to modify this function to receive
//              the appropriate data type and calculate its size accordingly
//
// Parameters:
//   pPayloadData [in] : pointer to WCHAR object
//
// Returns:    length of WCHAR string
//
ULONG PayloadSize(PWSTR pPayloadData)
{
    ULONG  cbPayload = 0;

    cbPayload = (ULONG) wcslen(pPayloadData) * sizeof(WCHAR);

    // Add sizeof(WCHAR) to account for NULL terminator.
    if (cbPayload != 0)
    {
        cbPayload = cbPayload + sizeof(WCHAR);
    }

    return cbPayload;
}

//-----------------------------------------------------------------------------
// Function:    ValidatePayloadData
// Purpose:     Examines a PEER_DATA object and determines if the contents are
//              displayable to the screen.  NOTE: No guarantees are made about
//              the format of payload data.  Therefore, you should alway
//              verify payload data from a PNRP endpoint before using it.
//
// Parameters:
//   pEndpoint [in] : pointer to PEER_DATA object
//
// Returns:    TRUE if the PEER_DATA object contains data displayable to the
//             screen, FALSE otherwise
//
BOOL VerifyPayloadData(__in PPEER_DATA pData)
{
    PWSTR pwzTemp = (PWSTR) pData->pbData;
    ULONG cchTemp = pData->cbData / sizeof(WCHAR);
    ULONG i = 0;

    for (i = 0; i < cchTemp-1; i++)
    {
        if (!iswprint(pwzTemp[i]))
        {
            return FALSE;
        }
    }
    if (pwzTemp[cchTemp-1] != L'\0')
    {
        return FALSE;
    }
    return TRUE;
}
//-------------------------------------------------
// Display Functions - present information to user
//-------------------------------------------------

//-----------------------------------------------------------------------------
// Function:    DisplayPNRPEndpoint
// Purpose:     Prints to screen information about a pnrp endpoint
// Parameters:
//   pEndpoint [in] : pointer to PEER_PNRP_ENDPOINT_INFO object
//
HRESULT DisplayPNRPEndpoint(__in PPEER_PNRP_ENDPOINT_INFO pEndpoint)
{
    HRESULT hr = S_OK;
    ULONG   i = 0;
    WCHAR   wzAddr[MAX_ADDR_LENGTH];

    // Print out the endpoint's name
    wprintf(L"\nPeer Name:\t%s\n", pEndpoint->pwzPeerName);

    // Display associated addresses
    for (i = 0; i < pEndpoint->cAddresses; i++)
    {
        DWORD dwLen = (sizeof(wzAddr) / sizeof(wzAddr[0]));

        hr = WSAAddressToString(
                (LPSOCKADDR) pEndpoint->ppAddresses[i],
                sizeof(SOCKADDR_IN6), NULL, wzAddr, &dwLen);

        if (SUCCEEDED(hr))
        {
            if (i == 0)
            {
                wprintf(L"Addresses:\t%s\n", wzAddr);
            }
            else
            {
                wprintf(L"\t\t%s\n", wzAddr);
            }
        }
    }

    // Display comment and info on extended payload.  For the purposes
    // of this sample, payload data is assumed to be a string.
    if (pEndpoint->pwzComment)
    {
        wprintf(L"Comment:\t%s\n", pEndpoint->pwzComment);
    }
    if (pEndpoint->payload.cbData == 0)
    {
        wprintf(L"Payload:\tNo\n");
    }
    // Always verify payload data before using it
    else
    {
        if (VerifyPayloadData(&pEndpoint->payload))
        {
            wprintf(L"Payload:\tYes (Data: %s)\n",
                (PWSTR) pEndpoint->payload.pbData);
        }
        else
        {
            wprintf(L"Payload:\tYes (Data undisplayable)\n");
        }
    }
    return hr;
}

//-----------------------------------------------------------------------------
// Function:    EnumCloudsCommand
// Purpose:     Prints names of all available clouds to screen
// Parameters:  None
//
// NOTE: The signature of this function must adhere to the MENU_COMMAND typedef
HRESULT EnumCloudsCommand()
{
    HRESULT     hr = S_OK;
    ULONG       cClouds;
    PPEER_PNRP_CLOUD_INFO   pCloudInfo;

    hr = PeerPnrpGetCloudInfo(&cClouds, &pCloudInfo);
    if (SUCCEEDED(hr))
    {
        ULONG i;
        wprintf(L"Clouds:\n");
        for (i = 0; i < cClouds; i++)
        {
            wprintf(L"   %s\n", pCloudInfo[i].pwzCloudName);
        }

        PeerFreeData(pCloudInfo);
    }
    return hr;
}



//-----------------------------------------------------------------------------
// Function:    PrintMenu
// Purpose:     Prints a menu of options to the user
// Parameters:  None
//
void PrintMenu()
{
    ULONG i = 0;
    PMENU_COMMAND pMenuOptions;

    if (g_hRegistration != NULL)
    {
        pMenuOptions = g_MenuRegistered;
    }
    else
    {
        pMenuOptions = g_MenuUnregistered;
    }

    for(i = 0; pMenuOptions[i].pfnCommand != NULL; i++)
    {
            wprintf(L"\n%d. %s", i+1, pMenuOptions[i].pwzTitle);
    }
    wprintf(L"\n%d. %s\n> ", i+1, pMenuOptions[i].pwzTitle);
}

//-----------------------------------------------------------------------------
// Function:    main
// Purpose:     Program entry point
// Parameters:
//      argc  :  count of program arguments
//      argv  :  program arguments
//

int __cdecl main(int argc, __in_ecount(argc) char *argv[])
{
    HRESULT     hr = S_OK;
    WCHAR       wzBuff[5] = {0};
    WSADATA     wsaData = {0};
    ULONG       ulOption = 0;
    ULONG       cOptions = 0;
    INT         iWinsockResult = 0;
    DWORD       dwErr = 0;
    PMENU_COMMAND pMenu = NULL;
    BOOL        fLoop = TRUE;

    //Unreferenced parameters
    argc;
    argv;

    // Setup Winsock
    iWinsockResult=WSAStartup(0x0002, &wsaData);
    if (iWinsockResult != 0)
    {
        dwErr = WSAGetLastError();
        wprintf(L"Winsock error %d\n", dwErr);
        return 0;
    }

    // Setup PNRP infrastructure
    hr = PeerPnrpStartup(PNRP_VERSION);
    if (FAILED(hr))
    {
        wprintf(L"Error starting up PNRP\nHRESULT=0x%x\n", hr);
        PrintError(hr);
    }

    while (fLoop)
    {
        PrintMenu();
        FLUSH_AND_GET_RESPONSE(hr, wzBuff);
        ulOption = _wtoi(wzBuff) - 1;

        if (g_hRegistration != NULL)
        {
            pMenu = g_MenuRegistered;
            cOptions = celems(g_MenuRegistered);
        }
        else
        {
            pMenu = g_MenuUnregistered;
            cOptions = celems(g_MenuUnregistered);
        }

        if (ulOption >= cOptions)
        {
            wprintf(L"Invalid selection.\n");
        }
        else if (pMenu[ulOption].pfnCommand == NULL)
        {
            fLoop = FALSE;
        }
        else
        {
            // Calls the function indicated by the
            // selected element of the MENU_COMMAND array
            hr = (*pMenu[ulOption].pfnCommand)();
            if (FAILED(hr))
            {
                wprintf(L"Error: HRESULT=0x%x\n", hr);
                PrintError(hr);
            }

        }
    }

    PeerPnrpShutdown();
    WSACleanup();
}
