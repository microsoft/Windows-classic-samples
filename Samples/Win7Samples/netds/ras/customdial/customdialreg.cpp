/*
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
 * ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 * Copyright © 2000  Microsoft Corporation.  All Rights Reserved.
 *
 * Author: Stephen R. Husak - Microsoft Developer Support
 *
 * Abstract: 
 *    The DLL provides DLLRegisterServer/DLLUnregisterServer
 *    exports to provide a way to install the DLL into the registry
 *    correctly for Windows 2000. In this manner one can use regsvr32.exe
 *    to add/remove the custom DLL in the registry. This is normally used
 *    for COM objects and servers but I use it here as a simple technique of
 *    wrapping everything in one place. 
 */

/*
 * Includes
 */
//#define WINVER 0x0500   // needed for Windows 2000 RAS extensions

#ifndef UNICODE
#define UNICODE         // this is UNICODE
#endif

#include <windows.h>    // include windows
#include <ras.h>        // include RAS for creating the entry
#include <raserror.h>
#include <stddef.h>     // includes type definitions
#include <strsafe.h>

#define CELEMS(x) ((sizeof(x))/(sizeof(x[0])))

/*
 * constants for configuration
 */
#define RASCUSTOMDIAL_KEYPATH   L"SYSTEM\\CurrentControlSet\\Services\\Rasman\\Parameters"
#define RASCUSTOMDIAL_VALUENAME L"CustomDLL"
#define RASCUSTOMDIAL_ENTRYNAME L"Steelhead"
#define RASCUSTOMDIAL_PHONENO   L"29719"

/*
 * from the customdial.cpp file
 */
extern HANDLE g_hInstance;      // from DLLMain
extern void OutputTraceString(LPTSTR lpszFormatString, ...);

/*
 * CreateTestEntry
 *
 * creates a test entry for testing this DLL
 */
BOOL CreateTestEntry(LPCTSTR szDLLPath)
{
  DWORD rc;               // return code from function calls
  DWORD dwSize;           // structure size
  RASENTRY *RasEntry = NULL;      // Ras Entry structure
  BOOL bResult = TRUE;    // return for the function
  RasEntry = (RASENTRY *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
	  sizeof(RASENTRY));

  OutputTraceString(L"CreateTestEntry called in customdial.dll\n");
  rc = RasGetEntryProperties(NULL, L"", RasEntry, &dwSize, NULL, NULL);
  if( rc == ERROR_BUFFER_TOO_SMALL )
  {
	RasEntry = (RASENTRY *) HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
		RasEntry, dwSize);
	RasEntry->dwSize = sizeof( RASENTRY );
	rc = RasGetEntryProperties(NULL, L"", RasEntry, &dwSize, NULL, NULL);
  }

  if( rc != ERROR_SUCCESS )
  {
    OutputTraceString(L"--- RasGetEntryProperties returned error: %ld.\n", rc );
    bResult = FALSE;
  }
  else
  {
    // Validate the format of the connection entry name.
    rc = RasValidateEntryName(NULL, RASCUSTOMDIAL_ENTRYNAME);
    if (rc == ERROR_INVALID_NAME)
    {
      OutputTraceString(L"--- RasValidateEntryName returned invalid name in CreateTestEntry: %d\n", rc);
      bResult = FALSE;
    }
    else if (rc == ERROR_ALREADY_EXISTS)
    {
      rc = RasGetEntryProperties(NULL, RASCUSTOMDIAL_ENTRYNAME, RasEntry, &dwSize, NULL, NULL);

      if (rc == ERROR_BUFFER_TOO_SMALL)
      {
        RasEntry = (RASENTRY *) HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
  		    RasEntry, dwSize);
        RasEntry->dwSize = sizeof( RASENTRY );
        rc = RasGetEntryProperties(NULL, RASCUSTOMDIAL_ENTRYNAME, RasEntry, &dwSize, NULL, NULL);
      }

      if (rc != ERROR_SUCCESS)
      {
        OutputTraceString(L"--- RasGetEntryProperties failed in CreateTestEntry: %d\n", rc);
        bResult = FALSE;
      }
      else
        // Just modify the existing entry to make sure DLL is set
        StringCchCopy(RasEntry->szCustomDialDll, CELEMS(RasEntry->szCustomDialDll), szDLLPath);
    }
    else
    {    

      rc = RasGetEntryProperties(NULL, L"", RasEntry, &dwSize, NULL, NULL);
      if (rc != ERROR_SUCCESS)
      {
        OutputTraceString(L"--- RasGetEntryProperties failed in CreateTestEntry: %d\n", rc);
        bResult = FALSE;
      }
      else
      {
        // Modify the default entry     
        StringCchCopy(RasEntry->szLocalPhoneNumber, CELEMS( RasEntry->szLocalPhoneNumber ), RASCUSTOMDIAL_PHONENO);
        StringCchCopy(RasEntry->szDeviceType, CELEMS( RasEntry->szDeviceType ), RASDT_Modem);
        StringCchCopy(RasEntry->szCustomDialDll, CELEMS( RasEntry->szCustomDialDll ), szDLLPath);
        
        //
        // enumerate to get the device name to use - we'll just pick one (a modem)
        //
        // first get the size of the buffer required
        LPRASDEVINFO lpRasDevInfo; 
        DWORD dwNumEntries;

        RasEnumDevices(NULL, &dwSize, &dwNumEntries);
        lpRasDevInfo = (LPRASDEVINFO) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);
        if (lpRasDevInfo == NULL)
        {
          OutputTraceString(L"--- HeapAlloc failed in CreateTestEntry (GetLastError = %d)\n", GetLastError());
          bResult = FALSE;
        }
        else
        {
          lpRasDevInfo->dwSize = sizeof(RASDEVINFO);
   
          rc = RasEnumDevices(lpRasDevInfo, &dwSize, &dwNumEntries);
          if (rc != ERROR_SUCCESS)
          {
            OutputTraceString(L"--- RasEnumDevices failed in CreateTestEntry: %d", rc);
            bResult = FALSE;
          }
          else
          {          
            for (UINT i=0; i < dwNumEntries; i++, lpRasDevInfo++)
            {           
              if (lstrcmpi(lpRasDevInfo->szDeviceType, RASDT_Modem) == 0)
              {
                OutputTraceString(L"-- Modem device: %s\n", lpRasDevInfo->szDeviceName);
                StringCchCopy(RasEntry->szDeviceName, CELEMS( RasEntry->szDeviceName ), lpRasDevInfo->szDeviceName);   
                break;
              }            
            }
          }
            
          HeapFree(GetProcessHeap(), 0, lpRasDevInfo);        
        }
      }
    }  
  }

  if( rc == ERROR_SUCCESS )
  {
    // write the phonebook entry.
    rc = RasSetEntryProperties(NULL, RASCUSTOMDIAL_ENTRYNAME, RasEntry, sizeof(RASENTRY), NULL, 0);
    if (rc != ERROR_SUCCESS)  
    {
      OutputTraceString(L"--- RasSetEntryProperties failed in CreateTestEntry: %d\n", rc);
      bResult = FALSE;
    }
  }
    
  OutputTraceString(L"CreateTestEntry exiting customdial.dll\n");

  if( RasEntry )
  {
    HeapFree( GetProcessHeap(), 0, RasEntry );
  }

  return bResult;
}

/*
 * CheckForSelf
 *
 * checks a buffer of \0 terminated strings for an instance of 
 * string that could be present and returns the position of that
 * string in the buffer
 */
TCHAR * CheckForSelf(LPCTSTR pSrc, LPBYTE pDest, DWORD dwBufLen)
{  
  DWORD dwLen = 0;  // length of the buffer 
  TCHAR * p;        // pointer into the buffer

  // loop through the buffer up to the length
  while (dwLen < dwBufLen)
    {
    p = (LPTSTR) &pDest[dwLen];   // get out string

    if (lstrcmpi(pSrc, p) == 0)   // check it
      return p; 
  
    // go past string we just checked + the terminating '\0'
    dwLen = dwLen + ((lstrlen(p) + 1) * sizeof(TCHAR));
    }

  return NULL;      // string was not found
}

/*
 * DllRegisterServer
 *
 * We are "overriding" the intent of this to provide an easy way to 
 * install our DLL into the correct registry locations using "common" tools
 * such as regsvr32.exe.
 *
 * We need to add our DLL to the list of custom dial DLLs in:
 * Path:       \\HKLM\System\CurrentControlSet\Services\Rasman\Parameters 
 * Key Type:   REG_MULTI_SZ
 * Value Name: CustomDLL
 * Value:      Full path to the DLL
 *
 * this either return E_UNEXPECTED if any error occurred or S_OK if success or
 * if it is already there
 */
STDAPI DllRegisterServer(void)
{
  HRESULT hr = E_UNEXPECTED;    // return code
  HKEY hKey;                    // handle to open registry key
  TCHAR wszPath[MAX_PATH];      // full path to this DLL


  OutputTraceString(L"DllRegisterServer called in customdial.dll\n");

  // get the path to this DLL
  GetModuleFileName((HMODULE) g_hInstance, wszPath, MAX_PATH);

  // open registry for reading/writing
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, RASCUSTOMDIAL_KEYPATH, 
               0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    {
    // read in current value
    DWORD dwSize = 0;     // size of buffer for reading registry
    DWORD dwBufSize = 0;  // size of buffer for writing to registry
    DWORD dwType;         // type of buffer
    LPBYTE buffer = NULL; // the buffer

    __try
      {
      // obtain current value size
      RegQueryValueEx(hKey, RASCUSTOMDIAL_VALUENAME, NULL, &dwType, NULL, &dwSize);
    
      // allocate buffer to read (and write) all NULLs will be there
      if (dwSize > 0)
        dwBufSize = dwSize + (lstrlen(wszPath) * sizeof(TCHAR)) + sizeof(TCHAR);   // should be terminated in a \0 
      else
        dwBufSize = (lstrlen(wszPath) * sizeof(TCHAR)) + (2 * sizeof(TCHAR));   // should be terminated in a \0\0 pair 

      buffer = (LPBYTE) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY | HEAP_GENERATE_EXCEPTIONS, dwBufSize);
      
      // read it if dwSize was > 0
      if (dwSize > 0)
        {
        if (RegQueryValueEx(hKey, RASCUSTOMDIAL_VALUENAME, NULL, &dwType, buffer, &dwSize) != ERROR_SUCCESS)
          return hr;    // force to finally block via AbnormalTermination     
        }
      
      // if it isn't there add it in
      if (CheckForSelf(wszPath, buffer, dwSize) == NULL)
        {
        DWORD dwBufPosition = 0;    // position of new string in the buffer

        if (dwSize > 0)
          dwBufPosition = dwSize - sizeof(TCHAR); // back off original pos

        // add to the buffer
        CopyMemory(&buffer[dwBufPosition], wszPath, lstrlen(wszPath) * sizeof(TCHAR));

        // write it
        dwType = REG_MULTI_SZ;
        if (RegSetValueEx(hKey, RASCUSTOMDIAL_VALUENAME, 0, dwType, buffer, dwBufSize) != ERROR_SUCCESS)
          return hr;    // force to finally block via AbnormalTermination
        }
      else
        __leave;      // it is already there - force to finally block normally
      }
    __finally
      {
      if (!AbnormalTermination())
        {
        hr = S_OK;    
        }
      
      if (buffer)
        HeapFree(GetProcessHeap(), 0, buffer);

      RegCloseKey(hKey);
      } // finally block
    }
  
  CreateTestEntry(wszPath);

  if (hr == S_OK)
    MessageBox(NULL, L"Windows may have to be restarted for the changes to take effect.", 
                     L"CustomDial DLL Sample", MB_OK);

  OutputTraceString(L"DllRegisterServer exiting customdial.dll with return code: 0x%08x\n", hr);

  return hr;
}

/*
 * DllUnregisterServer
 *
 * We are "overriding" the intent of this to provide an easy way to 
 * uninstall our DLL from the registry using "common" tools such as
 * regsvr32.exe.
 *
 * this either return E_UNEXPECTED if any error occurred or S_OK if success or
 * if it isn't there
 */
STDAPI DllUnregisterServer(void)
{
  HRESULT hr = E_UNEXPECTED;    // return code
  HKEY hKey;                    // handle to open registry key
  TCHAR wszPath[MAX_PATH];      // full path to this DLL
  DebugBreak();

  OutputTraceString(L"DllUnregisterServer called in customdial.dll\n");

  // get the path to this DLL
  GetModuleFileName((HMODULE) g_hInstance, wszPath, MAX_PATH);

  // open registry for reading/writing
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, RASCUSTOMDIAL_KEYPATH, 
               0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    {
    DWORD dwSize = 0;         // size of buffer for reading/writing registry
    DWORD dwType = 0;             // type of buffer
    LPBYTE buffer = NULL;     // the buffer
    LPBYTE newBuffer = NULL;  // pointer to buffer
    TCHAR * p = NULL;                // pointer to the string in the buffer
    ptrdiff_t uPos = 0;              // position of string in buffer

    __try
      {
      // obtain current value size
      RegQueryValueEx(hKey, RASCUSTOMDIAL_VALUENAME, NULL, &dwType, NULL, &dwSize);
  
      if (dwSize > 0)
        {
        // allocate buffer to read
        buffer = (LPBYTE) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY | HEAP_GENERATE_EXCEPTIONS, dwSize);
        }
      else
        __leave;        // there's nothing there...
     
      // read current value
      if (RegQueryValueEx(hKey, RASCUSTOMDIAL_VALUENAME, NULL, &dwType, buffer, &dwSize) != ERROR_SUCCESS)
        return hr;    // force to finally block via AbnormalTermination     
            
      // rebuild buffer without the string in there
      newBuffer = buffer;
      p = CheckForSelf(wszPath, newBuffer, dwSize);
      
      if (p == NULL)    // it's not there
        __leave;        
      
      // determine the position of the string in the buffer, move other data
      // accordingly and adjust size
      uPos = (LPBYTE) p - newBuffer;      
      if (uPos == 0)      
        {
        // it's the beginning - just move the buffer
        newBuffer = &newBuffer[(lstrlen(p) + 1) * sizeof(TCHAR)];
        dwSize = dwSize - ((lstrlen(p) + 1) * sizeof(TCHAR));
        }
      else
        if (uPos + (lstrlen(p) * sizeof(TCHAR)) + (2 * sizeof(TCHAR)) == dwSize)
          {
          // it's the last one - adjust size and add a double termination
          dwSize = dwSize - ((lstrlen(p) + 1) * sizeof(TCHAR));
          CopyMemory(&newBuffer[dwSize - sizeof(TCHAR)], L"\0", sizeof(TCHAR));
          }
        else
          {
          // it's in the middle - move the back to overwriting the org. position 
          DWORD dwCopySize = dwSize - (uPos + ((lstrlen(p) + 1) * sizeof(TCHAR)));
          dwSize = dwSize - ((lstrlen(p) + 1) * sizeof(TCHAR));
          CopyMemory(&newBuffer[uPos], &newBuffer[uPos + ((lstrlen(p) + 1) * sizeof(TCHAR))], dwCopySize);          
          }
                 
      // write it or delete it
      if (dwSize > sizeof(TCHAR))     // there is only 1 '\0' left... 
        {
        dwType = REG_MULTI_SZ;
        if (RegSetValueEx(hKey, RASCUSTOMDIAL_VALUENAME, 0, dwType, newBuffer, dwSize) != ERROR_SUCCESS)
          return hr;    // force to finally block via AbnormalTermination
        }
      else        
        if (RegDeleteValue(hKey, RASCUSTOMDIAL_VALUENAME) != ERROR_SUCCESS)
          return hr;    // force to finally block via AbnormalTermination 
      }
    __finally
      {
      if (!AbnormalTermination())
        {
        // means there was no error during calls and we reached here via normal
        // unwrapping of the stack
        hr = S_OK;    
        }
      
      if (buffer)
        HeapFree(GetProcessHeap(), 0, buffer);

      RegCloseKey(hKey);
      } // finally block
    }

  // remove the test entry
  if (DWORD rc = RasDeleteEntry(NULL, RASCUSTOMDIAL_ENTRYNAME) != ERROR_SUCCESS)
    OutputTraceString(L"--- RasDeleteEntry failed in DllUnregisterServer: Error = %d\n", rc);
    
  if (hr == S_OK)
    MessageBox(NULL, L"Windows may have to be restarted for the changes to take effect.", 
                     L"CustomDial DLL Sample", MB_OK);

  OutputTraceString(L"DllUnregisterServer exiting customdial.dll with return code: 0x%08x\n", hr);

  return hr;
}

// EOF: customdialreg.cpp