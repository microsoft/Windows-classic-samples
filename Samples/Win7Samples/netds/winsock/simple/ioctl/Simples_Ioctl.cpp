/******************************************************************************\
* Simples_ioctl.c - TCP server
*
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

#define DEFAULT_PORT        "5001"          // Default port for server
#define DEFAULT_BUFFER_LEN  4096            // Default send/recv buffer length

//
// Function Prototypes
//
void FreeSocket(SOCKET s, int rc);

//
// Function: Usage
//
// Description
//    Display usage information and exit.
//
void Usage(char *progname) 
{
	fprintf(stderr, "Usage\n%s -e [endpoint] -i [interface] [-4] [-6]\n"
	                "Where:\n"
                    "\t-e endpoint   - is the port to listen on\n"
	                "\t-i interface  - is the local string address to bind to\n"
                    "\t-4            - force IPv4\n"
                    "\t-6            - force IPv6\n"
                    "\n"
	                "Defaults are 5001 and INADDR_ANY and IN6ADDR_ANY (if IPv6 present)\n",
                    progname
                    );
	WSACleanup();
	exit(1);
}

//
// Function: main
//
// Description:
//    Parse the command line and set up a TCP server. This sample creates one
//    listening socket for each resolved address returned and marks the socket
//    non-blocking. It then adds each listening socket to the read FD_SET to
//    await incoming connections. Once a client connection is established,
//    data is received and written on that connection. After the client connection
//    is finished, the listening sockets are added back (i.e. the sample only
//    handles one client at a time). This sample simply illustrates the select
//    function. A more scalable implementation would have to handle a list
//    of connected client sockets and handle them accordingly. See the Accept
//    Winsock sample for an example.
//
int __cdecl main(int argc, char **argv) 
{
    struct addrinfo hints,
                   *results = NULL,
                   *addrptr = NULL;
    SOCKADDR_STORAGE client;

    WSADATA       wsaData;
    SOCKET       *server_sockets = NULL,
                  client_socket = INVALID_SOCKET;
    DWORD         optval = 1;
    char          buffer[DEFAULT_BUFFER_LEN],
                  hoststr[NI_MAXHOST],
                  servstr[NI_MAXSERV],
                 *pbuffer = NULL,
                 *interface = NULL,
                 *port = DEFAULT_PORT;
    int           address_family = AF_UNSPEC,
                  socket_count=0,
                  clientlen,
                  bytesread=0,
                  retval,
                  err,
                  i;
    struct fd_set readfds, 
                  writefds;

    // Parse the command line
    for(i=1;i <argc;i++) 
    {
        if ( (strlen(argv[i]) == 2) && ( (argv[i][0] == '-') || (argv[i][0] == '/') ) )
        {
            switch(tolower(argv[i][1])) 
            {
                case '4':           // Force IPv4
                    address_family = AF_INET;
                    break;

                case '6':           // Force IPv6
                    address_family = AF_INET6;
                    break;

                case 'i':           // Local interface to bind to
                    interface = argv[++i];
                    break;

                case 'e':           // Local port to bind to
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

    // Load Winsock
    if ((retval = WSAStartup(MAKEWORD(2,2), &wsaData)) != 0) 
    {
        fprintf(stderr, "WSAStartup failed with error %d\n", retval);
        WSACleanup();
        return -1;
    }

    // Make sure the wildcard port wasn't specified
    if ( _strnicmp(port, "0", 1) == 0 )
        Usage(argv[0]);

    // setup the hints structure for name resolution
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = address_family;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = ((interface == NULL) ? AI_PASSIVE : 0);

    // Resolve the address to bind to. If AF_UNSPEC was specified, interface is NULL,
    //    and IPv6 is installed, this call will return two addresses: 0.0.0.0 and ::
    retval = getaddrinfo(
            interface,
            port,
           &hints,
           &results
            );
    if (retval != 0)
    {
        fprintf(stderr, "getaddrinfo failed: %d\n", retval);
        goto cleanup;
    }

    // Make sure the list is non-NULL
    if (results == NULL)
    {
        fprintf(stderr, "Unable to resolve interface %s\n", interface);
        goto cleanup;
    }

    // Count how many address were returned
    socket_count = 0;
    addrptr = results;
    while (addrptr)
    {
        socket_count++;
        addrptr = addrptr->ai_next;
    }

    // Allocate the server sockets
    server_sockets = (SOCKET *)HeapAlloc(
            GetProcessHeap(),
            HEAP_ZERO_MEMORY,
            sizeof(SOCKET) * socket_count
            );
    if (server_sockets == NULL)
    {
        fprintf(stderr, "HeapAlloc failed: %d\n", GetLastError());
        return -1;
    }

    // Initialize the socket array
    for(i=0; i < socket_count ;i++)
        server_sockets[i] = INVALID_SOCKET;

    // Setup the server sockets
    socket_count = 0;
    addrptr = results;
    while (addrptr)
    {
        // Create the server socket
        server_sockets[socket_count] = socket(addrptr->ai_family, addrptr->ai_socktype, addrptr->ai_protocol);
        if (server_sockets[socket_count] == INVALID_SOCKET)
        {
            fprintf(stderr, "socket failed: %d\n", WSAGetLastError());
            goto cleanup;
        }

        // Bind the socket
        retval = bind(server_sockets[socket_count], addrptr->ai_addr, (int)addrptr->ai_addrlen);
        if (retval == SOCKET_ERROR)
        {
            fprintf(stderr, "bind failed: %d\n", WSAGetLastError());
            goto cleanup;
        }

        // For TCP make the socket listening 
        retval = listen(server_sockets[socket_count], 7);
        if (retval == SOCKET_ERROR)
        {
            fprintf(stderr, "listen failed: %d\n", WSAGetLastError());
            goto cleanup;
        }

        // Make the socket non-blocking
        retval = ioctlsocket(server_sockets[socket_count], FIONBIO, &optval);
        if (retval == SOCKET_ERROR)
        {
            fprintf(stderr, "ioctlsocket failed: %d\n", WSAGetLastError());
            goto cleanup;
        }

        // Display the address we bound to
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

        printf("Server socket 0x%x listening on %s and port %s\n",
                server_sockets[socket_count], hoststr, servstr);

        // Increment counters and pointers
        socket_count++;

        addrptr = addrptr->ai_next;
    }
    //
    // The fd sets should be zeroed out before using them to prevent errors.
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);

    // Set the server sockets
    for(i=0; i < socket_count ;i++)
    {
#pragma warning (push)
#pragma warning (disable: 4127)
        FD_SET(server_sockets[i], &readfds);
#pragma warning (pop)
    }

    //
    // The structure of the loop below is very simple. We only accept one
    // connection at a time. As soon as another client connects, we
    // disconnect the first one, and start talking to the new client.
    // All this server does is to echo the data received on the socket
    // back to the client.
    //
    // This is not a very realistic server, but it does serve to show that
    // select() does not scale very well on win32. If we were dealing
    // with more than one client, we would have to have a list of sockets
    // that are in each fdset to be able to check them when select()
    // returns.
    //
    for(;;)
    {
        //
        // A socket in the listen() state becomes ready to read when a
        // client connects to it. An accept() will complete without
        // blocking.
        //

        retval = select(0, &readfds, &writefds, NULL, NULL);
        if (i == SOCKET_ERROR) 
        {
            fprintf(stderr, "select failed %d\n", WSAGetLastError());
            goto cleanup;
        }

        // Check for data on the client socket if connected
        if ((client_socket != INVALID_SOCKET) && (FD_ISSET(client_socket, &readfds)))
        {
            retval = recv(client_socket, buffer, DEFAULT_BUFFER_LEN, 0);
            if (retval == 0)
            {
                // A successful recv of zero indicates graceful socket closure
               
                // Close the socket
                FreeSocket(client_socket, retval);
                client_socket = INVALID_SOCKET;

                // Add the listening sockets back to the read set
                FD_ZERO(&readfds);
                FD_ZERO(&writefds);

                for(i=0; i < socket_count ;i++)
#pragma warning (push)
#pragma warning (disable: 4127)
                    FD_SET(server_sockets[i], &readfds);
#pragma warning (pop)
            }
            else if (retval == SOCKET_ERROR)
            {
                // For WSAEWOULDBLOCK we simply go back to the top of the loop
                //    and select again. Note that client_socket is already in
                //    the readfds so we don't need to set it again.
                //
                if ((err = WSAGetLastError()) != WSAEWOULDBLOCK)
                {
                    // Some other fatal error occurred so close the socket and
                    //    prepare to select on the listening sockets again.

                    // Close the socket
                    FreeSocket(client_socket, retval);
                    client_socket = INVALID_SOCKET;
                
                    // Add the listening sockets back to the read set
                    FD_ZERO(&readfds);
                    FD_ZERO(&writefds);

                    for(i=0; i < socket_count ;i++)
#pragma warning (push)
#pragma warning (disable:4127)
                        FD_SET(server_sockets[i], &readfds);
#pragma warning (pop)
                }
            }
            else
            {
                // Data was successfully received into 'buffer' so now put the
                //    client_socket into the writefds so we can write it back

                bytesread = retval;

                printf("read %d bytes\n", bytesread);

                pbuffer = buffer;

                // Now wait for writeability to echo the data back
                FD_ZERO(&readfds);
                FD_ZERO(&writefds);
#pragma warning (push)
#pragma warning (disable:4127)
                FD_SET(client_socket, &writefds);
#pragma warning (pop)
            }
        }
        // Check for writeability on the client socket
        else if ((client_socket != INVALID_SOCKET) && (FD_ISSET(client_socket, &writefds)))
        {
            retval = send(client_socket, pbuffer, bytesread, 0);
            if (retval == 0)
            {
                // Connection was gracefully closed so clean things up and put the
                //    listening sockets back into the readfds

                // Close the socket
                FreeSocket(client_socket, retval);
                client_socket = INVALID_SOCKET;

                // Connection was closed so put the server sockets back in the readfds set
                FD_ZERO(&readfds);
                FD_ZERO(&writefds);
                for(i=0; i < socket_count ;i++)
#pragma warning (push)
#pragma warning (disable:4127)
                    FD_SET(server_sockets[i], &readfds);
#pragma warning (pop)
            }
            else if (retval == SOCKET_ERROR)
            {
                // If we got a WSAEWOULDBLOCK we need to try to send again. Note that
                //    client_socket is already in the writefds.
               
                if ((err = WSAGetLastError()) != WSAEWOULDBLOCK)
                {
                    // A fatal error occured so close the socket and put the listening
                    //    sockets back into the readfds
                  
                    fprintf(stderr, "send failed: %d\n", err);
                    
                    // Close the socket
                    FreeSocket(client_socket, retval);
                    client_socket = INVALID_SOCKET;

                    // Connection was closed so put the server sockets back in the readfds set
                    FD_ZERO(&readfds);
                    FD_ZERO(&writefds);
                    for(i=0; i < socket_count ;i++)
#pragma warning (push)
#pragma warning (disable:4127)
                        FD_SET(server_sockets[i], &readfds);
#pragma warning (pop)
                }
            }
            else
            {
                // Successfully sent some data
                printf("sent %d bytes\n", retval);

                if (bytesread != retval)
                {
                    // We didn't send all the data that was read (e.g. 10 bytes were
                    //    read but this send call send less then 10 bytes). We need 
                    //    to try to send again so increment the buffer pointer to the
                    //    data remaining and adjust the byte count. The client_socket
                    //    is already in writefds so just continue;
                    //
                    pbuffer = &buffer[bytesread-retval];
                    bytesread -= retval;

                    // Write isn't complete so leave the socket in the writefds
                    //    set and continue
                }
                else
                {
                    pbuffer = buffer;

                    // Write complete: set the socket for reading now
                    FD_ZERO(&readfds);
                    FD_ZERO(&writefds);
#pragma warning (push)
#pragma warning (disable:4127)
                    FD_SET(client_socket, &readfds);
#pragma warning (pop)
                }
            }
        }
        // Check for accepted client connection
        else
        {
            // See which listening socket has a connection
            for(i=0; i < socket_count ;i++)
            {
                if (FD_ISSET(server_sockets[i], &readfds))
                {
                    // Accept the connection
                    clientlen = sizeof(client);
                    client_socket = accept(server_sockets[i], (SOCKADDR *)&client, &clientlen);
                    if (client_socket == INVALID_SOCKET)
                    {
                        fprintf(stderr, "accept failed: %d\n", WSAGetLastError());
                        goto cleanup;
                    }

                    // Print the client's address out
                    retval = getnameinfo(
                            (SOCKADDR *)&client,
                            clientlen,
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

                    printf("Accepted client connection from %s and port %s\n",
                            hoststr, servstr);

                    FD_ZERO(&readfds);
#pragma warning (push)
#pragma warning (disable:4127)
                    FD_SET(client_socket, &readfds);
#pragma warning (pop)

                    // Since we handle only one conneciton at a time, break out
                    //    and start servicing the client connection
                    break;
                }
            }
        }
    }

cleanup:

    //
    // Clean up any allocations and handles, etc.
    //

    if (results)
    {
        freeaddrinfo(results);
        results = NULL;
    }

    if (client_socket != INVALID_SOCKET)
    {
        FreeSocket(client_socket, NO_ERROR);
        client_socket = INVALID_SOCKET;
    }

    if (server_sockets)
    {
        for(i=0; i < socket_count ;i++)
        {
            if (server_sockets[i] != INVALID_SOCKET)
            {
                closesocket(server_sockets[i]);
                server_sockets[i] = INVALID_SOCKET;
            }
        }

        HeapFree(GetProcessHeap(), 0, server_sockets);
        server_sockets = NULL;
    }

    WSACleanup();
    return 0;
}

//
// Function: FreeSocket
//
// Description:
//    If the socket closed without error shut it down and then close the 
//    socket handle.
//
void FreeSocket(SOCKET s, int rc)
{
    int     retval;

    // client connection was closed
    if (rc == NO_ERROR)
    {
        retval = shutdown(s, SD_SEND);
        if (retval == SOCKET_ERROR)
        {
            fprintf(stderr, "FreeSocket: shutdown failed: %d\n", WSAGetLastError());
        }
    }

    // Release the handle
    retval = closesocket(s);
    if (retval == SOCKET_ERROR)
    {
        fprintf(stderr, "FreeSocket: closesocket failed: %d\n", WSAGetLastError());
    }
}
