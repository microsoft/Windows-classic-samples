// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

/*
Assumptions:
    1) There exists a scope 10.0.0.0
    2) The scope 10.0.0.0 is part of some relationship
*/

#include<windows.h>
#include<stdio.h>
#include<dhcpsapi.h>

// This routine frees  LPDHCP_FAILOVER_RELATIONSHIP and its internal elements.
VOID FreeRelationshipMemory(LPDHCP_FAILOVER_RELATIONSHIP pFailRel)
{
    if (NULL != pFailRel) 
    {
        // Frees relationship name
        if (NULL != pFailRel->RelationshipName)
            DhcpRpcFreeMemory(pFailRel->RelationshipName);

        // Frees shared secret
        if( NULL != pFailRel->SharedSecret)
            DhcpRpcFreeMemory(pFailRel->SharedSecret);

        //Frees Primary server's name
        if (NULL != pFailRel->PrimaryServerName)
            DhcpRpcFreeMemory(pFailRel->PrimaryServerName);

        // Frees Secondary server's name
        if (NULL != pFailRel->SecondaryServerName)
            DhcpRpcFreeMemory(pFailRel->SecondaryServerName);

        //Frees pScopes
        if (NULL != pFailRel->pScopes)
        {
            //Frees  individual elements of pScopes
            if(NULL != pFailRel->pScopes->Elements)
            {
                DhcpRpcFreeMemory(pFailRel->pScopes->Elements);
            }
             DhcpRpcFreeMemory(pFailRel->pScopes);
        }
        //Frees the relationship
        DhcpRpcFreeMemory(pFailRel);
        pFailRel=NULL;
    }
}

int __cdecl main(void)
{
    LPDHCP_FAILOVER_RELATIONSHIP pRelationShip        = NULL;          // Failover relationship
    LPWSTR                       pwszServer           = NULL;          // Server IP Address
    DWORD                        dwError              = ERROR_SUCCESS; // Variable to hold error code
    DWORD                        dwScope              = 0xa000000;     // Subnet Address

    // This API fetches the information about the relationship of a specific scope
    dwError = DhcpV4FailoverGetScopeRelationship(
                        pwszServer,    // Server IP Address, a value of NULL reflects current server (where program is executed)
                        dwScope,       // Subnet Address
                        &pRelationShip // Failover relationship
                        );
    if( ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV4FailoverGetRelationship failed with Error = %d\n",dwError);
        goto cleanup;
    }
    wprintf(L"Relationship name = %s\n",pRelationShip->RelationshipName);
    FreeRelationshipMemory(pRelationShip);
cleanup:
    return 0;
}
