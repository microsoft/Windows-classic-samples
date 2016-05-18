// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// BTHCxnDemo.cpp - Simple Bluetooth application using Winsock 2.2 and RFCOMM protocol
//
//
//      This example demonstrates how to use Winsock-2.2 APIs to connect
//      between two remote bluetooth devices and transfer data between them.
//      This example only supports address family of type AF_BTH,
//      socket of type SOCK_STREAM, protocol of type BTHPROTO_RFCOMM.
//
//      Once this source code is built, the resulting application can be
//      run either in server mode or in client mode.  See command line help
//      for command-line-examples and detailed explanation about all options.
//

#include <stdio.h>
#include <initguid.h>
#include <winsock2.h>
#include <ws2bth.h>
#include <strsafe.h>
#include <intsafe.h>


// {B62C4E8D-62CC-404b-BBBF-BF3E3BBB1374}
DEFINE_GUID(g_guidServiceClass, 0xb62c4e8d, 0x62cc, 0x404b, 0xbb, 0xbf, 0xbf, 0x3e, 0x3b, 0xbb, 0x13, 0x74);

#define CXN_TEST_DATA_STRING              (L"~!@#$%^&*()-_=+?<>1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")
#define CXN_TRANSFER_DATA_LENGTH          (sizeof(CXN_TEST_DATA_STRING))


#define CXN_BDADDR_STR_LEN                17   // 6 two-digit hex values plus 5 colons
#define CXN_MAX_INQUIRY_RETRY             3
#define CXN_DELAY_NEXT_INQUIRY            15
#define CXN_SUCCESS                       0
#define CXN_ERROR                         1
#define CXN_DEFAULT_LISTEN_BACKLOG        4



wchar_t g_szRemoteName[BTH_MAX_NAME_SIZE + 1] = {0};  // 1 extra for trailing NULL character
wchar_t g_szRemoteAddr[CXN_BDADDR_STR_LEN + 1] = {0}; // 1 extra for trailing NULL character
int  g_ulMaxCxnCycles = 1;



ULONG NameToBthAddr(__in const LPWSTR pszRemoteName, __out PSOCKADDR_BTH pRemoteBthAddr);
ULONG RunClientMode(__in SOCKADDR_BTH ululRemoteBthAddr, __in int iMaxCxnCycles = 1);
ULONG RunServerMode(__in int iMaxCxnCycles = 1);
void  ShowCmdLineHelp(void);
ULONG ParseCmdLine(__in int argc, __in_ecount(argc) wchar_t * argv[]);




int _cdecl wmain(__in int argc, __in_ecount(argc)wchar_t *argv[])
{
    ULONG       ulRetCode = CXN_SUCCESS;
    WSADATA     WSAData = {0};
    SOCKADDR_BTH RemoteBthAddr = {0};

    //
    // Parse the command line
    //
    ulRetCode = ParseCmdLine(argc, argv);
    if ( CXN_ERROR == ulRetCode ) {
        //
        // Command line syntax error.  Display cmd line help
        //
        ShowCmdLineHelp();
    } else if ( CXN_SUCCESS != ulRetCode) {
        wprintf(L"-FATAL- | Error in parsing command line\n");
    }

    //
    // Ask for Winsock version 2.2.
    //
    if ( CXN_SUCCESS == ulRetCode) {
        ulRetCode = WSAStartup(MAKEWORD(2, 2), &WSAData);
        if (CXN_SUCCESS != ulRetCode) {
            wprintf(L"-FATAL- | Unable to initialize Winsock version 2.2\n");
        }
    }

    if ( CXN_SUCCESS == ulRetCode) {

        //
        // Note, this app "prefers" the name if provided, but it is app-specific
        // Other applications may provide more generic treatment.
        //
        if ( L'\0' != g_szRemoteName[0] ) {
            //
            // Get address from the name of the remote device and run the application
            // in client mode
            //
            ulRetCode = NameToBthAddr(g_szRemoteName, &RemoteBthAddr);
            if ( CXN_SUCCESS != ulRetCode ) {
                wprintf(L"-FATAL- | Unable to get address of the remote radio having name %s\n", g_szRemoteName);	
            }

            if ( CXN_SUCCESS == ulRetCode) {
                ulRetCode = RunClientMode(RemoteBthAddr, g_ulMaxCxnCycles);
            }
            
        } else if ( L'\0' != g_szRemoteAddr[0] ) {

            //
            // Get address from formated address-string of the remote device and
            // run the application in client mode
            //
            int iAddrLen = sizeof(RemoteBthAddr);
            ulRetCode = WSAStringToAddressW(g_szRemoteAddr,
                                               AF_BTH,
                                               NULL,
                                               (LPSOCKADDR)&RemoteBthAddr,
                                               &iAddrLen);
            if ( CXN_SUCCESS != ulRetCode ) {
                wprintf(L"-FATAL- | Unable to get address of the remote radio having formated address-string %s\n", g_szRemoteAddr);
            }

            if ( CXN_SUCCESS == ulRetCode ) {
                ulRetCode = RunClientMode(RemoteBthAddr, g_ulMaxCxnCycles);
            }

        } else {
            //
            // No remote name/address specified.  Run the application in server mode
            //
            ulRetCode = RunServerMode(g_ulMaxCxnCycles);
        }
    }

    return(int)ulRetCode;
}


//
// NameToBthAddr converts a bluetooth device name to a bluetooth address,
// if required by performing inquiry with remote name requests.
// This function demonstrates device inquiry, with optional LUP flags.
//
ULONG NameToBthAddr(__in const LPWSTR pszRemoteName, __out PSOCKADDR_BTH pRemoteBtAddr)
{
    INT             iResult = CXN_SUCCESS;
    BOOL            bContinueLookup = FALSE, bRemoteDeviceFound = FALSE;
    ULONG           ulFlags = 0, ulPQSSize = sizeof(WSAQUERYSET);
    HANDLE          hLookup = NULL;
    PWSAQUERYSET    pWSAQuerySet = NULL;

    pWSAQuerySet = (PWSAQUERYSET) HeapAlloc(GetProcessHeap(),
                                            HEAP_ZERO_MEMORY,
                                            ulPQSSize);
    if ( NULL == pWSAQuerySet ) {
        iResult = STATUS_NO_MEMORY;
        wprintf(L"!ERROR! | Unable to allocate memory for WSAQUERYSET\n");
    }

    //
    // Search for the device with the correct name
    //
    if ( CXN_SUCCESS == iResult) {

        for ( INT iRetryCount = 0;
            !bRemoteDeviceFound && (iRetryCount < CXN_MAX_INQUIRY_RETRY);
            iRetryCount++ ) {
            //
            // WSALookupService is used for both service search and device inquiry
            // LUP_CONTAINERS is the flag which signals that we're doing a device inquiry.
            //
            ulFlags = LUP_CONTAINERS;

            //
            // Friendly device name (if available) will be returned in lpszServiceInstanceName
            //
            ulFlags |= LUP_RETURN_NAME;

            //
            // BTH_ADDR will be returned in lpcsaBuffer member of WSAQUERYSET
            //
            ulFlags |= LUP_RETURN_ADDR;

            if ( 0 == iRetryCount ) {
                wprintf(L"*INFO* | Inquiring device from cache...\n");
            } else {
                //
                // Flush the device cache for all inquiries, except for the first inquiry
                //
                // By setting LUP_FLUSHCACHE flag, we're asking the lookup service to do
                // a fresh lookup instead of pulling the information from device cache.
                //
                ulFlags |= LUP_FLUSHCACHE;

                //
                // Pause for some time before all the inquiries after the first inquiry
                //
                // Remote Name requests will arrive after device inquiry has
                // completed.  Without a window to receive IN_RANGE notifications,
                // we don't have a direct mechanism to determine when remote
                // name requests have completed.
                //
                wprintf(L"*INFO* | Unable to find device.  Waiting for %d seconds before re-inquiry...\n", CXN_DELAY_NEXT_INQUIRY);
                Sleep(CXN_DELAY_NEXT_INQUIRY * 1000);

                wprintf(L"*INFO* | Inquiring device ...\n");
            }

            //
            // Start the lookup service
            //
            iResult = CXN_SUCCESS;
            hLookup = 0;
            bContinueLookup = FALSE;
            ZeroMemory(pWSAQuerySet, ulPQSSize);
            pWSAQuerySet->dwNameSpace = NS_BTH;
            pWSAQuerySet->dwSize = sizeof(WSAQUERYSET);
            iResult = WSALookupServiceBegin(pWSAQuerySet, ulFlags, &hLookup);

            //
            // Even if we have an error, we want to continue until we
            // reach the CXN_MAX_INQUIRY_RETRY
            //
            if ( (NO_ERROR == iResult) && (NULL != hLookup) ) {
                bContinueLookup = TRUE;
            } else if ( 0 < iRetryCount ) {
                wprintf(L"=CRITICAL= | WSALookupServiceBegin() failed with error code %d, WSAGetLastError = %d\n", iResult, WSAGetLastError());
                break;
            }

            while ( bContinueLookup ) {
                //
                // Get information about next bluetooth device
                //
                // Note you may pass the same WSAQUERYSET from LookupBegin
                // as long as you don't need to modify any of the pointer
                // members of the structure, etc.
                //
                // ZeroMemory(pWSAQuerySet, ulPQSSize);
                // pWSAQuerySet->dwNameSpace = NS_BTH;
                // pWSAQuerySet->dwSize = sizeof(WSAQUERYSET);
                if ( NO_ERROR == WSALookupServiceNext(hLookup,
                                                      ulFlags,
                                                      &ulPQSSize,
                                                      pWSAQuerySet) ) {
                    
                    //
                    // Compare the name to see if this is the device we are looking for.
                    //
                    if ( ( pWSAQuerySet->lpszServiceInstanceName != NULL ) &&
                         ( CXN_SUCCESS == _wcsicmp(pWSAQuerySet->lpszServiceInstanceName, pszRemoteName) ) ) {
                        //
                        // Found a remote bluetooth device with matching name.
                        // Get the address of the device and exit the lookup.
                        //
                        CopyMemory(pRemoteBtAddr,
                                   (PSOCKADDR_BTH) pWSAQuerySet->lpcsaBuffer->RemoteAddr.lpSockaddr,
                                   sizeof(*pRemoteBtAddr));
                        bRemoteDeviceFound = TRUE;
                        bContinueLookup = FALSE;
                    }
                } else {
                    iResult = WSAGetLastError();
                    if ( WSA_E_NO_MORE == iResult ) { //No more data
                        //
                        // No more devices found.  Exit the lookup.
                        //
                        bContinueLookup = FALSE;
                    } else if ( WSAEFAULT == iResult ) {
                        //
                        // The buffer for QUERYSET was insufficient.
                        // In such case 3rd parameter "ulPQSSize" of function "WSALookupServiceNext()" receives
                        // the required size.  So we can use this parameter to reallocate memory for QUERYSET.
                        //
                        HeapFree(GetProcessHeap(), 0, pWSAQuerySet);
                        pWSAQuerySet = (PWSAQUERYSET) HeapAlloc(GetProcessHeap(),
                                                                HEAP_ZERO_MEMORY,
                                                                ulPQSSize);
                        if ( NULL == pWSAQuerySet ) {
                            wprintf(L"!ERROR! | Unable to allocate memory for WSAQERYSET\n");
                            bContinueLookup = FALSE;
                        }
                    } else {
                        wprintf(L"=CRITICAL= | WSALookupServiceNext() failed with error code %d\n", iResult);
                        bContinueLookup = FALSE;
                    }
                }
            }

            //
            // End the lookup service
            //
            WSALookupServiceEnd(hLookup);
        }
    }

    if ( NULL != pWSAQuerySet ) {
        HeapFree(GetProcessHeap(), 0, pWSAQuerySet);
        pWSAQuerySet = NULL;
    }

    if ( bRemoteDeviceFound ) {
        iResult = CXN_SUCCESS;
    } else {
        iResult = CXN_ERROR;
    }

    return iResult;
}

//
// RunClientMode runs the application in client mode.  It opens a socket, connects it to a
// remote socket, transfer some data over the connection and closes the connection.
//
ULONG RunClientMode(__in SOCKADDR_BTH RemoteAddr, __in int iMaxCxnCycles)
{
    ULONG           ulRetCode = CXN_SUCCESS;
    int             iCxnCount = 0;
    wchar_t         *pszData = NULL;
    SOCKET          LocalSocket = INVALID_SOCKET;
    SOCKADDR_BTH    SockAddrBthServer = RemoteAddr;
    HRESULT         res;

    pszData = (wchar_t *) HeapAlloc(GetProcessHeap(),
                                 HEAP_ZERO_MEMORY,
                                 CXN_TRANSFER_DATA_LENGTH);
    if ( NULL == pszData ) {
        ulRetCode = STATUS_NO_MEMORY;
        wprintf(L"=CRITICAL= | HeapAlloc failed | out of memory, gle = [%d] \n", GetLastError());
    }

    if ( CXN_SUCCESS == ulRetCode ) {
        //
        // Setting address family to AF_BTH indicates winsock2 to use Bluetooth sockets
        // Port should be set to 0 if ServiceClassId is spesified.
        //
        SockAddrBthServer.addressFamily = AF_BTH;
        SockAddrBthServer.serviceClassId = g_guidServiceClass;
        SockAddrBthServer.port = 0;

        //
        // Create a static data-string, which will be transferred to the remote
        // Bluetooth device
        //
        res = StringCbCopyN(pszData, CXN_TRANSFER_DATA_LENGTH, CXN_TEST_DATA_STRING, CXN_TRANSFER_DATA_LENGTH);
        if ( FAILED(res) ) {
            wprintf(L"=CRITICAL= | Creating a static data string failed\n");
            ulRetCode = CXN_ERROR;
        }

    }

    if ( CXN_SUCCESS == ulRetCode ) {
    
        pszData[(CXN_TRANSFER_DATA_LENGTH/sizeof(wchar_t)) - 1] = 0;

        //
        // Run the connection/data-transfer for user specified number of cycles
        //
        for ( iCxnCount = 0;
            (0 == ulRetCode) && (iCxnCount < iMaxCxnCycles || iMaxCxnCycles == 0);
            iCxnCount++ ) {

            wprintf(L"\n");

            //
            // Open a bluetooth socket using RFCOMM protocol
            //
            LocalSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
            if ( INVALID_SOCKET == LocalSocket ) {
                wprintf(L"=CRITICAL= | socket() call failed. WSAGetLastError = [%d]\n", WSAGetLastError());
                ulRetCode = CXN_ERROR;
                break;
            }

            //
            // Connect the socket (pSocket) to a given remote socket represented by address (pServerAddr)
            //
            if ( SOCKET_ERROR == connect(LocalSocket,
                                         (struct sockaddr *) &SockAddrBthServer,
                                         sizeof(SOCKADDR_BTH)) ) {
                wprintf(L"=CRITICAL= | connect() call failed. WSAGetLastError=[%d]\n", WSAGetLastError());
                ulRetCode = CXN_ERROR;
                break;
            }

            //
            // send() call indicates winsock2 to send the given data
            // of a specified length over a given connection.
            //
            wprintf(L"*INFO* | Sending following data string:\n%s\n", pszData);
            if ( SOCKET_ERROR == send(LocalSocket,
                                      (char *)pszData,
                                      CXN_TRANSFER_DATA_LENGTH,
                                      0) ) {
                wprintf(L"=CRITICAL= | send() call failed w/socket = [0x%I64X], szData = [%p], dataLen = [%I64d]. WSAGetLastError=[%d]\n", LocalSocket, pszData, CXN_TRANSFER_DATA_LENGTH, WSAGetLastError());
                ulRetCode = CXN_ERROR;
                break;
            }

            //
            // Close the socket
            //
            if ( SOCKET_ERROR == closesocket(LocalSocket) ) {
                wprintf(L"=CRITICAL= | closesocket() call failed w/socket = [0x%I64X]. WSAGetLastError=[%d]\n", LocalSocket, WSAGetLastError());
                ulRetCode = CXN_ERROR;
                break;
            }

            LocalSocket = INVALID_SOCKET;

        }
    }

    if ( INVALID_SOCKET != LocalSocket ) {
        closesocket(LocalSocket);
        LocalSocket = INVALID_SOCKET;
    }

    if ( NULL != pszData ) {
        HeapFree(GetProcessHeap(), 0, pszData);
        pszData = NULL;
    }

    return(ulRetCode);
}

//
// RunServerMode runs the application in server mode.  It opens a socket, connects it to a
// remote socket, transfer some data over the connection and closes the connection.
//

#define CXN_INSTANCE_STRING L"Sample Bluetooth Server"

ULONG RunServerMode(__in int iMaxCxnCycles)
{
    ULONG           ulRetCode = CXN_SUCCESS;
    int             iAddrLen = sizeof(SOCKADDR_BTH);
    int             iCxnCount = 0;
    UINT            iLengthReceived = 0;
    UINT            uiTotalLengthReceived;
    size_t          cbInstanceNameSize = 0;
    char *          pszDataBuffer = NULL;
    char *          pszDataBufferIndex = NULL;
    wchar_t *       pszInstanceName = NULL;
    wchar_t         szThisComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD           dwLenComputerName = MAX_COMPUTERNAME_LENGTH + 1;
    SOCKET          LocalSocket = INVALID_SOCKET;
    SOCKET          ClientSocket = INVALID_SOCKET;
    WSAQUERYSET     wsaQuerySet = {0};
    SOCKADDR_BTH    SockAddrBthLocal = {0};
    LPCSADDR_INFO   lpCSAddrInfo = NULL;
    HRESULT         res;

    //
    // This fixed-size allocation can be on the stack assuming the
    // total doesn't cause a stack overflow (depends on your compiler settings)
    // However, they are shown here as dynamic to allow for easier expansion
    //
    lpCSAddrInfo = (LPCSADDR_INFO) HeapAlloc( GetProcessHeap(),
                                              HEAP_ZERO_MEMORY,
                                              sizeof(CSADDR_INFO) );
    if ( NULL == lpCSAddrInfo ) {
        wprintf(L"!ERROR! | Unable to allocate memory for CSADDR_INFO\n");
        ulRetCode = CXN_ERROR;
    }

    if ( CXN_SUCCESS == ulRetCode ) {

        if ( !GetComputerName(szThisComputerName, &dwLenComputerName) ) {
            wprintf(L"=CRITICAL= | GetComputerName() call failed. WSAGetLastError=[%d]\n", WSAGetLastError());
            ulRetCode = CXN_ERROR;
        }
    }

    //
    // Open a bluetooth socket using RFCOMM protocol
    //
    if ( CXN_SUCCESS == ulRetCode ) {
        LocalSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
        if ( INVALID_SOCKET == LocalSocket ) {
            wprintf(L"=CRITICAL= | socket() call failed. WSAGetLastError = [%d]\n", WSAGetLastError());
            ulRetCode = CXN_ERROR;
        }
    }

    if ( CXN_SUCCESS == ulRetCode ) {

        //
        // Setting address family to AF_BTH indicates winsock2 to use Bluetooth port
        //
        SockAddrBthLocal.addressFamily = AF_BTH;
        SockAddrBthLocal.port = BT_PORT_ANY;

        //
        // bind() associates a local address and port combination
        // with the socket just created. This is most useful when
        // the application is a server that has a well-known port
        // that clients know about in advance.
        //
        if ( SOCKET_ERROR == bind(LocalSocket,
                                  (struct sockaddr *) &SockAddrBthLocal,
                                  sizeof(SOCKADDR_BTH) ) ) {
            wprintf(L"=CRITICAL= | bind() call failed w/socket = [0x%I64X]. WSAGetLastError=[%d]\n", LocalSocket, WSAGetLastError());
            ulRetCode = CXN_ERROR;
        }
    }

    if ( CXN_SUCCESS == ulRetCode ) {

        ulRetCode = getsockname(LocalSocket,
                                (struct sockaddr *)&SockAddrBthLocal,
                                &iAddrLen);
        if ( SOCKET_ERROR == ulRetCode ) {
            wprintf(L"=CRITICAL= | getsockname() call failed w/socket = [0x%I64X]. WSAGetLastError=[%d]\n", LocalSocket, WSAGetLastError());
            ulRetCode = CXN_ERROR;
        }
    }

    if ( CXN_SUCCESS == ulRetCode ) {
        //
        // CSADDR_INFO
        //
        lpCSAddrInfo[0].LocalAddr.iSockaddrLength = sizeof( SOCKADDR_BTH );
        lpCSAddrInfo[0].LocalAddr.lpSockaddr = (LPSOCKADDR)&SockAddrBthLocal;
        lpCSAddrInfo[0].RemoteAddr.iSockaddrLength = sizeof( SOCKADDR_BTH );
        lpCSAddrInfo[0].RemoteAddr.lpSockaddr = (LPSOCKADDR)&SockAddrBthLocal;
        lpCSAddrInfo[0].iSocketType = SOCK_STREAM;
        lpCSAddrInfo[0].iProtocol = BTHPROTO_RFCOMM;

        //
        // If we got an address, go ahead and advertise it.
        //
        ZeroMemory(&wsaQuerySet, sizeof(WSAQUERYSET));
        wsaQuerySet.dwSize = sizeof(WSAQUERYSET);
        wsaQuerySet.lpServiceClassId = (LPGUID) &g_guidServiceClass;

        //
        // Adding a byte to the size to account for the space in the
        // format string in the swprintf call. This will have to change if converted
        // to UNICODE
        //
        res = StringCchLength(szThisComputerName, sizeof(szThisComputerName), &cbInstanceNameSize);
        if( FAILED(res) ) {
            wprintf(L"-FATAL- | ComputerName specified is too large\n");
            ulRetCode = CXN_ERROR;
        }
    }

    if ( CXN_SUCCESS == ulRetCode ) {
        cbInstanceNameSize += sizeof(CXN_INSTANCE_STRING) + 1;
        pszInstanceName = (LPWSTR)HeapAlloc(GetProcessHeap(),
                                           HEAP_ZERO_MEMORY,
                                           cbInstanceNameSize);
        if ( NULL == pszInstanceName ) {
            wprintf(L"-FATAL- | HeapAlloc failed | out of memory | gle = [%d] \n", GetLastError());
            ulRetCode = CXN_ERROR;
        }
    }

    if ( CXN_SUCCESS == ulRetCode ) {
        StringCbPrintf(pszInstanceName, cbInstanceNameSize, L"%s %s", szThisComputerName, CXN_INSTANCE_STRING);
        wsaQuerySet.lpszServiceInstanceName = pszInstanceName;
        wsaQuerySet.lpszComment = L"Example Service instance registered in the directory service through RnR";
        wsaQuerySet.dwNameSpace = NS_BTH;
        wsaQuerySet.dwNumberOfCsAddrs = 1;      // Must be 1.
        wsaQuerySet.lpcsaBuffer = lpCSAddrInfo; // Req'd.

        //
        // As long as we use a blocking accept(), we will have a race
        // between advertising the service and actually being ready to
        // accept connections.  If we use non-blocking accept, advertise
        // the service after accept has been called.
        //
        if ( SOCKET_ERROR == WSASetService(&wsaQuerySet, RNRSERVICE_REGISTER, 0) ) {
            wprintf(L"=CRITICAL= | WSASetService() call failed. WSAGetLastError=[%d]\n", WSAGetLastError());
            ulRetCode = CXN_ERROR;
        }
    }

    //
    // listen() call indicates winsock2 to listen on a given socket for any incoming connection.
    //
    if ( CXN_SUCCESS == ulRetCode ) {
        if ( SOCKET_ERROR == listen(LocalSocket, CXN_DEFAULT_LISTEN_BACKLOG) ) {
            wprintf(L"=CRITICAL= | listen() call failed w/socket = [0x%I64X]. WSAGetLastError=[%d]\n", LocalSocket, WSAGetLastError());
            ulRetCode = CXN_ERROR;
        }
    }

    if ( CXN_SUCCESS == ulRetCode ) {

        for ( iCxnCount = 0;
            (CXN_SUCCESS == ulRetCode) && ( (iCxnCount < iMaxCxnCycles) || (iMaxCxnCycles == 0) );
            iCxnCount++ ) {

            wprintf(L"\n");

            //
            // accept() call indicates winsock2 to wait for any
            // incoming connection request from a remote socket.
            // If there are already some connection requests on the queue,
            // then accept() extracts the first request and creates a new socket and
            // returns the handle to this newly created socket. This newly created
            // socket represents the actual connection that connects the two sockets.
            //
            ClientSocket = accept(LocalSocket, NULL, NULL);
            if ( INVALID_SOCKET == ClientSocket ) {
                wprintf(L"=CRITICAL= | accept() call failed. WSAGetLastError=[%d]\n", WSAGetLastError());
                ulRetCode = CXN_ERROR;
                break; // Break out of the for loop
            }

            //
            // Read data from the incoming stream
            //
            BOOL bContinue = TRUE;
            pszDataBuffer = (char *)HeapAlloc(GetProcessHeap(),
                                              HEAP_ZERO_MEMORY,
                                              CXN_TRANSFER_DATA_LENGTH);
            if ( NULL == pszDataBuffer ) {
                wprintf(L"-FATAL- | HeapAlloc failed | out of memory | gle = [%d] \n", GetLastError());
                ulRetCode = CXN_ERROR;
                break;
            }
            pszDataBufferIndex = pszDataBuffer;
            uiTotalLengthReceived = 0;
            while ( bContinue && (uiTotalLengthReceived < CXN_TRANSFER_DATA_LENGTH) ) {
                //
                // recv() call indicates winsock2 to receive data
                // of an expected length over a given connection.
                // recv() may not be able to get the entire length
                // of data at once.  In such case the return value,
                // which specifies the number of bytes received,
                // can be used to calculate how much more data is
                // pending and accordingly recv() can be called again.
                //
                iLengthReceived = recv(ClientSocket,
                                       (char *)pszDataBufferIndex,
                                       (CXN_TRANSFER_DATA_LENGTH - uiTotalLengthReceived),
                                       0);

                switch ( iLengthReceived ) {
                case 0: // socket connection has been closed gracefully
                    bContinue = FALSE;
                    break;

                case SOCKET_ERROR:
                    wprintf(L"=CRITICAL= | recv() call failed. WSAGetLastError=[%d]\n", WSAGetLastError());
                    bContinue = FALSE;
                    ulRetCode = CXN_ERROR;
                    break;

                default:

                    //
                    // Make sure we have enough room
                    //
                    if ( iLengthReceived > (CXN_TRANSFER_DATA_LENGTH - uiTotalLengthReceived)) {
                        wprintf(L"=CRITICAL= | received too much data\n");
                        bContinue = FALSE;
                        ulRetCode = CXN_ERROR;
                        break;
                    }

                    pszDataBufferIndex += iLengthReceived;
                    uiTotalLengthReceived += iLengthReceived;
                    break;
                }
            }

            if ( CXN_SUCCESS == ulRetCode ) {

                if ( CXN_TRANSFER_DATA_LENGTH != uiTotalLengthReceived ) {
                    wprintf(L"+WARNING+ | Data transfer aborted mid-stream. Expected Length = [%I64d], Actual Length = [%d]\n", CXN_TRANSFER_DATA_LENGTH, uiTotalLengthReceived);
                }
                wprintf(L"*INFO* | Received following data string from remote device:\n%s\n", (wchar_t *)pszDataBuffer);

                //
                // Close the connection
                //
                if ( SOCKET_ERROR == closesocket(ClientSocket) ) {
                    wprintf(L"=CRITICAL= | closesocket() call failed w/socket = [0x%I64X]. WSAGetLastError=[%d]\n", LocalSocket, WSAGetLastError());
                    ulRetCode = CXN_ERROR;
                } else {
                    //
                    // Make the connection invalid regardless
                    //
                    ClientSocket = INVALID_SOCKET;
                }
            }
        }
    }

    if ( INVALID_SOCKET != ClientSocket ) {
        closesocket(ClientSocket);
        ClientSocket = INVALID_SOCKET;
    }

    if ( INVALID_SOCKET != LocalSocket ) {
        closesocket(LocalSocket);
        LocalSocket = INVALID_SOCKET;
    }

    if ( NULL != lpCSAddrInfo ) {
        HeapFree(GetProcessHeap(), 0, lpCSAddrInfo);
        lpCSAddrInfo = NULL;
    }
    if ( NULL != pszInstanceName ) {
        HeapFree(GetProcessHeap(), 0, pszInstanceName);
        pszInstanceName = NULL;
    }

    if ( NULL != pszDataBuffer ) {
        HeapFree(GetProcessHeap(), 0, pszDataBuffer);
        pszDataBuffer = NULL;
    }

    return(ulRetCode);
}

//
// ShowCmdLineSyntaxHelp displays the command line usage
//
void ShowCmdLineHelp(void)
{
    wprintf(
          L"\n  Bluetooth Connection Sample application for demonstrating connection and data transfer."
          L"\n"
          L"\n"
          L"\n  BTHCxn.exe  [-n<RemoteName> | -a<RemoteAddress>] "
          L"\n                  [-c<ConnectionCycles>]"
          L"\n"
          L"\n"
          L"\n  Switches applicable for Client mode:"
          L"\n    -n<RemoteName>        Specifies name of remote BlueTooth-Device."
          L"\n"
          L"\n    -a<RemoteAddress>     Specifies address of remote BlueTooth-Device."
          L"\n                          The address is in form XX:XX:XX:XX:XX:XX"
          L"\n                          where XX is a hexidecimal byte"
          L"\n"
          L"\n                          One of the above two switches is required for client."
          L"\n"
          L"\n"
          L"\n  Switches applicable for both Client and Server mode:"
          L"\n    -c<ConnectionCycles>  Specifies number of connection cycles."
          L"\n                          Default value for this parameter is 1.  Specify 0 to "
          L"\n                          run infinite number of connection cycles."
          L"\n"
          L"\n"
          L"\n"
          L"\n  Command Line Examples:"
          L"\n    \"BTHCxn.exe -c0\""
          L"\n    Runs the BTHCxn server for infinite connection cycles."
          L"\n    The application reports minimal information onto the cmd window."
          L"\n"
          L"\n    \"BTHCxn.exe -nServerDevice -c50\""
          L"\n    Runs the BTHCxn client connecting to remote device (having name "
          L"\n    \"ServerDevice\" for 50 connection cycles."
          L"\n    The application reports minimal information onto the cmd window."
          L"\n"
          );
}

//
// ParseCmdLine parses the command line and sets the global variables accordingly.
// It returns CXN_SUCCESS if successful and CXN_ERROR if it detected a mistake in the
// command line parameter used.
//
ULONG ParseCmdLine (__in int argc, __in_ecount(argc) wchar_t * argv[])
{
    int     cbStrLen = 0;
    ULONG   ulRetCode = CXN_SUCCESS;
    HRESULT res;

    for ( int i = 1; i < argc; i++ ) {
        wchar_t * pszToken = argv[i];
        if ( *pszToken == L'-' || *pszToken == L'/' ) {
            wchar_t token;

            //
            // skip over the "-" or "/"
            //
            pszToken++;

            //
            // Get the command line option
            //
            token = *pszToken;

            //
            // Go one past the option the option-data
            //
            pszToken++;

            //
            // Get the option-data
            //
            switch ( token ) {
            case L'n':
                cbStrLen = lstrlen(pszToken);
                if ( ( 0 < cbStrLen ) && ( BTH_MAX_NAME_SIZE >= cbStrLen ) ) {
                    res = StringCbCopy(g_szRemoteName, sizeof(g_szRemoteName), pszToken);
                    if ( FAILED(res) ) {
                        ulRetCode = CXN_ERROR;
                        wprintf(L"!ERROR! | cmd line | Unable to parse -n<RemoteName>, length error (min 1 char, max %d chars)\n", BTH_MAX_NAME_SIZE);
                    }
                } else {
                    ulRetCode = CXN_ERROR;
                    wprintf(L"!ERROR! | cmd line | Unable to parse -n<RemoteName>, length error (min 1 char, max %d chars)\n", BTH_MAX_NAME_SIZE);
                }
                break;

            case L'a':
                cbStrLen = lstrlen(pszToken);
                if ( CXN_BDADDR_STR_LEN == cbStrLen ) {
                    res = StringCbCopy(g_szRemoteAddr, sizeof(g_szRemoteAddr), pszToken);
                    if ( FAILED(res) ) {
                        ulRetCode = CXN_ERROR;
                        wprintf(L"!ERROR! | cmd line | Unable to parse -a<RemoteAddress>, Remote bluetooth radio address string length expected %d | Found: %d)\n", CXN_BDADDR_STR_LEN, cbStrLen);
                    }
                } else {
                    ulRetCode = CXN_ERROR;
                    wprintf(L"!ERROR! | cmd line | Unable to parse -a<RemoteAddress>, Remote bluetooth radio address string length expected %d | Found: %d)\n", CXN_BDADDR_STR_LEN, cbStrLen);
                }
                break;

            case L'c':
                if ( 0 < lstrlen(pszToken) ) {
                    swscanf_s(pszToken, L"%d", &g_ulMaxCxnCycles);
                    if ( 0 > g_ulMaxCxnCycles ) {
                        ulRetCode = CXN_ERROR;
                        wprintf(L"!ERROR! | cmd line | Must provide +ve or 0 value with -c option\n");
                    }
                } else {
                    ulRetCode = CXN_ERROR;
                    wprintf(L"!ERROR! | cmd line | Must provide a value with -c option\n");
                }
                break;

            case L'?':
            case L'h':
            case L'H':
            default:
                ulRetCode = CXN_ERROR;
            }
        } else {
            ulRetCode = CXN_ERROR;
            wprintf(L"!ERROR! | cmd line | Bad option prefix, use '/' or '-' \n");
        }
    }

    return(ulRetCode);
}