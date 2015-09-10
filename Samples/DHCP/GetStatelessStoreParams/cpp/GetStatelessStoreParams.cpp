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

int __cdecl main(void)
{
    DWORD                      dwError        = ERROR_SUCCESS; // Variable to hold Error Codes
    LPWSTR                     pwszServer     = NULL;          // Variable to hold Server IP Address
    DHCP_IPV6_ADDRESS          subnetAddress  = {0};           // Subnet
    BOOL                       bServerLevel   = TRUE;          // Variable to signify scope or server level
    LPDHCPV6_STATELESS_PARAMS  pStoreParams   = NULL;          // Variable to hold stateless store params.

    
    dwError = DhcpV6GetStatelessStoreParams(
                pwszServer,      // Server IP Address, a value of NULL reflects the current server (where the program is executed),
                bServerLevel,    // Server level or Scope level
                subnetAddress,   // Contains the IPv6 subnet address of the stateless client
                &pStoreParams    //Pointer to a DHCPV6_STATELESS_PARAMS structure that contains the stateless client inventory configuration settings 
                );
    if(ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV6GetStatelessStoreParams failed with Error = %d\n", dwError);
        goto cleanup;
    }
    wprintf(L"Status = %d PurgeInterval: %u\n",pStoreParams->Status,pStoreParams->PurgeInterval);
    
cleanup:
    if(NULL != pStoreParams)
    {
        DhcpRpcFreeMemory(pStoreParams);
        pStoreParams = NULL;
    }
    return 0;
}
