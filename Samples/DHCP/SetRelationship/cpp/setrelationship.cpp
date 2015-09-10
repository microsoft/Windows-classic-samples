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
    LPWSTR                        pwszServer           = NULL;         // Server IP Address, a value of NULL signifies the current server (where program is executed)
    LPWSTR                        pwszRelationshipName = L"test";      // Relationship name
    LPDHCP_FAILOVER_RELATIONSHIP  pRelationship        = NULL;         // Relationship structure
    DWORD                         dwError              = ERROR_SUCCESS;// Variable to hold error code
    DWORD                         dwFlags              = 0;            // Flags to reflect which all fields are getting modified

    dwError = DhcpV4FailoverGetRelationship(
        pwszServer,           // Server IP Address
        pwszRelationshipName, // Relationship name
        &pRelationship        // Relationship structure (LPDHCP_FAILOVER_RELATIONSHIP)
        );
    if( ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV4FailoverGetRelationship failed with Error = %d\n",dwError);
        goto cleanup;
    }

    pRelationship->Mode=HotStandby;  // Failover mode
    pRelationship->Mclt=0xe20;       // 3616 seconds
    pRelationship->SafePeriod=0x300; // 768 seconds
    pRelationship->Percentage=0x46;  //70 %
    
    dwFlags |= MODE | SAFEPERIOD | MCLT | PERCENTAGE; // mode, safeperiod, mclt and percentage field needs to be modified
    dwError = DhcpV4FailoverSetRelationship(
                        pwszServer,    // Server IP Address
                        dwFlags,       // Flags to reflect which all fields are getting modified
                        pRelationship  // Relationship structure from which values to be modified will be picked
                        );
    if( ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV4FailoverSetRelationship failed with Error = %d\n",dwError);
    }
    FreeRelationshipMemory(pRelationship);
cleanup:
    return 0;
}
