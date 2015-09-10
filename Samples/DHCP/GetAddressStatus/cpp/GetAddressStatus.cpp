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
    DWORD   dwStatus   = 0;             // Status of the address
    DWORD   dwSubnet   = 0xa000033;     //(10.0.0.51) 
    DWORD   dwError    = ERROR_SUCCESS; // Variable to hold error code
    LPWSTR  pwszServer = NULL;          // Server IP Address

    dwError = DhcpV4FailoverGetAddressStatus(
                    pwszServer,
                    dwSubnet,
                    &dwStatus);
    if(ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV4FailoverGetAddressStatus failed with Error = %d\n",dwError);
        return 0;
    }
    switch(dwStatus)
    {
        case 0:
            wprintf(L"The address is with primary server\n");
            break;
        case 1:
            wprintf(L"The address is with secondary (partner) server\n");
            break;
        case 2:
            wprintf(L"The address is neither with primary nor with secondary\n");
            break;
        case 3:
            wprintf(L"The address corresponds to Reservation\n");
            break;
        default:
            wprintf(L"Unknown status\n");
    }
    return 0;
}
