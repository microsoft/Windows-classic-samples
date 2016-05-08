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
//		RasEnumDevices.c
//		
//		Usage:
//		RasEnumDevices
//
//		RAS API's used:
//			RasEnumDevices
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
	DWORD	        i = 0;
	DWORD	        dwRet = 0;
	DWORD	        cb = sizeof(RASDEVINFO);
	DWORD	        cDevices = 0;
	LPRASDEVINFO    lpRasDevInfo;
	LPRASDEVINFO    lpTempRasDevInfo;
    BOOL            fSuccess = FALSE;
    TCHAR           szTempBuf[256] = {0};


    // Allocate buffer with space for at least one structure
    lpRasDevInfo = (LPRASDEVINFO)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb);
    if (NULL == lpRasDevInfo)
    {
        printf("HeapAlloc failed.\n");
        return ERROR_OUTOFMEMORY;
    }

    lpRasDevInfo->dwSize = sizeof(RASDEVINFO);
	
    dwRet = RasEnumDevices(lpRasDevInfo, &cb, &cDevices);
    
    switch(dwRet)   // Check whether RasEnumDevices succeeded
    {
    case ERROR_BUFFER_TOO_SMALL:
        // If the buffer is too small, free the allocated memory and allocate a bigger buffer.
        HeapFree(GetProcessHeap(), 0, (LPVOID)lpRasDevInfo);
        lpRasDevInfo = (LPRASDEVINFO)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb);
        if (NULL == lpRasDevInfo)
        {
            printf("HeapAlloc failed.\n");
            return ERROR_OUTOFMEMORY;
        }

        lpRasDevInfo->dwSize = sizeof(RASDEVINFO);

        dwRet = RasEnumDevices(lpRasDevInfo, &cb, &cDevices);
        if (ERROR_SUCCESS == dwRet)
        {
            fSuccess = TRUE;
        }
        else
        {
            printf("RasEnumDevices failed: Error = %d\n", dwRet);
            goto done;
        }
        break;

    case ERROR_SUCCESS:
            fSuccess = TRUE;
            break;

    default:
        printf("RasEnumDevices failed: Error = %d\n", dwRet);
        goto done;
    }
    
	if (fSuccess)
	{
		printf("The following RAS capable devices were found on this machine:\n\n");
		printf("Device\t\t\tCategory\n\n");
		lpTempRasDevInfo = lpRasDevInfo;
		for (i = 0; i < cDevices; i++)
		{
            ZeroMemory((LPVOID)szTempBuf, sizeof(szTempBuf));  
            StringCchPrintf(szTempBuf, CELEMS(szTempBuf), "%s\t%s\n",lpTempRasDevInfo->szDeviceName, lpTempRasDevInfo->szDeviceType);
			printf(szTempBuf);
			lpTempRasDevInfo++;
		}
	}

done:
	HeapFree(GetProcessHeap(), 0, (LPVOID)lpRasDevInfo);
	return (int)dwRet;
}
