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
    DHCP_OPTION_SCOPE_INFO scopeInfo;
    LPDHCP_ALL_OPTION_VALUES_PB pOptionValues = NULL;

    ZeroMemory( &scopeInfo, sizeof(scopeInfo) );
    scopeInfo.ScopeType = DhcpSubnetOptions;
    scopeInfo.ScopeInfo.SubnetScopeInfo =dwScope;

    dwError = DhcpV4GetAllOptionValues(
                        pwszServer,
                        0,
                        &scopeInfo,
                        &pOptionValues);
    if(ERROR_SUCCESS != dwError)
    {
        //DhcpV4GetAllOptionValues returned error.
        wprintf(L"DhcpV4GetAllOptionValues failed with error %d\n",dwError);
    }
    if(pOptionValues != NULL)
    {
        DhcpRpcFreeMemory(pOptionValues);
    }
    return 0;
}
