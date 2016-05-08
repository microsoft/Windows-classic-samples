///////////////////////////////////////////////////////////////////////////////
//
//  File Name
//      customentry.c
//
// Written for Microsoft Developer Support
// Copyright (c) 1995 - 2002 Microsoft Corporation. All rights reserved.
//
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x500
#endif

#ifdef UNICODE
#undef UNICODE
#endif

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <ras.h>
#include <raserror.h>
#include <tchar.h>
#include <strsafe.h>			 

void Usage();
void OutputTraceString(LPTSTR lpszFormatString, ...);
BOOL CreateEntry(LPTSTR pszEntryName, LPTSTR pszPhoneNumber, LPTSTR pszDLLPath, BOOL fCustomScript);


// Macro for counting maximum characters that will fit into a buffer
#define CELEMS(x) ((sizeof(x))/(sizeof(x[0])))


#define RASCUSTOMDIAL_KEYPATH   "SYSTEM\\CurrentControlSet\\Services\\Rasman\\Parameters"
#define RASCUSTOMDIAL_VALUENAME "CustomDLL"

// Helper function to make custom dll a trusted component. 
// It writes custom DLL path into registry 
HRESULT CreateRegistryKey(LPTSTR pszPath);
TCHAR * CheckForSelf(LPCTSTR pSrc, LPBYTE pDest, DWORD dwBufLen);

#define RASCUSTOMDIAL_ENTRYNAME TEXT("SDKSample")
#define MAX_DEBUGSTR 256      // max string for ouput

int __cdecl main (int argc, TCHAR * argv[])
{
	DWORD dwErr = 0;
    TCHAR szEntryName[RAS_MaxEntryName + 1] = {0};
    TCHAR szPhoneNumber[RAS_MaxPhoneNumber + 1] = {0};
    TCHAR szCustomDLLPath[MAX_PATH + 1] = {0};
    RASENTRY rasEntry = {0};
	BOOL fRetVal = FALSE;
    BOOL fCustomScript = FALSE;

    // 1st argument is phone number, second is custom script, third is custom dial dll path
    if (3 <= argc)
    {
        if (3 == argc)
        {
            StringCchCopy(szPhoneNumber, CELEMS(szPhoneNumber), argv[1]);
            fCustomScript = (BOOL)_ttol(argv[2]);
        }
        else
        {
            if (4 == argc)
            {
                StringCchCopy(szPhoneNumber, CELEMS(szPhoneNumber), argv[1]);
                fCustomScript = (BOOL)_ttol(argv[2]);
                StringCchCopy(szCustomDLLPath, CELEMS(szCustomDLLPath), argv[3]);   
            }
            else
            {
                Usage();
                return ERROR_INVALID_PARAMETER;
            }
        }
    }
    else
    {
        Usage();
        return ERROR_INVALID_PARAMETER;
    }

    StringCchCopy(szEntryName, CELEMS(szEntryName), RASCUSTOMDIAL_ENTRYNAME);

    fRetVal = CreateEntry(szEntryName, szPhoneNumber, szCustomDLLPath, fCustomScript);

    if (fRetVal) 
    {
        _tprintf(_T("Successfully created the custom entry %s \n"),szEntryName);
    }
    else 
    {
        _tprintf(_T("Failed to Create the entry.\n"));
    }

    return (int)fRetVal;
}

void Usage()
{
    printf("PhoneNumber is required. CustomDLLPath is optional\n");   
    printf("Usage : customentry <PhoneNumber> <custom_script> [<CustomDLLPath>]\n");   
    printf("\ncustom_script:\n 0 - no custom script is being used.\n 1 - using custom script.\n");   
    
    return;
}


//
// CreateTestEntry
//
// creates a ras entry. If pszDLLPath isn't 0 length, then we'll set 
// the szCustomDialDll member of RASENTRY structure as well for use with 
// custom dial DLL.
//
BOOL CreateEntry(LPTSTR pszEntryName, LPTSTR pszPhoneNumber, LPTSTR pszDLLPath, BOOL fCustomScript)
{
    DWORD rc = 0;               // return code from function calls
    DWORD dwSize = 0;           // structure size
    RASENTRY *pRasEntry = NULL; // Ras Entry structure
    BOOL bResult = FALSE;       // return value for the function
    LPRASDEVINFO lpRasDevInfo = NULL; 
    LPRASDEVINFO lpRasDevInfoTemp = NULL; 
    DWORD dwNumEntries = 0;
    BOOL fSuccess = FALSE;

    OutputTraceString("CreateEntry called.\n");

    if (!pszEntryName || !pszPhoneNumber || !pszDLLPath)
    {
        return FALSE;
    }

    dwSize = sizeof(RASENTRY);
    pRasEntry = (RASENTRY*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);
    if (NULL == pRasEntry)
    {
        return FALSE;
    }

    // Initialize the RASENTRY structure.
    pRasEntry->dwSize = dwSize;

    // When passing in L"" as 2nd param this call populates pRasEntry with default values.
    rc = RasGetEntryProperties(NULL, "", pRasEntry, &dwSize, NULL, NULL);
    if (ERROR_BUFFER_TOO_SMALL == rc)
    {
        if (HeapFree(GetProcessHeap(), 0, (LPVOID)pRasEntry))
        {
            pRasEntry = (RASENTRY*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);
            if (NULL == pRasEntry)
            {
                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }

        // Initialize the RASENTRY structure.
        pRasEntry->dwSize = sizeof(RASENTRY);
        rc = RasGetEntryProperties(NULL, "", pRasEntry, &dwSize, NULL, NULL);
    }

    if (ERROR_SUCCESS != rc)
    {
        goto done;
    }
    

    // Modify the entry structure
    StringCchCopy(pRasEntry->szLocalPhoneNumber, CELEMS(pRasEntry->szLocalPhoneNumber), pszPhoneNumber);
    StringCchCopy(pRasEntry->szDeviceType, CELEMS(pRasEntry->szDeviceType), RASDT_Modem);
    StringCchCopy(pRasEntry->szCustomDialDll, CELEMS(pRasEntry->szCustomDialDll), pszDLLPath);
    
    pRasEntry->dwfOptions = 0;
    pRasEntry->dwfOptions = RASEO_PreviewPhoneNumber | RASEO_ShowDialingProgress | RASEO_PreviewUserPw | RASEO_PreviewDomain;

    if (fCustomScript)
    {
        printf("Setting RASEO_CustomScript option.\n");
        pRasEntry->dwfOptions |= RASEO_CustomScript;
    }

    pRasEntry->dwfNetProtocols = RASNP_Ip | RASNP_Ipx; 
    pRasEntry->dwSubEntries = 1; 
    pRasEntry->dwType =  RASET_Phone;
    pRasEntry->dwEncryptionType = ET_None; 

    //
    // enumerate to get the device name to use - we'll just pick one (a modem)
    //
    // first get the size of the buffer required
    // Allocate buffer with default value
    dwSize = sizeof(RASDEVINFO);
    lpRasDevInfo = (LPRASDEVINFO)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);
    if (NULL == lpRasDevInfo)
    {
        printf("HeapAlloc failed.\n");
        goto done;
    }

    lpRasDevInfo->dwSize = sizeof(RASDEVINFO);

    rc = RasEnumDevices(lpRasDevInfo, &dwSize, &dwNumEntries);
    
    switch(rc)   // Check whether RasEnumDevices succeeded
    {
    case ERROR_BUFFER_TOO_SMALL:
        // If the buffer is too small, free the allocated memory and allocate a bigger buffer.
        
        if (HeapFree(GetProcessHeap(), 0, (LPVOID)lpRasDevInfo))
        {
            lpRasDevInfo = (LPRASDEVINFO) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);
            if (NULL == lpRasDevInfo)
            {
                OutputTraceString(_T("--- HeapAlloc failed in CreateTestEntry (GetLastError = %d)\n"), GetLastError());
                goto done;
            }

            lpRasDevInfo->dwSize = sizeof(RASDEVINFO);

            rc = RasEnumDevices(lpRasDevInfo, &dwSize, &dwNumEntries);
            if (ERROR_SUCCESS != rc)
            {
                OutputTraceString(_TEXT("--- RasEnumDevices failed in CreateTestEntry: %d"), rc);
                goto done;
            }

            fSuccess = TRUE;
        }
        else
        {
            OutputTraceString(_T("--- HeapFree failed in CreateTestEntry (GetLastError = %d)\n"), GetLastError());
            goto done;
        }
        break;
    case ERROR_SUCCESS:
        fSuccess = TRUE;
        break;

    default:
        printf("RasEnumDevices failed: Error = %d\n", rc);
        goto done;
    }

    lpRasDevInfoTemp = lpRasDevInfo;
    if (fSuccess)
    {
        UINT i = 0;
        for (i = 0; i < dwNumEntries; i++, lpRasDevInfoTemp++)
        {           
            // See if this is a modem device
            if (CSTR_EQUAL == CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT),
                                            NORM_IGNORECASE, 
                                            lpRasDevInfoTemp->szDeviceType,
                                            -1,
                                            RASDT_Modem,
                                            -1))
            {    
                OutputTraceString(_TEXT("-- Modem device: %s\n"), lpRasDevInfoTemp->szDeviceName);
                StringCchCopy(pRasEntry->szDeviceName, CELEMS(pRasEntry->szDeviceName), lpRasDevInfoTemp->szDeviceName);   
                break;
            }            
        }
    }

    // Write the phonebook entry.
    rc = RasSetEntryProperties(NULL, pszEntryName, pRasEntry, sizeof(RASENTRY), NULL, 0);
    if (ERROR_SUCCESS != rc)  
    {
        OutputTraceString(_TEXT("--- RasSetEntryProperties failed in CreateTestEntry: %d\n"), rc);
    }
    else
    {
        if (lstrlen(pszDLLPath))
        {
            OutputTraceString(_TEXT("--- Calling CreateRegistryKey\n"));
            printf("--- Calling CreateRegistryKey\n");
            bResult  = (S_OK == CreateRegistryKey(pszDLLPath)? TRUE : FALSE);
        }
        else
        {
            bResult = TRUE;
        }
    }
    
done:

    if (pRasEntry)
    {
        HeapFree(GetProcessHeap(), 0, (LPVOID)pRasEntry);
    }

    if (lpRasDevInfo)
    {
        HeapFree(GetProcessHeap(), 0, (LPVOID)lpRasDevInfo);        
    }

    OutputTraceString(_TEXT("Exiting CreateEntry\n"));

    return bResult;
}

//
// OutputTraceString
//
// multiple argument front-end to OutputDebugString
//
void OutputTraceString(LPTSTR lpszFormatString, ...)
{
    TCHAR szOutputString[MAX_DEBUGSTR] = {0};  // the string to print - limited
    va_list arglist;                     // variable argument list


  // create the new string limited to the char count as indicated
  va_start(arglist, lpszFormatString);
  StringCchVPrintf(szOutputString, CELEMS(szOutputString), lpszFormatString, arglist);
  va_end(arglist);

  // send it out
  OutputDebugString(szOutputString);
}



HRESULT CreateRegistryKey(LPTSTR pszPath)
{
    HRESULT hr = E_UNEXPECTED;      // return code
    HKEY hKey;                      // handle to open registry key
    LONG lRet = 0;

    OutputTraceString(_TEXT("CreateRegistryKey called.\n"));

    // open registry for reading/writing
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, RASCUSTOMDIAL_KEYPATH, 
               0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    {
        // read in current value
        DWORD dwSize = 0;           // size of buffer for reading registry
        DWORD dwBufSize = 0;        // size of buffer for writing to registry
        DWORD dwType = 0;           // type of buffer
        LPBYTE buffer = NULL;       // the buffer
        DWORD dwBufPosition = 0;    // position of new string in the buffer

        // obtain current value size
        RegQueryValueEx(hKey, RASCUSTOMDIAL_VALUENAME, NULL, &dwType, NULL, &dwSize);

        // allocate buffer to read (and write) all NULLs will be there
        if (dwSize > 0)
        {
            dwBufSize = dwSize + (lstrlen(pszPath) * sizeof(TCHAR)) + sizeof(TCHAR);   // should be terminated in a \0 
        }
        else
        {
            dwBufSize = (lstrlen(pszPath) * sizeof(TCHAR)) + (2 * sizeof(TCHAR));   // should be terminated in a \0\0 pair 
        }

        buffer = (LPBYTE) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBufSize);
        
        if (!buffer)
        {
            hr = E_OUTOFMEMORY;
            return hr;
        }

        // read it if dwSize was > 0
        if (dwSize > 0)
        {
            lRet = RegQueryValueEx(hKey, RASCUSTOMDIAL_VALUENAME, NULL, &dwType, buffer, &dwSize);
            if (ERROR_SUCCESS != lRet)
            {
                hr = HRESULT_FROM_WIN32(lRet);    
                goto done;
            }
        }

        // if it isn't there add it in
        if (CheckForSelf(pszPath, buffer, dwSize) == NULL)
        {
            if (dwSize > 0)
            {
                dwBufPosition = dwSize - sizeof(TCHAR); // back off original pos
            }

            // add to the buffer
            CopyMemory(&buffer[dwBufPosition], pszPath, lstrlen(pszPath) * sizeof(TCHAR));

            // write it
            dwType = REG_MULTI_SZ;
        
            lRet = RegSetValueEx(hKey, RASCUSTOMDIAL_VALUENAME, 0, dwType, buffer, dwBufSize);
            if (ERROR_SUCCESS != lRet)
            {
                hr = HRESULT_FROM_WIN32(lRet);    
                goto done;
            }
        }

        hr = S_OK;    
      
done:      
        if (buffer)
        {
            HeapFree(GetProcessHeap(), 0, (LPVOID)buffer);
        }

        RegCloseKey(hKey);
      
    }

    return hr;
}

TCHAR * CheckForSelf(LPCTSTR pSrc, LPBYTE pDest, DWORD dwBufLen)
{  
    DWORD dwLen = 0;  // length of the buffer 
    TCHAR * p;        // pointer into the buffer

    if (!pSrc || !pDest)
    {
        return NULL;
    }

    // loop through the buffer up to the length
    while (dwLen < dwBufLen)
    {
        p = (LPTSTR) &pDest[dwLen];   // get out string

        // Compare the strings
        if (CSTR_EQUAL == CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT),
                                        NORM_IGNORECASE, 
                                        pSrc,
                                        -1,
                                        p,
                                        -1))
        {
            return p; 
        }
      
        // go past string we just checked + the terminating '\0'
        dwLen = dwLen + ((lstrlen(p) + 1) * sizeof(TCHAR));
    }

    return NULL;      // string was not found
}



