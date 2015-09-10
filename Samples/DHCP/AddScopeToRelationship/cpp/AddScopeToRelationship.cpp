// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

/*
Assumptions:
    1) There exists a relationship with the name "Test".
    2) Identical scope 172.0.0.0 should be present on primary as well as partner server, which should not be part of relationship.
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

        //Frees the relationship
        DhcpRpcFreeMemory(pFailRel);
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
DWORD ActivateScopes(_In_ LPWSTR pServer, _Inout_ LPDHCP_IP_ARRAY pArray)
{
    return ChangeScopeStates(pServer,pArray,DhcpSubnetEnabled);
}

int __cdecl main(void)
{
    LPWSTR                        pwszServer           = NULL;         // Server IP Address
    LPWSTR                        pwszRelationshipName = L"test";      // Relationship name
    LPDHCP_FAILOVER_RELATIONSHIP  pRelationship        = NULL;         // Relationship structure
    LPDHCP_IP_ARRAY               pIpAddressArray      = NULL;         // Array of IP Addresses
    DWORD                         dwError              = ERROR_SUCCESS;// Variable to hold error codes

    // Gets a relationship with the name stored in the variable "pwszRelationshipName"
    dwError = DhcpV4FailoverGetRelationship(
        pwszServer,           // Server IP Address
        pwszRelationshipName, // Relationship name
        &pRelationship        // Failover relationship
        );
    if(ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV4FailoverGetRelationship failed with Error = %d\n", dwError);
        goto cleanup;
    }
    // Create an array of IP address which will have the scopes to be added to the relationship
    pIpAddressArray = (LPDHCP_IP_ARRAY)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(DHCP_IP_ARRAY));
    if(pIpAddressArray == NULL)
    {
        wprintf(L"HeapAlloc failed !! Not enough memory\n");
        goto cleanup;
    }
    // Number of scopes to be added is 1
    pIpAddressArray->NumElements = 1;
    pIpAddressArray->Elements=(LPDHCP_IP_ADDRESS)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY, sizeof(DHCP_IP_ADDRESS));
    if(pIpAddressArray->Elements == NULL)
    {
        wprintf(L"HeapAlloc failed. Not enough memory\n");
        goto cleanup;
    }
    // Fill the elements in the array
    pIpAddressArray->Elements[0]=0xac100000; //172.16.0.0
    if (NULL != pRelationship->pScopes)
    {
        if(NULL != pRelationship->pScopes->Elements)
        {
           DhcpRpcFreeMemory(pRelationship->pScopes->Elements);
        }
        DhcpRpcFreeMemory(pRelationship->pScopes);
    }
    pRelationship->pScopes = pIpAddressArray;

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

    //Adds the scope to the relationship on the secondary server
    dwError = DhcpV4FailoverAddScopeToRelationship(
                        pRelationship->SecondaryServerName,
                        pRelationship);
    if( ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV4FailoverAddScopeToRelationship failed with Error = %d for partner server\n",dwError);
        goto ActivatePriAndCleanup;
    }

    //Adds the scope to the relationship on the primary server
    dwError = DhcpV4FailoverAddScopeToRelationship(
                        pwszServer,
                        pRelationship);
    if( ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV4FailoverAddScopeToRelationship failed with Error = %d for primary server\n",dwError);
    }

ActivatePriAndCleanup:
    //Activate scopes on primary server
    ActivateScopes(pRelationship->PrimaryServerName,pRelationship->pScopes);

ActivateSecAndCleanup:
    //Activate scopes on secondary server
    ActivateScopes(pRelationship->SecondaryServerName,pRelationship->pScopes);

cleanup:
    
    if( NULL != pIpAddressArray )
    {
        if( NULL != pIpAddressArray->Elements)
        {
            HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, pIpAddressArray->Elements);
            pIpAddressArray->Elements = NULL;
        }
        HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, pIpAddressArray);
        pIpAddressArray = NULL;
    }
    FreeRelationshipMemory(pRelationship);
    return 0;
}
