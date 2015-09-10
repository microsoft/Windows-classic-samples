// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include<stdio.h>
#include<windows.h>
#include<intsafe.h>
#include<dhcpsapi.h>


// This routine frees  LPDHCP_POLICY and its internal elements.
VOID FreeDhcpPolicyMemory(DHCP_POLICY pDhcpPolicy)
{
    // Frees the policy name
    if (NULL != pDhcpPolicy.PolicyName)
        DhcpRpcFreeMemory(pDhcpPolicy.PolicyName);

    // Frees the policy description
    if( NULL != pDhcpPolicy.Description)
        DhcpRpcFreeMemory(pDhcpPolicy.Description);

    // Frees the policy condition
    if(NULL != pDhcpPolicy.Conditions)
    {
        for(DWORD dwIndex = 0; dwIndex<pDhcpPolicy.Conditions->NumElements; dwIndex++)
        {
            // Frees the vendorName holder in condition's elements, if it exists
            if( NULL != pDhcpPolicy.Conditions->Elements[dwIndex].VendorName)
                DhcpRpcFreeMemory(pDhcpPolicy.Conditions->Elements[dwIndex].VendorName);

            // Frees the "bytes" used for storing condition values
            if (NULL != pDhcpPolicy.Conditions->Elements[dwIndex].Value)
                DhcpRpcFreeMemory(pDhcpPolicy.Conditions->Elements[dwIndex].Value);
        }
        DhcpRpcFreeMemory(pDhcpPolicy.Conditions);
    }

    // Frees the policy expression
    if(NULL != pDhcpPolicy.Expressions)
    {
        // Frees the expression elements, if they exist
        if(pDhcpPolicy.Expressions->NumElements && pDhcpPolicy.Expressions->Elements)
            DhcpRpcFreeMemory(pDhcpPolicy.Expressions->Elements);
        DhcpRpcFreeMemory(pDhcpPolicy.Expressions);
    }

    // Frees the policy ranges
    if(NULL != pDhcpPolicy.Ranges)
    {
        // Frees the individual range elements
        if(pDhcpPolicy.Ranges->Elements)
            DhcpRpcFreeMemory(pDhcpPolicy.Ranges->Elements);
        DhcpRpcFreeMemory(pDhcpPolicy.Ranges);
    }
}

// This routine frees  LPDHCP_POLICY_ARRAY and its internal elements.
VOID FreeDhcpPolicyInfoMemory(LPDHCP_POLICY_ARRAY pEnumPoliciesInfo)
{
    if(NULL != pEnumPoliciesInfo)
    {
        for(DWORD dwIndex=0; dwIndex < pEnumPoliciesInfo->NumElements; dwIndex++)
        {
            FreeDhcpPolicyMemory(pEnumPoliciesInfo->Elements[dwIndex]);
        }
        DhcpRpcFreeMemory(pEnumPoliciesInfo);
        pEnumPoliciesInfo = NULL;
    }
}

int __cdecl main(void)
{
    DWORD                dwError          = ERROR_MORE_DATA;
    DWORD                dwElementsRead   = 0;
    DWORD                dwScopeId        = 0xa000000;
    DWORD                dwElementsTotal  = 0;
    DWORD                preferredMax     = DWORD_MAX;
    LPWSTR               pwszServer       = NULL;
    LPDHCP_POLICY_ARRAY  enumPoliciesInfo = NULL;
    DWORD                resumeHandle     = 0;
    DWORD                dwIdx            = 0;
    for(;;)
    {
        dwError = DhcpV4EnumPolicies(
                    pwszServer,        // Server IP Address, a value of NULL reflects the current server (where the program is executed)
                    &resumeHandle,     // Resume Handle
                    preferredMax,      // Preferred maximum
                    (dwScopeId==0),    // fGlobalPolicy, global policy or scope level policy
                    dwScopeId,         // Subnet Address
                    &enumPoliciesInfo, // DHCP_POLICY_ARRAY containing policies
                    &dwElementsRead,   // Total number of elements read
                    &dwElementsTotal   // Total number of elements
                    );
        if ((NO_ERROR != dwError) &&
            (ERROR_MORE_DATA != dwError) &&
            (ERROR_NO_MORE_ITEMS != dwError ))
        {
            wprintf(L"Error in enumerating policies. Error = %d\n",dwError);
            break;
        }
        if (dwElementsRead && enumPoliciesInfo)
        {
            //operate on enumPoliciesInfo, for example in the below snippet, we are printing the names of the 
            // enumerated policies.
            for ( dwIdx = 0; dwIdx < enumPoliciesInfo->NumElements; dwIdx++ )
            {
                wprintf(L"policies name = %s \n",enumPoliciesInfo->Elements[dwIdx].PolicyName);
            }
            //one needs to free the enumPoliciesInfo once used
            FreeDhcpPolicyInfoMemory(enumPoliciesInfo);
            enumPoliciesInfo = NULL;
        }
        if (ERROR_NO_MORE_ITEMS == dwError || (dwError == ERROR_SUCCESS && dwElementsTotal == 0))
        {
            dwError=ERROR_SUCCESS;
            break;
        }
        dwElementsRead = 0;
        dwElementsTotal = 0;
    }
    return 0;
}
