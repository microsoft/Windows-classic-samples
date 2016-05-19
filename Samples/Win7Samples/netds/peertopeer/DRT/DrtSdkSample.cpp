// DrtSdkSample.cpp : Defines the entry point for the console application.

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#include "DrtSdkSample.h"
#include "CAPIWrappers.h"
#include "CustomBootstrapper.h"
#include "CustomSecurityProvider.h"
#include "FirewallConfig.h"

#pragma comment(lib,"drt")
#pragma comment(lib,"drtprov")
#pragma comment(lib,"drttransport")

//Global variable to enable the asynchronous display of DRT Events
BOOL g_DisplayEvents = false;


//********************************************************************************************
// Function: FlushCurrentLine
//
// Description: Clears any input lingering in the STDIN buffer
//
//********************************************************************************************
void FlushCurrentLine()
{
    int i;
    while((i = getc(stdin)) != EOF && i != '\n')
        continue;
}


//********************************************************************************************
// Function: GetUserChoice
//
// Description: Presents an interactive menu to the user and returns the user's choice
//
// Input: PCWSTR *choices - An array of strings representing the choices to be presented to 
//                          the users
//
//********************************************************************************************
int GetUserChoice(PCWSTR *choices, int numchoices)
{
   int chr = -1;
   wprintf(L"---------------------------------------------------------\n");
   for(int i=0; i<numchoices; i++)
   {
      wprintf(L"   %i. %s\n",i+1,choices[i]);
   }
   wprintf(L"---------------------------------------------------------\n");
   wprintf(L"Enter a choice (1-%i): ",numchoices);
   wscanf_s(L"%i",&chr);
   FlushCurrentLine();
   if(chr > 0 && chr <= numchoices)
   {
      return chr - 1;
   }
   else
   {
      wprintf(L"Invalid Choice\n");
      return -1;
   }
}

//********************************************************************************************
// Function: DisplayError
//
// Description: Maps common HRESULTs to descriptive error strings
//
//********************************************************************************************
void DisplayError(__in const PWSTR fnname, __in const HRESULT hr)
{
   struct error_description
   {
      HRESULT hr;
      PWSTR description;
   } valid_errors[] = 
   {
      {S_OK,                                L"Succeeded: S_OK"},
      {E_INVALIDARG,                        L"Error: E_INVALIDARG"},
      {E_OUTOFMEMORY,                       L"Error: E_OUTOFMEMORY"},
      {ERROR_INVALID_HANDLE,                L"Error: ERROR_INVALID_HANDLE"},
      {ERROR_INVALID_PARAMETER,             L"Error: ERROR_INVALID_PARAMETER"},
      {ERROR_ADAP_HDW_ERR,                  L"Error: ERROR_ADAP_HDW_ERR"},
      {DRT_E_TIMEOUT,                       L"Error: DRT_E_TIMEOUT"},
      {DRT_E_INVALID_KEY_SIZE,              L"Error: DRT_E_INVALID_KEY_SIZE"},
      {DRT_E_INVALID_CERT_CHAIN,            L"Error: DRT_E_INVALID_CERT_CHAIN"},
      {DRT_E_INVALID_MESSAGE,               L"Error: DRT_E_INVALID_MESSAGE"},
      {DRT_E_NO_MORE,                       L"Error: DRT_E_NO_MORE"},
      {DRT_E_INVALID_MAX_ADDRESSES,         L"Error: DRT_E_INVALID_MAX_ADDRESSES"},
      {DRT_E_SEARCH_IN_PROGRESS,            L"Error: DRT_E_SEARCH_IN_PROGRESS"},
      {DRT_E_INVALID_KEY,                   L"Error: DRT_E_INVALID_KEY"},
      {DRT_E_INVALID_PORT,                  L"Error: DRT_E_INVALID_PORT"},
      {DRT_E_STILL_IN_USE,                  L"Error: DRT_E_STILL_IN_USE"},
      {DRT_E_INVALID_ADDRESS,               L"Error: DRT_E_INVALID_ADDRESS"},
      {DRT_E_INVALID_SCOPE,                 L"Error: DRT_E_INVALID_SCOPE"},
      {DRT_E_INVALID_SETTINGS,              L"Error: DRT_E_INVALID_SETTINGS"},
      {DRT_S_RETRY,                         L"Error: DRT_S_RETRY"},
      {DRT_E_INVALID_MAX_ENDPOINTS,	        L"Error: DRT_E_INVALID_MAX_ENDPOINTS"},
      {DRT_E_INVALID_SEARCH_RANGE,	        L"Error: DRT_E_INVALID_SEARCH_RANGE"},
      {DRT_E_INVALID_TRANSPORT_PROVIDER,    L"Error: DRT_E_INVALID_TRANSPORT_PROVIDER"},
      {DRT_E_INVALID_SECURITY_PROVIDER,	    L"Error: DRT_E_INVALID_SECURITY_PROVIDER"},
      {DRT_E_STILL_IN_USE,	                L"Error: DRT_E_STILL_IN_USE"},
      {DRT_E_INVALID_BOOTSTRAP_PROVIDER,    L"Error: DRT_E_INVALID_BOOTSTRAP_PROVIDER"},
      {DRT_E_TRANSPORT_SHUTTING_DOWN,       L"Error: DRT_E_TRANSPORT_SHUTTING_DOWN"},
      {DRT_E_NO_ADDRESSES_AVAILABLE,        L"Error: DRT_E_NO_ADDRESSES_AVAILABLE"},
      {DRT_E_DUPLICATE_KEY,                 L"Error: DRT_E_DUPLICATE_KEY"},
      {DRT_E_TRANSPORTPROVIDER_IN_USE,      L"Error: DRT_E_TRANSPORTPROVIDER_IN_USE"},
      {DRT_E_TRANSPORTPROVIDER_NOT_ATTACHED,L"Error: DRT_E_TRANSPORTPROVIDER_NOT_ATTACHED"},
      {DRT_E_SECURITYPROVIDER_IN_USE,	    L"Error: DRT_E_SECURITYPROVIDER_IN_USE"},
      {DRT_E_SECURITYPROVIDER_NOT_ATTACHED,	L"Error: DRT_E_SECURITYPROVIDER_NOT_ATTACHED"},
      {DRT_E_BOOTSTRAPPROVIDER_IN_USE,	    L"Error: DRT_E_BOOTSTRAPPROVIDER_IN_USE"},
      {DRT_E_BOOTSTRAPPROVIDER_NOT_ATTACHED,L"Error: DRT_E_BOOTSTRAPPROVIDER_NOT_ATTACHED"},
      {DRT_E_TRANSPORT_ALREADY_BOUND,	    L"Error: DRT_E_TRANSPORT_ALREADY_BOUND"},
      {DRT_E_TRANSPORT_NOT_BOUND,	        L"Error: DRT_E_TRANSPORT_NOT_BOUND"},
      {DRT_E_TRANSPORT_UNEXPECTED,	        L"Error: DRT_E_TRANSPORT_UNEXPECTED"},
      {DRT_E_TRANSPORT_INVALID_ARGUMENT,	L"Error: DRT_E_TRANSPORT_INVALID_ARGUMENT"},
      {DRT_E_TRANSPORT_NO_DEST_ADDRESSES,	L"Error: DRT_E_TRANSPORT_NO_DEST_ADDRESSES"},
      {DRT_E_TRANSPORT_EXECUTING_CALLBACK,	L"Error: DRT_E_TRANSPORT_EXECUTING_CALLBACK"},
      {DRT_E_TRANSPORT_ALREADY_EXISTS_FOR_SCOPE,L"Error: DRT_E_TRANSPORT_ALREADY_EXISTS_FOR_SCOPE"},
      {DRT_E_INVALID_SEARCH_INFO,	        L"Error: DRT_E_INVALID_SEARCH_INFO"},
      {DRT_E_FAULTED,	                    L"Error: DRT_E_FAULTED"},
      {DRT_E_TRANSPORT_STILL_BOUND,	        L"Error: DRT_E_TRANSPORT_STILL_BOUND"},
      {DRT_E_INSUFFICIENT_BUFFER,	        L"Error: DRT_E_INSUFFICIENT_BUFFER"},
      {DRT_E_INVALID_INSTANCE_PREFIX,	    L"Error: DRT_E_INVALID_INSTANCE_PREFIX"},
      {DRT_E_INVALID_SECURITY_MODE,	        L"Error: DRT_E_INVALID_SECURITY_MODE"},
      {DRT_E_CAPABILITY_MISMATCH,	        L"Error: DRT_E_CAPABILITY_MISMATCH"},
      {CRYPT_E_FILE_ERROR,                  L"Error: CRYPT_E_FILE_ERROR (Check permissions on the current directory)"},

   };
    if (hr==S_OK)
        return;
   wprintf(L"%s ", fnname); 
   for(int eindex=0;
        eindex < sizeof(valid_errors) / sizeof(struct error_description);
        eindex++ )
    {
        if(hr==valid_errors[eindex].hr)
        {
            wprintf(L"%s\n",valid_errors[eindex].description);
            return;
        }
    }
    wprintf(L"Undocumented Error Code: 0x%x\n",hr);
}


//********************************************************************************************
// Function: DrtEventCallback
//
// Description: Callback to handle general DRT Events. 
//              These include registration state changes, leafset changes, and status changes.
//
//********************************************************************************************
void CALLBACK DrtEventCallback(__in PVOID Param, __in BOOLEAN TimedOut)
{
    HRESULT hr;
    DRT_CONTEXT *Drt = (DRT_CONTEXT*)Param;
    PDRT_EVENT_DATA pEventData = NULL;
    ULONG ulDrtEventDataLen = 0;
    UNREFERENCED_PARAMETER(TimedOut);

    hr = DrtGetEventDataSize (Drt->hDrt, &ulDrtEventDataLen);
    if(FAILED(hr))
    {
       if(hr != DRT_E_NO_MORE)
          wprintf(L"    DrtGetEventDataSize failed: 0x%x\n",hr);
       goto Cleanup;
    }
    pEventData = (PDRT_EVENT_DATA) malloc(ulDrtEventDataLen);
    if (pEventData == NULL)
    {
       printf ("    Out of memory\n");
       goto Cleanup;
    }
  
    hr = DrtGetEventData(Drt->hDrt, ulDrtEventDataLen, pEventData); 
    if(FAILED(hr))
    {
       if(hr != DRT_E_NO_MORE)
           wprintf(L"    DrtGetEventData failed: 0x%x\n",hr);
       goto Cleanup;
    }
  
    if(pEventData->type == DRT_EVENT_STATUS_CHANGED)
    {
       if(pEventData->statusChange.status == DRT_ACTIVE)
       {
           SetConsoleTitle(L"DrtSdkSample Current Drt Status: Active");
           if(g_DisplayEvents)
               wprintf(L"    DRT Status Changed to Active\n");
       }
       else if(pEventData->statusChange.status == DRT_ALONE)
       {
           SetConsoleTitle(L"DrtSdkSample Current Drt Status: Alone");
           if(g_DisplayEvents)
               wprintf(L"    DRT Status Changed to Alone\n");
       }
       else if(pEventData->statusChange.status == DRT_NO_NETWORK)
       {
           SetConsoleTitle(L"DrtSdkSample Current Drt Status: No Network");
           if(g_DisplayEvents)
               wprintf(L"    DRT Status Changed to No Network\n");
       }
       else if(pEventData->statusChange.status == DRT_FAULTED)
       {
           SetConsoleTitle(L"DrtSdkSample Current Drt Status: Faulted");
           if(g_DisplayEvents)
               wprintf(L"    DRT Status Changed to Faulted\n");
       }
    }
    else if(pEventData->type == DRT_EVENT_LEAFSET_KEY_CHANGED)
    {
       if(g_DisplayEvents)
       {
           if(pEventData->leafsetKeyChange.change == DRT_LEAFSET_KEY_ADDED)
               wprintf(L"    Leafset Key Added Event: [hr: 0x%x]\n", pEventData->hr);
           else if(pEventData->leafsetKeyChange.change == DRT_LEAFSET_KEY_DELETED)
               wprintf(L"    Leafset Key Deleted Event: [hr: 0x%x]\n", pEventData->hr);
       }
    }
    else if(pEventData->type == DRT_EVENT_REGISTRATION_STATE_CHANGED)
    {            
       if(g_DisplayEvents)
           wprintf(L"    Registration State Changed Event: [hr: 0x%x, registration state: %i]\n", pEventData->hr, pEventData->registrationStateChange.state);
    }
    free(pEventData);
    pEventData = NULL;

Cleanup:
   if(pEventData)
      free(pEventData);
   return;
}


//********************************************************************************************
// Function: InitializeDrt
//
// Description: Initializes and brings a DRT instance online
//   1) Brings up an ipv6 transport layer
//   2) Attaches a security provider (according to user's choice)
//   3) Attaches a bootstrap provider (according to user's choice)
//   4) Calls DrtOpen to bring the DRT instance online
//
//********************************************************************************************
bool InitializeDrt(DRT_CONTEXT *Drt)
{
    HRESULT         hr = S_OK;
    DWORD           dwSize = 0;
    PWSTR           pwszCompName = NULL;
    WCHAR           pwszBootstrapHostname[1024] = {0};
    USHORT          usBootstrapPort = 0;

    //
    // Initialize DrtSettings
    //
    Drt->port = 0;
    Drt->settings.pwzDrtInstancePrefix = L"Local_DRT";
    Drt->settings.dwSize = sizeof(DRT_SETTINGS);
    Drt->settings.cbKey = KEYSIZE;
    Drt->settings.ulMaxRoutingAddresses = 4;
    Drt->settings.bProtocolMajorVersion = 0x6;
    Drt->settings.bProtocolMinorVersion = 0x65;
    Drt->settings.eSecurityMode = DRT_SECURE_CONFIDENTIALPAYLOAD;
    Drt->settings.hTransport = NULL;
    Drt->settings.pSecurityProvider = NULL;
    Drt->settings.pBootstrapProvider = NULL;
    Drt->hDrt = NULL;

    //
    // *Transport*
    //

    hr = DrtCreateIpv6UdpTransport(
        DRT_GLOBAL_SCOPE, 
        0,
        300, 
        &Drt->port, 
        &Drt->settings.hTransport
        );
    
    VERIFY_OR_ABORT("DrtCreateTransport",hr);
    
    //
    // *Security Provider*
    //

    if(Drt->SecurityProviderType == 0) //Null Security Provider
    {
        hr = DrtCreateNullSecurityProvider(&Drt->settings.pSecurityProvider);
    }
    else if(Drt->SecurityProviderType == 1) //Derived Key Security Provider
    {
        hr = ReadCertFromFile(L"RootCertificate.cer", &Drt->pRoot, NULL);
        if (FAILED(hr))
        {
            wprintf(L"No RootCertificate.cer file found in the current directory, Creating a new root certificate.\n");
            hr = MakeCert(L"RootCertificate.cer", L"RootCert", NULL, NULL);
            VERIFY_OR_ABORT(L"MakeCert",hr);
            hr = ReadCertFromFile(L"RootCertificate.cer", &Drt->pRoot, NULL);
            VERIFY_OR_ABORT(L"ReadCertFromFile", hr);
        }

        // We now have a root cert, read an existing local cert or create one based on root cert
        hr = ReadCertFromFile(L"LocalCertificate.cer", &Drt->pLocal, NULL);
        if (FAILED(hr))
        {
            wprintf(L"No LocalCertificate.cer file found in the current directory, Creating a new local certificate.\n");
            hr = MakeCert(L"LocalCertificate.cer", L"LocalCert", L"RootCertificate.cer", L"RootCert");
            VERIFY_OR_ABORT(L"MakeCert",hr);
            hr = ReadCertFromFile(L"LocalCertificate.cer", &Drt->pLocal, NULL);
            VERIFY_OR_ABORT(L"ReadCertFromFile", hr);
        }
        hr = DrtCreateDerivedKeySecurityProvider(
            Drt->pRoot,
            Drt->pLocal,
            &Drt->settings.pSecurityProvider
            );
    }
    else if(Drt->SecurityProviderType == 2) //Custom Security Provider
    {
        hr = DrtCreateCustomSecurityProvider(&Drt->settings.pSecurityProvider);
    }
    else
    {
        wprintf(L"Invalid Security Provider passed to InitializeDrt");
        hr = E_FAIL;
    }
    VERIFY_OR_ABORT("DrtCreateSecurityProvider",hr);

    //
    // *Bootstrap Provider*
    //

    if(Drt->BootstrapProviderType == 0) //DNS Bootstrap Provider
    {
        GetComputerNameEx(ComputerNameDnsFullyQualified, NULL, &dwSize );
        pwszCompName = new WCHAR[dwSize+1];
        if(pwszCompName == NULL)
        {
            hr = E_OUTOFMEMORY;
            wprintf(L"Out of memory");
            goto Cleanup;
        }
        ::GetComputerNameEx(ComputerNameDnsFullyQualified, pwszCompName, &dwSize);

        wprintf(L"Enter 'hostname port' for DNS Bootstrap Provider (currently %s %i):\n", pwszCompName, Drt->port);
        if(wscanf_s(L"%s %hu",pwszBootstrapHostname, 1024, &usBootstrapPort) < 2)
        {
            FlushCurrentLine();
            hr = E_INVALIDARG;
            wprintf(L"Invalid hostname:port\n");
            goto Cleanup;
        }
        wprintf(L"DNS Bootstrapping from: %s:%i\n",pwszBootstrapHostname, usBootstrapPort);
        FlushCurrentLine();
     
        hr = DrtCreateDnsBootstrapResolver(
            usBootstrapPort,
            pwszBootstrapHostname,
            &Drt->settings.pBootstrapProvider
            );
    }
    else if (Drt->BootstrapProviderType == 1) //PNRP Bootstrap Provider
    {
        wprintf(L"Enter a PNRP name for PNRP Bootstrap Provider (IE 0.DemoName)\n");
        if(wscanf_s(L"%s",pwszBootstrapHostname, 1024) < 1)
        {
            FlushCurrentLine();
            wprintf(L"Invalid PNRP name\n");
            goto Cleanup;
        }
        wprintf(L"PNRP Bootstrapping from: %s\n",pwszBootstrapHostname);
        FlushCurrentLine();
        hr = DrtCreatePnrpBootstrapResolver(
            TRUE,
            pwszBootstrapHostname,
            L"Global_",
            NULL,
            &Drt->settings.pBootstrapProvider
            );
    }
    else if (Drt->BootstrapProviderType == 2) //Custom Bootstrap Provider
    {
        GetComputerNameEx(ComputerNameDnsFullyQualified, NULL, &dwSize );
        pwszCompName = new WCHAR[dwSize+1];
        ::GetComputerNameEx(ComputerNameDnsFullyQualified, pwszCompName, &dwSize);

        if(pwszCompName == NULL)
        {
            hr = E_OUTOFMEMORY;
            wprintf(L"Out of memory");
            goto Cleanup;
        }

        wprintf(L"Enter 'hostname port' for Custom Bootstrap Provider (currently %s %i\n", pwszCompName, Drt->port);
        if(wscanf_s(L"%s %hu",pwszBootstrapHostname, 1024, &usBootstrapPort) < 2)
        {
            FlushCurrentLine();
            hr = E_INVALIDARG;
            wprintf(L"Invalid hostname:port\n");
            goto Cleanup;
        }
        wprintf(L"Custom Bootstrapping from: %s:%i\n",pwszBootstrapHostname, usBootstrapPort);
        FlushCurrentLine();
     
        hr = DrtCreateCustomBootstrapResolver(
            usBootstrapPort,
            pwszBootstrapHostname,
            &Drt->settings.pBootstrapProvider
            );
    }
    else
    {
        wprintf(L"Invalid Bootstrap Provider passed to InitializeDrt");
        hr = E_FAIL;
    }
    VERIFY_OR_ABORT("DrtCreateBootstrapResolver",hr);

    //
    // *Make sure the Windows Firewall is open*
    //  Also open port 3540 (used by PNRP, if the PNRP bootstrap provider is chosen)
    //

    if(Drt->BootstrapProviderType==1)
        hr = OpenFirewallForDrtSdkSample(TRUE);
    else
        hr = OpenFirewallForDrtSdkSample(FALSE);

    VERIFY_OR_ABORT("OpenFirewallForDrtSdkSample",hr);
    
    //
    // Open the DRT
    //

    Drt->eventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(NULL == Drt->eventHandle)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    hr = DrtOpen(&Drt->settings, Drt->eventHandle, NULL, &Drt->hDrt);

    VERIFY_OR_ABORT("DrtOpen", hr);

    //
    // Register a callback to handle DRT Events
    //
    RegisterWaitForSingleObject(&Drt->DrtWaitEvent, Drt->eventHandle, (WAITORTIMERCALLBACK)DrtEventCallback, Drt, INFINITE, WT_EXECUTEDEFAULT);
    
Cleanup:
    if(pwszCompName)
        delete pwszCompName;
    return SUCCEEDED(hr);
}


//********************************************************************************************
// Function: GetKeyFromUser
//
// Description: Gets a registration key from the user (used for registration and search)
//
//********************************************************************************************
bool GetKeyFromUser(PCWSTR pcwszKeyName, BYTE* KeyData)
{
    int i;
    int hexdigit=0;
    wprintf(L"Enter %s as a string of hex digits, Example: 01 ff 0a b8 80 z\n", pcwszKeyName);
    wprintf(L"The current keysize is %i bytes.  Enter z as the last digit and the remainder of the key will be zero-filled (Most significant byte is first)\n",KEYSIZE);
    for(i=KEYSIZE-1;i>=0;i--)
    {
        if(wscanf_s(L"%2X",&hexdigit) < 1)
            break;
        KeyData[i] = (BYTE)hexdigit;
    }
    for(;i>=0;i--)
        KeyData[i] = 0;
    wprintf(L"Resulting %s:\n", pcwszKeyName);
    for(i=KEYSIZE-1;i>=0;i--)
        wprintf(L"%02x ",KeyData[i]);
    wprintf(L"\n");
    FlushCurrentLine();

    return true;
}

//********************************************************************************************
// Function: PerformDrtSearch
//
// Description: Initializes and performs a search through the DRT
//
//********************************************************************************************

bool PerformDrtSearch(DRT_CONTEXT* Drt, INT SearchType)
{
    HRESULT hr = S_OK;
    DWORD dwSize = 1024;
    bool fKeyFound = FALSE;
    DRT_SEARCH_INFO SearchInfo = {0};
    DRT_SEARCH_INFO* pSearchInfo = NULL;
    HDRT_SEARCH_CONTEXT SearchContext = {0};
    DRT_SEARCH_RESULT *pSearchResult = NULL;
    BYTE searchKeyData[KEYSIZE] = {0};
    BYTE minKeyData[KEYSIZE] = {0};
    BYTE maxKeyData[KEYSIZE] = {0};
    DRT_DATA searchKey = {0};
    DRT_DATA minKey = {0};
    DRT_DATA maxKey = {0};

    //Create a manual reset event 
    //The DRT will reset the event when the search result buffer has been consumed
    HANDLE hDrtSearchEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if(NULL == hDrtSearchEvent)
    {
        wprintf(L"Out of memory\n");
        goto Cleanup;
    }
        

    //Set Some Defaults for SearchInfo
    SearchInfo.dwSize = sizeof(DRT_SEARCH_INFO);
    SearchInfo.fIterative = FALSE;
    SearchInfo.fAllowCurrentInstanceMatch = TRUE;
    SearchInfo.fAnyMatchInRange = FALSE;
    SearchInfo.cMaxEndpoints = 1;
    SearchInfo.pMinimumKey = &minKey;
    SearchInfo.pMaximumKey = &maxKey;

    searchKey.cb = KEYSIZE;
    searchKey.pb = searchKeyData;
    minKey.cb = KEYSIZE;
    minKey.pb = minKeyData;
    maxKey.cb = KEYSIZE;
    maxKey.pb = maxKeyData;

    if(!GetKeyFromUser(L"Search Key",searchKeyData))
        goto Cleanup;

    if(SearchType==2) //Simple DRT Search
    {
        pSearchInfo = NULL;
    }
    else if(SearchType==3) //Nearest Match Search
    {
        SearchInfo.fAnyMatchInRange = FALSE;
        pSearchInfo = &SearchInfo;
        memset(minKeyData,0,KEYSIZE);
        memset(maxKeyData,0xFF,KEYSIZE);
    }
    else if(SearchType==4) //Iterative Search
    {
        SearchInfo.fIterative = TRUE;
        pSearchInfo = &SearchInfo;
        minKey.pb = searchKey.pb;
        maxKey.pb = searchKey.pb;
    }
    else if(SearchType==5) //Range Search
    {
        SearchInfo.fAnyMatchInRange = TRUE;
        if(!GetKeyFromUser(L"Min Search Key (01 z)",minKeyData))
            goto Cleanup;
        if(!GetKeyFromUser(L"Max Search Key (ff z)",maxKeyData))
            goto Cleanup;
        pSearchInfo = &SearchInfo;
    }
    else
    {
        wprintf(L"Invalid Search Type passed to DrtPerformSearch");
        goto Cleanup;
    }

    hr = DrtStartSearch(
        Drt->hDrt, 
        &searchKey, 
        pSearchInfo, 
        5000, 
        hDrtSearchEvent, 
        NULL, 
        &SearchContext);

    VERIFY_OR_ABORT("DrtStartSearch",hr);

    do
    {
        DWORD dwRes = WaitForSingleObject(hDrtSearchEvent, 30*1000);

        if(dwRes == WAIT_OBJECT_0)
        {
            hr = DrtGetSearchResultSize(SearchContext,&dwSize);
            if(hr != S_OK)
            {
                continue;
            }
            pSearchResult = (DRT_SEARCH_RESULT*)malloc(dwSize);
            if(pSearchResult == NULL)
            {
                wprintf(L"Error: Out of memory\n");
                break;
            }
            hr = DrtGetSearchResult(SearchContext, dwSize, pSearchResult);
            if(hr != S_OK)
            {
                continue;
            }
            if(pSearchResult->type == DRT_MATCH_EXACT)
            {
                fKeyFound = TRUE;
                wprintf(L"*Found Key*: ");
                for(int i=pSearchResult->registration.key.cb-1;i>=0;i--)
                    wprintf(L"%02x ",pSearchResult->registration.key.pb[i]);
                wprintf(L"\n");
                PrintSearchPath(SearchContext);
            }
            else if(pSearchResult->type == DRT_MATCH_NEAR)
            {
                wprintf(L"*Found Near Match*: ");
                for(int i=pSearchResult->registration.key.cb-1;i>=0;i--)
                    wprintf(L"%02x ",pSearchResult->registration.key.pb[i]);
                wprintf(L"\n");
                if(SearchType==3)
                    fKeyFound = TRUE;
                PrintSearchPath(SearchContext);
            }
            else if(pSearchResult->type == DRT_MATCH_INTERMEDIATE)
            {
                wprintf(L"Intermediate Match: ");
                for(int i=pSearchResult->registration.key.cb-1;i>=0;i--)
                    wprintf(L"%02x ",pSearchResult->registration.key.pb[i]);
                wprintf(L"\n");
                DrtContinueSearch(SearchContext);
            }
        }
        else
        {
            wprintf(L"Drt Search Timed out\n");
            break;
        }
        if(pSearchResult)
        {
            free(pSearchResult);
            pSearchResult = NULL;
        }
    } while( (hr == DRT_E_SEARCH_IN_PROGRESS) || (hr == S_OK) );
    DrtEndSearch(SearchContext);

    //
    // When the search is finished, the HRESULT should be DRT_E_NO_MORE
    //
    if(hr != DRT_E_NO_MORE)
    {
        wprintf(L"Unexpected HRESULT from DrtGetSearchResult: 0x%x\n",hr);
    }

    if(!fKeyFound)
        wprintf(L"Could not find key\n");
    
Cleanup:
    if(pSearchResult)
        free(pSearchResult);
    if (hDrtSearchEvent)
        CloseHandle(hDrtSearchEvent);
        
    return true;
}

void PrintSearchPath(HDRT_SEARCH_CONTEXT SearchContext)
{
    HRESULT hr = S_OK;
    ULONG ulSearchPathLen = 0;
    DRT_ADDRESS_LIST* pSearchPath = NULL;
    
    hr = DrtGetSearchPathSize(SearchContext, &ulSearchPathLen);

    if(FAILED(hr))
    {
        goto Cleanup;
    }

    pSearchPath = (PDRT_ADDRESS_LIST) malloc(ulSearchPathLen);
    if(pSearchPath == NULL)
    {
        printf ("Out of memory\n");
        goto Cleanup;
    }

    hr = DrtGetSearchPath(SearchContext, ulSearchPathLen, pSearchPath);
    
    if(FAILED(hr))
    {
        goto Cleanup;
    }

    wprintf(L"Search Path:\n");
    for(UINT i=0;i<pSearchPath->AddressCount;i++)
    {
        SOCKADDR_IN6 addr = *(SOCKADDR_IN6*)(&pSearchPath->AddressList[i].socketAddress);
        wprintf(L"Port: %i Flags: 0x%x, Nearness: %i Latency: %i\n",
            addr.sin6_port,
            pSearchPath->AddressList[i].flags,
            pSearchPath->AddressList[i].nearness,
            pSearchPath->AddressList[i].latency);
    }

Cleanup:
    if(pSearchPath)
    {
        free(pSearchPath);
    }
        
}


//********************************************************************************************
// Function: RegisterKey
//
// Description: Registers a key in the current DRT Instance
//
//********************************************************************************************
bool RegisterKey(DRT_CONTEXT* Drt)
{
    HRESULT hr = S_OK;
    BYTE* newKeyData = NULL;
    BYTE* newPayloadData = NULL;
    REG_CONTEXT reg = {0};
    CERT_CONTEXT *pRegCertContext = NULL;

    newKeyData = (BYTE*)malloc(KEYSIZE);
    newPayloadData = (BYTE*)malloc(KEYSIZE);
    reg.regInfo.key.cb = KEYSIZE;
    reg.regInfo.key.pb = newKeyData;
    reg.regInfo.appData.cb = KEYSIZE;
    reg.regInfo.appData.pb = newPayloadData;
    reg.hDrtReg = NULL;

    if(!newKeyData || !newPayloadData)
    {
        wprintf(L"Not enough memory");
        goto Cleanup;
    }

    if(Drt->SecurityProviderType == 1) // Derived Key Security Provider
    {
        wprintf(L"Generating a new certificate for the new registration...\n");
        hr = MakeCert(L"LastRegisteredCert.cer", L"LocalCert", L"RootCertificate.cer", L"RootCert");
        VERIFY_OR_ABORT(L"MakeCert",hr);
        wprintf(L"Creating a new key based on the generated certificate...\n");
        hr = ReadCertFromFile(L"LastRegisteredCert.cer", &pRegCertContext, NULL);
        VERIFY_OR_ABORT("ReadCertFromFile",hr);
        hr = DrtCreateDerivedKey(pRegCertContext,&reg.regInfo.key);
        VERIFY_OR_ABORT("DrtCreateDerivedKey",hr);
    }
    else
    {
        if(!GetKeyFromUser(L"Registration Key",newKeyData))
            goto Cleanup;
    }

    hr = DrtRegisterKey(Drt->hDrt, &reg.regInfo, pRegCertContext, &reg.hDrtReg);
    VERIFY_OR_ABORT("DrtRegisterKey",hr);

    if(SUCCEEDED(hr))
    {
        Drt->registrations.push_back(reg);
        
        // newKeyData and newPayloadData will be freed on unregister
        newKeyData = NULL;
        newPayloadData = NULL;
        
        wprintf(L"Successfully Registered: ");
        for(int i=reg.regInfo.key.cb-1;i>=0;i--)
            wprintf(L"%02x ",reg.regInfo.key.pb[i]);
        wprintf(L"\n");
    }

Cleanup:

    if(newKeyData)
        free(newKeyData);
    if(newPayloadData)
        free(newPayloadData);
    if(pRegCertContext)
        CertFreeCertificateContext(pRegCertContext);
    return true;
}


//********************************************************************************************
// Function: UnRegisterKey
//
// Description: Unregisters a previously registered key
//
//********************************************************************************************
bool UnRegisterKey(DRT_CONTEXT* Drt)
{
    int choice = 0;
    wprintf(L"Current Registrations:\n");
    for(UINT i=0; i<Drt->registrations.size(); i++)
    {
        wprintf(L"%i: ",i);
        for(int k=Drt->registrations[i].regInfo.key.cb-1; k>=0; k--)
            wprintf(L" %02x",Drt->registrations[i].regInfo.key.pb[k]);
        wprintf(L"\n");
    }
    wprintf(L"Enter a registration to unregister (or c to cancel):");
    if( (wscanf_s(L"%i",&choice) < 1) || 
        (choice >= (int)Drt->registrations.size()) || 
        (choice < 0) )
    {
        FlushCurrentLine();
        goto Cleanup;
    }
    FlushCurrentLine();

    wprintf(L"Unregistering key: ");
    for(int k=Drt->registrations[choice].regInfo.key.cb; k<=0; k++)
        wprintf(L" %02x",Drt->registrations[choice].regInfo.key.pb[k]);
    wprintf(L"\n");

    DrtUnregisterKey(Drt->registrations[choice].hDrtReg);

    if(Drt->registrations[choice].regInfo.key.pb)
        free(Drt->registrations[choice].regInfo.key.pb);
    if(Drt->registrations[choice].regInfo.appData.pb)
        free(Drt->registrations[choice].regInfo.appData.pb);
    Drt->registrations.erase(Drt->registrations.begin()+choice);

Cleanup:
    return true;
}


//********************************************************************************************
// Function: CleanupDrt
//
// Description: Deletes and Frees the various objects and providers used by the DRT
//
//********************************************************************************************
void CleanupDrt(DRT_CONTEXT *Drt)
{
    for(UINT i=0; i<Drt->registrations.size(); i++)
    {
        DrtUnregisterKey(Drt->registrations[i].hDrtReg);

        if(Drt->registrations[i].regInfo.key.pb)
            free(Drt->registrations[i].regInfo.key.pb);
        if(Drt->registrations[i].regInfo.appData.pb)
            free(Drt->registrations[i].regInfo.appData.pb);
    }
    Drt->registrations.clear();
    
    if (Drt->DrtWaitEvent != NULL)
    {
        UnregisterWait(Drt->DrtWaitEvent);
    }
    
    if (Drt->hDrt != NULL)
    {
        DrtClose(Drt->hDrt);
        Drt->hDrt = NULL;
    }
    
    if (Drt->eventHandle!= NULL)
    {
        CloseHandle(Drt->eventHandle);
        Drt->eventHandle = NULL;
    }

    if (Drt->settings.pBootstrapProvider != NULL)
    {
        if(Drt->BootstrapProviderType == 0)
            DrtDeleteDnsBootstrapResolver(Drt->settings.pBootstrapProvider);
        else if(Drt->BootstrapProviderType == 1)
            DrtDeletePnrpBootstrapResolver(Drt->settings.pBootstrapProvider);
        else if(Drt->BootstrapProviderType == 2)
            DrtDeleteCustomBootstrapResolver(Drt->settings.pBootstrapProvider);
    }

    if (Drt->settings.pSecurityProvider != NULL)
    {
        if(Drt->SecurityProviderType == 0)
            DrtDeleteNullSecurityProvider(Drt->settings.pSecurityProvider);
        else if(Drt->SecurityProviderType == 1)
            DrtDeleteDerivedKeySecurityProvider(Drt->settings.pSecurityProvider);
        else if(Drt->SecurityProviderType == 2)
            DrtDeleteCustomSecurityProvider(Drt->settings.pSecurityProvider);
    }

    if (Drt->pRoot != NULL)
    {
        CertFreeCertificateContext(Drt->pRoot);
    }

    if (Drt->pLocal != NULL)
    {
        CertFreeCertificateContext(Drt->pLocal);
    }
}


//********************************************************************************************
// Function: Main
//
// Description: The main function, initializes a DRT according to user specifications and
//                loops allowing the user to interact with the DRT.
//
//********************************************************************************************
int __cdecl main()
{
   int nSearchType;

   DRT_CONTEXT LocalDrt = { 0 };

   PCWSTR ppcwzSecurityProviderChoices[] = {
      L"Initialize DRT with Null Security Provider",
      L"Initialize DRT with Derived Key Security Provider",
      L"Initialize DRT with Custom Security Provider",
      L"Exit" };

   PCWSTR ppcwzBootStrapProviderChoices[] = {
      L"Bootstrap with Builtin DNS Bootstrap Provider",
      L"Bootstrap with Builtin PNRP Bootstrap Provider",
      L"Bootstrap with Custom Bootstrap Provider",
      L"Exit" };

   PCWSTR ppcwzSearchChoices[] = {
      L"Register a new key",
      L"Unregister a key",
      L"Simple DRT Search",
      L"Nearest Match Search",
      L"Iterative Search",
      L"Range Search",
      L"Hide DRT Events",
      L"Display DRT Events",
      L"Exit" };

    SetConsoleTitle(L"DrtSdkSample Current Drt Status: Initializing");

    LocalDrt.SecurityProviderType = GetUserChoice(ppcwzSecurityProviderChoices, 4);
    if(LocalDrt.SecurityProviderType==-1 || LocalDrt.SecurityProviderType==3)
        goto Cleanup;

    LocalDrt.BootstrapProviderType = GetUserChoice(ppcwzBootStrapProviderChoices, 4);
    if(LocalDrt.BootstrapProviderType==-1 || LocalDrt.BootstrapProviderType==3)
        goto Cleanup;
   
    if(!InitializeDrt(&LocalDrt))
        goto Cleanup;

    wprintf(L"DRT Initialization Complete\n");

    SetConsoleTitle(L"DrtSdkSample Current Drt Status: Bootstrapping");

    for(;;)
    {
        nSearchType = GetUserChoice(ppcwzSearchChoices, 9);
        if(nSearchType==0)
        {
            if(!RegisterKey(&LocalDrt))
                goto Cleanup;
        }
        else if(nSearchType==1)
        {
            if(!UnRegisterKey(&LocalDrt))
                goto Cleanup;
        }
        else if( 1 < nSearchType && nSearchType < 6 )
        {
            if(!PerformDrtSearch(&LocalDrt, nSearchType))
                goto Cleanup;
        }
        else if(nSearchType==6)
        {
            wprintf(L"DRT Events will not be displayed\n");
            g_DisplayEvents = 0;
        }
        else if(nSearchType==7)
        {
            wprintf(L"DRT Events will be displayed asynchronously\n");
            g_DisplayEvents = 1;
        }
        else if(nSearchType==8)
        {
            goto Cleanup;
        }
    }

Cleanup:
    CleanupDrt(&LocalDrt);
    return 0;
}
