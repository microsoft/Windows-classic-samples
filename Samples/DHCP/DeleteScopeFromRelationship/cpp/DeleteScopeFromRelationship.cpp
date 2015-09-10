// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#include<windows.h>
#include<stdio.h>
#include<dhcpsapi.h>

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

DWORD DeactivateScopes(_In_ LPWSTR pServer, _Inout_ LPDHCP_IP_ARRAY pArray)
{
    DWORD                 dwError       = ERROR_SUCCESS;
    DWORD                 dwIndex       = 0;
    LPDHCP_SUBNET_INFO_VQ pSubnetInfoVQ = NULL;
    for(dwIndex = 0 ; dwIndex < pArray->NumElements ; dwIndex++)
    {
        dwError = DhcpGetSubnetInfoVQ(pServer,pArray->Elements[dwIndex],&pSubnetInfoVQ);
        if(ERROR_SUCCESS != dwError)
            return dwError;
        
        // set the scope to deactivated
        pSubnetInfoVQ->SubnetState= DhcpSubnetDisabled;
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

int __cdecl main(void)
{
    LPWSTR                        pwszServer           = NULL;          //Server IP Address
    LPWSTR                        pwszRelationshipName = L"test";       // Name of the relationship
    LPDHCP_FAILOVER_RELATIONSHIP  pRelationship        = NULL;          // Failover relationship
    LPDHCP_IP_ARRAY               pIpAddressArray      = NULL;          // IP Address array having the scopes
    DWORD                         dwError              = ERROR_SUCCESS; // Variable to hold the error code
    dwError = DhcpV4FailoverGetRelationship(pwszServer,pwszRelationshipName,&pRelationship);
    if(ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV4FailoverGetRelationship failed with Error = %d\n", dwError);
        goto cleanup;
    }

    pIpAddressArray = (LPDHCP_IP_ARRAY)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(DHCP_IP_ARRAY));
    if(pIpAddressArray == NULL)
    {
        wprintf(L"HeapAlloc failed !! Not enough memory\n");
        goto cleanup;
    }
    pIpAddressArray->NumElements = 1;
    pIpAddressArray->Elements=(LPDHCP_IP_ADDRESS)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY, sizeof(DHCP_IP_ADDRESS));
    if(pIpAddressArray->Elements == NULL)
    {
        wprintf(L"HeapAlloc failed. Not enough memory\n");
        goto cleanup;
    }
    pIpAddressArray->Elements[0]=0xac100000;

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
        goto cleanup;
    }
    
    dwError = DhcpV4FailoverDeleteScopeFromRelationship(
                        pRelationship->SecondaryServerName,
                        pRelationship);
    if( ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV4FailoverDeleteScopeFromRelationship failed with Error = %d\n for partner server",dwError);
        goto cleanup;
    }

    dwError = DhcpV4FailoverDeleteScopeFromRelationship(
                        pwszServer,
                        pRelationship);
    if( ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV4FailoverDeleteScopeFromRelationship failed with Error = %d for primary server\n",dwError);
    }
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
