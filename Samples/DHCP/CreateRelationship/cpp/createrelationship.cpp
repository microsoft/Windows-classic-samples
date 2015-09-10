// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#include<windows.h>
#include<stdio.h>
#include<dhcpsapi.h>

/*
Assumptions:
    1) Primary Server IP: 10.0.0.1
    2) Name of the primary server: failover-test1
    3) Partner Server IP: 10.0.0.2
    4) Name of the partner server: failover-test2
    5) Identical scope should be present on both the server: (10.0.0.0)
*/

// This routine frees  LPDHCP_FAILOVER_RELATIONSHIP and its internal elements.
VOID FreeRelationshipMemory(LPDHCP_FAILOVER_RELATIONSHIP pFailRel)
{
    if (NULL != pFailRel) 
    {
        //Frees pScopes
        if (NULL != pFailRel->pScopes)
        {
            //Frees  individual elements of pScopes
            if(NULL != pFailRel->pScopes->Elements)
            {
                HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, pFailRel->pScopes->Elements);
            }
            HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, pFailRel->pScopes);
        }
        //Frees the relationship
        HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, pFailRel);
        pFailRel=NULL;
    }
}

//This routine frees LPDHCP_SUBNET_INFO_VQ
void FreeSubnetInfoVQ(LPDHCP_SUBNET_INFO_VQ pSubnetInfoVQ)
{
    // Frees NetBios Name
    if(pSubnetInfoVQ->PrimaryHost.NetBiosName) 
        DhcpRpcFreeMemory(pSubnetInfoVQ->PrimaryHost.NetBiosName);

    //Frees host name
    if(pSubnetInfoVQ->PrimaryHost.HostName) 
        DhcpRpcFreeMemory(pSubnetInfoVQ->PrimaryHost.HostName);

    // Frees Subnet Name
    if(pSubnetInfoVQ->SubnetName) 
        DhcpRpcFreeMemory(pSubnetInfoVQ->SubnetName);    

    // Frees subnet comment
    if(pSubnetInfoVQ->SubnetComment) 
        DhcpRpcFreeMemory(pSubnetInfoVQ->SubnetComment);

    DhcpRpcFreeMemory(pSubnetInfoVQ);
}

// This function changes the states of the scope.
DWORD ChangeScopeStates(_In_ LPWSTR pServer, _Inout_ LPDHCP_IP_ARRAY pArray, DHCP_SUBNET_STATE dwState)
{
    DWORD                 dwError       = ERROR_SUCCESS;
    DWORD                 dwIndex       = 0;
    LPDHCP_SUBNET_INFO_VQ pSubnetInfoVQ = NULL;
    for(dwIndex = 0 ; dwIndex < pArray->NumElements ; dwIndex++)
    {
        dwError = DhcpGetSubnetInfoVQ(pServer,pArray->Elements[dwIndex],&pSubnetInfoVQ);
        if(ERROR_SUCCESS != dwError)
            return dwError;
        
        // set the scope state to dwState
        pSubnetInfoVQ->SubnetState= dwState;
        dwError = DhcpSetSubnetInfoVQ(pServer,pArray->Elements[dwIndex],pSubnetInfoVQ);
        if(dwError != ERROR_SUCCESS)
        {
            return dwError;
        }
        // Free memory for pSubnetInfoVQ
        FreeSubnetInfoVQ(pSubnetInfoVQ);
        pSubnetInfoVQ = NULL;
    }
    return dwError;
}

// This function changes the scope state to "disabled"
DWORD DeactivateScopes(_In_ LPWSTR pServer, _Inout_ LPDHCP_IP_ARRAY pArray)
{
    return ChangeScopeStates(pServer,pArray,DhcpSubnetDisabled);
}

// This function changes the scope state to "enabled"
DWORD ActivateScopes(_In_ LPWSTR pServer,_Inout_ LPDHCP_IP_ARRAY pArray)
{
    return ChangeScopeStates(pServer,pArray,DhcpSubnetEnabled);
}

int __cdecl main(void)
{
    LPWSTR                        pwszServer           = NULL;          // Server IP Address
    LPDHCP_FAILOVER_RELATIONSHIP  pRelationship        = NULL;          // Relationship structure on the server
    LPDHCP_FAILOVER_RELATIONSHIP  pPartnerRelationship = NULL;          // Relationship structure on partner
    DWORD                         dwError              = ERROR_SUCCESS; // Variable to hold error codes

    pRelationship = (LPDHCP_FAILOVER_RELATIONSHIP)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(DHCP_FAILOVER_RELATIONSHIP));
    if(pRelationship == NULL)
    {
        wprintf(L"HeapAlloc failed. Not enough memory\n");
        return 0;
    }
    pRelationship->PrimaryServer = 0xa000001;            // Primary server IP Address (10.0.0.1)
    pRelationship->SecondaryServer = 0xa000002;          // Secondary server IP Address (10.0.0.2)
    pRelationship->Mode=LoadBalance;                     //Mode of the relationship. Other values can be HotStandby
    pRelationship->ServerType = PrimaryServer;           // Type of the server in the relationship (Primary or secondary)
    pRelationship->State= NO_STATE;                      // State of the relationship
    pRelationship->PrevState= NO_STATE;                  // Previous state
    pRelationship->Mclt=0xe10;                           // Maximum Client lead time => 1 hours (0xe10 sec)
    pRelationship->SafePeriod=0x258;                     // Safe period 10 min (0x258 sec)
    pRelationship->SharedSecret=L"secretpassword";       // Shared secret
    pRelationship->Percentage=0x32;                      // Percentage (50 %)
    pRelationship->PrimaryServerName=L"failover-test1";   // Primary server name
    pRelationship->SecondaryServerName=L"failover-test2"; //Secondary server name
    pRelationship->RelationshipName=L"test";              // Relationship name

    //Creates IP Array for storing scopes
    pRelationship->pScopes = (LPDHCP_IP_ARRAY)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(DHCP_IP_ARRAY));
    if(pRelationship->pScopes == NULL)
    {
        wprintf(L"HeapAlloc failed. Not enough memory\n");
        goto cleanup;
    }
    
    // Number of scopes in the relationship = 1
    pRelationship->pScopes->NumElements = 1;
    pRelationship->pScopes->Elements=(LPDHCP_IP_ADDRESS)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY, sizeof(DHCP_IP_ADDRESS));
    if(pRelationship->pScopes->Elements == NULL)
    {
        wprintf(L"HeapAlloc failed. Not enough memory\n");
        goto cleanup;
    }
    //Scope is 10.0.0.0
    pRelationship->pScopes->Elements[0]=0xa000000; // 10.0.0.0

    pPartnerRelationship = (LPDHCP_FAILOVER_RELATIONSHIP)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(DHCP_FAILOVER_RELATIONSHIP));
    if(pPartnerRelationship == NULL)
    {
        wprintf(L"HeapAlloc failed. Not enough memory\n");
        goto cleanup;
    }
    pPartnerRelationship->PrimaryServer = 0xa000001;             // Primary server IP Address (10.0.0.1)
    pPartnerRelationship->SecondaryServer = 0xa000002;           // Secondary server IP Address (10.0.0.2)
    pPartnerRelationship->Mode=LoadBalance;                      //Mode of the relationship. Other values can be HotStandby
    pPartnerRelationship->ServerType = SecondaryServer;          // Type of the server in the relationship (Primary or secondary)
    pPartnerRelationship->State= NO_STATE;                       // State of the relationship
    pPartnerRelationship->PrevState= NO_STATE;                   // Previous state
    pPartnerRelationship->Mclt=0xe10;                            // Maximum Client lead time => 1 hours (0xe10 sec)
    pPartnerRelationship->SafePeriod=0x258;                      // Safe period 10 min (0x258 sec)
    pPartnerRelationship->SharedSecret=L"secretpassword";        // Shared secret
    pPartnerRelationship->Percentage=0x32;                       // Percentage (50 %)
    pPartnerRelationship->PrimaryServerName=L"failover-test1";   // Primary server name
    pPartnerRelationship->SecondaryServerName=L"failover-test2"; //Secondary server name
    pPartnerRelationship->RelationshipName=L"test";              // Relationship name

    //Creates IP Array for storing scopes
    pPartnerRelationship->pScopes = (LPDHCP_IP_ARRAY)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(DHCP_IP_ARRAY));
    if(pPartnerRelationship->pScopes == NULL)
    {
        wprintf(L"HeapAlloc failed. Not enough memory\n");
        goto cleanup;
    }
    // Number of scopes in the relationship = 1
    pPartnerRelationship->pScopes->NumElements = 1;
    pPartnerRelationship->pScopes->Elements=(LPDHCP_IP_ADDRESS)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY, sizeof(DHCP_IP_ADDRESS));
    if(pPartnerRelationship->pScopes->Elements == NULL)
    {
        wprintf(L"HeapAlloc failed. Not enough memory\n");
        goto cleanup;
    }
    //Scope is 10.0.0.0
    pPartnerRelationship->pScopes->Elements[0]=0xa000000; //10.0.0.0

    // Deactivate scopes on partner server
    dwError = DeactivateScopes(pRelationship->SecondaryServerName, pRelationship->pScopes);
    if( ERROR_SUCCESS != dwError)
    {        wprintf(L"Failed to deactivate scopes on partner server\n");
        goto cleanup;
    }
    // Deactivate scopes on current server
    dwError = DeactivateScopes(pRelationship->PrimaryServerName, pRelationship->pScopes);
    if( ERROR_SUCCESS != dwError)
    {        wprintf(L"Failed to deactivate scopes on primary server\n");
        goto ActivateSecAndCleanup;
    }
    //Creates the relationship
    dwError = DhcpV4FailoverCreateRelationship(
                        pPartnerRelationship->SecondaryServerName,
                        pPartnerRelationship);
    if( ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV4FailoverCreateRelationship failed with Error = %d\n",dwError);
        goto ActivatePriAndCleanup;
    }
    dwError = DhcpV4FailoverCreateRelationship(
                        pwszServer,
                        pRelationship);
    if( ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV4FailoverCreateRelationship failed with Error = %d\n",dwError);
        DhcpV4FailoverDeleteRelationship(pPartnerRelationship->SecondaryServerName, pPartnerRelationship->RelationshipName);
    }
ActivatePriAndCleanup:
    //Activate scopes on primary server
    ActivateScopes(pRelationship->PrimaryServerName,pRelationship->pScopes);

ActivateSecAndCleanup:
    //Activate scopes on secondary server
    ActivateScopes(pRelationship->SecondaryServerName,pRelationship->pScopes);

cleanup:
    FreeRelationshipMemory(pRelationship);
    FreeRelationshipMemory(pPartnerRelationship);
    return 0;
}
