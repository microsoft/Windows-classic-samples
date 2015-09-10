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

    ZeroMemory( &scopeInfo, sizeof(scopeInfo) );
    scopeInfo.ScopeType = DhcpSubnetOptions;
    scopeInfo.ScopeInfo.SubnetScopeInfo =dwScope;

    dwError = DhcpV4RemoveOptionValue(
                        pwszServer,
                        0,
                        dwOptionId,
                        pwszName,
                        pwszVendorName,
                        &scopeInfo);
    if(ERROR_SUCCESS != dwError)
    {
        //DhcpV4RemoveOptionValue returned error.
        wprintf(L"DhcpV4RemoveOptionValue failed with error %d\n",dwError);
    }
    return 0;
}
