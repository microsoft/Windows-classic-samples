//==========================================================================
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//--------------------------------------------------------------------------

//
//This module implements registry functions for the newfsp service provider
//

#include "SampleFSP.h"

//+---------------------------------------------------------------------------
//
//  function:   GetNewFspRegistryData
//
//  Synopsis:   Get the registry data for the newfsp service provider
//
//  Arguments:  [bLoggingEnabled] - indicates if logging is enabled
//                [lpszLoggingDirectory] - indicates the logging directory
//                [pDeviceInfo] - pointer to the virtual fax devices
//                [pdwNumDevice] - pointer to the number of virtual fax devices
//
//  Returns:    TRUE on success
//
//----------------------------------------------------------------------------

BOOL
GetNewFspRegistryData(
                BOOL          *bLoggingEnabled,
                LPWSTR        lpszLoggingDirectory,
                DWORD          dwLoggingDirectoryBufferSize,
                PDEVICE_INFO  *pDeviceInfo,
                LPDWORD       pdwNumDevices
                )
{
        // hServiceProvidersKey is the handle to the fax service providers registry key
        HKEY          hServiceProvidersKey = NULL;
        // hNewFspKey is the handle to the newfsp service provider registry key
        HKEY          hNewFspKey = NULL;
        // hDevicesKey is the handle to the virtual fax devices registry key
        HKEY          hDevicesKey = NULL;
        // dwSubkeys is the number of virtual fax device registry subkeys
        DWORD         dwSubkeys;
        // dwIndex is a counter to enumerate each virtual fax device registry subkey
        DWORD         dwIndex;
        // szDeviceSubkey is the name of a virtual fax device registry subkey
        WCHAR         szDeviceSubkey[MAX_PATH] ={0};
        // hDeviceSubkey is the handle to a virtual fax device registry subkey
        HKEY          hDeviceSubkey = NULL;
        DWORD         dwType;

        // pCurrentDeviceInfo is a pointer to the current virtual fax device
        PDEVICE_INFO  pCurrentDeviceInfo = NULL;

        DWORD         dwLoggingEnabledSize;
        DWORD         dwLoggingDirectorySize = dwLoggingDirectoryBufferSize - 1;
        DWORD         dwDirectorySize;
        HRESULT hr = S_OK;
        BOOL bRetVal = FALSE;

        if (bLoggingEnabled != NULL) {
                *bLoggingEnabled = FALSE;
        }

        if (pDeviceInfo != NULL) {
                *pDeviceInfo = NULL;
        }

        if (pdwNumDevices != NULL) {
                *pdwNumDevices = 0;
        }


        // Open the fax service providers registry key
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                                FAX_PROVIDERS_REGKEY, 
                                0, 
                                KEY_QUERY_VALUE|KEY_ENUMERATE_SUB_KEYS, 
                                &hServiceProvidersKey) != ERROR_SUCCESS) {
                goto Exit;
        }

        // Open the newfsp service provider registry key
        if (RegOpenKeyEx(hServiceProvidersKey, 
                                NEWFSP_PROVIDER,
                                0,
                                KEY_QUERY_VALUE|KEY_ENUMERATE_SUB_KEYS,
                                &hNewFspKey) != ERROR_SUCCESS) {
                goto Exit;
        }

        if (bLoggingEnabled != NULL) {
                // Get the logging enabled
                dwLoggingEnabledSize = sizeof(BOOL);
                RegQueryValueEx(hNewFspKey, 
                                NEWFSP_LOGGING_ENABLED, 
                                NULL,
                                &dwType,
                                (LPBYTE) bLoggingEnabled,
                                &dwLoggingEnabledSize);
        }

        if (lpszLoggingDirectory != NULL) {
                // Get the logging directory
                if ((RegQueryValueEx(hNewFspKey,
                                                NEWFSP_LOGGING_DIRECTORY,
                                                NULL,
                                                &dwType,
                                                (LPBYTE) lpszLoggingDirectory,
                                                &dwLoggingDirectorySize) != ERROR_SUCCESS)||
                                (dwLoggingDirectorySize >= MAX_PATH)){                        
                        goto Exit;
                }
        }

        if ((pDeviceInfo != NULL) || (pdwNumDevices != NULL)) {
                // Open the virtual fax devices registry key
                if (RegOpenKeyEx(hNewFspKey,
                                        NEWFSP_DEVICES,
                                        0,
                                        KEY_QUERY_VALUE|KEY_ENUMERATE_SUB_KEYS,
                                        &hDevicesKey) != ERROR_SUCCESS) {
                        goto Exit;
                }

                // Determine the number of virtual fax device registry subkeys
                if (RegQueryInfoKey(hDevicesKey,
                                        NULL,
                                        NULL,
                                        NULL,
                                        &dwSubkeys,
                                        NULL,
                                        NULL,
                                        NULL,
                                        NULL,
                                        NULL,
                                        NULL,
                                        NULL) != ERROR_SUCCESS) {
                        goto Exit;
                }
        }

        if (pdwNumDevices != NULL) {
                if (dwSubkeys < NEWFSP_DEVICE_LIMIT) {
                        *pdwNumDevices = dwSubkeys;
                }
                else {
                        *pdwNumDevices = NEWFSP_DEVICE_LIMIT;
                }
        }

        if (pDeviceInfo != NULL) {
                if (dwSubkeys > 0) {
                        // Allocate a block of memory for the first virtual fax device data
                        *pDeviceInfo = (PDEVICE_INFO) MemAllocMacro(sizeof(DEVICE_INFO));
                }

                // Enumerate the virtual fax device registry subkeys
                for (pCurrentDeviceInfo = *pDeviceInfo, dwIndex = 0;
                                (dwIndex < dwSubkeys) && (dwIndex < NEWFSP_DEVICE_LIMIT);
                                pCurrentDeviceInfo = pCurrentDeviceInfo->pNextDeviceInfo, dwIndex++) {
                        if (pCurrentDeviceInfo == NULL) {
                                // A memory allocation for virtual fax device data failed, so go with what we have so far
                                *pdwNumDevices = dwIndex;

                                break;
                        }

                        // Set the name of the virtual fax device registry subkey
                        hr = StringCchPrintf(szDeviceSubkey,MAX_PATH, L"%d", dwIndex);
                        if(hr != S_OK)
                        {
                                WriteDebugString( L"StringCchPrintf failed, hr = 0x%x for szDeviceSubkey", hr );
                                bRetVal = FALSE;
                                goto Exit;
                        }                        
                        // Set the identifier of the virtual fax device
                        pCurrentDeviceInfo->DeviceId = dwIndex;

                        if (RegOpenKeyEx(hDevicesKey, 
                                                szDeviceSubkey,
                                                0, 
                                                KEY_QUERY_VALUE|KEY_ENUMERATE_SUB_KEYS,
                                                &hDeviceSubkey) == ERROR_SUCCESS) {


                                // Get the incoming fax directory for the virtual fax device
                                dwDirectorySize = sizeof(pCurrentDeviceInfo->Directory)/sizeof(pCurrentDeviceInfo->Directory[0]);
                                if (RegQueryValueEx(hDeviceSubkey,
                                                        NEWFSP_DEVICE_DIRECTORY,
                                                        NULL,
                                                        &dwType,
                                                        (LPBYTE) pCurrentDeviceInfo->Directory,
                                                        &dwDirectorySize) != ERROR_SUCCESS) {
                                        RegCloseKey(hDeviceSubkey);
                                        continue;
                                }
                                RegCloseKey(hDeviceSubkey);
                        }

                        // Allocate a block of memory for the next virtual fax device data
                        if ((dwIndex < (dwSubkeys - 1)) && (dwIndex < (NEWFSP_DEVICE_LIMIT - 1))) {
                                pCurrentDeviceInfo->pNextDeviceInfo =  (_DEVICE_INFO*) MemAllocMacro(sizeof(DEVICE_INFO));
                        }
                        else {
                                pCurrentDeviceInfo->pNextDeviceInfo = NULL;
                        }

                }
        }
        bRetVal = TRUE;
Exit:
        if(hDevicesKey)
        {
                RegCloseKey(hDevicesKey);
                hDevicesKey = NULL;
        }
        if(hNewFspKey)
        {
                RegCloseKey(hNewFspKey);
                hNewFspKey = NULL;
        }
        if(hServiceProvidersKey)
        {
                RegCloseKey(hServiceProvidersKey);
                hServiceProvidersKey = NULL;
        }
        return TRUE;
}
//+---------------------------------------------------------------------------
//
//  function:   SetNewFspRegistryData
//
//  Synopsis:   Set the registry data for the newfsp service provider
//
//  Arguments:  [bLoggingEnabled] - indicates if logging is enabled
//                [lpszLoggingDirectory] - indicates the logging directory
//                [pDeviceInfo] - pointer to the virtual fax devices
//
//  Returns:    void
//
//----------------------------------------------------------------------------

VOID
SetNewFspRegistryData(
                BOOL          bLoggingEnabled,
                LPWSTR        lpszLoggingDirectory,
                PDEVICE_INFO  pDeviceInfo
                )
{
        // hServiceProvidersKey is the handle to the fax service providers registry key
        HKEY          hServiceProvidersKey = NULL;
        // hNewFspKey is the handle to the newfsp service provider registry key
        HKEY          hNewFspKey = NULL;
        // hDevicesKey is the handle to the virtual fax devices registry key
        HKEY          hDevicesKey = NULL;
        // dwSubkeys is the number of virtual fax device registry subkeys
        DWORD         dwSubkeys;
        // dwIndex is a counter to enumerate each virtual fax device registry subkey
        DWORD         dwIndex;
        // szDeviceSubkey is the name of a virtual fax device registry subkey
        WCHAR         szDeviceSubkey[MAX_PATH] = {0};
        // hDeviceSubkey is the handle to a virtual fax device registry subkey
        HKEY          hDeviceSubkey = NULL;
        DWORD         dwDisposition;        
        HRESULT hr = S_OK;        
        BOOL bRetVal = FALSE;

        // pCurrentDeviceInfo is a pointer to the current virtual fax device
        PDEVICE_INFO  pCurrentDeviceInfo = NULL;

        // Open the fax service providers registry key
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                FAX_PROVIDERS_REGKEY,
                                0,
                                KEY_CREATE_SUB_KEY,
                                &hServiceProvidersKey) != ERROR_SUCCESS) {
                goto Exit;
        }

        // Open the newfsp service provider registry key
        if (RegCreateKeyEx(hServiceProvidersKey,
                                NEWFSP_PROVIDER,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_CREATE_SUB_KEY|KEY_SET_VALUE,
                                NULL,
                                &hNewFspKey,
                                &dwDisposition) != ERROR_SUCCESS) {
                goto Exit;                
        }

        // Set the logging enabled
        RegSetValueEx(hNewFspKey, 
                        NEWFSP_LOGGING_ENABLED,
                        0,
                        REG_DWORD,
                        (LPBYTE) &bLoggingEnabled,
                        sizeof(bLoggingEnabled));

        // Set the logging directory
        if (lpszLoggingDirectory != NULL) {
                RegSetValueEx(hNewFspKey,
                                NEWFSP_LOGGING_DIRECTORY,
                                0,
                                REG_SZ,
                                (LPBYTE) lpszLoggingDirectory,
                                (lstrlen(lpszLoggingDirectory) + 1) * sizeof(WCHAR));
        }

        // Open the virtual fax devices registry key
        if (RegCreateKeyEx(hNewFspKey,
                                NEWFSP_DEVICES,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_CREATE_SUB_KEY|KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE|KEY_SET_VALUE,
                                NULL,
                                &hDevicesKey,
                                &dwDisposition) != ERROR_SUCCESS) {
                goto Exit;                
        }

        // Determine the number of virtual fax device registry subkeys
        if (RegQueryInfoKey(hDevicesKey,
                                NULL,
                                NULL,
                                NULL,
                                &dwSubkeys,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL) != ERROR_SUCCESS) {
                goto Exit;
        }

        // Enumerate the virtual fax device registry subkeys
        for (pCurrentDeviceInfo = pDeviceInfo, dwIndex = 0;
                        pCurrentDeviceInfo; 
                        pCurrentDeviceInfo = pCurrentDeviceInfo->pNextDeviceInfo, dwIndex++) {
                // Set the name of the virtual fax device registry subkey
                hr = StringCchPrintf(szDeviceSubkey,MAX_PATH, L"%d", pCurrentDeviceInfo->DeviceId);
                if(hr != S_OK)
                {
                        WriteDebugString( L"StringCchPrintf failed, hr = 0x%x for szDeviceSubkey", hr );
                        bRetVal = FALSE;
                        goto Exit;
                }

                //wsprintf(szDeviceSubkey, L"%d", pCurrentDeviceInfo->DeviceId);
                if (RegCreateKeyEx(hDevicesKey,
                                        szDeviceSubkey,
                                        0,
                                        NULL,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_QUERY_VALUE|KEY_SET_VALUE,
                                        NULL,
                                        &hDeviceSubkey,
                                        &dwDisposition) == ERROR_SUCCESS) {
                        // Set the incoming fax directory for the virtual fax device
                        RegSetValueEx(hDeviceSubkey,
                                        NEWFSP_DEVICE_DIRECTORY,
                                        0,
                                        REG_SZ,
                                        (LPBYTE) pCurrentDeviceInfo->Directory,
                                        (lstrlen(pCurrentDeviceInfo->Directory) + 1) * sizeof(WCHAR));

                        RegCloseKey(hDeviceSubkey);
                }
        }

        // Delete any removed virtual fax device registry subkeys
        for ( ; dwIndex < dwSubkeys; dwIndex++) {
                // Set the name of the virtual fax device registry subkey                
                hr = StringCchPrintf(szDeviceSubkey,MAX_PATH, L"%d",dwIndex);
                if(hr != S_OK)
                {
                        WriteDebugString( L"StringCchPrintf failed, hr = 0x%x for szDeviceSubkey", hr );
                        bRetVal = FALSE;
                        goto Exit;
                }

                RegDeleteKey(hDevicesKey, szDeviceSubkey);
        }
Exit:
        if(hDevicesKey)
        {
                RegCloseKey(hDevicesKey);
                hDevicesKey = NULL;
        }
        if(hNewFspKey)
        {
                RegCloseKey(hNewFspKey);
                hNewFspKey = NULL;
        }
        if(hServiceProvidersKey)
        {
                RegCloseKey(hServiceProvidersKey);
                hServiceProvidersKey = NULL;
        }
}
