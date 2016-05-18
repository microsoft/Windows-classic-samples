/******************************************************************************\
* isb.cpp
*
* This simple IPv6 sample demonstrates the use of Winsock Ideal Send Backlog functionality.
*
* The Ideal Send Backlog functionality is new to Windows Sockets in Windows Vista SP1
* and Windows Server 2008.
* 
* This sample requires that TCP/IP version 6 be installed on the system (default
* configuration for Windows Vista and Windows Server 2008).
*
* For an application to effectively use this functionality, the application should: 
* 1. First query for the initial ideal send backlog value (idealsendbacklogquery).
* 2. Post overlapped idealsendbacklognotify to receive isb change indications. 
* 3. Upon notify completion, immediately post another idealsendbacklognotify and
*    query for new isb value (idealsendbacklogquery).
* 
* The I/O model in the sample uses blocking send/recv calls to keep the sample simple.
* A real world application could use non-blocking or overlapped I/O for possibly better performance.
* 
* Note: 
* On Windows 7/Server 2008 R2 and later versions, a new feature called Send Path Auto-Tuning is available.
* With this new functionaly, Windows can perform the send auto-tuning (ideal send backlog)
* on the application's behalf. To enable this functionality, the application:
* 1. Must not change the connected socket's send buffer limit (SO_SNDBUF).
* 2. Must not query for the ideal send backlog value. In other words, application must 
*    not call idealsendbacklogquery.
* 
* If application does either of the above, the send auto-tuning will be disabled for that connection. 
* 
* The appication may however, post idealsendbacklognotify to receive indications that a send auto-tuning
* event has occurred.
* 
*
*
* This is a part of the Microsoft Source Code Samples.
* Copyright 1996 - 2009 Microsoft Corporation.
* All rights reserved.
* This source code is only intended as a supplement to
* Microsoft Development Tools and/or WinHelp documentation.
* See these sources for detailed information regarding the
* Microsoft samples programs.
\******************************************************************************/


#include "stdafx.h"

#define ERR(e) \
        _tprintf(TEXT("\n%s failed: %d\n"),e,WSAGetLastError())

#define MALLOC(x) \
        HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,x)

#define FREE(p) \
        if(NULL != p) \
        {\
        HeapFree(GetProcessHeap(),0,p); \
        p = NULL;\
        }

#define CLOSESOCK(s) \
        if(INVALID_SOCKET != s) \
        {\
        closesocket(s); \
        s = INVALID_SOCKET; \
        }

#define FREEADDRINFO(p) \
        if(NULL != p) \
        {\
        FreeAddrInfo(p); \
        p = NULL; \
        }

#define WS_VER 0x0202 //Latest Winsock version (v2.2)

#define RECV_MAX_BUFFER_SIZE (16*(1024*1024)) //16MB 


VOID Usage()
{
    _tprintf(TEXT("Usage: isb -l -n <ListenerAddress> -e <port> -a\n") \
             TEXT("-l act as listener\n") \
             TEXT("-n act as originator\n")
             TEXT("-a enable auto-tuning\n")
             );
    ExitProcess(1);
}

BOOL    bListener = FALSE,
        bOriginator = FALSE,
        bAutoTuning = FALSE;
_TCHAR  szServerName[MAX_PATH];
USHORT  port = 0;

VOID ValidateArgs(int argc, _TCHAR** argv)
{
    int i;

    if (argc < 2)
    {
        Usage();
    }

    for (i=1; i < argc ;i++)
    {

        if (lstrlen(argv[i]) < 2)
            continue;

        if ((argv[i][0] == TEXT('-') ) || (argv[i][0] == TEXT('/') ))
        {

            switch (argv[i][1])
            {

            case TEXT('l'):
                bListener = TRUE;
                break;
            case TEXT('n'):
                bOriginator = TRUE;
                StringCbCopy(szServerName,
                             sizeof szServerName,
                             argv[i+1]
                             );
                break;
            case TEXT('e'):
                port = (USHORT)_ttoi(argv[i+1]);
                break;
            case TEXT('a'):
                bAutoTuning = TRUE;
                break;
            default:
                Usage();
                ExitProcess(1);
                break;
            }
        }
    }

    if ((bListener && bOriginator) ||
        ((!bListener) && (!bOriginator)) )
    {
        Usage();
    }
}

/*
DualStackSocket returns a socket which can be used for either v4 or v6 connections
on Vista and later, by setting the IPV6_V6ONLY option value to zero.
*/
SOCKET DualStackSocket(INT SockType)
{
    SOCKET  sock = INVALID_SOCKET;
    INT     off = 0;

    __try
    {
        if (INVALID_SOCKET == (sock = socket(AF_INET6,
                                             SockType,
                                             0
                                             )))
        {
            ERR(TEXT("socket"));
            __leave;
        }

        if (SOCKET_ERROR == setsockopt(sock,
                                       IPPROTO_IPV6,
                                       IPV6_V6ONLY,
                                       (char*)&off,
                                       sizeof off
                                       ))
        {
            ERR(TEXT("setsockopt"));
            CLOSESOCK(sock);
            __leave;
        }
    }
    __finally
    {
    }

    return sock;
}


HANDLE hExit = NULL;

BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType)
{
    BOOL bHandled = FALSE;

    switch (dwCtrlType)
    {
    case CTRL_C_EVENT:
        SetEvent(hExit);
        bHandled = TRUE;
        break;
    default:
        break;
    }

    return bHandled;
}


int _tmain(int argc, _TCHAR** argv)
{
    WSADATA             wsd;
    ADDRINFOT           hints = {0}, 
                        *res = NULL;
    SOCKADDR_STORAGE    lAddr = {0};
    SOCKET              sl = INVALID_SOCKET,
                        sc = INVALID_SOCKET,
                        sa = INVALID_SOCKET;
    PCHAR               buffer = NULL;
    WSAOVERLAPPED       ov = {0};
    HANDLE              hEvents[2]; 
    INT                 nStartup = 0,
                        rc = 0,
                        err = 0,
                        isb = RECV_MAX_BUFFER_SIZE, //Ideal Send Backlog value
                        rcvsize = 0, //recv window size
                        timeout = 5000; //recv timeout val
    _TCHAR              szPort[6];
    _int64              TotalBytes = 0;

    ValidateArgs(argc,argv);

    //set exit/cleanup event

    hEvents[0] = hExit = CreateEvent(NULL,
                                     TRUE,
                                     FALSE,
                                     NULL
                                     );

    if (!SetConsoleCtrlHandler(ConsoleCtrlHandler,
                               TRUE
                               ))
    {
        ERR(TEXT("SetConsoleCtrlHandler"));
        return 1;
    }

    //Initialize winsock

    nStartup = WSAStartup(WS_VER,&wsd);
    if(nStartup)
        _tprintf(TEXT("WSAStartup failed: %d\n"),nStartup);
    else
        nStartup++;


    __try
    {
        //allocate our buffer 
        if (NULL == (buffer = (PCHAR)MALLOC(RECV_MAX_BUFFER_SIZE)))
        {
            ERR(TEXT("HeapAlloc"));
            __leave;
        }

        /*
        listener mode - create a dual stack socket and listen to 
        v6 addr_any on the given port.
        */
        if (bListener)
        {
            //Fill send buffer
            FillMemory(buffer,
                       RECV_MAX_BUFFER_SIZE,
                       '@'
                       );

            if (INVALID_SOCKET == (sl = DualStackSocket(SOCK_STREAM)))
            {
                ERR(TEXT("DualStackSocket"));
                __leave;
            }

            lAddr.ss_family = AF_INET6;
            INETADDR_SETANY((PSOCKADDR)&lAddr);
            SS_PORT((PSOCKADDR)&lAddr) = htons(port);

            if (SOCKET_ERROR == bind(sl,(PSOCKADDR)&lAddr,
                                     sizeof lAddr
                                     ))
            {
                ERR(TEXT("bind"));
                __leave;
            }

            if (SOCKET_ERROR == listen(sl,
                                       1
                                       ))
            {
                ERR(TEXT("listen"));
                __leave;
            }

            if (INVALID_SOCKET == (sa = accept(sl,
                                               NULL,
                                               NULL
                                               )))
            {
                ERR(TEXT("accept"));
                __leave;
            }

            if (!bAutoTuning)
            {

                //query for initial isb
                if (SOCKET_ERROR == (idealsendbacklogquery(sa,
                                                           (PULONG)&isb
                                                          )))
                {
                    ERR(TEXT("idealsendbacklogquery"));
                    __leave;
                }
            }

            hEvents[1] = ov.hEvent = CreateEvent(NULL,
                                                 FALSE,
                                                 FALSE,
                                                 NULL
                                                 );

            //post inital notify
            if (SOCKET_ERROR == idealsendbacklognotify(sa,
                                                       &ov,
                                                       NULL
                                                      ))
            {
                err = WSAGetLastError();
                if (WSA_IO_PENDING != err)
                {
                    ERR(TEXT("idealsendbacklognotify"));
                    __leave;
                }
            }

            //send data until error or exit
            for (;;)
            {
                if (SOCKET_ERROR == (rc = send(sa,
                                               buffer,
                                               isb,
                                               0
                                               )))
                {
                    ERR(TEXT("send"));
                    __leave;
                }

                TotalBytes += (_int64)rc;

                _tprintf(TEXT("Last send completed with %d bytes. Total sent: %I64d \r\r"),rc,TotalBytes);

                rc = WaitForMultipleObjects(2,
                                            hEvents,
                                            FALSE,
                                            1
                                            );

                switch (rc)
                {
                case WAIT_OBJECT_0:
                    //exit event
                    __leave;
                    break;
                case WAIT_OBJECT_0+1:
                    //immediately post another notify so we don't lose any indications
                    if (SOCKET_ERROR == idealsendbacklognotify(sa,
                                                               &ov,
                                                               NULL
                                                              ))
                    {
                        err = WSAGetLastError();
                        if (WSA_IO_PENDING != err)
                        {
                            ERR(TEXT("idealsendbacklognotify"));
                            __leave;
                        }
                    }
                    _tprintf(TEXT("\nisb change was indicated.\n"));

                    if (!bAutoTuning)
                    {

                        //query the updated backlog value
                        if (SOCKET_ERROR == idealsendbacklogquery(sa,
                                                                  (PULONG)&isb
                                                                 ))
                        {
                            ERR(TEXT("idealsendbacklogquery"));
                            __leave;
                        }
                        _tprintf(TEXT("\nisb value indicated: %d\n"),isb);
                    }
                    break;
                case WAIT_TIMEOUT:
                    break;
                case WAIT_FAILED:
                default:
                    _tprintf(TEXT("\nisb notify wait failed: %d\n"),rc);
                    __leave;
                    break;
                }
            }
        }
        else  //Originator
        {
            hints.ai_socktype = SOCK_STREAM;

            StringCbPrintf(szPort,
                           sizeof szPort,
                           TEXT("%d"),
                           port
                           );

            if (GetAddrInfo(szServerName,
                            szPort,
                            &hints,
                            &res
                            ))
            {
                ERR(TEXT("getaddrinfo"));
                __leave;
            }

            sc = socket(res->ai_family,
                        res->ai_socktype,
                        res->ai_protocol
                        );

            if (SOCKET_ERROR == connect(sc,
                                        res->ai_addr,
                                        (int)res->ai_addrlen
                                        ))
            {
                ERR(TEXT("connect"));
                __leave;
            }

            if (SOCKET_ERROR == setsockopt(sc,
                                           SOL_SOCKET,
                                           SO_RCVTIMEO,
                                           (PCHAR)&timeout,
                                           sizeof timeout
                                           ))
            {
                ERR(TEXT("SO_RCVTIMEO"));
                __leave;
            }

            //set a large receive window 
            rcvsize = RECV_MAX_BUFFER_SIZE;
            if (SOCKET_ERROR == (rc = setsockopt(sc,
                                                 SOL_SOCKET,
                                                 SO_RCVBUF,
                                                 (PCHAR)&rcvsize,
                                                 sizeof rcvsize
                                                 )))
            {
                ERR(TEXT("SO_RCVBUF"));
                __leave;
            }
            
            //recv data until fin, error or exit
            for (;;)
            {
                if (WAIT_OBJECT_0 == WaitForSingleObject(hExit,
                                                         1
                                                         ))
                {
                    __leave;
                }

                if (SOCKET_ERROR == (rc = recv(sc,
                                               buffer,
                                               RECV_MAX_BUFFER_SIZE,
                                               0
                                               )))
                {
                    if (WSAETIMEDOUT == WSAGetLastError())
                    {
                        _tprintf(TEXT("\nThe last recv timed-out after 5 seconds.\n"));
                        continue;
                    }

                    ERR(TEXT("recv"));
                    __leave;
                }

                if (0 == rc)
                {
                    _tprintf(TEXT("\nFIN was indicated.\n"));
                    __leave;
                }

                TotalBytes += (_int64)rc;

                _tprintf(TEXT("Last recv completed with %d bytes. Total recvd: %I64d \r\r"),rc,TotalBytes);
                
            }

            
        }
    }
    __finally
    {
        FREE(buffer);

        CLOSESOCK(sl);
        CLOSESOCK(sc);
        CLOSESOCK(sa);

        FREEADDRINFO(res);

        SetConsoleCtrlHandler(ConsoleCtrlHandler,
                              FALSE);
    }

    if(nStartup)
        WSACleanup();

    _tprintf(TEXT("\nExiting.\n"));
    
	return 0;
}

