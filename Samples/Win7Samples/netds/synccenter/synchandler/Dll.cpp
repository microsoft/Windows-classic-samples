//////////////////////////////////////////////////////////////////////////////
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Module Name:
//      Dll.cpp
//
//  Abstract:
//      Defines the entry point for the DLL application.
//
//////////////////////////////////////////////////////////////////////////////

#include "Pch.h"
#include "ClassFactory.h"

#include <advpub.h>         // For RegInstall

//////////////////////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////////////////////

LONG        g_cRefThisDll = 0;          // Reference count of this DLL.
HINSTANCE   g_hmodThisDll = NULL;       // Handle to this DLL.

//----------------------------------------------------------------------------
//
//  Description:
//      Main entry point for the DLL.
//
//  Parameters:
//      hInstance
//          Module instance for the DLL.
//
//      dwReason
//          Reason the DLL was called:
//              DLL_PROCESS_ATTACH - The DLL was loaded by a process.
//              DLL_PROCESS_DETACH - The DLL was unloaded by a process.
//
//      pReserved
//          Unused.
//
//  Return Values:
//      TRUE - Initialization was successful.
//
//----------------------------------------------------------------------------
STDAPI_(int) DllMain(__in HINSTANCE hInstance, __in DWORD dwReason, __in void *pReserved)
{
    UNREFERENCED_PARAMETER(pReserved);

    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hmodThisDll = hInstance;
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        assert(g_cRefThisDll == 0);
    }

    return TRUE;

} //*** DllMain

//----------------------------------------------------------------------------
//
//  Description:
//      Register the components implemented in this DLL.
//
//  Return Values:
//      S_OK, E_FAIL
//
//----------------------------------------------------------------------------
STDAPI DllRegisterServer(void)
{
// RegInstallW lives in advpack.lib, which is currently not included in the
// Public SDK, though being documented on MSDN.  Instead of linking in the
// library, we can delay load it using LoadLibrary
#if 0
    return RegInstallW(g_hmodThisDll, L"RegDll", NULL);
#else
    typedef HRESULT (WINAPI *PFNREGINSTALLW)(HMODULE, LPCWSTR, LPCSTRTABLE);
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    HMODULE hmodInstall = LoadLibraryW(L"advpack.dll");
    if (hmodInstall != NULL)
    {
        PFNREGINSTALLW pfnInstall = (PFNREGINSTALLW)GetProcAddress(hmodInstall, "RegInstallW");
        if (pfnInstall != NULL)
        {
            hr = ((*pfnInstall)(g_hmodThisDll, L"RegDll", NULL));
        }
        FreeLibrary(hmodInstall);
    }
    return hr;
#endif

} //*** DllRegisterServer

//----------------------------------------------------------------------------
//
//  Description:
//      Unregister the components implemented in this DLL.
//
//  Return Values:
//      S_OK, E_FAIL
//
//----------------------------------------------------------------------------
STDAPI DllUnregisterServer(void)
{
#if 0
    return RegInstallW(g_hmodThisDll, L"UnregDll", NULL);
#else
    typedef HRESULT (WINAPI *PFNREGINSTALLW)(HMODULE, LPCWSTR, LPCSTRTABLE);
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    HMODULE hmodInstall = LoadLibraryW(L"advpack.dll");
    if (hmodInstall != NULL)
    {
        PFNREGINSTALLW pfnInstall = (PFNREGINSTALLW)GetProcAddress(hmodInstall, "RegInstallW");
        if (pfnInstall != NULL)
        {
            hr = ((*pfnInstall)(g_hmodThisDll, L"UnregDll", NULL));
        }
        FreeLibrary(hmodInstall);
    }
    return hr;
#endif

} //*** DllUnregisterServer

//----------------------------------------------------------------------------
//
//  Description:
//      Indicate whether it is safe to unload this DLL or not.
//
//  Return Values:
//      S_OK    - It is safe to unload this DLL.
//      S_FALSE - It is not safe to unload this DLL - a component is in use.
//
//----------------------------------------------------------------------------
STDAPI DllCanUnloadNow(void)
{
    return ((g_cRefThisDll == 0) ? S_OK : S_FALSE);
} //*** DllCanUnloadNow

//----------------------------------------------------------------------------
//
//  Description:
//      Get a pointer to an object implemented in this DLL.
//
//  Parameters:
//      rclsid          - CLSID of the object to get.
//      riid            - Interface ID to get.
//      ppv             - Interface pointer returned from the object.
//
//  Return Values:
//      S_OK            - Object retrieved successfully.
//      Other HRESULTs  - Operation failed.
//
//----------------------------------------------------------------------------
STDAPI DllGetClassObject(__in REFCLSID rclsid, __in REFIID riid, __deref_out void **ppv)
{
    return CClassFactory_CreateInstance(rclsid, riid, ppv);
} //*** DllGetClassObject

//----------------------------------------------------------------------------
//
//  Description:
//      Increment the reference count on the DLL to prevent graceful unloads.
//
//----------------------------------------------------------------------------
STDAPI_(void) DllAddRef()
{
    InterlockedIncrement(&g_cRefThisDll);
} //*** DllAddRef

//----------------------------------------------------------------------------
//
//  Description:
//      Decrement the reference count on the DLL.  If the count drops to
//      zero, a graceful unload (by calling DllCanUnloadNow) will be allowed.
//
//----------------------------------------------------------------------------
STDAPI_(void) DllRelease()
{
    InterlockedDecrement(&g_cRefThisDll);
} //*** DllRelease
