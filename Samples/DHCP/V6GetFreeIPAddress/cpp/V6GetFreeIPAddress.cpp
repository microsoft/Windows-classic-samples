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
    DWORD             dwError     = ERROR_SUCCESS; // Variable to store the error code
    DWORD             dwNumAddr   = 4;             // specifies the number of IPv6 addresses retrieved from the specified scope in IPAddrList.
    DWORD             dwIndex     = 0;
    LPWSTR            pwszServer  = NULL;          // Server IP Address, Null signifies the current server (where the program is run)
    DHCP_IPV6_ADDRESS dwScope     = {0};           // Subnet address
    DHCP_IPV6_ADDRESS dwStartIp   = {0};           // 0 signifies scope's default start IP
    DHCP_IPV6_ADDRESS dwEndIp     = {0};           // 0 signifies scope's default End IP
    LPDHCPV6_IP_ARRAY pIpAddrList = NULL;

    //Scope with prefix 2001:db8::
    dwScope.HighOrderBits = 0xfc00000000000000;
    dwScope.LowOrderBits = 0;
    
    // Retrieves the list of available IPv6 addresses that can be leased to clients
    dwError = DhcpV6GetFreeIPAddress(
                            pwszServer,    //ServerIpAddress, if this is NULL, it means the current server on which the program is run
                            dwScope,       // Subnet address
                            dwStartIp,     // Start IP Address after which free IP address needs to be retrieved
                            dwEndIp,       // End IP Address before which free IP address needs to be retrieved
                            dwNumAddr,     // Total number of free IP address requested
                            &pIpAddrList   // List of retrieved IP Addresses
                          );
    if(ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV6GetFreeIPAddress failed with Error = %d\n",dwError);
    }else
    {
        for(dwIndex = 0; dwIndex < dwNumAddr; dwIndex++)
        {
            wprintf(L"Free IpAddress HighOrderBits = %llu LowOrderBits = %llu\n",pIpAddrList->Elements[dwIndex].HighOrderBits, pIpAddrList->Elements[dwIndex].LowOrderBits);
        }
        if(pIpAddrList)
        {
            if( NULL != pIpAddrList->Elements)
            {
                DhcpRpcFreeMemory(pIpAddrList->Elements);
            }
            DhcpRpcFreeMemory(pIpAddrList);
        }
        pIpAddrList = NULL;
    }    
    return 0;
}
