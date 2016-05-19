/*---------------------------------------------------------------------------
    Copyright (c) 1998 - 2000  Microsoft Corporation
    Module Name: IpRoute.exe   
    File       : IpRoute.cpp
    Description: This file demonstrates the use of IP Helper APIs to
                 manipulate routing table.
    Author:
    Frank Li            April 18, 1998
    
    Revision History:
    Who         When        What
    --------    --------    ----------------------------------
    Frank Li    04-18-98    created   
---------------------------------------------------------------------------*/

#include "IpRoute.h"

void Usage(char * pszProgramName)
{
    printf("Manipulates network routing tables.\n\n");

    printf("%s -p                                               ...Prints route table.\n", pszProgramName);
    printf("%s -a destination netmask gateway interface [metric]...Adds a route.\n", pszProgramName);
    printf("%s -d destination                                   ...Deletes routes to\n", pszProgramName);
    printf("                                                            destination.\n\n");
    
    printf("destination  Specifies the destination host.\n\n");

    printf("netmask      Specifies a subnet mask value to be associated\n");
    printf("             with this route entry.\n\n");
    
    printf("gateway      Specifies gateway.\n\n");

    printf("interface    Specifies the interface ip.\n\n");

    printf("metric       The cost for this destination\n\n");

    printf("Diagnostic Notes:\n\n");
    printf("Invalid MASK generates an error, that is when (DEST & MASK) != DEST.\n");
    printf("Example> IpRoute -a 157.0.0.0 155.0.0.0 157.55.80.1 157.55.80.9\n");
    printf("         IpRoute: Invalid Mask 155.0.0.0\n\n");

    printf("Examples:\n\n");

    printf("> IpRoute -p\n");
    printf("> IpRoute -a 157.0.0.0    255.0.0.0 157.55.80.1  157.55.80.9           1\n");
    printf("             ^destination ^mask     ^gateway     ^existing interface   ^metric\n");
    printf("> IpRoute -p\n");
    printf("> IpRoute -d 157.0.0.0\n");
    printf("> IpRoute -p\n");

    WSACleanup();
    exit(1);
}


void _cdecl main(int argc, char **argv)
{
    WORD wVersionRequested = MAKEWORD(1,1);
    WSADATA wsaData;
    int nRet;

    nRet = WSAStartup(wVersionRequested, &wsaData);
    if (wsaData.wVersion != wVersionRequested)
    {    
        fprintf(stderr,"\n Wrong version\n");
        return;
    }

    if ((argc < 2) || (argv[1][0] != '-'))        
        Usage("IpRoute");
    if (strlen(argv[1]) > 2)     
        Usage("IpRoute");  

    switch(argv[1][1]) 
    {    
    case 'p':
        // Print routing table
        DoGetIpForwardTable();
        break;
    case 'a':   
        //Adds an entry into routing table   
        if (argc == 6)
            DoSetIpForwardEntry(argv[2], argv[3], argv[4], argv[5]); // dest mask gateway if
        else if (argc == 7)
        {
            DWORD dwMetric;
            if (sscanf_s(argv[6], "%u", &dwMetric ) == 1)
                DoSetIpForwardEntry(argv[2], argv[3], argv[4], argv[5], dwMetric);
            else
                printf("IpRoute: Bad argument %s\n", argv[6]);
        }
        else
            Usage("IpRoute");
        break;    
         
    case 'd':       
        //Delete an entry from the routing table    
        if (argc == 3)
            DoDeleteIpForwardEntry(argv[2]);
        else
            Usage("IpRoute");                
        break;  
    default:    
        // help
        Usage("IpRoute");        
        break;    
    }
    WSACleanup();
}

void DoGetIpForwardTable()
{
    DWORD dwStatus;
    PMIB_IPFORWARDTABLE pIpRouteTab = NULL; // Ip routing table
    

    if ( (dwStatus = MyGetIpForwardTable(pIpRouteTab, TRUE)) == NO_ERROR)
    {
        PrintIpForwardTable(pIpRouteTab);
        free(pIpRouteTab);
        return;
    }
    else if ( dwStatus == ERROR_NO_DATA)
    {
        printf("No entries in route table.\n");
        if (pIpRouteTab)
            free (pIpRouteTab);
        return;
    }
    else
    {
        if (pIpRouteTab)
            free (pIpRouteTab);
        printf("IpRoute returned 0x%x\n", dwStatus);
        return;
    }
}

void DoSetIpForwardEntry(char* pszDest, char* pszNetMask, char* pszGateway, char* pszInterface, DWORD dwMetric)
{
    DWORD dwStatus;

    MIB_IPFORWARDROW routeEntry;            // Ip routing table row entry
    PMIB_IPADDRTABLE pIpAddrTable = NULL;   // Ip Addr Table
    DWORD dwIfIndex;                        // Interface index number  
    DWORD dwIfMask;                         // Interface Subnet Mask
    DWORD dwIfIpAddr;                       // Interface Ip Address
    
    memset(&routeEntry, 0, sizeof(MIB_IPFORWARDROW));
    

    // converting and checking input arguments...
    if (pszDest == NULL || pszNetMask == NULL || pszGateway == NULL)
    {
        printf("IpRoute: Bad Argument\n");
        return;
    }

    routeEntry.dwForwardDest = inet_addr(pszDest); // convert dotted ip addr. to ip addr.
    if (routeEntry.dwForwardDest == INADDR_NONE)
    {
        printf("IpRoute: Bad Destination %s\n", pszDest);
        return;
    }

    routeEntry.dwForwardMask = inet_addr(pszNetMask);
    if ( (routeEntry.dwForwardMask == INADDR_NONE) && 
         (strcmp("255.255.255.255", pszNetMask) != 0) )
    {
        printf("IpRoute: Bad Mask %s\n", pszNetMask);
        return;
    }

    routeEntry.dwForwardNextHop = inet_addr(pszGateway);
    if (routeEntry.dwForwardNextHop == INADDR_NONE)
    {
        printf("IpRoute: Bad Gateway %s\n", pszGateway);
        return;
    }

    if ( (routeEntry.dwForwardDest & routeEntry.dwForwardMask) != routeEntry.dwForwardDest)
    {
        printf("IpRoute: Invalid Mask %s\n", pszNetMask);
        return;
        
    }

    dwIfIpAddr = inet_addr(pszInterface);
    if (dwIfIpAddr == INADDR_NONE)
    {
        printf("IpRoute: Bad Interface %s\n", pszInterface);
        return;
    }


    // Check if we have the given interface
    if ( (dwStatus = MyGetIpAddrTable(pIpAddrTable)) != NO_ERROR)
    {
        printf("GetIpAddrTable returned 0x%x\n", dwStatus);
        if (pIpAddrTable)
            free(pIpAddrTable);
        return;
    }
    assert(pIpAddrTable);
    if ( InterfaceIpToIdxAndMask(pIpAddrTable, pszInterface, dwIfIndex, dwIfMask) == FALSE)
    {
        printf("IpRoute: Bad Argument %s\n", pszInterface);
        return;
    }
    free(pIpAddrTable);


    if ( (routeEntry.dwForwardNextHop & dwIfMask) != (dwIfIpAddr & dwIfMask) )
    {
        printf("IpRoute: Gateway %s and Interface %s are not in the same subnet.\n", pszGateway, pszInterface);
        return;
    }

    routeEntry.dwForwardIfIndex = dwIfIndex;
    
    routeEntry.dwForwardMetric1 = dwMetric;

    // some default values
    routeEntry.dwForwardProto = MIB_IPPROTO_NETMGMT;
    routeEntry.dwForwardMetric2 = (DWORD)-1;
    routeEntry.dwForwardMetric3 = (DWORD)-1;
    routeEntry.dwForwardMetric4 = (DWORD)-1;
    
    dwStatus = SetIpForwardEntry(&routeEntry); 
    if (dwStatus != NO_ERROR)
    {
        printf("IpRoute: couldn't add (%s), dwStatus = %lu.\n",
                    pszDest, dwStatus);
    }

}


void DoDeleteIpForwardEntry(char* pszDest)
{
    DWORD dwStatus, dwDelStatus, i;
    PMIB_IPFORWARDTABLE pIpRouteTab = NULL; // Ip routing table
    MIB_IPFORWARDROW routeEntry;            // Ip routing table row entry
    DWORD dwForwardDest = 0;
    bool fDeleted = FALSE;
    

    memset(&routeEntry, 0, sizeof(MIB_IPFORWARDROW));
    dwForwardDest = inet_addr(pszDest); // convert dotted ip addr. to ip addr.
    if (dwForwardDest == INADDR_NONE)
    {
        printf("IpRoute: Bad Destination %s\n", pszDest);
        return;
    }


    if ( (dwStatus = MyGetIpForwardTable(pIpRouteTab, TRUE)) == NO_ERROR)
    {
        for (i = 0; i < pIpRouteTab->dwNumEntries; i++)
        {
            if (dwForwardDest == pIpRouteTab->table[i].dwForwardDest)
            {
                memcpy(&routeEntry, &(pIpRouteTab->table[i]), sizeof(MIB_IPFORWARDROW));
                dwDelStatus = DeleteIpForwardEntry(&routeEntry); 
                if (dwDelStatus != NO_ERROR)
                {
                    printf("IpRoute: couldn't delete (%s), dwStatus = %lu.\n",
                            pszDest, dwDelStatus);
                    return;
                }
                else
                    fDeleted = TRUE;
            }
        }
        free(pIpRouteTab);
        if (! fDeleted)
            printf("IpRoute: The route specified was not found.\n");
        return;
    }
    else if ( dwStatus == ERROR_NO_DATA)
    {
        printf("IpRoute: No entries in route table.\n");
        if (pIpRouteTab)
            free (pIpRouteTab);
        return;
    }
    else
    {
        if (pIpRouteTab)
            free (pIpRouteTab);
        printf("IpRoute returned 0x%x\n", dwStatus);
        return;
    }

}



//----------------------------------------------------------------------------
// If returned status is NO_ERROR, then pIpRouteTab points to a routing
// table.
//----------------------------------------------------------------------------
DWORD MyGetIpForwardTable(PMIB_IPFORWARDTABLE& pIpRouteTab, BOOL fOrder)
{
    DWORD status = NO_ERROR;
    DWORD statusRetry = NO_ERROR;
    DWORD dwActualSize = 0;
    
    // query for buffer size needed
    status = GetIpForwardTable(pIpRouteTab, &dwActualSize, fOrder);

    if (status == NO_ERROR)
    {
        printf("No error\n");
        return status;
    }
    else if (status == ERROR_INSUFFICIENT_BUFFER)
    {
        // need more space

        pIpRouteTab = (PMIB_IPFORWARDTABLE) malloc(dwActualSize);
        assert(pIpRouteTab);
        
        statusRetry = GetIpForwardTable(pIpRouteTab, &dwActualSize, fOrder);
        return statusRetry;
    }
    else
    {
        return status;
    }

}

//----------------------------------------------------------------------------
// Print out ip routing table in the following format:
//Active Routes:
//
//  Network Address          Netmask  Gateway Address        Interface  Metric
//          0.0.0.0          0.0.0.0     157.54.176.1   157.54.177.149       1
//        127.0.0.0        255.0.0.0        127.0.0.1        127.0.0.1       1
//     157.54.176.0    255.255.252.0   157.54.177.149   157.54.177.149       1
//   157.54.177.149  255.255.255.255        127.0.0.1        127.0.0.1       1
//   157.54.255.255  255.255.255.255   157.54.177.149   157.54.177.149       1
//        224.0.0.0        224.0.0.0   157.54.177.149   157.54.177.149       1
//  255.255.255.255  255.255.255.255   157.54.177.149   157.54.177.149       1
//----------------------------------------------------------------------------
void PrintIpForwardTable(PMIB_IPFORWARDTABLE pIpRouteTable)
{
    DWORD i, dwStatus, dwCurrIndex;
    struct in_addr inadDest;
    struct in_addr inadMask;
    struct in_addr inadGateway;  
    char szIpAddr[128];          // ip address of an interface index
    PMIB_IPADDRTABLE pIpAddrTable = NULL;

    char szDestIp[128];
    char szMaskIp[128];
    char szGatewayIp[128];

    if (pIpRouteTable == NULL)
    {
        printf( "pIpRouteTable == NULL in line %d\n", __LINE__);
        return;
    }
    // get IP Address Table for mapping interface index number to ip address
    if ( (dwStatus = MyGetIpAddrTable(pIpAddrTable)) != NO_ERROR)
    {
        printf("GetIpAddrTable returned 0x%x\n", dwStatus);
        if (pIpAddrTable)
            free(pIpAddrTable);
        return;
    }
    assert(pIpAddrTable);
    
    
    printf("Active Routes:\n\n");

    printf("  Network Address          Netmask  Gateway Address        Interface  Metric\n");
    for (i = 0; i < pIpRouteTable->dwNumEntries; i++)
    {
        dwCurrIndex = pIpRouteTable->table[i].dwForwardIfIndex;
        if (InterfaceIdxToInterfaceIp(pIpAddrTable, dwCurrIndex, szIpAddr) == FALSE)
        {
            printf("Error: Could not convert Interface number 0x%X to IP address.\n",dwCurrIndex);
            if (pIpAddrTable)
                free(pIpAddrTable);
            return;
        }
        
        inadDest.s_addr = pIpRouteTable->table[i].dwForwardDest;
        inadMask.s_addr = pIpRouteTable->table[i].dwForwardMask;
        inadGateway.s_addr = pIpRouteTable->table[i].dwForwardNextHop;
        
        strcpy_s(szDestIp, sizeof(szDestIp), inet_ntoa(inadDest));
        strcpy_s(szMaskIp, sizeof(szDestIp), inet_ntoa(inadMask));
        strcpy_s(szGatewayIp, sizeof(szDestIp), inet_ntoa(inadGateway));
        printf("  %15s %16s %16s %16s %7d\n", 
                szDestIp, 
                szMaskIp, 
                szGatewayIp, 
                szIpAddr, 
                pIpRouteTable->table[i].dwForwardMetric1);
        
    }

    if (pIpAddrTable)
        free(pIpAddrTable);

}

//----------------------------------------------------------------------------
// Inputs: pIpAddrTable is the IP address table
//         dwIndex is the Interface Number
// Output: If it returns TRUE, str contains the ip address of the interface
//----------------------------------------------------------------------------
bool InterfaceIdxToInterfaceIp(PMIB_IPADDRTABLE pIpAddrTable, DWORD dwIndex, char str[])
{
    struct in_addr inadTmp;
    char* szIpAddr2;

    if (pIpAddrTable == NULL ||  str == NULL)
        return FALSE;
    str[0] = '\0';
    for (DWORD dwIdx = 0; dwIdx < pIpAddrTable->dwNumEntries; dwIdx++)
    {
        if (dwIndex == pIpAddrTable->table[dwIdx].dwIndex)
        {
            inadTmp.s_addr = pIpAddrTable->table[dwIdx].dwAddr;
            szIpAddr2 = inet_ntoa(inadTmp);
            if (szIpAddr2)
            {
                strcpy_s(str, 128, szIpAddr2);
                return TRUE;
            }
            else
                return FALSE;
        }
    }
    return FALSE;

}

//----------------------------------------------------------------------------
// Inputs: pIpAddrTable is the IP address table
//         str is the Interface Ip address in dotted decimal format
// Output: If it returns TRUE, dwIndex contains the interface index number
//         and dwMask contains the corresponding subnet mask.
//----------------------------------------------------------------------------
bool InterfaceIpToIdxAndMask(PMIB_IPADDRTABLE pIpAddrTable, char str[], DWORD& dwIndex, DWORD& dwMask)
{
    DWORD dwIfIpAddr; // Interface Ip Address

    if (pIpAddrTable == NULL)
        return FALSE;

    dwIfIpAddr = inet_addr(str);
    if (dwIfIpAddr == INADDR_NONE)
        return FALSE;

    for (DWORD dwIdx = 0; dwIdx < pIpAddrTable->dwNumEntries; dwIdx++)
    {
        if (dwIfIpAddr == pIpAddrTable->table[dwIdx].dwAddr)
        {
            dwIndex = pIpAddrTable->table[dwIdx].dwIndex;
            dwMask = pIpAddrTable->table[dwIdx].dwMask;
            return TRUE;
        }
    }
    return FALSE;
}

//----------------------------------------------------------------------------
// If returned status is NO_ERROR, then pIpAddrTable points to a Ip Address
// table.
//----------------------------------------------------------------------------
DWORD MyGetIpAddrTable(PMIB_IPADDRTABLE& pIpAddrTable, BOOL fOrder)
{
    DWORD status = NO_ERROR;
    DWORD statusRetry = NO_ERROR;
    DWORD dwActualSize = 0;
    
    // query for buffer size needed
    status = GetIpAddrTable(pIpAddrTable, &dwActualSize, fOrder);

    if (status == NO_ERROR)
    {
        printf("No error\n");
        return status;
    }
    else if (status == ERROR_INSUFFICIENT_BUFFER)
    {
        // need more space

        pIpAddrTable = (PMIB_IPADDRTABLE) malloc(dwActualSize);
        assert(pIpAddrTable);
        
        statusRetry = GetIpAddrTable(pIpAddrTable, &dwActualSize, fOrder);
        return statusRetry;
    }
    else
    {
        return status;
    }
}

