//
//       This is a part of the Microsoft Source Code Samples. 
//       Copyright 1993 - 2000 Microsoft Corporation.
//       All rights reserved. 
//       This source code is only intended as a supplement to 
//       Microsoft Development Tools and/or Help documentation.
//       See these sources for detailed information regarding the 
//       Microsoft samples programs.
//

//
//		RasDialAsync.c
//		
//		Usage:
//		RasDialAsync
//
//		RAS API's used:
//				RasDial
//				RasHangUp
//
//		Callback function:
//				RasDialFunc
//
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x500
#endif

#ifdef UNICODE
#undef UNICODE
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <stdio.h>
#include <windows.h>
#include <ras.h>
#include <raserror.h>
#include <string.h>
#include <winbase.h>
#include <time.h>
#include <stdlib.h>
#include <tchar.h>
#include <strsafe.h>

// Macro for counting maximum characters that will fit into a buffer
#define CELEMS(x) ((sizeof(x))/(sizeof(x[0])))

void WINAPI RasDialFunc(UINT unMsg,
                        RASCONNSTATE rasconnstate,
                        DWORD dwError );

HANDLE gEvent_handle; // Global event handle

// Usage
void Usage(char *pszProgName) 
{
    TCHAR szTempBuf[256] = {0};

    if (pszProgName)
    {
        StringCchPrintf(szTempBuf, 
                  CELEMS(szTempBuf), 
                  "Usage\n%s \t-e [entry name] -p [phone number] \n\t\t-u [username] -z [password] -d [domain]\n",
                  pszProgName);
        printf(szTempBuf);
    }
    return;
}


// Begin main()... 

int __cdecl main(int argc, char*argv[])
{
    LPRASDIALPARAMS lpRasDialParams = NULL;   // Structure to store the RasDial parameters
    HRASCONN        hRasConn = NULL;          // Handle to RAS connection
    DWORD           nRet = 0;                 // Return value from a function
    DWORD           dwMaxTickCount = 0;               // Final Tick Count value
    int             i = 0;
    int             j = 0;
    BOOL            fRequired = FALSE;    
    TCHAR           szTempBuf[256] = {0};

    // Create the event, which indicates completion of RasDial()
    gEvent_handle = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!gEvent_handle)
    {
        nRet = GetLastError();
        printf("CreateEvent failed: Error = %d", nRet);
        return nRet;
    }

    // Initialize the RASDIALPARAMS structure
    lpRasDialParams = (LPRASDIALPARAMS) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(RASDIALPARAMS));
    if (NULL == lpRasDialParams)
    {
	    printf("HeapAlloc failed\n");
	    goto done;
    }

    lpRasDialParams->dwSize =sizeof(RASDIALPARAMS);

    // Copy command line arguments into the RASDIALPARAMS structure
    if (argc > 1) 
    {
	    for(i = 1; i < (argc - 1); i++) 
	    {
		    if (argv[i] && ((argv[i][0] == '-') || (argv[i][0] == '/')))
		    {
			    switch(tolower(argv[i][1])) 
			    {
			        case 'e': // Entry name
                        j = ++i; // Increment i and assign it to j
                        if (argv[j])
                        {
                            StringCchCopy(lpRasDialParams->szEntryName, CELEMS(lpRasDialParams->szEntryName), argv[j]);
                            fRequired = TRUE;
                        }
					    break;
				    case 'p': // Phone number
                        j = ++i; // Increment i and assign it to j
                        if (argv[j])
                        {
                            StringCchCopy(lpRasDialParams->szPhoneNumber, CELEMS(lpRasDialParams->szPhoneNumber), argv[j]);
                        }
                        break;
                    case 'u': // User name
                        j = ++i; // Increment i and assign it to j
                        if (argv[j])
                        {
                            StringCchCopy(lpRasDialParams->szUserName, CELEMS(lpRasDialParams->szUserName), argv[j]);
                        }
                        break;
                    case 'z': // Password
                        j = ++i; // Increment i and assign it to j
                        if (argv[j])
                        {
                            StringCchCopy(lpRasDialParams->szPassword, CELEMS(lpRasDialParams->szPassword), argv[j]);
                        }
                        break;
                    case 'd': // Domain name
                        j = ++i; // Increment i and assign it to j
                        if (argv[j])
                        {
                            StringCchCopy(lpRasDialParams->szDomain, CELEMS(lpRasDialParams->szDomain), argv[j]);
                        }
                        break;
                    default:
                        Usage(argv[0]);
                        goto done;
                        break;
			    }
		    }
            else
            {
                Usage(argv[0]);
                goto done;
            }
	    }
    }
    else
    {
        Usage(argv[0]);
        goto done;
    }

    // Check if we got at least required entry name
    if (FALSE == fRequired)
    {
        Usage(argv[0]);
        goto done;
    }

    // Dial out asynchronously using RasDial()
    ZeroMemory((LPVOID)szTempBuf, sizeof(szTempBuf));  
    StringCchPrintf(szTempBuf, CELEMS(szTempBuf) - 1, "Dialing... %s\n", lpRasDialParams->szPhoneNumber);
    printf(szTempBuf);
    hRasConn = NULL;

    nRet = RasDial(NULL, NULL, lpRasDialParams, 0, &RasDialFunc, &hRasConn);
    // Check whether RasDial() succeeded
    if (nRet)
    {
        printf("RasDial failed: Error = %d\n", nRet);
        goto done;
    }

    // Wait for the RasDial() to complete, signaled by the SetEvent()
    nRet = WaitForSingleObject(gEvent_handle, 50000);
    switch (nRet)
    {
        case WAIT_OBJECT_0:
        // Normal completion or Ras Error encountered
            printf("Will hang up in 5 seconds...\n");
            Sleep(5000);

            break;
    	
        case WAIT_TIMEOUT: // RasDial timed out
            printf("RasDial Timed out...\n");
            break;
            
        default:
            printf("WaitForSingleObject returned %d\n", nRet);
            break;
    }

    printf("Calling RasHangUp...\n");
    nRet = RasHangUp(hRasConn);
    if (ERROR_SUCCESS == nRet)
    {
        ZeroMemory((LPVOID)szTempBuf, sizeof(szTempBuf));  
        StringCchPrintf(szTempBuf, CELEMS(szTempBuf), "Connection to %s terminated.\n", lpRasDialParams->szPhoneNumber);
        printf(szTempBuf);
    }
    else
    {
        printf("RasHangUp failed with error = %d.\n", nRet);
    }
    

done:
   CloseHandle(gEvent_handle);

    if (lpRasDialParams)
    {
        // Clear password from memory
        ZeroMemory(lpRasDialParams->szPassword, sizeof(lpRasDialParams->szPassword));
        HeapFree(GetProcessHeap(), 0, (LPVOID)lpRasDialParams);
    }

    return (int)nRet;
}


// Callback function RasDialFunc()
void WINAPI RasDialFunc(UINT unMsg, 
                        RASCONNSTATE rasconnstate, 
                        DWORD dwError )
{
    char szRasString[256] = {0}; // Buffer for storing the error string
    TCHAR szTempBuf[256] = {0};  // Buffer used for printing out the text

    if (dwError)  // Error occurred
    {
        RasGetErrorString((UINT)dwError, szRasString, CELEMS(szRasString));
        
        ZeroMemory((LPVOID)szTempBuf, sizeof(szTempBuf));  
        StringCchPrintf(szTempBuf, CELEMS(szTempBuf), "Error: %d - %s\n", dwError, szRasString);
        printf(szTempBuf);

        SetEvent(gEvent_handle);
        return;
    }

   // Map each of the states of RasDial() and display on the screen
   // the next state that RasDial() is entering
   switch (rasconnstate)
   {
      case RASCS_OpenPort:
            printf ("RASCS_OpenPort = %d\n", rasconnstate);
            printf ("Opening port...\n");
			break;
        case RASCS_PortOpened:
            printf ("RASCS_PortOpened = %d\n", rasconnstate);
            printf ("Port opened.\n");
        	break;
        case RASCS_ConnectDevice: 
            printf ("RASCS_ConnectDevice = %d\n", rasconnstate);
            printf ("Connecting device...\n");
           break;
        case RASCS_DeviceConnected: 
            printf ("RASCS_DeviceConnected = %d\n", rasconnstate);
            printf ("Device connected.\n");
            break;
        case RASCS_AllDevicesConnected:
            printf ("RASCS_AllDevicesConnected = %d\n", rasconnstate);
            printf ("All devices connected.\n");
            break;
        case RASCS_Authenticate: 
            printf ("RASCS_Authenticate = %d\n", rasconnstate);
            printf ("Authenticating...\n");
            break;
        case RASCS_AuthNotify:
            printf ("RASCS_AuthNotify = %d\n", rasconnstate);
            printf ("Authentication notify.\n");
            break;
        case RASCS_AuthRetry: 
            printf ("RASCS_AuthRetry = %d\n", rasconnstate);
            printf ("Retrying authentication...\n");
            break;
        case RASCS_AuthCallback:
            printf ("RASCS_AuthCallback = %d\n", rasconnstate);
            printf ("Authentication callback...\n");
            break;
        case RASCS_AuthChangePassword: 
            printf ("RASCS_AuthChangePassword = %d\n", rasconnstate);
            printf ("Change password...\n");
            break;
        case RASCS_AuthProject: 
            printf ("RASCS_AuthProject = %d\n", rasconnstate);
            printf ("Projection phase started...\n");
            break;
        case RASCS_AuthLinkSpeed: 
            printf ("RASCS_AuthLinkSpeed = %d\n", rasconnstate);
            printf ("Negoting speed...\n");
            break;
        case RASCS_AuthAck: 
            printf ("RASCS_AuthAck = %d\n", rasconnstate);
            printf ("Authentication acknowledge...\n");
            break;
        case RASCS_ReAuthenticate: 
            printf ("RASCS_ReAuthenticate = %d\n", rasconnstate);
            printf ("Retrying Authentication...\n");
            break;
        case RASCS_Authenticated: 
            printf ("RASCS_Authenticated = %d\n", rasconnstate);
            printf ("Authentication complete.\n");
            break;
        case RASCS_PrepareForCallback: 
            printf ("RASCS_PrepareForCallback = %d\n", rasconnstate);
            printf ("Preparing for callback...\n");
            break;
        case RASCS_WaitForModemReset: 
            printf ("RASCS_WaitForModemReset = %d\n", rasconnstate);
            printf ("Waiting for modem reset...\n");
            break;
        case RASCS_WaitForCallback:
            printf ("RASCS_WaitForCallback = %d\n", rasconnstate);
            printf ("Waiting for callback...\n");
            break;
        case RASCS_Projected:  
            printf ("RASCS_Projected = %d\n", rasconnstate);
            printf ("Projection completed.\n");
            break;
    #if (WINVER >= 0x400) 
        case RASCS_StartAuthentication:    // Windows 95 only 
            printf ("RASCS_StartAuthentication = %d\n", rasconnstate);
            printf ("Starting authentication...\n");

            break;
        case RASCS_CallbackComplete:       // Windows 95 only 
            printf ("RASCS_CallbackComplete = %d\n", rasconnstate);
            printf ("Callback complete.\n");
            break;
        case RASCS_LogonNetwork:           // Windows 95 only 
            printf ("RASCS_LogonNetwork = %d\n", rasconnstate);
            printf ("Login to the network.\n");
            break;
    #endif 
        case RASCS_SubEntryConnected:
            printf ("RASCS_SubEntryConnected = %d\n", rasconnstate);
            printf ("Subentry connected.\n");
            break;
        case RASCS_SubEntryDisconnected:
            printf ("RASCS_SubEntryDisconnected = %d\n", rasconnstate);
            printf ("Subentry disconnected.\n");
            break;
		//PAUSED STATES:
		case RASCS_Interactive:
            printf ("RASCS_Interactive = %d\n", rasconnstate);
            printf ("In Paused state: Interactive mode.\n");
            break;
		case RASCS_RetryAuthentication:
            printf ("RASCS_RetryAuthentication = %d\n", rasconnstate);
            printf ("In Paused state: Retry Authentication...\n");
            break;
		case RASCS_CallbackSetByCaller:
            printf ("RASCS_CallbackSetByCaller = %d\n", rasconnstate);
            printf ("In Paused state: Callback set by Caller.\n");
            break;
		case RASCS_PasswordExpired:
            printf ("RASCS_PasswordExpired = %d\n", rasconnstate);
            printf ("In Paused state: Password has expired...\n");
            break;
 
        case RASCS_Connected: // = RASCS_DONE: 
            printf ("RASCS_Connected = %d\n", rasconnstate);
            printf ("Connection completed.\n");
			SetEvent(gEvent_handle);
            break;
        case RASCS_Disconnected: 
            printf ("RASCS_Disconnected = %d\n", rasconnstate);
            printf ("Disconnecting...\n");
            break;
		default:
			printf ("Unknown Status = %d\n", rasconnstate);
			printf ("What are you going to do about it?\n");
			break;
   }
} 