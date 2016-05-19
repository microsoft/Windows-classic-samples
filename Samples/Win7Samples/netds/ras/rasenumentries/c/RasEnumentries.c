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
//		RasEnumEntries.c
//		
//		Usage:
//		RasEnumEntry
//
//		RAS API's used:
//			RasEnumEntries
//
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
	LPRASENTRYNAME lpRasEntryName = NULL;
	LPRASENTRYNAME lpTempRasEntryName = NULL;
	DWORD cb = sizeof(RASENTRYNAME);
	DWORD cEntries = 0;
	int nRet = 0;
	DWORD i = 0;
    BOOL fSuccess = FALSE;
    TCHAR           szTempBuf[256] = {0};

	lpRasEntryName = (LPRASENTRYNAME)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb);
	if (NULL == lpRasEntryName)
	{
		printf("HeapAlloc failed.\n");
		return ERROR_OUTOFMEMORY;
	}

	lpRasEntryName->dwSize = sizeof(RASENTRYNAME);

	// Getting the size required for the RASENTRYNAME buffer

	nRet = RasEnumEntries(NULL, NULL, lpRasEntryName, &cb, &cEntries);

	switch (nRet)
	{
	case ERROR_BUFFER_TOO_SMALL:
		if (HeapFree(GetProcessHeap(), 0, (LPVOID)lpRasEntryName))
        {
        
            lpRasEntryName = (LPRASENTRYNAME)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb);
    	    if (NULL == lpRasEntryName)
		    {
			    printf("HeapAlloc failed.\n");
                return ERROR_OUTOFMEMORY;
		    }

		    lpRasEntryName->dwSize = sizeof(RASENTRYNAME);
    		
		    // Calling RasEnumEntries to enumerate the phonebook entries for a default phonebook	
		    nRet = RasEnumEntries(NULL, NULL, lpRasEntryName, &cb, &cEntries);
		    if (ERROR_SUCCESS != nRet)
		    {
			    printf("RasEnumEntries failed: Error %d\n", nRet);
			    goto done;
		    }
		    else
		    {
                fSuccess = TRUE;
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
		printf("RasEnumEntries failed: Error = %d\n", nRet);
		goto done;
	}


    if (fSuccess)
    {
		printf("Phone book entries in the default phonebook:\n\n");
		lpTempRasEntryName = lpRasEntryName;
		for (i = 0; i < cEntries; i++)
		{
            ZeroMemory((LPVOID)szTempBuf, sizeof(szTempBuf));  
            StringCchPrintf(szTempBuf, CELEMS(szTempBuf), "%s\n",lpTempRasEntryName->szEntryName);
			printf(szTempBuf);
			lpTempRasEntryName++;
		}
    }
		
done:
    HeapFree(GetProcessHeap(), 0, (LPVOID)lpRasEntryName);

    return nRet;
}