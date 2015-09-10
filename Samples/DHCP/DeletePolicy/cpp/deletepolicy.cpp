// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

/*
Assumptions:
    1) There exists a scope 10.0.0.0
    2) There exists a policy with the name "PolicyNewName" on scope 10.0.0.0
*/

#include<stdio.h>
#include<windows.h>
#include<dhcpsapi.h>

int __cdecl main(void)
{
    LPWSTR        pwszServer = NULL;             //NULL signifies current server
    DWORD         dwScope    = 0xa000000;        // (10.0.0.0) Subnet Address
    LPWSTR        pwszName   = L"PolicyNewName"; // Name of the policy to be deleted
    DWORD         dwError    = ERROR_SUCCESS;    // Variable to hold the error code.

    dwError = DhcpV4DeletePolicy(
                        pwszServer,     // ServerIpAddress, from where the policy needs to be deleted
                        (dwScope == 0), // fGloabalPolicy, true in case it is a global policy, for global policyies subnetAddress is zero.
                        dwScope,        // SubnetAddress
                        pwszName        // Name of the policy to be deleted.
                        );
    if(ERROR_SUCCESS != dwError)
    {
        //DhcpV4DeletePolicy returned error.
        wprintf(L"DhcpV4DeletePolicy failed with error %d\n",dwError);
    }
    return 0;
}
