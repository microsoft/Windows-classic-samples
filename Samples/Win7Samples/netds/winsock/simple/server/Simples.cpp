/******************************************************************************\
* simples.c - Simple TCP/UDP server using Winsock 1.1
*       This is a part of the Microsoft Source Code Samples.
*       Copyright 1996 - 2000 Microsoft Corporation.
*       All rights reserved.
*       This source code is only intended as a supplement to
*       Microsoft Development Tools and/or WinHelp documentation.
*       See these sources for detailed information regarding the
*       Microsoft samples programs.
\******************************************************************************/
#ifdef _IA64_
    #pragma warning(disable:4127)
#endif 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <wspiapi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DEFAULT_PORT        "5001"            // Default listening port
#define DEFAULT_PROTO       SOCK_STREAM       // Default to TCP protocol
#define DEFAULT_BUFFER_LEN  4096              // Default recv buffer length

// Function prototype
DWORD WINAPI ServerThread(LPVOID lpParam);

// 
// Function: Usage
//
// Description:
//     Print usage information to the console and exits.
//
void Usage(char *progname) 
{
    fprintf(stderr,"Usage\n%s -p [protocol] -e [endpoint] -i [interface] [-4] [-6]\n"
            "Where:\n"
            "\t-p protocol   - is one of \"TCP\" or \"UDP\"\n"
            "\t-e endpoint   - is the port to listen on\n"
            "\t-i interface  - is the string local address to bind to\n"
            "\t-4            - force IPv4\n"
            "\t-6            - force IPv6\n"
            "\n"
            "Defaults are TCP,5001 and INADDR_ANY and IN6ADDR_ANY (if IPv6 present)\n",
            progname
           );
    WSACleanup();
    exit(1);
}


//
// Function: main
//
// Description:
//    Parse the command line arguments and resolve the given local interface.
//    By default we will request AF_UNSPEC which may cause getaddrinfo to return
//    multiple addresses. In this case, we will create a server socket for each
//    address returned (e.g. IPv4 and IPv6). A thread will be created for each
//    socket to service the server socket. For TCP just a single client is handled
//    at a time. For UDP, we'll simply receive a datagram and send it back to its
//    source. The main thread waits until all child threads have terminated at which
//    point it cleans up all resources and exits.
//
int __cdecl main(int argc, char **argv) 
{
    struct addrinfo  hints, 
    *results = NULL,
    *addrptr = NULL;
    WSADATA     wsaData;
    SOCKET     *server_sockets = NULL;
    HANDLE     *server_threads = NULL;
    char        hoststr[NI_MAXHOST],
    servstr[NI_MAXSERV];
    char       *interface = NULL,
    *port = DEFAULT_PORT;
    int         socket_type = DEFAULT_PROTO,
    address_family = AF_UNSPEC,     // Default to any (IPv4 or IPv6)
    socket_count = 0,
    retval,
    i;

    /* Parse arguments */
    if (argc >1)
    {
        for (i=1;i <argc;i++)
        {
            if ( (strlen(argv[i]) == 2) && ( (argv[i][0] == '-') || (argv[i][0] == '/') ) )
            {
                switch (tolower(argv[i][1]))
                {
                case '4':       // IPv4
                    address_family = AF_INET;
                    break;

                case '6':       // IPv6
                    address_family = AF_INET6;
                    break;

                case 'p':
                    if (!_strnicmp(argv[i+1], "TCP", 3) )
                        socket_type = SOCK_STREAM;
                    else if (!_strnicmp(argv[i+1], "UDP", 3) )
                        socket_type = SOCK_DGRAM;
                    else
                        Usage(argv[0]);
                    i++;
                    break;

                case 'i':       // Local interface to listen/recv on
                    interface = argv[++i];
                    break;

                case 'e':       // Port number to listen/recv on
                    port = argv[++i];
                    break;

                default:
                    Usage(argv[0]);
                    break;
                }
            }
            else
                Usage(argv[0]);
        }
    }

    // Load Winsock
    if ((retval = WSAStartup(0x202,&wsaData)) != 0)
    {
        fprintf(stderr,"WSAStartup failed with error %d\n",retval);
        WSACleanup();
        return -1;
    }

    // Make sure the supplied port isn't wildcard
    if ( _strnicmp(port, "0", 1) == 0)
    {
        Usage(argv[0]);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = address_family;
    hints.ai_socktype = socket_type;
    hints.ai_protocol = ((socket_type == SOCK_DGRAM) ? IPPROTO_UDP : IPPROTO_TCP);
    // If interface is NULL then request the passive "bind" address
    hints.ai_flags    = ((interface == NULL) ? AI_PASSIVE : 0);

    retval = getaddrinfo(interface, port, &hints, &results);
    if (retval != 0)
    {
        fprintf(stderr, "getaddrinfo failed: %d\n", retval);
        goto cleanup;
    }

    // Make sure we got at least one address back
    if (results == NULL)
    {
        fprintf(stderr, "Unable to resolve interface %s\n", interface);
        goto cleanup;
    }

    // Count how many addresses were returned
    addrptr = results;
    while (addrptr)
    {
        socket_count++;
        addrptr = addrptr->ai_next;
    }

    // Allocate space for the server sockets
    server_sockets = (SOCKET *)HeapAlloc(
                                        GetProcessHeap(), 
                                        HEAP_ZERO_MEMORY, 
                                        sizeof(SOCKET) * socket_count
                                        );
    if (server_sockets == NULL)
    {
        fprintf(stderr, "HeapAlloc failed: %d\n", GetLastError());
        goto cleanup;
    }

    // Initialize the socket array first
    for (i=0; i < socket_count ;i++)
        server_sockets[i] = INVALID_SOCKET;

    // Create the server sockets - one for each address returned
    socket_count = 0;
    addrptr = results;
    while (addrptr)
    {
        // Create the socket according to the parameters returned
        server_sockets[socket_count] = socket(
                                             addrptr->ai_family, 
                                             addrptr->ai_socktype, 
                                             addrptr->ai_protocol
                                             );
        if (server_sockets[socket_count] == INVALID_SOCKET)
        {
            fprintf(stderr, "socket failed: %d\n", WSAGetLastError());
            goto cleanup;
        }

        // Bind the socket to the address returned
        retval = bind(server_sockets[socket_count],
                      addrptr->ai_addr,
                      (int)addrptr->ai_addrlen
                     );
        if (retval == SOCKET_ERROR)
        {
            fprintf(stderr, "bind failed: %d\n", WSAGetLastError());
            goto cleanup;
        }

        // If a TCP socket, call listen on it
        if (addrptr->ai_socktype == SOCK_STREAM)
        {
            retval = listen(
                           server_sockets[socket_count],
                           7
                           );
            if (retval == SOCKET_ERROR)
            {
                fprintf(stderr, "listen failed: %d\n", WSAGetLastError());
                goto cleanup;
            }
        }

        // Print the address this socket is bound to
        retval = getnameinfo(
                            addrptr->ai_addr,
                            (socklen_t)addrptr->ai_addrlen,
                            hoststr,
                            NI_MAXHOST,
                            servstr,
                            NI_MAXSERV,
                            NI_NUMERICHOST | NI_NUMERICSERV
                            );
        if (retval != 0)
        {
            fprintf(stderr, "getnameinfo failed: %d\n", retval);
            goto cleanup;
        }

        fprintf(stdout, "socket 0x%x bound to address %s and port %s\n",
                server_sockets[socket_count], hoststr, servstr);

        // Increment the socket count again
        socket_count++;

        // Increment the address pointer
        addrptr = addrptr->ai_next;
    }

    // We need a server thread per socket so allocate space for the thread handle
    server_threads = (HANDLE *)HeapAlloc(
                                        GetProcessHeap(),
                                        HEAP_ZERO_MEMORY,
                                        sizeof(HANDLE) * socket_count
                                        );
    if (server_threads == NULL)
    {
        fprintf(stderr, "HeapAlloc failed: %d\n", GetLastError());
        goto cleanup;
    }

    // Create a thread for each address family which will handle that socket
    for (i=0; i < socket_count ;i++)
    {
        server_threads[i] = CreateThread(
                                        NULL,
                                        0,
                                        ServerThread,
                                        (LPVOID)server_sockets[i],
                                        0,
                                        NULL
                                        );
        if (server_threads[i] == NULL)
        {
            fprintf(stderr, "CreateThread failed: %d\n", GetLastError());
            goto cleanup;
        }
    }

    // Wait until the threads exit, then cleanup
    retval = WaitForMultipleObjects(
                                   socket_count,
                                   server_threads,
                                   TRUE,
                                   INFINITE
                                   );
    if ((retval == WAIT_FAILED) || (retval == WAIT_TIMEOUT))
    {
        fprintf(stderr, "WaitForMultipleObjects failed: %d\n", GetLastError());
        goto cleanup;
    }

    cleanup:

    if (results != NULL)
    {
        freeaddrinfo(results);
        results = NULL;
    }

    // Release socket resources
    if (server_sockets != NULL)
    {
        for (i=0; i < socket_count ;i++)
        {
            if (server_sockets[i] != INVALID_SOCKET)
                closesocket(server_sockets[i]);
            server_sockets[i] = INVALID_SOCKET;
        }

        // Free the array
        HeapFree(GetProcessHeap(), 0, server_sockets);
        server_sockets = NULL;
    }

    // Release thread resources
    if (server_threads != NULL)
    {
        for (i=0; i < socket_count ;i++)
        {
            if (server_threads[i] != NULL)
                CloseHandle(server_threads[i]);
            server_threads[i] = NULL;
        }

        // Free the array
        HeapFree(GetProcessHeap(), 0, server_threads);
        server_threads = NULL;
    }

    return 0;
}

//
// Function: ServerThread
//
// Description:
//    This routine services a single server socket. For TCP this means accept
//    a client connection and then recv and send in a loop. When the client
//    closes the connection, wait for another client, etc. For UDP, we simply
//    sit in a loop and recv a datagram and echo it back to its source. For any
//    error, this routine exits.
//
DWORD WINAPI ServerThread(LPVOID lpParam)
{
    SOCKET           s,                     // Server socket
    sc = INVALID_SOCKET;   // Client socket (TCP)
    SOCKADDR_STORAGE from;
    char             Buffer[DEFAULT_BUFFER_LEN],
    servstr[NI_MAXSERV],
    hoststr[NI_MAXHOST];
    int              socket_type,
    retval,
    fromlen,
    bytecount;

    // Retrieve the socket handle
    s = (SOCKET) lpParam;

    // Get the socket type back
    fromlen = sizeof(socket_type);
    retval = getsockopt(s, SOL_SOCKET, SO_TYPE, (char *)&socket_type, &fromlen);
    if (retval == INVALID_SOCKET)
    {
        fprintf(stderr, "getsockopt(SO_TYPE) failed: %d\n", WSAGetLastError());
        goto cleanup;
    }

    for (;;)
    {
        fromlen = sizeof(from);

        if (socket_type == SOCK_STREAM)
        {
            if (sc != INVALID_SOCKET)
            {
                //
                // If we have a client connection recv and send until done
                //
                bytecount = recv(sc, Buffer, DEFAULT_BUFFER_LEN, 0);
                if (bytecount == SOCKET_ERROR)
                {
                    fprintf(stderr, "recv failed: %d\n", WSAGetLastError());
                    goto cleanup;
                }
                else if (bytecount == 0)
                {
                    // Client connection was closed
                    retval = shutdown(sc, SD_SEND);
                    if (retval == SOCKET_ERROR)
                    {
                        fprintf(stderr, "shutdown failed: %d\n", WSAGetLastError());
                        goto cleanup;
                    }

                    closesocket(sc);
                    sc = INVALID_SOCKET;
                }
                else
                {
                    printf("read %d bytes\n", bytecount);

                    //
                    // Now echo the data back
                    //

                    bytecount =  send(sc, Buffer, bytecount, 0);
                    if (bytecount == SOCKET_ERROR)
                    {
                        fprintf(stderr, "send failed: %d\n", WSAGetLastError());
                        goto cleanup;
                    }

                    printf("wrote %d bytes\n", bytecount);
                }
            }
            else
            {
                //
                // No client connection so wait for one
                //
                sc = accept(s, (SOCKADDR *)&from, &fromlen);
                if (sc == INVALID_SOCKET)
                {
                    fprintf(stderr, "accept failed: %d\n", WSAGetLastError());
                    goto cleanup;
                }

                // Display the client's address
                retval = getnameinfo(
                                    (SOCKADDR *)&from,
                                    fromlen,
                                    hoststr,
                                    NI_MAXHOST,
                                    servstr,
                                    NI_MAXSERV,
                                    NI_NUMERICHOST | NI_NUMERICSERV
                                    );
                if (retval != 0)
                {
                    fprintf(stderr, "getnameinfo failed: %d\n", retval);
                    goto cleanup;
                }

                printf("Accepted connection from host %s and port %s\n",
                       hoststr, servstr);
            }
        }
        else
        {
            //
            // Receive and send data
            //
            bytecount = recvfrom(s, Buffer, DEFAULT_BUFFER_LEN, 0, (SOCKADDR *)&from, &fromlen);
            if (bytecount == SOCKET_ERROR)
            {
                //
                // We may get WSAECONNRESET errors in response to ICMP port unreachable
                //    messages so we'll just ignore those
                //
                if (WSAGetLastError() != WSAECONNRESET)
                {
                    fprintf(stderr, "recvfrom failed; %d\n", WSAGetLastError());
                    goto cleanup;
                }
                else
                {
                    continue;
                }
            }

            //
            // Display the source of the datagram
            //
            retval = getnameinfo(
                                (SOCKADDR *)&from,
                                fromlen,
                                hoststr,
                                NI_MAXHOST,
                                servstr,
                                NI_MAXSERV,
                                NI_NUMERICHOST | NI_NUMERICSERV
                                );
            if (retval != 0)
            {
                fprintf(stderr, "getnameinfo failed: %d\n", retval);
                goto cleanup;
            }

            printf("read %d bytes from host %s and port %s\n",
                   bytecount, hoststr, servstr);

            //
            // Echo the datagram back
            //
            bytecount = sendto(s, Buffer, bytecount, 0, (SOCKADDR *)&from, fromlen);
            if (bytecount == SOCKET_ERROR)
            {
                fprintf(stderr, "sendto failed: %d\n", WSAGetLastError());
                goto cleanup;
            }

            printf("sent %d bytes to host %s and port %s\n",
                   bytecount, hoststr, servstr);
        }

    }

    cleanup:

    // Close the client connection if present
    if (sc != INVALID_SOCKET)
    {
        closesocket(sc);
        sc = INVALID_SOCKET;
    }

    return 0;
}
