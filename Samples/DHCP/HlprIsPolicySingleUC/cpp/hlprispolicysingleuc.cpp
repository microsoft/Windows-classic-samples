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
*/

#include<stdio.h>
#include<windows.h>
#include<dhcpsapi.h>

// This routine frees  LPDHCP_POLICY and its internal elements.
VOID FreeDhcpPolicyMemory(LPDHCP_POLICY pDhcpPolicy)
{
    if (NULL != pDhcpPolicy) 
    {
        // Frees the policy name
        if (NULL != pDhcpPolicy->PolicyName)
            DhcpRpcFreeMemory(pDhcpPolicy->PolicyName);

        // Frees the policy description
        if( NULL != pDhcpPolicy->Description)
            DhcpRpcFreeMemory(pDhcpPolicy->Description);

        // Frees the policy condition
        if(NULL != pDhcpPolicy->Conditions)
        {
            for(DWORD dwIndex = 0; dwIndex<pDhcpPolicy->Conditions->NumElements; dwIndex++)
            {
                // Frees the vendorName holder in condition's elements, if it exists
                if( NULL != pDhcpPolicy->Conditions->Elements[dwIndex].VendorName)
                    DhcpRpcFreeMemory(pDhcpPolicy->Conditions->Elements[dwIndex].VendorName);

                // Frees the "bytes" used for storing condition values
                if (NULL != pDhcpPolicy->Conditions->Elements[dwIndex].Value)
                    DhcpRpcFreeMemory(pDhcpPolicy->Conditions->Elements[dwIndex].Value);
            }
            DhcpRpcFreeMemory(pDhcpPolicy->Conditions);
        }

        // Frees the policy expression
        if(NULL != pDhcpPolicy->Expressions)
        {
            // Frees the expression elements, if they exist
            if(pDhcpPolicy->Expressions->NumElements && pDhcpPolicy->Expressions->Elements)
                DhcpRpcFreeMemory(pDhcpPolicy->Expressions->Elements);
            DhcpRpcFreeMemory(pDhcpPolicy->Expressions);
        }

        // Frees the policy ranges
        if(NULL != pDhcpPolicy->Ranges)
        {
            // Frees the individual range elements
            if(pDhcpPolicy->Ranges->Elements)
                DhcpRpcFreeMemory(pDhcpPolicy->Ranges->Elements);
            DhcpRpcFreeMemory(pDhcpPolicy->Ranges);
        }

        // Finally frees the policy structure
        DhcpRpcFreeMemory(pDhcpPolicy);
        pDhcpPolicy=NULL;
    }
}

int __cdecl main(void)
{
    LPDHCP_POLICY pPolicy    = NULL;          // Variable to hold the DHCP Policy structure
    LPWSTR        pwszServer = NULL;          //Server IP Address, NULL signifies current server (where the program is executed)
    DWORD         dwScope    = 0xa000000;     // Subnet Address (10.0.0.0)
    DWORD         dwError    = ERROR_SUCCESS; // Variable to hold error code
    LPWSTR        pwszName   = L"testPolicy"; // Policy Name
    BOOL          bIsUCBased = FALSE;
    dwError = DhcpV4GetPolicy(
                    pwszServer,     // Server IP Address, NULL signifies the current server (where the program is executed)
                    (dwScope == 0), // fGlobalPolicy, TRUE means a global policy, for a global policy SubnetAddress is 0.
                    dwScope,        // Subnet address, if it is a global policy, its value is 0
                    pwszName,       // Name of the policy
                    &pPolicy        // Policy structure obtained from the server
                    );
    if(ERROR_SUCCESS != dwError)
    {
        //DhcpV4GetPolicy returned error.
        wprintf(L"DhcpV4GetPolicy failed with error %d\n",dwError);
        goto cleanup;
    }
    // DhcpHlprIsV4PolicySingleUC returns true or false.
    // True means the policy contains single user class based conditions.
    // False means the policy in question doesnt contain just one user class based condition
    bIsUCBased = DhcpHlprIsV4PolicySingleUC(pPolicy);
    if(bIsUCBased)
        wprintf(L"Policy contains single user class based condition\n");
    else
        wprintf(L"Policy does not contain just one user class based condition\n");
    FreeDhcpPolicyMemory(pPolicy);
cleanup:
    return 0;
}
