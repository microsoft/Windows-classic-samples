// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Abstract:
//
//      This module implements a device for the FD provider Sample
//
//---------------------------------------------------------------------------

#include "stdafx.h"

struct TRecvTreadInfo
{
    SOCKET hDiscoverySocket;
    HANDLE hStopEvent;
    GUID* pDeviceId;
    TDeviceInfo* pDeviceInfo;
};  // SRecvTreadInfo

VOID PrintUsage()
{
    printf("FDProviderSampleDevice.exe /Switch Value </Switch Value>\n\n");
    printf("The following switches are supported:\n");
    printf("  DeviceId        (Required) a UUID that represents the unique identifier for the device.\n");
    printf("  DeviceCategory  (Required) Identies the type of device\n");
    printf("  FriendlyName    (Required) Name that users will see\n");
    printf("  HardwareId      (Required) PnP HardwareId for the device.\n");
    printf("  Manufacturer    (Optional) Manufacturer name.\n");
    printf("  ManufacturerUrl (Optional) Manufacturer;s web page\n");
    printf("  ModelName       (Optional) Model name\n");
    printf("  ModelNumber     (Optional) Model number\n");
    printf("  ModelUrl        (Optional) Model Url\n");
    printf("  Upc             (Optional) UPC code for the device\n");
    printf("  FirmwareVersion (Optional) Device's firware version\n");
    printf("  SerialNumber    (Optional) Device's serial number\n");
    printf("  PresentationUrl (Optional) Device's internal web page for administration\n\n");
    printf("Example:\n");
    printf("  FDProviderSampleDevice /DeviceId e3fa675d-3b7f-4894-99c5-9b1348100037 /DeviceCategory \"MFP,Printers,Scanners\" /FriendlyName \"FD sample device 1\" /HardwareId \"SampleHardwareId\" /FirmwareVersion \"2.3.400.57-1 E3\"\n");
}  // PrintUsage

VOID PrintStartInfo(
    PCWSTR pszDeviceId,
    const TDeviceInfo& DeviceInfo)
{
    // Print the device configuration
    printf("Starting FD Sample device with properties:\n");
    printf("  DeviceId:        %S\n", pszDeviceId);
    printf("  DeviceCategory:  %S\n", DeviceInfo.szDeviceCategory);
    printf("  FriendlyName:    %S\n", DeviceInfo.szFriendlyName);
    printf("  HardwareId:      %S\n", DeviceInfo.szPnPHardwareId);
    printf("  Manufacturer:    %S\n", DeviceInfo.szManufacturer);
    printf("  ManufacturerUrl: %S\n", DeviceInfo.szManufacturerUrl);
    printf("  ModelName:       %S\n", DeviceInfo.szModelName);
    printf("  ModelNumber:     %S\n", DeviceInfo.szModelNumber);
    printf("  ModelUrl:        %S\n", DeviceInfo.szModelUrl);
    printf("  UPC:             %S\n", DeviceInfo.szUpc);
    printf("  FirmwareVersion: %S\n", DeviceInfo.szFirmwareVersion);
    printf("  SerialNumber:    %S\n\n", DeviceInfo.szSerialNumber);
    printf("  PresentationUrl: %S\n\n", DeviceInfo.szPresentationUrl);
    
    // Print key map
    printf("Press\n");
    printf("  q : Quit\n");
    printf("  h : Send Hello\n");
    printf("  b : Send Bye\n");
}  // PrintStartInfo

int ParseCMDLine(
    int argc,
    __in_ecount(argc) PWSTR argv[], 
    __out PWSTR& pszDeviceId,
    __out GUID& DeviceId,
    __out TDeviceInfo& DeviceInfo)
{
    RPC_STATUS retVal = RPC_S_OK;

    for (int i = 1; i < argc - 1; i += 2)
    {
        if (_wcsicmp(L"/?", argv[i]) == 0)
        {
            PrintUsage();
            return 1;
        }
        else if (_wcsicmp(L"/DeviceId", argv[i]) == 0)
        {
            pszDeviceId = argv[i+1];
            retVal = UuidFromString((RPC_WSTR) pszDeviceId, &DeviceId);
            if (RPC_S_OK != retVal)
            {
                printf("DeviceId was not a valid UUID\n");
                return 1;
            }
        }
        else if (_wcsicmp(L"/DeviceCategory", argv[i]) == 0)
        {
            StringCbCopy(DeviceInfo.szDeviceCategory, sizeof(DeviceInfo.szDeviceCategory), argv[i+1]);
        }
        else if (_wcsicmp(L"/FriendlyName", argv[i]) == 0)
        {
            StringCbCopy(DeviceInfo.szFriendlyName, sizeof(DeviceInfo.szFriendlyName), argv[i+1]);
        }
        else if (_wcsicmp(L"/HardwareId", argv[i]) == 0)
        {
            StringCbCopy(DeviceInfo.szPnPHardwareId, sizeof(DeviceInfo.szPnPHardwareId), argv[i+1]);
        }
        else if (_wcsicmp(L"/Manufacturer", argv[i]) == 0)
        {
            StringCbCopy(DeviceInfo.szManufacturer, sizeof(DeviceInfo.szManufacturer), argv[i+1]);
        }
        else if (_wcsicmp(L"/ManufacturerUrl", argv[i]) == 0)
        {
            StringCbCopy(DeviceInfo.szManufacturerUrl, sizeof(DeviceInfo.szManufacturerUrl), argv[i+1]);
        }
        else if (_wcsicmp(L"/ModelName", argv[i]) == 0)
        {
            StringCbCopy(DeviceInfo.szModelName, sizeof(DeviceInfo.szModelName), argv[i+1]);
        }
        else if (_wcsicmp(L"/ModelNumber", argv[i]) == 0)
        {
            StringCbCopy(DeviceInfo.szModelNumber, sizeof(DeviceInfo.szModelNumber), argv[i+1]);
        }
        else if (_wcsicmp(L"/ModelUrl", argv[i]) == 0)
        {
            StringCbCopy(DeviceInfo.szModelUrl, sizeof(DeviceInfo.szModelUrl), argv[i+1]);
        }
        else if (_wcsicmp(L"/Upc", argv[i]) == 0)
        {
            StringCbCopy(DeviceInfo.szUpc, sizeof(DeviceInfo.szUpc), argv[i+1]);
        }
        else if (_wcsicmp(L"/FirmwareVersion", argv[i]) == 0)
        {
            StringCbCopy(DeviceInfo.szFirmwareVersion, sizeof(DeviceInfo.szFirmwareVersion), argv[i+1]);
        }
        else if (_wcsicmp(L"/SerialNumber", argv[i]) == 0)
        {
            StringCbCopy(DeviceInfo.szSerialNumber, sizeof(DeviceInfo.szSerialNumber), argv[i+1]);
        }
        else if (_wcsicmp(L"/PresentationUrl", argv[i]) == 0)
        {
            StringCbCopy(DeviceInfo.szPresentationUrl, sizeof(DeviceInfo.szPresentationUrl), argv[i+1]);
        }
        else
        {
            printf("Unrecognized command line parameter %S\n", argv[i]);

            PrintUsage();

            return 1;
        }
    }

    // Check that the required parameters were supplied
    if (   !pszDeviceId
        || !*DeviceInfo.szDeviceCategory
        || !*DeviceInfo.szFriendlyName
        || !*DeviceInfo.szPnPHardwareId)
    {
        printf("All required switches were not supplied\n\n");

        PrintUsage();

        return 1;
    }

    return 0;
}  // ParseCMDLine

ULONG GetDefaultMulticastInterfaceIndex(
    ULONG AddressFamily, 
    __out ULONG& InterfaceIndex)
{
    ULONG RetVal = 0;
    PIP_ADAPTER_ADDRESSES pAdapterAddresses = NULL;
    PIP_ADAPTER_ADDRESSES pCurrentAdapter = NULL;
    ULONG BufSize = 0;
    ULONG i = 0;

    assert((   (AF_INET == AddressFamily)
            || (AF_INET6 == AddressFamily)));
    
    for (i = 0; i < 5; ++i)
    {
        RetVal =  GetAdaptersAddresses(
            AddressFamily, 
            GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_FRIENDLY_NAME | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_UNICAST,
            NULL, 
            pAdapterAddresses, 
            &BufSize);
        
        if (RetVal != ERROR_BUFFER_OVERFLOW) 
        {
            break;
        }

        if (pAdapterAddresses)
        {
            free(pAdapterAddresses);
        }
        
        pAdapterAddresses = (PIP_ADAPTER_ADDRESSES) malloc(BufSize);
        if (!pAdapterAddresses) 
        {
            RetVal = ERROR_OUTOFMEMORY;
        }
    }

    if (NO_ERROR == RetVal)
    {
        // Got the list of adapters.
        // Now find the first interface that is up and supports multicast
        pCurrentAdapter = pAdapterAddresses;
        while (pCurrentAdapter)
        {
            if (   (IfOperStatusUp == pCurrentAdapter->OperStatus)  // Adapter is operational
                && ((IP_ADAPTER_NO_MULTICAST & pCurrentAdapter->Flags) == 0))  // Adapter is multicast capable
            {
                break;
            }

            pCurrentAdapter = pCurrentAdapter->Next;
        }

        if (pCurrentAdapter)
        {
            if (AF_INET == AddressFamily)
            {
                InterfaceIndex = pCurrentAdapter->IfIndex;
            }
            else
            {
                InterfaceIndex = pCurrentAdapter->Ipv6IfIndex;
            }
        }
        else
        {
            RetVal = ERROR_INVALID_FUNCTION;
        }
    }

    // Cleanup
    if (pAdapterAddresses)
    {
        free(pAdapterAddresses);
    }

    return RetVal;
}  // GetDefaultMulticastInterfaceIndex


int JoinMulticastGroupAndSetSocketOptions(
    SOCKET hDiscoverySocket, 
    __in PADDRINFOW pMulticastAddrInfo,
    __in PADDRINFOW pLocalAddrInfo)
{
    int err = 0;
    BOOL fEnabled = TRUE;
    BOOL fDisabled = FALSE;
    int sendRecvBufSize = 64 * 1024;
    ULONG cByteCount = 0;
    ULONG InterfaceIndex = 0;

    // Set SO_EXCLUSIVEADDRUSE and SO_REUSEADDR for the socket 
    // so that there can be multiple listeners on the same machine
    err = setsockopt(
        hDiscoverySocket, 
        SOL_SOCKET, 
        SO_EXCLUSIVEADDRUSE,
        (char*) &fDisabled,
        sizeof(fDisabled));
    if (0 != err)
    {
        err = WSAGetLastError();
        printf("setsockopt SO_EXCLUSIVEADDRUSE failed: %d\n", err);
        goto Cleanup;
    }
    err = setsockopt(
        hDiscoverySocket, 
        SOL_SOCKET, 
        SO_REUSEADDR,
        (char*) &fEnabled,
        sizeof(fEnabled));
    if (0 != err)
    {
        err = WSAGetLastError();
        printf("setsockopt SO_REUSEADDR failed: %d\n", err);
        goto Cleanup;
    }

    // Disable connection reset for UDP
    err = WSAIoctl(
        hDiscoverySocket,
        SIO_UDP_CONNRESET,
        &fDisabled,
        sizeof(fDisabled),
        NULL,
        0,
        &cByteCount,
        NULL,
        NULL);
    if (0 != err)
    {
        err = WSAGetLastError();
        printf("WSAIoctl SIO_UDP_CONNRESET failed: %d\n", err);
        goto Cleanup;
    }

    // Adjust the send and receive buffer size
    err = setsockopt(
        hDiscoverySocket,
        SOL_SOCKET,
        SO_RCVBUF,
        (char*) &sendRecvBufSize,
        sizeof(sendRecvBufSize));
    if (0 != err)
    {
        err = WSAGetLastError();
        printf("setsockopt SO_RCVBUF failed: %d\n", err);
        goto Cleanup;
    }
    err = setsockopt(
        hDiscoverySocket,
        SOL_SOCKET,
        SO_SNDBUF,
        (char*) &sendRecvBufSize,
        sizeof(sendRecvBufSize));
    if (0 != err)
    {
        err = WSAGetLastError();
        printf("setsockopt SO_SNDBUF failed: %d\n", err);
        goto Cleanup;
    }

    err = GetDefaultMulticastInterfaceIndex(
        pMulticastAddrInfo->ai_family,
        InterfaceIndex);
    if (0 != err)
    {
        printf("GetDefaultMulticastInterfaceIndex failed.  Error %u\n", err);
        goto Cleanup;
    }

    if (AF_INET == pMulticastAddrInfo->ai_family)  // IPv4
    {
        IP_MREQ mreq = {0};

        // Join the multicast group
        mreq.imr_multiaddr = ((PSOCKADDR_IN)pMulticastAddrInfo->ai_addr)->sin_addr;  // Set the multicast address
        memcpy(&mreq.imr_interface, &InterfaceIndex, sizeof(mreq.imr_interface));   // Set the interface index

        err = setsockopt(
            hDiscoverySocket,
            IPPROTO_IP,
            IP_ADD_MEMBERSHIP,
            (char*) &mreq,
            sizeof(mreq));
        
        if (0 != err)
        {
            err = WSAGetLastError();
            printf("setsockopt IP_ADD_MEMBERSHIP failed.  Error %u\n", err);
            goto Cleanup;
        }

        // Set the outgoing multicast interface
        err = setsockopt(
            hDiscoverySocket,
            IPPROTO_IP,
            IP_MULTICAST_IF,
            (char*) &InterfaceIndex,
            sizeof(InterfaceIndex));
        if (0 != err)
        {
            err = WSAGetLastError();
            printf("setsockopt IP_MULTICAST_IF failed.  Error %u\n", err);
            goto Cleanup;
        }
    }
    else // IPv6
    {
        IPV6_MREQ mreq6 = {0};

        // Join the multicast group
        mreq6.ipv6mr_multiaddr = ((PSOCKADDR_IN6) pMulticastAddrInfo->ai_addr)->sin6_addr;  // Set the multicast address
        mreq6.ipv6mr_interface = InterfaceIndex;  

        err = setsockopt(
            hDiscoverySocket,
            IPPROTO_IPV6,
            IPV6_ADD_MEMBERSHIP,
            (char*) &mreq6,
            sizeof(mreq6));
        
        if (0 != err)
        {
            err = WSAGetLastError();
            printf("setsockopt IPV6_ADD_MEMBERSHIP failed.  Error %u\n", err);
            goto Cleanup;
        }

        // Set the outgoing multicast interface
        err = setsockopt(
            hDiscoverySocket,
            IPPROTO_IPV6,
            IPV6_MULTICAST_IF,
            (char*) &InterfaceIndex,
            sizeof(InterfaceIndex));
        if (0 != err)
        {
            err = WSAGetLastError();
            printf("setsockopt IPV6_MULTICAST_IF failed.  Error %u\n", err);
            goto Cleanup;
        }
    }

Cleanup:

    return err;
}  // JoinMulticastGroup

DWORD __stdcall RecvThread(
    __in PVOID pParameter)
{
    DWORD RetVal = 0;
    TRecvTreadInfo* pRecvThreadInfo = (TRecvTreadInfo*) pParameter;
    HANDLE hSelectEvent = NULL;
    HANDLE hWaitHandles[2] = {0};
    DWORD WaitSignal = 0;
    ULONG cByteCount = 0;
    SOCKADDR_STORAGE FromAddr = {0};
    int FromAddrLen = 0;
    TQueryMessage* pQueryMessage = NULL;
    TReplyMessage ReplyMessage;

    // Initialize random number generation on this thread.
    srand((unsigned) time(NULL));

    // Initilize the Reply message
    ReplyMessage.MessageType = Reply;
    ReplyMessage.DeviceId = *(pRecvThreadInfo->pDeviceId);
    ReplyMessage.DeviceInfo = *(pRecvThreadInfo->pDeviceInfo);

    // Allocate a message recv buffer
    pQueryMessage = (TQueryMessage*) malloc(MaxMessageSize);
    if (!pQueryMessage)
    {
        RetVal = ERROR_OUTOFMEMORY;
        printf("Out of memory.  Messages will not be received.\n");
        goto Cleanup;
    }

    // Create select event
    hSelectEvent = WSACreateEvent();
    if (!hSelectEvent)
    {
        RetVal = WSAGetLastError();
        printf("Failed to create hSelectEvent.  Messages will not be received. Error %u\n", RetVal);
        goto Cleanup;
    }

    // Register socket for select events
    RetVal = WSAEventSelect(
        pRecvThreadInfo->hDiscoverySocket,
        hSelectEvent,
        FD_READ);

    if (0 != RetVal)
    {
        RetVal = WSAGetLastError();
        printf("WSAEventSelect.  Messages will not be received. Error %u\n", RetVal);
        goto Cleanup;
    }

    hWaitHandles[0] = pRecvThreadInfo->hStopEvent;
    hWaitHandles[1] = hSelectEvent;

    while (true)
    {
        RetVal = 0;

        // Wait for a message to arrive or shutdown to be signaled
        WaitSignal = WaitForMultipleObjects(
            ARRAYSIZE(hWaitHandles),
            hWaitHandles,
            FALSE,
            INFINITE);

        // These values should never occur
        if (   (WaitSignal == WAIT_TIMEOUT )
            || (WaitSignal == WAIT_ABANDONED_0)
            || (WaitSignal == WAIT_ABANDONED_0 + 1))
        {
            printf("Unexpected wait result.  Continue.\n");

            // Reset the event
            WSAResetEvent(hSelectEvent);
            continue;
        }

        if (WAIT_OBJECT_0 == WaitSignal)
        {
            // Stop event has been signaled
            break;
        }

        //
        // A message has been received, process it
        // 
        ZeroMemory(&FromAddr, sizeof(FromAddr));
        FromAddrLen = sizeof(FromAddr);
        cByteCount = recvfrom(
            pRecvThreadInfo->hDiscoverySocket,
            (char *) pQueryMessage,
            MaxMessageSize,
            0,
            (sockaddr*) &FromAddr,
            &FromAddrLen);

        if (SOCKET_ERROR != cByteCount)
        {
            // Process message if it is a Query message
            if (   (sizeof(TQueryMessage) == cByteCount)
                && (pQueryMessage->MessageType == Query))
            {
                // Respond if 
                // the device category is not specified 
                // or the same as our device criteria
                if (   (L'\0' == pQueryMessage->szDeviceCategory[0]) 
                    || (wcscmp(pQueryMessage->szDeviceCategory, pRecvThreadInfo->pDeviceInfo->szDeviceCategory) == 0))
                {
                    // back off a bit so we don't overwelm the client if there are many devices.
                    Sleep(rand() % 100);

                    cByteCount = sendto(
                        pRecvThreadInfo->hDiscoverySocket,
                        (char*) &ReplyMessage,
                        sizeof(ReplyMessage),
                        0,
                        (sockaddr*) &FromAddr,
                        FromAddrLen);

                    if (0 != cByteCount)
                    {
                        printf("Reply sent.\n");
                    }
                    else
                    {
                        RetVal = WSAGetLastError();
                        printf("send failed. Error %u\n", RetVal);
                    }
                }
            }
        }
        else
        {
            RetVal = WSAGetLastError();
            printf("recv failed. Error %u\n", RetVal);
        }

        // Reset the event
        WSAResetEvent(hSelectEvent);
    }

Cleanup:
    if (pQueryMessage)
    {
        free(pQueryMessage);
    }
    if (hSelectEvent)
    {
        WSACloseEvent(hSelectEvent);
    }

    return RetVal;
}  // RecvThread

VOID StopRecv(
    HANDLE hStopEvent, 
    HANDLE hRecvThread)
{
    // Signal the Recv thread to stop.
    if (!SetEvent(hStopEvent))
    {
        printf("Failed to set hStopEvent.  Error: %u\n", GetLastError());
    }

    // Wait for the thread to end.
    if (WaitForSingleObject(hRecvThread, INFINITE) != WAIT_OBJECT_0)
    {
        printf("WaitForSingleObject failed\n");
    }
}  // StopRecv

VOID SendHello(
    SOCKET hDiscoverySocket, 
    __in GUID& DeviceId,
    __in TDeviceInfo& DeviceInfo,
    __in PADDRINFOW pMulticastAddrInfo)
{
    int cByteCount = 0;
    THelloMessage HelloMessage;

    HelloMessage.MessageType = Hello;
    HelloMessage.DeviceId = DeviceId;
    HelloMessage.DeviceInfo = DeviceInfo;

    cByteCount = sendto(
        hDiscoverySocket,
        (char*) &HelloMessage,
        sizeof(HelloMessage),
        0,
        pMulticastAddrInfo->ai_addr,
        (int) pMulticastAddrInfo->ai_addrlen);

    if (0 != cByteCount)
    {
        printf("Hello Sent\n\n");
    }
    else
    {
        printf("send failed. Error %u\n", WSAGetLastError());
    }
}  // SendHello

VOID SendBye(
    SOCKET hDiscoverySocket,
    __in GUID& DeviceId,
    __in PADDRINFOW pMulticastAddrInfo)
{
    int cByteCount = 0;
    TByeMessage ByeMessage;

    ByeMessage.MessageType = Bye;
    ByeMessage.DeviceId = DeviceId;

    cByteCount = sendto(
        hDiscoverySocket,
        (char*) &ByeMessage,
        sizeof(ByeMessage),
        0,
        pMulticastAddrInfo->ai_addr,
        (int) pMulticastAddrInfo->ai_addrlen);

    if (0 != cByteCount)
    {
        printf("Bye Sent\n\n");
    }
    else
    {
        printf("send failed. Error %u\n", WSAGetLastError());
    }
}  // SendBye

int __cdecl wmain(
    int argc, 
    __in_ecount(argc) PWSTR argv[])
{
    int err = 0;
    WSADATA wsaData = {0};
    PWSTR pszDeviceId = NULL;
    GUID sDeviceId = {0};
    TDeviceInfo DeviceInfo = {0};
    WCHAR ch = 0;
    SOCKET hDiscoverySocket = INVALID_SOCKET;
    HANDLE hStopEvent = NULL;
    HANDLE hRecvThread = NULL;
    TRecvTreadInfo RecvThreadInfo = {0};
    ADDRINFOW addrHints = {0};
    PADDRINFOW pMulticastAddrInfo = NULL;
    PADDRINFOW pLocalAddrInfo = NULL;
    
    // Parse command line parameters;
    err = ParseCMDLine(
        argc,
        argv,
        pszDeviceId,
        sDeviceId,
        DeviceInfo);
    if (0 != err)
    {
        goto Cleanup;
    }

    PrintStartInfo(pszDeviceId, DeviceInfo);

    // Initialize Winsock
    err = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (0 == err)
    {
        if (   (2 != LOBYTE(wsaData.wVersion))
            || (2 != HIBYTE(wsaData.wVersion)))
        {
            printf("Winsock 2.2 is not supported\n");
            err = 1;
            goto Cleanup;
        }
    }
    else
    {
        printf("Error initializing Winsock: %d\n", err);
        goto Cleanup;
    }

    // Create musticast socket address structure
    addrHints.ai_flags = AI_NUMERICHOST;
    addrHints.ai_family = AF_UNSPEC;
    addrHints.ai_socktype = SOCK_DGRAM;
    addrHints.ai_protocol = IPPROTO_UDP;

    err = GetAddrInfo(
        szMulticastAddress,
        szMulticastPort,
        &addrHints,
        &pMulticastAddrInfo);

    if (0 != err)
    {
        printf("GetAddrInfo failed for the multicast address: %d\n", err);
        goto Cleanup;
    }

    // Create local address to bind to
    addrHints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;
    addrHints.ai_family = pMulticastAddrInfo->ai_family;
    addrHints.ai_socktype = pMulticastAddrInfo->ai_socktype;
    addrHints.ai_protocol = pMulticastAddrInfo->ai_protocol;

    err = GetAddrInfo(
        NULL,  // local socket
        szMulticastPort,  
        &addrHints,
        &pLocalAddrInfo);

    if (0 != err)
    {
        printf("GetAddrInfo failed for the local address: %d\n", err);
        goto Cleanup;
    }

    // Create a socket to listen on
    hDiscoverySocket = socket(
        pLocalAddrInfo->ai_family,
        pLocalAddrInfo->ai_socktype,
        pLocalAddrInfo->ai_protocol);
    if (hDiscoverySocket == INVALID_SOCKET)
    {
        err = WSAGetLastError();
        printf("Failed to create socket: %d\n", err);
        goto Cleanup;
    }

    err = JoinMulticastGroupAndSetSocketOptions(
        hDiscoverySocket, 
        pMulticastAddrInfo,
        pLocalAddrInfo);

    if (0 != err)
    {
        goto Cleanup;
    }

    // Bind the socket to the local address
    err = bind(
        hDiscoverySocket,
        pLocalAddrInfo->ai_addr,
        (int) pLocalAddrInfo->ai_addrlen);

    if (0 != err)
    {
        err = WSAGetLastError();
        printf("bind failed: %d\n",err);
        goto Cleanup;
    }

    // Create hStopEvent
    hStopEvent = CreateEvent(
        NULL, 
        TRUE,
        FALSE,
        NULL);

    if (!hStopEvent)
    {
        err = GetLastError();
        printf("CreateEvent failed: %d\n", err);
        goto Cleanup;
    }

    // Start RecvThread
    RecvThreadInfo.hDiscoverySocket = hDiscoverySocket;
    RecvThreadInfo.hStopEvent = hStopEvent;
    RecvThreadInfo.pDeviceId = &sDeviceId;
    RecvThreadInfo.pDeviceInfo = &DeviceInfo;

    hRecvThread = CreateThread(
        NULL,
        0,
        RecvThread,
        &RecvThreadInfo,
        0,
        NULL);

    if (!hRecvThread)
    {
        err = GetLastError();
        printf("CreateThread failed: %d\n", err);
        goto Cleanup;
    }

    // Process input
    while (true)
    {
        ch = _getwch();
        switch (ch)
        {
        case L'q':
        case L'Q':
            StopRecv(
                hStopEvent, 
                hRecvThread);
            goto Cleanup;
        case L'h':
        case L'H':
            SendHello(
                hDiscoverySocket,
                sDeviceId,
                DeviceInfo,
                pMulticastAddrInfo);
            break;
        case L'b':
        case L'B':
            SendBye(
                hDiscoverySocket,
                sDeviceId,
                pMulticastAddrInfo);
            break;
        }
    }

Cleanup:
    if (pMulticastAddrInfo)
    {
        FreeAddrInfo(pMulticastAddrInfo);
    }
    if (pLocalAddrInfo)
    {
        FreeAddrInfo(pLocalAddrInfo);
    }
    if (hDiscoverySocket != INVALID_SOCKET)
    {
        closesocket(hDiscoverySocket);
    }
    if (hStopEvent)
    {
        CloseHandle(hStopEvent);
    }
    if (hRecvThread)
    {
        CloseHandle(hRecvThread);
    }
    WSACleanup();


    return err;
}  // wmain

