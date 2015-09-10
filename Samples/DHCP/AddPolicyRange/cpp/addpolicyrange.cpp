// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

/*
Assumptions:
    1) There exists a scope 10.0.0.0
    2) There exists a policy with the name "testPolicy" on subnet address 10.0.0.0
*/


#include<windows.h>
#include<stdio.h>
#include<dhcpsapi.h>

#ifndef OPTION_USER_CLASS
#define OPTION_USER_CLASS 77
#endif

int __cdecl main(void)
{
    DHCP_IP_RANGE range          = {0};           // Variable to hold the range value to be added to the policy.
    DWORD         dwError        = ERROR_SUCCESS; // Variable to store the error code
    LPWSTR        pwszPolicyName = L"testPolicy"; // Name of the policy
    LPWSTR        pwszServer     = NULL;          // Server IP Address, Null signifies the current server (where the program is run)
    DWORD         dwScope        = 0xa000000;     //(10.0.0.0) Subnet address
    DWORD         dwStartAddress = 0xa00000a;     //(10.0.0.10) Start Address for the range
    DWORD         dwEndAddress   = 0xa000032;     //(10.0.0.50) End Address for the range

    range.StartAddress = dwStartAddress;
    range.EndAddress   = dwEndAddress;

    // Adds a policy range for the policy "testPolicy" on subnet address 10.0.0.0
    dwError = DhcpV4AddPolicyRange(
                          pwszServer,    //ServerIpAddress, if this is NULL, it means the current server on which the program is run
                          dwScope,       // SubnetAddress
                          pwszPolicyName,// Name of the policy
                          &range         // Range to be added
                          );
    if(ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV4AddPolicyRange failed with Error = %d\n",dwError);
    }
    return 0;
}
