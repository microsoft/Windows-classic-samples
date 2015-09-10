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


// This routine frees  LPDHCPV6_STATELESS_STATS
VOID FreeDhcpv6StatelessStats(LPDHCPV6_STATELESS_STATS pStatelessStats)
{
    if(NULL != pStatelessStats)
    {
        if(NULL != pStatelessStats->ScopeStats)
        {
            DhcpRpcFreeMemory(pStatelessStats->ScopeStats);
        }
        DhcpRpcFreeMemory(pStatelessStats);
        pStatelessStats = NULL;

    }
}

int __cdecl main(void)
{
    DWORD                     dwError           = ERROR_SUCCESS; // Variable to hold Error Codes
    LPWSTR                    pwszServer        = NULL;          // Variable to hold Server IP Address
    LPDHCPV6_STATELESS_STATS  pStatelessStats   = NULL;          // Variable to hold stateless statistics
    DWORD                     dwIdx             = 0;
    
    dwError = DhcpV6GetStatelessStatistics(
                pwszServer,      // Server IP Address, a value of NULL reflects the current server (where the program is executed)
                &pStatelessStats //Pointer to a DHCPV6_STATELESS_STATS structure that contain DHCPv6 stateless server IPv6 subnet statistics
                );
    if(ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV6GetStatelessStatistics failed with Error = %d\n", dwError);
        goto cleanup;
    }
    if(pStatelessStats->NumScopes == 0 || pStatelessStats->ScopeStats == NULL)
    {
        wprintf(L"No scopes are found\n");
        goto cleanup;
    }
    
    wprintf(L"Statistics:\n");
    for(dwIdx = 0; dwIdx < pStatelessStats->NumScopes; dwIdx++)
    {
        wprintf(L"Scope High Ordered bits: %llu Low Ordered bits: %llu \n",pStatelessStats->ScopeStats[dwIdx].SubnetAddress.HighOrderBits, pStatelessStats->ScopeStats[dwIdx].SubnetAddress.LowOrderBits);
        wprintf(L"NumStatelessClientsAdded   : %llu\n",pStatelessStats->ScopeStats[dwIdx].NumStatelessClientsAdded);
        wprintf(L"NumStatelessClientsRemoved : %llu\n",pStatelessStats->ScopeStats[dwIdx].NumStatelessClientsRemoved);
    }

cleanup:
    if(NULL != pStatelessStats)
    {
        FreeDhcpv6StatelessStats(pStatelessStats);
        pStatelessStats = NULL;
    }
    return 0;
}
