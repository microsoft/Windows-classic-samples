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


// This routine frees  LPDHCP_IP_RESERVATION_INFO and its internal elements.
VOID FreeDhcpEnumSubnetReservationsMemory(LPDHCP_IP_RESERVATION_INFO pEnumReservationInfo)
{
    if( NULL != pEnumReservationInfo)
    {
        if(pEnumReservationInfo->ReservedForClient.Data)
        {
            DhcpRpcFreeMemory(pEnumReservationInfo->ReservedForClient.Data);
            pEnumReservationInfo->ReservedForClient.Data = NULL;
        }
        if(pEnumReservationInfo->ReservedClientName)
        {
            DhcpRpcFreeMemory(pEnumReservationInfo->ReservedClientName);
            pEnumReservationInfo->ReservedClientName= NULL;
        }
        if(pEnumReservationInfo->ReservedClientDesc)
        {
            DhcpRpcFreeMemory(pEnumReservationInfo->ReservedClientDesc);
            pEnumReservationInfo->ReservedClientDesc= NULL;
        }
        DhcpRpcFreeMemory(pEnumReservationInfo);
    }
    pEnumReservationInfo = NULL;
}

// This routine frees  LPDHCP_RESERVATION_INFO_ARRAY and its internal elements.
VOID FreeDhcpEnumSubnetReservationsInfo(LPDHCP_RESERVATION_INFO_ARRAY pEnumReservationInfoArray)
{
    if(NULL != pEnumReservationInfoArray)
    {
        for(DWORD dwIndex=0; dwIndex < pEnumReservationInfoArray->NumElements; dwIndex++)
        {
            FreeDhcpEnumSubnetReservationsMemory(pEnumReservationInfoArray->Elements[dwIndex]);
        }
        DhcpRpcFreeMemory(pEnumReservationInfoArray);
        pEnumReservationInfoArray = NULL;
    }
}

int __cdecl main(void)
{
    DWORD                          dwError               = ERROR_SUCCESS;   // Variable to hold error code
    DWORD                          dwElementsRead        = 0;               // Number of elements read
    DWORD                          dwSubnetAddress       = 0xa000000;       // Subnet Address 10.0.0.0
    DWORD                          dwElementsTotal       = 0;               // Variable to hold total number of elements
    DWORD                          preferredMax          = DWORD_MAX;       // Preferred maximum value to be used in DhcpV4EnumSubnetReservations
    LPWSTR                         pwszServer            = NULL;            // Server IP Address
    LPDHCP_RESERVATION_INFO_ARRAY  pEnumReservationInfo  = NULL;            // Variable to hold the reservations
    DWORD                          resumeHandle          = 0;               // Variable to hold resume handle
    DWORD                          dwIdx                 = 0;
    for(;;)
    {
        dwError = DhcpV4EnumSubnetReservations(
                    pwszServer,             // Server IP Address, a value of NULL reflects the current server (where the program is executed)
                    dwSubnetAddress,        // Subnet Address
                    &resumeHandle,          // Resume Handle
                    preferredMax,           // Preferred maximum
                    &pEnumReservationInfo,  // Pointer to a DHCP_RESERVATION_INFO_ARRAY, contains the reservations elements available for the specified subnet.
                    &dwElementsRead,        // Total number of elements read
                    &dwElementsTotal        // Total number of elements
                    );
        if ((NO_ERROR != dwError) &&
            (ERROR_MORE_DATA != dwError) &&
            (ERROR_NO_MORE_ITEMS != dwError ))
        {
            wprintf(L"Error in enumerating policies. Error = %d\n",dwError);
            break;
        }
        if (dwElementsRead && pEnumReservationInfo)
        {
            //operate on pEnumReservationInfo
            for ( dwIdx = 0; dwIdx < pEnumReservationInfo->NumElements; dwIdx++ )
            {
                wprintf(L"ReservedClientName = %s \n",pEnumReservationInfo->Elements[dwIdx]->ReservedClientName);
                wprintf(L"ReservedIpAddress =  %u\n",pEnumReservationInfo->Elements[dwIdx]->ReservedIpAddress);
            }
            //one needs to free the pEnumReservationInfo once used
            FreeDhcpEnumSubnetReservationsInfo(pEnumReservationInfo);
            pEnumReservationInfo= NULL;
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
