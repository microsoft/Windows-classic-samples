/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All Rights Reserved.

Abstract:
    This C++ file includes sample code that restricts a service using
    the Microsoft Windows Firewall APIs.

--********************************************************************/


#include <windows.h>
#include <stdio.h>
#include <netfw.h>


// Forward declarations
HRESULT     WFCOMInitialize(INetFwPolicy2** ppNetFwPolicy2);


int __cdecl main()
{
    HRESULT hrComInit = S_OK;
    HRESULT hr = S_OK;

    VARIANT_BOOL isServiceRestricted = FALSE;

    INetFwPolicy2 *pNetFwPolicy2 = NULL;
    INetFwServiceRestriction *pFwServiceRestriction = NULL;

    // The Service and App name to use
    BSTR bstrServiceName = SysAllocString(L"policyagent");
    BSTR bstrAppName = SysAllocString(L"%systemDrive%\\WINDOWS\\system32\\svchost.exe");

    // Error checking for BSTR allocations
    if (NULL == bstrServiceName) { printf("Failed to allocate bstrServiceName\n"); goto Cleanup; }
    if (NULL == bstrAppName) { printf("Failed to allocate bstrAppName\n"); goto Cleanup; }

    // Initialize COM.
    hrComInit = CoInitializeEx(
                    0,
                    COINIT_APARTMENTTHREADED
                    );

    // Ignore RPC_E_CHANGED_MODE; this just means that COM has already been
    // initialized with a different mode. Since we don't care what the mode is,
    // we'll just use the existing mode.
    if (hrComInit != RPC_E_CHANGED_MODE)
    {
        if (FAILED(hrComInit))
        {
            printf("CoInitializeEx failed: 0x%08lx\n", hrComInit);
            goto Cleanup;
        }
    }

    // Retrieve INetFwPolicy2
    hr = WFCOMInitialize(&pNetFwPolicy2);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    // Retrieve INetFwServiceRestriction
    hr = pNetFwPolicy2->get_ServiceRestriction(&pFwServiceRestriction);
    if (FAILED(hr))
    {
        printf("get_ServiceRestriction failed: 0x%08lx\n", hr);
        goto Cleanup;
    }

    // Restrict the policyagent Service
    hr = pFwServiceRestriction->RestrictService(bstrServiceName, bstrAppName, TRUE, FALSE);
    if (FAILED(hr))
    {
        printf("RestrictService failed: 0x%08lx\n", hr);
        goto Cleanup;
    }

    // Check to see if the Service is Restricted
    hr = pFwServiceRestriction->ServiceRestricted(bstrServiceName, bstrAppName, &isServiceRestricted);
    if (FAILED(hr))
    {
        printf("ServiceRestricted failed: 0x%08lx\n", hr);
        goto Cleanup;
    }

    if (isServiceRestricted)
    {
        printf ("Service got restricted. All connections to the service will be blocked.");
    }
    else
    {
        printf ("The Service is not restricted.");
    }

Cleanup:

    // Free BSTR's
    SysFreeString(bstrServiceName);
    SysFreeString(bstrAppName);

    // Release INetFwPolicy2
    if (pNetFwPolicy2 != NULL)
    {
        pNetFwPolicy2->Release();
    }

    // Uninitialize COM.
    if (SUCCEEDED(hrComInit))
    {
        CoUninitialize();
    }
   
    return 0;
}


// Instantiate INetFwPolicy2
HRESULT WFCOMInitialize(INetFwPolicy2** ppNetFwPolicy2)
{
    HRESULT hr = S_OK;

    hr = CoCreateInstance(
        __uuidof(NetFwPolicy2), 
        NULL, 
        CLSCTX_INPROC_SERVER, 
        __uuidof(INetFwPolicy2), 
        (void**)ppNetFwPolicy2);

    if (FAILED(hr))
    {
        printf("CoCreateInstance for INetFwPolicy2 failed: 0x%08lx\n", hr);
        goto Cleanup;        
    }

Cleanup:
    return hr;
}
