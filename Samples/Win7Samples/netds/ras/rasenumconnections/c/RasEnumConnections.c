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
//		RasEnumConnections.c
//		
//		Usage:
//		RasEnumConnections
//
//		RAS API's used:
//				RasEnumConnections
//
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x500
#endif


#include <windows.h>
#include <stdio.h>
#include <ras.h>
#include <raserror.h>
#include <tchar.h>
#include <strsafe.h>

// Macro for counting maximum characters that will fit into a buffer
#define CELEMS(x) ((sizeof(x))/(sizeof(x[0])))

int __cdecl main(void)
{
	DWORD		dwRet = 0;
	LPRASCONN	lpRasConn = NULL;
	LPRASCONN   lpTempRasConn = NULL;
    DWORD		cb = sizeof(RASCONN);
	DWORD		dwConnections = 0;
	DWORD		i = 0;
    BOOL        fSuccess = FALSE;
    TCHAR       szTempBuf[256] = {0};

	// Allocate buffer with space for at least one structure
	lpRasConn = (LPRASCONN)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb);
	if (NULL == lpRasConn)
	{
		printf("HeapAlloc failed.\n");
		return ERROR_OUTOFMEMORY;
	}

	lpRasConn->dwSize = sizeof(RASCONN);

	dwRet = RasEnumConnections(lpRasConn, &cb, &dwConnections);

	switch (dwRet) // Check whether RasEnumConnections succeeded
	{
	case ERROR_BUFFER_TOO_SMALL: // Since initial buffer allocation was small.
		if (HeapFree(GetProcessHeap(), 0, (LPVOID)lpRasConn)) // Free initial buffer
        {
		    // And reassign a new buffer with the value returned in cb
	        lpRasConn = (LPRASCONN)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb);
		    if (NULL == lpRasConn)
		    {
			    printf("HeapAlloc failed.\n");
			    return ERROR_OUTOFMEMORY;
		    }
    		
		    lpRasConn->dwSize = sizeof(RASCONN);

		    // Again call RasEnumConnections
		    dwRet = RasEnumConnections(lpRasConn, &cb, &dwConnections);
		    if (ERROR_SUCCESS == dwRet)
		    { 
                fSuccess = TRUE;
            }
            else
            {
                // RasEnumConnections failed
			    printf("RasEnumConnections failed: Error = %d\n", dwRet);
		        goto done;
		    }
        }
        else
        {
            printf("HeapFree failed.\n");
            goto done;
        }
		break;
	
	case ERROR_SUCCESS: // RasEnumConnections succeeded with intial buffer allocation.
		fSuccess = TRUE;
		break;
	
	default: // RasEnumConnections failed with some error.
		printf("RasEnumConnections failed: Error = %d\n", dwRet);
		break;
	}

    if (fSuccess)
    {


        // RasEnumConnections succeeded. Print the results.
		printf("The following RAS connections are currently active\n\n");
		lpTempRasConn = lpRasConn;
		for (i = 0; i < dwConnections; i++)
		{
			printf("Size: %d\n", lpTempRasConn->dwSize);

            ZeroMemory((LPVOID)szTempBuf, sizeof(szTempBuf));  
            StringCchPrintf(szTempBuf, CELEMS(szTempBuf), "Entry name: %s\n", lpTempRasConn->szEntryName);
			printf(szTempBuf);

            ZeroMemory((LPVOID)szTempBuf, sizeof(szTempBuf));  
            StringCchPrintf(szTempBuf, CELEMS(szTempBuf), "Device name: %s\n", lpTempRasConn->szDeviceName);
			printf(szTempBuf);
			lpTempRasConn++;
		}
    }

done:
    HeapFree(GetProcessHeap(), 0, (LPVOID)lpRasConn); 
	return (int)dwRet;
}