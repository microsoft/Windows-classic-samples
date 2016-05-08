//////////////////////////////////////////////////////////////////////////
//
// Registry.h: Registry helpers.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

namespace MediaFoundationSamples
{

    #ifndef CHARS_IN_GUID
    const DWORD CHARS_IN_GUID = 39;
    #endif


    // Forward declares
    HRESULT RegisterObject(HMODULE hModule, const GUID& guid, const TCHAR *sDescription, const TCHAR *sThreadingModel);
    HRESULT UnregisterObject(const GUID& guid);
    HRESULT CreateObjectKeyName(const GUID& guid, TCHAR *sName, DWORD cchMax);
    HRESULT SetKeyValue(HKEY hKey, const TCHAR *sName, const TCHAR *sValue);


    ///////////////////////////////////////////////////////////////////////
    // Name: RegisterObject
    // Desc: Creates the registry entries for a COM object.
    //
    // guid: The object's CLSID
    // sDescription: Description of the object
    // sThreadingMode: Threading model. e.g., "Both"
    ///////////////////////////////////////////////////////////////////////

    inline HRESULT RegisterObject(HMODULE hModule, const GUID& guid, const TCHAR *sDescription, const TCHAR *sThreadingModel)
    {
        HKEY hKey = NULL;
        HKEY hSubkey = NULL;

        TCHAR achTemp[MAX_PATH];

        // Create the name of the key from the object's CLSID
        HRESULT hr = CreateObjectKeyName(guid, achTemp, MAX_PATH);

        // Create the new key.
        if (SUCCEEDED(hr))
        {
            LONG lreturn = RegCreateKeyEx(
                HKEY_CLASSES_ROOT,
                (LPCTSTR)achTemp,     // subkey
                0,                    // reserved
                NULL,                 // class string (can be NULL)
                REG_OPTION_NON_VOLATILE,
                KEY_ALL_ACCESS,
                NULL,                 // security attributes
                &hKey,
                NULL                  // receives the "disposition" (is it a new or existing key)
                );

            hr = __HRESULT_FROM_WIN32(lreturn);
        }

        // The default key value is a description of the object.
        if (SUCCEEDED(hr))
        {
            hr = SetKeyValue(hKey, NULL, sDescription);
        }

        // Create the "InprocServer32" subkey
        if (SUCCEEDED(hr))
        {
            const TCHAR *sServer = TEXT("InprocServer32");

            LONG lreturn = RegCreateKeyEx(hKey, sServer, 0, NULL,
                REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hSubkey, NULL);

            hr = __HRESULT_FROM_WIN32(lreturn);
        }

        // The default value for this subkey is the path to the DLL.
        // Get the name of the module ...
        if (SUCCEEDED(hr))
        {
            DWORD res = GetModuleFileName(hModule, achTemp, MAX_PATH);
            if (res == 0)
            {
                hr = __HRESULT_FROM_WIN32(GetLastError());
            }
            if (res == MAX_PATH)
            {
                hr = E_FAIL; // buffer too small
            }
        }

        // ... and set the default key value.
        if (SUCCEEDED(hr))
        {
            hr = SetKeyValue(hSubkey, NULL, achTemp);
        }

        // Add a new value to the subkey, for "ThreadingModel" = <threading model>
        if (SUCCEEDED(hr))
        {
            hr = SetKeyValue(hSubkey, TEXT("ThreadingModel"), sThreadingModel);
        }

        // close hkeys

        if (hSubkey != NULL)
        {
            RegCloseKey( hSubkey );
        }

        if (hKey != NULL)
        {
            RegCloseKey( hKey );
        }

        return hr;



    }

    ///////////////////////////////////////////////////////////////////////
    // Name: UnregisterObject
    // Desc: Deletes the registry entries for a COM object.
    //
    // guid: The object's CLSID
    ///////////////////////////////////////////////////////////////////////

    inline HRESULT UnregisterObject(const GUID& guid)
    {
        TCHAR achTemp[MAX_PATH];

        HRESULT hr = CreateObjectKeyName(guid, achTemp, MAX_PATH);

        if (SUCCEEDED(hr))
        {
            // Delete the key recursively.
            DWORD res = RegDeleteTree(HKEY_CLASSES_ROOT, achTemp);

            if (res == ERROR_SUCCESS)
            {
                hr = S_OK;
            }
            else
            {
                hr = __HRESULT_FROM_WIN32(res);
            }
        }

        return hr;
    }


    ///////////////////////////////////////////////////////////////////////
    // Name: CreateObjectKeyName
    // Desc: Converts a CLSID into a string with the form "CLSID\{clsid}"
    ///////////////////////////////////////////////////////////////////////

    inline HRESULT CreateObjectKeyName(const GUID& guid, TCHAR *sName, DWORD cchMax)
    {
      // convert CLSID uuid to string 
      OLECHAR szCLSID[CHARS_IN_GUID];
      HRESULT hr = StringFromGUID2(guid, szCLSID, CHARS_IN_GUID);
      if (FAILED(hr))
      {
          return hr;
      }

      // Create a string of the form "CLSID\{clsid}"
      return StringCchPrintf(sName, cchMax, TEXT("CLSID\\%ls"), szCLSID);
    }

    ///////////////////////////////////////////////////////////////////////
    // Name: SetKeyValue
    // Desc: Sets a string value (REG_SZ) for a registry key
    //
    // hKey:   Handle to the registry key.
    // sName:  Name of the value. Use NULL for the default value.
    // sValue: The string value.
    ///////////////////////////////////////////////////////////////////////

    inline HRESULT SetKeyValue(HKEY hKey, const TCHAR *sName, const TCHAR *sValue)
    {
        size_t cch = 0;

        HRESULT hr = StringCchLength(sValue, MAXLONG, &cch);
        if (SUCCEEDED(hr))
        {
            // Size must include NULL terminator, which is not counted in StringCchLength
            DWORD  cbData = ((DWORD)cch + 1) * sizeof(TCHAR);

            // set description string
            LONG ret = RegSetValueEx(hKey, sName, 0, REG_SZ, (BYTE*)sValue, cbData);
            if (ret == ERROR_SUCCESS)
            {
                hr = S_OK;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ret);
            }
        }
        return hr;
    }

}; // namespace MediaFoundationSamples
