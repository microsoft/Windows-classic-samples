//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//
//  Copyright (C) Microsoft Corporation, 2008-2009
//
//  File:       iparp.h
//
//--------------------------------------------------------------------------

#ifndef HEADER_IPARP
#define HEADER_IPARP

#include <winsock2.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>
#include <mstcpip.h>
#include <stdio.h>
#include <assert.h>

#ifndef WIN_SUCCESS
#define WIN_SUCCESS(x) ((x) == NO_ERROR)
#endif //WIN_SUCCESS

//
// Forward declarations
//
void DoGetIpNetTable();
void DoSetIpNetEntry(char* pszDottedInetAddr, char* pszPhysAddr, char* pszInterface = NULL);
void DoDeleteIpNetEntry(char* pszDottedInetAddr, char* pszInterface = NULL);
DWORD MyGetIpNetTable(PMIB_IPNETTABLE& pIpNetTable, bool fOrder = TRUE);
void PrintIpNetTable(PMIB_IPNETTABLE pIpNetTable);
int StringToPhysAddr(char* szInEther,  char* szOutEther);
DWORD MyGetIpAddrTable(PMIB_IPADDRTABLE& pIpAddrTable, bool fOrder = TRUE);


#endif
