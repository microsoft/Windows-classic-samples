// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*++

Module Name:

    Tcpserver.c

Abstract:

    This file contains sample code for a secure TCP Winsock server that
    accepts secure connections from TCP clients using the Secure Socket API.

--*/

#ifndef UNICODE
#define UNICODE
#endif

#include <rpc.h>
#include <ntdsapi.h>
#include <wchar.h>
#include <Winsock2.h>
#include <mstcpip.h>
#include <ws2tcpip.h>
#include "Tcpcommon.h"

#define SERVER_PORT 27015

DWORD
SecureTcpAcceptConnection(
   IN int addrFamily,
   IN const SOCKET_SECURITY_SETTINGS* securitySettings,
   IN ULONG settingsLen
)
/*++

Routine Description:

    This routine creates a TCP server socket, securely accepts connections from
    clients, impersonates the client for access checks, and securely 
    sends & receives data from the client

Arguments:

    addrFamily - Winsock address family that should be used for creating the 
                 socket.
    securitySettings - pointer to the socket security settings that should be
                       applied to the socket

    settingsLen - length of securitySettings in bytes

Return Value:

    Winsock error code indicating the status of the operation, or NO_ERROR if 
    the operation succeeded.

--*/
{
   DWORD result = 0;
   int sockErr = 0;
   SOCKET listenSock = INVALID_SOCKET;
   SOCKET clientSock = INVALID_SOCKET;
   WSABUF wsaBuf = {0};
   char* dataBuf = "91011121314";
   DWORD bytesSent = 0;
   char recvBuf[RECV_DATA_BUF_SIZE] = {0};
   DWORD bytesRecvd = 0;
   DWORD flags = 0;
   struct sockaddr_in v4Addr = {0};
   struct sockaddr_in6 v6Addr = {0};
   struct sockaddr* addr = NULL;
   ULONG addrLen = 0;

   //-----------------------------------------
   // Create a TCP socket
   //
   // Choosing IPv4 for illustration purposes. Otherwise with minor tweaks the 
   // sample code should work for IPv6 as well.
   listenSock = WSASocket(
                  addrFamily,
                  SOCK_STREAM,
                  IPPROTO_TCP,
                  NULL,
                  0,
                  0
                );
   if (listenSock == INVALID_SOCKET)
   {
      result = WSAGetLastError();
      wprintf(L"WSASocket returned error %ld\n", result); \
      goto cleanup;
   }

   //-----------------------------------------
   // Turn on security for the socket.
   sockErr = WSASetSocketSecurity (
               listenSock,
               securitySettings,
               settingsLen,
               NULL,
               NULL
            );
   if (sockErr == SOCKET_ERROR)
   {
      result = WSAGetLastError();
      wprintf(L"WSASetSocketSecurity returned error %ld\n", result);
      goto cleanup;
   }

   //-----------------------------------------
   // Bind the socket to the server port
   if(addrFamily == AF_INET)
   {
      v4Addr.sin_family = AF_INET;
      v4Addr.sin_port = htons(SERVER_PORT);
      addr = (struct sockaddr*)&v4Addr;
      addrLen = sizeof(v4Addr);
   }
   else
   {
      v6Addr.sin6_family = AF_INET6;
      v6Addr.sin6_port = htons(SERVER_PORT);
      addr = (struct sockaddr*)&v6Addr;
      addrLen = sizeof(v6Addr);
   }
   sockErr = bind(
               listenSock, 
               addr,
               addrLen
            );
   if (sockErr == SOCKET_ERROR)
   {
      result = WSAGetLastError();
      wprintf(L"bind returned error %ld\n", result);
      goto cleanup;
   }

   //-----------------------------------------
   // Listen for incoming connection requests on the bound socket
   sockErr = listen(listenSock, 10);
   if (sockErr == SOCKET_ERROR)
   {
      result = WSAGetLastError();
      wprintf(L"listen returned error %ld\n", result);
      goto cleanup;
   }
   
   while(TRUE)
   {
      //-----------------------------------------
      // Wait for client to connect
      wprintf(L"Listening on socket...\n");
      clientSock = WSAAccept(
                     listenSock,
                     NULL,
                     NULL,
                     NULL,
                     0
                   );
      if (listenSock == INVALID_SOCKET)
      {
         result = WSAGetLastError();
         wprintf(L"WSAAccept returned error %ld\n", result); \
         goto cleanup;
      }
      wprintf(L"Connected to a client\n");

      //-----------------------------------------
      // Match and print IPSec SA information for the connection 
      // (Note: this is optional)
      result = MatchIPsecSAsForConnectedSocket(clientSock);
      if (result)
      {
         wprintf(L"MatchIPsecSAsForConnectedSocket returned error %ld\n", result);
         goto cleanup;
      }

      //-----------------------------------------
      // Receive client's data
      wsaBuf.len = RECV_DATA_BUF_SIZE;
      wsaBuf.buf = recvBuf;
      sockErr = WSARecv(
                 clientSock,
                 &wsaBuf,
                 1,
                 &bytesRecvd,
                 &flags,
                 NULL,
                 NULL
               );
      if (sockErr == SOCKET_ERROR)
      {
         result = WSAGetLastError();
         wprintf(L"WSARecv returned error %ld\n", result);
         goto cleanup;
      }
      wprintf(L"Received %d bytes of data from the client\n", bytesRecvd);

      //-----------------------------------------
      // Impersonate the client 
      sockErr = WSAImpersonateSocketPeer (
                  clientSock,
                  NULL,
                  0
               );
      if (sockErr == SOCKET_ERROR)
      {
         result = WSAGetLastError();
         wprintf(L"WSAImpersonateSocketPeer returned error %ld\n", result);
         goto cleanup;
      }
      wprintf(L"Impersonating the client\n");

      //    At this point the server is impersonating the client and can access 
      // requested resources (such as files, etc) on behalf of the client.
      // This will ensure that all access controls associated with the
      // resources will be enforced for the client. If access checks fail, 
      // server should fail the connection.
      //    However if the server wants to instead perform access checks 
      // against an explicit security descriptor, then instead of impersonating
      // the client, it should get a handle to the client access token using 
      // WSAQuerySocketSecurity().
      
      //-----------------------------------------
      // Revert the impersonation 
      sockErr = WSARevertImpersonation();
      if (sockErr == SOCKET_ERROR)
      {
         result = WSAGetLastError();
         wprintf(L"WSARevertImpersonation returned error %ld\n", result);
         goto cleanup;
      }
      wprintf(L"Reverted Impersonation\n");

      //-----------------------------------------
      // Send response
      wsaBuf.len = (ULONG)strlen(dataBuf);
      wsaBuf.buf = dataBuf;
      sockErr = WSASend(
                 clientSock,
                 &wsaBuf,
                 1,
                 &bytesSent,
                 0,
                 NULL,
                 NULL
               );
      if (sockErr == SOCKET_ERROR)
      {
         result = WSAGetLastError();
         wprintf(L"WSASend returned error %ld\n", result);
         goto cleanup;
      }
      wprintf(L"Sent %d bytes of data to the client\n", bytesSent);

      //-----------------------------------------
      // Close the client socket
      closesocket(clientSock);
      clientSock = INVALID_SOCKET;
   }

cleanup:
   //Note this will trigger the cleanup of all IPsec filters and policies that
   //were added for this socket. The cleanup will happen only after all
   //outstanding data has been sent out on the wire.
   if(clientSock != INVALID_SOCKET)
   {
      closesocket(clientSock);
   }
   if(listenSock != INVALID_SOCKET)
   {
      closesocket(listenSock);
   }
   return result;
}

void ShowUsage(IN const wchar_t* progName)
/*++

Routine Description:

    This routine prints the intended usage for this program.

Arguments:

    progName - NULL terminated string representing the name of the executable

Return Value:

    None

--*/
{
   wprintf(L"Usage: %s [-adv] [-v6]\n", progName);
   wprintf(L"-adv: Enable advanced mode where a customized policy is specified by the application\n");
   wprintf(L"-v6: Use IPv6. Default is to use IPv4\n");
}

int __cdecl wmain(int argc, const wchar_t* const argv[])
{	
   DWORD result = 0;
   BOOL success = TRUE;
   WSADATA data;
   WORD version = MAKEWORD(2, 2);
   BOOL wsaCleanup = FALSE;
   SOCKET_SECURITY_SETTINGS basicSettings = {0};
   SOCKET_SECURITY_SETTINGS_IPSEC advSettings = {0};
   SOCKET_SECURITY_SETTINGS* settings = NULL;
   ULONG settingsLen = 0;
   BOOL useAdv = FALSE;
   int addrFamily = AF_INET;
   HANDLE fwpHandle = NULL;
   UINT32 i = 0;

   //-----------------------------------------
   // Parse the command line arguments
   if(argc > 3)
   {
      // Incorrect usage
      ShowUsage(argv[0]);
      goto cleanup;
   }

   for(i=1; i<(UINT32)argc; i++)
   {
      if(_wcsicmp(argv[i], L"-adv") == 0)
      {
         // Enable advanced mode
         useAdv = TRUE;
      }
      else if(_wcsicmp(argv[i], L"-v6") == 0)
      {
         // Use IPv6
         addrFamily = AF_INET6;
      }
      else
      {
         ShowUsage(argv[0]);
         goto cleanup;
      }
   }

   //----------------------
   // Initialize Winsock
   result = WSAStartup(version, &data);
   if (result)
   {
      wprintf(L"WSAStartup returned error %ld\n", result);
      goto cleanup;
   }
   // Set flag to indicate that WSACleanup() should be called.
   wsaCleanup = TRUE;

   if(useAdv)
   {
      //-----------------------------------------
      // Construct advanced socket security settings

      // Use IPsec security protocol and specify IPsec specific settings.
      advSettings.SecurityProtocol = SOCKET_SECURITY_PROTOCOL_IPSEC;
      // Guarantee encryption
      advSettings.SecurityFlags = SOCKET_SETTINGS_GUARANTEE_ENCRYPTION;
      // Create and add customized IPsec policy
      result = AddCustomIPsecPolicy(&fwpHandle, &advSettings.AuthipQMPolicyKey);
      if (result)
      {
         wprintf(L"AddCustomIPsecPolicy returned error %ld\n", result);
         goto cleanup;
      }
      // Cast the (SOCKET_SECURITY_SETTINGS_IPSEC*) to 
      // (SOCKET_SECURITY_SETTINGS*) and set the length appropriately.
      settings = (SOCKET_SECURITY_SETTINGS*)&advSettings;
      settingsLen = sizeof(advSettings);
   }
   else
   {
      //-----------------------------------------
      // Construct basic socket security settings

      // Use default security protocol with default settings.
      basicSettings.SecurityProtocol = SOCKET_SECURITY_PROTOCOL_DEFAULT;
      // Guarantee encryption
      basicSettings.SecurityFlags = SOCKET_SETTINGS_GUARANTEE_ENCRYPTION;
      settings = &basicSettings;
      settingsLen = sizeof(basicSettings);
   }

   //-----------------------------------------
   // Accept secure TCP connections from clients
   result = SecureTcpAcceptConnection(
               addrFamily,
               settings,
               settingsLen
               );
   if (result)
   {
      wprintf(L"SecureTcpAcceptConnection returned error %ld\n", result);
      goto cleanup;
   }

   wprintf(L"Finished\n");

cleanup:
   if(useAdv)
   {
      RemoveCustomIPsecPolicy(fwpHandle, &advSettings.AuthipQMPolicyKey);
   }
   if(wsaCleanup)
   {
      WSACleanup();
   }
   return result;
}

