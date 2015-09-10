// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#include<windows.h>
#include<stdio.h>
#include<dhcpsapi.h>

#define DHCP_DATE_TIME_RANDOM_HIGH_VALUE     0x1ccba28
#define DHCP_DATE_TIME_RANDOM_LOW_VALUE      0x3f4115f0
#define DHCP_SAMPLE_IAID                     0x1400155d
#define SAMPLE_HOST_NAME                     L"TEST-HOSTNAME"
#define SAMPLE_COMMENT                       L"TEST-COMMENT"

int __cdecl main(void)
{
    DWORD               dwError        = ERROR_SUCCESS;         // Variable to store the error code
    DHCP_IPV6_ADDRESS   dwIpAddress    = {0};
    LPWSTR              pwszServer     = NULL;                  // Server IP Address, Null signifies the current server (where the program is run)
    DHCP_CLIENT_INFO_V6 clientInfo     = {0};                   // Client Info to be created
    LPWSTR              pwszComment    = SAMPLE_COMMENT;        // Client comment
    DATE_TIME           leaseExpTime   = {0};                   // Lease expiry time
    char*               hwAddress      = "12121212";
    size_t              dwHwAddrLen    = strlen(hwAddress);

    // Create LeaseTime
    // The assigned value mean that lease wont expire (Reservation)
    leaseExpTime.dwHighDateTime = DHCP_DATE_TIME_RANDOM_HIGH_VALUE;
    leaseExpTime.dwLowDateTime  = DHCP_DATE_TIME_RANDOM_LOW_VALUE;

    dwIpAddress.HighOrderBits = 0xfc00000000000000;
    dwIpAddress.LowOrderBits = 0xa;

    // Fill clientInfo
    clientInfo.ClientIpAddress         = dwIpAddress;
    clientInfo.ClientName              = SAMPLE_HOST_NAME;
    clientInfo.ClientComment           = pwszComment;
    clientInfo.ClientValidLeaseExpires = leaseExpTime;
    clientInfo.ClientPrefLeaseExpires  = leaseExpTime;
    clientInfo.AddressType             = 0; // IANA
    clientInfo.IAID                    = DHCP_SAMPLE_IAID;
    clientInfo.OwnerHost.HostName      = SAMPLE_HOST_NAME;
    clientInfo.OwnerHost.NetBiosName   = SAMPLE_HOST_NAME;
    clientInfo.OwnerHost.IpAddress     = dwIpAddress;

    clientInfo.ClientDUID.DataLength = (DWORD)dwHwAddrLen;
    clientInfo.ClientDUID.Data=(BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwHwAddrLen);
    if(clientInfo.ClientDUID.Data == NULL)
        goto Cleanup;
    memcpy_s(clientInfo.ClientDUID.Data, dwHwAddrLen, hwAddress, dwHwAddrLen);

    //Creates a DHCPv4 client lease record in the DHCP server database
    dwError = DhcpV6CreateClientInfo(
                          pwszServer,    //ServerIpAddress, if this is NULL, it means the current server on which the program is run
                          &clientInfo
                          );
    if(ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV6CreateClientInfo failed with Error = %d\n",dwError);
    }else
    {
        wprintf(L"DhcpV6CreateClientInfo returned success\n");
    }
    HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, clientInfo.ClientDUID.Data);
Cleanup:
    return 0;
}

