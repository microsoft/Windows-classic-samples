// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Module:
//      atmevent.cpp
//
// Abstract:
//      This application demonstrates a very simple-minded Winsock application
//      that uses the native ATM Winsock 2 service provider.  The specific 
//      ATM protocol addressing demonstrated by this application is AAL5.  This 
//      application makes no attempt to do anything useful per se, but concentrates 
//      how setting up a connection using AAL5 and sending some data.  Additionally, 
//      this application demonstrates several Winsock2-specific APIs and techniques 
//      that are useful whether or not your main interest is ATM or not.
//
//      Commandline parameters determine whether this application functions as 
//      either a sender (-n), a receiver (-i), or simply allows ATM interfaces to
//      be enumerated.  When specifying the ATM interface to bind to (on the receiver)
//      or the remote ATM interface to connect to on the sender the interface
//      is a 40 character NSAP address, with the last 2 hex digits (0 - 255 decimal)
//      representing the port.  The commandline switch (-a) can be used to enumerate 
//      all ATM interface on the local machine - note the last 2 digits will be 00.
//
//      Use the -? commandline switch to determine available options.
//
//      This application simply sends a buffer of data when acting as the sender,
//      to an instance of itself acting as a receiver.  Any number of senders (up
//      to MAXCLIENTS) can be started to interact with one instance of a receiver.
//      The size of the data buffer (-b), its contents (-d), and the number of 
//      times to send (-l) this buffer are controllable via commandline switches. 
//      A repeat value of 0 implies continuous sending.  A delay 
//      commandline switch (-s:#), measured in milliseconds can be specified for the 
//      sender to insure the total transfer spans a measured unit of time.  Additionally,
//      the commandline switch (-s2:#) is available to delay closing the connection,
//      measured in milliseconds, to allow the receiver time to pick up the data.  This
//      is important because neither AAL5 nor this application has any handshake
//      mechanism to determine when the receiver has received all the data - AAL5
//      does not support a graceful connection closure.
//
// Entry Points:
//      main - this is where it all starts
//
// Usage:
//      Enumerate ATM interfaces
//          atmevent -a
//      Act as a sender and send buffer 10 times
//      and wait 2 seconds before closing the connection
//          atmevent -n:receiver_atm_interface -l:10 -c:2000
//      Act as a receiver
//          atmevent -i:local_atm_interface
//
// Build:
//      Use the headers and libs from the May99 Platform SDK or later.
//      Link with ws2_32.lib
//      
//

#define ATMEVENT_MAIN
#include "atmevent.h"


static BOOL ValidOptions(int argc, char **argv, OPTIONS *pOptions);
static VOID PrintOptions(OPTIONS *pOptions);
static VOID Usage(CHAR *szProgramname, OPTIONS *pOptions);
static BOOL FindATMServiceProvider(WSAPROTOCOL_INFO *pProtocolInfo);



int __cdecl main(int argc, char **argv)
{
    int n = 0;
    OPTIONS options;
    WSADATA WSAData;

    if (!ValidOptions(argc, argv, &options))
        return 1;

    if (0 != (n = WSAStartup(MAKEWORD(2,2), &WSAData)))
    {
        printf("WSAStartup failed w/%d\n", n);
        return 1;
    }

    if (!FindATMServiceProvider(&options.protocolInfo))
    {
        printf("No matching service provider found\n");
    } else
    {
        PrintOptions(&options);
        if (CMD_RECEIVER == options.cmd)
            Receiver(&options);
        else if (CMD_SENDER == options.cmd)
            Sender(&options);
        else // enumerate ATM interfaces
            Enumerator(&options);
    }

    if (options.buf) Free(options.buf);
    WSACleanup();
    return 0;
}






// Abstract:
//      Verify options passed in and set options structure accordingly.
//
BOOL ValidOptions(int argc, char **argv, OPTIONS *pOptions)
{
    OPTIONS     default_options;
    HRESULT hRet;
    size_t dwSize;


    pOptions->buf            = NULL;
    pOptions->cmd            = CMD_RECEIVER;
    pOptions->dwSleep        = 0;
    pOptions->dwSleepClose   = 1000;
    pOptions->dwTotalClients = 0;
    pOptions->fillchar       = 'c';
    pOptions->nBufSize       = 4096;
    pOptions->nRepeat        = 1;
    pOptions->szLocalInterface[0]  = '\0';
    pOptions->szRemoteInterface[0] = '\0';
    ZeroMemory(&pOptions->protocolInfo, sizeof(pOptions->protocolInfo));
    default_options = *pOptions;

    for (int i=1; i<argc; i++)
    {
        if ((argv[i][0] == '-') || (argv[i][0] == '/') )
        {
            switch (tolower(argv[i][1]))
            {
            case 'a' :
                pOptions->cmd = CMD_ENUMERATOR;
                break;

            case 'b' :
                if (strlen(argv[i]) > 3)
                    pOptions->nBufSize = 1024*atoi(&argv[i][3]);
                break;

            case 'c' :
                if (strlen(argv[i]) > 3)
                    pOptions->dwSleepClose = atoi(&argv[i][3]);
                break;

            case 'd' :
                if (strlen(argv[i]) > 3)
                    pOptions->fillchar = argv[i][3];
                break;

            case 'i' : 
                pOptions->cmd = CMD_RECEIVER; 
                if (strlen(argv[i]) > 3)
                {
                    hRet = StringCbLength(&argv[i][3],sizeof(pOptions->szLocalInterface),&dwSize);
                    hRet = StringCbCopyN(pOptions->szLocalInterface,sizeof(pOptions->szLocalInterface),&argv[i][3],dwSize);
                }
                break;

            case 'l' :
                if (strlen(argv[i]) > 3)
                    pOptions->nRepeat = atoi(&argv[i][3]);
                break;

            case 'n' : 
                pOptions->cmd = CMD_SENDER; 
                if (strlen(argv[i]) > 3)
                {
                    hRet = StringCbLength(&argv[i][3],sizeof(pOptions->szRemoteInterface),&dwSize);
                    hRet = StringCbCopyN(pOptions->szRemoteInterface,sizeof(pOptions->szRemoteInterface),&argv[i][3],dwSize);
                }
                break;

            case 's' :
                if (strlen(argv[i]) > 3)
                    pOptions->dwSleep = atoi(&argv[i][3]);
                break;

            case '?' :
                Usage(argv[0], &default_options);
                return FALSE;
                break;

            default:
                printf("  unknown options flag %s\n", argv[i]);
                Usage(argv[0], &default_options);
                return FALSE;
                break;
            }
        } else
        {
            printf("  unknown option %s\n", argv[i]);
            Usage(argv[0], &default_options);
            return FALSE;
        }
    }


    if (CMD_RECEIVER == pOptions->cmd)
        pOptions->fillchar = 0;

    pOptions->buf = (char*)Malloc(pOptions->nBufSize);
    
    if (!pOptions->buf)
        return FALSE;
    else
    {
        FillMemory(pOptions->buf, pOptions->nBufSize, pOptions->fillchar);
        return TRUE;
    }
}




// Abstract:
//      Just print the options to be used for this run.
//
static VOID PrintOptions(OPTIONS *pOptions)
{
    printf("Options\n");
    if (CMD_ENUMERATOR == pOptions->cmd)
        printf("  Enumerate ATM interfaces\n");
    else if (CMD_RECEIVER == pOptions->cmd)
        printf("  Act as Receiver and bind to local interface <%s>\n",
               pOptions->szLocalInterface);
    else
    {
        printf("  Act as sender and send to remote interface <%s>\n", pOptions->szRemoteInterface);
        printf("  Sleep %d milliseconds between sends\n", pOptions->dwSleep);
        printf("  Fill buffer with <%c>\n", pOptions->fillchar);
        if (pOptions->nRepeat)
            printf("  Repeat sending the buffer %d times\n", pOptions->nRepeat);
        else
            printf("  Repeat sending the buffer continually\n");
    }
    printf("  Bufsize %d (bytes)\n", pOptions->nBufSize);
    printf("\n");
    return;
}




// Abstract:
//      Print out usage table for the program
//
VOID Usage(CHAR *szProgramname, OPTIONS *pOptions)
{
    printf("usage:\n  %s -?\n\n", szProgramname);
    printf("  %s [-a | -i:interface | -n:interface] [-b:#] [-d:c] [-l:#] [-s:#]\n\n",
           szProgramname);
    printf("  -?\t\tDisplay this help\n");
    printf("\n");
    printf("  -a\t\tPrint out all ATM interfaces on this machine\n");
    printf("  -i:interface\tAct as receiver and bind to interface\n");
    printf("  -n:interface\tAct as sender and connect to remote interface\n");
    printf("\n");
    printf("  -b:bufsize\tSize of send/recv buffer; in 1K increments (Def:%d)\n",
           pOptions->nBufSize);
    printf("  -c:#\t\tSleep # milliseconds before closing socket (def: 2000)\n");
    printf("  -d:c\t\tCharacter used to fill buffer (Def:%c)\n",
           pOptions->fillchar);
    printf("  -l:loop\tLoop count for sending buffer (Def:continuous)\n");
    printf("  -s:#\t\tSleep # milliseconds between sends (def: 0)\n");
    printf("\n");
    return;
}



// Abstract:
//      Look for an ATM service provider.  
//
//      Notice that actual service provider names are different on NT and Win9x 
//      when searching for a service provider, you should not use the service provider 
//      name, but instead use the various flags in the protocol structure.
//
BOOL FindATMServiceProvider(
                           WSAPROTOCOL_INFO    *pProtocolInfo
                           )
{
    int nRet = 0;
    WSAPROTOCOL_INFO *lpProtocolBuf = NULL;
    DWORD dwBufLen = 0;
    DWORD dwErr = 0;
    BOOL bProtocolFound = FALSE;


    // first have WSAEnumProtocols tell us how big a buffer is needed
    nRet = WSAEnumProtocols(NULL, lpProtocolBuf, &dwBufLen);
    if (SOCKET_ERROR != nRet)
    {
        printf("WSAEnumProtocols: should not have suceeded\n");
    } else if (WSAENOBUFS != (dwErr = WSAGetLastError()))
    {
        // WSAEnumProtocols failed for some reason not relating to buffer size 
        printf("WSAEnumProtocols(1): %d\n", WSAGetLastError());
    } else
    {
        // WSAEnumProtocols failed for the "expected" reason, allocate a buffer
        // of the needed size
        lpProtocolBuf = (WSAPROTOCOL_INFO *)Malloc(dwBufLen);
        if (lpProtocolBuf)
        {
            // now we can call WSAEnumProtocols again with the expectation it will
            // succeed because we have allocated a big enough buffer.
            nRet = WSAEnumProtocols(NULL, lpProtocolBuf, &dwBufLen);
            if (SOCKET_ERROR == nRet)
            {
                printf("WSAEnumProtocols(3): %d\n", WSAGetLastError());
            } else
            {
                // loop thru protocols, looking for a matching service provider 
                for (int i=0; i<nRet; i++)
                {
                    printf("  sp <%s>\n", lpProtocolBuf[i].szProtocol);
                    if (AF_ATM        == lpProtocolBuf[i].iAddressFamily &&
                        ATMPROTO_AAL5 == lpProtocolBuf[i].iProtocol &&
                        SOCK_RAW      == lpProtocolBuf[i].iSocketType)
                    {
                        *pProtocolInfo = lpProtocolBuf[i];
                        bProtocolFound = TRUE;
                        break;
                    }
                }     
            } 
            if (lpProtocolBuf) Free(lpProtocolBuf);
        }
    } 

    if (bProtocolFound)
        printf("  Using service provider <%s>\n\n", pProtocolInfo->szProtocol);

    return bProtocolFound;
}

