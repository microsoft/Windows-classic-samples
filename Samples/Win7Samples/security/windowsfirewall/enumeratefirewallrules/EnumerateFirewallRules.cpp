/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All Rights Reserved.

Abstract:
    This C++ file includes sample code for enumerating Windows Firewall
    rules using the Microsoft Windows Firewall APIs.

********************************************************************/

#include <windows.h>
#include <stdio.h>
#include <comutil.h>
#include <atlcomcli.h>
#include <netfw.h>

#define NET_FW_IP_PROTOCOL_TCP_NAME L"TCP"
#define NET_FW_IP_PROTOCOL_UDP_NAME L"UDP"

#define NET_FW_RULE_DIR_IN_NAME L"In"
#define NET_FW_RULE_DIR_OUT_NAME L"Out"

#define NET_FW_RULE_ACTION_BLOCK_NAME L"Block"
#define NET_FW_RULE_ACTION_ALLOW_NAME L"Allow"

#define NET_FW_RULE_ENABLE_IN_NAME L"TRUE"
#define NET_FW_RULE_DISABLE_IN_NAME L"FALSE"


// Forward declarations
void        DumpFWRulesInCollection(INetFwRule* FwRule);
HRESULT     WFCOMInitialize(INetFwPolicy2** ppNetFwPolicy2);


int __cdecl main()
{
    HRESULT hrComInit = S_OK;
    HRESULT hr = S_OK;

    ULONG cFetched = 0; 
    CComVariant var;

    IUnknown *pEnumerator;
    IEnumVARIANT* pVariant = NULL;

    INetFwPolicy2 *pNetFwPolicy2 = NULL;
    INetFwRules *pFwRules = NULL;
    INetFwRule *pFwRule = NULL;

    long fwRuleCount;

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

    // Retrieve INetFwRules
    hr = pNetFwPolicy2->get_Rules(&pFwRules);
    if (FAILED(hr))
    {
        wprintf(L"get_Rules failed: 0x%08lx\n", hr);
        goto Cleanup;
    }

    // Obtain the number of Firewall rules
    hr = pFwRules->get_Count(&fwRuleCount);
    if (FAILED(hr))
    {
        wprintf(L"get_Count failed: 0x%08lx\n", hr);
        goto Cleanup;
    }
    
    wprintf(L"The number of rules in the Windows Firewall are %d\n", fwRuleCount);

    // Iterate through all of the rules in pFwRules
    pFwRules->get__NewEnum(&pEnumerator);

    if(pEnumerator)
    {
        hr = pEnumerator->QueryInterface(__uuidof(IEnumVARIANT), (void **) &pVariant);
    }

    while(SUCCEEDED(hr) && hr != S_FALSE)
    {
        var.Clear();
        hr = pVariant->Next(1, &var, &cFetched);

        if (S_FALSE != hr)
        {
            if (SUCCEEDED(hr))
            {
                hr = var.ChangeType(VT_DISPATCH);
            }
            if (SUCCEEDED(hr))
            {
                hr = (V_DISPATCH(&var))->QueryInterface(__uuidof(INetFwRule), reinterpret_cast<void**>(&pFwRule));
            }

            if (SUCCEEDED(hr))
            {
                // Output the properties of this rule
                DumpFWRulesInCollection(pFwRule);
            }
        }
    }
 
Cleanup:

    // Release pFwRule
    if (pFwRule != NULL)
    {
        pFwRule->Release();
    }

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


// Output properties of a Firewall rule 
void DumpFWRulesInCollection(INetFwRule* FwRule)
{
    variant_t InterfaceArray;
    variant_t InterfaceString;  

    VARIANT_BOOL bEnabled;
    BSTR bstrVal;

    long lVal = 0;
    long lProfileBitmask = 0;

    NET_FW_RULE_DIRECTION fwDirection;
    NET_FW_ACTION fwAction;

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

    wprintf(L"---------------------------------------------\n");

    if (SUCCEEDED(FwRule->get_Name(&bstrVal)))
    {
        wprintf(L"Name:             %s\n", bstrVal);
    }

    if (SUCCEEDED(FwRule->get_Description(&bstrVal)))
    {
        wprintf(L"Description:      %s\n", bstrVal);
    }

    if (SUCCEEDED(FwRule->get_ApplicationName(&bstrVal)))
    {
        wprintf(L"Application Name: %s\n", bstrVal);
    }

    if (SUCCEEDED(FwRule->get_ServiceName(&bstrVal)))
    {
        wprintf(L"Service Name:     %s\n", bstrVal);
    }

    if (SUCCEEDED(FwRule->get_Protocol(&lVal)))
    {
        switch(lVal)
        {
            case NET_FW_IP_PROTOCOL_TCP: 

                wprintf(L"IP Protocol:      %s\n", NET_FW_IP_PROTOCOL_TCP_NAME);
                break;

            case NET_FW_IP_PROTOCOL_UDP: 

                wprintf(L"IP Protocol:      %s\n", NET_FW_IP_PROTOCOL_UDP_NAME);
                break;

            default:

                break;
        }

        if(lVal != NET_FW_IP_VERSION_V4 && lVal != NET_FW_IP_VERSION_V6)
        {
            if (SUCCEEDED(FwRule->get_LocalPorts(&bstrVal)))
            {
                wprintf(L"Local Ports:      %s\n", bstrVal);
            }

            if (SUCCEEDED(FwRule->get_RemotePorts(&bstrVal)))
            {
                wprintf(L"Remote Ports:      %s\n", bstrVal);
            }
        }
        else
        {
            if (SUCCEEDED(FwRule->get_IcmpTypesAndCodes(&bstrVal)))
            {
                wprintf(L"ICMP TypeCode:      %s\n", bstrVal);
            }
        }
    }

    if (SUCCEEDED(FwRule->get_LocalAddresses(&bstrVal)))
    {
        wprintf(L"LocalAddresses:   %s\n", bstrVal);
    }

    if (SUCCEEDED(FwRule->get_RemoteAddresses(&bstrVal)))
    {
        wprintf(L"RemoteAddresses:  %s\n", bstrVal);
    }

    if (SUCCEEDED(FwRule->get_Profiles(&lProfileBitmask)))
    {
        // The returned bitmask can have more than 1 bit set if multiple profiles 
        //   are active or current at the same time

        for (int i=0; i<3; i++)
        {
            if ( lProfileBitmask & ProfileMap[i].Id  )
            {
                wprintf(L"Profile:  %s\n", ProfileMap[i].Name);
            }
        }
    }

    if (SUCCEEDED(FwRule->get_Direction(&fwDirection)))
    {
        switch(fwDirection)
        {
            case NET_FW_RULE_DIR_IN:

                wprintf(L"Direction:        %s\n", NET_FW_RULE_DIR_IN_NAME);
                break;

            case NET_FW_RULE_DIR_OUT:

                wprintf(L"Direction:        %s\n", NET_FW_RULE_DIR_OUT_NAME);
                break;

            default:

                break;
        }
    }

    if (SUCCEEDED(FwRule->get_Action(&fwAction)))
    {
        switch(fwAction)
        {
            case NET_FW_ACTION_BLOCK:

                wprintf(L"Action:           %s\n", NET_FW_RULE_ACTION_BLOCK_NAME);
                break;

            case NET_FW_ACTION_ALLOW:

                wprintf(L"Action:           %s\n", NET_FW_RULE_ACTION_ALLOW_NAME);
                break;

            default:

                break;
        }
    }

    if (SUCCEEDED(FwRule->get_Interfaces(&InterfaceArray)))
    {
        if(InterfaceArray.vt != VT_EMPTY)
        {
            SAFEARRAY    *pSa = NULL;

            pSa = InterfaceArray.parray;

            for(long index= pSa->rgsabound->lLbound; index < (long)pSa->rgsabound->cElements; index++)
            {
                SafeArrayGetElement(pSa, &index, &InterfaceString);
                wprintf(L"Interfaces:       %s\n", (BSTR)InterfaceString.bstrVal);
            }
        }
    }

    if (SUCCEEDED(FwRule->get_InterfaceTypes(&bstrVal)))
    {
        wprintf(L"Interface Types:  %s\n", bstrVal);
    }

    if (SUCCEEDED(FwRule->get_Enabled(&bEnabled)))
    {
        if (bEnabled)
        {
            wprintf(L"Enabled:          %s\n", NET_FW_RULE_ENABLE_IN_NAME);
        }
        else
        {
            wprintf(L"Enabled:          %s\n", NET_FW_RULE_DISABLE_IN_NAME);
        }
    }

    if (SUCCEEDED(FwRule->get_Grouping(&bstrVal)))
    {
        wprintf(L"Grouping:         %s\n", bstrVal);
    }

    if (SUCCEEDED(FwRule->get_EdgeTraversal(&bEnabled)))
    {
        if (bEnabled)
        {
            wprintf(L"Edge Traversal:   %s\n", NET_FW_RULE_ENABLE_IN_NAME);
        }
        else
        {
            wprintf(L"Edge Traversal:   %s\n", NET_FW_RULE_DISABLE_IN_NAME);
        }
    }
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
