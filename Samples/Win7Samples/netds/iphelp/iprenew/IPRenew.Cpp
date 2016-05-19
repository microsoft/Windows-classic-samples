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

    iprenew.cpp

Abstract:

    This module illustrates how to programmatically release and renew IP addresses
    obtained through DHCP.  This program also demonstrates how to retrieve existing
    network adapter configuration information.

    IP address assignment from DHCP can be updated using the IpRenewAddress() and
    IpReleaseAddress() IP Helper APIs.  These APIs require you to understand adapter
    index numbers.  In Windows, every network adapter has a unique index ID.  Adapter
    index IDs can be retrieved using the GetAdaptersInfo() IP Helper API.  This program
    features a list option that displays current network adapter configuration
    information showing all adapters index numbers associated with their corresponding
    network adaptors.

    To execute this application, simply build the application using the Visual C++
    Nmake.exe program generation facility.  An iprenew.exe program should result.
    Execute the iprenew.exe with the following parameters:

    Iprenew.exe [ -l ] [ -r<index id> ] [ -n<index id> ]

        -l List adapters with corresponding index ID information
        -r Release IP address for adapter index ID
        -n Renew IP address for adapter index ID



Revision History:

*/


#include <windows.h>
#include <iphlpapi.h>
#include <stdio.h>

void Usage(void) {
    printf("Usage: Iprenew [ -l ] [ -r<index id> ] [ -n<index id>]\n\n"
            "\t -l List adapters with corresponding index ID information\n"
            "\t -r Release IP address for adapter index ID\n"
            "\t -n Renew IP address for adapter index ID\n");
}

void main(int argc, char *argv[])
{
    DWORD InterfaceInfoSize = 0;
    PIP_INTERFACE_INFO pInterfaceInfo;
    DWORD Err;
    INT i;
    PIP_ADAPTER_INFO pAdapterInfo, pAdapt;
    DWORD AdapterInfoSize = 0;
    DWORD Index = (DWORD) -1;
    BOOL OptList = FALSE;
    BOOL OptRelease = FALSE;
    BOOL OptRenew = FALSE;

    if (argc < 2)
    {
        Usage();
        return;
    }
    for (i = 1; i < argc; i++)
    {
        if ((argv[i][0] == '-') || (argv[i][0] == '/')) 
        {
            switch(tolower(argv[i][1])) 
            {
                case 'l':
                    OptList = TRUE;
                    break;
                case 'r':
                    OptRelease = TRUE;
                    if (strlen(argv[i]) > 2)
                        Index = atoi(&argv[i][2]);
                    break;
                case 'n':
                    OptRenew = TRUE;
                    if (strlen(argv[i]) > 2)
                        Index = atoi(&argv[i][2]);
                    break;
                default:
                    Usage();
                    return;
                }
        }
        else
        {
            Usage();
            return;
        }
    }

    // Check options
    if ((OptRelease && Index == -1) || (OptRenew && Index == -1))
    {
        Usage();
        return;
    }

    if (OptList)
    {
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

        printf("Index     Adapter\n");
        while (pAdapt)
        {
            printf("%-10.i%s\n", pAdapt->Index, pAdapt->Description);
            pAdapt = pAdapt->Next;
        }
    }

    if ((Err = GetInterfaceInfo(NULL, &InterfaceInfoSize)) != 0)
    {
        if (Err != ERROR_INSUFFICIENT_BUFFER)
        {
            printf("GetInterfaceInfo sizing failed with error %d\n", Err);
            return;
        }
    }

    // Allocate memory from sizing information
    if ((pInterfaceInfo = (PIP_INTERFACE_INFO) GlobalAlloc(GPTR, InterfaceInfoSize)) == NULL)
    {
        printf("Memory allocation error\n");
        return;
    }

    // Get actual adapter information
    if ((Err = GetInterfaceInfo(pInterfaceInfo, &InterfaceInfoSize)) != 0)
    {
        printf("GetInterfaceInfo failed with error %d\n", Err);
        return;
    }


    if (OptRelease)
    {
        for (i = 0; i < pInterfaceInfo->NumAdapters; i++)
            if (Index == pInterfaceInfo->Adapter[i].Index)
            {
                if ((Err = IpReleaseAddress(&pInterfaceInfo->Adapter[i])) != 0)
                {
                    printf("IpReleaseAddress failed with error %d\n", Err);
                    return;
                }
                break;
            }
    }


    if (OptRenew)
    {
        for (i = 0; i < pInterfaceInfo->NumAdapters; i++)
            if (Index == pInterfaceInfo->Adapter[i].Index)
            {
                if ((Err = IpRenewAddress(&pInterfaceInfo->Adapter[i])) != 0)
                {
                    printf("IpRenewAddress failed with error %d\n", Err);
                    return;
                }
                break;
            }
    }
}

