/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All Rights Reserved.

Abstract:
    This C++ file includes sample code that adds a GRE rule for the
    currently active profiles using the Microsoft Windows Firewall APIs.

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

    INetFwPolicy2 *pNetFwPolicy2 = NULL;
    INetFwRules *pFwRules = NULL;
    INetFwRule *pFwRule = NULL;

    long CurrentProfilesBitMask = 0;

    // The rule name, description, and group are provided as indirect strings for 
    // localization purposes. These resource strings can be found in the rc file

    BSTR bstrRuleName = SysAllocString(L"@Add_GRE_Rule.exe,-128");
    BSTR bstrRuleDescription = SysAllocString(L"@Add_GRE_Rule.exe,-129");
    BSTR bstrRuleGroup = SysAllocString(L"@Add_GRE_Rule.exe,-127");

    // Error checking for BSTR allocations
    if (NULL == bstrRuleName) { printf("Failed to allocate bstrRuleName\n"); goto Cleanup; }
    if (NULL == bstrRuleDescription) { printf("Failed to allocate bstrRuleDescription\n"); goto Cleanup; }
    if (NULL == bstrRuleGroup) { printf("Failed to allocate bstrRuleGroup\n"); goto Cleanup; }

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

    // Retrieve INetFwRules
    hr = pNetFwPolicy2->get_Rules(&pFwRules);
    if (FAILED(hr))
    {
        printf("get_Rules failed: 0x%08lx\n", hr);
        goto Cleanup;
    }

    // Retrieve Current Profiles bitmask
    hr = pNetFwPolicy2->get_CurrentProfileTypes(&CurrentProfilesBitMask);
    if (FAILED(hr))
    {
        printf("get_CurrentProfileTypes failed: 0x%08lx\n", hr);
        goto Cleanup;
    }

    // When possible we avoid adding firewall rules to the Public profile.
    // If Public is currently active and it is not the only active profile, we remove it from the bitmask
    if ((CurrentProfilesBitMask & NET_FW_PROFILE2_PUBLIC) &&
        (CurrentProfilesBitMask != NET_FW_PROFILE2_PUBLIC))
    {
        CurrentProfilesBitMask ^= NET_FW_PROFILE2_PUBLIC;
    }

    // Create a new Firewall Rule object.
    hr = CoCreateInstance(
                __uuidof(NetFwRule),
                NULL,
                CLSCTX_INPROC_SERVER,
                __uuidof(INetFwRule),
                (void**)&pFwRule);
    if (FAILED(hr))
    {
        printf("CoCreateInstance for Firewall Rule failed: 0x%08lx\n", hr);
        goto Cleanup;
    }

    // Populate the Firewall Rule Name
    hr = pFwRule->put_Name(bstrRuleName);
    if (FAILED(hr))
    {
        printf("put_Name failed: 0x%08lx\n", hr);
        goto Cleanup;
    }

    // Populate the Firewall Rule Description
    hr = pFwRule->put_Description(bstrRuleDescription);
    if (FAILED(hr))
    {
        printf("put_Description failed: 0x%08lx\n", hr);
        goto Cleanup;
    }

    // Populate the Firewall Rule Protocol
    hr = pFwRule->put_Protocol(47);
    if (FAILED(hr))
    {
        printf("put_Protocol failed: 0x%08lx\n", hr);
        goto Cleanup;
    }

    // Populate the Firewall Rule Group
    hr = pFwRule->put_Grouping(bstrRuleGroup);
    if (FAILED(hr))
    {
        printf("put_Grouping failed: 0x%08lx\n", hr);
        goto Cleanup;
    }

    // Populate the Firewall Rule Profiles
    hr = pFwRule->put_Profiles(CurrentProfilesBitMask);
    if (FAILED(hr))
    {
        printf("put_Profiles failed: 0x%08lx\n", hr);
        goto Cleanup;
    }

    // Populate the Firewall Rule Action
    hr = pFwRule->put_Action(NET_FW_ACTION_ALLOW);
    if (FAILED(hr))
    {
        printf("put_Action failed: 0x%08lx\n", hr);
        goto Cleanup;
    }

    // Populate the Firewall Rule Enabled
    hr = pFwRule->put_Enabled(VARIANT_TRUE);
    if (FAILED(hr))
    {
        printf("put_Enabled failed: 0x%08lx\n", hr);
        goto Cleanup;
    }

    // Add the Firewall Rule
    hr = pFwRules->Add(pFwRule);
    if (FAILED(hr))
    {
        printf("Firewall Rule Add failed: 0x%08lx\n", hr);
        goto Cleanup;
    }

Cleanup:

    // Free BSTR's
    SysFreeString(bstrRuleName);
    SysFreeString(bstrRuleDescription);
    SysFreeString(bstrRuleGroup);

    // Release the INetFwRule object
    if (pFwRule != NULL)
    {
        pFwRule->Release();
    }

    // Release the INetFwRules object
    if (pFwRules != NULL)
    {
        pFwRules->Release();
    }

    // Release the INetFwPolicy2 object
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
