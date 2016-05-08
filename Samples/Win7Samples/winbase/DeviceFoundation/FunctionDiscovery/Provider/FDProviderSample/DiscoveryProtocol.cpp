// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//  Abstract:
//
//      This module implements the CDiscoveryProtocol class.
//      This class implements a very ridimentary discovery protocol for
//      sample purposes only.  

#include "stdafx.h"

//---------------------------------------------------------------------------
// Begin Implementaion class definitions
//---------------------------------------------------------------------------

// Class used to represent Async socket IO data
class TAsyncSocketData
{
public:
    TAsyncSocketData(
        ULONG bufferSize);
    
    ~TAsyncSocketData();

    bool Init();

    VOID Reset();

public:
    WSAOVERLAPPED m_Overlapped;
    WSABUF m_RecvBuffer;
    SOCKADDR_STORAGE m_FromAddr;
    INT m_FromAddrLen;
};  // TAsyncSocketData

// Class that implements a Discovery Protocol query
class TQueryDevices
{
public:
    TQueryDevices();
    ~TQueryDevices();

    HRESULT StartQuery(
        __in TFunctionDiscoveryProvider* pFunctionDiscoveryProvider,
        __in_opt PCWSTR pszDeviceCategory,
        __in_opt PCWSTR pszInstanceQueryId);

protected:

    VOID StopQuery();

    HRESULT StartAsycRecv();

    static VOID CALLBACK ThreadpoolIOCallback(
        PTP_CALLBACK_INSTANCE Instance,
        __in PVOID pContext,
        __in PVOID pOverlapped,
        ULONG IoResult,
        ULONG_PTR NumberOfBytesTransferred,
        PTP_IO Io);

    static VOID CALLBACK QueryCompleteTimerCallback(
        PTP_CALLBACK_INSTANCE Instance,
        __in PVOID pContext,
        PTP_TIMER Timer);

protected:
    TFunctionDiscoveryProvider* m_pFunctionDiscoveryProvider;
    PWSTR m_pszDeviceCategory;
    bool m_fInstanceQuery;
    GUID m_InstanceQueryId;
    bool m_fWinsockInit;
    TNetworkIOThreadpool* m_pIOThreadpool;
    TNetworkTimersThreadpool* m_pTimerThreadpool;
    bool m_fThreadPoolEnvironmentInit;
    TP_CALLBACK_ENVIRON m_NetworkIOEnvironment;
    TP_CALLBACK_ENVIRON m_NetworkTimerEnvironment;
    PTP_IO m_hThreadpoolIO;
    PTP_TIMER m_hQueryTimer;
    SOCKET m_hSocket;
    TAsyncSocketData m_AsyncSocketData;
    ULONG m_InterfaceIndex;
    BOOL m_fStopQueryLatch;
    HANDLE m_hHelloByeRegistration;

};  // TQueryDevices;

// Class to store a Hello / Bye registration
class TRegistrationEntry:
    public TList<TRegistrationEntry>::TListEntry
{
public:
    TRegistrationEntry(
        __in TFunctionDiscoveryProvider* pFunctionDiscoveryProvider,
        __in_opt PCWSTR pszDeviceCategory,
        __in_opt GUID* pInstanceQueryId);

    TFunctionDiscoveryProvider* pFunctionDiscoveryProvider;
    PCWSTR pszDeviceCategory;
    GUID* pInstanceQueryId;
};  // TRegistrationEntry

// Class that implements listening to Hello / Bye messages and notify registered clients
class THelloByeManager
{
public:
    THelloByeManager();
    ~THelloByeManager();

    HRESULT Register(
        __in TFunctionDiscoveryProvider* pFunctionDiscoveryProvider,
        __in_opt PCWSTR pszDeviceCategory,
        __in_opt GUID* pInstanceQueryId,
        __deref_out PHANDLE phRegistration);

    VOID Unregister(
        HANDLE hRegistration);

protected:
    HRESULT DispatchMsg();
    
    VOID NotifyAllClientsOfErrorAndResetIO(
        HRESULT hr);

    HRESULT StartListening();
    VOID StopListening();

    HRESULT StartAsycRecv();

    static VOID CALLBACK ThreadpoolIOCallback(
        PTP_CALLBACK_INSTANCE Instance,
        __in PVOID pContext,
        __in PVOID pOverlapped,
        ULONG IoResult,
        ULONG_PTR NumberOfBytesTransferred,
        PTP_IO Io);

protected:
    TLock m_RegistrationLock;
    TLock m_RegistrationListLock;
    TList<TRegistrationEntry> m_RegistrationList;
    bool m_fIOReset;
    TNetworkIOThreadpool* m_pThreadpool;
    bool m_fThreadPoolEnvironmentInit;
    TP_CALLBACK_ENVIRON m_NetworkIOEnvironment;
    PTP_IO m_hThreadpoolIO;
    SOCKET m_hSocket;
    ULONG m_InterfaceIndex;
    TAsyncSocketData m_AsyncSocketData;

};  // THelloByeManager

//---------------------------------------------------------------------------
// End Implementaion class definitions
//---------------------------------------------------------------------------

// Global variables
THelloByeManager g_HelloByeManager;

//---------------------------------------------------------------------------
// Begin Util functions
//---------------------------------------------------------------------------

PWSTR AllocString(
    __in PCWSTR pszOriginalString)
{
    size_t cBufLen = (wcslen(pszOriginalString) + 1) * sizeof(WCHAR);
    PVOID pBuf = malloc(cBufLen);
    
    if (pBuf)
    {
        memcpy(pBuf, pszOriginalString, cBufLen);
    }

    return (PWSTR) pBuf;
}  // AllocString


HRESULT GetDefaultMulticastInterfaceIndex(
    ULONG AddressFamily, 
    ULONG& InterfaceIndex)
{
    HRESULT hr = S_OK;
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
            hr = E_FAIL;
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(hr);
    }

    // Cleanup
    if (pAdapterAddresses)
    {
        free(pAdapterAddresses);
    }

    return hr;
}  // GetDefaultMulticastInterfaceIndex

//---------------------------------------------------------------------------
// End Util functions
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Begin TAsyncSocketData Implementation
//---------------------------------------------------------------------------

TAsyncSocketData::TAsyncSocketData(
    ULONG bufferSize)
{
    m_RecvBuffer.len = bufferSize;
    m_RecvBuffer.buf = NULL;
}  // TAsyncSocketData::TAsyncSocketData

bool TAsyncSocketData::Init()
{
    bool fInit = false;

    m_RecvBuffer.buf = (char*) malloc(m_RecvBuffer.len);
    if (m_RecvBuffer.buf)
    {
        Reset();
        fInit = true;
    }

    return fInit;
}  // TAsyncSocketData::Init()

TAsyncSocketData::~TAsyncSocketData()
{
    if (m_RecvBuffer.buf)
    {
        free(m_RecvBuffer.buf);
        m_RecvBuffer.buf = NULL;
    }
}  // TAsyncSocketData::~TAsyncSocketData

VOID TAsyncSocketData::Reset()
{
    ZeroMemory(&m_Overlapped, sizeof(m_Overlapped));
    ZeroMemory(&m_FromAddr, sizeof(m_FromAddr));
    // ZeroMemory(m_RecvBuffer.buf, m_RecvBuffer.len);
    m_FromAddrLen = sizeof(m_FromAddr);
}  // VOID TAsyncSocketData::Reset()

//---------------------------------------------------------------------------
// End TAsyncSocketData Implementation
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Begin TQueryDevices Implementation
//---------------------------------------------------------------------------

TQueryDevices::TQueryDevices():
    m_pFunctionDiscoveryProvider(NULL),
    m_pszDeviceCategory(NULL),
    m_fInstanceQuery(false),
    m_fWinsockInit(false),
    m_pIOThreadpool(NULL),
    m_pTimerThreadpool(NULL),
    m_fThreadPoolEnvironmentInit(false),
    m_hThreadpoolIO(NULL),
    m_hQueryTimer(NULL),
    m_hSocket(INVALID_SOCKET),
    m_InterfaceIndex(0),
    m_fStopQueryLatch(FALSE),
    m_AsyncSocketData(MaxMessageSize),
    m_hHelloByeRegistration(NULL)
{
    ZeroMemory(&m_InstanceQueryId, sizeof(m_InstanceQueryId));
    ZeroMemory(&m_NetworkIOEnvironment, sizeof(m_NetworkIOEnvironment));
    ZeroMemory(&m_NetworkTimerEnvironment, sizeof(m_NetworkTimerEnvironment));
}  // TQueryDevices::TQueryDevices

TQueryDevices::~TQueryDevices()
{
    StopQuery();

    if (m_hQueryTimer)
    {
        SetThreadpoolTimer(
            m_hQueryTimer,
            NULL,
            0,
            0);
        WaitForThreadpoolTimerCallbacks(
            m_hQueryTimer,
            TRUE);
        CloseThreadpoolTimer(m_hQueryTimer);
        m_hQueryTimer = NULL;
    }

    if (m_fThreadPoolEnvironmentInit)
    {
        DestroyThreadpoolEnvironment(&m_NetworkIOEnvironment);
        ZeroMemory(&m_NetworkIOEnvironment, sizeof(m_NetworkIOEnvironment));
        
        DestroyThreadpoolEnvironment(&m_NetworkTimerEnvironment);
        ZeroMemory(&m_NetworkTimerEnvironment, sizeof(m_NetworkTimerEnvironment));

        m_fThreadPoolEnvironmentInit = false;
    }

    if (m_pIOThreadpool)
    {
        m_pIOThreadpool->Release();
        m_pIOThreadpool = NULL;
    }

    if (m_pTimerThreadpool)
    {
        m_pTimerThreadpool->Release();
        m_pTimerThreadpool = NULL;
    }

    if (m_hHelloByeRegistration)
    {
        g_HelloByeManager.Unregister(m_hHelloByeRegistration);
        m_hHelloByeRegistration = NULL;
    }

    if (m_fWinsockInit)
    {
        WSACleanup();
        m_fWinsockInit = false;
    }

    if (m_pszDeviceCategory)
    {
        free(m_pszDeviceCategory);
        m_pszDeviceCategory = NULL;
    }

    m_pFunctionDiscoveryProvider = NULL;
    
}  // TQueryDevices::~TQueryDevices

HRESULT TQueryDevices::StartQuery(
    __in TFunctionDiscoveryProvider* pFunctionDiscoveryProvider,
    __in_opt PCWSTR pszDeviceCategory,
    __in_opt PCWSTR pszInstanceQueryId)
{
    HRESULT hr = S_OK;
    TQueryMessage QueryMessage;
    WSADATA wsaData = {0};
    int err = 0;
    int recvBufSize = 64 * 1024;
    ADDRINFOW addrHints = {0};
    PADDRINFOW pMulticastAddrInfo = NULL;
    PADDRINFOW pLocalAddrInfo = NULL;
    BOOL fDisabled = FALSE;
    ULONG cByteCount = 0;
    LONGLONG DueTime = QueryTimeout * -10000;  // Relative timeout in 100 nanosecond increments

    // Save pFunctionDiscoveryProvider
    // NOTE:  Do not AddRef() pFunctionDiscoveryProvider
    // The provider owns TQueryDevices not the other way around.
    m_pFunctionDiscoveryProvider = pFunctionDiscoveryProvider;
    
    // Initialize m_NetworkIOEnvironment
    InitializeThreadpoolEnvironment(&m_NetworkIOEnvironment);
    InitializeThreadpoolEnvironment(&m_NetworkTimerEnvironment);
    m_fThreadPoolEnvironmentInit = true;

    if (pszDeviceCategory)
    {
        m_pszDeviceCategory = AllocString(pszDeviceCategory);
        if (!m_pszDeviceCategory)
        {
            hr = E_OUTOFMEMORY;
        }
    }

   if(S_OK == hr)
    {
        if (pszInstanceQueryId)
        {
            if (UuidFromString((RPC_WSTR) pszInstanceQueryId, &m_InstanceQueryId) == RPC_S_OK)
            {
                m_fInstanceQuery = true;
            }
            else
            {
                hr = E_INVALIDARG;
            }
        }
    }

    // Initialize the Query message
    if(S_OK == hr)
    {
        QueryMessage.MessageType = Query;
        if (pszDeviceCategory)
        {
            hr = StringCbCopy(
                QueryMessage.szDeviceCategory, 
                sizeof(QueryMessage.szDeviceCategory), 
                pszDeviceCategory);
        }
        else
        {
            *QueryMessage.szDeviceCategory = L'\0';
        }
    }

    // Get a pointer to the IO Threadpool
    if (S_OK == hr)
    {
        m_pIOThreadpool = TNetworkIOThreadpool::GetThreadpool();
        if (!m_pIOThreadpool)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    // Associate the Threadpool with the environment block
    if (S_OK == hr)
    {
        SetThreadpoolCallbackPool(
            &m_NetworkIOEnvironment,
            m_pIOThreadpool->GetPTP_POOL());
    }

    // Get a pointer to the Timers Threadpool
    if (S_OK == hr)
    {
        m_pTimerThreadpool = TNetworkTimersThreadpool::GetThreadpool();
        if (!m_pTimerThreadpool)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    // Associate the Threadpool with the environment block
    if (S_OK == hr)
    {
        SetThreadpoolCallbackPool(
            &m_NetworkTimerEnvironment,
            m_pTimerThreadpool->GetPTP_POOL());
    }

    // Initialize Winsock
    if(S_OK == hr)
    {
        err = WSAStartup(MAKEWORD(2,2), &wsaData);
        if (0 == err)
        {
            m_fWinsockInit = true;
            if (   (2 != LOBYTE(wsaData.wVersion))
                || (2 != HIBYTE(wsaData.wVersion)))
            {
                // We needed winsock 2.2
                hr = HRESULT_FROM_WIN32(WSAVERNOTSUPPORTED);
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(err);
        }
    }

    // Create multicast socket address structure
    if (S_OK == hr)
    {
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
            hr = HRESULT_FROM_WIN32(err);
        }
    }

    // Create local address to bind to
    if (S_OK == hr)
    {
        addrHints.ai_flags = AI_PASSIVE;
        addrHints.ai_family = pMulticastAddrInfo->ai_family;
        addrHints.ai_socktype = pMulticastAddrInfo->ai_socktype;
        addrHints.ai_protocol = pMulticastAddrInfo->ai_protocol;

        err = GetAddrInfo(
            NULL,  // local socket
            L"0",  // any port
            &addrHints,
            &pLocalAddrInfo);

        if (0 != err)
        {
            hr = HRESULT_FROM_WIN32(err);
        }
    }
    
    // Create a socket to send the Query request and receive the responses
    if (S_OK == hr)
    {
        m_hSocket = WSASocket(
            pLocalAddrInfo->ai_family,
            pLocalAddrInfo->ai_socktype,
            pLocalAddrInfo->ai_protocol,
            NULL,
            NULL,
            WSA_FLAG_OVERLAPPED);

        if (m_hSocket == INVALID_SOCKET)
        {
            hr = HRESULT_FROM_WIN32(WSAGetLastError());
        }
    }

    // Disable connection reset for UDP
    if (S_OK == hr)
    {
        err = WSAIoctl(
            m_hSocket,
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
            hr = HRESULT_FROM_WIN32(WSAGetLastError());
        }
    }

    // Adjust the receive buffer size
    if (S_OK == hr)
    {
        err = setsockopt(
            m_hSocket,
            SOL_SOCKET,
            SO_RCVBUF,
            (char*) &recvBufSize,
            sizeof(recvBufSize));
        if (0 != err)
        {
            hr = HRESULT_FROM_WIN32(WSAGetLastError());
        }
    }

    // Get the default interface
    if (S_OK == hr)
    {
        hr = GetDefaultMulticastInterfaceIndex(
            pMulticastAddrInfo->ai_family,
            m_InterfaceIndex);
    }

    // Set the outgoing multicast interface
    if (S_OK == hr)
    {
        if (AF_INET == pMulticastAddrInfo->ai_family)
        {
            err = setsockopt(
                m_hSocket,
                IPPROTO_IP,
                IP_MULTICAST_IF,
                (char*) &m_InterfaceIndex,
                sizeof(m_InterfaceIndex));
        }
        else
        {
            err = setsockopt(
                m_hSocket,
                IPPROTO_IPV6,
                IPV6_MULTICAST_IF,
                (char*) &m_InterfaceIndex,
                sizeof(m_InterfaceIndex));
        }

        if (0 != err)
        {
            hr = HRESULT_FROM_WIN32(WSAGetLastError());
        }
    }

    // Bind the socket
    if (S_OK == hr)
    {
        err = bind(
            m_hSocket,
            pLocalAddrInfo->ai_addr,
            (int) pLocalAddrInfo->ai_addrlen);

        if (0 != err)
        {
            hr = HRESULT_FROM_WIN32(WSAGetLastError());
        }
    }

    // Bind the socket to Threadpool IO
    if (S_OK == hr)
    {
        m_hThreadpoolIO = CreateThreadpoolIo(
            (HANDLE) m_hSocket,
            &ThreadpoolIOCallback,
            this,
            &m_NetworkIOEnvironment);
        
        if (!m_hThreadpoolIO)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    // Create a timer to track the query time
    if (S_OK == hr)
    {
        m_hQueryTimer = CreateThreadpoolTimer(
            &QueryCompleteTimerCallback,
            this,
            &m_NetworkTimerEnvironment);

        if (!m_hQueryTimer)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    // Initialize m_AsyncSocketData
    if (S_OK == hr)
    {
        if (!m_AsyncSocketData.Init())
        {
            hr = E_OUTOFMEMORY;
        }
    }

    // Start receving messages from the socket
    if (S_OK == hr)
    {
        hr = StartAsycRecv();
    }

    // Register for Hello and Bye messages
    if (S_OK == hr)
    {
        hr = g_HelloByeManager.Register(
            m_pFunctionDiscoveryProvider,
            m_pszDeviceCategory,
            (m_fInstanceQuery) ? &m_InstanceQueryId : NULL,
            &m_hHelloByeRegistration);
    }

    // Send the Query message
    if (S_OK == hr)
    {
        err = sendto(
            m_hSocket, 
            (char*) &QueryMessage,
            sizeof(TQueryMessage),
            0,
            pMulticastAddrInfo->ai_addr,
            (int) pMulticastAddrInfo->ai_addrlen);
        if (err == SOCKET_ERROR)
        {
            hr = HRESULT_FROM_WIN32(WSAGetLastError());
        }
    }

    // Set the query timer
    if (S_OK == hr)
    {
        SetThreadpoolTimer(
            m_hQueryTimer,
            (PFILETIME) &DueTime,
            0,
            0);
    }

    // Cleanup
    if (pMulticastAddrInfo)
    {
        FreeAddrInfo(pMulticastAddrInfo);
    }
    if (pLocalAddrInfo)
    {
        FreeAddrInfo(pLocalAddrInfo);
    }

    return hr;
}  // TQueryDevices::StartQuery

VOID TQueryDevices::StopQuery()
{
    // Because StopQuery is called from the timer callback,
    // there is a race with a client calling Release.
    // m_fStopQueryLatch makes sure we only stop once.

    BOOL fStopQueryLatch = InterlockedCompareExchange(
        (LONG*) &m_fStopQueryLatch,
        TRUE,
        FALSE);

    if (!fStopQueryLatch)
    {
        // Close the socket
        if (m_hSocket != INVALID_SOCKET)
        {
            closesocket(m_hSocket);
            m_hSocket = INVALID_SOCKET; 
        }

        // Wait for all IO callbacks to complete and close the IO
        if (m_hThreadpoolIO)
        {
            WaitForThreadpoolIoCallbacks(
                m_hThreadpoolIO,
                FALSE);
            CloseThreadpoolIo(m_hThreadpoolIO);
            m_hThreadpoolIO = NULL;
        }
    }
}  // TQueryDevices::StopQuery

HRESULT TQueryDevices::StartAsycRecv()
{
    HRESULT hr = S_OK;
    int err = 0;
    DWORD Flags = 0;

    // Reset the Async data 
    m_AsyncSocketData.Reset();

    StartThreadpoolIo(m_hThreadpoolIO);

// Start Asyc Recv
    err = WSARecvFrom(
        m_hSocket,
        &m_AsyncSocketData.m_RecvBuffer,
        1,
        NULL,
        &Flags,
        (LPSOCKADDR) &m_AsyncSocketData.m_FromAddr,
        &m_AsyncSocketData.m_FromAddrLen,
        &m_AsyncSocketData.m_Overlapped,
        NULL);

    if (0 != err)
    {
        err = WSAGetLastError();
        if (WSA_IO_PENDING != err)
        {
            // An error has occured, cancel the Threadpool IO
            CancelThreadpoolIo(m_hThreadpoolIO);

            hr = HRESULT_FROM_WIN32(err);
        }
    }

    return hr;
}  // TQueryDevices::StartAsycRecv

VOID CALLBACK TQueryDevices::ThreadpoolIOCallback(
        PTP_CALLBACK_INSTANCE Instance,
        __in PVOID pContext,
        __in_opt PVOID pOverlapped,
        ULONG IoResult,
        ULONG_PTR BytesTransferred,
        PTP_IO Io)
{
    HRESULT hr = S_OK;
    TQueryDevices* pQueryDevices = (TQueryDevices*) pContext;
    TReplyMessage* pReplyMessage = (TReplyMessage*) pQueryDevices->m_AsyncSocketData.m_RecvBuffer.buf;
    TFunctionInstanceInfo* pFunctionInstanceInfo = NULL;

    if (   pOverlapped
        && (WSA_OPERATION_ABORTED != IoResult))
    {
        if (NO_ERROR != IoResult)
        {
            hr = HRESULT_FROM_WIN32(IoResult);
        }

        // Dispatch the message to the client if it is valid
        if (S_OK == hr)
        {
            if (   (sizeof(TReplyMessage) <= BytesTransferred)
                && (Reply == pReplyMessage->MessageType))
            {
                // Message is valid
                
                if (   !pQueryDevices->m_fInstanceQuery
                    || IsEqualGUID(pQueryDevices->m_InstanceQueryId, pReplyMessage->DeviceId))
                {
                    // This is not an instance query, 
                    // or this is the instance we are looking for.
                    // Notify the client that a device has been found

                    hr = TFunctionInstanceInfo::CreateInstance(
                        &pReplyMessage->DeviceId,
                        &pReplyMessage->DeviceInfo,
                        &pQueryDevices->m_AsyncSocketData.m_FromAddr,
                        pQueryDevices->m_AsyncSocketData.m_FromAddrLen,
                        pQueryDevices->m_InterfaceIndex,
                        &pFunctionInstanceInfo);

                    if (S_OK == hr)
                    {
                        hr = pQueryDevices->m_pFunctionDiscoveryProvider->SubmitNotifyClientOnUpdate(
                            QUA_ADD,
                            pFunctionInstanceInfo);

                        pFunctionInstanceInfo->Release();
                    }
                }
            }
            // else just drop the message and continue to the next one
        }

        if (S_OK != hr)
        {
            // An error has occured notify the client.
            pQueryDevices->m_pFunctionDiscoveryProvider->SubmitNotifyClientOnError(hr);
        }

        // Initiate the next Async Recv
        // Do this even in the error case to handle WSA_OPERATION_ABORTED callback when the socket is closed.
        // hr intentionally overridden
        hr = pQueryDevices->StartAsycRecv();

        if (S_OK != hr)
        {
            // An error has occured notify the client.
            pQueryDevices->m_pFunctionDiscoveryProvider->SubmitNotifyClientOnError(hr);
        }
    }
}  // TQueryDevices::ThreadpoolIOCallback

VOID CALLBACK TQueryDevices::QueryCompleteTimerCallback(
    PTP_CALLBACK_INSTANCE Instance,
    __in PVOID pContext,
    PTP_TIMER Timer)
{
    HRESULT hr = S_OK;
    TQueryDevices* pQueryDevices = (TQueryDevices*) pContext;

    // Stop the query and free resources
    pQueryDevices->StopQuery();
    
    // Notify the client that the Query is complete
    hr = pQueryDevices->m_pFunctionDiscoveryProvider->SubmitNotifyClientOnEvent(FD_EVENTID_SEARCHCOMPLETE);

    if (S_OK != hr)
    {
        // Attempt to notify the client that an error occured.
        pQueryDevices->m_pFunctionDiscoveryProvider->SubmitNotifyClientOnError(hr);
    }

}  // TQueryDevices::QueryCompleteTimerCallback

//---------------------------------------------------------------------------
// End TQueryDevices Implementation
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Begin TRegistrationEntry Implementation
//---------------------------------------------------------------------------

TRegistrationEntry::TRegistrationEntry(
        __in TFunctionDiscoveryProvider* pFunctionDiscoveryProvider,
        __in_opt PCWSTR pszDeviceCategory,
        __in_opt GUID* pInstanceQueryId):

    pFunctionDiscoveryProvider(pFunctionDiscoveryProvider),
    pszDeviceCategory(pszDeviceCategory),
    pInstanceQueryId(pInstanceQueryId)
{
}  // TRegistrationEntry::TRegistrationEntry

//---------------------------------------------------------------------------
// End TRegistrationEntry Implementation
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Begin THelloByeManager
//---------------------------------------------------------------------------

THelloByeManager::THelloByeManager():
    m_fIOReset(false),
    m_pThreadpool(NULL),
    m_fThreadPoolEnvironmentInit(false),
    m_hThreadpoolIO(NULL),
    m_hSocket(INVALID_SOCKET),
    m_InterfaceIndex(0),
    m_AsyncSocketData(MaxMessageSize)
{
    ZeroMemory(&m_NetworkIOEnvironment, sizeof(m_NetworkIOEnvironment));
}  // THelloByeManager::THelloByeManager

THelloByeManager::~THelloByeManager()
{
    assert(m_RegistrationList.IsEmpty());
}  // THelloByeManager::THelloByeManager

HRESULT THelloByeManager::Register(
        __in TFunctionDiscoveryProvider* pFunctionDiscoveryProvider,
        __in_opt PCWSTR pszDeviceCategory,
        __in_opt GUID* pInstanceQueryId,
        __deref_out PHANDLE phRegistration)
{
    HRESULT hr = S_OK;
    BOOL fStartListening = false;
    bool fIOReset = false;
    TRegistrationEntry* pRegistration = new(std::nothrow) TRegistrationEntry(
        pFunctionDiscoveryProvider,
        pszDeviceCategory,
        pInstanceQueryId);

    *phRegistration = NULL;
    
    if (pRegistration)
    {
        m_RegistrationLock.AcquireExclusive();

        // Add the registration to the list and 
        // cache if the IO needs to be reset or IO needs to be started.
        m_RegistrationListLock.AcquireExclusive();

        fStartListening = m_RegistrationList.IsEmpty();
        fIOReset = m_fIOReset;
        
        m_fIOReset = false;  // reset the flag
        
         m_RegistrationList.InsertTail(pRegistration);

         *phRegistration = (PHANDLE) pRegistration;
        
        m_RegistrationListLock.ReleaseExclusive();

        // If IO needs to be reset, stop listenting
        if (fIOReset)
        {
            StopListening();
            
            assert(fStartListening);
        }

        // If this is the first registration, start listening for messages.
        if (fStartListening)
        {
            hr = StartListening();
        }

        if (S_OK != hr)
        {
            // If the start failed, clean up the registration

            m_RegistrationListLock.AcquireExclusive();

            m_RegistrationList.RemoveEntry(pRegistration);
            
            m_RegistrationListLock.ReleaseExclusive();

            delete pRegistration;
        }
        
        m_RegistrationLock.ReleaseExclusive();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}  // THelloByeManager::Register

VOID THelloByeManager::Unregister(
    HANDLE hRegistration)
{
    TRegistrationEntry* pRegistration = (TRegistrationEntry*) hRegistration;
    BOOL fListIsEmpty = FALSE;

    m_RegistrationLock.AcquireExclusive();

    m_RegistrationListLock.AcquireExclusive();

    // Remove the registration from the list.
    m_RegistrationList.RemoveEntryIfInList(pRegistration);

    fListIsEmpty = m_RegistrationList.IsEmpty();

    m_RegistrationListLock.ReleaseExclusive();

    // If this is the last registration, stop listening for messages and free resources
    if (fListIsEmpty)
    {
        StopListening();
    }

    m_RegistrationLock.ReleaseExclusive();

    delete pRegistration;
}  // THelloByeManager::Unregister

HRESULT THelloByeManager::DispatchMsg()
{
    HRESULT hr = S_OK;
    TMessage* pMessage = (TMessage*) m_AsyncSocketData.m_RecvBuffer.buf;
    TFunctionInstanceInfo* pFunctionInstanceInfo = NULL;

    if (Hello == pMessage->MessageType)
    {
        THelloMessage* pHelloMessage = (THelloMessage*) m_AsyncSocketData.m_RecvBuffer.buf;

        hr = TFunctionInstanceInfo::CreateInstance(
            &pHelloMessage->DeviceId,
            &pHelloMessage->DeviceInfo,
            &m_AsyncSocketData.m_FromAddr,
            m_AsyncSocketData.m_FromAddrLen,
            m_InterfaceIndex,
            &pFunctionInstanceInfo);

        if (S_OK == hr)
        {
            m_RegistrationListLock.AcquireShared();

            for (
                TList<TRegistrationEntry>::TIterator pRegistrationEntry = m_RegistrationList.Begin();
                pRegistrationEntry != m_RegistrationList.End();
                pRegistrationEntry = ++pRegistrationEntry)
            {

                if (   (   pRegistrationEntry->pInstanceQueryId
                        && (IsEqualGUID(*pRegistrationEntry->pInstanceQueryId, pHelloMessage->DeviceId)))
                    || !pRegistrationEntry->pszDeviceCategory
                    || (wcscmp(pRegistrationEntry->pszDeviceCategory, pHelloMessage->DeviceInfo.szDeviceCategory)))
                {
                    // This is not an instance query and the device categories match
                    // or this is the instance we are looking for

                    hr = pRegistrationEntry->pFunctionDiscoveryProvider->SubmitNotifyClientOnUpdate(
                            QUA_ADD,
                            pFunctionInstanceInfo);

                    if (S_OK != hr)
                    {
                        // If an error occurs, attempt to notify the client
                        pRegistrationEntry->pFunctionDiscoveryProvider->SubmitNotifyClientOnError(hr);
                        hr = S_OK;
                    }
                }
            }

            m_RegistrationListLock.ReleaseShared();
        }
    }
    else // (Bye == pMessage->MessageType)
    {
        TByeMessage* pByeMessage = (TByeMessage*) m_AsyncSocketData.m_RecvBuffer.buf;

        hr = TFunctionInstanceInfo::CreateInstance(
                &pByeMessage->DeviceId,
                NULL,
                &m_AsyncSocketData.m_FromAddr,
                m_AsyncSocketData.m_FromAddrLen,
                m_InterfaceIndex,
                &pFunctionInstanceInfo);

        if (S_OK == hr)
        {
            m_RegistrationListLock.AcquireShared();

            for (
                TList<TRegistrationEntry>::TIterator pRegistrationEntry = m_RegistrationList.Begin();
                pRegistrationEntry != m_RegistrationList.End();
                pRegistrationEntry = ++pRegistrationEntry)
            {

                if (   !pRegistrationEntry->pInstanceQueryId
                    || (IsEqualGUID(*pRegistrationEntry->pInstanceQueryId, pByeMessage->DeviceId)))
                {
                    // This is not an instance query
                    // or this is the instance we are looking for
                    hr = pRegistrationEntry->pFunctionDiscoveryProvider->SubmitNotifyClientOnUpdate(
                            QUA_REMOVE,
                            pFunctionInstanceInfo);

                    if (S_OK != hr)
                    {
                        // If an error occurs, attempt to notify the client
                        pRegistrationEntry->pFunctionDiscoveryProvider->SubmitNotifyClientOnError(hr);
                        hr = S_OK;
                    }
                }
            }

            m_RegistrationListLock.ReleaseShared();
        }
    }

    if (pFunctionInstanceInfo)
    {
        pFunctionInstanceInfo->Release();
    }

    return hr;
}
// THelloByeManager::DispatchMsg

//
//  NotifyAllClientsOfErrorAndResetIO must only be called from Treadpool threads
//
VOID THelloByeManager::NotifyAllClientsOfErrorAndResetIO(
    HRESULT hr)
{
    TRegistrationEntry* pRegistrationEntry = NULL;

    m_RegistrationListLock.AcquireExclusive();

    // Notify all the registered clients that an error has occured
    // and free the registrations
    while (!m_RegistrationList.IsEmpty())
    {
        pRegistrationEntry = m_RegistrationList.RemoveHead();

        pRegistrationEntry->pFunctionDiscoveryProvider->SubmitNotifyClientOnError(hr);
    }

    m_fIOReset = true;

    m_RegistrationListLock.ReleaseExclusive();
}

HRESULT THelloByeManager::StartListening()
{
    HRESULT hr = S_OK;
    WSADATA wsaData = {0};
    int err = 0;
    int recvBufSize = 64 * 1024;
    ADDRINFOW addrHints = {0};
    PADDRINFOW pMulticastAddrInfo = NULL;
    PADDRINFOW pLocalAddrInfo = NULL;
    BOOL fEnabled = TRUE;
    BOOL fDisabled = FALSE;

    // Initialize m_NetworkIOEnvironment
    InitializeThreadpoolEnvironment(&m_NetworkIOEnvironment);
    m_fThreadPoolEnvironmentInit = true;

    // Get a pointer to the IO Threadpool
    if (S_OK == hr)
    {
        m_pThreadpool = TNetworkIOThreadpool::GetThreadpool();
        if (!m_pThreadpool)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    // Associate the Threadpool with the environment block
    if (S_OK == hr)
    {
        SetThreadpoolCallbackPool(
            &m_NetworkIOEnvironment,
            m_pThreadpool->GetPTP_POOL());
    }

    // Initialize m_AsyncSocketData
    if (S_OK == hr)
    {
        if (!m_AsyncSocketData.Init())
        {
            hr = E_OUTOFMEMORY;
        }
    }

    // Create multicast socket address structure
    if (S_OK == hr)
    {
        addrHints.ai_flags = AI_NUMERICHOST;
        addrHints.ai_family = AF_UNSPEC;
        addrHints.ai_socktype = SOCK_DGRAM;
        addrHints.ai_protocol = IPPROTO_UDP;

        err = GetAddrInfo(
            szMulticastAddress,
            NULL,
            &addrHints,
            &pMulticastAddrInfo);
        if (0 != err)
        {
            hr = HRESULT_FROM_WIN32(err);
        }
    }

    // Create local address to bind to
    if (S_OK == hr)
    {
        addrHints.ai_flags = AI_PASSIVE;
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
            hr = HRESULT_FROM_WIN32(err);
        }
    }
    
    // Create a socket to listen on
    if (S_OK == hr)
    {
        m_hSocket = WSASocket(
            pLocalAddrInfo->ai_family,
            pLocalAddrInfo->ai_socktype,
            pLocalAddrInfo->ai_protocol,
            NULL,
            NULL,
            WSA_FLAG_OVERLAPPED);

        if (m_hSocket == INVALID_SOCKET)
        {
            hr = HRESULT_FROM_WIN32(WSAGetLastError());
        }
    }

    // Set SO_EXCLUSIVEADDRUSE and SO_REUSEADDR for the socket 
    // so that there can be multiple listeners on the same machine
    if (S_OK == hr)
    {
        err = setsockopt(
            m_hSocket, 
            SOL_SOCKET, 
            SO_EXCLUSIVEADDRUSE,
            (char*) &fDisabled,
            sizeof(fDisabled));

        if (0 != err)
        {
            hr = HRESULT_FROM_WIN32(WSAGetLastError());
        }
    }
    if (S_OK == hr)
    {
        err = setsockopt(
            m_hSocket, 
            SOL_SOCKET, 
            SO_REUSEADDR,
            (char*) &fEnabled,
            sizeof(fEnabled));

        if (0 != err)
        {
            hr = HRESULT_FROM_WIN32(WSAGetLastError());
        }
    }

    // Adjust the receive buffer size
    if (S_OK == hr)
    {
        err = setsockopt(
            m_hSocket,
            SOL_SOCKET,
            SO_RCVBUF,
            (char*) &recvBufSize,
            sizeof(recvBufSize));
        if (0 != err)
        {
            hr = HRESULT_FROM_WIN32(WSAGetLastError());
        }
    }

    // Join the socket to the multicast group
    if (S_OK == hr)
    {
        if (AF_INET == pMulticastAddrInfo->ai_family)  // IPv4
        {
            IP_MREQ mreq = {0};

            hr = GetDefaultMulticastInterfaceIndex(
                AF_INET,
                m_InterfaceIndex);

            if (S_OK == hr)
            {
                mreq.imr_multiaddr = ((PSOCKADDR_IN)pMulticastAddrInfo->ai_addr)->sin_addr;  // Set the multicast address
                memcpy(&mreq.imr_interface, &m_InterfaceIndex, sizeof(mreq.imr_interface));    // Set the interface index

                err = setsockopt(
                    m_hSocket,
                    IPPROTO_IP,
                    IP_ADD_MEMBERSHIP,
                    (char*) &mreq,
                    sizeof(mreq));

                if (0 != err)
                {
                    hr = HRESULT_FROM_WIN32(WSAGetLastError());
                }
            }
        }
        else // IPv6
        {
            IPV6_MREQ mreq6 = {0};

            hr = GetDefaultMulticastInterfaceIndex(
                AF_INET6,
                m_InterfaceIndex);

            if (S_OK == hr)
            {

                mreq6.ipv6mr_multiaddr = ((PSOCKADDR_IN6) pMulticastAddrInfo->ai_addr)->sin6_addr;  // Set the multicast address
                mreq6.ipv6mr_interface = m_InterfaceIndex;

                err = setsockopt(
                    m_hSocket,
                    IPPROTO_IPV6,
                    IPV6_ADD_MEMBERSHIP,
                    (char*) &mreq6,
                    sizeof(mreq6));

                if (0 != err)
                {
                    hr = HRESULT_FROM_WIN32(WSAGetLastError());
                }
            }
        }
    }

    // Bind the socket
    if (S_OK == hr)
    {
        err = bind(
            m_hSocket,
            pLocalAddrInfo->ai_addr,
            (int) pLocalAddrInfo->ai_addrlen);

        if (0 != err)
        {
            hr = HRESULT_FROM_WIN32(WSAGetLastError());
        }
    }

    // Bind the socket to Threadpool IO
    if (S_OK == hr)
    {
        m_hThreadpoolIO = CreateThreadpoolIo(
            (HANDLE) m_hSocket,
            &ThreadpoolIOCallback,
            this,
            &m_NetworkIOEnvironment);
        
        if (!m_hThreadpoolIO)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    // Start listening
    if (S_OK == hr)
    {
        hr = StartAsycRecv();
    }

    // If a failure occured, clean up
    if (S_OK != hr)
    {
        StopListening();
    }

    // Cleanup
    if (pMulticastAddrInfo)
    {
        FreeAddrInfo(pMulticastAddrInfo);
    }
    if (pLocalAddrInfo)
    {
        FreeAddrInfo(pLocalAddrInfo);
    }

    return hr;
}  // THelloByeManager::StartListening

VOID THelloByeManager::StopListening()
{
    // Close the socket
    if (m_hSocket != INVALID_SOCKET)
    {
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET; 
    }

    // Wait for all IO callbacks to complete and close the IO
    if (m_hThreadpoolIO)
    {
        WaitForThreadpoolIoCallbacks(
            m_hThreadpoolIO,
            FALSE);
        CloseThreadpoolIo(m_hThreadpoolIO);
        m_hThreadpoolIO = NULL;
    }

    if (m_fThreadPoolEnvironmentInit)
    {
        DestroyThreadpoolEnvironment(&m_NetworkIOEnvironment);
        ZeroMemory(&m_NetworkIOEnvironment, sizeof(m_NetworkIOEnvironment));
        m_fThreadPoolEnvironmentInit = false;
    }

    if (m_pThreadpool)
    {
        m_pThreadpool->Release();
        m_pThreadpool = NULL;
    }
}  // THelloByeManager::StopListening

HRESULT THelloByeManager::StartAsycRecv()
{
    HRESULT hr = S_OK;
    int err = 0;
    DWORD Flags = 0;

    // Reset he Async data 
    m_AsyncSocketData.Reset();

    StartThreadpoolIo(m_hThreadpoolIO);

    // Start Asyc Recv
    err = WSARecvFrom(
        m_hSocket,
        &m_AsyncSocketData.m_RecvBuffer,
        1,
        NULL,
        &Flags,
        (LPSOCKADDR) &m_AsyncSocketData.m_FromAddr,
        &m_AsyncSocketData.m_FromAddrLen,
        &m_AsyncSocketData.m_Overlapped,
        NULL);

    if (0 != err)
    {
        err = WSAGetLastError();
        if (WSA_IO_PENDING != err)
        {
            // An error has occured, cancel the Threadpool IO
            CancelThreadpoolIo(m_hThreadpoolIO);

            hr = HRESULT_FROM_WIN32(err);
        }
    }

    return hr;
}  // THelloByeManager::StartAsycRecv

VOID CALLBACK THelloByeManager::ThreadpoolIOCallback(
    PTP_CALLBACK_INSTANCE Instance,
    __in PVOID pContext,
    __in_opt PVOID pOverlapped,
    ULONG IoResult,
    ULONG_PTR BytesTransferred,
    PTP_IO Io)
{
    HRESULT hr = S_OK;
    THelloByeManager* pHelloByeManager = (THelloByeManager*) pContext;
    TMessage* pMessage = (TMessage*) pHelloByeManager->m_AsyncSocketData.m_RecvBuffer.buf;

    if (   pOverlapped
        && (WSA_OPERATION_ABORTED != IoResult))
    {
        if (NO_ERROR != IoResult)
        {
            hr = HRESULT_FROM_WIN32(IoResult);
        }

        // Dispatch the message to the client if it is valid
        if (S_OK == hr)
        {
            if (   (sizeof(TMessage) <= BytesTransferred)
                && (   (   (pMessage->MessageType == Hello)
                        && (sizeof(THelloMessage) <= BytesTransferred))
                    || (   (pMessage->MessageType == Bye)
                        && (sizeof(TByeMessage) <= BytesTransferred))))
            {
                // Message is valid,  Dispatch it to the client

                hr = pHelloByeManager->DispatchMsg();
            }
            // else just drop the message and continue to the next one
        }

        // Initiate the next Async Recv
        // Do this even in the error case to handle WSA_OPERATION_ABORTED callback when the socket is closed.
        // hr intentionally overridden
        hr = pHelloByeManager->StartAsycRecv();

        if (S_OK != hr)
        {
            // An error has occured notify the client.  (No more messages will be delivered)

            pHelloByeManager->NotifyAllClientsOfErrorAndResetIO(hr);
        }
    }

}  // THelloByeManager::ThreadpoolIOCallback

//---------------------------------------------------------------------------
// End THelloByeManager
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Begin public interface implementation
//---------------------------------------------------------------------------

HRESULT StartDiscoveryQuery(
    __in TFunctionDiscoveryProvider* pFunctionDiscoveryProvider,
    __in_opt PCWSTR pszDeviceCategory,
    __in_opt PCWSTR pszInstanceQueryId,
    __deref_out PHANDLE phQuery)
{
    HRESULT hr = S_OK;
    TQueryDevices* pQueryDevices = NULL;

    *phQuery = NULL;

    pQueryDevices = new(std::nothrow) TQueryDevices;
    if (pQueryDevices)
    {
        hr = pQueryDevices->StartQuery(
            pFunctionDiscoveryProvider,
            pszDeviceCategory,
            pszInstanceQueryId);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    if (S_OK == hr)
    {
        *phQuery = (PHANDLE) pQueryDevices;
    }
    else
    {
        delete pQueryDevices;
    }

    return hr;
}  // StartDiscoveryQuery

VOID CloseDiscoveryQuery(HANDLE hQuery)
{
    delete (TQueryDevices*) hQuery;
}  // CloseDiscoveryQuery

//---------------------------------------------------------------------------
// End public interface implementation
//---------------------------------------------------------------------------
