/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.
  
   Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

    TCSample.c

Abstract:

    This program implements a simple tool which plumbs in a TC Flow and Filter 
    based on the user input
    
Environment:

    User-Mode only

------------------------------------------------------------------------------*/
#pragma warning(disable:4995)  // Traffic Control was deprecated in Vista timeframe
#pragma warning(disable:4127)  // conditional expression is constant
#pragma warning(disable:4201)  // nameless struct/union

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#include <ntddndis.h>
#include <traffic.h>


//******************************************************************************
// Forward declarations
//******************************************************************************

VOID
help();


//******************************************************************************
// Global defines
//******************************************************************************

#define NOT_SPECIFIED 0xFFFF

typedef struct ifc_info
{
    HANDLE hIfc;
    HANDLE hFlow;
    HANDLE hFilter;  
} IFC_INFO, *PIFC_INFO;

typedef struct ifc_list
{
    ULONG IfcCount;
    PIFC_INFO pIfcInfo;
} IFC_LIST, *PIFC_LIST;


//******************************************************************************
// Global variables
//******************************************************************************
TCI_CLIENT_FUNC_LIST ClientHandlerList;
HANDLE hEvent;


//******************************************************************************
// Routine: 
//      Control_C_Handler
//
// Description:
//      Handles the ctrl-c event and sets the gbRunning flag to false
//      this indicates the app to exit
//            
//******************************************************************************
BOOL 
Control_C_Handler(DWORD dwCtrlType)
{
    UNREFERENCED_PARAMETER(dwCtrlType);
    printf("**Got ^C event**\n");
    SetEvent(hEvent);
    return TRUE;
}

//******************************************************************************
// Routine: 
//      ClNotifyHandler
//
// Description:
//      Empty notification handler
//            
//******************************************************************************
void 
ClNotifyHandler(HANDLE ClRegCtx, HANDLE ClIfcCtx, ULONG Event, HANDLE SubCode, ULONG BufSize, PVOID Buffer)
{
    UNREFERENCED_PARAMETER(ClRegCtx);
    UNREFERENCED_PARAMETER(ClIfcCtx);
    //
    // Notification was unexpected
    //
    printf("Unexpected notification: Event=%d, SubCode=%p, BufSize=%d, Buffer=%p", (int)Event, (void *)SubCode, (int)BufSize, Buffer);
}

//******************************************************************************
// Routine: 
//      AddTcFlows
//
// Description:
//      Add Tc Flow in pTcFlow to each interface in IfcList
//            
//******************************************************************************
BOOL
AddTcFlows(IFC_LIST IfcList, PTC_GEN_FLOW pTcFlow)
{
    UINT i;
    ULONG err;
    BOOL status = FALSE;
    PIFC_INFO pCurrentIfcInfo = IfcList.pIfcInfo;

    //
    // For each interface in the list, add a TC flow
    //
    for (i = 0; i < IfcList.IfcCount; i++)
    {
        HANDLE hFlow;

        err = TcAddFlow(pCurrentIfcInfo->hIfc,
                        0,
                        0,
                        pTcFlow,
                        &hFlow);
        if (err != NO_ERROR)
        {
            printf("TcAddFlow Failed %d\n", err);
            goto Exit;
        }

        pCurrentIfcInfo->hFlow = hFlow;
        pCurrentIfcInfo++;
    }

    status = TRUE;

Exit:
    return status;
}

//******************************************************************************
// Routine: 
//      AddTcFilters
//
// Description:
//      Add Tc Filter in pTcFilter to each interface in IfcList
//            
//******************************************************************************
BOOL
AddTcFilters(IFC_LIST IfcList, PTC_GEN_FILTER pTcFilter)
{
    UINT i;
    ULONG err;
    BOOL status = FALSE;
    PIFC_INFO pCurrentIfcInfo = IfcList.pIfcInfo;

    //
    // For each interface in the list, add TC filter on the corresponding TcFlow
    //
    for (i = 0; i < IfcList.IfcCount; i++)
    {
        HANDLE hFilter;

        err = TcAddFilter(pCurrentIfcInfo->hFlow,
                          pTcFilter,
                          &hFilter);
        if (err != NO_ERROR)
        {
            printf("TcAddFilter Failed %d\n", err);
            goto Exit;
        }

        pCurrentIfcInfo->hFilter = hFilter;
        pCurrentIfcInfo++;      
    }

    status = TRUE;

Exit:
    return status;
}

//******************************************************************************
// Routine: 
//      ClearIfcList
//
// Description:
//      Clears the IfcList and its member variables
//            
//******************************************************************************
BOOL 
ClearIfcList(PIFC_LIST pIfcList)
{
    ULONG i;

    if (!pIfcList)
    {
        return TRUE;
    }

    if (pIfcList->pIfcInfo)
    {
        //
        // Delete filter, flow and interface
        //
        PIFC_INFO pCurrentIfcInfo = pIfcList->pIfcInfo;

        for (i = 0; i < pIfcList->IfcCount; i++)
        {
            if (pCurrentIfcInfo->hFilter)
            {
                TcDeleteFilter(pCurrentIfcInfo->hFilter);
            }
            if (pCurrentIfcInfo->hFlow)
            {
                TcDeleteFlow(pCurrentIfcInfo->hFlow);
            }
            if (pCurrentIfcInfo->hIfc)
            {
                TcCloseInterface(pCurrentIfcInfo->hIfc);
            }

            pCurrentIfcInfo++;
        }

        free(pIfcList->pIfcInfo);
    }

    ZeroMemory(pIfcList, sizeof(IFC_LIST));

    return TRUE;
}

//******************************************************************************
// Routine: 
//      MakeIfcList
//
// Arguments:
//      hClient   - Handle returned by TcRegisterClient
//      pIfcList  - ptr to IfcList structure which will be populated by the function
//      
// Description:
//      The function enumerates all TC enabled interfaces. 
//      opens each TC enabled interface and stores each ifc handle in IFC_LIST struct
//      pointed to by pIfcList
//            
//******************************************************************************
BOOL
MakeIfcList(HANDLE hClient, PIFC_LIST pIfcList)
{
    BOOL status = FALSE;
    ULONG err = ERROR_INVALID_PARAMETER;

    ULONG BufferSize = 1, ActualBufferSize, RemainingBufferSize = 0;
    PTC_IFC_DESCRIPTOR pIfcBuffer = NULL, pCurrentIfc;
    PIFC_INFO pIfcInfo = NULL, pCurrentIfcInfo;
    ULONG nIfcs = 0;

    //
    // Enumerate the TC enabled interfaces
    //
    while (TRUE)
    {
        ActualBufferSize = BufferSize;
        pIfcBuffer = (PTC_IFC_DESCRIPTOR)malloc(ActualBufferSize);
        if (pIfcBuffer == NULL)
        {
            break;
        }

        err = TcEnumerateInterfaces(hClient,
                                    &ActualBufferSize,
                                    pIfcBuffer);
        if (err == ERROR_INSUFFICIENT_BUFFER)
        {
            free(pIfcBuffer);
            BufferSize *= 2;
        }
        else
        {
            break;
        }
    }

    if (err != NO_ERROR)
    {
        goto Exit;
    }

    //
    // Count the number of interfaces
    //

    pCurrentIfc = pIfcBuffer;
    RemainingBufferSize = ActualBufferSize;
    while(RemainingBufferSize)
    {
        nIfcs++;

        RemainingBufferSize -= pCurrentIfc->Length;
        pCurrentIfc = (PTC_IFC_DESCRIPTOR)(((PBYTE)pCurrentIfc) + pCurrentIfc->Length);
    }
  
    if (nIfcs == 0)
    {
        goto Exit;
    }

    //
    // Allocate memory for the size(IFC_INFO) X nIfcs
    // 
    pIfcInfo = (PIFC_INFO)malloc(sizeof(IFC_INFO) * nIfcs);
    if (!pIfcInfo)
    {
        goto Exit;
    }
  
    ZeroMemory(pIfcInfo, sizeof(IFC_INFO) * nIfcs);

    ClearIfcList(pIfcList);
    pIfcList->IfcCount = nIfcs;
    pIfcList->pIfcInfo = pIfcInfo;
  
    //
    // Open Each interface and store the ifc handle in ifcList
    //

    pCurrentIfc = pIfcBuffer;
    pCurrentIfcInfo = pIfcInfo;

    RemainingBufferSize = ActualBufferSize;
    while (RemainingBufferSize)
    {
        HANDLE hIfc;

        err = TcOpenInterfaceW(pCurrentIfc->pInterfaceName,
                               hClient,
                               0,
                               &hIfc);
        if (err != NO_ERROR)
        {
            printf("TcOpenInterface Failed %d\n", err);
            break;
        }

        pCurrentIfcInfo->hIfc = hIfc;

        RemainingBufferSize -= pCurrentIfc->Length;
        pCurrentIfc = (PTC_IFC_DESCRIPTOR)(((PBYTE)pCurrentIfc) + pCurrentIfc->Length);
        pCurrentIfcInfo++;
    }
  
    if (err != NO_ERROR)
    {
        goto Exit;
    }

    status = TRUE;

Exit:
    if (!status)
    {
        ClearIfcList(pIfcList);
    }
   
    //
    // Cleanup the IfcBuffer
    //
    if (pIfcBuffer)
    {
        free(pIfcBuffer);
    }

    return status;
}

//******************************************************************************
// Routine: 
//      DeleteFlow
//
// Description:
//      Deletes the flow and its member variables
//            
//******************************************************************************
BOOL 
DeleteFlow(PTC_GEN_FLOW *pFlow)
{
    if (pFlow == NULL || *pFlow == NULL)
    {
        return TRUE;
    }

    free(*pFlow);
    *pFlow = NULL;

    return TRUE;
}

//******************************************************************************
// Routine: 
//      CreateFlow
//
// Arguments:
//      ppTcFlowObj  - double ptr to Flow struct in which the function returns the flow
//      DSCPValue    - dscp value for the flow
//      OnePValue    - 802.1p value for the flow
//      ThrottleRate - throttle rate for the flow
//
// Description:
//      The function returns a tc flow in ppTcFlowObj on success 
//            
//******************************************************************************
BOOL 
CreateFlow(PTC_GEN_FLOW *_ppTcFlowObj, USHORT DSCPValue, USHORT OnePValue, ULONG ThrottleRate)
{
    BOOL status = FALSE;

    //
    // Flow Parameters
    //
    ULONG TokenRate = QOS_NOT_SPECIFIED;
    ULONG TokenBucketSize = QOS_NOT_SPECIFIED;
    ULONG PeakBandwidth = QOS_NOT_SPECIFIED;
    ULONG Latency = QOS_NOT_SPECIFIED;
    ULONG DelayVariation = QOS_NOT_SPECIFIED;
    SERVICETYPE ServiceType = SERVICETYPE_BESTEFFORT;
    ULONG MaxSduSize = QOS_NOT_SPECIFIED;
    ULONG MinimumPolicedSize = QOS_NOT_SPECIFIED;

    PVOID pCurrentObject;
    PTC_GEN_FLOW _pTcFlowObj = NULL;

    int Length = 0;
  
    //
    // Calculate the memory size required for the optional TC objects
    //
    Length += (OnePValue == NOT_SPECIFIED ? 0 : sizeof(QOS_TRAFFIC_CLASS)) + 
              (DSCPValue == NOT_SPECIFIED ? 0 : sizeof(QOS_DS_CLASS));

    //
    // Print the Flow parameters
    //
    printf("Flow Parameters:\n");
    DSCPValue == NOT_SPECIFIED ? printf("\tDSCP: *\n"):printf("\tDSCP: %u\n", DSCPValue);
    OnePValue == NOT_SPECIFIED ? printf("\t802.1p: *\n"):printf("\t802.1p: %u\n", OnePValue);
    if (ThrottleRate == QOS_NOT_SPECIFIED) 
    {
        printf("\tThrottleRate: *\n");
        printf("\tServiceType: Best effort\n");
    }
    else 
    {
        printf("\tThrottleRate: %u\n", ThrottleRate);
        printf("\tServiceType: Guaranteed\n");
        ServiceType = SERVICETYPE_GUARANTEED;
    }

    TokenRate = TokenBucketSize = ThrottleRate;

    //
    // Allocate the flow descriptor
    //
    _pTcFlowObj = (PTC_GEN_FLOW)malloc(FIELD_OFFSET(TC_GEN_FLOW, TcObjects) + Length);
    if (!_pTcFlowObj) 
    {
        printf("Flow Allocation Failed\n");
        goto Exit;
    }

    _pTcFlowObj->SendingFlowspec.TokenRate = TokenRate;
    _pTcFlowObj->SendingFlowspec.TokenBucketSize = TokenBucketSize;
    _pTcFlowObj->SendingFlowspec.PeakBandwidth = PeakBandwidth;
    _pTcFlowObj->SendingFlowspec.Latency = Latency;
    _pTcFlowObj->SendingFlowspec.DelayVariation = DelayVariation;
    _pTcFlowObj->SendingFlowspec.ServiceType = ServiceType;
    _pTcFlowObj->SendingFlowspec.MaxSduSize = MaxSduSize;
    _pTcFlowObj->SendingFlowspec.MinimumPolicedSize = MinimumPolicedSize;

    memcpy(&(_pTcFlowObj->ReceivingFlowspec), &(_pTcFlowObj->SendingFlowspec), sizeof(_pTcFlowObj->ReceivingFlowspec));

    _pTcFlowObj->TcObjectsLength = Length;

    //
    // Add any requested objects
    //
    pCurrentObject = (PVOID)_pTcFlowObj->TcObjects;

    if (OnePValue != NOT_SPECIFIED)
    {
        QOS_TRAFFIC_CLASS *pTClassObject = (QOS_TRAFFIC_CLASS*)pCurrentObject;
        pTClassObject->ObjectHdr.ObjectType = QOS_OBJECT_TRAFFIC_CLASS;
        pTClassObject->ObjectHdr.ObjectLength = sizeof(QOS_TRAFFIC_CLASS);
        pTClassObject->TrafficClass = OnePValue; //802.1p tag to be used

        pCurrentObject = (PVOID)(pTClassObject + 1);
    }

    if (DSCPValue != NOT_SPECIFIED)
    {
        QOS_DS_CLASS *pDSClassObject = (QOS_DS_CLASS*)pCurrentObject;
        pDSClassObject->ObjectHdr.ObjectType = QOS_OBJECT_DS_CLASS;
        pDSClassObject->ObjectHdr.ObjectLength = sizeof(QOS_DS_CLASS);
        pDSClassObject->DSField = DSCPValue; //Services Type
    }

    DeleteFlow(_ppTcFlowObj);
    *_ppTcFlowObj = _pTcFlowObj;

    status = TRUE;

 Exit:
    if (!status)
    {
        printf("Flow Creation Failed\n");
        DeleteFlow(&_pTcFlowObj);
    }
    else
    {
        printf("Flow Creation Succeeded\n");
    }

    return status;
}

//******************************************************************************
// Routine: 
//      DeleteFilter
//
// Description:
//      Deletes the filter and its member variables
//            
//******************************************************************************
BOOL 
DeleteFilter(PTC_GEN_FILTER *ppFilter)
{
    PTC_GEN_FILTER pFilter;

    if (ppFilter == NULL || *ppFilter == NULL)
    {
        return TRUE;
    }

    pFilter = (*ppFilter);

    if (pFilter->Pattern)
    {
        free(pFilter->Pattern);
    }

    if (pFilter->Mask)
    {
        free(pFilter->Mask);
    }

    *ppFilter = NULL;

    return TRUE;
}

//******************************************************************************
// Routine: 
//      PrintAddress
//
// Description:
//      The function prints out the address contained in the SOCKADDR_STORAGE
//            
//******************************************************************************
VOID 
PrintAddress(PSOCKADDR_STORAGE pSocketAddress)
{  
    DWORD cbBuffer = 2000;
    WCHAR buffer[2000];
    SOCKADDR_STORAGE socketAddress;
    ULONG Length;

    if (!pSocketAddress)
    {
        return;
    }

    ZeroMemory(buffer, sizeof(buffer));
    ZeroMemory(&socketAddress, sizeof(socketAddress));

    //
    // If the address is empty, print *
    //
    if (!memcmp(((PBYTE)&socketAddress) + sizeof(USHORT),  
                ((PBYTE)pSocketAddress) + sizeof(USHORT), 
                sizeof(socketAddress) - sizeof(USHORT)))
    {
        printf("\tAddress: *\n");
        return;
    }

    Length = (pSocketAddress->ss_family == AF_INET ? sizeof(SOCKADDR_IN) : sizeof(SOCKADDR_IN6));
    if (SOCKET_ERROR == WSAAddressToStringW((LPSOCKADDR)(pSocketAddress),
                                            Length,
                                            NULL,
                                            buffer,
                                            &cbBuffer))
    {
        printf("WSAAddressToStringW() failed with 0x%x\n", WSAGetLastError());
        buffer[0] = L'\0';
    }

    printf("\tAddress: %S\n", buffer);
}

//******************************************************************************
// Routine: 
//      CreateFilter
//
// Arguments:
//      ppFilter   - double ptr to Filter struct in which the function returns the filter
//      Address    - destination address of the outgoing packets of interest.
//      Port       - destination port of the outgoing packets of interest.
//      ProtocolId - protocol of the outgoing packets of interest.
//
// Description:
//      The function returns a tc filter in ppFilter on success 
//            
//******************************************************************************
BOOL 
CreateFilter(PTC_GEN_FILTER *ppFilter, SOCKADDR_STORAGE Address, USHORT Port, UCHAR ProtocolId)
{  
    BOOL status = FALSE;
    USHORT AddressFamily = Address.ss_family;
    PTC_GEN_FILTER pFilter = NULL;
    PIP_PATTERN pPattern = NULL;
    PIP_PATTERN pMask = NULL;

    //
    // Print out the Filter Parameters
    //
    printf("Filter Parameters:\n");
    PrintAddress(&Address);
    printf("\tDest Port: %u\n", Port);
    printf("\tProtocol: ");
    switch (ProtocolId)
    {
        case IPPROTO_IP:
        {
            printf("IP\n");
            break;
        }
        case IPPROTO_TCP:
        {
            printf("TCP\n");
            break;
        }
        case IPPROTO_UDP:
        {
            printf("UDP\n");
            break;
        }
        default:
        {
            printf("Invalid Protocol\n");
            break;
        }
    };
  
    if (AddressFamily != AF_INET)
    {
        goto Exit;
    }
  
    //
    // Allocate memory for the filter
    //
    pFilter = (PTC_GEN_FILTER)malloc(sizeof(TC_GEN_FILTER));
    if (!pFilter)
    {
        printf("Error, No memory for filter\n");
        goto Exit;
    }
    ZeroMemory(pFilter, sizeof(TC_GEN_FILTER));
      
    //
    // Allocate memory for the pattern and mask
    //
    pPattern = (PIP_PATTERN)malloc(sizeof(IP_PATTERN));
    pMask    = (PIP_PATTERN)malloc(sizeof(IP_PATTERN));

    if (!pPattern || !pMask)
    {
        goto Exit;
    }
  
    memset(pPattern, 0, sizeof(IP_PATTERN));
  
    pPattern->DstAddr = ((SOCKADDR_IN *)&Address)->sin_addr.s_addr;
    pPattern->tcDstPort = htons(Port);
    pPattern->ProtocolId = ProtocolId;

    memset(pMask, (ULONG)-1, sizeof(IP_PATTERN));
    //
    // Set the source address and port to wildcard
    // 0 -> wildcard, 0xFF-> exact match 
    //
    pMask->SrcAddr = 0;
    pMask->tcSrcPort = 0;
  
    //
    // If the user specified 0 for dest port, dest address or protocol
    // set the appropriate mask as wildcard
    // 0 -> wildcard, 0xFF-> exact match 
    //

    if (pPattern->tcDstPort == 0)
    {
        pMask->tcDstPort = 0;
    }

    if (pPattern->ProtocolId == 0)
    {
        pMask->ProtocolId = 0;
    }

    if (pPattern->DstAddr == 0)
    {
        pMask->DstAddr = 0;
    }

    pFilter->AddressType = NDIS_PROTOCOL_ID_TCP_IP;
    pFilter->PatternSize = sizeof(IP_PATTERN);
    pFilter->Pattern = pPattern;
    pFilter->Mask = pMask;
  
    //
    // Delete any previous instances of the Filter
    //
    DeleteFilter(ppFilter);
    *ppFilter = pFilter;

    status = TRUE;

Exit:
    if (!status)
    {
        printf("Filter Creation Failed\n");
        DeleteFilter(&pFilter);
    }
    else
    {
        printf("Filter Creation Succeeded\n");
    }

    return status; 
}

//******************************************************************************
// Routine: 
//      GetSockAddrFromString
//
// Arguments:
//      strAddress     - Address in the String format
//      pSocketAddress - Pointer to SOCKADDR_STORAGE structure where the 
//      address is returned
//
// Description:
//      Takes a string format address and returns a pointer to 
//      SOCKADDR_STORAGE structure containing the address.
//      Only resolves numeric addresses
//******************************************************************************
BOOL
GetSockAddrFromString(const WCHAR *strAddress, PSOCKADDR_STORAGE pSocketAddress)
{
    BOOL result = FALSE;    
    ADDRINFOW *pAddress = NULL;
    int wsaResult = 0;
    ADDRINFOW hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_NUMERICHOST;

    // Resolve the address
    wsaResult = GetAddrInfoW(strAddress, NULL, &hints, &pAddress);
    if (0 != wsaResult)
    {
        printf("Failed to resolve \"%S\". GetAddrInfoW() failed with 0x%x\n", strAddress, wsaResult);
        goto Exit;
    }

    // Make sure the address resolved to only one entry
    if (NULL != pAddress->ai_next)
    {
        printf("%S Resolved to more than one address. Please use a numeric Address\n", strAddress);
        goto Exit;
    }

    // Copy the address to the storage
    CopyMemory(pSocketAddress, pAddress->ai_addr, pAddress->ai_addrlen);

    result = TRUE;

Exit:
    // Cleanup
    if (NULL != pAddress)
    {
        
        FreeAddrInfoW(pAddress);
    }

    return result;
}

//******************************************************************************
// Routine: 
//      ParseCommandline
//
// Arguments:
//      argv      - Commandline Parameters
//      pTcFlow   - Pointer to PTC_GEN_FLOW structure.
//      pTcFilter - Pointer to PTC_GEN_FILTER structure.
//
// Description:
//      Parses the commandline and populates creates TC_GEN_FLOW and 
//      TC_GEN_FILTER structures with the appropriate values.
//******************************************************************************
BOOL
ParseCommandLine(WCHAR **argv, PTC_GEN_FLOW *pTcFlow, PTC_GEN_FILTER *pTcFilter)
{
    BOOL status = FALSE;

    UCHAR ProtocolId = 0;
    USHORT Port = 0;
    USHORT DSCPValue = NOT_SPECIFIED, OnePValue = NOT_SPECIFIED;
    ULONG ThrottleRate = QOS_NOT_SPECIFIED;

    PWCHAR strAddress = L"0";
    SOCKADDR_STORAGE Address = { 0 };

    Address.ss_family = AF_INET;

    //
    // Extract commandline parameters and do some basic validation
    //
    argv++;
    while (*argv)
    {
        if (!_wcsicmp(*argv, L"-proto"))
        {
            argv++;
            if (!(*argv))
            {
                goto Exit;
            }

            if (!_wcsicmp(*(argv), L"tcp"))
            {
                ProtocolId = IPPROTO_TCP;
            }
            else if (!_wcsicmp(*(argv), L"udp"))
            {
                ProtocolId = IPPROTO_UDP;
            }
            else if (!_wcsicmp(*(argv), L"ip"))
            {
                ProtocolId = IPPROTO_IP;
            }
            else 
            {
                printf("Invalid Protocol\n");
                goto Exit;
            }
        }
        else if (!_wcsicmp(*argv, L"-destip"))
        {
            argv++;
            if (!(*argv))
            {
                goto Exit;
            }

            strAddress = (*argv);
        }
        else if (!_wcsicmp(*argv, L"-destport"))
        {
            argv++;
            if (!(*argv))
            {
                goto Exit;
            }

            Port = (USHORT)_wtoi(*(argv));
        }
        else if (!_wcsicmp(*argv, L"-dscp"))
        {
            argv++;
            if (!(*argv))
            {
                goto Exit;
            }

            DSCPValue = (USHORT)_wtoi(*(argv));
            if (DSCPValue < 0 || DSCPValue > 63)
            {
                printf("Invalid DSCP Value\n");
                goto Exit;
            }
        }
        else if (!_wcsicmp(*argv, L"-onep"))
        {
            argv++;
            if (!(*argv))
            {
                goto Exit;
            }

            OnePValue = (USHORT)_wtoi(*(argv));
            if (OnePValue < 0 || OnePValue > 7)
            {
                printf("Invalid 802.1p Value\n");
                goto Exit;
            }            
        }
        else if (!_wcsicmp(*argv, L"-throttle"))
        {
            argv++;
            if (!(*argv))
            {
                goto Exit;
            }

            ThrottleRate = (ULONG)_wtoi(*(argv));
        }
        else
        {
            goto Exit;
        }

        argv++;
    }

    //
    // if dest addr is not specified or specified as zero
    // treat it as wildcard, dont try to resolve it
    //
    if (_wtoi(strAddress) != 0)
    {
        if (!GetSockAddrFromString(strAddress, &Address))
        {
            goto Exit;
        }
    }

    //
    // Create the TC Flow with the parameters
    //
    if (!CreateFlow(pTcFlow, DSCPValue, OnePValue, ThrottleRate))
    {
        goto Exit;
    }

    //
    // Create the TC Filter with the parameters
    //
    if (!CreateFilter(pTcFilter, Address, Port, ProtocolId))
    {
        goto Exit;
    }

    status = TRUE;

Exit:
    if (!status)
    {
        printf("Invalid Argument(s)\n");
    }

    return status;
}

//******************************************************************************
// Routine: 
//      main
//
// Description:
//      Entry point. Parses the commandline and calls CreateFilter and CreateFlow
//      with appropriate parameters.
//******************************************************************************
int _cdecl wmain(int argc, WCHAR *argv[])
{
    BOOL status = FALSE;
    ULONG err;
    int wsaResult; 

    //
    // TC Structures
    //
    PTC_GEN_FLOW pTcFlow = NULL;
    PTC_GEN_FILTER pTcFilter = NULL;
    HANDLE hClient = TC_INVALID_HANDLE;

    IFC_LIST IfcList = { 0 };
    WSADATA wsaData;

    UNREFERENCED_PARAMETER(argc);

#if !defined (_WIN64) && _WIN32_WINNT >= 0x0600
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
#endif

    //
    // Start Winsock
    //
    wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaResult != 0)
    {
        printf("Winsock Startup Failed\n");
        help();
        return 0;
    }
  
    //
    // Parse the Commandline and populate the TC_GEN_FILTER and TC_GEN_FLOW structures
    //
    if (!ParseCommandLine(argv, &pTcFlow, &pTcFilter))
    {
        goto Exit;
    }

    //
    // Register TC client
    //
    memset(&ClientHandlerList, 0, sizeof(ClientHandlerList));
    ClientHandlerList.ClNotifyHandler = (TCI_NOTIFY_HANDLER)ClNotifyHandler;
    err = TcRegisterClient(CURRENT_TCI_VERSION,
                           0,
                           &ClientHandlerList,
                           &hClient);
    if (err != NO_ERROR)
    {
        printf("TcRegisterClient Failed %d\n", err);

        if (err == ERROR_OPEN_FAILED)
        {
            printf("Please make sure you are running with admin credentials\n");
        }

        goto Exit;
    }
  
    //
    // Enumerate All TC enabled Interfaces and 
    // store the information in IfcList
    //
    if (!MakeIfcList(hClient, &IfcList))
    {
        printf("Error reading interface list, make sure QoS Packet Scheduler is active for this interface\n");
        goto Exit;
    }
    printf("Interface list created...\n");
  
    //
    // Add pTcFlow on all the Ifcs in the IfcList
    //
    if (!AddTcFlows(IfcList, pTcFlow))
    {
        printf("Error adding flows\n");
        goto Exit;
    }
    printf("Flows added...\n");
  
    //
    // Add pTcFilter to all the corresponding TcFlows
    // on all the Ifcs in the IfcList
    //
    if (!AddTcFilters(IfcList, pTcFilter))
    {
        printf("Error adding filter...\n");
        goto Exit;
    }
    printf("Filter added...\n");

    //
    // Enable the Ctrl-C handler
    //  
    hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!hEvent)
    {
        printf("Failed to create event\n");
        goto Exit;
    }
  
    printf("**Hit ^C to Exit**\n");

    SetConsoleCtrlHandler((PHANDLER_ROUTINE)Control_C_Handler, TRUE);
    WaitForSingleObject(hEvent, INFINITE);
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)Control_C_Handler, FALSE);

    status = TRUE;

Exit: 
    if (!status)
    {
        help();
    }
  
    //
    // Cleanup
    //
    ClearIfcList(&IfcList);
    DeleteFilter(&pTcFilter);
    DeleteFlow(&pTcFlow);

    TcDeregisterClient(hClient);

    WSACleanup();

    if (hEvent)
    {
        CloseHandle(hEvent);
    }

    return 0;
} 

//******************************************************************************
// Routine: 
//      help
//
// Description:
//      This routine prints out the usage information for the program
//
//******************************************************************************
VOID
help()
{
    printf("\nUsage: tcsample <optional parameters>\n");
    printf("\t-proto   : protocol(tcp/udp/ip) (Default: All IP Protocols)\n");
    printf("\t-destip  : destination IP Address (Default: All IP Addresses)\n");
    printf("\t-destport: destination port number (Default: All Ports)\n");
    printf("\t-dscp    : dscp value to tag matching packets (Default: No DSCP Override)\n");
    printf("\t-onep    : 802.1p value to tag matching packets (Default: No 802.1p Override)\n");
    printf("\t-throttle: throttle rate(Bps) to throttle matching packets (Default: No Throttling)\n");
    printf("\nExample: tcsample -destip 192.168.1.10 -proto ip -dscp 40\n");
    printf("will result in all outgoing IP Packets destined to\n"); 
    printf("192.168.1.10 to be marked with DSCP value 40\n");
} 
