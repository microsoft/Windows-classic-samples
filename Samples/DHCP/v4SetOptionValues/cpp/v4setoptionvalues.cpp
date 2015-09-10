// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include<stdio.h>
#include<windows.h>
#include<dhcpsapi.h>

#define NUM_OPTION_VALUES 2
int __cdecl main(void)
{
    LPWSTR        pwszServer = NULL; //NULL signifies current server
    DWORD         dwScope    = 0xa000000;
    DWORD         dwIndex = 0;
    DWORD         dwError    = ERROR_SUCCESS;
    LPWSTR        pwszName   = L"testPolicy";
    LPWSTR        pwszVendorName   = NULL;
    DHCP_OPTION_SCOPE_INFO scopeInfo;
    DHCP_OPTION_VALUE_ARRAY optionValueArray = {0};

    ZeroMemory( &scopeInfo, sizeof(scopeInfo) );
    scopeInfo.ScopeType = DhcpSubnetOptions;
    scopeInfo.ScopeInfo.SubnetScopeInfo =dwScope;

    optionValueArray.NumElements = 2;
    optionValueArray.Values = (LPDHCP_OPTION_VALUE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (NUM_OPTION_VALUES*sizeof(DHCP_OPTION_VALUE)));
    if(optionValueArray.Values == NULL)
    {
        wprintf(L"Not Enough memory\n");
        goto cleanup;
    }
    optionValueArray.Values[0].OptionID = 0xd;
    optionValueArray.Values[0].Value.NumElements = 1;
    optionValueArray.Values[0].Value.Elements=(LPDHCP_OPTION_DATA_ELEMENT)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(DHCP_OPTION_DATA_ELEMENT));
    if(optionValueArray.Values[0].Value.Elements == NULL)
    {
        wprintf(L"Not Enough Memory\n");
        goto cleanup;
    }
    optionValueArray.Values[0].Value.Elements->OptionType = DhcpDWordOption;
    optionValueArray.Values[0].Value.Elements->Element.DWordOption = 0xa;

    optionValueArray.Values[1].OptionID = 0x3;
    optionValueArray.Values[1].Value.NumElements = 1;
    optionValueArray.Values[1].Value.Elements=(LPDHCP_OPTION_DATA_ELEMENT)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(DHCP_OPTION_DATA_ELEMENT));
    if(optionValueArray.Values[1].Value.Elements == NULL)
    {
        wprintf(L"Not Enough Memory\n");
        goto cleanup;
    }
    optionValueArray.Values[1].Value.Elements->OptionType = DhcpIpAddressOption;
    optionValueArray.Values[1].Value.Elements->Element.IpAddressOption = 0xa000004;

    dwError = DhcpV4SetOptionValues(
                        pwszServer,
                        0,
                        pwszName,
                        pwszVendorName,
                        &scopeInfo,
                        &optionValueArray);
    if(ERROR_SUCCESS != dwError)
    {
        //DhcpV4SetOptionValue returned error.
        wprintf(L"DhcpV4SetOptionValue failed with error %d\n",dwError);
    }
cleanup:
    if(optionValueArray.Values != NULL)
    {
        for(dwIndex = 0; dwIndex < optionValueArray.NumElements; dwIndex++)
        {
            if((optionValueArray.Values[dwIndex].Value.Elements != NULL))
            {
                HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, optionValueArray.Values[dwIndex].Value.Elements);
            }
        }
        HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, optionValueArray.Values);
    }
    return 0;
}
