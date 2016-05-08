//+===================================================================
//
//  This file is part of the Microsoft .NET Framework SDK Code Samples.
// 
//  Copyright (C) Microsoft Corporation.  All rights reserved.
// 
//This source code is intended only as a supplement to Microsoft
//Development Tools and/or on-line documentation.  See these other
//materials for detailed information regarding Microsoft code samples.
// 
//THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
//KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//PARTICULAR PURPOSE.
//
//      GetConnInfo.c
//		Gets information about the active connections on the server. Prints the port on
//      which the connection has been accepted on the server and the IPv4 and IPv6 
//      address that the client connection acquired.
//              
//
//
//+===================================================================

#define MAX_IPV6_STRING	46
#define IPV6_INTERFACE_ID_LENGTH_IN_BYTES 8

//TODO: Need to uncomment this when nt.h is in SDK
/* 
#include <nt.h>    //included for RtlIpv6AddressToString
#include <ntrtl.h> //included for RtlIpv6AddressToString
#include <nturtl.h>//included for RtlIpv6AddressToString*/
#include <Ws2tcpip.h>
#include <windows.h>
#include <mprapi.h>
#include <wchar.h>
//#include <ws2ipdef.h>
//#include <Ws2tcpip.h> //included for IN6_ADDR

VOID
CopyPrefixBytes(
   BYTE *Address, 
    BYTE* Prefix, 
    ULONG PrefixLength,
    ULONG AddressBytes
    )
//
//Routine Description:
//
//    Copy an address prefix of the given length
//
//Arguments:
//    Address - Supplies the address buffer to fill in.
//    Prefix - Supplies the initial prefix.
//    PrefixLength - Supplies the initial prefix length in bits.
//    AddressBytes- Prefix length in bytes (ceiling)
//
{
    ULONG PLBytes, PLRemainderBits, Loop;

    PLBytes = PrefixLength / 8;
    PLRemainderBits = PrefixLength % 8;
    if (AddressBytes != (PLBytes+ (PLRemainderBits?1:0))) return;
    for (Loop = 0; Loop < PLBytes; Loop++) {
           Address[Loop] = Prefix[Loop];
    }
    if (PLRemainderBits ) {
        Address[PLBytes] = (UCHAR)(Prefix[PLBytes] &
                (0xff << (8 - PLRemainderBits)));
    }
}

DWORD ConvertAddressToString(IN6_ADDR *ipv6addr,WCHAR * hoststring)
//
//Routine Description:
//
//    Converts an IPv6 address in IN6_ADDR to a readable host string format
//
//Arguments:
//    ipv6addr - Pointer to IN6_ADDR structure having the IPv6 address.
//    hoststring - The buffer to be returned with the address in host string format.
//
{
  struct sockaddr_in6 sin6;
  int RetVal = 0;
  DWORD dwRet = ERROR_SUCCESS;
  WSADATA WsaData;

  if(WSAStartup(0x202,&WsaData))
  {
	  dwRet = GetLastError();
	  wprintf(L"Error in WSAStartup[Error:%d]\n",dwRet);
	  goto Cleanup;
  }
  
  sin6.sin6_family = AF_INET6;
  sin6.sin6_port = 0;
  sin6.sin6_flowinfo = 0;
  sin6.sin6_scope_id = 0;
  sin6.sin6_addr = *ipv6addr;

  RetVal = GetNameInfoW((SOCKADDR *)&sin6,
	                    sizeof(struct sockaddr_in6),
						hoststring,
						MAX_IPV6_STRING,
						NULL,
						0,
						NI_NUMERICHOST);
  if (RetVal != 0)
  {
	  dwRet = WSAGetLastError();
	  wprintf(L"Could not convert IPv6 address to address string format![Error:%d]",dwRet);
      
  }
Cleanup:
  WSACleanup();
  return dwRet;
}


int _cdecl wmain(int argc,WCHAR * argv[])
{
     
  MPR_SERVER_HANDLE hMprServer = NULL;   // Variable to hold the handle to the Server to be used in subsequent calls to the Router
  HANDLE hConnection = NULL; 				//Handle to the client connection whose connection information we query

  DWORD   dwStatus = ERROR_SUCCESS;    
  DWORD   dwLevel = 0;

  LPBYTE  lplpbBuffer      = NULL;                  // Buffer to be used in MprAdminPortEnum to retrieve the information
  LPBYTE  lplpConnBuffer   = NULL;		     // Buffer to be used in MprAdminConnectionGetInfo to get the information about a connection
  DWORD   dwEntriesRead  = 0;		    
  DWORD   dwTotalEntries = 0;
  DWORD   dwResumeHandle = 0;

  WCHAR * szErrorString = NULL;

  PRAS_PORT_0 pRasPort = NULL;
  int ActivePortCount = 0;

  IN6_ADDR    ipv6Address = {0};   		     // Variable to hold IPv6 address of the client connection
  WCHAR		szIpv6AddrString[MAX_IPV6_STRING] = {0};  //Variable to hold the IPv6 address string of the client connection

  if (argc!=1 && argc!=2)
  	{
  	  wprintf(L"Usage:\n");
	  wprintf(L"%s - For local machine\n",argv[0]);
	  wprintf(L"%s Hostname|IP address - For remote machine\n",argv[0]);
	  wprintf(L"If you specify a remote machine's hostname or IP address\n");
	  wprintf(L"here, you should ensure that you have administrative priveleges\n");
	  wprintf(L"to execute this program on the remote machine. For this you can\n");
	  wprintf(L"do a net use to the IPC$ on the machine before executing this program\n");
	  wprintf(L"For e.g. net use \\\\home-computer\\ipc$ \n");
	  dwStatus = -1;
	  goto Cleanup;
  	}


   // Obtain a handle to the RRAS server using the API MprAdminServerConnect.
   // This handle is used in all subsequest MPRAPI calls to the RRAS server.
   
   if (argc == 1)
      	   dwStatus = MprAdminServerConnect(NULL, &hMprServer);
   else
          dwStatus = MprAdminServerConnect(argv[1], &hMprServer);
   

   if(dwStatus != ERROR_SUCCESS)
   {
	MprAdminGetErrorString(dwStatus,&szErrorString);
	wprintf(L" MprAdminServerConnect  failed with ERROR:[%d:%s]\n", dwStatus,szErrorString);
	goto Cleanup;
   }
   	
   dwLevel = 0;
  

  // We enumerate all the ports on the RRAS server and use the information returned
  // to find the connection information for the active ports
  
   dwStatus = MprAdminPortEnum(hMprServer,
   							dwLevel,
   							INVALID_HANDLE_VALUE,
   							&lplpbBuffer,
                                     	       (DWORD)-1,
                                     		&dwEntriesRead,
                                     		&dwTotalEntries,
                                     		&dwResumeHandle);
        
   if(dwStatus != ERROR_SUCCESS)
   {
       MprAdminGetErrorString(dwStatus,&szErrorString);
	wprintf(L"MprAdminPortEnum failed with ERROR:[%d:%s]\n",dwStatus,szErrorString);
	goto Cleanup;
	   
   }


  if ((dwEntriesRead != 0) && (lplpbBuffer != NULL))
   {
	   PRAS_CONNECTION_3 pRasConnInfo = NULL;
       DWORD i = 0;
	   

	   pRasPort= (PRAS_PORT_0) (lplpbBuffer);
	      
	  
	   // Log the port information of the connection and get additional information for the connection
	   // using the connection handle returned in RAS_PORT_0 structure in the above call
	   
	   for (i=0;i<(dwEntriesRead);i++)
	   {
                 if (pRasPort[i].dwPortCondition == RAS_PORT_AUTHENTICATED) // Try to get the connection information only for the active ports
				 	
			{	  
 			       ActivePortCount++;   
				   hConnection = pRasPort[i].hConnection;
				  

 				   // Get the connection information for the active ports. dwLevel is set to 3 so that
 				   // RAS_CONNECTION_3 structures are returned by the call
 				   
				   dwLevel = 3;

				   dwStatus = MprAdminConnectionGetInfo(hMprServer,dwLevel,hConnection,&lplpConnBuffer);
				   if(dwStatus != ERROR_SUCCESS)
				   {
					   MprAdminGetErrorString(dwStatus,&szErrorString);
					   
					   wprintf(L"MprAdminConnectionGetInfo:FAIL with ERROR:[%d:%s]\n",dwStatus,szErrorString);
					   goto Cleanup;

				   }

				   pRasConnInfo = (PRAS_CONNECTION_3) (lplpConnBuffer);


				   //We print the information about the connection that we got 
				   //from the call to MprAdminConnectionGetInfo
				   
				   wprintf(L"User %s has connected on port %s for this connection\n",pRasConnInfo->wszUserName,pRasPort[i].wszPortName);
				  

				    
				   // For the IPv6 address of the client, we get the interface ID, the prefix and the prefix length 
				   // from the RAS_CONNECTION_3 structure. Here we copy the prefix and the 64-bit interface
				   // ID to a IN6_ADDR structure and use GetNameInfoW with the flag NI_NUMERICHOST to convert it to 
				   // IPv6 address string in the readable form
				   // 
				   // As the prefix length we get from the structure is in bits we need the 'CopyPrefixBytes' 
				   // module to convert the length into bytes and copy the prefix accordingly.
				  
				  
				   CopyPrefixBytes(ipv6Address.u.Byte,
				   				pRasConnInfo->PppInfo3.ipv6.bPrefix,
				         			pRasConnInfo->PppInfo3.ipv6.dwPrefixLength,
				         			sizeof(ipv6Address)-IPV6_INTERFACE_ID_LENGTH_IN_BYTES);

				   // Copy the RemoteInterfaceIdentifier to the last 8 bytes of ipv6Address.u.Byte
		                memcpy(&ipv6Address.u.Byte[IPV6_INTERFACE_ID_LENGTH_IN_BYTES],
						 pRasConnInfo->PppInfo3.ipv6.bRemoteInterfaceIdentifier,
						 IPV6_INTERFACE_ID_LENGTH_IN_BYTES);
				   
		          // Convert the IPv6 address in IN6_ADDR to a readable IPv6 address string      
				  dwStatus = ConvertAddressToString(&ipv6Address,szIpv6AddrString);

				  if (dwStatus == ERROR_SUCCESS)
					 wprintf(L"IPv6 address of the client: %s\n",szIpv6AddrString);

				  wprintf(L"IPv4 address of the client: %s\n",pRasConnInfo->PppInfo3.ip.wszRemoteAddress);

				  MprAdminBufferFree(lplpConnBuffer);
				  lplpConnBuffer = NULL;
	   	}
	   }
   
   }
if (ActivePortCount !=0 )
	wprintf(L"Successfully retrieved the connection information!\n");
else
{
    wprintf(L"No active connections on the server!\n");
    wprintf(L"Make a connection to this server and try running the program again\n");
    dwStatus = -1;
 }

//cleanup
Cleanup:
wprintf(L"Exiting....\n");
if (szErrorString)	
	MprAdminBufferFree(szErrorString);

if (lplpbBuffer)
	MprAdminBufferFree(lplpbBuffer);

if (lplpConnBuffer)
	MprAdminBufferFree(lplpConnBuffer);

return dwStatus;
}
	
 
































