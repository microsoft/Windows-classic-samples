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
#include <winsock.h> // for getprotocolbyname
#include <atlcomcli.h>
#include <netfw.h>
#include <shlwapi.h> // for SHLoadIndirectString

#define IP_PROTOCOL_ICMP4  1
#define IP_PROTOCOL_ICMP6  58

// Forward declarations
void        DumpFWRulesInCollection(INetFwRule* FwRule);

void EnumerateFWRules()
{
    HRESULT hr;

    // Retrieve INetFwPolicy2
    CComPtr<INetFwPolicy2> pNetFwPolicy2;
    hr = pNetFwPolicy2.CoCreateInstance(__uuidof(NetFwPolicy2));
    if (FAILED(hr))
    {
        wprintf(L"CoCreateInstance failed: 0x%08lx\n", hr);
        return;
    }

    // Retrieve INetFwRules
    CComPtr<INetFwRules> pFwRules;
    hr = pNetFwPolicy2->get_Rules(&pFwRules);
    if (FAILED(hr))
    {
        wprintf(L"get_Rules failed: 0x%08lx\n", hr);
        return;
    }

    // Obtain the number of Firewall rules
    long fwRuleCount;
    hr = pFwRules->get_Count(&fwRuleCount);
    if (FAILED(hr))
    {
        wprintf(L"get_Count failed: 0x%08lx\n", hr);
        return;
    }

    wprintf(L"The number of rules in the Windows Firewall are %d\n", fwRuleCount);

    // Iterate through all of the rules in pFwRules
    CComPtr<IUnknown> pEnumerator;
    hr = pFwRules->get__NewEnum(&pEnumerator);
    if (FAILED(hr))
    {
        wprintf(L"get__NewEnum failed: 0x%08lx\n", hr);
        return;
    }

    CComPtr<IEnumVARIANT> pVariant;
    hr = pEnumerator.QueryInterface(&pVariant);
    if (FAILED(hr))
    {
        wprintf(L"get__NewEnum failed to produce IEnumVariant: 0x%08lx\n", hr);
        return;
    }

    ULONG cFetched = 0;
    for (CComVariant var; pVariant->Next(1, &var, &cFetched) == S_OK; var.Clear())
    {
        CComPtr<INetFwRule> pFwRule;
        if (SUCCEEDED(var.ChangeType(VT_DISPATCH)) &&
            SUCCEEDED(V_DISPATCH(&var)->QueryInterface(IID_PPV_ARGS(&pFwRule))))
        {
            // Output the properties of this rule
            DumpFWRulesInCollection(pFwRule);
        }
    }
}

void PrintLocalizableString(PCWSTR label, PCWSTR value)
{
    wchar_t buffer[256];
    if (value[0] == L'@' &&
        SUCCEEDED(SHLoadIndirectString(value, buffer, ARRAYSIZE(buffer), nullptr)))
    {
        value = buffer;
    }
    wprintf(L"%s%s\n", label, value);
}

// Output properties of a Firewall rule 
void DumpFWRulesInCollection(INetFwRule* FwRule)
{
    wprintf(L"---------------------------------------------\n");

    CComBSTR name;
    if (SUCCEEDED(FwRule->get_Name(&name)) && name)
    {
        PrintLocalizableString(L"Name:             ", name);
    }

    CComBSTR description;
    if (SUCCEEDED(FwRule->get_Description(&description)) && description)
    {
        PrintLocalizableString(L"Description:      ", description);
    }

    CComBSTR applicationName;
    if (SUCCEEDED(FwRule->get_ApplicationName(&applicationName)) && applicationName)
    {
        wprintf(L"Application Name: %ls\n", static_cast<BSTR>(applicationName));
    }

    CComBSTR serviceName;
    if (SUCCEEDED(FwRule->get_ServiceName(&serviceName)) && serviceName)
    {
        wprintf(L"Service Name:     %ls\n", static_cast<BSTR>(serviceName));
    }

    long protocolNumber = 0;
    if (SUCCEEDED(FwRule->get_Protocol(&protocolNumber)))
    {
        // Try to convert the protocol number to a name, for readability.
        PCSTR protocolName = nullptr;

        // This special value means "any protocol".
        if (protocolNumber == NET_FW_IP_PROTOCOL_ANY)
        {
            protocolName = "Any";
        }
        else
        {
            protoent* ent = getprotobynumber(protocolNumber);
            if (ent)
            {
                protocolName = ent->p_name;
            }
        }
        if (protocolName)
        {
            wprintf(L"IP Protocol:      %d (%hs)\n", protocolNumber, protocolName);
        }
        else
        {
            wprintf(L"IP Protocol:      %d\n", protocolNumber);
        }

        if (protocolNumber != IP_PROTOCOL_ICMP4 && protocolNumber != IP_PROTOCOL_ICMP6)
        {
            CComBSTR localPorts;
            if (SUCCEEDED(FwRule->get_LocalPorts(&localPorts)) && localPorts)
            {
                wprintf(L"Local Ports:      %ls\n", static_cast<BSTR>(localPorts));
            }

            CComBSTR remotePorts;
            if (SUCCEEDED(FwRule->get_RemotePorts(&remotePorts)) && remotePorts)
            {
                wprintf(L"Remote Ports:     %ls\n", static_cast<BSTR>(remotePorts));
            }
        }
        else
        {
            CComBSTR icmpTypesAndCodes;
            if (SUCCEEDED(FwRule->get_IcmpTypesAndCodes(&icmpTypesAndCodes)) && icmpTypesAndCodes)
            {
                wprintf(L"ICMP TypeCode:    %ls\n", static_cast<BSTR>(icmpTypesAndCodes));
            }
        }
    }

    CComBSTR localAddresses;
    if (SUCCEEDED(FwRule->get_LocalAddresses(&localAddresses)) && localAddresses)
    {
        wprintf(L"LocalAddresses:   %ls\n", static_cast<BSTR>(localAddresses));
    }

    CComBSTR remoteAddresses;
    if (SUCCEEDED(FwRule->get_RemoteAddresses(&remoteAddresses)) && remoteAddresses)
    {
        wprintf(L"RemoteAddresses:  %ls\n", static_cast<BSTR>(remoteAddresses));
    }

    long lProfileBitmask = 0;
    if (SUCCEEDED(FwRule->get_Profiles(&lProfileBitmask)))
    {
        // The returned bitmask can have more than 1 bit set if multiple profiles 
        // are active or current at the same time
        static const struct ProfileMapElement
        {
            NET_FW_PROFILE_TYPE2 Id;
            LPCWSTR Name;
        } ProfileMap[3] = {
            { NET_FW_PROFILE2_DOMAIN, L"Domain" },
            { NET_FW_PROFILE2_PRIVATE, L"Private" },
            { NET_FW_PROFILE2_PUBLIC, L"Public" },
        };

        for (ProfileMapElement const& entry : ProfileMap)
        {
            if (lProfileBitmask & entry.Id)
            {
                wprintf(L"Profile:          %ls\n", entry.Name);
            }
        }
    }

    NET_FW_RULE_DIRECTION fwDirection;
    if (SUCCEEDED(FwRule->get_Direction(&fwDirection)))
    {
        switch (fwDirection)
        {
        case NET_FW_RULE_DIR_IN:
            wprintf(L"Direction:        In\n");
            break;

        case NET_FW_RULE_DIR_OUT:
            wprintf(L"Direction:        Out\n");
            break;
        }
    }

    NET_FW_ACTION fwAction;
    if (SUCCEEDED(FwRule->get_Action(&fwAction)))
    {
        switch (fwAction)
        {
        case NET_FW_ACTION_BLOCK:
            wprintf(L"Action:           Block\n");
            break;

        case NET_FW_ACTION_ALLOW:
            wprintf(L"Action:           Allow\n");
            break;
        }
    }

    CComVariant InterfaceArray;
    if (SUCCEEDED(FwRule->get_Interfaces(&InterfaceArray)))
    {
        if (InterfaceArray.vt == (VT_VARIANT | VT_ARRAY))
        {
            SAFEARRAY* pSa = NULL;

            pSa = InterfaceArray.parray;

            for (long index = pSa->rgsabound->lLbound; index < (long)pSa->rgsabound->cElements; index++)
            {
                CComVariant InterfaceString;
                if (SUCCEEDED(SafeArrayGetElement(pSa, &index, &InterfaceString)) &&
                    (InterfaceString.vt == VT_BSTR))
                {
                    wprintf(L"Interfaces:       %ls\n", InterfaceString.bstrVal);
                }
            }
        }
    }

    CComBSTR interfaceTypes;
    if (SUCCEEDED(FwRule->get_InterfaceTypes(&interfaceTypes)) && interfaceTypes)
    {
        wprintf(L"Interface Types:  %ls\n", static_cast<BSTR>(interfaceTypes));
    }

    VARIANT_BOOL enabled;
    if (SUCCEEDED(FwRule->get_Enabled(&enabled)))
    {
        wprintf(L"Enabled:          %ls\n", enabled ? L"TRUE" : L"FALSE");
    }

    CComBSTR grouping;
    if (SUCCEEDED(FwRule->get_Grouping(&grouping)) && grouping)
    {
        PrintLocalizableString(L"Grouping:         ", grouping);
    }

    if (SUCCEEDED(FwRule->get_EdgeTraversal(&enabled)))
    {
        wprintf(L"Edge Traversal:   %ls\n", enabled ? L"TRUE" : L"FALSE");
    }
}

int __cdecl main()
{
    // Initialize COM.
    if (SUCCEEDED(CoInitialize(0)))
    {
        // We use WinSock only to convert protocol numbers to names.
        // If we cannot initialize WinSock, then just proceed without it.
        WSADATA wsaData;
        int err = WSAStartup(MAKEWORD(2, 2), &wsaData);

        EnumerateFWRules();

        // Clean up WinSock if we had started it.
        if (err == 0)
        {
            WSACleanup();
        }

        // Uninitialize COM.
        CoUninitialize();
    }

    return 0;
}

