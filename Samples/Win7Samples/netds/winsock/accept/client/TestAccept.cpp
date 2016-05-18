// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 2002  Microsoft Corporation.  All Rights Reserved.
//
// Module Name: TestAccept.cpp
//
// Description:
//             This file contains the functions for a client that tests
// the accept implementation.

#pragma warning (disable: 4267)  
#pragma warning (disable: 4127)


#include <winsock2.h>   // for Winsock API
#include <windows.h>    // for Win32 APIs and types
#include <ws2tcpip.h>   // for IPv6 support
#include <wspiapi.h>    // for IPv6 support
#include <strsafe.h>    // for safe versions of string functions
#include <stdio.h>      // for printing to stdout.


// The following are the default values for the various command-line
// options.
#define DEFAULT_ADDRESS_FAMILY          AF_UNSPEC
#define DEFAULT_SERVER                  "localhost"
#define DEFAULT_PORT                    "7243"
#define DEFAULT_SEND_BUFFER_SIZE        1024
#define DEFAULT_RECV_BUFFER_SIZE        10240 
#define DEFAULT_LOOP_COUNT              1
#define DEFAULT_DELAY                   5000
#define DEFAULT_SCENARIO                5


// The following are the different scenarios supported for testing
// the Accept server implementation. The ideal send receive is the
// recommended scenario for real world use. The other scenarios
// are meant to understand the various problems that would be
// caused in the server if the client behaves (or misbehaves) in a 
// particular way.
enum
{
    SC_SEND_THEN_RECV = 1,
    SC_SEND_NO_RECV = 2,
    SC_SEND_WAIT_RECV = 3,
    SC_WAIT_SEND_RECV = 4,
    SC_IDEAL_SEND_RECV = 5,
    SC_NUM_SCENARIOS = 5
};


// structure that bundles all the global variables needed between different
// functions into a global context.
typedef struct _ClientContext
{
    BYTE    addressFamily;
    char    *szServer;
    char    *szPort;
    LONG    sendBufSize;
    LONG    recvBufSize;    
    LONG    loopCount;
    LONG    delay;
    BYTE    scenario;    
    char    *pSendBuf;
    char    *pRecvBuf;    
    int     nBytesRemainingToBeSent;
    int     nBytesRecd;
    SOCKET  sock;
    
} ClientContext;

ClientContext g_ClientContext;

/*
    This function converts a given address family into its corresponding string
    representation for display purposes.
*/
const char *AFImage(BYTE addressFamily)
{
    char *szRetVal;

    // return the corresponding string for the given address family.
    switch (addressFamily)
    {
        case AF_UNSPEC : szRetVal = "AF_UNSPEC";
                         break;
        case AF_INET   : szRetVal = "AF_INET";
                         break;
        case AF_INET6  : szRetVal = "AF_INET6";
                         break;
        default        : szRetVal = "Unrecognized";
                         break;
    }
    
    return szRetVal;
}


/*
    This function prints the available command-line options, the arguments
    expected by each of them and the valid input values and the default 
    values for each them.
*/
void PrintUsage(char *szProgramName)
{
    printf("\n\n"
           "Usage:\n"
           "------\n"
           "   %s <options> \n\n"
           "where <options> is one or more of the following: \n\n"
           "   -a <0|4|6>      Address Family: 0 for Either\n"
           "                                   4 for IPv4\n"
           "                                   6 for IPv6\n"
           "                   Default: %d\n\n"
           "   -n <server>     Server Name or IP\n"
           "                   Default: %s\n\n"
           "   -e <endpoint>   Port number\n"
           "                   Default: %s\n\n"
           "   -b <bytes>      Bytes to send\n"
           "                   Default: %d\n\n"
           "   -l <count>      Loop count times \n"
           "                   Default: %d\n\n"
           "   -d <millisecs>  Delay after or before send (see -s 3 & -s 4)\n"
           "                   Default: %d\n\n"
           "   -s <1..5>       Scenario: \n"
           "                      1 : Finish all sends, then recv\n"
           "                      2 : Finish all sends, no recv\n"
           "                      3 : Finish all sends, wait, then recv\n"
           "                      4 : First wait, then send, then recv\n"
           "                      5 : Ideal Send and Recv sequence\n"
           "                   Default: %d\n\n"
           "\n",
           szProgramName,
           DEFAULT_ADDRESS_FAMILY,
           DEFAULT_SERVER,
           DEFAULT_PORT,
           DEFAULT_SEND_BUFFER_SIZE,
           DEFAULT_LOOP_COUNT,
           DEFAULT_DELAY,
           DEFAULT_SCENARIO
           );

    return;
}

/*
    This function parses the given input arguments and fills up the
    corresponding fields in the g_ClientContext structure.
*/
BOOL ParseArguments(int argc, char *argv[])
{
    // holds the return value from this function.
    // TRUE indicates that all the supplied arguments are valid. 
    // FALSE indicates incorrect or insufficient number of arguments.
    BOOL retVal = FALSE;

    // loop index to go over the command-line arguments one by one.
    int i;
    
    printf("Entering ParseArguments()\n");

    // fill up the default arguments and let the user options override these.
    g_ClientContext.addressFamily = DEFAULT_ADDRESS_FAMILY;
    g_ClientContext.szServer = DEFAULT_SERVER;
    g_ClientContext.szPort = DEFAULT_PORT;
    g_ClientContext.sendBufSize = DEFAULT_SEND_BUFFER_SIZE;
    g_ClientContext.recvBufSize = DEFAULT_RECV_BUFFER_SIZE;    
    g_ClientContext.loopCount = DEFAULT_LOOP_COUNT;
    g_ClientContext.delay = DEFAULT_DELAY;
    g_ClientContext.scenario = DEFAULT_SCENARIO;        

    // process each argument in the argv list.
    for (i = 1; i < argc ; i++)
    {
        char firstChar = argv[i][0];

        // make sure the option begins with a - or /
        if (!(firstChar == '-' || firstChar == '/'))
        {
            printf("ERROR: Option has to begin with - or / : %s\n", argv[i]);
            PrintUsage(argv[0]);
            goto CLEANUP;
        }

        // process the option.
        switch(argv[i][1])
        {
            case 'a' :
            
                // Address Family. 
                // should be -a 0 or -a 4 or -a 6
                
                // first check if there's one more argument.
                if (i + 1 >= argc)
                {
                    printf("ERROR: Argument 0/4/6 needed for -a option\n");
                    PrintUsage(argv[0]);
                    goto CLEANUP;
                }

                // extract and validate the AF number.
                switch(atoi(argv[i+1]))
                {
                    // Unspecified. 
                    case 0:
                      g_ClientContext.addressFamily = AF_UNSPEC;
                      break;

                    // IPv4.
                    case 4:
                      g_ClientContext.addressFamily = AF_INET;
                      break;                      

                    // IPv6.
                    case 6:
                      g_ClientContext.addressFamily = AF_INET6;
                      break;                      

                    // Invalid value.
                    default:
                      printf("ERROR: Invalid address family. Must be 0/4/6\n");
                      PrintUsage(argv[0]);
                      goto CLEANUP;
                }

                // indicate that we have processed the next argument as well.
                i++; 

                // AF was fine. continue.
                break;

            case 'n' :

                // Interface to listen on.
                // should be -n <server Name>
                
                // first check if there's one more argument.
                if (i + 1 >= argc)
                {
                    printf("ERROR: Server name needed for -n option\n");
                    PrintUsage(argv[0]);
                    goto CLEANUP;
                }

                // make sure the input string length is less than
                // the INET6_ADDRSTRLEN, the maximum valid IP address length.
                if (FAILED(StringCchLength(argv[i+1],INET6_ADDRSTRLEN, NULL)))
                {
                    printf("ERROR: Server name string too long. "
                           "can't exceed %d characters\n", 
                           INET6_ADDRSTRLEN);
                    PrintUsage(argv[0]);
                    goto CLEANUP;
                }

                // remember the interface string.
                g_ClientContext.szServer = argv[i+1];

                // indicate that we have processed the next argument as well.
                i++; 

                // continue.            
                break;

            case 'e' :
            
                // Endpoint or Port.
                // should be -e <port number>
                
                // first check if there's one more argument.
                if (i + 1 >= argc)
                {
                    printf("ERROR: Port number needed for -e option\n");
                    PrintUsage(argv[0]);
                    goto CLEANUP;
                }

                // make sure the input string length is less than
                // the maximum length for a service name.
                if (FAILED(StringCchLength(argv[i+1], NI_MAXSERV, NULL)))
                {
                    printf("ERROR: Port number too long. "
                           "can't exceed %d characters\n", 
                           NI_MAXSERV);

                    PrintUsage(argv[0]);
                    goto CLEANUP;
                }

                // remember the port number string.
                g_ClientContext.szPort = argv[i+1];

                // indicate that we have processed the next argument as well.
                i++; 

                // continue.            
                break;
                
            case 'b' :
           
                // Bytes to Send
                // should be -b <bytes>.
                
                // first check if there's one more argument.
                if (i + 1 >= argc)
                {
                    printf("ERROR: Number of bytes needed for -b option\n");
                    PrintUsage(argv[0]);
                    goto CLEANUP;
                }

                // extract the bytes value
                g_ClientContext.sendBufSize = atol(argv[i+1]);

                // indicate that we have processed the next argument as well.
                i++; 

                // validate the buffer size.
                if (g_ClientContext.sendBufSize < 0)
                {
                    printf("ERROR: Number of bytes must be > 0 \n");
                    PrintUsage(argv[0]);
                    goto CLEANUP;
                }

                // Continue.
                break;
                
            case 'l' :
           
                // Loop Count
                // should be -l or -l <count>.
                
                // if there's one more argument, parse it.
                if (i + 1 < argc)
                {
                    g_ClientContext.loopCount = atol(argv[i+1]);

                    // make sure loop count is 1 or more, if specified.
                    if (g_ClientContext.loopCount < 1)
                    {
                        printf("ERROR: Loop Count must be > 0 : %d\n", 
                                g_ClientContext.loopCount);
                        PrintUsage(argv[0]);
                        goto CLEANUP;
                    }

                    // indicate that we have processed the next argument 
                    // as well.                    
                    i++;
                }


                // Continue.
                break;

            case 'd' :
           
                // Delay in milliseconds.
                // should be -d <millisecs>.
                
                // first check if there's one more argument.
                if (i + 1 >= argc)
                {
                    printf("ERROR: Number of milliseconds needed "
                           "for -d option\n");
                    PrintUsage(argv[0]);
                    goto CLEANUP;
                }

                // extract the delay value
                g_ClientContext.delay = atol(argv[i+1]);

                // indicate that we have processed the next argument as well.
                i++; 

                // Continue.
                break;                

            case 's' :
           
                // Scenario Number.
                // should be -s <scenario number>.
                
                // first check if there's one more argument.
                if (i + 1 >= argc)
                {
                    printf("ERROR: Scenario Number needed for -s option\n");
                    PrintUsage(argv[0]);
                    goto CLEANUP;
                }

                // extract the scenario value
                g_ClientContext.scenario = (BYTE) atoi(argv[i+1]);

                // indicate that we have processed the next argument as well.
                i++; 

                // validate the scenario type.
                if (g_ClientContext.scenario < 1 ||
                    g_ClientContext.scenario > SC_NUM_SCENARIOS)
                {
                    printf("ERROR: Invalid scenario type: %d. Must be between "
                           "1 to %d\n",g_ClientContext.scenario, 
                           SC_NUM_SCENARIOS);
                    PrintUsage(argv[0]);
                    goto CLEANUP;
                }

                // Continue.
                break;                
                
            case 'h' : // help
            case '?' : // help
                        PrintUsage(argv[0]);
                        goto CLEANUP;                        
            default  : 
                        printf("ERROR: Unrecognized option: %s\n", argv[i]);
                        PrintUsage(argv[0]);
                        goto CLEANUP;                        
        }
    }

    // echo the final list of values that'll be used.
    // remember, these need not be the same as the input arguments.
    // rather, this is what we'll use inside our program.
    printf("Parsed input arguments. The following values will be used : \n");
    printf("\tAddress Family = %s\n", AFImage(g_ClientContext.addressFamily));
    printf("\tServer = %s\n",g_ClientContext.szServer);
    printf("\tPort = %s\n", g_ClientContext.szPort);
    printf("\tBytes to Send = %ld\n", g_ClientContext.sendBufSize);  
    printf("\tReceive buffer size = %ld\n", g_ClientContext.recvBufSize);      
    printf("\tIterations  = %ld\n", g_ClientContext.loopCount);
    printf("\tDelay = %ld\n", g_ClientContext.delay);
    printf("\tScenario = %d\n", g_ClientContext.scenario);

    // all went well, signal that we can proceed.
    retVal = TRUE;

CLEANUP:    

    printf("Exiting ParseArguments()\n");
    return retVal;
}


/*
    Prints the given socket address in a printable string format.
*/
void PrintAddressString(LPSOCKADDR pSockAddr, DWORD dwSockAddrLen)
{
    // INET6_ADDRSTRLEN is the maximum size of a valid IPv6 address 
    // including port,colons,NULL,etc.
    char buf[INET6_ADDRSTRLEN];
    DWORD dwBufSize = 0;    

    memset(buf,0,sizeof(buf));
    dwBufSize = sizeof(buf);

    // This function converts the pSockAddr to a printable format into buf.   
    if (WSAAddressToString(pSockAddr, 
                           dwSockAddrLen, 
                           NULL, 
                           buf, 
                           &dwBufSize) == SOCKET_ERROR)
    {
        printf("ERROR: WSAAddressToString failed %d \n", WSAGetLastError());
        goto CLEANUP;
    }

    printf("%s\n", buf);

CLEANUP:
    return;
}


/*
    This function create a socket for the client and connects it to the
    server.
*/
SOCKET CreateClientSocket()
{
    struct addrinfo hints;
    struct addrinfo *res;
    struct addrinfo *pAddr;
    SOCKET clientSock = INVALID_SOCKET;
    int i;
    int rc;

    printf("Entering CreateClientSocket()\n");

    memset(&hints, 0, sizeof(hints));
    
    hints.ai_family = g_ClientContext.addressFamily;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_socktype = SOCK_STREAM;

    // get the local addresses that are suitable for connecting to the
    // given server address.
    if (getaddrinfo(g_ClientContext.szServer, 
                g_ClientContext.szPort,
                &hints, 
                &res) != NO_ERROR)
    {
        printf("getaddrinfo failed. Error = %d\n", WSAGetLastError());
        goto CLEANUP;
    }

    if (res == NULL)
    {
        printf("getaddrinfo returned res = NULL\n");
        goto CLEANUP;
    }

    printf("getaddrinfo successful.Enumerating the returned addresses ...\n\n");

    for (pAddr = res, i = 1; pAddr != NULL; pAddr = pAddr->ai_next, i++)
    {
        printf("Processing Address %d returned by getaddrinfo : ", i);
        PrintAddressString(pAddr->ai_addr, pAddr->ai_addrlen);
        clientSock = WSASocket(pAddr->ai_family, 
                            pAddr->ai_socktype,
                            pAddr->ai_protocol,
                            NULL,
                            NULL,
                            0);
        if (clientSock == INVALID_SOCKET)
        {
            printf("WSASocket failed. Error = %d\n", WSAGetLastError());
            printf("Ignoring this address and continuing with the next. \n\n");
            
            // anyway, let's continue with other addresses.
            continue;
        }

        printf("Created client socket with handle = %d\n", clientSock);

        rc = connect(clientSock, pAddr->ai_addr, pAddr->ai_addrlen);
        if (rc == SOCKET_ERROR)
        {
            printf("connect failed. Error = %d\n", WSAGetLastError());
            closesocket(clientSock);
            clientSock = INVALID_SOCKET;
            continue;
        }

        printf("Connected successfully\n");
        break;
    }

   
CLEANUP:

    // if getaddrinfo succeeded, it would have allocated memory for the
    // res structure which we have to free.
    if (res)
    {
        freeaddrinfo(res);
        printf("Freed the memory allocated for res by getaddrinfo\n");
    }

    printf("Exiting CreateClientSocket()\n");
    return clientSock;
}


/*
    Allocate a buffer to use for sending data and initialize the buffer
    with suitable initial values.
*/
BOOL PrepareSendBuffer()
{
    BOOL bSuccess = FALSE;
    
    printf("Entering PrepareSendBuffer()\n");
    
    g_ClientContext.pSendBuf = (char *) malloc(g_ClientContext.sendBufSize + 1);
    if (g_ClientContext.pSendBuf == NULL)
    {
        printf("malloc failed.\n");
        goto CLEANUP;
    }
    
    printf("Allocated Send Buffer: %p\n", g_ClientContext.pSendBuf);

    // fill up with some info. here we are just sending 1 character.
    memset(g_ClientContext.pSendBuf, 'H', g_ClientContext.sendBufSize);   
    g_ClientContext.pSendBuf[g_ClientContext.sendBufSize] = '\0';
    g_ClientContext.nBytesRemainingToBeSent = g_ClientContext.sendBufSize;
    
    bSuccess = TRUE;

CLEANUP:    
    printf("Exiting PrepareSendBuffer()\n");    
    return bSuccess;
}


/*
    Deallocate the buffer that was used for sending.
*/
void FreeSendBuffer()
{
    if (g_ClientContext.pSendBuf != NULL)
    {
        free(g_ClientContext.pSendBuf);
        printf("Freed Send Buffer : %p\n", g_ClientContext.pSendBuf);
        g_ClientContext.pSendBuf = NULL;
    }
}


/*
    Allocate a buffer for receiving data from the server.
    Currently since we are not using the received data, we'll just receive
    all the data in a big one-time buffer for all the receives and overwrite
    the same buffer each time. The count of received bytes is what we
    are presently interested in.
*/
BOOL PrepareRecvBuffer()
{
    BOOL bSuccess = FALSE;
    
    printf("Entering PrepareRecvBuffer()\n");
    
    g_ClientContext.pRecvBuf = (char *) malloc(g_ClientContext.recvBufSize + 1);
    if (g_ClientContext.pRecvBuf == NULL)
    {
        printf("malloc failed.\n");
        goto CLEANUP;
    }
    
    printf("Allocated Recv Buffer: %p\n", g_ClientContext.pRecvBuf);
    
    memset(g_ClientContext.pRecvBuf, 0, g_ClientContext.recvBufSize + 1);   
    g_ClientContext.nBytesRecd = 0;
    
    bSuccess = TRUE;

CLEANUP:    
    printf("Exiting PrepareRecvBuffer()\n");    
    return bSuccess;
}


/*
    Deallocate the buffer used for receiving.
*/
void FreeRecvBuffer()
{
    if (g_ClientContext.pRecvBuf != NULL)
    {
        free(g_ClientContext.pRecvBuf);
        printf("Freed Recv Buffer : %p\n", g_ClientContext.pRecvBuf);
        g_ClientContext.pRecvBuf = NULL;
    }
}

/*
    Try sending data to server once. Return the error, if any.
*/
int DoSendOnce()
{
    int nBytesSent;    
    int startPosition;
    int err = 0;
    
    printf("Entering DoSendOnce()\n");

    // send from the position where we left off last.
    startPosition = g_ClientContext.sendBufSize - 
                    g_ClientContext.nBytesRemainingToBeSent;
    nBytesSent = send(g_ClientContext.sock, 
                      g_ClientContext.pSendBuf + startPosition, 
                      g_ClientContext.nBytesRemainingToBeSent,
                      0);
    
    if (nBytesSent == SOCKET_ERROR)
    {
        err = WSAGetLastError();
        goto CLEANUP;
    }

    // remember where to begin send next time.
    g_ClientContext.nBytesRemainingToBeSent -= nBytesSent;
    printf("Sent %d bytes so far\n", startPosition + nBytesSent);

CLEANUP:
  
    printf("Exiting DoSendOnce()\n");
    return err;
}


/*
    Try receiving from the server. This function returns the number of bytes
    received (unlike DoSendOnce which returns the error code).
*/
int DoRecvOnce()
{
    int nBytesRecd;    
    
    printf("Entering DoRecvOnce()\n");

    // receive into the same global receive buffer, overwriting the previous
    // contents, as we are only interested in the number of bytes received
    // for verification.
    nBytesRecd = recv(g_ClientContext.sock, 
                      g_ClientContext.pRecvBuf, 
                      g_ClientContext.recvBufSize,
                      0);
    
    if (nBytesRecd == SOCKET_ERROR)
    {
        printf("recv returned: %d\n", WSAGetLastError());
        goto CLEANUP;
    }

    // update the stats.
    g_ClientContext.nBytesRecd += nBytesRecd;
    printf("Recd %d bytes so far\n", g_ClientContext.nBytesRecd);

CLEANUP:
  
    printf("Exiting DoRecvOnce()\n");
    return nBytesRecd;
}


/*
    Repeat sending the data to the server until all the data has been
    completely sent to the server.
*/
void DoSendUntilDone()
{
    int err;

    printf("Entering DoSendUntilDone()\n");    
    
    do
    {
        // Get the job done by callnig DoSendOnce.
        err = DoSendOnce();

        // handle error cases as appropriate.
        switch(err)
        {
            case 0:
                if (g_ClientContext.nBytesRemainingToBeSent == 0)
                {
                    printf("Send completed\n");
                    goto CLEANUP;
                }
                break;
               
            case WSAEWOULDBLOCK:
                printf("Got WSAEWOULDBLOCK from send.\n");
                printf("Waiting for 1 second before retrying send ...\n");
                Sleep(1000);
                break;

            default:
                // other errors.
                printf("ERROR: send failed: Error = %d\n", err);
                goto CLEANUP;
        }
    } while (TRUE);

CLEANUP:
    printf("Exiting DoSendUntilDone()\n");
    return;
}


/*
    Receive data on the socket until recv returns 0 or error.
*/
void DoRecvUntilDone()
{
    int err;

    printf("Entering DoRecvUntilDone()\n");
    do
    {
        // get the job done by calling DoRecvOnce.
        switch(DoRecvOnce())
        {
            case 0:
                printf("Recv returned 0. Remote socket must have been "
                       "gracefully closed.\n");
                goto CLEANUP;
              
            case SOCKET_ERROR:
                err = WSAGetLastError();
                if (err == WSAEWOULDBLOCK)
                {
                    printf("Got WSAEWOULDBLOCK from recv.\n");
                    printf("Waiting for 1 second before retrying send ...\n");
                    Sleep(1000);
                }
                else
                {
                    printf("ERROR: recv returned : %d\n", err);
                    goto CLEANUP;
                }
                break;

            default:
                // > 0 bytes read. let's continue.
                break;
        }
    } while (1);

CLEANUP:
    printf("Exiting DoRecvUntilDone()\n");
    return;
}


/*
    This functions shuts down the socket before closing it so that
    it would send a FIN packet on the TCP connection to the remote side
    to indicate that there won't be any more data sent on this socket.
    Only if this side shuts down, the remote side will get a 0 for a
    recv and conclude that the client is done with sending data.
*/
void DoShutDown()
{
    if (shutdown(g_ClientContext.sock, SD_SEND) == SOCKET_ERROR)
        printf("shutdown failed. err = %d\n", WSAGetLastError());
    else
        printf("shutdown successful\n");
}

/*
    This function implements the scenario where all the sends are done first
    and all the recvs next.
*/
void DoSendThenRecv()
{
    DoSendUntilDone();
    DoShutDown();
    DoRecvUntilDone();
}

/*
    This function implements the scenario where the client only sends but
    doesn't recv the echoed data back. This helps demonstrate possible
    deadlocks in the server side for huge sized data buffer.
*/
void DoSendNoRecv()
{
    DoSendUntilDone();
    DoShutDown();    
}

/*
    This function implements the scenario where the client waits for a certain
    time before doing the recv thereby allowing the server code to get
    a WSAEWOULDBLOCK on a send, especially when the data buffer is huge.
*/
void DoSendWaitRecv()
{
    DoSendUntilDone();
    DoShutDown();      
    printf("Waiting before doing recv ...\n");
    Sleep(g_ClientContext.delay);
    DoRecvUntilDone();
}

/*
    This function implements the scenario to test the server to get
    a FD_WRITE event before an FD_READ event.
*/
void DoWaitSendRecv()
{
    printf("Waiting before doing initial send ...\n");
    Sleep(g_ClientContext.delay);
    DoSendUntilDone();
    DoShutDown();        
    DoRecvUntilDone();
}

/*
    This function is supplied as the call back function for CreateThread.
    Although it is doing nothing other than calling DoRecvUntilDone, it
    needs to have this particular signature to be passed as a valid 
    call back function for CreateThread.
*/
DWORD WINAPI ReceiverThread(LPVOID pv)
{
    pv = NULL;

    DoRecvUntilDone();
    return 0;
}

/*
    This function illustrates the ideal send and recv scenario where
    one thread does the recv independently until it gets a 0 or an error
    and the other thread (in this case, the current thread) does sends
    until all data is sent. This is the truly asynchronous implementation
    of the client unlike the earlier scenarios which were mostly meant
    for testing specific sequence of events in the server and understanding
    the need for different parts of the server code.
*/
void DoIdealSendRecv()
{
    HANDLE hThread;

    // start the receiver thread.
    hThread = CreateThread(NULL, 0, ReceiverThread, NULL, 0, NULL);
    if (hThread == NULL)
    {
        printf("ERROR: CreateThread failed. Error = %d\n", GetLastError());
        goto CLEANUP;
    }

    // In this thread, send all data until done.
    DoSendUntilDone();
    DoShutDown();

    // wait till recv thread is done.
    if (WaitForSingleObject(hThread, INFINITE) != WAIT_OBJECT_0)
    {
        printf("ERROR: WaitForSingleObject failed: %d\n", GetLastError());
        goto CLEANUP;
    }
    printf("Recv Thread done.\n");

CLEANUP:
    
    return;
}

/*
    This function is the entry point for this program.
    Based on the command-line arguments, it invokes the suitable functions.
*/
int _cdecl main(int argc, char *argv[])
{
    // holds the return value from this function.
    // 0 indicates success, non-zero indicates failure.
    int retVal;
    int i;
    BOOL bStartupSuccessful = FALSE;

    WSADATA wsaData;

    printf("Entering main()\n");

    // parse and validate the given arguments and determine if we should
    // continue the execution or return error.
    if (ParseArguments(argc, argv) == FALSE)
    {
        // error input. return a non-zero error code.
        retVal = 1;
        goto CLEANUP;
    }

    // call WSAStartup to initialize Winsock before calling any of its APIs.
    retVal = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (retVal != 0)
    {
        printf("WSAStartup failed. Error = %d\n", retVal);
        goto CLEANUP;        
    }

    // remember that the WSAStartup was successful so as to call WSACleanup 
    // once, correspondingly.
    bStartupSuccessful = TRUE;

    for(i = 1; i <= g_ClientContext.loopCount; i++)
    {
        printf("\n\nPerforming Iteration : %d\n"
                   "-------------------------\n",i);

        // create a socket that's connected to the server.
        g_ClientContext.sock = CreateClientSocket();
        if (g_ClientContext.sock == INVALID_SOCKET)
            goto CLEANUP;

        // allocate and initialize the buffer to be sent.
        if (PrepareSendBuffer() == FALSE)
            goto CLOSE_SOCKET;

        // allocate and initialize a buffer for receiving.
        if (PrepareRecvBuffer() == FALSE)
            goto CLOSE_SOCKET;
        
        // perform the sends/recv according to the requested scenario.      
        switch(g_ClientContext.scenario)
        {
            case SC_SEND_THEN_RECV :    DoSendThenRecv();
                                        break;
            case SC_SEND_NO_RECV :      DoSendNoRecv();
                                        break;
            case SC_SEND_WAIT_RECV :    DoSendWaitRecv();
                                        break;
            case SC_WAIT_SEND_RECV :    DoWaitSendRecv();
                                        break;
            case SC_IDEAL_SEND_RECV:    DoIdealSendRecv();
                                        break;                                    
            default:                    printf("Unrecognized scenario\n");
                                        break;
        }

CLOSE_SOCKET:

        // free the send and recv buffers.
        FreeSendBuffer();    
        FreeRecvBuffer();

        // close the client socket.    
        closesocket(g_ClientContext.sock);  
        printf("Closed socket %d. "
           "Total Bytes Recd = %d, "
           "Total Bytes Sent = %d\n",
            g_ClientContext.sock, 
            g_ClientContext.nBytesRecd,
            g_ClientContext.sendBufSize - 
            g_ClientContext.nBytesRemainingToBeSent);

    }

 
CLEANUP:

    // call WSACleanup only if WSAStartup was successful.
    if (bStartupSuccessful)
    {
        // Inform Winsock that we are done with all the APIs.
        WSACleanup();
    }

    printf("Exiting main()\n");
    return retVal;
}


