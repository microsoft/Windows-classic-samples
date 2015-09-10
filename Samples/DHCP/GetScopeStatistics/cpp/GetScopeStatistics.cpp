// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

/*
Assumptions:
    1) There exists a scope 10.0.0.0
*/

#include<windows.h>
#include<stdio.h>
#include<dhcpsapi.h>

int __cdecl main(void)
{
    LPDHCP_FAILOVER_STATISTICS pStats     = NULL;          // Variable to hold Failover statistics for a scope
    DHCP_IP_ADDRESS            dwScope    = 0xa000000;     //(10.0.0.0) Subnet Address
    LPWSTR                     pwszServer = NULL;          // Server IP Address
    DWORD                      dwError    = ERROR_SUCCESS; // Variable to hold error values

    // This API returns the statistics w.r.t a specific scope.
    dwError = DhcpV4FailoverGetScopeStatistics(
                    pwszServer, // Server IP Address
                    dwScope,    // Subnet address
                    &pStats     // Failover statistics for the scope
                    );
    if(ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV4FailoverGetScopeStatistics failed with Error = %d\n", dwError);
        goto cleanup;
    }
    wprintf(L"Statistics:\n");
    wprintf(L"Number of addresses: %d\n",pStats->NumAddr);
    wprintf(L"Number of free addresses: %d\n",pStats->AddrFree);
    wprintf(L"Number of addresses in use : %d\n",pStats->AddrInUse);
    wprintf(L"Number of free partner addresses: %d\n",pStats->PartnerAddrFree);
    wprintf(L"Number of partner addresses in use: %d\n",pStats->PartnerAddrInUse);
cleanup:
    if(NULL != pStats)
    {
        DhcpRpcFreeMemory(pStats);
        pStats = NULL;
    }
    return 0;
}
