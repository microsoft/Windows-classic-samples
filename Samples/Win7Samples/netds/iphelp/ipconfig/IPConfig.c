/******************************************************************************\
*       This is a part of the Microsoft Source Code Samples.
*       Copyright 1996 - 2000 Microsoft Corporation.
*       All rights reserved.
*       This source code is only intended as a supplement to
*       Microsoft Development Tools and/or WinHelp documentation.
*       See these sources for detailed information regarding the
*       Microsoft samples programs.
\******************************************************************************/

/*
Module Name:

    Ipconfig.cpp

Abstract:

    This module illustrates how to programmatically retrieve IP configuration
    information similar to the IPCONFIG.EXE utility.  It demonstrates how to use
    the IP Helper APIs GetNetworkParams() and GetAdaptersInfo().

    To execute this application, simply build the application using the Microsoft Visual C++
    nmake.exe program generation utility to make an executable ipconfig.exe.  After the
    build is complete, simply execute the resulting ipconfig.exe program.


Author:

    Jim Ohlund 21-Apr-98

Revision History:
    stbaker 13-Dec-2006

*/


#include <winsock2.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <time.h>

void __cdecl main(void) {

    DWORD Err;

    PFIXED_INFO pFixedInfo;
    DWORD FixedInfoSize = 0;

    PIP_ADAPTER_INFO pAdapterInfo, pAdapt;
    DWORD AdapterInfoSize;
    PIP_ADDR_STRING pAddrStr;

    UINT i;

    struct tm newtime;
    char buffer[32];    
    errno_t error;

    //
    // Get the main IP configuration information for this machine using a FIXED_INFO structure
    //
    if ((Err = GetNetworkParams(NULL, &FixedInfoSize)) != 0)
    {
        if (Err != ERROR_BUFFER_OVERFLOW)
        {
            printf("GetNetworkParams sizing failed with error %d\n", Err);
            return;
        }
    }

    // Allocate memory from sizing information
    if ((pFixedInfo = (PFIXED_INFO) GlobalAlloc(GPTR, FixedInfoSize)) == NULL)
    {
        printf("Memory allocation error\n");
        return;
    }

    if ((Err = GetNetworkParams(pFixedInfo, &FixedInfoSize)) == 0)
    {
        printf("\tHost Name . . . . . . . . . : %s\n", pFixedInfo->HostName);
        printf("\tDNS Servers . . . . . . . . : %s\n", pFixedInfo->DnsServerList.IpAddress.String);
        pAddrStr = pFixedInfo->DnsServerList.Next;
        while(pAddrStr)
        {
            printf("%51s\n", pAddrStr->IpAddress.String);
            pAddrStr = pAddrStr->Next;
        }

        printf("\tNode Type . . . . . . . . . : ");
        switch (pFixedInfo->NodeType)
        {
            case 1:
                printf("%s\n", "Broadcast");
                break;
            case 2:
                printf("%s\n", "Peer to peer");
                break;
            case 4:
                printf("%s\n", "Mixed");
                break;
            case 8:
                printf("%s\n", "Hybrid");
                break;
            default:
                printf("\n");
        }

        printf("\tNetBIOS Scope ID. . . . . . : %s\n", pFixedInfo->ScopeId);
        printf("\tIP Routing Enabled. . . . . : %s\n", (pFixedInfo->EnableRouting ? "yes" : "no"));
        printf("\tWINS Proxy Enabled. . . . . : %s\n", (pFixedInfo->EnableProxy ? "yes" : "no"));
        printf("\tNetBIOS Resolution Uses DNS : %s\n", (pFixedInfo->EnableDns ? "yes" : "no"));
    } else
    {
        printf("GetNetworkParams failed with error %d\n", Err);
        return;
    }

    //
    // Enumerate all of the adapter specific information using the IP_ADAPTER_INFO structure.
    // Note:  IP_ADAPTER_INFO contains a linked list of adapter entries.
    //
    AdapterInfoSize = 0;
    if ((Err = GetAdaptersInfo(NULL, &AdapterInfoSize)) != 0)
    {
        if (Err != ERROR_BUFFER_OVERFLOW)
        {
            printf("GetAdaptersInfo sizing failed with error %d\n", Err);
            return;
        }
    }

    // Allocate memory from sizing information
    if ((pAdapterInfo = (PIP_ADAPTER_INFO) GlobalAlloc(GPTR, AdapterInfoSize)) == NULL)
    {
        printf("Memory allocation error\n");
        return;
    }

    // Get actual adapter information
    if ((Err = GetAdaptersInfo(pAdapterInfo, &AdapterInfoSize)) != 0)
    {
        printf("GetAdaptersInfo failed with error %d\n", Err);
        return;
    }

    pAdapt = pAdapterInfo;

    while (pAdapt)
    {
        switch (pAdapt->Type)
        {
            case MIB_IF_TYPE_ETHERNET:
                printf("\nEthernet adapter ");
                break;
            case MIB_IF_TYPE_TOKENRING:
                printf("\nToken Ring adapter ");
                break;
            case MIB_IF_TYPE_FDDI:
                printf("\nFDDI adapter ");
                break;
            case MIB_IF_TYPE_PPP:
                printf("\nPPP adapter ");
                break;
            case MIB_IF_TYPE_LOOPBACK:
                printf("\nLoopback adapter ");
                break;
            case MIB_IF_TYPE_SLIP:
                printf("\nSlip adapter ");
                break;
            case MIB_IF_TYPE_OTHER:
            default:
                printf("\nOther adapter ");
        }
        printf("%s:\n\n", pAdapt->AdapterName);

        printf("\tDescription . . . . . . . . : %s\n", pAdapt->Description); 

        printf("\tPhysical Address. . . . . . : ");
	 
        for (i=0; i<pAdapt->AddressLength; i++)
        {
            if (i == (pAdapt->AddressLength - 1))
                printf("%.2X\n",(int)pAdapt->Address[i]);
            else
                printf("%.2X-",(int)pAdapt->Address[i]);
        }        

        printf("\tDHCP Enabled. . . . . . . . : %s\n", (pAdapt->DhcpEnabled ? "yes" : "no"));

        pAddrStr = &(pAdapt->IpAddressList);
        while(pAddrStr)
        {
            printf("\tIP Address. . . . . . . . . : %s\n", pAddrStr->IpAddress.String);
            printf("\tSubnet Mask . . . . . . . . : %s\n", pAddrStr->IpMask.String);
            pAddrStr = pAddrStr->Next;
        }

        printf("\tDefault Gateway . . . . . . : %s\n", pAdapt->GatewayList.IpAddress.String);
        pAddrStr = pAdapt->GatewayList.Next;
        while(pAddrStr)
        {
            printf("%51s\n", pAddrStr->IpAddress.String);
            pAddrStr = pAddrStr->Next;
        }

        printf("\tDHCP Server . . . . . . . . : %s\n", pAdapt->DhcpServer.IpAddress.String);
        printf("\tPrimary WINS Server . . . . : %s\n", pAdapt->PrimaryWinsServer.IpAddress.String);
        printf("\tSecondary WINS Server . . . : %s\n", pAdapt->SecondaryWinsServer.IpAddress.String);

        // Display coordinated universal time - GMT 
#ifdef WIN64
        error = _localtime64_s(&newtime, &pAdapt->LeaseObtained);
#else
        error = _localtime32_s(&newtime, &pAdapt->LeaseObtained);
#endif 
        if (error)
        {
            printf("Invalid Argument to _localtime32_s.");
        } else {
            // Convert to an ASCII representation 
            error = asctime_s(buffer, 32, &newtime);
            if (error)
            {
                printf("Invalid Argument to asctime_s.");
            } else {  
                printf( "\tLease Obtained. . . . . . . : %s", buffer);
            }
        }

#ifdef WIN64
        error = _localtime64_s(&newtime, &pAdapt->LeaseExpires); 
#else
        error = _localtime32_s(&newtime, &pAdapt->LeaseExpires); 
#endif 
        if (error)
        {
            printf("Invalid Argument to _localtime32_s.");
        } else {
            // Convert to an ASCII representation 
            error = asctime_s(buffer, 32, &newtime);
            if (error)
            {
                printf("Invalid Argument to asctime_s.");
            } else { 
                printf( "\tLease Expires . . . . . . . : %s", buffer);
            }
        }

        pAdapt = pAdapt->Next;
    }
}

