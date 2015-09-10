// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

/*
Assumptions:
    1) There exists a scope 10.0.0.0
    2) There exists a policy with the name "testPolicy" on scope 10.0.0.0
    3) There exists a policy range in the "testPolicy" with start address 10.0.0.10 and end address 10.0.0.50
*/

#include<stdio.h>
#include<windows.h>
#include<dhcpsapi.h>

int __cdecl main(void)
{
    DHCP_IP_RANGE range          = {0};           // Variable to hold the range
    DWORD         dwError        = ERROR_SUCCESS; // Variable to hold the error code
    LPWSTR        pwszPolicyName = L"testPolicy"; // Name of the policy
    LPWSTR        pwszServer     = NULL;          //Server IP Address, NULL signifies the current server (where the program is executed)
    DWORD         dwScope        = 0xa000000;     //(10.0.0.0) subnet address
    DWORD         dwStartAddress = 0xa00000a;     //(10.0.0.10) start address of the range
    DWORD         dwEndAddress   = 0xa000032;     //(10.0.0.50) end address of the range

    range.StartAddress = dwStartAddress;
    range.EndAddress   = dwEndAddress;

    // Removes the range from the policy
    dwError = DhcpV4RemovePolicyRange(
        pwszServer,     //Server IP Address, NULL signifies the current server (where the program is executed)
        dwScope,        // Subnet Address
        pwszPolicyName, // Name of the policy
        &range          // Range that needs to be removed
        );
    if(ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV4RemovePolicyRange failed with Error = %d\n",dwError);
    }
    return 0;
}
