// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#include<windows.h>
#include<stdio.h>
#include<dhcpsapi.h>

#define DHCP_DATE_TIME_INFINIT_HIGH     0x7FFFFFFF  
#define DHCP_DATE_TIME_INFINIT_LOW      0xFFFFFFFF 
#define ADDRESS_STATE_ACTIVE            1
#define SAMPLE_HOST_NAME                L"TEST-HOSTNAME"
#define SAMPLE_COMMENT                  L"TEST-COMMENT"

int __cdecl main(void)
{
    DWORD               dwError        = ERROR_SUCCESS;         // Variable to store the error code
    DWORD               dwIpAddress    = 0xa00000a;             // 10.0.0.10
    DWORD               dwSubnetMask   = 0xa000000;             // 10.0.0.0
    LPWSTR              pwszServer     = NULL;                  // Server IP Address, Null signifies the current server (where the program is run)
    DHCP_CLIENT_INFO_PB clientInfo     = {0};                   // Client Info to be created
    LPWSTR              pwszComment    = SAMPLE_COMMENT;        // Client comment
    LPWSTR              pwszPolicyName = NULL            ;      // Policy Name
    DATE_TIME           probEndTime    = {0};                   // Probation end time
    DATE_TIME           leaseExpTime   = {0};                   // Lease expiry time
    BYTE                clientType     = CLIENT_TYPE_NONE;      // Client Type
    BYTE                addressState   = ADDRESS_STATE_ACTIVE;  // ADDRESS_STATE_ACTIVE == 1
    BOOL                bNAPCapable    = FALSE;                 // NAP Capable or not
    QuarantineStatus    bNAPStatus     = NOQUARANTINE;          // Quarantine status
    char*               hwAddress      = "121212";
    size_t              dwHwAddrLen    = strlen(hwAddress);

    // Create LeaseTime
    // The assigned value mean that lease wont expire (Reservation)
    leaseExpTime.dwHighDateTime = DHCP_DATE_TIME_INFINIT_HIGH;
    leaseExpTime.dwLowDateTime  = DHCP_DATE_TIME_INFINIT_LOW;

    // Fill clientInfo
    clientInfo.ClientIpAddress       = dwIpAddress;
    clientInfo.SubnetMask            = dwSubnetMask;
    clientInfo.ClientName            = SAMPLE_HOST_NAME;
    clientInfo.ClientComment         = pwszComment;
    clientInfo.ClientLeaseExpires    = leaseExpTime;
    clientInfo.bClientType           = clientType;
    clientInfo.Status                = bNAPStatus;
    clientInfo.ProbationEnds         = probEndTime;
    clientInfo.QuarantineCapable     = bNAPCapable;
    clientInfo.FilterStatus          = 0;
    clientInfo.PolicyName            = pwszPolicyName;
    clientInfo.AddressState          = addressState;
    clientInfo.OwnerHost.HostName    = SAMPLE_HOST_NAME;
    clientInfo.OwnerHost.NetBiosName = SAMPLE_HOST_NAME;
    clientInfo.OwnerHost.IpAddress   = dwIpAddress;

    clientInfo.ClientHardwareAddress.DataLength = (DWORD)dwHwAddrLen;
    clientInfo.ClientHardwareAddress.Data=(BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwHwAddrLen);
    if(clientInfo.ClientHardwareAddress.Data == NULL)
        goto Cleanup;
    memcpy_s(clientInfo.ClientHardwareAddress.Data,dwHwAddrLen, hwAddress, dwHwAddrLen);

    //Creates a DHCPv4 client lease record in the DHCP server database
    dwError = DhcpV4CreateClientInfo(
                          pwszServer,    //ServerIpAddress, if this is NULL, it means the current server on which the program is run
                          &clientInfo
                          );
    if(ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV4CreateClientInfo failed with Error = %d\n",dwError);
    }else
    {
        wprintf(L"DhcpV4CreateClientInfo returned success\n");
    }
    HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, clientInfo.ClientHardwareAddress.Data);
Cleanup:
    return 0;
}
