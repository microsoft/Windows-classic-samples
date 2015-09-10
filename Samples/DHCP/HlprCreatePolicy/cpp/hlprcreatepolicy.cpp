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
    LPDHCP_POLICY       pPolicy         = NULL;                 // Variable to hold the policy structure
    DWORD               dwError         = ERROR_SUCCESS;        // Variable to hold the error code
    DHCP_IP_ADDRESS     dwSubnet        = 0;                    // Subnet Address, a value of 0 means server level policy
    LPWSTR              pwszName        = L"testPolicy";        // Name of the policy that needs to be created
    LPWSTR              pwszDescription = L"PolicyDescription"; // Description of the policy
    DHCP_POL_LOGIC_OPER policyOperator  = DhcpLogicalOr;        // Root operator

    //Helper routine is invoked to create/fill the policy structure.
    dwError=DhcpHlprCreateV4Policy(
        pwszName,        // Policy Name
        (dwSubnet == 0), // fGloabalPolicy, if scope is zero, this means it is a global policy else it is for a specific scope
        dwSubnet,        // Scope
        0,               // Processing order
        policyOperator,  // Logical operator, possible values are: DhcpLogicalOr, DhcpLogicalAnd
        pwszDescription, // Policy description
        TRUE,            // Policy active or not
        &pPolicy         // This is the actual structure that holds the policy
    );
    if(ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpHlprCreateV4Policy failed with Error = %d\n", dwError);
        goto cleanup;
    }
    // Frees the policy structure
    if(NULL != pPolicy)
    {
        DhcpHlprFreeV4Policy(pPolicy);
    }
cleanup:
    return 0;
}
