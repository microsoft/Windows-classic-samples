/*---------------------------------------------------------------------------
    Copyright (c) 1998 - 2000  Microsoft Corporation
    Module Name: IpStat.exe    
    File       : IpStat.cpp
    Description: This file demonstrates the use of IP Helper APIs to
                 get IP statistics.
    Author:
    Frank Li            April 21, 1998

    
    Revision History:
    Who         When        What
    --------    --------    ----------------------------------
    Frank Li    04-21-98    created   
---------------------------------------------------------------------------*/

#include "IpStat.h"

void Usage(char * pszProgramName)
{
    printf("Manipulates IP Statistics.\n\n");


    printf("%s -p proto      Shows connections for the protocol specified\n", pszProgramName);
    printf("                     by proto, proto may be tcp or udp.\n");  
    printf("%s -s [-p proto] Displays per-protocol statistics.\n", pszProgramName);
    printf("                     By default, statistics are shown for\n");
    printf("                     IP, ICMP, TCP and UDP; the -p option\n");
    printf("                     may be used to specify a subset of the default.\n");

    printf("Examples:\n\n");

    printf("> IpStat -p tcp\n");
    printf("> IpStat  -s\n");

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
        Usage("IpStat");
    if (strlen(argv[1]) > 2)     
        Usage("IpStat");  
    
    switch(argv[1][1]) 
    {    
    case 'p':
        // Print connection table
        if (argc == 3)
            DoGetConnTable(argv[2]);
        else
            Usage("IpStat");
        break;
    case 's':   
        // show statistics
        if (argc == 4 && argv[2][1] == 'p')
            DoGetStat(argv[3]); // Get stat for a specific protocol
        else if (argc == 2)
            DoGetStat(); // Get stat for all protocols
        else
            Usage("IpStat");
        break;    
         
        
    default:
        // help
        Usage("IpStat");        
        break;    
    }
    WSACleanup();
}

void DoGetConnTable(char* pszProto)
{
    DWORD dwStatus;
    if (_strnicmp(pszProto, "tcp", 3) == 0)
    {
        //Print Tcp Connnection Table
        PMIB_TCPTABLE pTcpTable = NULL;
        dwStatus = MyGetTcpTable(pTcpTable, TRUE);
        if (dwStatus != NO_ERROR)
        {
            printf("Ipstat: Couldn't get tcp connection table.\n");
            if (pTcpTable)
                free(pTcpTable);
            return;
        }
        else
        {
            DumpTcpTable(pTcpTable);
            free(pTcpTable);
        }
    }
    else if (_strnicmp(pszProto, "udp", 3) == 0)
    {
        //Print Udp Table
        PMIB_UDPTABLE pUdpTable = NULL;
        dwStatus = MyGetUdpTable(pUdpTable, TRUE);
        if (dwStatus != NO_ERROR)
        {
            printf("Ipstat: Couldn't get udp table.\n");
            if (pUdpTable)
                free(pUdpTable);
            return;
        }
        else
        {
            DumpUdpTable(pUdpTable);
            free(pUdpTable);
        }
    }
    else
        Usage("IpStat");
}

void DoGetStat(char* pszProto /*NULL*/)
{
    if (pszProto == NULL)
    {
        // by default, display all statistics
        {
        PMIB_IPSTATS pIpStats = NULL;
        if (MyGetIpStatistics(pIpStats) != NO_ERROR)
            printf("IpStat: error in getting ip statistics.\n");
        else
            PrintIpStats(pIpStats);

        if (pIpStats)
            free(pIpStats);
        }
        {
        PMIB_ICMP pIcmpStats = NULL;
        if (MyGetIcmpStatistics(pIcmpStats) != NO_ERROR)
            printf("IpStat: error in getting icmp statistics.\n");
        else
            PrintIcmpStats(&(pIcmpStats->stats));

        if (pIcmpStats)
            free(pIcmpStats);

        }
        {
        PMIB_TCPSTATS pTcpStats = NULL;
        if (MyGetTcpStatistics(pTcpStats) != NO_ERROR)
            printf("IpStat: error in getting tcp statistics.\n");
        else
            PrintTcpStats(pTcpStats);

        if (pTcpStats)
            free(pTcpStats);
        }
        {
        PMIB_UDPSTATS pUdpStats = NULL;
        if (MyGetUdpStatistics(pUdpStats) != NO_ERROR)
            printf("IpStat: error in getting udp statistics.\n");
        else
            PrintUdpStats(pUdpStats);

        if (pUdpStats)
            free(pUdpStats);

        }
    }
    // make sure the protocol specified was ip not ipx or some other string
    else if (strlen(pszProto) == 2 && _strnicmp(pszProto, "ip", 2) == 0)
    {
        PMIB_IPSTATS pIpStats = NULL;
        if (MyGetIpStatistics(pIpStats) != NO_ERROR)
            printf("IpStat: error in getting ip statistics.\n");
        else
            PrintIpStats(pIpStats);

        if (pIpStats)
            free(pIpStats);
    }

    else if (_strnicmp(pszProto, "icmp", 4) == 0)
    {
        PMIB_ICMP pIcmpStats = NULL;
        if (MyGetIcmpStatistics(pIcmpStats) != NO_ERROR)
            printf("IpStat: error in getting icmp statistics.\n");
        else
            PrintIcmpStats(&(pIcmpStats->stats));

        if (pIcmpStats)
            free(pIcmpStats);

    }

    else if (_strnicmp(pszProto, "tcp", 3) == 0)
    {
        PMIB_TCPSTATS pTcpStats = NULL;
        if (MyGetTcpStatistics(pTcpStats) != NO_ERROR)
            printf("IpStat: error in getting tcp statistics.\n");
        else
            PrintTcpStats(pTcpStats);

        if (pTcpStats)
            free(pTcpStats);

    }

    else if (_strnicmp(pszProto, "udp", 3) == 0)
    {
        PMIB_UDPSTATS pUdpStats = NULL;
        if (MyGetUdpStatistics(pUdpStats) != NO_ERROR)
            printf("IpStat: error in getting udp statistics.\n");
        else
            PrintUdpStats(pUdpStats);

        if (pUdpStats)
            free(pUdpStats);
    }
    else
        printf("IpStat: no available statistics for %s.\n", pszProto);

}


void DumpTcpTable(PMIB_TCPTABLE pTcpTable)
{
    char    strState[MAX_STRLEN];
    struct  in_addr    inadLocal, inadRemote;
    DWORD   dwRemotePort = 0;
    char    szLocalIp[MAX_STRLEN];
    char    szRemIp[MAX_STRLEN];

    if (pTcpTable != NULL)
    {
        printf("TCP TABLE\n");
        printf("%20s %10s %20s %10s %s\n", "Loc Addr", "Loc Port", "Rem Addr",
                "Rem Port", "State");
        for (UINT i = 0; i < pTcpTable->dwNumEntries; ++i)
        {
            switch (pTcpTable->table[i].dwState)
            {
            case MIB_TCP_STATE_CLOSED:
                strcpy_s(strState, MAX_STRLEN, "CLOSED");
                break;
            case MIB_TCP_STATE_TIME_WAIT:
                strcpy_s(strState, MAX_STRLEN, "TIME_WAIT");
                break;
            case MIB_TCP_STATE_LAST_ACK:
                strcpy_s(strState, MAX_STRLEN, "LAST_ACK");
                break;
            case MIB_TCP_STATE_CLOSING:
                strcpy_s(strState, MAX_STRLEN, "CLOSING");
                break;
            case MIB_TCP_STATE_CLOSE_WAIT:
                strcpy_s(strState, MAX_STRLEN, "CLOSE_WAIT");
                break;
            case MIB_TCP_STATE_FIN_WAIT1:
                strcpy_s(strState, MAX_STRLEN, "FIN_WAIT1");
                break;
            case MIB_TCP_STATE_ESTAB:
                strcpy_s(strState, MAX_STRLEN, "ESTAB");
                break;
            case MIB_TCP_STATE_SYN_RCVD:
                strcpy_s(strState, MAX_STRLEN, "SYN_RCVD");
                break;
            case MIB_TCP_STATE_SYN_SENT:
                strcpy_s(strState, MAX_STRLEN, "SYN_SENT");
                break;
            case MIB_TCP_STATE_LISTEN:
                strcpy_s(strState, MAX_STRLEN, "LISTEN");
                break;
            case MIB_TCP_STATE_DELETE_TCB:
                strcpy_s(strState, MAX_STRLEN, "DELETE");
                break;
            default:
                printf("Error: unknown state!\n");
                break;
            }
            inadLocal.s_addr = pTcpTable->table[i].dwLocalAddr;

            if (strcmp(strState, "LISTEN") != 0)
            {
                dwRemotePort = pTcpTable->table[i].dwRemotePort;
            }
            else
                dwRemotePort = 0;


            inadRemote.s_addr = pTcpTable->table[i].dwRemoteAddr;
            strcpy_s(szLocalIp, MAX_STRLEN, inet_ntoa(inadLocal));
            strcpy_s(szRemIp, MAX_STRLEN, inet_ntoa(inadRemote));
            printf("%20s %10u %20s %10u %s\n", 
                szLocalIp,  ntohs((unsigned short)(0x0000FFFF & pTcpTable->table[i].dwLocalPort)),
                szRemIp, ntohs((unsigned short)(0x0000FFFF & dwRemotePort)),
                strState);
        }
    }
}

void DumpUdpTable(PMIB_UDPTABLE pUdpTable)
{

    struct in_addr    inadLocal;
    if (pUdpTable != NULL)
    {
        printf("UDP TABLE\n");
        printf("%20s %10s\n", "Loc Addr", "Loc Port");
        for (UINT i = 0; i < pUdpTable->dwNumEntries; ++i)
        {
            inadLocal.s_addr = pUdpTable->table[i].dwLocalAddr;

            printf("%20s %10u \n", 
                inet_ntoa(inadLocal), ntohs((unsigned short)(0x0000FFFF & pUdpTable->table[i].dwLocalPort)));
        }
    }
}

//----------------------------------------------------------------------------
// Wrapper to GetTcpTable()
//----------------------------------------------------------------------------
DWORD MyGetTcpTable(PMIB_TCPTABLE& pTcpTable, BOOL fOrder)
{
    DWORD status = NO_ERROR;
    DWORD statusRetry = NO_ERROR;
    DWORD dwActualSize = 0;


    // query for buffer size needed
    status = GetTcpTable(pTcpTable, &dwActualSize, fOrder);

    if (status == NO_ERROR)
    {
        printf("No error\n");
        return status;
    }
    else if (status == ERROR_INSUFFICIENT_BUFFER)
    {
        // need more space
        pTcpTable = (PMIB_TCPTABLE) malloc(dwActualSize);
        assert(pTcpTable);
        
        statusRetry = GetTcpTable(pTcpTable, &dwActualSize, fOrder);
        return statusRetry;
    }
    else
    {
        return status;
    }
}

//----------------------------------------------------------------------------
// Wrapper to GetUdpTable()
//----------------------------------------------------------------------------
DWORD MyGetUdpTable(PMIB_UDPTABLE& pUdpTable, BOOL fOrder)
{
    DWORD status = NO_ERROR;
    DWORD statusRetry = NO_ERROR;
    DWORD dwActualSize = 0;


    // query for buffer size needed
    status = GetUdpTable(pUdpTable, &dwActualSize, fOrder);

    if (status == NO_ERROR)
    {
        printf("No error\n");
        return status;
    }
    else if (status == ERROR_INSUFFICIENT_BUFFER)
    {
        // need more space
        pUdpTable = (PMIB_UDPTABLE) malloc(dwActualSize);
        assert(pUdpTable);
        
        statusRetry = GetUdpTable(pUdpTable, &dwActualSize, fOrder);
        return statusRetry;
    }
    else
    {
        return status;
    }
}

DWORD MyGetIpStatistics(PMIB_IPSTATS& pIpStats)
{
    pIpStats = (PMIB_IPSTATS)malloc(sizeof(MIB_IPSTATS));
    assert(pIpStats != NULL);
    return GetIpStatistics(pIpStats);
}

DWORD MyGetIcmpStatistics(PMIB_ICMP& pIcmpStats)
{
    pIcmpStats = (PMIB_ICMP)malloc(sizeof(MIB_ICMP));
    assert(pIcmpStats != NULL);
    return GetIcmpStatistics(pIcmpStats);
}

DWORD MyGetTcpStatistics(PMIB_TCPSTATS& pTcpStats)
{
    pTcpStats = (PMIB_TCPSTATS)malloc(sizeof(MIB_TCPSTATS));
    assert(pTcpStats != NULL);
    return GetTcpStatistics(pTcpStats);
}

DWORD MyGetUdpStatistics(PMIB_UDPSTATS& pUdpStats)
{
    pUdpStats = (PMIB_UDPSTATS)malloc(sizeof(MIB_UDPSTATS));
    assert(pUdpStats != NULL);
    return GetUdpStatistics(pUdpStats);
}

void PrintIpStats(PMIB_IPSTATS pStats)
{
    if (pStats != NULL)
    {
        printf("\nIP Statistics:\n");
        
        printf("\
  dwForwarding       = %lu\n\
  dwDefaultTTL       = %lu\n\
  dwInReceives       = %lu\n\
  dwInHdrErrors      = %lu\n\
  dwInAddrErrors     = %lu\n\
  dwForwDatagrams    = %lu\n\
  dwInUnknownProtos  = %lu\n\
  dwInDiscards       = %lu\n\
  dwInDelivers       = %lu\n\
  dwOutRequests      = %lu\n\
  dwRoutingDiscards  = %lu\n\
  dwOutDiscards      = %lu\n\
  dwOutNoRoutes      = %lu\n\
  dwReasmTimeout     = %lu\n\
  dwReasmReqds       = %lu\n\
  dwReasmOks         = %lu\n\
  dwReasmFails       = %lu\n\
  dwFragOks          = %lu\n\
  dwFragFails        = %lu\n\
  dwFragCreates      = %lu\n\
  dwNumIf            = %lu\n\
  dwNumAddr          = %lu\n\
  dwNumRoutes        = %lu\n",
                  pStats->dwForwarding,
                  pStats->dwDefaultTTL,
                  pStats->dwInReceives,
                  pStats->dwInHdrErrors,
                  pStats->dwInAddrErrors,
                  pStats->dwForwDatagrams,
                  pStats->dwInUnknownProtos,
                  pStats->dwInDiscards,
                  pStats->dwInDelivers,
                  pStats->dwOutRequests,
                  pStats->dwRoutingDiscards,
                  pStats->dwOutDiscards,
                  pStats->dwOutNoRoutes,
                  pStats->dwReasmTimeout,
                  pStats->dwReasmReqds,
                  pStats->dwReasmOks,
                  pStats->dwReasmFails,
                  pStats->dwFragOks,
                  pStats->dwFragFails,
                  pStats->dwFragCreates,
                  pStats->dwNumIf,
                  pStats->dwNumAddr,
                  pStats->dwNumRoutes);
    }

}

void PrintIcmpStats(MIBICMPINFO *pStats)
{
    if (pStats != NULL)
    {
        printf("\n%20s %10s %10s\n","ICMP Statistics", "IN", "OUT");
        printf("%20s %10s %10s\n","---------------", "------", "------");
        printf("%20s %10lu %10lu\n", "dwMsgs", pStats->icmpInStats.dwMsgs, pStats->icmpOutStats.dwMsgs);
        printf("%20s %10lu %10lu\n", "dwErrors", pStats->icmpInStats.dwErrors, pStats->icmpOutStats.dwErrors);
        printf("%20s %10lu %10lu\n", "dwDestUnreachs", pStats->icmpInStats.dwDestUnreachs, pStats->icmpOutStats.dwDestUnreachs);
        printf("%20s %10lu %10lu\n", "dwTimeExcds", pStats->icmpInStats.dwTimeExcds, pStats->icmpOutStats.dwTimeExcds);
        printf("%20s %10lu %10lu\n", "dwParmProbs", pStats->icmpInStats.dwParmProbs, pStats->icmpOutStats.dwParmProbs);
        printf("%20s %10lu %10lu\n", "dwSrcQuenchs", pStats->icmpInStats.dwSrcQuenchs, pStats->icmpOutStats.dwSrcQuenchs);
        printf("%20s %10lu %10lu\n", "dwRedirects", pStats->icmpInStats.dwRedirects, pStats->icmpOutStats.dwRedirects);
        printf("%20s %10lu %10lu\n", "dwEchos", pStats->icmpInStats.dwEchos, pStats->icmpOutStats.dwEchos);
        printf("%20s %10lu %10lu\n", "dwEchoReps", pStats->icmpInStats.dwEchoReps, pStats->icmpOutStats.dwEchoReps);
        printf("%20s %10lu %10lu\n", "dwTimestamps", pStats->icmpInStats.dwTimestamps, pStats->icmpOutStats.dwTimestamps);
        printf("%20s %10lu %10lu\n", "dwTimestampReps", pStats->icmpInStats.dwTimestampReps, pStats->icmpOutStats.dwTimestampReps);
        printf("%20s %10lu %10lu\n", "dwAddrMasks", pStats->icmpInStats.dwAddrMasks, pStats->icmpOutStats.dwAddrMasks);
        printf("%20s %10lu %10lu\n", "dwAddrMaskReps", pStats->icmpInStats.dwAddrMaskReps, pStats->icmpOutStats.dwAddrMaskReps);
    }
}

void PrintTcpStats(PMIB_TCPSTATS pStats)
{
    if (pStats != NULL)
    {
        printf("\nTCP Statistics\n");
        printf("\
  dwRtoAlgorithm     = %lu\n\
  dwRtoMin           = %lu\n\
  dwRtoMax           = %lu\n\
  dwMaxConn          = %lu\n\
  dwActiveOpens      = %lu\n\
  dwPassiveOpens     = %lu\n\
  dwAttemptFails     = %lu\n\
  dwEstabResets      = %lu\n\
  dwCurrEstab        = %lu\n\
  dwInSegs           = %lu\n\
  dwOutSegs          = %lu\n\
  dwRetransSegs      = %lu\n\
  dwInErrs           = %lu\n\
  dwOutRsts          = %lu\n\
  dwNumConns         = %lu\n",
pStats->dwRtoAlgorithm,
pStats->dwRtoMin,
pStats->dwRtoMax,
pStats->dwMaxConn,
pStats->dwActiveOpens,
pStats->dwPassiveOpens,
pStats->dwAttemptFails,
pStats->dwEstabResets,
pStats->dwCurrEstab,
pStats->dwInSegs,
pStats->dwOutSegs,
pStats->dwRetransSegs,
pStats->dwInErrs,
pStats->dwOutRsts,
pStats->dwNumConns);
    }
}

void PrintUdpStats(PMIB_UDPSTATS pStats)
{
    if (pStats != NULL)
    {
        printf("\nUDP Statistics\n");
        printf("\
  dwInDatagrams      = %lu\n\
  dwNoPorts          = %lu\n\
  dwInErrors         = %lu\n\
  dwOutDatagrams     = %lu\n\
  dwNumAddrs         = %lu\n",
pStats->dwInDatagrams,
pStats->dwNoPorts,
pStats->dwInErrors,
pStats->dwOutDatagrams,
pStats->dwNumAddrs);
    }
}

