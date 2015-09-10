// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#include<windows.h>
#include<stdio.h>
#include<dhcpsapi.h>


int __cdecl main(void)
{
    DWORD           dwError     = ERROR_SUCCESS; // Variable to store the error code
    DWORD           dwNumAddr   = 2;             // specifies the number of IPv4 addresses retrieved from the specified scope in IPAddrList.
    DWORD           dwIndex     = 0;
    LPWSTR          pwszServer  = NULL;          // Server IP Address, Null signifies the current server (where the program is run)
    DHCP_IP_ADDRESS dwScope     = 0xa000000;     // Subnet address (10.0.0.0) 
    DHCP_IP_ADDRESS dwStartIp   = 0;             // 0 signifies scope's default start IP
    DHCP_IP_ADDRESS dwEndIp     = 0;             // 0 signifies scope's default End IP
    LPDHCP_IP_ARRAY pIpAddrList = NULL;

    // Retrieves the list of available IPv4 addresses that can be leased to clients
    dwError = DhcpV4GetFreeIPAddress(
                          pwszServer,    //ServerIpAddress, if this is NULL, it means the current server on which the program is run
                          dwScope,       // Subnet address (10.0.0.0) 
                          dwStartIp,     // Start IP Address after which free IP address needs to be retrieved
                          dwEndIp,       // End IP Address before which free IP address needs to be retrieved
                          dwNumAddr,     // Total number of free IP address requested
                          &pIpAddrList   // List of retrieved IP Addresses
                          );
    if(ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV4GetFreeIPAddress failed with Error = %d\n",dwError);
    }else
    {
        if(pIpAddrList)
        {
            for(dwIndex = 0; dwIndex < pIpAddrList->NumElements; dwIndex++)
            {
                wprintf(L"Free IpAddress = %u\n",pIpAddrList->Elements[dwIndex]);
            }
            DhcpRpcFreeMemory(pIpAddrList->Elements);
            DhcpRpcFreeMemory(pIpAddrList);
            pIpAddrList = NULL;
        }
    }
    return 0;
}
