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
#include "common.h"
#include "DevicePropertyPage.h"
#include "CDevicePropertyPageFactory.h"

// Define the unique COM id for the COM object
class __declspec(uuid("f8ee49d1-02e4-472f-b55d-d3fc30f70de6")) DevicePropertyPage;

const ULONG OLEGUID_LEN_CCH = 40;
const WCHAR OBJECT_NAME[] = L"Device Property Sheet";
const WCHAR THREADING_MODEL[] = L"Apartment";
HINSTANCE g_hInstance = NULL;
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
    UNREFERENCED_PARAMETER( lpReserved );

    BOOL bRet = TRUE;

    if( DLL_PROCESS_ATTACH == dwReason )
    {
        bRet = DisableThreadLibraryCalls( hInstance );
        g_hInstance = hInstance;
    }
    else if( DLL_PROCESS_DETACH == dwReason )
    {
        g_hInstance = NULL;
    }

    return bRet;
}// DllMain


//------------------------------------------------------------------------------
// DllGetClassObject
//------------------------------------------------------------------------------
STDAPI DllGetClassObject(
    __in REFCLSID rclsid,
    __in REFIID riid,
    __deref_out LPVOID* ppv
    )
{
    HRESULT hr = E_FAIL;
    CDevicePropertyPageFactory* pCDevicePropertyPageFactory = NULL; 

    if( rclsid != __uuidof(DevicePropertyPage) )
    {
        return CLASS_E_CLASSNOTAVAILABLE;
    }

    pCDevicePropertyPageFactory = new CDevicePropertyPageFactory();
    if( NULL == pCDevicePropertyPageFactory )
    {
        return E_OUTOFMEMORY;
    }
    
    hr = pCDevicePropertyPageFactory->QueryInterface( riid, ppv );
    pCDevicePropertyPageFactory->Release();

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
    HRESULT hr                          = S_OK;
    LONG    lResult                     = ERROR_SUCCESS;
    WCHAR   szFilename[MAX_PATH]        = {0};
    WCHAR   szKey[MAX_PATH]             = {0};
    WCHAR   szCLSID[OLEGUID_LEN_CCH]    = {0};

    //
    // Grab the fully qualified path to this dll
    //
    if( 0 == GetModuleFileNameW( g_hInstance, szFilename, ARRAYSIZE(szFilename) ) )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
    }

    //
    // Register the COM object in the registry
    //
    if( S_OK == hr &&
        0 == StringFromGUID2( __uuidof(DevicePropertyPage), szCLSID, ARRAYSIZE(szCLSID) ) )
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
    WCHAR   szKey[MAX_PATH]             = {0};
    WCHAR   szCLSID[OLEGUID_LEN_CCH]    = {0};

    //
    // Register the COM object in the registry
    //
    if( 0 == StringFromGUID2( __uuidof(DevicePropertyPage), szCLSID, ARRAYSIZE(szCLSID) ) )
    {  
        hr = HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER );
    }

    if( S_OK == hr )
    {
        hr = StringCchPrintfW( szKey, ARRAYSIZE(szKey), L"%s\\InProcServer32", szCLSID );
    }

    if( S_OK == hr )
    {
        lResult = RegOpenKeyExW(
            HKEY_CLASSES_ROOT,
            L"CLSID",
            0,
            KEY_SET_VALUE,
            &hKey
            );
        hr = HRESULT_FROM_WIN32( lResult );
    }

    if( S_OK == hr )
    {
        lResult = RegDeleteKeyW( hKey, szKey );
        hr = HRESULT_FROM_WIN32( lResult );
    }

    if( S_OK == hr )
    {
        lResult = RegDeleteKeyW( hKey, szCLSID );
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

