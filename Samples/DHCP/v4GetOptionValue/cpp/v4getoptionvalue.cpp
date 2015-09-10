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
    LPDHCP_OPTION_VALUE pOptionValue = NULL;

    ZeroMemory( &scopeInfo, sizeof(scopeInfo) );
    scopeInfo.ScopeType = DhcpSubnetOptions;
    scopeInfo.ScopeInfo.SubnetScopeInfo =dwScope;

    dwError = DhcpV4GetOptionValue(
                        pwszServer,
                        0,
                        dwOptionId,
                        pwszName,
                        pwszVendorName,
                        &scopeInfo,
                        &pOptionValue);
    if(ERROR_SUCCESS != dwError)
    {
        //DhcpV4GetOptionValue returned error.
        wprintf(L"DhcpV4GetOptionValue failed with error %d\n",dwError);
    }
    if(pOptionValue != NULL)
    {
        DhcpRpcFreeMemory(pOptionValue);
    }
    return 0;
}
