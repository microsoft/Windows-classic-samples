// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#include<windows.h>
#include<stdio.h>
#include<dhcpsapi.h>

// This routine frees  LPDHCPV4_FAILOVER_CLIENT_INFO and its internal elements.
VOID FreeClientInfoMemory(LPDHCPV4_FAILOVER_CLIENT_INFO pClientInfo)
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
    DHCP_SEARCH_INFO              *pSearchInfo = NULL;         //SearchInfo
    LPDHCPV4_FAILOVER_CLIENT_INFO  pClientInfo = NULL;         // ClientInfo
    DWORD                          dwError    = ERROR_SUCCESS; // Variable that holds the error code
    LPWSTR                         pwszServer = NULL;          //Server IP Address

    pSearchInfo = (DHCP_SEARCH_INFO *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,sizeof(DHCP_SEARCH_INFO));
    if(pSearchInfo == NULL)
    {
        wprintf(L"HeapAlloc failed. Not Enough memory\n");
        goto cleanup;
    }
    pSearchInfo->SearchType = DhcpClientIpAddress;
    pSearchInfo->SearchInfo.ClientIpAddress=0xa000033; // Client with IP Address 10.0.0.51
    dwError = DhcpV4FailoverGetClientInfo(
                    pwszServer,  // Server IP Address
                    pSearchInfo, // Search Info on the basis of which clients will be fetched
                    &pClientInfo // ClientInfo obtained from the server
                    );
    if(ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV4FailoverGetClientInfo failed with Error = %d\n", dwError);
    }
    HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, pSearchInfo);
    pSearchInfo = NULL;
    FreeClientInfoMemory(pClientInfo);
    pClientInfo = NULL;
cleanup:
    return 0;
}
