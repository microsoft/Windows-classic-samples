// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include<stdio.h>
#include<windows.h>
#include<dhcpsapi.h>

int __cdecl main(void)
{
    LPWSTR        pwszServer = NULL; //NULL signifies current server
    DWORD         dwScope    = 0xa000000;
    DWORD         dwError    = ERROR_SUCCESS;
    LPWSTR        pwszName   = L"testPolicy";
    LPWSTR        pwszVendorName   = NULL;
    DWORD        dwOptionId             =2;
    DHCP_OPTION_SCOPE_INFO scopeInfo;
    DHCP_OPTION_DATA optionData = {0};

    ZeroMemory( &scopeInfo, sizeof(scopeInfo) );
    scopeInfo.ScopeType = DhcpSubnetOptions;
    scopeInfo.ScopeInfo.SubnetScopeInfo =dwScope;

    optionData.NumElements = 1;
    optionData.Elements = (LPDHCP_OPTION_DATA_ELEMENT)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(DHCP_OPTION_DATA_ELEMENT));
    if(optionData.Elements == NULL)
    {
        wprintf(L"Not Enough memory\n");
        goto cleanup;
    }
    optionData.Elements->OptionType=DhcpDWordOption;
    optionData.Elements->Element.DWordOption = 0x5;
    dwError = DhcpV4SetOptionValue(
                        pwszServer,
                        0,
                        dwOptionId,
                        pwszName,
                        pwszVendorName,
                        &scopeInfo,
                        &optionData);
    if(ERROR_SUCCESS != dwError)
    {
        //DhcpV4SetOptionValue returned error.
        wprintf(L"DhcpV4SetOptionValue failed with error %d\n",dwError);
    }
    if(optionData.Elements != NULL)
    {
        HeapFree(GetProcessHeap(),0, optionData.Elements);
    }
cleanup:
    return 0;
}
