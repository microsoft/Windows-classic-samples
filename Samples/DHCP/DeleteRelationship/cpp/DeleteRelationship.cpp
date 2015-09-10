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
    LPWSTR                       pwszServer           = NULL;
    LPWSTR                       pwszRelationshipName = L"test";
    LPDHCP_FAILOVER_RELATIONSHIP pRelationShip        = NULL;
    DWORD                        dwError              = ERROR_SUCCESS;

    dwError = DhcpV4FailoverGetRelationship(
                        pwszServer,
                        pwszRelationshipName,
                        &pRelationShip);
    if( ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV4FailoverGetRelationship failed with Error = %d\n",dwError);
        goto cleanup;
    }
    // Deactivate scopes on partner server
    dwError = DeactivateScopes(pRelationShip->SecondaryServerName, pRelationShip->pScopes);
    if( ERROR_SUCCESS != dwError)
    {
        wprintf(L"Failed to deactivate scopes on partner server\n");
        goto cleanup;
    }
    // Deactivate scopes on current server
    dwError = DeactivateScopes(pRelationShip->PrimaryServerName, pRelationShip->pScopes);
    if( ERROR_SUCCESS != dwError)
    {
        wprintf(L"Failed to deactivate scopes on primary server\n");
        goto cleanup;
    }

    //Delete relationship from partner server
    dwError = DhcpV4FailoverDeleteRelationship(
                        pRelationShip->SecondaryServerName,
                        pwszRelationshipName);
    if( ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV4FailoverDeleteRelationship failed with Error = %d\n",dwError);
        //Even if relationship on partner server is not getting deleted, we shouldnt stop deleting relationship on the current server
        // It might have happened that the tcp connection is down and partner server is not reachable.
        // That shouldnt restrict us from deleting the relationship on the current server.
    }

    //Delete relationship from primary server
    dwError = DhcpV4FailoverDeleteRelationship(
                        pwszServer,
                        pwszRelationshipName);
    if( ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV4FailoverDeleteRelationship failed with Error = %d\n",dwError);
    }
cleanup:
    FreeRelationshipMemory(pRelationShip);
    return 0;
}
