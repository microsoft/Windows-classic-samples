// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Module:
//      enumerator.cpp
//
// Abstract:
//
// Entry Points:
//      Enumerator - enumerate ATM interfaces on the local machine.
//


#include "atmevent.h"


// Abstract:
//      Determine the number of ATM adapters on the machine and then print out
//      a list of each adaptors NSAP address.
//
//
void Enumerator(
               OPTIONS          *pOptions
               )
{
    SOCKET          sd = INVALID_SOCKET;
    SOCKADDR_ATM    atm_addr = {0};
    CHAR            szAddress[MAX_ATM_INTERFACE_LEN] = {'\0'};
    DWORD           dwNumInterfaces = 0;
    DWORD           dwAddrLen = 0;
    DWORD           dwBytes=sizeof(DWORD);
    int             nRet = 0;


    sd = WSASocket(FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO, 
                   &pOptions->protocolInfo, 0, WSA_FLAG_OVERLAPPED);
    if (INVALID_SOCKET == sd)
    {
        printf("WSASocket: %d\n", WSAGetLastError());
        return;
    }

    nRet = WSAIoctl(sd, SIO_GET_NUMBER_OF_ATM_DEVICES, 
                    NULL, 0,
                    (LPVOID)&dwNumInterfaces, sizeof(dwNumInterfaces), &dwBytes, 
                    NULL, NULL);
    if (SOCKET_ERROR == nRet)
    {
        printf("WSAIoctl:SIO_GET_NUMBER_OF_ATM_DEVICES: %d\n", WSAGetLastError());
        return;
    }

    for (DWORD i=0; i < dwNumInterfaces ;i++)
    {
        ZeroMemory(&atm_addr, sizeof(SOCKADDR_ATM));

        nRet = WSAIoctl(sd, SIO_GET_ATM_ADDRESS, 
                        (LPVOID)&i, sizeof(DWORD),
                        (LPVOID)&atm_addr.satm_number, sizeof(atm_addr.satm_number), &dwBytes, 
                        NULL, NULL);
        if (SOCKET_ERROR == nRet)
        {
            printf("WSAIoctl:SIO_GET_ATM_ADDRESS: %d\n", WSAGetLastError());
            return;
        }

        // fill in remainder of ATM address structure
        atm_addr.satm_family                 = AF_ATM;
        atm_addr.satm_number.AddressType     = ATM_NSAP;
        atm_addr.satm_number.NumofDigits     = ATM_ADDR_SIZE;
        atm_addr.satm_blli.Layer2Protocol    = SAP_FIELD_ANY;
        atm_addr.satm_blli.Layer3Protocol    = SAP_FIELD_ABSENT;
        atm_addr.satm_bhli.HighLayerInfoType = SAP_FIELD_ABSENT;

        ZeroMemory(szAddress, sizeof(szAddress));        
        dwAddrLen = sizeof(szAddress);
        if (SOCKET_ERROR == WSAAddressToString((LPSOCKADDR)&atm_addr, sizeof(atm_addr), 
                               &pOptions->protocolInfo, szAddress, &dwAddrLen))
        {
            printf("WSAAddressToString: %d\n", WSAGetLastError());
            break;
        }
        printf("ATM Interface [%d]: <%s>\n", i, szAddress);
    }

    if (INVALID_SOCKET != sd)
    {
        closesocket(sd);
        sd = INVALID_SOCKET;
    }
    return;
}
