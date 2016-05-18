/******************************************************************************\
*       This is a part of the Microsoft Source Code Samples.
*       Copyright 1996 - 2000 Microsoft Corporation.
*       All rights reserved.
*       This source code is only intended as a supplement to
*       Microsoft Development Tools and/or WinHelp documentation.
*       See these sources for detailed information regarding the
*       Microsoft samples programs.
\******************************************************************************/

/*
Module Name:

    bcast.c

Abstract:

    This module illustrates the Win32 Winsock and Mailslot APIs to do a generic
    broadcast over IPX, UDP and Mailslot protocols.

    This example implements a client and a server. The example has a number of
    command line options. For example,

    -s To run the example as a server(default role).
    
    -c To run the example as a client.
   
    -p <i or m or u> To specify the protocol to be used.
     i - IPX.
     m - Mailslots.
     u - UDP(default protocol).
    
    -e <Endpoint> To specify an end point of your choice. This is a mandatory
    parameter. Servers create this endpoint and read broadcast messages. An 
    endpoint in case Mailslot protocol is a Mailslot name.(default is 5005). 
    
    -d <DomainName> - To specify a domain name or a workgroup name. This is 
    useful for Mailslot clients, only.

    To run the application as a server, the following command lines can be 
    specified:
    
    bcast -s -e 8000 -p u
    bcast -s -e 8000 -p i
    bcast -s -e MAILSLOT1 -p m

    To run the application as a client, the following command lines can be
    specified:
    
    bcast -c -e 8000 -p u
    bcast -c -e 8000 -p i
    bcast -c -e MAILSLOT1 -p m -d DOMAIN1
    bcast -c -e MAILSLOT1 -p m -d WORKGROUP1

Author:

    Rajesh Dadhia (rajeshd) 02-Mar-96

Revision History:

    Stephen R. Husak 06-Dec-99		fix for empty commandline
*/

#pragma warning (disable:4127)

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <strsafe.h>
#include <winsock.h>
#include <wsipx.h>
#include <wsnwlink.h>



#define MAX_MSGLEN 80
#define MAX_ADDLEN 80
#define MAX_MSLOTNAME 80

typedef enum _MODE
{
    CLIENT=0, SERVER
} MODE;
typedef enum _PROTOCOL
{
    UDP=0, IPX, MAILSLOT
} PROTOCOL;

BOOL __stdcall
CtrlCHandler (
             DWORD dwEvent
             );

void __stdcall
DoMailSlot (
           MODE mRole,
           LPSTR lpsEndPoint,
           LPSTR lpsDomainName
           );

void __stdcall
DoIpx (
      MODE mRole,
      USHORT usEndPoint
      );

void __stdcall
DoUdp (
      MODE mRole,
      USHORT usEndPoint
      );

void __stdcall
DoMailSlotServer (
                 LPSTR lpsEndPoint
                 );

void __stdcall
DoMailSlotClient (
                 LPSTR lpsEndPoint,
                 LPSTR lpsDomainName
                 );

void __stdcall
DoUdpServer (
            USHORT usEndPoint
            );

void __stdcall
DoUdpClient (
            USHORT usEndPoint
            );

void __stdcall 
DoIpxServer (
            USHORT usEndPoint
            );

void __stdcall
DoIpxClient (
            USHORT usEndPoint
            );

void __stdcall
Usage (
      CHAR *pszProgramName
      );

void __stdcall
PrintError (
           LPSTR lpszRoutine,
           LPSTR lpszCallName,
           DWORD dwError
           );

CHAR * __stdcall
IpxnetAddr (
           CHAR *lpBuffer,
           CHAR *lpsNetnum, 
           CHAR *lpsNodenum 
           );

void __stdcall
DoStartup ( void );

void __stdcall
DoCleanup ( void );

//
// Global Variables
//

// If Startup was successful, fStarted is used to keep track.
BOOL fStarted = FALSE;

// Global socket descriptor
SOCKET sock = INVALID_SOCKET;

//GLobal Mail Slot handle for server and client
HANDLE hSlot = INVALID_HANDLE_VALUE;

void __cdecl
main (
     INT argc,
     CHAR **argv
     )
{

    // Default role of the application is SERVER, which means receiver of
    // the broadcast messages.
    MODE mRole = SERVER;

    // Deafult protocol used is UDP.
    PROTOCOL pProto = UDP;

    // Default Endpoint.
    USHORT usEndPoint = 5005;

    // Strings pointing to Endpoint and DomainName(necessary for Mailslots).
    LPSTR lpsEndPoint = NULL, lpsDomainName = NULL;
    INT  i = 0;
    CHAR chProto = '\0';

    //
    // Install the CTRL+BREAK Handler
    //
    if ( FALSE == SetConsoleCtrlHandler ( (PHANDLER_ROUTINE) CtrlCHandler,
                                          TRUE 
                                        ) )
    {
        PrintError ( "main", "SetConsoleCtrlHandler", GetLastError ( ) );
    }

    //
    // allow the user to override settings with command line switches
    //
    for ( i = 1; i < argc; i++ )
    {
        if ( ( *argv[i] == '-' ) || ( *argv[i] == '/' ) )
        {
            switch ( tolower ( *( argv[i]+1 ) ) )
            {
            //
            // Role of the application (Client - Sender of broadcasts).
            //
            case 'c':  
                mRole = CLIENT;
                break;

                //
                // Role of the application (Server - Receiver of broadcasts)
                //
            case 's':  
                mRole = SERVER;
                break;

                //
                // Network protocol (Mailslots, IPX or UDP).
                //
            case 'p':
                chProto = (char)tolower ( *argv[++i] );
                if ( 'm' == (char)chProto )
                {
                    pProto = MAILSLOT;
                } else if ( 'i' == chProto )
                {
                    pProto = IPX;
                } else
                    pProto = UDP;
                break;

                //
                // EndPoint.
                //
            case 'e': 
                lpsEndPoint = argv[++i];
                break;

                //
                // DomainName (Important for Mailslot broadcasts, only).
                //
            case 'd':
                lpsDomainName = argv[++i];
                break;

                //
                // Help.
                //
            case 'h':
            case '?':
            default:
                Usage ( argv[0] );
            }
        } else
            //
            // Help.
            //
            Usage ( argv[0] );
    }

    //
    // If the protocol specified is not MAILSLOT, convert the endpoint
    // information to integer format from the string format.
    //
    if ( MAILSLOT != pProto )
    {
        if (lpsEndPoint != NULL)
            usEndPoint = (USHORT)atoi ( lpsEndPoint );
    }

    //
    // Print a Summary of the switches specfied 
    // Helpful for debugging
    //
    fprintf ( stdout, "SUMMARY:\n" );
    fprintf ( stdout, "\tRole-> %s\n", (CLIENT == mRole)?"Client":"Server" );
    fprintf ( stdout, "\tProtocol-> %s\n", 
              ( MAILSLOT == pProto ) ? "MAILSLOT" : 
              ( IPX == pProto ) ? "IPX" : "UDP" );
    if (lpsEndPoint != NULL)
        fprintf ( stdout, "\tEndpoint-> %s\n", lpsEndPoint );
    else
        fprintf ( stdout, "\tEndPoint-> %d\n", usEndPoint );

    //
    // Check the protocol specified.
    // Call the appropriate handler rouine. By default the protocol
    // is UDP.
    //
    switch ( pProto )
    {
    case MAILSLOT :
        DoMailSlot ( mRole, lpsEndPoint, lpsDomainName );
        break;

    case IPX:
        DoStartup ( );
        DoIpx ( mRole, usEndPoint );
        break;

    default:
        DoStartup ( );
        DoUdp ( mRole, usEndPoint );
        break;
    }

    return;
}

//
// CtrlCHandler () intercepts the CTRL+BREAK or CTRL+C events and calls the
// cleanup routines.
//
BOOL __stdcall
CtrlCHandler (
             DWORD dwEvent
             )
{
    if ( ( CTRL_C_EVENT == dwEvent ) || ( CTRL_BREAK_EVENT == dwEvent ) )
    {
        DoCleanup ( );    
    }

    return FALSE;
}

//
// DoMailSlot () function calls appropriate handler function (client/server),
// if protocol=MAILSLOT is specified. By default, the role of the application
// is - SERVER.
//
void __stdcall
DoMailSlot (
           MODE mRole,
           LPSTR lpsEndPoint,
           LPSTR lpsDomainName
           )
{
    switch ( mRole )
    {
    case CLIENT:
        DoMailSlotClient ( lpsEndPoint, lpsDomainName );
        break;

    default:
        DoMailSlotServer ( lpsEndPoint );
    }
    return;
}

//
// DoIpx () function calls appropriate handler function (client/server),
// if protocol=IPX  is specified. By default, the role of the application
// is - SERVER.
//
void __stdcall
DoIpx (
      MODE mRole,
      USHORT usEndPoint
      )
{
    //
    // Initialize the global socket descriptor.
    //
    sock = socket ( AF_IPX, SOCK_DGRAM, NSPROTO_IPX );

    if ( INVALID_SOCKET ==  sock )
    {
        PrintError( "DoIpx", "socket", WSAGetLastError ( ) );
    }

    switch ( mRole )
    {
    case CLIENT:
        DoIpxClient ( usEndPoint );
        break;

    default:
        DoIpxServer ( usEndPoint );
    }
    return;
}


//
// DoUdp () function calls appropriate handler function (client/server),
// if protocol=UDP  is specified. By default, the role of the application
// is - SERVER.
//
void __stdcall
DoUdp (
      MODE mRole,
      USHORT usEndPoint
      )
{
    //
    // Initialize the global socket descriptor.
    //
    sock = socket ( AF_INET, SOCK_DGRAM, 0 );

    if ( INVALID_SOCKET ==  sock)
    {
        PrintError ( "DoUdp", "socket", WSAGetLastError() );
    }

    switch ( mRole )
    {
    case CLIENT:
        DoUdpClient ( usEndPoint );
        break;

    default:
        DoUdpServer ( usEndPoint );
    }
    return;
}

//
// DoMailSlotServer () function receives a mailslot message on a particular
// mailslot. The function creates a mailslot, posts a ReadFile () to receive
// the message. The function then checks the first four bytes of the message
// for the message ID, and discards all the messages with same ID, in future.
//
void __stdcall
DoMailSlotServer (
                 LPSTR lpsEndPoint
                 )
{


    // Variables that store MessageID, previous messageID, number of bytes to 
    // read/read, size of next available message and the number of messages.
    DWORD dwMessageID = 0,
    dwPrevID = 0,
    cbMessage = 0,
    cbRead = 0, 
    cbToRead = 0, 
    nMessages  = 0;
    HRESULT hRet;
    size_t dwSize;

    BOOL fResult = FALSE;

    CHAR achMailSlotName[MAX_MSLOTNAME], 
    achBuffer[MAX_MSGLEN + sizeof ( DWORD )];

    // Variable that points past the message ID part in the message.
    LPSTR lpsMessage = NULL;

    //
    // Create a string for the mailslot name.
    //

    if(FAILED(hRet = StringCchPrintf(achMailSlotName, MAX_MSLOTNAME, "\\\\.\\mailslot\\%s", lpsEndPoint)))
    {
        PrintError("DoMailSlotServer","StringCchPrintf",hRet);
    }


    //
    // Create the mailslot.
    //
    hSlot = CreateMailslot ( achMailSlotName,
                             0,
                             MAILSLOT_WAIT_FOREVER,
                             (LPSECURITY_ATTRIBUTES) NULL
                           );

    if ( INVALID_HANDLE_VALUE == hSlot )
    {
        PrintError ( "DoMailSlotServer",  "CreateMailSlot", GetLastError() );
    }

    //
    // Post ReadFile() and read a message.
    //
    cbToRead = MAX_MSGLEN + sizeof (DWORD);

    fResult = ReadFile ( hSlot,
                         achBuffer,
                         cbToRead,
                         &cbRead,
                         (LPOVERLAPPED) NULL
                       );

    if ( TRUE != fResult )
    {
        PrintError ( "DoMailSlotServer", "ReadFile", GetLastError() );
    }

    achBuffer[cbRead] = '\0';

    //
    // Get the message ID part from the message.
    //
    
    memcpy ( &dwMessageID, achBuffer, sizeof ( DWORD ) );

    //
    // Adjust the actual message pointer.
    //
    lpsMessage = achBuffer + sizeof ( DWORD );

    hRet = StringCbLength(lpsMessage,cbRead,&dwSize);

    //
    // Print the message
    //
    fprintf ( stdout, 
              "A MailSlot Message of %d bytes received with ID %d\n", 
              dwSize, 
              dwMessageID 
            );

    fprintf ( stdout, "MessageText->%s\n", lpsMessage );

    //
    // Check for duplicate messages.
    //
    dwPrevID = dwMessageID;

    while ( TRUE )
    {
        //
        // get information on pending messages.
        //
        fResult = GetMailslotInfo ( hSlot,
                                    (LPDWORD) NULL,
                                    &cbMessage, 
                                    &nMessages,
                                    (LPDWORD) NULL
                                  );
        if ( TRUE != fResult )
        {
            PrintError ( "DoMailSlotServer", 
                         "GetMailSlotInfo", 
                         GetLastError ( ) 
                       );
        }

        //
        // Break if no more messages.
        //
        if ( MAILSLOT_NO_MESSAGE == cbMessage )
            break;

        //
        // We now know how much to read.
        //
        cbToRead = cbMessage;
        fResult = ReadFile ( hSlot,
                             achBuffer,
                             cbToRead,
                             &cbRead,
                             (LPOVERLAPPED) NULL
                           );

        if ( TRUE != fResult )
        {
            PrintError ( "DoMailSlotServer",
                         "ReadFile", 
                         GetLastError ( ) 
                       );
        }

        achBuffer[cbRead] = '\0';
        memcpy ( &dwMessageID, achBuffer, sizeof (DWORD) );

        //
        // print the message only if it is not a duplicate.
        //
        lpsMessage = achBuffer + sizeof (DWORD);
        hRet = StringCbLength(lpsMessage,cbRead,&dwSize);
        if ( dwMessageID != dwPrevID )
        {
            
            
            fprintf ( stdout, 
                      "A MailSlot Message of %d bytes received with ID %d\n", 
                      dwSize, 
                      dwMessageID 
                    );

            fprintf ( stdout, "MessageText->%s\n", achBuffer );
        }

        dwPrevID = dwMessageID;
    }

    //
    // Close the handle to our mailslot.
    //
    fResult = CloseHandle ( hSlot );

    if ( TRUE != fResult )
    {
        PrintError ( "DoMailSlotServer", "CloseHandle", GetLastError() );
    }
    return;
}

//
// DoMailSlotClient () function implements the broadcast routine for a
// Mailslot client. The function opens handle to the mailslot using 
// CreateFile (). CreateFile will fail on Windows NT for local mailslots,
// if the mailslot is not already created using CreateMailSlot () API. 
//  
// The function appends a message number to the message which the server uses
// to discard duplicate messages. In the event of a client runnig on a system
// with multiple transport protocols loaded, a mailsot message is sent over
// each protocol.
//
// This routine broadcasts a mailslot message to everyone on a Windows NT
// domain, it can also be used to send a mailslot message to a particular
// host or a workgroup.
//
void __stdcall
DoMailSlotClient (
                 LPSTR lpsEndPoint,
                 LPSTR lpsDomainName
                 )
{
    // Variables that store MessageID, number of bytes to write/written.
    DWORD dwMessageID = 0, 
    cbWritten = 0,
    cbToWrite = 0;
    HRESULT hRet;

    BOOL fResult = FALSE;

    CHAR achMailSlotName[MAX_MSLOTNAME],
    achBuffer[MAX_MSGLEN + sizeof ( DWORD ) ];

    if ( NULL == lpsDomainName )
    {
        fprintf ( stdout, 
                  "Domain/Workgroup name must be specified....Exiting\n"
                );
        exit ( 1 );
    }

    //
    // Create a string for the mailslot name.
    //


    if(FAILED(hRet = StringCchPrintf(achMailSlotName,
                           MAX_MSLOTNAME,
                           "\\\\%s\\mailslot\\%s",
                           lpsDomainName,
                           lpsEndPoint
                          )))
    {
        PrintError("DoMailSlotClient","StringCchPrintf",hRet);
    }


    //
    // Open a handle to the mailslot.
    //
    hSlot = CreateFile ( achMailSlotName,
                         GENERIC_WRITE,
                         FILE_SHARE_READ,
                         (LPSECURITY_ATTRIBUTES) NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         (HANDLE) NULL 
                       );

    if ( INVALID_HANDLE_VALUE == hSlot)
    {
        PrintError ( "DoMailSlotClient", "CreateFile", GetLastError ( ) );
    }

    //
    // Generate a Message ID.
    //
    srand ( (UINT) time ( NULL) );
    dwMessageID = rand ( );

    if(FAILED(hRet = StringCbPrintf(achBuffer,sizeof(DWORD),"%d",dwMessageID)))
    {
        PrintError("DoMailSlotClient","StringCchPrintf",hRet);
    }
    
    if(FAILED(hRet = StringCchCopy(achBuffer + sizeof(DWORD),MAX_MSGLEN,"A MailSlot Broadcast Message")))
    {
        PrintError("DoMailSlotClient","StringCchCopy",hRet);
    }

    //
    // Total number of bytes to write.
    //
    cbToWrite = sizeof ( DWORD ) + (int) ( strlen ( achBuffer + sizeof ( DWORD ) ) );

    //
    // Send a mailslot message.
    //
    fResult = WriteFile ( hSlot,
                          achBuffer,
                          cbToWrite,
                          &cbWritten,
                          (LPOVERLAPPED) NULL
                        );

    if ( TRUE != fResult )
    {
        PrintError ( "DoMailSlotClient", "WriteFile", GetLastError ( ) );
    }

    fprintf ( stdout, 
              "%d bytes of MailSlot data broadcasted with ID %d\n", 
              cbWritten, 
              dwMessageID
            );

    //
    // Close the mailslot handle.
    //
    fResult = CloseHandle ( hSlot );

    if ( TRUE != fResult )
    {
        PrintError ( "DoMailSlotClient", "CloseHandle", GetLastError() );
    }
    return;
}

//
// DoUdpServer () function receives the broadcast on a specified port. The
// server will have to post a recv (), before the client sends the broadcast.
// 
void __stdcall
DoUdpServer (
            USHORT usEndPoint
            )
{

    // IP address structures needed to bind to a local port and get the sender's
    // information.
    SOCKADDR_IN saUdpServ = {0}, saUdpCli = {0};

    INT err = 0, nSize = 0;

    CHAR achBuffer[MAX_MSGLEN] = {'\0'};

    //
    // bind to the specified port.
    //
    saUdpServ.sin_family = AF_INET;
    saUdpServ.sin_addr.s_addr = htonl ( INADDR_ANY );
    saUdpServ.sin_port = htons ( usEndPoint );

    err = bind ( sock, (SOCKADDR FAR *)&saUdpServ, sizeof ( SOCKADDR_IN ) );

    if ( SOCKET_ERROR == err )
    {
        PrintError ( "DoUdpServer", "bind", WSAGetLastError ( ) );
    }

    //
    // receive a datagram on the bound port number.
    //
    nSize = sizeof ( SOCKADDR_IN );
    err = recvfrom ( sock,
                     achBuffer,
                     MAX_MSGLEN,
                     0,
                     (SOCKADDR FAR *) &saUdpCli,
                     &nSize
                   );

    if ( SOCKET_ERROR == err )
    {
        PrintError ( "DoUdpServer", "recvfrom", WSAGetLastError ( ) );
    }

    //
    // print the sender's information.
    //
    achBuffer[err] = '\0';
    fprintf ( stdout, "A Udp Datagram of length %d bytes received from ", err );
    fprintf ( stdout, "\n\tIP Adress->%s ", inet_ntoa ( saUdpCli.sin_addr ) );
    fprintf ( stdout, "\n\tPort Number->%d\n", ntohs ( saUdpCli.sin_port ) );
    fprintf ( stdout, "MessageText->%s\n", achBuffer );

    //
    // Call the cleanup routine
    //
    DoCleanup ( );

    return;
}

//
// DoUdpClient () function implements the broadcast routine for an UDP
// client. The function sets the SO_BROADCAST option with the global socket.
// Calling this API is important. After binding to a local port, it sends an 
// UDP boradcasts to the IP address INADDR_BROADCAST, with a particular
// port number.
//
void __stdcall
DoUdpClient (
            USHORT usEndPoint
            )
{

    // IP address structures needed to fill the source and destination 
    // addresses.
    SOCKADDR_IN saUdpServ = {0}, saUdpCli = {0};
    INT err = 0;
    CHAR achMessage[MAX_MSGLEN] = {'\0'};
    HRESULT hRet;
    size_t dwSize;

    // Variable to set the broadcast option with setsockopt ().
    BOOL fBroadcast = TRUE;


    err = setsockopt ( sock, 
                       SOL_SOCKET,
                       SO_BROADCAST,
                       (CHAR *) &fBroadcast,
                       sizeof ( BOOL )
                     );

    if ( SOCKET_ERROR == err )
    {
        PrintError ( "DoUdpClient", "setsockopt", WSAGetLastError ( )  );
    }

    //
    // bind to a local socket and an interface.
    //
    saUdpCli.sin_family = AF_INET;
    saUdpCli.sin_addr.s_addr = htonl ( INADDR_ANY );
    saUdpCli.sin_port = htons ( 0 );

    err = bind ( sock, (SOCKADDR *) &saUdpCli, sizeof (SOCKADDR_IN) );

    if ( SOCKET_ERROR == err )
    {
        PrintError ( "DoUdpClient", "bind", WSAGetLastError ( ) );
    }

    //
    // Fill an IP address structure, to send an IP broadcast. The 
    // packet will be broadcasted to the specified port.
    //
    saUdpServ.sin_family = AF_INET;
    saUdpServ.sin_addr.s_addr = htonl ( INADDR_BROADCAST );
    saUdpServ.sin_port = htons ( usEndPoint );



    if(FAILED(hRet = StringCchCopy(achMessage,MAX_MSGLEN,"A Broadcast Datagram")))
    {
        PrintError("DoUdpClient","StringCchCopy",hRet);
    }

    if(FAILED(hRet = StringCbLength(achMessage,MAX_MSGLEN,&dwSize)))
    {
        PrintError("DoUdpClient","StringCbLength",hRet);
    }

    err = sendto ( sock,
                   achMessage,
                   (int)dwSize,
                   0,
                   (SOCKADDR *) &saUdpServ,
                   (int)sizeof ( SOCKADDR_IN )
                 );

    if ( SOCKET_ERROR == err )
    {
        PrintError ( "DoUdpClient", "sendto", WSAGetLastError ( ) );
    }

    fprintf ( stdout, "%d bytes of data broadcasted\n", err );

    //
    // Call the cleanup routine.
    //
    DoCleanup ( );

    return;
}


//
// DoIpxServer () function receives the broadcast on a specified socket. The
// server will have to post a recv (), before the client sends the broadcast. 
// It is necessary call setsockopt () with SO_BROADCAST flag set, in order to
// receive IPX broadcasts on Windows 95.
//
void __stdcall 
DoIpxServer (
            USHORT usEndPoint
            )
{

    // IPX address structures needed to bind to a local socket and get the
    // sender's information.
    SOCKADDR_IPX saIpxServ = {0}, saIpxCli = {0};

    INT err = 0, nSize = 0;

    CHAR achBuffer[MAX_MSGLEN] = {'\0'}, 
    achAddress[MAX_ADDLEN] = {'\0'};

    OSVERSIONINFO osVer = {0};

    // Variable to set the broadcast option with setsockopt ().
    BOOL fResult = FALSE, fBroadcast = TRUE;

    //
    // Check the platform.
    //
    osVer.dwOSVersionInfoSize = sizeof ( OSVERSIONINFO );
    fResult  = GetVersionEx ( &osVer);

    if ( FALSE == fResult)
    {
        PrintError ( "DoIpxServer", "GetVersionEx", GetLastError ( ) );
    }

    //
    // If the platform is Windows 95, call setsockopt ().
    //
    if ( VER_PLATFORM_WIN32_WINDOWS == osVer.dwPlatformId )
    {
        err = setsockopt ( sock, 
                           SOL_SOCKET, 
                           SO_BROADCAST, 
                           (CHAR *) &fBroadcast, 
                           sizeof ( BOOL ) 
                         );

        if ( SOCKET_ERROR == err )
        {
            PrintError ( "DoIpxServer", "setsockopt", WSAGetLastError() );
        }
    }

    //
    // bind to the specified socket.
    //
    saIpxServ.sa_family = AF_IPX;
    saIpxServ.sa_socket = usEndPoint;
    memset ( saIpxServ.sa_netnum, 0, sizeof (saIpxServ.sa_netnum ) );
    memset ( saIpxServ.sa_nodenum, 0, sizeof (saIpxServ.sa_nodenum ) );

    err = bind ( sock, (SOCKADDR *) &saIpxServ, sizeof (SOCKADDR_IPX) );

    if ( SOCKET_ERROR == err )
    {
        PrintError ( "DoIpxServer", "bind", WSAGetLastError ( ) );
    }

    //
    // receive a datagram on the bound socket number.
    //
    nSize = sizeof ( SOCKADDR_IPX );
    err = recvfrom ( sock,
                     achBuffer,
                     MAX_MSGLEN,
                     0,
                     (SOCKADDR *) &saIpxCli,
                     &nSize 
                   );

    if ( SOCKET_ERROR == err )
    {
        PrintError ( "DoIpxServer", "recvfrom", WSAGetLastError ( ) );
    }

    //
    // print the sender's information.
    //
    achBuffer[err] = '\0';
    fprintf ( stdout, 
              "An Ipx Datagram of length %d bytes received from ", 
              err );
    fprintf ( stdout, 
              "\n\tIPX Adress->%s ", 
              IpxnetAddr ( achAddress, 
                           saIpxCli.sa_netnum, 
                           saIpxCli.sa_nodenum 
                         ) 
            );
    fprintf ( stdout, "\n\tSocket Number->%d\n", saIpxCli.sa_socket );
    fprintf ( stdout, "MessageText->%s\n", achBuffer );

    //
    // Call the cleanup routine.
    //
    DoCleanup ( );
    return;              

}

//
// DoIpxClient () function implements the broadcast routine for a an IPX
// client. The fucntion sets the SO_BROADCAST option with the gloabal socket.
// Calling this API is important. After binding to a local port, it sends an IPX
// packet to the address with node number as all 1's and net number as all 0's,
// with a particuler socket number.
//
void __stdcall
DoIpxClient (
            USHORT usEndPoint
            )
{

    // IPX address structures needed to fill the source and destination 
    // addresses.
    SOCKADDR_IPX saIpxServ = {0}, saIpxCli = {0};
    INT err = 0;
    CHAR achMessage[MAX_MSGLEN] = {'\0'};
    HRESULT hRet;
    size_t dwSize;

    // Variable to set the broadcast option with setsockopt ().
    BOOL fBroadcast = TRUE;

    err = setsockopt ( sock, 
                       SOL_SOCKET, 
                       SO_BROADCAST,
                       (CHAR *) &fBroadcast, 
                       sizeof ( BOOL ) 
                     );

    if ( SOCKET_ERROR == err )
    {
        PrintError ( "DoIpxClient", "setsockopt", WSAGetLastError ( ) );
    }

    //
    // bind to a local socket and an interface.
    //
    saIpxCli.sa_family = AF_IPX;
    saIpxCli.sa_socket = (USHORT) 0;
    memset ( saIpxCli.sa_netnum, 0, sizeof ( saIpxCli.sa_netnum ) );
    memset ( saIpxCli.sa_nodenum, 0, sizeof ( saIpxCli.sa_nodenum ) );

    err = bind ( sock, (SOCKADDR  *) &saIpxCli, sizeof ( SOCKADDR_IPX ) );

    if ( SOCKET_ERROR == err )
    {
        PrintError ( "DoIpxClient", "bind", WSAGetLastError() );
    }

    //
    // Fill an IPX address structure, to send an IPX broadcast. The 
    // packet will be broadcasted to the specified socket.
    // 
    saIpxServ.sa_family = AF_IPX;
    saIpxServ.sa_socket = usEndPoint;
    memset ( saIpxServ.sa_netnum, 0, sizeof ( saIpxServ.sa_netnum ) );
    memset ( saIpxServ.sa_nodenum, 0xFF, sizeof ( saIpxServ.sa_nodenum ) );

    if(FAILED(hRet = StringCchCopy(achMessage,MAX_MSGLEN,"A Broadcast Datagram")))
    {
        PrintError("DoIpxClient","StringCchCopy",hRet);
    }

    if(FAILED(hRet = StringCbLength(achMessage,MAX_MSGLEN,&dwSize)))
    {
        PrintError("DoIpxClient","StringCbLength",hRet);
    }


    err = sendto ( sock,
                   achMessage,
                   (int)dwSize,
                   0,
                   (SOCKADDR *) &saIpxServ,
                   sizeof ( SOCKADDR_IPX )
                 );

    if ( SOCKET_ERROR == err )
    {
        PrintError ( "DoIpxClient", "sendto", WSAGetLastError ( ) );
    }

    fprintf ( stdout, "%d bytes of data broadcasted\n", err);

    //
    // Call the cleanup routine.
    //
    DoCleanup ( );

    return;
}

//
// Usage () lists the available command line options.
//
void __stdcall
Usage (
      CHAR *pszProgramName
      )
{
    fprintf ( stderr, "Usage:  %s\n", pszProgramName );
    fprintf ( stderr, 
              "\t-s or -c (s - server, c - client, default - server)\n" );
    fprintf ( stderr, 
              "\t-p <i or m or u> (i - IPX, m - Mailslots, u - UDP)\n" );
    fprintf ( stderr, "\t-e <Endpoint>\n" );
    fprintf ( stderr, 
              "\t-d <DomainName> - needed only for a Mailslot client\n" );
    fprintf ( stderr, 
              "\n\tDefault Values-> Role:Server, Protocol:UDP, EndPoint:5005\n" );

    exit ( 1 );
}


//
// PrintError () is a function available globally for printing the error and 
// doing the cleanup.
//
void __stdcall
PrintError (
           LPSTR lpszRoutine,
           LPSTR lpszCallName,
           DWORD dwError
           )
{

    fprintf ( stderr, 
              "The Call to %s() in routine() %s failed with error %d\n", 
              lpszCallName, 
              lpszRoutine,
              dwError 
            );

    DoCleanup ( );

    exit ( 1 );
}

//
// IpxnetAddr () function converts an IPX address address in the binary form
// to ascii format, it fills the input buffer with the address and returns a
// pointer to it.
//
CHAR * __stdcall
IpxnetAddr (
           CHAR *lpBuffer,
           CHAR *lpsNetnum, 
           CHAR *lpsNodenum 
           )
{

    HRESULT hRet;

    if (NULL == lpBuffer)
        return NULL;
    if (NULL == lpsNetnum)
        return NULL;
    if (NULL == lpsNodenum)
        return NULL;


    if(FAILED(hRet = StringCchPrintf(lpBuffer,
                           MAX_MSGLEN,
                           "%02X%02X%02X%02X.%02X%02X%02X%02X%02X%02X",
                           (UCHAR) lpsNetnum[0], (UCHAR) lpsNetnum[1], 
                           (UCHAR) lpsNetnum[2], (UCHAR) lpsNetnum[3],
                           (UCHAR) lpsNodenum[0], (UCHAR) lpsNodenum[1],
                           (UCHAR) lpsNodenum[2], (UCHAR) lpsNodenum[3],
                           (UCHAR) lpsNodenum[4], (USHORT) lpsNodenum[5]
                          )))
    {
        PrintError("IpxnetAddr","StringCchPrintf",hRet);
    }




    return( lpBuffer);
}

//
// DoStartup () initializes the Winsock DLL with Winsock version 1.1
//
void __stdcall
DoStartup ( void )
{
    WSADATA wsaData;

    INT iRetVal = 0;

    iRetVal = WSAStartup ( MAKEWORD ( 1,1 ), &wsaData );

    if ( 0 != iRetVal)
    {
        PrintError ( "DoStartup", "WSAStartup", iRetVal );
    }

    //
    // Set the global flag.
    //
    fStarted = TRUE;

    return;
}

//
// DoCleanup () will close the global socket which was opened successfully by
// a call to socket (). Additionally, it will call WSACleanup (), if a call
// to WSAStartup () was made successfully.
//
void __stdcall
DoCleanup ( void )
{
    if ( INVALID_SOCKET != sock )
    {
        closesocket ( sock );
        sock = INVALID_SOCKET;
    }

    if (INVALID_HANDLE_VALUE != hSlot)
    {
        CloseHandle(hSlot);
        hSlot = INVALID_HANDLE_VALUE;
    }

    if ( TRUE == fStarted )
    {
        WSACleanup ( );
    }

    fprintf ( stdout, "DONE\n" );

    return;
}
