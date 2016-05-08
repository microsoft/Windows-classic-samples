//---------------------------------------------------------------------
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
//  RasNapAdmin.c
//     Implements a RAS administration DLL which implements the callback functions
//  to decide if the reauthentication attempt can be allowed or not. It also logs the
//  user and port information whenever a new connection/link is accepted. This 
//  creates a file with the name log.txt under %windir%\system32.
// 
//  The RAS administration DLL registry setup need to be performed as mentioned 
//  in MSDN before running this application.
//  
//---------------------------------------------------------------------

#ifndef UNICODE
#define UNICODE
#endif


// INCLUDES
#include "RasNapAdmin.h" 

 // The log file to which we will log the information 
 // when each callback function is called

 
FILE			*g_fpFile	= NULL; 

//
// Function which decides id reauthentication can take place
//
// Parameters: Username for this connection
//

BOOL IsReAuthAllowedForUser(WCHAR *User)
{
	return TRUE;
}

//
//    Converts an IPv6 address in IN6_ADDR to a readable host string format
//
//Arguments:
//    ipv6addr - Pointer to IN6_ADDR structure having the IPv6 address.
//    hoststring - The buffer to be returned with the address in host string format.
//
DWORD ConvertAddressToString(IN6_ADDR *ipv6addr,WCHAR * hoststring)
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

//    Copy an address prefix of the given length
//
//Parameters:
//    Address - Supplies the address buffer to fill in.
//    Prefix - Supplies the initial prefix.
//    PrefixLength - Supplies the initial prefix length in bits.
//    AddressBytes- Prefix length in bytes (ceiling)
//
VOID
CopyPrefixBytes(
   BYTE *Address, 
    BYTE* Prefix, 
    ULONG PrefixLength,
    ULONG AddressBytes
    )
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


//
// Function is called whenever PPP reauthentication happens due to change in 
// quarantine state of the client
//
// Parameters: pointer to a RAS_CONNECTION_0 structure
//			  pointer to a RAS_CONNECTION_1 structure
//			  pointer to a RAS_CONNECTION_2 structure
//			  pointer to a RAS_CONNECTION_3 structure
//
BOOL WINAPI MprAdminAcceptReauthentication(RAS_CONNECTION_0 *pRasCon0, 
	RAS_CONNECTION_1 *pRasCon1,
	RAS_CONNECTION_2 *pRasCon2,
	RAS_CONNECTION_3 *pRasCon3)
{
	BOOL fReAuthAllowed = TRUE;
	fwprintf(g_fpFile,L"============MprAdminAcceptReauthentication callback============\n");
	fwprintf(g_fpFile,L"User = %s: quarstate=%d: probationtimer=%d \n",pRasCon3->wszUserName,pRasCon3->rasQuarState,pRasCon3->timer.dwLowDateTime);

	//
	// Here we have a function based on the username whether the reauthentication 
	// attempt is allowed or not. This function can be implemented according to
	// the administrator's requirements. In this sample, the function just returns
	// true always.
	// 
	fReAuthAllowed = IsReAuthAllowedForUser(pRasCon3->wszUserName);
	
	if(fReAuthAllowed)
	{
		fwprintf(g_fpFile,L"Reauth allowed\n");
	}
	else
	{
		fwprintf(g_fpFile,L"Reauth not allowed\n");
	}

	fwprintf(g_fpFile,L"Exiting MprAdminReauthentication");
	fflush(g_fpFile);
	return fReAuthAllowed == 0? FALSE : TRUE;
	
}

//
// Function is called whenever a link is disconnected. 
//
// Parameters: pointer to a RAS_PORT_0 structure
//			   pointer to a RAS_PORT_1 structure
//
VOID WINAPI 
MprAdminLinkHangupNotification(RAS_PORT_0 * pRasPort0,
                               RAS_PORT_1 * pRasPort1)
{
	fwprintf(g_fpFile,L"============MprAdminLinkHangupNotification callback============\n");
	fflush(g_fpFile);
	return;
}         

//
// Function is called whenever a call is disconnected. 
//
// Parameters: pointer to a RAS_CONNECTION_0 structure
//			   pointer to a RAS_CONNECTION_1 structure
//			   pointer to a RAS_CONNECTION_2 structure
//
VOID WINAPI 
MprAdminConnectionHangupNotification3(RAS_CONNECTION_0 * pRasConnection0,
                                      RAS_CONNECTION_1 * pRasConnection1,
                                      RAS_CONNECTION_2 * pRasConnection2,
                                      RAS_CONNECTION_3* pRasConnection3)
{
    SYSTEMTIME systime;
	IN6_ADDR    ipv6Address = {0};   		     // Variable to hold IPv6 address of the client connection
    WCHAR		szIpv6AddrString[MAX_IPV6_STRING] = {0};  //Variable to hold the IPv6 address string of the client connection
	DWORD dwStatus = ERROR_SUCCESS;

	fwprintf(g_fpFile,L"============MprAdminConnectionHangupNotification3 callback============\n");
	
	// We log the time the user has disconnected alongwith the username, IPv4 address
	// and the IPv6 address. This information can be used for billing
	// purposes.

	GetSystemTime(&systime);
	
	fwprintf(g_fpFile,L"Time(HR:MIN:SEC): %d:%d:%d \n",systime.wHour,systime.wMinute,systime.wSecond);
	fwprintf(g_fpFile,L"Username: %s\n",pRasConnection3->wszUserName);
	fwprintf(g_fpFile,L"IPv4 address of client: %s\n",pRasConnection3->PppInfo3.ip.wszRemoteAddress);
	
	   // For the IPv6 address of the client, we get the interface ID, the prefix and the prefix length 
	   // from the RAS_CONNECTION_3 structure. Here we copy the prefix and the 64-bit interface
	   // ID to a IN6_ADDR structure and use GetNameInfoW with the flag NI_NUMERICHOST to convert it to 
	   // IPv6 address string in the readable form
	   // As the prefix length we get from the structure is in bits we need the 'CopyPrefixBytes' 
	   // module to convert the length into bytes and copy the prefix accordingly.
	  
	   
	    CopyPrefixBytes(ipv6Address.u.Byte,
	   				pRasConnection3->PppInfo3.ipv6.bPrefix,
	         			pRasConnection3->PppInfo3.ipv6.dwPrefixLength,
	         			sizeof(ipv6Address)-IPV6_INTERFACE_ID_LENGTH_IN_BYTES);

	   // Copy the RemoteInterfaceIdentifier to the last 8 bytes of ipv6Address.u.Byte
            memcpy(&ipv6Address.u.Byte[IPV6_INTERFACE_ID_LENGTH_IN_BYTES],
			 pRasConnection3->PppInfo3.ipv6.bRemoteInterfaceIdentifier,
			 IPV6_INTERFACE_ID_LENGTH_IN_BYTES);
	   
        // Convert the IPv6 address in IN6_ADDR format to readable host string format
		dwStatus = ConvertAddressToString(&ipv6Address,szIpv6AddrString);

	    if (dwStatus == ERROR_SUCCESS)
			fwprintf(g_fpFile,L"IPv6 address of the client: %s\n",szIpv6AddrString);
	
	fflush(g_fpFile);
	return;
}         



//
// Function is called whenever a call is connected.
//
// Parameters: pointer to a RAS_CONNECTION_0 structure
//			   pointer to a RAS_CONNECTION_1 structure
//			   pointer to a RAS_CONNECTION_2 structure
//
BOOL WINAPI 
MprAdminAcceptNewConnection3(RAS_CONNECTION_0 * pRasConnection0,
                             RAS_CONNECTION_1 * pRasConnection1,
                             RAS_CONNECTION_2 * pRasConnection2,
                             RAS_CONNECTION_3* pRasConnection3)
{
    SYSTEMTIME systime;
	DWORD dwStatus = ERROR_SUCCESS;
	IN6_ADDR    ipv6Address = {0};   		     // Variable to hold IPv6 address of the client connection
    WCHAR		szIpv6AddrString[MAX_IPV6_STRING] = {0};  //Variable to hold the IPv6 address string of the client connection
	fwprintf(g_fpFile,L"============MprAdminAcceptNewConnection3 callback============\n");
	
	// We log the username, the IPv4 and IPv6 address of the client
	// which can be used to see how much time the user was connected
	

	GetSystemTime(&systime);
	
	fwprintf(g_fpFile,L"Time(HR:MIN:SEC): %d:%d:%d \n",systime.wHour,systime.wMinute,systime.wSecond);
	fwprintf(g_fpFile,L"Username: %s\n",pRasConnection3->wszUserName);
	fwprintf(g_fpFile,L"IPv4 address of client: %s\n",pRasConnection3->PppInfo3.ip.wszRemoteAddress);

	   // For the IPv6 address of the client, we get the interface ID, the prefix and the prefix length 
	   // from the RAS_CONNECTION_3 structure. Here we copy the prefix and the 64-bit interface
	   // ID to a IN6_ADDR structure and use GetNameInfoW with the flag NI_NUMERICHOST to convert it to 
	   // IPv6 address string in the readable form
	   // As the prefix length we get from the structure is in bits we need the 'CopyPrefixBytes' 
	   // module to convert the length into bytes and copy the prefix accordingly.
	  
	   
	    CopyPrefixBytes(ipv6Address.u.Byte,
	   				pRasConnection3->PppInfo3.ipv6.bPrefix,
	         			pRasConnection3->PppInfo3.ipv6.dwPrefixLength,
	         			sizeof(ipv6Address)-IPV6_INTERFACE_ID_LENGTH_IN_BYTES);

	   // Copy the RemoteInterfaceIdentifier to the last 8 bytes of ipv6Address.u.Byte
            memcpy(&ipv6Address.u.Byte[IPV6_INTERFACE_ID_LENGTH_IN_BYTES],
			 pRasConnection3->PppInfo3.ipv6.bRemoteInterfaceIdentifier,
			 IPV6_INTERFACE_ID_LENGTH_IN_BYTES);
	   
        // Convert the IPv6 address in IN6_ADDR format to readable host string format
		dwStatus = ConvertAddressToString(&ipv6Address,szIpv6AddrString);

	    if (dwStatus == ERROR_SUCCESS)
			fwprintf(g_fpFile,L"IPv6 address of the client: %s\n",szIpv6AddrString);

	fflush(g_fpFile);
    return TRUE;
}                    

//
// Function is called whenever a new link is made.
//
// Parameters: pointer to a RAS_PORT_0 structure
//			   pointer to a RAS_PORT_1 structure
//
BOOL WINAPI 
MprAdminAcceptNewLink(RAS_PORT_0 * pRasPort0,
                      RAS_PORT_1 * pRasPort1)
{
	    fwprintf(g_fpFile,L"============MprAdminAcceptNewLink callback============\n");

	    // We log the port to which the user connected
	    fwprintf(g_fpFile,L"Link accepted on port : %s\n",pRasPort0->wszPortName);
		fflush(g_fpFile);	    
        return TRUE;
    }


       
//
// Function used for DLL specific initialization. 
//
// Returns: NO_ERROR if successful
//		    The last error number if an error is encountered
//
DWORD WINAPI 
MprAdminInitializeDll(void)
{
    DWORD dwRetVal = ERROR_SUCCESS;
    dwRetVal = InitializeDll();
    if (  ERROR_SUCCESS != dwRetVal ) 
        fwprintf(g_fpFile, L"Initialization function failed with error %d\n", dwRetVal);
    else   
        fwprintf(g_fpFile,L"RAS Admin DLL  initialized.\n");

    fflush(g_fpFile);
    return dwRetVal;
}

//
// Function used to form DLL specific cleanup. 
//
DWORD WINAPI 
MprAdminTerminateDll(void)
{
   	
	fwprintf(g_fpFile,L"Terminating RAS Admin DLL ...\n");
	CleanUp();
    return NO_ERROR;
}

//
// Function initializes everything needed 
//
// Returns: TRUE if all is well
//			FALSE otherwise
DWORD
InitializeDll()
{
	HKEY	hKey = NULL;
	WCHAR	DLLName[MAX_PATH];
	DWORD dwBufferLength = MAX_PATH;
	DWORD dwRetval = ERROR_SUCCESS;
	

	//
	// Open the log file. 
	//
	if ((dwRetval = _wfopen_s(&g_fpFile,LOG_FILE,L"w")) != 0)
	{
		goto CleanUp;
	}

    
	dwRetval = RegOpenKeyEx(HKEY_LOCAL_MACHINE,ADMINDLL_KEY,0,KEY_QUERY_VALUE,&hKey);
	if ( ERROR_SUCCESS != dwRetval )
	{
	    fwprintf(g_fpFile,L"Could not open registry key required for Admin DLL!\n");
		goto CleanUp;
	}

	dwRetval = RegQueryValueEx(hKey,DISPLAYNAME_VALUE,NULL,NULL,(LPBYTE)DLLName,&dwBufferLength);
	if ( ERROR_SUCCESS != dwRetval)
    {
        fwprintf(g_fpFile,L"Display Name of Admin DLL not found in registry!\n");
		goto CleanUp;
    }

 CleanUp:
	if (hKey)
		RegCloseKey(hKey);

    if (dwRetval== ERROR_SUCCESS)
	{
		fwprintf(g_fpFile,L"Display Name of Admin DLL: %s\n",DLLName);
		fflush(g_fpFile);
	}
	else
	{
		fflush(g_fpFile);
		CleanUp();
	}       
	
	return dwRetval;
}                                                     

//
// Function cleans up all threads, handles, memory, etc.
//
void
CleanUp()
{
	
	//
	// Close the log file
	//
	_fcloseall();
}

