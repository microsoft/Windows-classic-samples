//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//
//  Copyright (C) Microsoft Corporation, 2008-2009
//
//  File:       ipstat.h
//
//--------------------------------------------------------------------------

#ifndef HEADER_IPSTAT
#define HEADER_IPSTAT

#include <winsock2.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>
#include <mstcpip.h>
#include <stdio.h>
#include <assert.h>

#ifndef WIN_SUCCESS
#define WIN_SUCCESS(x) ((x) == NO_ERROR)
#endif //WIN_SUCCESS

#define MAX_STRLEN 255

//
// Forward declarations
//
void DoGetConnTable(char* pszProto);
void DoGetStat(char* pszProto = NULL);
DWORD MyGetTcpTable(PMIB_TCPTABLE& pTcpTable, BOOL fOrder);
void DumpTcpTable(PMIB_TCPTABLE pTcpTable);
DWORD MyGetUdpTable(PMIB_UDPTABLE& pUdpTable, BOOL fOrder);
void DumpUdpTable(PMIB_UDPTABLE pUdpTable);
DWORD MyGetIpStatistics(PMIB_IPSTATS& pIpStats);
DWORD MyGetIcmpStatistics(PMIB_ICMP& pIcmpStats);
DWORD MyGetTcpStatistics(PMIB_TCPSTATS& pTcpStats);
DWORD MyGetUdpStatistics(PMIB_UDPSTATS& pUdpStats);
void PrintIpStats(PMIB_IPSTATS pStats);
void PrintIcmpStats(MIBICMPINFO *pStats);
void PrintTcpStats(PMIB_TCPSTATS pStats);
void PrintUdpStats(PMIB_UDPSTATS pStats);



#endif
