// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Utils.h"
#include <new>

//********************************************************************************************
// Function: FlushCurrentLine
//
// Description: Clears any input lingering in the STDIN buffer
//
//********************************************************************************************

void FlushCurrentLine()
{
    int i;
    while ((i = getc(stdin)) != EOF && i != '\n')
    {
        continue;
    }
}

//********************************************************************************************
//  Function: GetPreferredAddress
//
//  Description: This function sorts a list of Ipv4 & Ipv6 addresses, and returns the "best" address that stack determines
//
//********************************************************************************************

DWORD GetPreferredAddress(_In_ DWORD numElement, 
                          _In_reads_(numElement) SOCKADDR_IN6 *pAddrList, 
                          _Out_ SOCKADDR_STORAGE *pPreferredAddr)
{
    DWORD dwErr = ERROR_SUCCESS;
    WORD wVersionRequested = MAKEWORD(2,2);
    WSADATA wsaData = {0};
    SOCKET socketIoctl = INVALID_SOCKET;
    SOCKET_ADDRESS_LIST *pSocketAddrList = NULL;
    SOCKADDR_IN6 *pBestAddress = NULL;
    DWORD dwSize = 0;
    DWORD dwBytes = 0;
    DWORD i = 0;
    BOOL fWSAStarted = FALSE;

    ZeroMemory (pPreferredAddr, sizeof(SOCKADDR_STORAGE));

    //This do--while(FALSE) loop is used to break in between based on error conditions
    do
    {
        // Initialize WinSock 
        dwErr = WSAStartup(wVersionRequested, &wsaData);
        if (dwErr != ERROR_SUCCESS)
        {
            wprintf(L"WSAStartup failed, (dwErr = %#x).", dwErr);
            break;
        }

        fWSAStarted = TRUE;
            
        // create socket
        socketIoctl = WSASocket(AF_INET6, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
        if (socketIoctl == INVALID_SOCKET)
        {
            dwErr = WSAGetLastError();
            wprintf(L"WSASocket failed, (dwErr = %#x).", dwErr);
            break;
        }

        dwSize = SIZEOF_SOCKET_ADDRESS_LIST(numElement);        
        pSocketAddrList = reinterpret_cast<SOCKET_ADDRESS_LIST *>(new BYTE[dwSize]);
        if (pSocketAddrList == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            wprintf(L"out of memory, (dwErr = %#x).", dwErr);
            break;
        }
        
        for (i = 0; i < numElement; i++ )
        {
            pSocketAddrList->Address[i].lpSockaddr = (LPSOCKADDR)(pAddrList + i);
            pSocketAddrList->Address[i].iSockaddrLength = sizeof(SOCKADDR_IN6);
        }
    
        pSocketAddrList->iAddressCount = numElement;

        // sort addresses
        dwErr = WSAIoctl(
                    socketIoctl,
                    SIO_ADDRESS_LIST_SORT,
                    (LPVOID)pSocketAddrList,
                    dwSize,
                    (LPVOID)pSocketAddrList,
                    dwSize,
                    &dwBytes,
                    NULL,
                    NULL
                    );
        
        if (dwErr == SOCKET_ERROR)
        {
            dwErr = WSAGetLastError();
            wprintf(L"WSAIoctl sort address failed, (dwErr = %#x).", dwErr);
            break;
        }

        pBestAddress = reinterpret_cast<SOCKADDR_IN6 *>(pSocketAddrList->Address[0].lpSockaddr);

        //If mapped IPv6, retrieve the original IPv4 dest address
        if (IN6ADDR_ISV4MAPPED(pBestAddress))
        {
            pPreferredAddr->ss_family = AF_INET;
            (reinterpret_cast<SOCKADDR_IN *> (pPreferredAddr))->sin_addr = *((PIN_ADDR) IN6_GET_ADDR_V4MAPPED(&(pBestAddress->sin6_addr)));
            (reinterpret_cast<SOCKADDR_IN *> (pPreferredAddr))->sin_port = pBestAddress->sin6_port;
        }
        else
        {
            pPreferredAddr->ss_family = AF_INET6;
            (reinterpret_cast<SOCKADDR_IN6 *> (pPreferredAddr))->sin6_addr = pBestAddress->sin6_addr;
            (reinterpret_cast<SOCKADDR_IN6 *> (pPreferredAddr))->sin6_scope_id = pBestAddress->sin6_scope_id;
            (reinterpret_cast<SOCKADDR_IN6 *> (pPreferredAddr))->sin6_port = pBestAddress->sin6_port;
        }
    }while(FALSE);
    
    if (socketIoctl != INVALID_SOCKET)
    {
        closesocket(socketIoctl);
        socketIoctl = INVALID_SOCKET;
    }

    if (fWSAStarted)
    {
        WSACleanup();
    }

    if (pSocketAddrList)
    {
        delete[] reinterpret_cast<BYTE *>(pSocketAddrList);
    }
    return dwErr;
}


//********************************************************************************************
// Function: ConvertStringToSockAddr
//
// Description: Converts destination hostname or URL to IP address
//
//********************************************************************************************

HRESULT ConvertStringToSockAddr(_In_ LPWSTR destIPAddr, _Out_ SOCKADDR_STORAGE *socketAddress)
{
    HRESULT hr = S_OK;
    DWORD dwErr = NO_ERROR;
    struct addrinfoW *result = NULL;
    struct addrinfoW *ptr = NULL;
    WSADATA wsaData;
    int count = 0;
    int addrCount = 0;
    SOCKADDR_IN6 *destAddrList = NULL;
    SOCKADDR_IN *pDestSockAddrIn = NULL;
    BOOL fWSAStarted = FALSE;

    //This do--while(FALSE) loop is used to break in between based on error conditions
    do
    {
        //Intialize socket, to use GetAddrInfoW()
        dwErr = WSAStartup (SOCKET_VERSION_REQUESTED,&wsaData);
        if (dwErr != NO_ERROR)
        {
            hr = HRESULT_FROM_WIN32(dwErr);
            break;
        }
        fWSAStarted = TRUE;
        //get Destination IP address from Destination address string hostname or URL
        dwErr = GetAddrInfoW(destIPAddr,NULL,NULL,&result);
        if (dwErr == NO_ERROR)
        {
            //Get the number of socket addresses returned by GetAddrInfoW
            for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
            {
                addrCount ++;
            }
            destAddrList = new (std::nothrow) SOCKADDR_IN6[addrCount]; 
            ZeroMemory (destAddrList, (sizeof(SOCKADDR_IN6) * addrCount));
            for (ptr = result; ((ptr != NULL) && (count<addrCount)) ; ptr = ptr->ai_next)
            {
                if (ptr->ai_family == AF_INET)
                {
                    //Map IPv4 to mapped IPV6 to sort consistent IPv6 list using CreateSortedAddressPairs()
                    pDestSockAddrIn = reinterpret_cast<SOCKADDR_IN *>(ptr->ai_addr);
                    IN6ADDR_SETV4MAPPED(&(destAddrList[count]),reinterpret_cast<IN_ADDR *>(&(pDestSockAddrIn->sin_addr)),
                        IN4ADDR_SCOPE_ID(pDestSockAddrIn), pDestSockAddrIn->sin_port);
                }
                else if (ptr->ai_family == AF_INET6)
                {
                    
                    destAddrList[count] = *(reinterpret_cast<SOCKADDR_IN6 *>(result->ai_addr));
                }
                else
                {
                    hr = E_UNEXPECTED;
                    wprintf(L"Unsupported IP family, please enter IPv4 or IPv6 destination\n");
                    break;
                }
                count++;
            }
            
            // determine the preferred IP address
            dwErr = GetPreferredAddress(addrCount,
                                        destAddrList,
                                        socketAddress);
            if (dwErr != NO_ERROR)
            {
                wprintf(L"WSAIoctl failed, (dwErr = %#x).", dwErr );
                hr = HRESULT_FROM_WIN32(dwErr);
            }            
        }
        else
        {
            hr = HRESULT_FROM_WIN32(dwErr);
        }
    }while (FALSE);
    if (fWSAStarted)
    {
        WSACleanup();
    }

    pDestSockAddrIn = NULL;
    delete [] destAddrList;
    ptr = NULL;
    FreeAddrInfoW(result);
    DisplayError(hr);
    return hr;
}


//********************************************************************************************
// Function: GetInterfaceType
//
// Description: Gets the interface type for each connection
//
//********************************************************************************************

void GetInterfaceType (_In_ GUID interfaceGUID, _In_ HRESULT hr)
{   
    NET_LUID interfaceLUID = {0};
    MIB_IF_ROW2 mib;
    if (hr == S_OK)
    {
        // Get interface LUID 
        DWORD dwError = ConvertInterfaceGuidToLuid(&interfaceGUID,&interfaceLUID);
        if (dwError == NO_ERROR)
        {
            //Get interface info entry
            mib.InterfaceLuid = interfaceLUID;
            dwError= GetIfEntry2(&mib);
            if (dwError == NO_ERROR)
            {
                // Get interface type
                wprintf(L"Connection Interface   : %ws\n", mib.Description);
            }
        }
        hr = HRESULT_FROM_WIN32(dwError);
    }
    DisplayError(hr);
}  
                


//********************************************************************************************
// Function: IsDataPlanStatusAvailable
//
// Description: Checks if the data plan status values are default values, or provided by the MNO
//
//********************************************************************************************
BOOL IsDataPlanStatusAvailable(_In_ const NLM_DATAPLAN_STATUS *pDataPlanStatus)
{
    BOOL isAvailable = FALSE;
    //
    // usage data is valid only if both planUsage and lastUpdatedTime are valid
    //
    if (pDataPlanStatus->UsageData.UsageInMegabytes != NLM_UNKNOWN_DATAPLAN_STATUS
        && (pDataPlanStatus->UsageData.LastSyncTime.dwHighDateTime != 0 
            || pDataPlanStatus->UsageData.LastSyncTime.dwLowDateTime != 0))
    {
        isAvailable = TRUE;
    }
    else if (pDataPlanStatus->DataLimitInMegabytes != NLM_UNKNOWN_DATAPLAN_STATUS)
    {
        isAvailable = TRUE;
    }
    else if (pDataPlanStatus->InboundBandwidthInKbps != NLM_UNKNOWN_DATAPLAN_STATUS)
    {
        isAvailable = TRUE;
    }
    else if (pDataPlanStatus->OutboundBandwidthInKbps != NLM_UNKNOWN_DATAPLAN_STATUS)
    {
        isAvailable = TRUE;
    }
    else if (pDataPlanStatus->NextBillingCycle.dwHighDateTime != 0 
            || pDataPlanStatus->NextBillingCycle.dwLowDateTime != 0)
    {
        isAvailable = TRUE;

    }
    else if (pDataPlanStatus->MaxTransferSizeInMegabytes != NLM_UNKNOWN_DATAPLAN_STATUS)
    {
        isAvailable = TRUE;
    }
    return isAvailable;
}

//********************************************************************************************
// Function: DisplayCost
//
// Description: Displays meaningful cost values to the user
//
//********************************************************************************************

void DisplayCostDescription (_In_ DWORD cost)
{
    if (cost == NLM_CONNECTION_COST_UNKNOWN)
    {
        wprintf(L"Cost                  : Unknown\n");
    }
    
    else if (cost & NLM_CONNECTION_COST_UNRESTRICTED)
    {
        wprintf(L"Cost                  : Unrestricted\n");
    }
    
    else if (cost & NLM_CONNECTION_COST_FIXED)
    {
        wprintf(L"Cost                  : Fixed\n");
    }
    
    else if (cost & NLM_CONNECTION_COST_VARIABLE)
    {
        wprintf(L"Cost                  : Variable\n");
    }
    
    if (cost & NLM_CONNECTION_COST_OVERDATALIMIT)
    {
        wprintf(L"OverDataLimit         : Yes\n");
    }

    else
    {
        wprintf(L"OverDataLimit         : No\n");
    }

    if (cost & NLM_CONNECTION_COST_APPROACHINGDATALIMIT)
    {
        wprintf(L"Approaching DataLimit : Yes\n");
    }

    else
    {
        wprintf(L"Approaching DataLimit : No\n");
    }
    
    if (cost & NLM_CONNECTION_COST_CONGESTED)
    {
        wprintf(L"Congested             : Yes\n");
    }

    else
    {
        wprintf(L"Congested             : No\n");
    }
    
    if (cost & NLM_CONNECTION_COST_ROAMING)
    {
        wprintf(L"Roaming               : Yes\n");
    }   

    else
    {
        wprintf(L"Roaming               : No\n");
    }
}

//********************************************************************************************
// Function: DisplayDataPlanStatus
//
// Description: Displays data plan status values to the user
//
//********************************************************************************************

void DisplayDataPlanStatus (_In_ const NLM_DATAPLAN_STATUS *pDataPlanStatus)
{
    if (FALSE == IsDataPlanStatusAvailable(pDataPlanStatus))
    {
        wprintf(L"Plan Data usage unknown\n");
    }
    else
    {
        WCHAR szGuid[39]={0};
        StringFromGUID2(pDataPlanStatus->InterfaceGuid, szGuid, 39);
        wprintf(L"Interface ID                              :   %s\n",szGuid);
        
        //check for default or unknown value
        if (pDataPlanStatus->UsageData.UsageInMegabytes != NLM_UNKNOWN_DATAPLAN_STATUS)
        {
            wprintf(L"Data Usage in Megabytes                   :   %d\n", pDataPlanStatus->UsageData.UsageInMegabytes);
        }

        if ((pDataPlanStatus->UsageData.LastSyncTime.dwHighDateTime != 0) || (pDataPlanStatus->UsageData.LastSyncTime.dwLowDateTime != 0))
        {
            wprintf(L"Data Usage Synced Time                    :   ");
            PrintFileTime (pDataPlanStatus->UsageData.LastSyncTime);
        }
        if (pDataPlanStatus->DataLimitInMegabytes != NLM_UNKNOWN_DATAPLAN_STATUS)
        {
            wprintf(L"Data Limit in Megabytes                   :   %d\n", pDataPlanStatus->DataLimitInMegabytes);                             
        }
        if (pDataPlanStatus->InboundBandwidthInKbps != NLM_UNKNOWN_DATAPLAN_STATUS)
        {
            wprintf(L"Inbound Bandwidth in Kbps                         :   %d\n", pDataPlanStatus->InboundBandwidthInKbps);
        }
        if (pDataPlanStatus->OutboundBandwidthInKbps != NLM_UNKNOWN_DATAPLAN_STATUS)
        {
            wprintf(L"Outbound Bandwidth in Kbps                         :   %d\n", pDataPlanStatus->OutboundBandwidthInKbps);
        }
        if ((pDataPlanStatus->NextBillingCycle.dwHighDateTime != 0) || (pDataPlanStatus->NextBillingCycle.dwLowDateTime != 0))
        {
            wprintf(L"Next Billing Cycle                        :   ");
            PrintFileTime (pDataPlanStatus->NextBillingCycle);
        }   
        if (pDataPlanStatus->MaxTransferSizeInMegabytes != NLM_UNKNOWN_DATAPLAN_STATUS)
        {
            wprintf(L"Maximum Transfer Size in Megabytes   :   %d\n", pDataPlanStatus->MaxTransferSizeInMegabytes);
        }   
    }

}



//********************************************************************************************
// Function: PrintFileTime
//
// Description: Converts file time to local time, to display to the user
//
//********************************************************************************************

void PrintFileTime(_In_ FILETIME time)
{
    SYSTEMTIME stUTC, stLocal;

    // Convert filetime to local time.
    FileTimeToSystemTime(&time, &stUTC);
    SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);      
    wprintf(L"%02d/%02d/%d  %02d:%02d:%02d\n", 
           stLocal.wMonth, stLocal.wDay, stLocal.wYear,stLocal.wHour, stLocal.wMinute, stLocal.wSecond);
}     


//********************************************************************************************
// Function: DisplayError
//
// Description: Maps common HRESULTs to descriptive error strings
//
//********************************************************************************************

void DisplayError(_In_ HRESULT hr)
{

    struct errorDescription
    {
        HRESULT hr;
        PWSTR description;
    } validErrors[] = 
    {
      {E_OUTOFMEMORY,                           L"Error: Failed to allocate necessary memory"},
      {E_NOINTERFACE,                           L"Error: Network cost API is only supported on WIN8 client machine."},
      {E_POINTER,                               L"Error: Pointer is not valid"},
      {E_PENDING,                               L"Error: In the process of determining cost. If you already registered for cost change, notification will be raised when cost change is determined"},
      {HRESULT_FROM_WIN32(ERROR_NO_NETWORK),    L"Error: Machine might have lost connectivity, No network found"},     
    };
    if (hr != S_OK)
    {
        for(int eindex=0;
            eindex < sizeof(validErrors) / sizeof(struct errorDescription);
            eindex++ )
        {
            if (hr == validErrors[eindex].hr)
            {
                wprintf(L"%s\n",validErrors[eindex].description);
                return;
            }
        }
        wprintf(L"Error Code: 0x%x\n",hr);
    }
}

//********************************************************************************************
// Function: GetConnectionFromGUID
//
// Description: Gets the connection type from the connection GUID
//
//********************************************************************************************

HRESULT GetConnectionFromGUID(_In_ INetworkListManager *pManager, _In_ GUID connID, _Out_ INetworkConnection **ppConnection)
{
    HRESULT hr = S_OK;
    BOOL bFound = FALSE;
    BOOL bDone = FALSE;
    ULONG cFetched = 0;
    CComPtr<IEnumNetworkConnections> pNetworkConnections;
    GUID guid;
    *ppConnection = NULL;
    
    hr = pManager->GetNetworkConnections(&pNetworkConnections);  
    if (SUCCEEDED(hr))
    {
        while (!bDone)
        {
            CComPtr<INetworkConnection> pConnection;
            hr = pNetworkConnections->Next(1, &pConnection, &cFetched);
            if (SUCCEEDED(hr) && cFetched > 0)
            {
                hr = pConnection->GetConnectionId(&guid);
                if (SUCCEEDED(hr))
                {
                    if (IsEqualGUID(guid,connID))
                    {
                        *ppConnection = pConnection;
                        (*ppConnection)->AddRef(); 
                        bFound = TRUE;
                        break;
                    }
                }
            } 
            else
            {
                bDone = TRUE;
            }
        }
        if (!bFound)
        {
            hr = E_FAIL;
        }
    }
    return hr;
}

//********************************************************************************************
// Function: GetDestinationAddress
//
// Description: Gets Destination String and converts it to socket address
//
//********************************************************************************************
HRESULT GetDestinationAddress(_Out_ DESTINATION_INFO *pDestIPAddr)
{
    HRESULT hr;
    WCHAR destAddress[IP_ADDRESS_SIZE] = {0};
    NLM_SOCKADDR destSocketAddress = {0};
    ZeroMemory (pDestIPAddr, sizeof(DESTINATION_INFO));
    
    //The feature allow registration for multiple destination addresses, the sample SDK restricts to one Destination address
    wprintf(L"Please enter the destination address :\n");
    wscanf_s(L"%s",destAddress,IP_ADDRESS_SIZE-1);
    //convert destination addr string to numeric IP address
    hr = ConvertStringToSockAddr(destAddress, reinterpret_cast<SOCKADDR_STORAGE *>(&destSocketAddress));
    if (hr == S_OK)
    {
        //enter the destination addr string and the numeric IP address to structure, to retrieve the string later for display info
        wmemcpy(pDestIPAddr->addrString, destAddress, IP_ADDRESS_SIZE);
        pDestIPAddr->ipAddr = destSocketAddress;
    }
    return hr;
}

