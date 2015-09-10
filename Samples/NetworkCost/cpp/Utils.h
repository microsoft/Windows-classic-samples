// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <stdio.h>
#include <atlbase.h>
#include <oaidl.h>
#include <netlistmgr.h>
#include <ws2tcpip.h>
#include <Mstcpip.h>
#include <Iphlpapi.h>
#include <ws2def.h>
#include <ws2ipdef.h>
#include <Winsock2.h>
#include <wchar.h>
#include <string.h>

#define CHOICE_EXIT 10
#define IP_ADDRESS_SIZE 256
#define SOCKET_VERSION_REQUESTED MAKEWORD(2,2)

//
// Structure to store destination address string and converted numeric IP address
//
struct DESTINATION_INFO
{
    WCHAR               addrString[IP_ADDRESS_SIZE];
    NLM_SOCKADDR        ipAddr;
};

//
// Clears any input lingering in the STDIN buffer
//
void FlushCurrentLine();

//
// Maps common HRESULTs to descriptive error strings
//
void DisplayError(_In_ HRESULT hr);

//
// Converts file time to local time, to display to the user
//
void PrintFileTime(_In_ FILETIME time);

//
// Description: Gets the interface type for each connection
//
void GetInterfaceType (_In_ GUID interfaceGUID, _In_ HRESULT hr);

//
// Description: Checks if the data plan status values are default values, or provided by the MNO
//
BOOL IsDataPlanStatusAvailable(_In_ const NLM_DATAPLAN_STATUS *pDataPlanStatus);

//
// Description: Converts Destination address string to IP address
//
HRESULT ConvertStringToSockAddr(_In_ LPWSTR destIPaddr, _Out_ SOCKADDR_STORAGE *socketAddress);

//
// Description: Gets the connection type from the connection GUID
//
HRESULT GetConnectionFromGUID(_In_ INetworkListManager *pManager, _In_ GUID connID, _Out_ INetworkConnection **ppConnection);

//
// Description: Gets Destination String and converts it to socket address
//
HRESULT GetDestinationAddress(_Out_ DESTINATION_INFO *pDestIPAddr);

//
// Description: Displays meaningful cost values to the user
//
void DisplayCostDescription (_In_ DWORD cost);

//
// Description: Displays data plan status to the user
//
void DisplayDataPlanStatus (_In_ const NLM_DATAPLAN_STATUS *pDataPlanStatus);

//
//  Description:This function sorts a list of Ipv4 & Ipv6 addresses, and returns the "best" address that stack determines
//
DWORD GetPreferredAddress(_In_ DWORD numElement, 
                          _In_reads_(numElement) SOCKADDR_IN6 *pAddrList, 
                          _Out_ SOCKADDR_STORAGE *pPreferredAddr);
