// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#include<windows.h>
#include<stdio.h>
#include<dhcpsapi.h>

VOID FreeClientInfoMemory(LPDHCP_CLIENT_INFO_PB pClientInfo)
{
    if (NULL != pClientInfo) 
    {
        // Frees client name
        if( NULL != pClientInfo->ClientName)
            DhcpRpcFreeMemory(pClientInfo->ClientName);

        //Frees policy name
        if (NULL != pClientInfo->PolicyName)
            DhcpRpcFreeMemory(pClientInfo->PolicyName);

        //Frees client comments
        if (NULL != pClientInfo->ClientComment)
            DhcpRpcFreeMemory(pClientInfo->ClientComment);

        //Frees the ClientHardwareAddress
        if(NULL != pClientInfo->ClientHardwareAddress.Data && pClientInfo->ClientHardwareAddress.DataLength > 0)
            DhcpRpcFreeMemory(pClientInfo->ClientHardwareAddress.Data);

        //Frees the HostName
        if(NULL != pClientInfo->OwnerHost.HostName)
            DhcpRpcFreeMemory(pClientInfo->OwnerHost.HostName);

        //Frees the NetBiosName
        if(NULL != pClientInfo->OwnerHost.NetBiosName)
            DhcpRpcFreeMemory(pClientInfo->OwnerHost.NetBiosName);

        // Frees the clientInfo
        DhcpRpcFreeMemory(pClientInfo);
        pClientInfo=NULL;
    }
}

int __cdecl main(void)
{
    DWORD                 dwError        = ERROR_SUCCESS;         // Variable to store the error code
    DWORD                 dwIpAddress    = 0xa00000a;             // 10.0.0.10
    LPWSTR                pwszServer     = NULL;                  // Server IP Address, Null signifies the current server (where the program is run)
    LPDHCP_CLIENT_INFO_PB pClientInfo    = NULL;                  // Client Info to be created
    DHCP_SEARCH_INFO      searchInfo;

    searchInfo.SearchType = DhcpClientIpAddress;
    searchInfo.SearchInfo.ClientIpAddress = dwIpAddress;

    //Creates a DHCPv4 client lease record in the DHCP server database
    dwError = DhcpV4GetClientInfo(
                          pwszServer,    //ServerIpAddress, if this is NULL, it means the current server on which the program is run
                          &searchInfo,
                          &pClientInfo
                          );
    if(ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV4GetClientInfo failed with Error = %d\n",dwError);
    }else
    {
        wprintf(L"Client Name = %s\n",pClientInfo->ClientName);
        wprintf(L"Client Comment = %s\n",pClientInfo->ClientComment);
        FreeClientInfoMemory(pClientInfo);
    }
    return 0;
}
