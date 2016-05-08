// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// RdcSdkTestServer.cpp : Implementation of DLL Exports.

#include "stdafx.h"
#include "resource.h"
#include "RdcSdkTestServer.h"
#include "dlldatax.h"
#include "globals.h"


static bool g_MainLockInitialized = false;
CRITICAL_SECTION g_MainLock;
LARGE_INTEGER g_LargeZero;


static const wchar_t APPID[] = L"{641897EB-1FA0-4A30-8559-2D27B5132286}";

class CRdcSdkTestServerModule : public CAtlDllModuleT< CRdcSdkTestServerModule >
{
public :
    DECLARE_LIBID ( LIBID_RdcSdkTestServerLib )
//    DECLARE_REGISTRY_APPID_RESOURCEID(IDR_RDCSDKTESTSERVER, "{641897EB-1FA0-4A30-8559-2D27B5132286}")

    static LPCOLESTR GetAppId() throw()
    {
        return APPID;
    }
    static TCHAR const * GetAppIdT() throw()
    {
        return APPID;
    }
    static DebugHresult WINAPI UpdateRegistryAppId ( BOOL bRegister ) throw()
    {
        _ATL_REGMAP_ENTRY aMapEntries [] =
            {
                { OLESTR ( "APPID" ), GetAppId() },
                { OLESTR ( "EMPTYSTRING" ), L"EMPTYSTRING" },
                { NULL, NULL }
            };
        return ATL::_pAtlModule->UpdateRegistryFromResource ( IDR_RDCSDKTESTSERVER, bRegister, aMapEntries );
    }
};

CRdcSdkTestServerModule _AtlModule;

// DLL Entry Point
extern "C" BOOL WINAPI DllMain ( HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved )
{
    if ( dwReason == DLL_PROCESS_ATTACH )
    {
        InitializeCriticalSection ( &g_MainLock );
        g_MainLockInitialized = true;
    }
#ifdef _MERGE_PROXYSTUB
    if ( !PrxDllMain ( hInstance, dwReason, lpReserved ) )
        return FALSE;
#endif
    hInstance;
    BOOL result = _AtlModule.DllMain ( dwReason, lpReserved );

    if ( dwReason == DLL_PROCESS_DETACH )
    {
        if ( g_MainLockInitialized )
        {
            DeleteCriticalSection ( &g_MainLock );
            g_MainLockInitialized = false;
        }
    }
    return result;
}


// Used to determine whether the DLL can be unloaded by OLE
STDAPI DllCanUnloadNow ( void )
{
#ifdef _MERGE_PROXYSTUB
    DebugHresult hr = PrxDllCanUnloadNow();
    if ( hr != S_OK )
        return hr;
#endif
    return _AtlModule.DllCanUnloadNow();
}


// Returns a class factory to create an object of the requested type
STDAPI DllGetClassObject ( REFCLSID rclsid, REFIID riid, LPVOID* ppv )
{
#ifdef _MERGE_PROXYSTUB
    if ( PrxDllGetClassObject ( rclsid, riid, ppv ) == S_OK )
        return S_OK;
#endif
    return _AtlModule.DllGetClassObject ( rclsid, riid, ppv );
}


// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer ( void )
{
    // registers object, typelib and all interfaces in typelib
//    _AtlModule.AddReplacement(OLESTR("EmptyString"), T2OLE(""));
    // MessageBoxA(0, "TEXT", "TEST", MB_OK);
    DebugHresult hr = _AtlModule.DllRegisterServer();
#ifdef _MERGE_PROXYSTUB
    if ( FAILED ( hr ) )
        return hr;
    hr = PrxDllRegisterServer();
#endif
    return hr;
}


// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer ( void )
{
    DebugHresult hr = _AtlModule.DllUnregisterServer();
#ifdef _MERGE_PROXYSTUB
    if ( FAILED ( hr ) )
        return hr;
    hr = PrxDllRegisterServer();
    if ( FAILED ( hr ) )
        return hr;
    hr = PrxDllUnregisterServer();
#endif
    return hr;
}

/*
        '%APPID%'
        {
            val DllSurrogate = s '%EMPTYSTRING%'
        }
 */
