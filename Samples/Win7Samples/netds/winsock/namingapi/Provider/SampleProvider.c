/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All Rights Reserved.

Module Name:

    SampleProvider.c

Abstract:

    This C file includes sample code for a simple nspv2 naming 
    provider.

Feedback:
    If you have any questions or feedback, please contact us using
    any of the mechanisms below:

    Email: peerfb@microsoft.com
    Newsgroup: Microsoft.public.win32.programmer.networks
    Website: http://www.microsoft.com/p2p

--********************************************************************/

#include "SampleProvider.h"

// NOTE:  *** Every provider should generate their own unique id ***
//
// if you use this sample as the basis for your own provider, you must change the next line.
GUID g_SampleProviderId = 
    { 0xe6401bf9, 0x5579, 0x472a, { 0x83, 0x82, 0xca, 0x07, 0x07, 0x66, 0x11, 0xbd }};


typedef struct _LookupRequest
{
    PWSTR pwzEmailAddress;
    PWSTR pwzServiceName;
    BOOL  fPreferredName;
    BOOL  fProcessed;
} LookupRequest;


// Utility routine that calculates the size of a WSAQUERYSET2 based on a specific
// email address and array of CSADDR_INFO.
//
size_t CalculateSizeHelper(PCWSTR pcwzEmail, 
                          ULONG cAddresses, 
                          __in_ecount(cAddresses) CSADDR_INFO *prgAddresses)
{
    WSAQUERYSET2 qs = {0};

    // Fill out our temp queryset w/ the required fields.
    //
    qs.lpszServiceInstanceName = (PWSTR) pcwzEmail;
    qs.dwNumberOfCsAddrs = cAddresses; 
    qs.lpcsaBuffer = prgAddresses;
    qs.lpNSProviderId = &g_SampleProviderId;

    // Call the generic utility from utils.c
    //
    return GetWSAQuerySet2Size(&qs);
}

//
// NSP Function Entry Points
//

// Called each time a new client process begins using the provider  Providers may use the 
// ppvClientSessionArg field to store information about this session -- the value returned 
// from this function will be passed to subsequent calls in the same session.
//
INT WSAAPI NSPv2Startup(__in  LPGUID lpguidProvider, 
                        __out LPVOID* ppvClientSessionArg)
{
    lpguidProvider; //unreferenced param
    ppvClientSessionArg; //unreferenced param

    *ppvClientSessionArg = NULL;
    return NO_ERROR;
}

// Called when a client is shutting down.  The ppvClientSessionArg is the same one returned 
// from this provider from the call to NSPv2Startup.
//
INT WSAAPI NSPv2Cleanup(__in  LPGUID lpguidProvider, 
                        __in  LPVOID* ppvClientSessionArg)
{
    lpguidProvider; //unreferenced param
    ppvClientSessionArg; //unreferenced param
    return NO_ERROR;
}


// Called when a new lookup is beginning.  The provider should store any required information 
// in order to  perform the actual lookup, which will happen in NSPv2LookupServiceNextEx.   
// The provider returns a "handle" in phLookup, which will be passed back to the provider 
// in NSPv2LookupServiceNextEx and NSPv2LookupServiceEnd to keep track of which lookup session
// is being used.
//
INT WSAAPI NSPv2LookupServiceBegin(__in  LPGUID lpguidProvider,
                                   __in  LPWSAQUERYSET2 pqsRestrictions, 
                                         DWORD dwControlFlags, 
                                   __in  LPVOID pvClientSessionArg,
                                   __out LPHANDLE phLookup)
{
    INT err = NO_ERROR;
    LookupRequest *pRequest = malloc(sizeof(LookupRequest));

    lpguidProvider; //unreferenced param
    pvClientSessionArg; //unreferenced param

    if (pRequest == NULL)
    {
        err = WSA_NOT_ENOUGH_MEMORY; 
        goto cleanup;
    }

    ZeroMemory(pRequest, sizeof(*pRequest));
    pRequest->fProcessed = FALSE;

    if (dwControlFlags & LUP_RETURN_PREFERRED_NAMES)
    {
        pRequest->fPreferredName = TRUE;
    }
    else
    {
        pRequest->fPreferredName = FALSE;
        err = CopyString(pqsRestrictions->lpszServiceInstanceName, &pRequest->pwzEmailAddress);
        if (err != NO_ERROR)
        {
            goto cleanup;
        }
        err = CopyString(pqsRestrictions->lpszContext, &pRequest->pwzServiceName);
        if (err != NO_ERROR)
        {
            goto cleanup;
        }
    }

cleanup:

    if (err != NO_ERROR)
    {
        SetLastError(err);
        return SOCKET_ERROR;
    }
    else
    {
       *phLookup = (HANDLE) pRequest;
        return NO_ERROR;
    }
}


VOID WSAAPI NSPv2LookupServiceNextEx(        HANDLE hAsyncCall,
                                             HANDLE hLookup,
                                             DWORD dwControlFlags,
                                     __inout LPDWORD pdwBufLen, 
                                     __out   LPWSAQUERYSET2 pqsResults)
{
    INT err = NO_ERROR;
    ULONG cAddresses = 0;
    CSADDR_INFO *pAddresses = NULL;
    LookupRequest *pRequest = (LookupRequest*) hLookup;
    PWSTR pwzEmailAddress = NULL;

    dwControlFlags; //unreferenced param

    if (pdwBufLen == NULL)
    {
        SetLastError(WSA_INVALID_PARAMETER);
        WSAProviderCompleteAsyncCall(hAsyncCall, WSA_INVALID_PARAMETER);
        return;
    }

    ZeroMemory(pqsResults, *pdwBufLen);

    if (pRequest->fProcessed == TRUE)
    {
        // we have already returned a result for this request, and we only support one.
        err = WSA_E_NO_MORE;
    }
    else 
    {
        if (pRequest->fPreferredName)
        {
            PWSTR wzPreferred = L"You@sampleprovider.net";

            pAddresses = NULL;
            cAddresses = 0;
            pwzEmailAddress = wzPreferred;
        }
        else
        {
            RegInfo *pRegInfo = NULL;

            // The pRegInfo returned from FindRegistration is just a pointer to an item in an internal 
            // array that could go away, based on another call to SetService.  Thus, we enter a 
            // critical section to protect that.
            EnterCriticalSection(&g_cs); 

            // Lookup the email address.  In our implementation this is just a lookup in a local cache.
            //
            if (FindRegistration(pRequest->pwzEmailAddress, pRequest->pwzServiceName, &pRegInfo))
            {
                pAddresses = pRegInfo->prgAddresses;
                cAddresses = pRegInfo->cAddresses;
                pwzEmailAddress = pRegInfo->pwzEmailAddress;
            }
            else
            {
                // Failed to find an entry for this email address in our cache.
                //
                err = WSANO_DATA;
            }

            LeaveCriticalSection(&g_cs); 
        }
    }

    if (err == NO_ERROR)
    {
        // Found a registration!   Figure out how much space is required to return the data 
        // to the caller.
        // 
        size_t cbRequired = CalculateSizeHelper(pwzEmailAddress, 
                                                cAddresses, 
                                                pAddresses);

        if (*pdwBufLen < cbRequired)
        {
            // They didn't pass us a large enough buffer.  Tell them how much we need, and 
            // return WSAEFAULT.
            //
            *pdwBufLen = (DWORD) cbRequired;
            err = WSAEFAULT;
        }
        else
        {
            // They passed us a large enough buffer, let's fill it out.
            //
            WSAQUERYSET2 qs = {0};

            qs.dwSize = sizeof(WSAQUERYSET2);
            qs.dwNameSpace = NS_EMAIL;
            qs.lpszServiceInstanceName = pwzEmailAddress;
            qs.dwNumberOfCsAddrs  = cAddresses;
            qs.lpcsaBuffer = pAddresses;
            qs.lpNSProviderId = &g_SampleProviderId;
            
            if (pRequest->fPreferredName)
                qs.dwOutputFlags = LUP_RETURN_PREFERRED_NAMES;

            // We need to hand back our memory in buffer from a single allocation (pointers in 
            // the WSAQUERYSET2 structure need to simply point to memory later in the same 
            // allocation).   We call our utility routine to perform this serialization.
            err = BuildSerializedQuerySet2(&qs, *pdwBufLen, (BYTE*) pqsResults);

            if (err == NO_ERROR)
            {
                // Update the request handle, to remember we have already processed this one.
                pRequest->fProcessed = TRUE;
            }
        }
    }


    // It is safe to complete the call on the same thread.  In fact, even if this call had
    // take a "long time" -- the calling threads would not have been blocked.  The design is
    // such that naming providers need not worry about this.
    WSAProviderCompleteAsyncCall(hAsyncCall, err);
}

// Called when a particular lookup is complete.  In our case, we need to free the memory 
// associated w/ our "handle".
//
INT WSAAPI NSPv2LookupServiceEnd(HANDLE hLookup)
{
    LookupRequest *pRequest = (LookupRequest *) hLookup;

    if (pRequest != NULL)
    {
        free(pRequest->pwzEmailAddress);
        free(pRequest->pwzServiceName);
        free(pRequest);
        return NO_ERROR;
    }
    else
    {
        SetLastError(WSA_INVALID_PARAMETER);
        return SOCKET_ERROR;
    }
}

// Called when the client wants to add or remove a new registration for the specified email address.
//
void WSAAPI NSPv2SetServiceEx(     HANDLE hAsyncCall, 
                             __in  LPGUID pProviderGuid,
                             __in  LPWSAQUERYSET2 pQuerySet, 
                                   WSAESETSERVICEOP eSetServiceOp,
                                   DWORD dwControlFlags,
                             __in  LPVOID pvClientSessionArg)
{
    INT retval = NO_ERROR;

    pProviderGuid; //unreferenced param
    dwControlFlags; //unreferenced param
    pvClientSessionArg; //unreferenced param
    
    if (eSetServiceOp == RNRSERVICE_REGISTER)
    {
        retval = AddRegistration(pQuerySet);
    }
    else
    {
        retval = RemoveRegistration(pQuerySet);
    }

    WSAProviderCompleteAsyncCall(hAsyncCall, retval);
}

void __cdecl main(int argc, __in_ecount(argc) char *argv[])
{
    HRESULT hr = S_OK;
    WSADATA wsaData         = { 0 };
    WORD wVersionRequested  = MAKEWORD( 2, 2 );

    INT iErr = WSAStartup(wVersionRequested, &wsaData);
    if (iErr != 0)
    {
        hr = HRESULT_FROM_WIN32(iErr);
        return;
    }

    if (argc < 2)
    {
        printf("Usage: %s [-install] [-run]\n", argv[0]);
    }
    else if (_stricmp(argv[1], "/install") == 0 ||
             _stricmp(argv[1], "-install") == 0)
    {
        INT iResult = DoInstall(g_SampleProviderId);
        if (iResult != NO_ERROR)
        {
            // If the service provider is already installed, this will return 
            // an error.  Reinstalling the service provider will insure there
            // are no other installation issues.
            //
            DoUninstall(g_SampleProviderId);
            iResult = DoInstall(g_SampleProviderId);
        }

        if (iResult != NO_ERROR)
        {
            printf("Failed to install provider.  Be sure you are running as admin.  Err=0x%x\n", WSAGetLastError());
        }
        else
        {
            printf("Successfully installed provider.  Please start service by running: %s [-run]\n", argv[0]);
        }
    }
    else
    {
        // We are doing normal startup.  Need to inform winsock about the entry points 
        // to our NSP routines
        //
        NSPV2_ROUTINE NspRoutines = {0};

        NspRoutines.cbSize = sizeof(NspRoutines);
        NspRoutines.dwMajorVersion = 1;
        NspRoutines.dwMinorVersion = 0;
        NspRoutines.NSPv2Startup = NSPv2Startup;
        NspRoutines.NSPv2Cleanup = NSPv2Cleanup;
        NspRoutines.NSPv2LookupServiceBegin = NSPv2LookupServiceBegin;  
        NspRoutines.NSPv2LookupServiceNextEx = NSPv2LookupServiceNextEx;
        NspRoutines.NSPv2LookupServiceEnd = NSPv2LookupServiceEnd;  
        NspRoutines.NSPv2SetServiceEx = NSPv2SetServiceEx;
        NspRoutines.NSPv2ClientSessionRundown = NULL;

        InitializeCriticalSection(&g_cs);

        // Inform winsock that we are ready to receive registrations and lookups,
        //
        if (WSAAdvertiseProvider(&g_SampleProviderId, &NspRoutines) != 0)
        {
            printf("WSAAdvertiseProvider failed. (0x%x)\n", WSAGetLastError());
            return;
        }

        printf("Hit Enter to quit...\n");
        getchar();

        // All done, do some cleanup
        //
        WSAUnadvertiseProvider(&g_SampleProviderId);
        RemoveAllRegistrations();
    }

    if (WSACleanup() != NO_ERROR)
    {
        fprintf(stderr, "WSACleanup failed WSAError = 0x%x\n", WSAGetLastError());
    }    
}
