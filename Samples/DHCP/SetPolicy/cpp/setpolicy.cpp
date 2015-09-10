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
    LPDHCP_POLICY   pPolicy            = NULL;             // Variable that holds the DHCP_POLICY structure
    DWORD           dwError            = ERROR_SUCCESS;    // Variable that holds the error code
    DWORD           dwFieldsModified   = 0;                // Flag to reflect which all fields from DHCP_POLICY are modified
    DHCP_IP_ADDRESS dwSubnet           = 0xa000000;        // SubnetAddress
    LPWSTR          pwszName           = L"testPolicy";    // Name of the policy
    LPWSTR          pwszNewName        = L"PolicyNewName"; // New name of the policy
    LPWSTR          pwszServer         = NULL;             //Server IP Address, NULL signifies the current server (where the program is executed)

    dwError = DhcpV4GetPolicy(
                        pwszServer,      // Server IP Address, NULL signifies the current server (where the program is executed)
                        (dwSubnet == 0), // fGlobalPolicy, TRUE means a global policy, for a global policy SubnetAddress is 0.
                        dwSubnet,        // Subnet address, if it is a global policy, its value is 0
                        pwszName,        // Name of the policy
                        &pPolicy         // Policy structure obtained from the server
                        );
    if(ERROR_SUCCESS != dwError)
    {
        //DhcpV4GetPolicy returned error.
        wprintf(L"DhcpV4GetPolicy failed with error %d\n",dwError);
        return 0;
    }
    // Frees the policy name
    DhcpRpcFreeMemory(pPolicy->PolicyName);

    pPolicy->PolicyName = pwszNewName;
    dwFieldsModified |= DhcpUpdatePolicyName;

    // DhcpV4SetPolicy reads the value for "dwFieldsModified" and whichever fields from DHCP_POLICY are getting modified,
    // are taken from "pPolicy" and the necessary modification is performed.
    // For example, "dwFieldsModified" has values "DhcpUpdatePolicyName". This means the "Name" field of DHCP_POLICY 
    // need to be modified. In that case, during modification, "Name" field is queried from "pPolicy" and necessary modification
    // is performed.
    dwError=DhcpV4SetPolicy(
                pwszServer,       // Server IP Address, NULL signifies the current server (where the program is executed)
                dwFieldsModified, // Fields modified, flag to reflect which all parameters of DHCP_POLICY needs to be modified
                (dwSubnet == 0),  // fGloabalPolicy, whether the policy is for a specific scope or at server level. if subnetAddress is 0, it means it is at server level
                dwSubnet,         // Subnet Address
                pwszName,         // Name of the policy
                pPolicy           // policy structure whose values will form the modified policy
                );
    if(ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV4SetPolicy failed with Error = %d\n", dwError);
    }
    FreeDhcpPolicyMemory(pPolicy);
    return 0;
}
