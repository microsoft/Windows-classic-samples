//
//       This is a part of the Microsoft Source Code Samples. 
//       Copyright 1993 - 2002 Microsoft Corporation.
//       All rights reserved. 
//       This source code is only intended as a supplement to 
//       Microsoft Development Tools and/or WinHelp documentation.
//       See these sources for detailed information regarding the 
//       Microsoft samples programs.
//

//
//		RasDial.c
//		
//		Usage:
//		RasDial -e [entry name] -p [phone number] -u [username] 
//				-z [password] -d [domain]
//
//		RAS API's used:
//			RasDial
//			RasHangUp
//			RasGetConnectStatus
//			RasGetProjectionInfo
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
#include <stdlib.h>
#include <time.h>
#include <tchar.h>
#include <strsafe.h>

// Macro for counting maximum characters that will fit into a buffer
#define CELEMS(x) ((sizeof(x))/(sizeof(x[0])))

// Usage
void Usage(char *pszProgName) 
{
    TCHAR szTempBuf[256] = {0};

    if (pszProgName)
    {
        StringCchPrintf(szTempBuf, 
                  CELEMS(szTempBuf), 
                  "Usage\n%s \t-e entry_name -p [phone_number] \n\t\t-u [username] -z [password] -d [domain]\n", 
                  pszProgName);

        printf(szTempBuf);
    }

    return;
}


int __cdecl main(int argc, char **argv) 
{
    LPRASDIALPARAMS     lpRasDialParams = NULL;
    HRASCONN            hRasConn = NULL;
    LPRASCONNSTATUS     lpRasConnStatus = NULL;
    RASPPPIP            *lpProjection = NULL;
    int                 i = 0;
    int                 j = 0;
    DWORD               dwRet = 0;
    DWORD               cb = sizeof(RASDIALPARAMS);
    DWORD               dwMaxTickCount = 0;	
    BOOL                fRequired = FALSE;
    BOOL                fSuccess = FALSE;
    TCHAR               szTempBuf[256] = {0};

    lpRasDialParams = (LPRASDIALPARAMS)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb); 
    if (NULL == lpRasDialParams)
    {
        printf("HeapAlloc failed\n");
        return ERROR_OUTOFMEMORY;
    }

    lpRasDialParams->dwSize = sizeof(RASDIALPARAMS);

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
                        j = ++i;
                        if (argv[j])
                        {
                            StringCchCopy(lpRasDialParams->szEntryName, CELEMS(lpRasDialParams->szEntryName), argv[j]);
                            fRequired = TRUE;
                        }
                        break;
                    case 'p': // Phone number
                        j = ++i;
                        if (argv[j])
                        {
                            StringCchCopy(lpRasDialParams->szPhoneNumber, CELEMS(lpRasDialParams->szPhoneNumber), argv[j]);
                        }
                        break;
                    case 'u': // User name
                        j = ++i;
                        if (argv[j])
                        {
                            StringCchCopy(lpRasDialParams->szUserName, CELEMS(lpRasDialParams->szUserName), argv[j]);
                        }
                        break;
                    case 'z': // Password
                        j = ++i;
                        if (argv[j])
                        {
                            StringCchCopy(lpRasDialParams->szPassword, CELEMS(lpRasDialParams->szPassword), argv[j]);
                        }
                        break;
                    case 'd': // Domain name
                        j = ++i;
                        if (argv[j])
                        {
                            StringCchCopy(lpRasDialParams->szDomain, CELEMS(lpRasDialParams->szDomain), argv[j]);
                        }
                        break;
                    default:
                        Usage(argv[0]);
                        dwRet = ERROR_INVALID_PARAMETER;
                        break;
	            }
	        }
            else
            {
                Usage(argv[0]);
                dwRet = ERROR_INVALID_PARAMETER;
                goto done;
            }
        }
    }
    else
    {
        Usage(argv[0]);
        dwRet = ERROR_INVALID_PARAMETER;
        goto done;
    }

    // Check if we got at least required entry name
    if (FALSE == fRequired)
    {
        Usage(argv[0]);
        dwRet = ERROR_INVALID_PARAMETER;
        goto done;
    }

    printf("Dialing...\n"); 

    // Calling RasDial synchronously
    dwRet = RasDial(NULL, NULL, lpRasDialParams, 0, 0L, &hRasConn);
    if (dwRet)
    {
        printf("RasDial failed: Error = %d\n", dwRet);
        goto done;
    }

    lpRasConnStatus = (LPRASCONNSTATUS)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(RASCONNSTATUS));	
    if (NULL == lpRasConnStatus)
    {
        printf("HeapAlloc failed.\n");
        goto disconnect;
    }

    // Set the appropriate size
    lpRasConnStatus->dwSize = sizeof(RASCONNSTATUS);

    // Checking connection status using RasGetConnectStatus
    dwRet = RasGetConnectStatus(hRasConn, lpRasConnStatus);
    if (ERROR_SUCCESS != dwRet)
    {
        printf("RasGetConnectStatus failed: Error = %d\n", dwRet);
        goto disconnect;
    }
    else
    {
        // Since the call succeeded, let's see what ras connection state we are in
        // by using the RASCONNSTATUS structure.
        if (lpRasConnStatus->rasconnstate == RASCS_Connected)
        {
            ZeroMemory((LPVOID)szTempBuf, sizeof(szTempBuf));
            StringCchPrintf(szTempBuf, CELEMS(szTempBuf), "Connection estabilished using %s\n", lpRasConnStatus->szDeviceName);
            printf(szTempBuf);
        }
        else
        {
            // We don't seem to be connected. Just try to terminate the connection by 
            // hanging up.
            printf("lpRasConnStatus->rasconnstate = %d\n", lpRasConnStatus->rasconnstate);
            goto disconnect;
        }
    }
	
    lpProjection = (RASPPPIP *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(RASPPPIP));
    if (NULL == lpProjection)
    {
        printf("HeapAlloc failed.\n");
        dwRet = ERROR_OUTOFMEMORY;
        goto disconnect;
    }

    cb = sizeof(RASPPPIP);
    lpProjection->dwSize = cb;
	
   // Getting the Ras client and server IP address using RasGetProjectionInfo
   dwRet = RasGetProjectionInfo(hRasConn, RASP_PppIp, lpProjection, &cb);
	
    switch (dwRet)
    {
    case ERROR_BUFFER_TOO_SMALL:
        if (HeapFree(GetProcessHeap(), 0, (LPVOID)lpProjection))
        {

            lpProjection = (RASPPPIP *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb);
            if (NULL == lpProjection)
            {
                printf("HeapAlloc failed.\n");
                goto disconnect;
            }

            dwRet = RasGetProjectionInfo(hRasConn, RASP_PppIp, lpProjection, &cb);
            if (ERROR_SUCCESS == dwRet)
            {
                fSuccess = TRUE;
            }
            else
            {
                printf("RasGetProjectionInfo failed: Error %d", dwRet);
                goto disconnect;
            }
        }
        else
        {
            printf("HeapFree failed.\n");
            goto disconnect;
        }
        break;

   case ERROR_SUCCESS:
        fSuccess = TRUE;
        break;

   default:
         printf("RasGetProjectionInfo failed: Error %d", dwRet);
		 goto disconnect;

        break;
    }

    if (fSuccess)
    {
        // Print out the IP addresses
        ZeroMemory((LPVOID)szTempBuf, sizeof(szTempBuf));
        StringCchPrintf(szTempBuf, CELEMS(szTempBuf), "\nRas Client IP address: %s\n", lpProjection->szIpAddress);
        printf(szTempBuf);

        ZeroMemory((LPVOID)szTempBuf, sizeof(szTempBuf));  
        StringCchPrintf(szTempBuf, CELEMS(szTempBuf), "Ras Server IP address: %s\n\n", lpProjection->szServerIpAddress);
        printf(szTempBuf);
    }

    printf("Pausing for 5 seconds before disconnecting...\n");
    Sleep(5000);

disconnect:

    // Terminating the connection using RasHangUp
    dwRet = RasHangUp(hRasConn);
    if (ERROR_SUCCESS != dwRet)
    {
	    printf("RasHangUp failed: Error = %d", dwRet);
	    goto done;
    }

    // Keep checking  for 10 seconds and make sure we are really disconnected.
    // Once the connection is disconnected, RasGetConnectStatus returns ERROR_INVALID_HANDLE.
    // or our timeout is reached we exit the while loop.
    // This gives the RAS API time to make sure the modem hangs up before we exit this process.
    // If a process exits with a connected RAS connection, the port could be stranded.
    dwMaxTickCount = GetTickCount() + 10000;

    while((RasGetConnectStatus(hRasConn, lpRasConnStatus) != ERROR_INVALID_HANDLE) && (dwMaxTickCount > GetTickCount()))
    {
	    Sleep(50);
    }

    printf("Diconnected\n");
    
    dwRet = ERROR_SUCCESS;

done:
    if (lpProjection)
    {
        HeapFree(GetProcessHeap(), 0, (LPVOID)lpProjection);
    }

    if (lpRasDialParams)
    {
        HeapFree(GetProcessHeap(), 0, (LPVOID)lpRasDialParams);
    }

    if (lpRasConnStatus)
    {
        HeapFree(GetProcessHeap(), 0, (LPVOID)lpRasConnStatus);
    }

   return (int)dwRet;
}