//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//
//      RasAgileVPN.cpp
//
//      Usage:
//      RasAgileVPN -e entry_name -p [destination_ipaddress] -u [username] -z [password]
//              -d [domain] -i [interface_index] -n [new_interface_index]
//
//      RAS API's used:
//          RasDial
//          RasGetConnectStatus
//          RasGetProjectionInfoEx
//          RasHangUp
//          RasUpdateConnection
//

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x601
#endif


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <ras.h>
#include <raserror.h>
#include <stdlib.h>
#include <strsafe.h>
#include <Ws2tcpip.h>


// PrintUsage
void PrintUsage(__in LPWSTR pszProgName) 
{
    WCHAR szTempBuf[256] = {0};

    if (pszProgName)
    {
        StringCchPrintf(szTempBuf, 
            ARRAYSIZE(szTempBuf), 
            L"Usage\n%s \t-e entry_name -p [destination_ipaddress] -u [username] -z [password] \n\t\t-d [domain] -i [interface_index] -n [new_interface_index]\n", 
            pszProgName);

        wprintf(szTempBuf);
    }

    return;
}

int __cdecl wmain(int argc, __in_ecount(argc) LPWSTR * argv)
{
    HRASCONN            hRasConn = NULL;
    LPRASDIALPARAMS     lpRasDialParams = NULL;
    RAS_PROJECTION_INFO *lpProjectionInfo = NULL;
    LPRASUPDATECONN     lpRasUpdateConn = NULL;
    LPRASCONNSTATUS     lpRasConnStatus = NULL;
    int                 i = 0;
    int                 j = 0;
    DWORD               dwRet = 0;
    DWORD               dwSize = 0;
    DWORD               dwMaxTickCount = 0;	
    DWORD               dwNewInterfaceIndex = 0;
    DWORD               cb = sizeof(RASDIALPARAMS);
    BOOL                fRequired = FALSE;
    BOOL                fIKEv2Connection = FALSE;
    WCHAR               szTempBuf[256] = {0};

    (void)HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    lpRasDialParams = (LPRASDIALPARAMS)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb); 
    if (NULL == lpRasDialParams)
    {
        dwRet = GetLastError();
        wprintf(L"HeapAlloc failed: Error %d\n", dwRet);
        return (int)dwRet;
    }

    lpRasDialParams->dwSize = sizeof(RASDIALPARAMS);

    // Copy command line arguments into the RASDIALPARAMS structure
    if (argc > 1) 
    {
        for(i = 1; i < (argc - 1); i++) 
        {
            if (argv[i] && ((argv[i][0] == '-') || (argv[i][0] == '/')))
            {
                switch(tolower(argv[i][1])) 
                {
                case 'e': // Entry name
                    j = ++i;
                    if (argv[j])
                    {
                        StringCchCopy(lpRasDialParams->szEntryName, ARRAYSIZE(lpRasDialParams->szEntryName), argv[j]);
                        fRequired = TRUE;
                    }
                    break;
                case 'p': // Phone number
                    j = ++i;
                    if (argv[j])
                    {
                        StringCchCopy(lpRasDialParams->szPhoneNumber, ARRAYSIZE(lpRasDialParams->szPhoneNumber), argv[j]);
                    }
                    break;
                case 'u': // User name
                    j = ++i;
                    if (argv[j])
                    {
                        StringCchCopy(lpRasDialParams->szUserName, ARRAYSIZE(lpRasDialParams->szUserName), argv[j]);
                    }
                    break;
                case 'z': // Password
                    j = ++i;
                    if (argv[j])
                    {
                        StringCchCopy(lpRasDialParams->szPassword, ARRAYSIZE(lpRasDialParams->szPassword), argv[j]);
                    }
                    break;
                case 'd': // Domain name
                    j = ++i;
                    if (argv[j])
                    {
                        StringCchCopy(lpRasDialParams->szDomain, ARRAYSIZE(lpRasDialParams->szDomain), argv[j]);
                    }
                    break;
                case 'i': // Interface Index to dial first
                    j = ++i;
                    if (argv[j])
                    {
                        lpRasDialParams->dwIfIndex = _wtoi(argv[j]);
                    }
                    break;
                case 'n': // New interface Index to perform MOBIKE switch
                    j = ++i;
                    if (argv[j])
                    {
                        dwNewInterfaceIndex = _wtoi(argv[j]);
                    }
                    break;
                default:
                    PrintUsage(argv[0]);
                    dwRet = ERROR_INVALID_PARAMETER;
                    break;
                }
            }
            else
            {
                PrintUsage(argv[0]);
                dwRet = ERROR_INVALID_PARAMETER;
                goto done;
            }
        }
    }
    else
    {
        PrintUsage(argv[0]);
        dwRet = ERROR_INVALID_PARAMETER;
        goto done;
    }

    // Check if we got at least required entry name
    if (FALSE == fRequired)
    {
        PrintUsage(argv[0]);
        dwRet = ERROR_INVALID_PARAMETER;
        goto done;
    }

    wprintf(L"\nDialing...\n");

    // Calling RasDial synchronously
    dwRet = RasDial(NULL, NULL, lpRasDialParams, 0, 0L, &hRasConn);
    if (dwRet)
    {
        wprintf(L"RasDial failed: Error = %d\n", dwRet);
        goto done;
    }


    lpProjectionInfo = (RAS_PROJECTION_INFO *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(RAS_PROJECTION_INFO));
    if (NULL == lpProjectionInfo)
    {
        dwRet = GetLastError();
        wprintf(L"HeapAlloc failed: Error %d\n", dwRet);
        goto disconnect;
    }

    lpProjectionInfo->version = RASAPIVERSION_CURRENT;
    dwSize = sizeof(RAS_PROJECTION_INFO);

    // Getting the RASPROJECTION_INFO_TYPE and Ras client and server IP addresses
    dwRet = RasGetProjectionInfoEx(hRasConn, lpProjectionInfo, &dwSize);

    switch (dwRet)
    {
    case ERROR_BUFFER_TOO_SMALL:
        if (HeapFree(GetProcessHeap(), 0, (LPVOID)lpProjectionInfo))
        {
            lpProjectionInfo = (RAS_PROJECTION_INFO *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);
            if (NULL == lpProjectionInfo)
            {
                dwRet = GetLastError();
                wprintf(L"HeapAlloc failed: Error %d\n", dwRet);
                goto disconnect;
            }

            lpProjectionInfo->version = RASAPIVERSION_CURRENT;
            dwRet = RasGetProjectionInfoEx(hRasConn, lpProjectionInfo, &dwSize);

            if (ERROR_SUCCESS != dwRet)
            {
                wprintf(L"RasGetProjectionInfoEx failed: Error %d\n", dwRet);
                goto disconnect;
            }
        }
        else
        {
            dwRet = GetLastError();
            wprintf(L"HeapFree failed: Error %d\n", dwRet);
            lpProjectionInfo = NULL;
            goto disconnect;
        }
        break;

    case ERROR_SUCCESS:
        break;

    default:
        wprintf(L"RasGetProjectionInfoEx failed: Error %d\n", dwRet);
        goto disconnect;
    }


    fIKEv2Connection = (lpProjectionInfo->type == PROJECTION_INFO_TYPE_IKEv2) ? TRUE : FALSE;

    // Print out the RASPROJECTION_INFO_TYPE and the Client and Server IP addresses
    wprintf(L"RASPROJECTION_INFO_TYPE is: %s\n", fIKEv2Connection ? L"IKEv2" : L"PPP");

    if(!lpProjectionInfo->ikev2.dwIPv4NegotiationError)
    {
        InetNtop(AF_INET, &(lpProjectionInfo->ikev2.ipv4Address), szTempBuf, ARRAYSIZE(szTempBuf));
        wprintf(L"Ras Client IP Address: %s\n", szTempBuf);
        InetNtop(AF_INET, &(lpProjectionInfo->ikev2.ipv4ServerAddress), szTempBuf, ARRAYSIZE(szTempBuf));
        wprintf(L"Ras Server IP Address: %s\n", szTempBuf);
    }
    else if(!lpProjectionInfo->ikev2.dwIPv6NegotiationError)
    {
        InetNtop(AF_INET6, &(lpProjectionInfo->ikev2.ipv6Address), szTempBuf, ARRAYSIZE(szTempBuf));
        wprintf(L"Ras Client IPv6 Address: %s\n", szTempBuf);
        InetNtop(AF_INET6, &(lpProjectionInfo->ikev2.ipv6ServerAddress), szTempBuf, ARRAYSIZE(szTempBuf));
        wprintf(L"Ras Server IPv6 Address: %s\n", szTempBuf);
    }

    lpRasConnStatus = (LPRASCONNSTATUS)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(RASCONNSTATUS));
    if (NULL == lpRasConnStatus)
    {
        dwRet = GetLastError();
        wprintf(L"HeapAlloc failed: Error %d\n", dwRet);
        goto disconnect;
    }

    // Set the appropriate size
    lpRasConnStatus->dwSize = sizeof(RASCONNSTATUS);

    // Before calling RasUpdateConnection ensure that the client supports Mobile IKE (MOBIKE), it is an IKEv2 
    // connection and the interfaces as specified and different.
    if( fIKEv2Connection && dwNewInterfaceIndex != 0 && lpRasDialParams->dwIfIndex !=0 &&
        dwNewInterfaceIndex != lpRasDialParams->dwIfIndex &&
        (lpProjectionInfo->ikev2.dwFlags & RASIKEv2_FLAGS_MOBIKESUPPORTED))
    {

        // Get the Ras local tunnel end point using RasGetConnectStatus
        dwRet = RasGetConnectStatus(hRasConn, lpRasConnStatus);
        if (ERROR_SUCCESS != dwRet)
        {
            wprintf(L"RasGetConnectStatus failed: Error = %d\n", dwRet);
            goto disconnect;
        }
        if(lpRasConnStatus->localEndPoint.dwType == RASTUNNELENDPOINT_IPv4)
        {
            InetNtop(AF_INET, &(lpRasConnStatus->localEndPoint.ipv4), szTempBuf, ARRAYSIZE(szTempBuf));
            wprintf(L"\nRas local tunnel end point: %s\n", szTempBuf);
        }
        else
        {
            InetNtop(AF_INET6, &(lpRasConnStatus->localEndPoint.ipv6), szTempBuf, ARRAYSIZE(szTempBuf));
            wprintf(L"\nRas local tunnel end point: %s\n", szTempBuf);
        }

        lpRasUpdateConn = (LPRASUPDATECONN)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(RASUPDATECONN));
        if (NULL == lpRasUpdateConn)
        {
            dwRet = GetLastError();
            wprintf(L"HeapAlloc failed: Error %d\n", dwRet);
            goto disconnect;
        }
        lpRasUpdateConn->version = RASAPIVERSION_CURRENT;
        lpRasUpdateConn->dwSize = sizeof(RASUPDATECONN);
        lpRasUpdateConn->dwIfIndex = dwNewInterfaceIndex;

        // Performing a MOBIKE Switch. Updating the connection to the new interface using RasUpdateConnection
        wprintf(L"RasUpdateConnection performing MOBIKE switch...\n");
        dwRet = RasUpdateConnection(hRasConn, lpRasUpdateConn);
        if (ERROR_SUCCESS != dwRet)
        {
            wprintf(L"RasUpdateConnection failed: Error = %d\n", dwRet);
            goto disconnect;
        }
        else
        {
            wprintf(L"RasUpdateConnection Successful!\n");
        }
        
        ZeroMemory(lpRasConnStatus, sizeof(RASCONNSTATUS));
        lpRasConnStatus->dwSize = sizeof(RASCONNSTATUS);
        // Get the Ras local tunnel end point using RasGetConnectStatus
        dwRet = RasGetConnectStatus(hRasConn, lpRasConnStatus);
        if (ERROR_SUCCESS != dwRet)
        {
            wprintf(L"RasGetConnectStatus failed: Error = %d\n", dwRet);
            goto disconnect;
        }
        if(lpRasConnStatus->localEndPoint.dwType == RASTUNNELENDPOINT_IPv4)
        {
            InetNtop(AF_INET, &(lpRasConnStatus->localEndPoint.ipv4), szTempBuf, ARRAYSIZE(szTempBuf));
            wprintf(L"Ras local tunnel end point: %s\n", szTempBuf);
        }
        else
        {
            InetNtop(AF_INET6, &(lpRasConnStatus->localEndPoint.ipv6), szTempBuf, ARRAYSIZE(szTempBuf));
            wprintf(L"Ras local tunnel end point: %s\n", szTempBuf);
        }
    }
    else
    {
        if(!fIKEv2Connection)
        {
            wprintf(L"Cannot do MOBIKE switch as connection in not IKEv2\n");
        }
        else if(!(lpProjectionInfo->ikev2.dwFlags & RASIKEv2_FLAGS_MOBIKESUPPORTED))
        {
            wprintf(L"Cannot do MOBIKE switch as the client does not support Mobile IKE (MOBIKE)\n");
        }
        else if(!lpRasDialParams->dwIfIndex)
        {
            wprintf(L"Cannot do MOBIKE switch as the existing interface index is not specified\n");
        }
        else if(!dwNewInterfaceIndex)
        {
            wprintf(L"Cannot do MOBIKE switch as the new interface index is not specified\n");
        }
        else if(dwNewInterfaceIndex == lpRasDialParams->dwIfIndex)
        {
            wprintf(L"Cannot do MOBIKE switch as the existing and new interface index specified are same\n");
        }
    }

    wprintf(L"\nPausing for 5 seconds before disconnecting...\n");
    Sleep(5000);

disconnect:

    // Terminating the connection using RasHangUp
    dwRet = RasHangUp(hRasConn);
    if (ERROR_SUCCESS != dwRet)
    {
        wprintf(L"RasHangUp failed: Error = %d", dwRet);
        goto done;
    }

    // Keep checking  for 10 seconds and make sure we are really disconnected.
    // Once the connection is disconnected, RasGetConnectStatus returns ERROR_INVALID_HANDLE.
    // or our timeout is reached we exit the while loop.
    // If a process exits with a connected RAS connection, the port could be stranded.
    dwMaxTickCount = GetTickCount() + 10000;

    if(lpRasConnStatus)
    {
        ZeroMemory(lpRasConnStatus, sizeof(RASCONNSTATUS));
        lpRasConnStatus->dwSize = sizeof(RASCONNSTATUS);
        while((RasGetConnectStatus(hRasConn, lpRasConnStatus) != ERROR_INVALID_HANDLE) && (dwMaxTickCount > GetTickCount()))
        {
            Sleep(50);
        }
    }

    wprintf(L"Disconnected\n");

    dwRet = ERROR_SUCCESS;

done:

    if (lpRasDialParams)
    {
        HeapFree(GetProcessHeap(), 0, (LPVOID)lpRasDialParams);
    }

    if (lpProjectionInfo)
    {
        HeapFree(GetProcessHeap(), 0, (LPVOID)lpProjectionInfo);
    }

    if (lpRasUpdateConn)
    {
        HeapFree(GetProcessHeap(), 0, (LPVOID)lpRasUpdateConn);
    }

    if (lpRasConnStatus)
    {
        HeapFree(GetProcessHeap(), 0, (LPVOID)lpRasConnStatus);
    }

    return (int)dwRet;
}
