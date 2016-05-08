/********************************************************************
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All Rights Reserved.

Abstract:
    Sample code for 'Windows Firewall with Advanced Security' COM interfaces.

    Illustrates how to add firewall rule with the EdgeTraversalOptions.

    Note that in order for Windows Firewall to dynamically allow edge traffic to the application 
    (i.e allow edge traffic only when the app indicates so, block otherwise) two things must be done:
    1. Application should use the IPV6_PROTECTION_LEVEL socket option on the listening socket
        and set it to PROTECTION_LEVEL_UNRESTRICTED whenever it wants to receive edge traffic. And reset it
        back to other options when edge traffic is not needed.
    2. The Windows Firewall rule added for the application should set 
            EdgeTraversalOptions = NET_FW_EDGE_TRAVERSAL_TYPE_DEFER_TO_APP

    For help on the IPV6_PROTECTION_LEVEL socket option refer Winsock reference documentation on MSDN. 

    This sample only illustrates one of the EdgeTraversalOptions values.
    You can find the complete set of Windows 7 supported EdgeTraversalOptions values by looking up the 
    enumerated type 'NET_FW_EDGE_TRAVERSAL_TYPE' for 'Windows Firewall with Advanced Security' on MSDN.
    Posting values here for quick reference but always check MSDN for correct values:
        NET_FW_EDGE_TRAVERSAL_TYPE_DENY               = 0  'always block edge traffic.
        NET_FW_EDGE_TRAVERSAL_TYPE_ALLOW              = 1  'always allow edge traffic.
        NET_FW_EDGE_TRAVERSAL_TYPE_DEFER_TO_APP       = 2  'dynamically allow edge traffic based on when app sets the IPV6_PROTECTION_LEVEL socket option to 'Unrestricted'.
        NET_FW_EDGE_TRAVERSAL_TYPE_DEFER_TO_USER      = 3  'generate Windows Security Alert to ask the user to permit edge traffic when app sets the IPV6_PROTECTION_LEVEL socket option to 'Unrestricted'.
                                                           'Note that in order to use the DEFER_TO_USER option, the firewall rule must only have application path and protocol scopes specified, nothing more, nothing less.


********************************************************************/

#include <windows.h>
#include <stdio.h>
#include <netfw.h>
#include <strsafe.h>

#include "resource.h"


#define STRING_BUFFER_SIZE  500     


// Forward declarations
HRESULT    WFCOMInitialize(INetFwPolicy2** ppNetFwPolicy2);
void       WFCOMCleanup(INetFwPolicy2* pNetFwPolicy2);
HRESULT    AddFirewallRuleWithEdgeTraversal(__in INetFwPolicy2 *pNetFwPolicy2);


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

    // Add firewall rule with EdgeTraversalOption=DeferApp (Windows7+) if available 
    //   else add with Edge=True (Vista and Server 2008).
    AddFirewallRuleWithEdgeTraversal(pNetFwPolicy2);

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

// Add firewall rule with EdgeTraversalOption=DeferApp (Windows7+) if available 
//   else add with Edge=True (Vista and Server 2008).
HRESULT    AddFirewallRuleWithEdgeTraversal(__in INetFwPolicy2 *pNetFwPolicy2)
{
    HRESULT hr = S_OK;
    INetFwRules *pNetFwRules = NULL;
    
    INetFwRule  *pNetFwRule = NULL;
    INetFwRule2 *pNetFwRule2 = NULL;

    WCHAR pwszTemp[STRING_BUFFER_SIZE] = L"";

    BSTR RuleName = NULL;
    BSTR RuleGroupName = NULL;
    BSTR RuleDescription = NULL;
    BSTR RuleAppPath = NULL;

    
    hr = StringCchPrintfW(pwszTemp, STRING_BUFFER_SIZE, L"@EdgeTraversalOptions.exe,-%d", RULE_NAME_STRING_ID);
    if (FAILED(hr))
    {
        wprintf(L"Failed to compose a resource identifier string: 0x%08lx\n", hr);
        goto Cleanup;        
    }
    RuleName = SysAllocString(pwszTemp);
    if (NULL == RuleName)
    {
        wprintf(L"\nERROR: Insufficient memory\n");
        goto Cleanup;
    }

    hr = StringCchPrintfW(pwszTemp, STRING_BUFFER_SIZE, L"@EdgeTraversalOptions.exe,-%d", RULE_GROUP_STRING_ID);
    if (FAILED(hr))
    {
        wprintf(L"Failed to compose a resource identifier string: 0x%08lx\n", hr);
        goto Cleanup;        
    }
    RuleGroupName = SysAllocString(pwszTemp);  // Used for grouping together multiple rules
    if (NULL == RuleGroupName)
    {
        wprintf(L"\nERROR: Insufficient memory\n");
        goto Cleanup;
    }

    hr = StringCchPrintfW(pwszTemp, STRING_BUFFER_SIZE, L"@EdgeTraversalOptions.exe,-%d", RULE_DESCRIPTION_STRING_ID);
    if (FAILED(hr))
    {
        wprintf(L"Failed to compose a resource identifier string: 0x%08lx\n", hr);
        goto Cleanup;        
    }
    RuleDescription = SysAllocString(pwszTemp);
    if (NULL == RuleDescription)
    {
        wprintf(L"\nERROR: Insufficient memory\n");
        goto Cleanup;
    }

    RuleAppPath = SysAllocString(L"%ProgramFiles%\\EdgeTraversalOptions\\EdgeTraversalOptions.exe");
    if (NULL == RuleAppPath)
    {
        wprintf(L"\nERROR: Insufficient memory\n");
        goto Cleanup;
    }

    hr = pNetFwPolicy2->get_Rules(&pNetFwRules);

    if (FAILED(hr))
    {
        wprintf(L"Failed to retrieve firewall rules collection : 0x%08lx\n", hr);
        goto Cleanup;        
    }

    hr = CoCreateInstance(
        __uuidof(NetFwRule),    //CLSID of the class whose object is to be created
        NULL, 
        CLSCTX_INPROC_SERVER, 
        __uuidof(INetFwRule),   // Identifier of the Interface used for communicating with the object
        (void**)&pNetFwRule);

    if (FAILED(hr))
    {
        wprintf(L"CoCreateInstance for INetFwRule failed: 0x%08lx\n", hr);
        goto Cleanup;        
    }

    hr = pNetFwRule->put_Name(RuleName);
    if ( FAILED(hr) )
    {
        wprintf(L"Failed INetFwRule::put_Name failed with error: 0x %x.\n", hr);
        goto Cleanup;
    }

    hr = pNetFwRule->put_Grouping(RuleGroupName);
    if ( FAILED(hr) )
    {
        wprintf(L"Failed INetFwRule::put_Grouping failed with error: 0x %x.\n", hr);
        goto Cleanup;
    }

    hr = pNetFwRule->put_Description(RuleDescription);
    if ( FAILED(hr) )
    {
        wprintf(L"Failed INetFwRule::put_Description failed with error: 0x %x.\n", hr);
        goto Cleanup;
    }

    hr = pNetFwRule->put_Direction(NET_FW_RULE_DIR_IN);
    if ( FAILED(hr) )
    {
        wprintf(L"Failed INetFwRule::put_Direction failed with error: 0x %x.\n", hr);
        goto Cleanup;
    }

    hr = pNetFwRule->put_Action(NET_FW_ACTION_ALLOW);
    if ( FAILED(hr) )
    {
        wprintf(L"Failed INetFwRule::put_Action failed with error: 0x %x.\n", hr);
        goto Cleanup;
    }

    hr = pNetFwRule->put_ApplicationName(RuleAppPath);
    if ( FAILED(hr) )
    {
        wprintf(L"Failed INetFwRule::put_ApplicationName failed with error: 0x %x.\n", hr);
        goto Cleanup;
    }

    hr = pNetFwRule->put_Protocol(6);  // TCP
    if ( FAILED(hr) )
    {
        wprintf(L"Failed INetFwRule::put_Protocol failed with error: 0x %x.\n", hr);
        goto Cleanup;
    }

    hr = pNetFwRule->put_Profiles(NET_FW_PROFILE2_DOMAIN | NET_FW_PROFILE2_PRIVATE);
    if ( FAILED(hr) )
    {
        wprintf(L"Failed INetFwRule::put_Profiles failed with error: 0x %x.\n", hr);
        goto Cleanup;
    }

    hr = pNetFwRule->put_Enabled(VARIANT_TRUE);
    if ( FAILED(hr) )
    {
        wprintf(L"Failed INetFwRule::put_Enabled failed with error: 0x %x.\n", hr);
        goto Cleanup;
    }


    // Check if INetFwRule2 interface is available (i.e Windows7+)
    // If supported, then use EdgeTraversalOptions
    // Else use the EdgeTraversal boolean flag.

    if (SUCCEEDED(pNetFwRule->QueryInterface(__uuidof(INetFwRule2), (void**)&pNetFwRule2)))
    {
        hr = pNetFwRule2->put_EdgeTraversalOptions(NET_FW_EDGE_TRAVERSAL_TYPE_DEFER_TO_APP);
        if ( FAILED(hr) )
        {
            wprintf(L"Failed INetFwRule::put_EdgeTraversalOptions failed with error: 0x %x.\n", hr);
            goto Cleanup;
        }
    }
    else
    {
        hr = pNetFwRule->put_EdgeTraversal(VARIANT_TRUE);
        if ( FAILED(hr) )
        {
            wprintf(L"Failed INetFwRule::put_EdgeTraversal failed with error: 0x %x.\n", hr);
            goto Cleanup;
        }
    }

    hr = pNetFwRules->Add(pNetFwRule);
    if (FAILED(hr))
    {
        wprintf(L"Failed to add firewall rule to the firewall rules collection : 0x%08lx\n", hr);
        goto Cleanup;        
    }

    wprintf(L"Successfully added firewall rule !\n");

Cleanup:
   
    SysFreeString(RuleName);
    SysFreeString(RuleGroupName);
    SysFreeString(RuleDescription);
    SysFreeString(RuleAppPath);

    if (pNetFwRule2 != NULL)
    {
        pNetFwRule2->Release();
    }

    if (pNetFwRule != NULL)
    {
        pNetFwRule->Release();
    }

    if (pNetFwRules != NULL)
    {
        pNetFwRules->Release();
    }

    return hr;
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