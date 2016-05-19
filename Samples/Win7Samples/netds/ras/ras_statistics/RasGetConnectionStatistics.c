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
//		RasGetConnectionStatistics.c
//		
//		Usage:
//		RasGetConnectionStatistics
//
//		RAS API's used:
//			RasEnumConnections
//			RasGetConnectionStatistics
//
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x500
#endif

// Macro for counting maximum characters that will fit into a buffer
#define CELEMS(x) ((sizeof(x))/(sizeof(x[0])))

#include <windows.h>
#include <stdio.h>
#include <ras.h>
#include <raserror.h>
#include <tchar.h>
#include <strsafe.h>

int __cdecl main(void)
{
	DWORD		nRet = 0;
	LPRASCONN	lpRasConn = NULL;
    LPRASCONN	lpTempRasConn = NULL;
	DWORD		cb = sizeof(RASCONN);
	DWORD		cConnections = 0;
    RAS_STATS	*lpStatistics = NULL;
    BOOL        fSuccess = FALSE;
    DWORD       i = 0;
    TCHAR       szTempBuf[256] = {0};

	// Allocate buffer with default value
	lpRasConn = (LPRASCONN)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb);
	if (NULL == lpRasConn)
	{
		printf("HeapAlloc failed.\n");
		return ERROR_OUTOFMEMORY;
	}

	lpRasConn->dwSize = sizeof(RASCONN);

	// Call RasEnumConnections to obtain the handle of the current active RAS connection
	nRet = RasEnumConnections(lpRasConn, &cb, &cConnections);
	
    switch (nRet)
    {
    case ERROR_BUFFER_TOO_SMALL:
        if (HeapFree(GetProcessHeap(), 0, (LPVOID)lpRasConn))
        {

            lpRasConn = (LPRASCONN)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb);
	        if (NULL == lpRasConn)
	        {
		        printf("HeapAlloc failed.\n");
		        return ERROR_OUTOFMEMORY;
	        }

	        lpRasConn->dwSize = sizeof(RASCONN);

            nRet = RasEnumConnections(lpRasConn, &cb, &cConnections);
            if (ERROR_SUCCESS == nRet)
            {
                fSuccess = TRUE;
            }
            else
            {
		        printf("RasEnumConnections failed: Error = %d\n", nRet);
                goto done;
            }
        }
        else
        {
            printf("HeapFree failed.\n");
            return GetLastError();
        }
        break;

    case ERROR_SUCCESS:
            fSuccess = TRUE;
            break;

    default:
		printf("RasEnumConnections failed: Error = %d\n", nRet);
        goto done;
        break;
    }

    if (fSuccess)
    {
	    // Allocate buffer to obtain the RAS statistics
	    lpStatistics = (RAS_STATS*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(RAS_STATS));

	    if (NULL == lpStatistics)
	    {
		    printf("HeapAlloc failed.\n");
		    goto done;
	    }

        lpTempRasConn = lpRasConn;
        for (i = 0; i < cConnections; i++)
        {
            ZeroMemory(lpStatistics, sizeof(RAS_STATS));
	        lpStatistics->dwSize = sizeof(RAS_STATS);

	        // Call RasGetConnectionStatistics
	        nRet = RasGetConnectionStatistics(lpTempRasConn->hrasconn, lpStatistics);
	        if(ERROR_SUCCESS != nRet) // RasGetConnectionStatistics returned an error
	        {
		        printf("RasGetConnectionStatistics failed: Error = %d\n", nRet);
                goto done;
	        }

            
	        
            // Print the results obtained
	        StringCchPrintf(szTempBuf, CELEMS(szTempBuf), "Statistics for %s connection\n\n", lpTempRasConn->szEntryName);
            printf(szTempBuf);
            	
	        printf("Bytes Xmited\t\t\t%d\n", lpStatistics->dwBytesXmited);
	        printf("Bytes Received\t\t\t%d\n", lpStatistics->dwBytesRcved);
	        printf("Frames Xmited\t\t\t%d\n", lpStatistics->dwFramesXmited);
	        printf("Frames Received\t\t\t%d\n", lpStatistics->dwFramesRcved);
	        printf("Crc Error\t\t\t%d\n", lpStatistics->dwCrcErr);
	        printf("Timeout Error\t\t\t%d\n", lpStatistics->dwTimeoutErr);
	        printf("Alignment Error\t\t\t%d\n", lpStatistics->dwAlignmentErr);
	        printf("Hardware Overrun Error\t\t%d\n", lpStatistics->dwHardwareOverrunErr);
	        printf("Framing Error\t\t\t%d\n", lpStatistics->dwFramingErr);
	        printf("Buffer Overrun Error\t\t%d\n", lpStatistics->dwBufferOverrunErr);
	        printf("Compression Ratio [In]\t\t%d\n", lpStatistics->dwCompressionRatioIn);
	        printf("Compression Ratio [Out]\t\t%d\n", lpStatistics->dwCompressionRatioOut);
	        printf("Baud Rate [bps]\t\t\t%d\n", lpStatistics->dwBps);
	        printf("Connection Duration [mili sec]\t%d\n", lpStatistics->dwConnectDuration);
            lpTempRasConn++;
        }

        nRet = ERROR_SUCCESS;
    }

done:
	// Clean up
	if (lpRasConn)
    {
        HeapFree(GetProcessHeap(), 0, (LPVOID)lpRasConn);
    }

    if (lpStatistics)
    {
	    HeapFree(GetProcessHeap(), 0, (LPVOID)lpStatistics);
    }
	
	return (int)nRet;
}