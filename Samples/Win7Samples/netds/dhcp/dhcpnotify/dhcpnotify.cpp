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
 *    DhcpRegisterParamChange/DhcpDeRegisterParamChange
 */

/*
 * Includes
 */
#define UNICODE
#include <windows.h>      // windows
#include <stdio.h>        // standard i/o 
#include <iphlpapi.h>     // ip helper 
#include <dhcpcsdk.h>     // dhcp client options api

// Exit Event Handler
HANDLE g_hExitEvent;

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
 * CtrlHandler
 *
 * handle control events to provide graceful cleanup
 */
BOOL WINAPI CtrlHandler(DWORD dwCtrlType)
{
  printf("\n\nStop Event Received... Aborting... \n\n");

  switch (dwCtrlType)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_SHUTDOWN_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_CLOSE_EVENT:      
      SetEvent(g_hExitEvent);
      break;
    default:
      return FALSE;
    }

  return TRUE;
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
  char * ptr;                                // pointer to adapter name

  g_hExitEvent = CreateEvent(NULL, FALSE, FALSE, L"DHCP_EXIT_EVENT");
  if (g_hExitEvent == NULL)
    OutputError(GetLastError());

  SetConsoleCtrlHandler(CtrlHandler, true);

  if (argc > 1)    
    // use what's on the command line for the adapter ID 
    ptr = argv[1];            
  else  
    // use IP Helper API to get the adapter ID to use
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

  //
  // Here the watch is set up - since this is an easy example, the request
  // is set up statically, however in a real-world scenario this may require
  // building the watch array in a more 'dynamic' way
  //
  // Also for this sample we are using one item that is almost always in a 
  // DHCP configuration. 
  //
  // the DHCP Client Options API array for watching the options
  //
  DHCPCAPI_PARAMS watch[1] = 
              {{0, OPTION_ROUTER_ADDRESS, FALSE, NULL, 0}}; // gateway address

  // set-up the actual array  
  DHCPCAPI_PARAMS_ARRAY watcharray = {1, watch}; // we are watching 1 item

  printf("Watching DHCP Options Change on Adapter [%S]\n", wszAdapter);

  HANDLE hEvent;  // handle that will be set when parameter(s) change

  // make the request on the adapter
  dwResult = DhcpRegisterParamChange(DHCPCAPI_REGISTER_HANDLE_EVENT, 
                                     NULL, 
                                     wszAdapter,
                                     NULL, 
                                     watcharray, 
                                     &hEvent);
  
  // wait for the events to become signaled
  if (dwResult == ERROR_SUCCESS)
    {
    HANDLE lpHandles[2] = {g_hExitEvent, hEvent};
    DWORD rc = 0;

    while (rc = WaitForMultipleObjects(2, lpHandles, FALSE, INFINITE))
      {
      if (rc == WAIT_OBJECT_0)
        break;

      ResetEvent(hEvent);
      printf("Parameter has changed.\n");
      
      // the change could then be read and applied as needed
      // see the sample DHCPREQUEST or the DhcpRequestParams() API 
      // for more information
      }
    }
  else
    {    
    OutputError(dwResult);        
    }
     
  dwResult = DhcpDeRegisterParamChange(DHCPCAPI_REGISTER_HANDLE_EVENT, 
                                       NULL, &hEvent);
  if (dwResult != ERROR_SUCCESS)
    OutputError(dwResult);

  // de-init the api
  DhcpCApiCleanup();

  // close out the exit event handle
  CloseHandle(g_hExitEvent);

  return 0;
}
