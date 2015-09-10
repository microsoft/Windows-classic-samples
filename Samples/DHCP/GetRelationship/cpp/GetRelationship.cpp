// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

/*
Assumptions:
    1) There exists a relationship with the name "test"
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
    LPWSTR                       pwszRelationshipName = L"test";       // Relationship name to be fetched
    DWORD                        dwError              = ERROR_SUCCESS; // Variable to hold error code
    dwError = DhcpV4FailoverGetRelationship(
                        pwszServer,           // Server IP Address, if NULL, reflects the current server (where the program is executed)
                        pwszRelationshipName, // Relationship name
                        &pRelationShip        // Failover relationship
                        );
    if( ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV4FailoverGetRelationship failed with Error = %d\n",dwError);
    }
    FreeRelationshipMemory(pRelationShip);
    return 0;
}
