/*
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
 * ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 * Copyright ©  Microsoft Corporation.  All Rights Reserved.
 *
 * Author: Raunak Pandya
 * Abstract:
 *    Dump statistics across all the V4 scopes on a DHCP server.
 *
 */
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <winsock.h>
#include <dhcpsapi.h>

#define SERVERNAME_BUF_SIZE 255
#define IPADDR_BUF_SIZE 36

DWORD __cdecl main(int argc, char* argv[])
{
	LPDHCP_MIB_INFO_V5 MibInfoV5 = NULL;
	DWORD error = ERROR_SUCCESS;

	if(2 != argc)
	{
        wprintf(L"Usage: DhcpServerScopeStatsV4.exe <Server IpAdd/Name>\n");
		return ERROR_INVALID_PARAMETER;
	}
    WCHAR szServer[SERVERNAME_BUF_SIZE] = {0};
    MultiByteToWideChar(0, 0, argv[1], (int)strlen(argv[1]), szServer, SERVERNAME_BUF_SIZE);      

	// Call DhcpGetMibInfo to get the get the statistics across all the v4 scopes on the DHCP server.
	error = DhcpGetMibInfoV5(szServer, &MibInfoV5);
	if(ERROR_SUCCESS != error)
	{
		wprintf(L"DhcpGetMibInfoV5 returned with error: %d", error);
		if(MibInfoV5)
		{
			DhcpRpcFreeMemory(MibInfoV5);
			MibInfoV5 = NULL;
		}

		return error;
	}

	wprintf(L"Discovers:		%d \n", MibInfoV5->Discovers);
	wprintf(L"Offers:			%d \n", MibInfoV5->Offers);
	wprintf(L"Delayed Offers:		%d \n", MibInfoV5->DelayedOffers);
	wprintf(L"Requests:		%d \n", MibInfoV5->Requests);
	wprintf(L"Acks:			%d \n", MibInfoV5->Acks);
	wprintf(L"Naks:			%d \n", MibInfoV5->Naks);
	wprintf(L"Declines:		%d \n", MibInfoV5->Declines);
	wprintf(L"Releases:		%d \n", MibInfoV5->Releases);
	wprintf(L"Scopes:			%d \n", MibInfoV5->Scopes);
	wprintf(L"Scopes with delay configured: %d \n", MibInfoV5->ScopesWithDelayedOffers);
	
	// Now dump the scope level information
	wprintf(L"\n----Scope Level Information----\n");
	for(DWORD i = 0; i < MibInfoV5->Scopes; i++)
	{
		IN_ADDR scopeAddress = {0};
		scopeAddress.S_un.S_addr = ntohl((ULONG)MibInfoV5->ScopeInfo[i].Subnet);
		char* strScopeAddress = inet_ntoa(scopeAddress);
        
        	WCHAR szScopeAddr[IPADDR_BUF_SIZE] = {0};
        	MultiByteToWideChar(0, 0, strScopeAddress, (int)strlen(strScopeAddress), szScopeAddr, IPADDR_BUF_SIZE);      

		wprintf(L"\nSubnet:	%s \n", szScopeAddr);
		wprintf(L"\t No. of addresses in use:	%d \n", MibInfoV5->ScopeInfo[i].NumAddressesInuse);
		wprintf(L"\t No. of free addresses:		%d \n", MibInfoV5->ScopeInfo[i].NumAddressesFree);
		wprintf(L"\t No. of pending offers:		%d \n", MibInfoV5->ScopeInfo[i].NumPendingOffers);
	}

	if(MibInfoV5)
	{
		DhcpRpcFreeMemory(MibInfoV5);
		MibInfoV5 = NULL;
	}

	return 0;
}