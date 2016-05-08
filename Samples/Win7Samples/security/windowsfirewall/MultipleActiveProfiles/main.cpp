/*
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All Rights Reserved.


    SYNOPSIS

        Sample code for 'Windows Firewall with Advanced Security' COM interfaces.

        Illustrates correct usage of following methods/properties of INetFwPolicy2 COM interface
        when multiple firewall profiles are active/current at the same time:
          - CurrentProfileTypes
          - IsRuleGroupCurrentlyEnabled
          - IsRuleGroupEnabled
          - LocalPolicyModifyState
*/


#include <windows.h>
#include <stdio.h>
#include <netfw.h>

// Forward declarations
HRESULT    WFCOMInitialize(INetFwPolicy2** ppNetFwPolicy2);
void       WFCOMCleanup(INetFwPolicy2* pNetFwPolicy2);
HRESULT    GetCurrentFirewallState(__in INetFwPolicy2 *pNetFwPolicy2);
HRESULT    IsRuleGroupEnabled(__in INetFwPolicy2 *pNetFwPolicy2);
HRESULT    IsRuleGroupCurrentlyEnabled(__in INetFwPolicy2 *pNetFwPolicy2);
HRESULT    GetLocalPolicyModifyState(__in INetFwPolicy2 *pNetFwPolicy2);


int __cdecl wmain()
{
    HRESULT hrComInit = S_OK;
    HRESULT hr = S_OK;
    INetFwPolicy2 *pNetFwPolicy2 = NULL;

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
            wprintf(L"CoInitializeEx failed: 0x%08lx\n", hrComInit);
            goto Cleanup;
        }
    }

    // Retrieve INetFwPolicy2
    hr = WFCOMInitialize(&pNetFwPolicy2);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    // Show Firewall ON/OFF state on current profiles
    GetCurrentFirewallState(pNetFwPolicy2);

    // Show status of 'File and Printer Sharing' rule group on current profiles
    IsRuleGroupCurrentlyEnabled(pNetFwPolicy2);

    // Show status of 'File and Printer Sharing' rule group on specified profiles
    IsRuleGroupEnabled(pNetFwPolicy2);

    // For the current firewall profiles display whether the changes to firewall rules 
    //  will take effect or not
    GetLocalPolicyModifyState(pNetFwPolicy2);

Cleanup:

    // Release INetFwPolicy2
    WFCOMCleanup(pNetFwPolicy2);

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
        wprintf(L"CoCreateInstance for INetFwPolicy2 failed: 0x%08lx\n", hr);
        goto Cleanup;        
    }

Cleanup:
    return hr;
}


// Release INetFwPolicy2
void WFCOMCleanup(INetFwPolicy2* pNetFwPolicy2)
{
    // Release the INetFwPolicy2 object (Vista+)
    if (pNetFwPolicy2 != NULL)
    {
        pNetFwPolicy2->Release();
    }
}


// For the currently active firewall profiles display whether firewall is ON or OFF
HRESULT GetCurrentFirewallState(__in INetFwPolicy2 *pNetFwPolicy2)
{
    HRESULT hr = S_FALSE;
    long    CurrentProfilesBitMask = 0;
    VARIANT_BOOL bActualFirewallEnabled = VARIANT_FALSE;
    struct ProfileMapElement 
    {
        NET_FW_PROFILE_TYPE2 Id;
        LPCWSTR Name;
    };
    ProfileMapElement ProfileMap[3];
    ProfileMap[0].Id = NET_FW_PROFILE2_DOMAIN;
    ProfileMap[0].Name = L"Domain";
    ProfileMap[1].Id = NET_FW_PROFILE2_PRIVATE;
    ProfileMap[1].Name = L"Private";
    ProfileMap[2].Id = NET_FW_PROFILE2_PUBLIC;
    ProfileMap[2].Name = L"Public";

    wprintf(L"\n\nCurrent Firewall State:\n");
    wprintf(L"-----------------------\n");

    hr = pNetFwPolicy2->get_CurrentProfileTypes(&CurrentProfilesBitMask);
    if (FAILED(hr))
    {
        wprintf(L"Failed to get CurrentProfileTypes. Error: %x.\n", hr);
        goto CLEANUP;
    }

    // The returned 'CurrentProfiles' bitmask can have more than 1 bit set if multiple profiles 
    //   are active or current at the same time

    for (int i=0; i<3; i++)
    {
        if ( CurrentProfilesBitMask & ProfileMap[i].Id  )
        {
            /*Is Firewall Enabled?*/
            hr = pNetFwPolicy2->get_FirewallEnabled(ProfileMap[i].Id, &bActualFirewallEnabled);
            if (FAILED(hr))
            {
                wprintf(L"Failed to get FirewallEnabled settings for %s profile. Error: %x.\n", ProfileMap[i].Name, hr);
                goto CLEANUP;
            }
            wprintf(L"On %s profile (Current) : Firewall state is %s\n", ProfileMap[i].Name, (bActualFirewallEnabled ? L"ON" : L"OFF"));
        }
    }


CLEANUP:
    return hr;
}


// For the currently active firewall profiles display whether the rule group is enabled or not
HRESULT IsRuleGroupCurrentlyEnabled(__in INetFwPolicy2 *pNetFwPolicy2)
{
    HRESULT hr = S_OK;
    VARIANT_BOOL bActualEnabled = VARIANT_FALSE;

    BSTR GroupName = SysAllocString(L"File and Printer Sharing");
    if (NULL == GroupName)
    {
        wprintf(L"\nERROR: Insufficient memory\n");
        goto Cleanup;
    }

    wprintf(L"\n\nIs 'File and Printer Sharing' rule group currently enabled ?\n");
    wprintf(L"------------------------------------------------------------\n");


    hr = pNetFwPolicy2->get_IsRuleGroupCurrentlyEnabled(GroupName, &bActualEnabled);

    if ( SUCCEEDED(hr) )
    {
        if (VARIANT_TRUE == bActualEnabled && S_OK == hr)
        {
            wprintf(L"Rule Group currently enabled on all the current profiles\n");
        }
        else if (VARIANT_TRUE == bActualEnabled && S_FALSE == hr)
        {
            wprintf(L"Rule Group currently enabled on some of the current profiles but not on all the current profiles\n");
        }
        else if (VARIANT_FALSE == bActualEnabled)
        {
            wprintf(L"Rule Group Currently not enabled on any of the current profiles\n");
        }
    }
    else
    {  
        wprintf(L"Failed calling API IsRuleGroupCurrentlyEnabled. Error: 0x %x.\n", hr);
        goto Cleanup;
    }
    

Cleanup:
    SysFreeString(GroupName);
    return hr;
}


// For the specified firewall profiles display whether the rule group is enabled or not
HRESULT IsRuleGroupEnabled(__in INetFwPolicy2 *pNetFwPolicy2)
{
    HRESULT hr = S_OK;
    VARIANT_BOOL bActualEnabled = VARIANT_FALSE;

    BSTR GroupName = SysAllocString(L"File and Printer Sharing");
    if (NULL == GroupName)
    {
        wprintf(L"\nERROR: Insufficient memory\n");
        goto Cleanup;
    }

    wprintf(L"\n\nIs 'File and Printer Sharing' rule group enabled in public and private profiles ?\n");
    wprintf(L"---------------------------------------------------------------------------------\n");

    hr = pNetFwPolicy2->IsRuleGroupEnabled(NET_FW_PROFILE2_PRIVATE | NET_FW_PROFILE2_PUBLIC, GroupName, &bActualEnabled);

    if ( SUCCEEDED(hr) )
    {
        if (VARIANT_TRUE == bActualEnabled && S_OK == hr)
        {
            wprintf(L"Rule Group currently enabled on both public and private profiles\n");
        }
        else if (VARIANT_TRUE == bActualEnabled && S_FALSE == hr)
        {
            wprintf(L"Rule Group currently enabled on either public or private profile but not both\n");
        }
        else if (VARIANT_FALSE == bActualEnabled)
        {
            wprintf(L"Rule Group currently disabled on both public and private profiles\n");
        }
    }
    else
    {  
        wprintf(L"Failed calling API IsRuleGroupCurrentlyEnabled. Error: 0x %x.\n", hr);
        goto Cleanup;
    }
    

Cleanup:
    SysFreeString(GroupName);
    return hr;
}


// For the currently active firewall profiles display whether the changes to firewall rules will take effect or not
HRESULT GetLocalPolicyModifyState(__in INetFwPolicy2 *pNetFwPolicy2)
{
    HRESULT hr;
    NET_FW_MODIFY_STATE modifystate;

    wprintf(L"\n\nWill changes to firewall rules take effect ?\n");
    wprintf(L"--------------------------------------------\n");

    hr = pNetFwPolicy2->get_LocalPolicyModifyState(&modifystate);
    if (FAILED(hr))
    {
        wprintf(L"Failed calling API get_LocalPolicyModifyState. Error: %x.\n", hr);
        return hr;
    }

    if(modifystate == NET_FW_MODIFY_STATE_OK)
    {
        if (hr == S_OK)
        {
            wprintf(L"Changing or adding firewall rule (or group) to the current profiles will take effect on all current profiles.\n");
        }
        else if (hr == S_FALSE)
        {
            wprintf(L"Changing or adding firewall rule (or group) to the current profiles will take effect on only some current profiles but not all.\n");
        }
    }
    else if(modifystate == NET_FW_MODIFY_STATE_GP_OVERRIDE)
    {
        if (hr == S_OK)
        {
            wprintf(L"Changing or adding a firewall rule (or group) to the current profiles will not take effect because group policy overrides it on all current profiles.\n");
        }
        else if (hr == S_FALSE)
        {
            wprintf(L"Changing or adding a firewall rule (or group) to the current profiles will not take effect because group policy overrides it on some of the current profiles.\n");
        }
    }
    else if(modifystate == NET_FW_MODIFY_STATE_INBOUND_BLOCKED)
    {
        if (hr == S_OK)
        {
            wprintf(L"Changing or adding firewall rule (or group) to the current profiles will not take effect because unsolicited inbound traffic is not allowed on all the current profiles.\n");
        }
        else if (hr == S_FALSE)
        {
            wprintf(L"Changing or adding firewall rule (or group) to the current profiles will not take effect because unsolicited inbound traffic is not allowed on some of the current profiles.\n");
        }
    }
    
    return hr;
}
