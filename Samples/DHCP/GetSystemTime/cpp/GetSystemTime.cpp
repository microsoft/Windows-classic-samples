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
    DWORD  dwError        = ERROR_SUCCESS; // Variable to hold returned error code
    DWORD  dwTime         = 0;             // Variable to hold system time of the server
    DWORD  dwAllowedDelay = 0;             // Variable to hold the max allowed delta in time.
    LPWSTR pwszServer     = NULL;          // Server IP Address
    dwError = DhcpV4FailoverGetSystemTime(
                    pwszServer,     // Server IP Address, a value of NULL reflects the current server (where the program is executed)
                    &dwTime,        // System time of the server
                    &dwAllowedDelay // Max time difference allowed
                    );
    if( ERROR_SUCCESS != dwError )
    {
        wprintf(L"DhcpV4FailoverGetSystemTime failed with Error = %d\n",dwError);
    }
    return 0;
}
