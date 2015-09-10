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


// This routine frees  LPDHCP_CLIENT_INFO_PB and its internal elements.
VOID FreeDhcpEnumSubnetClientMemory(LPDHCP_CLIENT_INFO_PB pEnumClientInfo)
{
    if( NULL != pEnumClientInfo)
    {

        if(pEnumClientInfo->ClientHardwareAddress.Data)
        {
            DhcpRpcFreeMemory(pEnumClientInfo->ClientHardwareAddress.Data);
            pEnumClientInfo->ClientHardwareAddress.Data = NULL;
        }
        if(pEnumClientInfo->ClientName)
        {
            DhcpRpcFreeMemory(pEnumClientInfo->ClientName);
            pEnumClientInfo->ClientName = NULL;
        }
        if(pEnumClientInfo->ClientComment)
        {
            DhcpRpcFreeMemory(pEnumClientInfo->ClientComment);
            pEnumClientInfo->ClientComment = NULL;
        }
        if(pEnumClientInfo->OwnerHost.HostName)
        {
            DhcpRpcFreeMemory(pEnumClientInfo->OwnerHost.HostName);
            pEnumClientInfo->OwnerHost.HostName = NULL;
        }
        if(pEnumClientInfo->OwnerHost.NetBiosName)
        {
            DhcpRpcFreeMemory(pEnumClientInfo->OwnerHost.NetBiosName);
            pEnumClientInfo->OwnerHost.NetBiosName = NULL;
        }
        if(pEnumClientInfo->PolicyName)
        {
            DhcpRpcFreeMemory(pEnumClientInfo->PolicyName);
            pEnumClientInfo->PolicyName = NULL;
        }
        DhcpRpcFreeMemory(pEnumClientInfo);
    }
    pEnumClientInfo = NULL;
}

// This routine frees  LPDHCP_CLIENT_INFO_PB_ARRAY and its internal elements.
VOID FreeDhcpEnumSubnetClientInfo(LPDHCP_CLIENT_INFO_PB_ARRAY pEnumClientArray)
{
    if(NULL != pEnumClientArray)
    {
        for(DWORD dwIndex=0; dwIndex < pEnumClientArray->NumElements; dwIndex++)
        {
            FreeDhcpEnumSubnetClientMemory(pEnumClientArray->Clients[dwIndex]);
        }
        DhcpRpcFreeMemory(pEnumClientArray);
        pEnumClientArray = NULL;
    }
}

int __cdecl main(void)
{
    DWORD                        dwError          = ERROR_SUCCESS;   // Variable to hold error code
    DWORD                        dwElementsRead   = 0;               // Variable to hold number of elements read
    DWORD                        dwSubnetAddress  = 0xa000000;       // Variable to hold subnet address
    DWORD                        dwElementsTotal  = 0;               // Variable to hold total number of elements
    DWORD                        preferredMax     = DWORD_MAX;       // Variable to hold Preferred maximum value to be fetched in one call of DhcpV4EnumSubnetClients
    LPWSTR                       pwszServer       = NULL;            // Variable to hold Server IP Address
    LPDHCP_CLIENT_INFO_PB_ARRAY  pEnumClientInfo  = NULL;            // Variable to hold subnet clients fetched using DhcpV4EnumSubnetClients
    DWORD                        resumeHandle     = 0;               // variable to hold resume handle
    DWORD                        dwIdx            = 0;

    for(;;)
    {
        dwError = DhcpV4EnumSubnetClients(
                    pwszServer,        // Server IP Address, a value of NULL reflects the current server (where the program is executed)
                    dwSubnetAddress,   // Subnet Address
                    &resumeHandle,     // Resume Handle
                    preferredMax,      // Preferred maximum
                    &pEnumClientInfo,  // Pointer to a DHCP_CLIENT_INFO_PB_ARRAY, contains the DHCP client lease records set available for the specified subnet.
                    &dwElementsRead,   // Total number of elements read
                    &dwElementsTotal   // Total number of elements
                    );
        if ((ERROR_SUCCESS != dwError) &&
            (ERROR_MORE_DATA != dwError) &&
            (ERROR_NO_MORE_ITEMS != dwError ))
        {
            wprintf(L"Error in enumerating policies. Error = %d\n",dwError);
            break;
        }
        if (dwElementsRead && pEnumClientInfo)
        {
            //operate on pEnumClientInfo
            for ( dwIdx = 0; dwIdx < pEnumClientInfo->NumElements; dwIdx++ )
            {
                wprintf(L"ClientIpAddress = %u \n",pEnumClientInfo->Clients[dwIdx]->ClientIpAddress);
                wprintf(L"PolicyName = %s \n",pEnumClientInfo->Clients[dwIdx]->PolicyName);
            }
            //one needs to free the pEnumClientInfo once used
            FreeDhcpEnumSubnetClientInfo(pEnumClientInfo);
            pEnumClientInfo = NULL;
        }
        if (dwError == ERROR_SUCCESS || dwError == ERROR_NO_MORE_ITEMS)
        {
            break;
        }
        dwElementsRead = 0;
        dwElementsTotal = 0;
    }
    return 0;
}
