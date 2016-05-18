#ifndef _IPROUTE_H
#define _IPROUTE_H

#include <windows.h>
#include <winsock.h>
#include <iphlpapi.h>
#include <assert.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <iptypes.h>

void DoGetIpForwardTable();
void DoSetIpForwardEntry(char* pszDest, char* pszNetMask, char* pszGateway, char* pszInterface, DWORD dwMetric = 1);
void DoDeleteIpForwardEntry(char* pszDest);
bool InterfaceIdxToInterfaceIp(PMIB_IPADDRTABLE pIpAddrTable, DWORD dwIndex, char str[]);
bool InterfaceIpToIdxAndMask(PMIB_IPADDRTABLE pIpAddrTable, char str[], DWORD& dwIndex, DWORD& dwMask);
DWORD MyGetIpAddrTable(PMIB_IPADDRTABLE& pIpAddrTable, BOOL fOrder = FALSE);
DWORD MyGetIpForwardTable(PMIB_IPFORWARDTABLE& pIpRouteTab, BOOL fOrder = FALSE);
void PrintIpForwardTable(PMIB_IPFORWARDTABLE pIpRouteTable);
#endif