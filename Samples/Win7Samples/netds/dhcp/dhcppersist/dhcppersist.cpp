/*
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
 * ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 * Copyright © 1999 - 2000  Microsoft Corporation.  All Rights Reserved.
 *
 * Author: Stephen R. Husak - Microsoft Developer Support
 *
 * Abstract:
 *    This code demonstrates the use of the DHCP Client Options API -
 *    DhcpRequestParams/DhcpUndoRequestParams for setting options
 *    to be persistent
 *
 */

/*
 * Includes
 */
#include <windows.h>      // windows
#include <stdio.h>        // standard i/o 
#include <iphlpapi.h>     // ip helper 
#include <dhcpcsdk.h>     // dhcp client options api

// initial buffer size for options buffer
#define INITIAL_BUFFER_SIZE 256
#define DHCP_PERSISTENT_APP_STRING L"DhcpRequestSample"

/*
 * OutputError
 *
 * retrieves the system message for an error
 */
void OutputError(DWORD dwError)
{
  LPVOID lpMsgBuf;    // buffer to copy string into (allocated by call)

  if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                FORMAT_MESSAGE_FROM_SYSTEM | 
                FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwError,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                (LPTSTR) &lpMsgBuf, 0, NULL) == 0)
    printf("Error Reported: %d GetLastError: %d\n", dwError, GetLastError());
  else  
    {
    printf("Error %d: %s\n", dwError, (LPCSTR) lpMsgBuf);  
    
    // free the buffer.
    LocalFree(lpMsgBuf);
    }
}

/*
 * DetermineAdapter
 * 
 * NOTE:
 *   
 *    This code retrieves the Adapter Name to use for the DHCP Client API 
 *    using the IPHelper API.
 *   
 *    NT has a name for the adapter that through this API has device 
 *    information in front of it followed by a {GUID}, 98 does not and 
 *    the Index is used instead. So if the string is set to ?? (what it is 
 *    in 98) we revert to using the string representation of the index.
 *     
 */
LPSTR DetermineAdapter()
{
  DWORD dwResult;                            // result of API calls
  IP_INTERFACE_INFO * pInfo = NULL;          // adapter information structure
  DWORD dwSize = 0;                          // size of required buffer
  CHAR szAdapter[MAX_ADAPTER_NAME] = {0};    // the adapter to use 
  char * ptr;                                // pointer to adapter name

  // get buffer size
  dwResult = GetInterfaceInfo(NULL, &dwSize);     
  if (dwResult == ERROR_INSUFFICIENT_BUFFER)
    {
    // allocate buffer
    pInfo = (IP_INTERFACE_INFO *) LocalAlloc(LPTR, dwSize);
    if (!pInfo)
      {
      OutputError(GetLastError());
      exit(1);
      }

    // make the actual call
    dwResult = GetInterfaceInfo(pInfo, &dwSize);
    if (dwResult != ERROR_SUCCESS)
      {
      OutputError(GetLastError());
      exit(2);
      }
    }
  else
    {
    OutputError(GetLastError());
    exit(3);
    }
  
  // convert, parse, and convert back
  ptr = NULL;
  WideCharToMultiByte(0, 0, pInfo->Adapter[0].Name, 
                            lstrlenW(pInfo->Adapter[0].Name), 
                            szAdapter, MAX_ADAPTER_NAME, NULL, NULL);
  if (szAdapter[0] != '?')
    {
    // find the GUID
    ptr = strchr(szAdapter, '{'); 
    }

  // use index if the pointer is not set
  if (!ptr)
    {
    sprintf_s(szAdapter, MAX_ADAPTER_NAME, "%ld\0", pInfo->Adapter[0].Index);            
    ptr = szAdapter;
    }

  // free what was allocated
  if (pInfo)
    LocalFree(pInfo);

  return ptr;
}

/*
 * main
 *
 * this is where it all happens
 */
int main(int argc, char * argv[])
{
  DWORD dwResult;                            // result from API call
  DWORD dwVersion;                           // API version reported
  WCHAR wszAdapter[MAX_ADAPTER_NAME] = {0};  // the adapter name in wide chars
  char * ptr = NULL;                         // pointer to adapter name
  BOOL bRemovePersist = FALSE;               // remove persistent request
  BOOL bAddPersist = FALSE;                  // add persistent request
    
  // check for persist options and adapter ID
  for (int i = 1; i < argc; i++)
    {
    char * p;
    p = argv[i];
    if ((p[0] == '/') || (p[0] == '-'))
      {
      p++;
      if ((p[0] == 'p') || (p[0] == 'P'))
        bAddPersist = TRUE;
      if ((p[0] == 'r') || (p[0] == 'R'))
        bRemovePersist = TRUE;
      }
    else
      ptr = argv[i];
    }

  if (!ptr)
    ptr = DetermineAdapter();

  // convert it for the API call
  MultiByteToWideChar(0, 0, ptr, (int) strlen(ptr), wszAdapter, MAX_ADAPTER_NAME);      

  // initialize the DHCP Client Options API
  dwResult = DhcpCApiInitialize(&dwVersion);
  if (dwResult != 0)
    {
    OutputError(dwResult);
    exit(4);
    }
  else
    printf("DHCP Client Options API version %d\n", dwVersion);

  printf("Setting DHCP Options on Adapter [%S]\n", wszAdapter);

  // remove persistence if required
  if (bRemovePersist)
    {
    printf("Removing Option Persistence...\n");
    dwResult = DhcpUndoRequestParams(0, NULL, wszAdapter, 
                                     DHCP_PERSISTENT_APP_STRING);
    if (dwResult != ERROR_SUCCESS)
      OutputError(dwResult);
    }
  else
    {
    //
    // Here the request is set up - since this is an easy example, the request
    // is set up statically, however in a real-world scenario this may require
    // building the request array in a more 'dynamic' way
    //
    // the DHCP Client Options API arrays for getting the options  
    DHCPCAPI_PARAMS requests[1] = 
                {{0, OPTION_TIME_SERVERS, FALSE, NULL, 0}}; // gateway address

    // set-up the actual arrays
    DHCPCAPI_PARAMS_ARRAY sendarray = {0, NULL}; // we aren't sending anything
    DHCPCAPI_PARAMS_ARRAY requestarray = {1, requests}; // we are requesting 1

    // buffer variables
    DWORD dwSize = INITIAL_BUFFER_SIZE; // size of buffer for options
    LPBYTE buffer = NULL;               // buffer for options  
    IN_ADDR addr;                       // address in return code

    // loop until buffer is big enough to get the data and then make request
    do 
      {    
      if (buffer)
        LocalFree(buffer);

      buffer = (LPBYTE) LocalAlloc(LPTR, dwSize);       // allocate the buffer
      if (!buffer)
        {
        OutputError(GetLastError());
        exit(5);
        }

      // make the request on the adapter
      DWORD dwFlags = DHCPCAPI_REQUEST_SYNCHRONOUS;
      if (bAddPersist)
        {
        dwFlags = DHCPCAPI_REQUEST_PERSISTENT;
        printf("Making the request persistent...\n");
        }

      dwResult = DhcpRequestParams(dwFlags, 
                                   NULL, 
                                   wszAdapter,
                                   NULL, sendarray, 
                                   requestarray, 
                                   buffer, &dwSize, 
                                   bAddPersist ? 
                                        DHCP_PERSISTENT_APP_STRING : NULL);
      }
    while (dwResult == ERROR_MORE_DATA);
  
    // parse out results only if we made the synchronous request
    if (dwResult == ERROR_SUCCESS)
      {
      if (bAddPersist != TRUE)        
        {    
        // check for time server
        if (requests[0].nBytesData > 0) 
          {      
          memcpy(&addr, (LPVOID) requests[0].Data, 4);
          printf("Time Server: %s\n", inet_ntoa(addr));
          }
        else
          printf("Time Server NOT present!\n");
        }
      }
    else
      {    
      OutputError(dwResult);        
      }

    // free the buffer
    if (buffer)
      LocalFree(buffer);
    }

  // de-init the api
  DhcpCApiCleanup();

  return 0;
}
