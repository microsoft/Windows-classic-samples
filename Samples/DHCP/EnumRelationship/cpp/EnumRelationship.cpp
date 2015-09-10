// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#include<windows.h>
#include<stdio.h>
#include<intsafe.h>
#include<dhcpsapi.h>

void FreeIPArray(LPDHCP_IP_ARRAY Array) 
{
    if(Array)
    {
        DhcpRpcFreeMemory(Array->Elements);

    }
    DhcpRpcFreeMemory(Array);
    Array = NULL;

}

void
FreeDhcpFailoverRelationship(
    DHCP_FAILOVER_RELATIONSHIP FailoverRelationship)
{
    DhcpRpcFreeMemory(FailoverRelationship.RelationshipName);
    DhcpRpcFreeMemory(FailoverRelationship.PrimaryServerName);
    DhcpRpcFreeMemory(FailoverRelationship.SecondaryServerName);
    FreeIPArray(FailoverRelationship.pScopes);
    DhcpRpcFreeMemory(FailoverRelationship.SharedSecret);
}

void
FreeDhcpFailoverRelationship(
    LPDHCP_FAILOVER_RELATIONSHIP pFailoverInfo)
{
    if (NULL == pFailoverInfo)
        return;

    FreeDhcpFailoverRelationship(*pFailoverInfo);

    DhcpRpcFreeMemory(pFailoverInfo);
    pFailoverInfo = NULL;
}


void
FreeDhcpFailoverRelationshipArray(
    LPDHCP_FAILOVER_RELATIONSHIP_ARRAY FailoverEnumInfo)
{
    if (NULL == FailoverEnumInfo)
        return;

    for (DWORD i = 0; i < FailoverEnumInfo->NumElements; i++)
    {
        FreeDhcpFailoverRelationship(FailoverEnumInfo->pRelationships[i]);
    }

    DhcpRpcFreeMemory(FailoverEnumInfo->pRelationships);
    DhcpRpcFreeMemory(FailoverEnumInfo);
    FailoverEnumInfo = NULL;
}

int __cdecl main(void)
{
    DWORD                               dwError          = ERROR_MORE_DATA;
    DWORD                               dwElementsRead   = 0;
    DWORD                               dwElementsTotal  = 0;
    DWORD                               preferredMax     = DWORD_MAX;
    LPWSTR                              pwszServer       = NULL;
    LPDHCP_FAILOVER_RELATIONSHIP_ARRAY  enumRelationInfo = NULL;
    DWORD                               resumeHandle     = 0;
    DWORD                               dwIdx            = 0;
    for(;;)
    {
        dwError = DhcpV4FailoverEnumRelationship(
                    pwszServer,
                    &resumeHandle,
                    preferredMax,
                    &enumRelationInfo,
                    &dwElementsRead,
                    &dwElementsTotal);

        if ((NO_ERROR != dwError) &&
            (ERROR_MORE_DATA != dwError) &&
            (ERROR_NO_MORE_ITEMS != dwError ))
        {
            wprintf(L"Error in enumerating relationships. Error = %d\n",dwError);
            break;
        }
        if (dwElementsRead && enumRelationInfo)
        {
            //operate on enumPoliciesInfo, for example in the below snippet, we are printing the names of the 
            // enumerated relationships.
            for ( dwIdx = 0; dwIdx < enumRelationInfo->NumElements; dwIdx++ )
            {
                wprintf(L"Relationship name = %s\n",enumRelationInfo->pRelationships[dwIdx].RelationshipName);
            }
            FreeDhcpFailoverRelationshipArray(enumRelationInfo);
            enumRelationInfo = NULL;
        }
        if (ERROR_NO_MORE_ITEMS == dwError || (dwError == ERROR_SUCCESS && dwElementsTotal == 0))
        {
            dwError=ERROR_SUCCESS;
            break;
        }
        dwElementsRead = 0;
        dwElementsTotal = 0;
    }

    return 0;
}
