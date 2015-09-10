// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

/*
Assumptions:
    1) There exists a relationship with the name "test"
*/

#include<windows.h>
#include<stdio.h>
#include<dhcpsapi.h>

int __cdecl main(void)
{
    DWORD  dwError              = ERROR_SUCCESS;
    LPWSTR pwszServerName       = NULL;
    LPWSTR pwszRelationshipName = L"test";

    // This triggers the address allocation
    dwError = DhcpV4FailoverTriggerAddrAllocation(
                        pwszServerName,      // Server IP Address, a value of NULL signifies the current server (where program is executed)
                        pwszRelationshipName // Relationship name
                        );
    if(ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV4FailoverTriggerAddrAllocation failed with Error = %d\n",dwError);
    }
    return 0;
}
