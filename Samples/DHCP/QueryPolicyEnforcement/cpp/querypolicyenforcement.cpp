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

#include<stdio.h>
#include<windows.h>
#include<dhcpsapi.h>

int __cdecl main(void)
{
    LPWSTR pwszServer         = NULL;         // Server IP Address, a value of NULL signifies the current server (where program is executed)
    DWORD  dwSubnet           = 0xa000000;    //(10.0.0.0) Subnet address
    DWORD  dwError            = ERROR_SUCCESS;// Variable that holds the error code
    BOOL   bIsPolicyActivated = TRUE;

    // DhcpV4QueryPolicyEnforcement sets bPolicyActivated to TRUE if policies are enabled for the scope "dwSubnet" on the server, otherwise
    // bPolicyActivated is set to FALSE.
    dwError = DhcpV4QueryPolicyEnforcement(
                     pwszServer,        // Server IP Address, a value of NULL means the current server (where the program is executed)
                    (dwSubnet == 0),    // fGlobalPolicy, signifies whether the policy enforcement is queried for server or a specific scope
                    dwSubnet,           // Subnet Address
                    &bIsPolicyActivated //PBOOL to hold the results of whether the policies are enabled or not
                    );
    if(ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV4QueryPolicyEnforcement failed with Error = %d\n", dwError);
    }
    if(bIsPolicyActivated)
        wprintf(L"Policy is enabled for scope %d\n",dwSubnet);
    else 
        wprintf(L"Policy is disabled for scope %d\n", dwSubnet);
    return 0;
}
