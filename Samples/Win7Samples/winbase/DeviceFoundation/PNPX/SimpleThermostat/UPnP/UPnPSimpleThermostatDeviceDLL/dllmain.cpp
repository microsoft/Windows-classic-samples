////////////////////////////////////////////////////////////////////////////////
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
////////////////////////////////////////////////////////////////////////////////

// Public Headers
#include <windows.h>
#include <strsafe.h>

// Sample Headers
#include "CSimpleThermostatDeviceFactory.h"
#include "SimpleThermostatDevice.h"
#include "common.h"

const ULONG OLEGUID_LEN_CCH = 40;
const WCHAR OBJECT_NAME[] = L"UPnP Simple Thermostat Device";
const WCHAR TYPELIB_NAME[] = L"UPnP Simple Thermostat Device Type Library";
const WCHAR THREADING_MODEL[] = L"Free";
LONG g_cLockCount = 0;

extern "C"

//------------------------------------------------------------------------------
// DllMain
//------------------------------------------------------------------------------
BOOL WINAPI DllMain(
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID lpReserved
    )
{
    BOOL bRet = TRUE;

    if( DLL_PROCESS_ATTACH == dwReason )
    {
        bRet = DisableThreadLibraryCalls( hInstance );
    }

    return bRet;
}// DllMain


//------------------------------------------------------------------------------
// DllGetClassObject
//------------------------------------------------------------------------------
STDAPI DllGetClassObject(
    REFCLSID rclsid,
    REFIID riid,
    LPVOID* ppv
    )
{
    HRESULT hr = E_FAIL;
    CSimpleThermostatDeviceFactory* pCSimpleThermostatDeviceFactory = NULL; 

    if( rclsid != CLSID_SimpleThermostatDevice )
    {
        return CLASS_E_CLASSNOTAVAILABLE;
    }

    pCSimpleThermostatDeviceFactory = new CSimpleThermostatDeviceFactory();
    if( NULL == pCSimpleThermostatDeviceFactory )
    {
        return E_OUTOFMEMORY;
    }
	
    hr = pCSimpleThermostatDeviceFactory->QueryInterface( riid, ppv );
    pCSimpleThermostatDeviceFactory->Release();

    return hr;
}// DllGetClassObject


//------------------------------------------------------------------------------
// DllCanUnloadNow
//------------------------------------------------------------------------------
STDAPI DllCanUnloadNow()
{
    if( 0 == g_cLockCount )
    {
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}// DllCanUnloadNow


//------------------------------------------------------------------------------
// DllRegisterServer
//------------------------------------------------------------------------------
STDAPI DllRegisterServer()
{
    HKEY    hKey                        = NULL;
    HMODULE hModule                     = NULL;
    HRESULT hr                          = S_OK;
    LONG    lResult                     = ERROR_SUCCESS;
    WCHAR   szFilename[MAX_PATH]        = {0};
    WCHAR   szKey[MAX_PATH]             = {0};
    WCHAR   szValue[MAX_PATH]           = {0};
    WCHAR   szCLSID[OLEGUID_LEN_CCH]    = {0};
    WCHAR   szLIBID[OLEGUID_LEN_CCH]    = {0};

    //
    // Grab the fully qualified path to this dll
    //
    hModule = GetModuleHandleW( L"UPnPSimpleThermostatDevice" );
    if( NULL == hModule )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
    }
    else if( 0 == GetModuleFileNameW( hModule, szFilename, ARRAYSIZE(szFilename) ) )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
    }

    //
    // Register the COM object in the registry
    //
    if( S_OK == hr &&
        0 == StringFromGUID2( CLSID_SimpleThermostatDevice, szCLSID, ARRAYSIZE(szCLSID) ) )
    {  
        hr = HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER );
    }

    if( S_OK == hr )
    {
        hr = StringCchPrintfW( szKey, ARRAYSIZE(szKey), L"CLSID\\%s", szCLSID );
    }

    if( S_OK == hr )
    {
        lResult = RegCreateKeyExW(
            HKEY_CLASSES_ROOT,
            szKey,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_SET_VALUE,
            NULL,
            &hKey,
            NULL
            );
        hr = HRESULT_FROM_WIN32( lResult );
    }

    if( S_OK == hr )
    {
        lResult = RegSetValueExW(
            hKey,
            NULL,
            0,
            REG_SZ,
            (BYTE*)OBJECT_NAME,
            (static_cast<DWORD>(wcslen(OBJECT_NAME))+1)*sizeof(WCHAR)
            );
        hr = HRESULT_FROM_WIN32( lResult );
    }
    RegCloseKey( hKey );
    hKey = NULL;

    if( S_OK == hr )
    {
        hr = StringCchPrintfW(
            szKey,
            ARRAYSIZE(szKey),
            L"CLSID\\%s\\InProcServer32",
            szCLSID
            );
    }

    if( S_OK == hr )
    {
        lResult = RegCreateKeyExW(
            HKEY_CLASSES_ROOT,
            szKey,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_SET_VALUE,
            NULL,
            &hKey,
            NULL
            );
        hr = HRESULT_FROM_WIN32( lResult );
    }

    if( S_OK == hr )
    {
        lResult = RegSetValueExW(
            hKey,
            NULL,
            0,
            REG_SZ,
            (BYTE*)szFilename,
            (static_cast<DWORD>(wcslen(szFilename))+1)*sizeof(WCHAR)
            );
        hr = HRESULT_FROM_WIN32( lResult );
    }

    if( S_OK == hr )
    {
        lResult = RegSetValueExW(
            hKey,
            L"ThreadingModel",
            0,
            REG_SZ,
            (BYTE*)THREADING_MODEL,
            (static_cast<DWORD>(wcslen(THREADING_MODEL))+1)*sizeof(WCHAR)
            );
        hr = HRESULT_FROM_WIN32( lResult );
    }
    RegCloseKey( hKey );
    hKey = NULL;

    if( S_OK == hr )
    {
        hr = StringCchPrintfW(
            szKey,
            ARRAYSIZE(szKey),
            L"CLSID\\%s\\TypeLib",
            szCLSID
            );
    }

    if( S_OK == hr )
    {
        lResult = RegCreateKeyExW(
            HKEY_CLASSES_ROOT,
            szKey,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_SET_VALUE,
            NULL,
            &hKey,
            NULL
            );
        hr = HRESULT_FROM_WIN32( lResult );
    }

    if( S_OK == hr &&
        0 == StringFromGUID2( LIBID_SimpleThermostatDeviceLib, szLIBID, ARRAYSIZE(szLIBID) ) )
    {  
        hr = HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER );
    }

    if( S_OK == hr )
    {
        lResult = RegSetValueExW(
            hKey,
            NULL,
            0,
            REG_SZ,
            (BYTE*)szLIBID,
            (static_cast<DWORD>(wcslen(szCLSID))+1)*sizeof(WCHAR)
            );
        hr = HRESULT_FROM_WIN32( lResult );
    }
    RegCloseKey( hKey );
    hKey = NULL;

    //
    // Register Type Library
    //
    if( S_OK == hr )
    {
        hr = StringCchPrintfW(
            szKey,
            ARRAYSIZE(szKey),
            L"SOFTWARE\\Classes\\TypeLib\\%s",
            szLIBID
            );
    }

    if( S_OK == hr )
    {
        lResult = RegCreateKeyExW(
            HKEY_LOCAL_MACHINE,
            szKey,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_SET_VALUE,
            NULL,
            &hKey,
            NULL
            );
        hr = HRESULT_FROM_WIN32( lResult );
    }
    RegCloseKey( hKey );
    hKey = NULL;

    if( S_OK == hr )
    {
        hr = StringCchPrintfW(
            szKey,
            ARRAYSIZE(szKey),
            L"SOFTWARE\\Classes\\TypeLib\\%s\\1.0",
            szLIBID
            );
    }

    if( S_OK == hr )
    {
        lResult = RegCreateKeyExW(
            HKEY_LOCAL_MACHINE,
            szKey,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_SET_VALUE,
            NULL,
            &hKey,
            NULL
            );
        hr = HRESULT_FROM_WIN32( lResult );
    }

    if( S_OK == hr )
    {
        lResult = RegSetValueExW(
            hKey,
            NULL,
            0,
            REG_SZ,
            (BYTE*)TYPELIB_NAME,
            (static_cast<DWORD>(wcslen(TYPELIB_NAME))+1)*sizeof(WCHAR)
            );
        hr = HRESULT_FROM_WIN32( lResult );
    }
    RegCloseKey( hKey );
    hKey = NULL;

    if( S_OK == hr )
    {
        hr = StringCchPrintfW(
            szKey,
            ARRAYSIZE(szKey),
            L"SOFTWARE\\Classes\\TypeLib\\%s\\1.0\\0",
            szLIBID
            );
    }

    if( S_OK == hr )
    {
        lResult = RegCreateKeyExW(
            HKEY_LOCAL_MACHINE,
            szKey,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_SET_VALUE,
            NULL,
            &hKey,
            NULL
            );
        hr = HRESULT_FROM_WIN32( lResult );
    }
    RegCloseKey( hKey );
    hKey = NULL;

    if( S_OK == hr )
    {
        hr = StringCchPrintfW(
            szKey,
            ARRAYSIZE(szKey),
            L"SOFTWARE\\Classes\\TypeLib\\%s\\1.0\\0\\win32",
            szLIBID
            );
    }

    if( S_OK == hr )
    {
        lResult = RegCreateKeyExW(
            HKEY_LOCAL_MACHINE,
            szKey,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_SET_VALUE,
            NULL,
            &hKey,
            NULL
            );
        hr = HRESULT_FROM_WIN32( lResult );
    }

    if( S_OK == hr )
    {
        lResult = RegSetValueExW(
            hKey,
            NULL,
            0,
            REG_SZ,
            (BYTE*)szFilename,
            (static_cast<DWORD>(wcslen(szFilename))+1)*sizeof(WCHAR)
            );
        hr = HRESULT_FROM_WIN32( lResult );
    }
    RegCloseKey( hKey );
    hKey = NULL;

    if( S_OK == hr )
    {
        hr = StringCchPrintfW(
            szKey,
            ARRAYSIZE(szKey),
            L"SOFTWARE\\Classes\\TypeLib\\%s\\1.0\\FLAGS",
            szLIBID
            );
    }

    if( S_OK == hr )
    {
        lResult = RegCreateKeyExW(
            HKEY_LOCAL_MACHINE,
            szKey,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_SET_VALUE,
            NULL,
            &hKey,
            NULL
            );
        hr = HRESULT_FROM_WIN32( lResult );
    }

    if( S_OK == hr )
    {
        lResult = RegSetValueExW(
            hKey,
            NULL,
            0,
            REG_SZ,
            (BYTE*)L"0",
            (static_cast<DWORD>(wcslen(L"0"))+1)*sizeof(WCHAR)
            );
        hr = HRESULT_FROM_WIN32( lResult );
    }
    RegCloseKey( hKey );
    hKey = NULL;

    if( S_OK == hr )
    {
        hr = StringCchPrintfW(
            szKey,
            ARRAYSIZE(szKey),
            L"SOFTWARE\\Classes\\TypeLib\\%s\\1.0\\HELPDIR",
            szLIBID
            );
    }

    if( S_OK == hr )
    {
        lResult = RegCreateKeyExW(
            HKEY_LOCAL_MACHINE,
            szKey,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_SET_VALUE,
            NULL,
            &hKey,
            NULL
            );
        hr = HRESULT_FROM_WIN32( lResult );
    }

    if( S_OK == hr )
    {
        lResult = GetCurrentDirectoryW(
            ARRAYSIZE(szValue),
            szValue
            );
        if( 0 == lResult )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
        }
    }

    if( S_OK == hr )
    {
        lResult = RegSetValueExW(
            hKey,
            NULL,
            0,
            REG_SZ,
            (BYTE*)szValue,
            (static_cast<DWORD>(wcslen(szValue))+1)*sizeof(WCHAR)
            );
        hr = HRESULT_FROM_WIN32( lResult );
    }
    RegCloseKey( hKey );
    hKey = NULL;

    return hr;
}// DllRegisterServer


//------------------------------------------------------------------------------
// DllUnregisterServer
//------------------------------------------------------------------------------
STDAPI DllUnregisterServer()
{
    HKEY    hKey                        = NULL;
    HRESULT hr                          = S_OK;
    LONG    lResult                     = ERROR_SUCCESS;
    WCHAR   szValue[MAX_PATH]           = {0};
    WCHAR   szCLSID[OLEGUID_LEN_CCH]    = {0};
    WCHAR   szLIBID[OLEGUID_LEN_CCH]    = {0};

    //
    // Register the COM object in the registry
    //
    if( 0 == StringFromGUID2(
        CLSID_SimpleThermostatDevice,
        szCLSID, ARRAYSIZE(szCLSID) )
        )
    {  
        hr = HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER );
    }

    if( S_OK == hr )
    {
        lResult = RegOpenKeyExW(
            HKEY_CLASSES_ROOT,
            NULL,
            0,
            KEY_SET_VALUE,
            &hKey
            );
        hr = HRESULT_FROM_WIN32( lResult );
    }

    if( S_OK == hr )
    {
        hr = StringCchPrintfW(
            szValue,
            ARRAYSIZE(szValue),
            L"CLSID\\%s\\InProcServer32",
            szCLSID
            );
    }

    if( S_OK == hr )
    {
        lResult = RegDeleteKeyW( hKey, szValue );
        hr = HRESULT_FROM_WIN32( lResult );
    }

    if( S_OK == hr )
    {
        hr = StringCchPrintfW(
            szValue,
            ARRAYSIZE(szValue),
            L"CLSID\\%s\\TypeLib",
            szCLSID
            );
    }

    if( S_OK == hr )
    {
        lResult = RegDeleteKeyW( hKey, szValue );
        hr = HRESULT_FROM_WIN32( lResult );
    }

    if( S_OK == hr )
    {
        hr = StringCchPrintfW(
            szValue,
            ARRAYSIZE(szValue),
            L"CLSID\\%s",
            szCLSID
            );
    }

    if( S_OK == hr )
    {
        lResult = RegDeleteKeyW( hKey, szValue );
        hr = HRESULT_FROM_WIN32( lResult );
    }

    RegCloseKey( hKey );
    hKey = NULL;

    if( S_OK == hr )
    {
        lResult = RegOpenKeyExW(
            HKEY_LOCAL_MACHINE,
            NULL,
            0,
            KEY_SET_VALUE,
            &hKey
            );
        hr = HRESULT_FROM_WIN32( lResult );
    }

    if( S_OK == hr &&
        0 == StringFromGUID2( LIBID_SimpleThermostatDeviceLib, szLIBID, ARRAYSIZE(szLIBID) ) )
    {  
        hr = HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER );
    }

    if( S_OK == hr )
    {
        hr = StringCchPrintfW(
            szValue,
            ARRAYSIZE(szValue),
            L"SOFTWARE\\Classes\\TypeLib\\%s\\1.0\\0\\win32",
            szLIBID
            );
    }

    if( S_OK == hr )
    {
        lResult = RegDeleteKeyW( hKey, szValue );
        hr = HRESULT_FROM_WIN32( lResult );
    }

    if( S_OK == hr )
    {
        hr = StringCchPrintfW(
            szValue,
            ARRAYSIZE(szValue),
            L"SOFTWARE\\Classes\\TypeLib\\%s\\1.0\\0",
            szLIBID
            );
    }

    if( S_OK == hr )
    {
        lResult = RegDeleteKeyW( hKey, szValue );
        hr = HRESULT_FROM_WIN32( lResult );
    }

    if( S_OK == hr )
    {
        hr = StringCchPrintfW(
            szValue,
            ARRAYSIZE(szValue),
            L"SOFTWARE\\Classes\\TypeLib\\%s\\1.0\\FLAGS",
            szLIBID
            );
    }

    if( S_OK == hr )
    {
        lResult = RegDeleteKeyW( hKey, szValue );
        hr = HRESULT_FROM_WIN32( lResult );
    }

    if( S_OK == hr )
    {
        hr = StringCchPrintfW(
            szValue,
            ARRAYSIZE(szValue),
            L"SOFTWARE\\Classes\\TypeLib\\%s\\1.0\\HELPDIR",
            szLIBID
            );
    }

    if( S_OK == hr )
    {
        lResult = RegDeleteKeyW( hKey, szValue );
        hr = HRESULT_FROM_WIN32( lResult );
    }

    if( S_OK == hr )
    {
        hr = StringCchPrintfW(
            szValue,
            ARRAYSIZE(szValue),
            L"SOFTWARE\\Classes\\TypeLib\\%s\\1.0",
            szLIBID
            );
    }

    if( S_OK == hr )
    {
        lResult = RegDeleteKeyW( hKey, szValue );
        hr = HRESULT_FROM_WIN32( lResult );
    }

    if( S_OK == hr )
    {
        hr = StringCchPrintfW(
            szValue,
            ARRAYSIZE(szValue),
            L"SOFTWARE\\Classes\\TypeLib\\%s",
            szLIBID
            );
    }

    if( S_OK == hr )
    {
        lResult = RegDeleteKeyW( hKey, szValue );
        hr = HRESULT_FROM_WIN32( lResult );
    }

    RegCloseKey( hKey );
    hKey = NULL;

    return hr;
}// DllUnregisterServer


VOID DllIncLockCount()
{
    InterlockedIncrement( &g_cLockCount );
}// DllIncLockCount


VOID DllDecLockCount()
{
    InterlockedDecrement( &g_cLockCount );
}// DllDecLockCount