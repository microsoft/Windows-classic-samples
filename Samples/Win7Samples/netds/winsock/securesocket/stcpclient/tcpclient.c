// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*++

Module Name:

    Tcpclient.c

Abstract:

    This file contains sample code for a secure TCP Winsock client that
    establishes a secure connection to a TCP server using the Secure Socket API.

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

DWORD
SecureTcpConnect(
   IN const struct sockaddr* serverAddr,
   IN ULONG serverAddrLen,
   IN const wchar_t* serverSPN,
   IN const SOCKET_SECURITY_SETTINGS* securitySettings,
   IN ULONG settingsLen
)
/*++

Routine Description:

    This routine creates a TCP client socket, securely connects to the 
    specified server, sends & receives data from the server, and then closes 
    the socket

Arguments:

    serverAddr - a pointer to the sockaddr structure for the server.

    serverAddrLen - length of serverAddr in bytes

    serverSPN - a NULL terminated string representing the SPN 
               (service principal name) of the server host computer

    securitySettings - pointer to the socket security settings that should be
                       applied to the connection

    serverAddrLen - length of securitySettings in bytes

Return Value:

    Winsock error code indicating the status of the operation, or NO_ERROR if 
    the operation succeeded.

--*/
{
   DWORD result = 0;
   int sockErr = 0;
   SOCKET sock = INVALID_SOCKET;
   WSABUF wsaBuf = {0};
   char* dataBuf = "12345678";
   DWORD bytesSent = 0;
   char recvBuf[RECV_DATA_BUF_SIZE] = {0};
   DWORD bytesRecvd = 0;
   DWORD flags = MSG_WAITALL;
   SOCKET_PEER_TARGET_NAME* peerTargetName = NULL;
   DWORD serverSpnStringLen = (DWORD) wcslen(serverSPN);
   DWORD peerTargetNameLen = sizeof(SOCKET_PEER_TARGET_NAME) + 
                              (serverSpnStringLen * sizeof(wchar_t));

   //-----------------------------------------
   // Create a TCP socket
   sock = WSASocket(
            serverAddr->sa_family,
            SOCK_STREAM,
            IPPROTO_TCP,
            NULL,
            0,
            0
          );
   if (sock == INVALID_SOCKET)
   {
      result = WSAGetLastError();
      wprintf(L"WSASocket returned error %ld\n", result); \
      goto cleanup;
   }
   
   //-----------------------------------------
   // Turn on security for the socket.
   sockErr = WSASetSocketSecurity (
               sock,
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
   // Specify the server SPN
   peerTargetName = HeapAlloc(
                     GetProcessHeap(),
                     HEAP_ZERO_MEMORY,
                     peerTargetNameLen
                     );
   if(!peerTargetName)
   {
      result = ERROR_NOT_ENOUGH_MEMORY;
      wprintf(L"Out of memory\n");
      goto cleanup;
   }
   // Use the security protocol as specified by the settings
   peerTargetName->SecurityProtocol = securitySettings->SecurityProtocol;
   // Specify the server SPN 
   peerTargetName->PeerTargetNameStringLen = serverSpnStringLen;
   RtlCopyMemory(
      (BYTE*)peerTargetName->AllStrings,
      (BYTE*)serverSPN,
      serverSpnStringLen * sizeof(wchar_t)
      );

   sockErr = WSASetSocketPeerTargetName(
               sock,
               peerTargetName,
               peerTargetNameLen,
               NULL,
               NULL
            );
   if (sockErr == SOCKET_ERROR)
   {
      result = WSAGetLastError();
      wprintf(L"WSASetSocketPeerTargetName returned error %ld\n", result);
      goto cleanup;
   }

   //-----------------------------------------
   // Connect to the server
   sockErr = WSAConnect(
              sock,
              serverAddr,
              serverAddrLen,
              NULL,
              NULL,
              NULL,
              NULL
            );
   if (sockErr == SOCKET_ERROR)
   {
      result = WSAGetLastError();
      wprintf(L"WSAConnect returned error %ld\n", result);
      goto cleanup;
   }

   // At this point a secure connection must have been established.
   wprintf(L"Secure connection established to the server\n");

   //-----------------------------------------
   // Match and print IPSec SA information for the connection 
   // (Note: this is optional)
   result = MatchIPsecSAsForConnectedSocket(sock);
   if (result)
   {
      wprintf(L"MatchIPsecSAsForConnectedSocket returned error %ld\n", result);
      goto cleanup;
   }

   //-----------------------------------------
   // Send some data securely
   wsaBuf.len = (ULONG) strlen(dataBuf);
   wsaBuf.buf = dataBuf;
   sockErr = WSASend(
              sock,
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
   wprintf(L"Sent %d bytes of data to the server\n", bytesSent);

   //-----------------------------------------
   // Receive server's response securely
   wsaBuf.len = RECV_DATA_BUF_SIZE;
   wsaBuf.buf = recvBuf;
   sockErr = WSARecv(
              sock,
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
   wprintf(L"Received %d bytes of data from the server\n", bytesRecvd);

cleanup:
   if(sock != INVALID_SOCKET)
   {
      //This will trigger the cleanup of all IPsec filters and policies that
      //were added for this socket. The cleanup will happen only after all
      //outstanding data has been sent out on the wire.
      closesocket(sock);
   }
   if(peerTargetName)
   {
      HeapFree(GetProcessHeap(), 0, peerTargetName);
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
   wprintf(L"Usage: %s [-adv] [-v6] serverDnsName\n", progName);
   wprintf(L"-adv: Enable advanced mode where a customized policy is specified by the application\n");
   wprintf(L"-v6: Use IPv6. Default is to use IPv4\n");
   wprintf(L"serverDnsName: Fully qualified DNS name of the server\n");
}

int __cdecl wmain(int argc, const wchar_t* const argv[])
{	
   DWORD result = 0;
   WSADATA data;
   WORD version = MAKEWORD(2, 2);
   BOOL wsaCleanup = FALSE;
   SOCKET_SECURITY_SETTINGS basicSettings = {0};
   SOCKET_SECURITY_SETTINGS_IPSEC advSettings = {0};
   SOCKET_SECURITY_SETTINGS* settings = NULL;
   ULONG settingsLen = 0;
   UINT32 i = 0;
   BOOL useAdv = FALSE;
   int addrFamily = AF_INET;
   wchar_t* serverHostDnsName = NULL;
   wchar_t serverSPN[MAX_PATH] = {0};
   DWORD serverSPNLen = MAX_PATH;
   struct sockaddr* serverAddr = NULL;
   ULONG serverAddrLen = 0;
   HANDLE fwpHandle = NULL;
   struct addrinfoW aiHints = {0};
   struct addrinfoW* aiList = NULL;
   wchar_t* serverPort = L"27015";

   //-----------------------------------------
   // Parse the command line arguments
   if((argc < 2) || (argc > 4))
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
      else if(_wcsicmp(argv[i], L"/?") == 0)
      {
         ShowUsage(argv[0]);
         goto cleanup;
      }
      else
      {
         //server name.
         if(!serverHostDnsName)
         {
            serverHostDnsName = (wchar_t*)argv[i];
         }
      }
   }
   if(!serverHostDnsName)
   {
      // Incorrect usage
      ShowUsage(argv[0]);
      goto cleanup;
   }
   if(wcslen(serverHostDnsName) > MAX_PATH)
   {
      wprintf(L"DNS name is too large\n");
      goto cleanup;
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

   //-----------------------------------------
   // Get the server IP address from the DNS name.
   aiHints.ai_family = addrFamily;
   aiHints.ai_socktype = SOCK_STREAM;
   aiHints.ai_protocol = IPPROTO_TCP;
   result = GetAddrInfoW(
              serverHostDnsName,
              serverPort,
              &aiHints,
              &aiList
            );
   if (result)
   {
      wprintf(L"GetAddrInfoW returned error %ld\n", result);
      goto cleanup;
   }
   serverAddr = aiList->ai_addr;
   serverAddrLen = (ULONG) aiList->ai_addrlen;

   //-----------------------------------------
   // Get the server SPN from the DNS name.
   result = DsMakeSpn(
               L"host",
               serverHostDnsName,
               NULL,
               0,
               NULL,
               &serverSPNLen,
               serverSPN
            );
   if(result)
   {
      wprintf(L"DsMakeSpn returned error %ld\n", result);
      goto cleanup;
   }

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
   // Establish secure TCP connection to the server
   result = SecureTcpConnect(
               serverAddr,
               serverAddrLen,
               serverSPN,
               settings,
               settingsLen
            );
   if (result)
   {
      wprintf(L"SecureTcpConnect returned error %ld\n", result);
      goto cleanup;
   }

   wprintf(L"Finished\n");

cleanup:
   if(useAdv)
   {
      RemoveCustomIPsecPolicy(fwpHandle, &advSettings.AuthipQMPolicyKey);
   }
   if(aiList)
   {
      FreeAddrInfoW(aiList);
   }
   if(wsaCleanup)
   {
      WSACleanup();
   }
   return result;
}

