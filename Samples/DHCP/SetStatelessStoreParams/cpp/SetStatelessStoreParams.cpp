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
    DWORD                      dwError          = ERROR_SUCCESS;
    LPWSTR                     pwszServer       = NULL;
    DHCP_IPV6_ADDRESS          subnetAddress    = {0};
    DWORD                      dwFieldModified  = 0;
    BOOL                       bServerLevel     = TRUE;
    DHCPV6_STATELESS_PARAMS    storeParams      = {0};

    dwFieldModified |= DhcpStatelessPurgeInterval; // The parameters that need to be modified
    storeParams.PurgeInterval = 1; // 1 hour
    
    dwError = DhcpV6SetStatelessStoreParams(
                pwszServer,      // Server IP Address, a value of NULL reflects the current server (where the program is executed),
                bServerLevel,    // Server level or Scope level
                subnetAddress,   // Contains the IPv6 subnet address of the stateless client
                dwFieldModified, // Flag to reflect the fields which are getting modified in storeParams
                &storeParams      //Pointer to a DHCPV6_STATELESS_PARAMS structure that contains the stateless client inventory configuration settings 
                );
    if(ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV6SetStatelessStoreParams failed with Error = %d\n", dwError);
    }
    return 0;
}
