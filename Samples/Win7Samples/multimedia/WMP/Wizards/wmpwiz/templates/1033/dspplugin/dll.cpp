// [!output root]dll.cpp : Implementation of DLL Exports.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>

#include "[!output root].h"
#include "[!output Root]PropPage.h"

#include <crtdbg.h>     // Debug header
#include <uuids.h>      // DirectX SDK media types and subtyes
#include <dmoreg.h>     // DirectX SDK registration

CComModule _Module;
BOOL IsVistaOrLater(); // forward declaration

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_[!output Safe_root], C[!output Safe_root])
OBJECT_ENTRY(CLSID_[!output Safe_root]PropPage, C[!output Safe_root]PropPage)
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point
/////////////////////////////////////////////////////////////////////////////

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        _Module.Term();
    }

    return TRUE;    // ok
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE
/////////////////////////////////////////////////////////////////////////////

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type
/////////////////////////////////////////////////////////////////////////////

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry
/////////////////////////////////////////////////////////////////////////////

STDAPI DllRegisterServer(void)
{
    CComPtr<IWMPMediaPluginRegistrar> spRegistrar;
    HRESULT hr;

    // Create the registration object
    hr = spRegistrar.CoCreateInstance(CLSID_WMPMediaPluginRegistrar, NULL, CLSCTX_INPROC_SERVER);
    if (FAILED(hr))
    {
        return hr;
    }
    
    // Load friendly name and description strings
    CComBSTR    bstrFriendlyName;
    CComBSTR    bstrDescription;

    bstrFriendlyName.LoadString(IDS_FRIENDLYNAME);
    bstrDescription.LoadString(IDS_DESCRIPTION);
[!if AUDIO]

    // Describe the type of data handled by the plug-in
    DMO_PARTIAL_MEDIATYPE mt = { 0 };
    mt.type = MEDIATYPE_Audio;
    mt.subtype = MEDIASUBTYPE_PCM;

    // Register the plug-in with WMP (for legacy pipeline support)
    hr = spRegistrar->WMPRegisterPlayerPlugin(
                    bstrFriendlyName,   // friendly name (for menus, etc)
                    bstrDescription,    // description (for Tools->Options->Plug-ins)
                    NULL,               // path to app that uninstalls the plug-in
                    1,                  // DirectShow priority for this plug-in
                    WMP_PLUGINTYPE_DSP, // Plug-in type
                    CLSID_[!output Safe_root],// Class ID of plug-in
                    1,                  // No. media types supported by plug-in
                    &mt);               // Array of media types supported by plug-in

    // Also register for out-of-proc playback in the MF pipeline
    // We'll only do this on Windows Vista or later operating systems because
    // WMP 11 and Vista are required at a minimum.
    if (SUCCEEDED(hr) && 
        TRUE == IsVistaOrLater())
    {
        hr = spRegistrar->WMPRegisterPlayerPlugin(
                        bstrFriendlyName,   // friendly name (for menus, etc)
                        bstrDescription,    // description (for Tools->Options->Plug-ins)
                        NULL,               // path to app that uninstalls the plug-in
                        1,                  // DirectShow priority for this plug-in
                        WMP_PLUGINTYPE_DSP_OUTOFPROC, // Plug-in type
                        CLSID_[!output Safe_root],// Class ID of plug-in
                        1,                  // No. media types supported by plug-in
                        &mt);               // Array of media types supported by plug-in
    }
[!endif]
[!if VIDEO]

    // Describe the type of data handled by the plug-in
    DMO_PARTIAL_MEDIATYPE* mt = new DMO_PARTIAL_MEDIATYPE[C[!output Safe_root]::k_dwValidSubtypesCount];
    if (NULL == mt)
    {
        return E_OUTOFMEMORY;
    }

    ::ZeroMemory( mt, ( sizeof( DMO_PARTIAL_MEDIATYPE ) * C[!output Safe_root]::k_dwValidSubtypesCount));

    for( DWORD i = 0; i < C[!output Safe_root]::k_dwValidSubtypesCount; i++)
    {
        mt[i].type = MEDIATYPE_Video;
        mt[i].subtype = *C[!output Safe_root]::k_guidValidSubtypes[i];
    }

    // Register the plug-in with WMP
    hr = spRegistrar->WMPRegisterPlayerPlugin(
                    bstrFriendlyName,   // friendly name (for menus, etc)
                    bstrDescription,    // description (for Tools->Options->Plug-ins)
                    NULL,               // path to app that uninstalls the plug-in
                    1,                  // DirectShow priority for this plug-in
                    WMP_PLUGINTYPE_DSP, // Plug-in type
                    CLSID_[!output Safe_root], // Class ID of plug-in
                    C[!output Safe_root]::k_dwValidSubtypesCount, // No. media types supported by plug-in
                    mt);                // Array of media types supported by plug-in

    // Also register for out-of-proc playback in the MF pipeline
    // We'll only do this on Windows Vista or later operating systems because
    // WMP 11 and Vista are required at a minimum.
    if (SUCCEEDED(hr) && 
        TRUE == IsVistaOrLater())
    {
        hr = spRegistrar->WMPRegisterPlayerPlugin(
                        bstrFriendlyName,   // friendly name (for menus, etc)
                        bstrDescription,    // description (for Tools->Options->Plug-ins)
                        NULL,               // path to app that uninstalls the plug-in
                        1,                  // DirectShow priority for this plug-in
                        WMP_PLUGINTYPE_DSP_OUTOFPROC, // Plug-in type
                        CLSID_[!output Safe_root],// Class ID of plug-in
                        C[!output Safe_root]::k_dwValidSubtypesCount, // No. media types supported by plug-in
                        mt);               // Array of media types supported by plug-in
    }

    delete [] mt;
[!endif]

    if (FAILED(hr))
    {
        return hr;
    }

    // registers object, typelib and all interfaces in typelib
    return _Module.RegisterServer();
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry
/////////////////////////////////////////////////////////////////////////////

STDAPI DllUnregisterServer(void)

{
    CComPtr<IWMPMediaPluginRegistrar> spRegistrar;
    HRESULT hr;

    // Create the registration object
    hr = spRegistrar.CoCreateInstance(CLSID_WMPMediaPluginRegistrar, NULL, CLSCTX_INPROC_SERVER);
    if (FAILED(hr))
    {
        return hr;
    }

    // Tell WMP to remove this plug-in (for legacy pipeline support)
    hr = spRegistrar->WMPUnRegisterPlayerPlugin(WMP_PLUGINTYPE_DSP, CLSID_[!output Safe_root]);
 
    if(TRUE == IsVistaOrLater())
    {
        // Also unregister from the MF pipeline
        hr = spRegistrar->WMPUnRegisterPlayerPlugin(WMP_PLUGINTYPE_DSP_OUTOFPROC, CLSID_[!output Safe_root]);
    }

    return _Module.UnregisterServer();
}

////////////////////////////////////////////////////////////////////////
// IsVistaOrLater - Returns a BOOLEAN indicating whether the operating
//                   system is Windows Vista or newer.
////////////////////////////////////////////////////////////////////////

BOOL IsVistaOrLater()
{
    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    BOOL bResult = FALSE;
    BOOL bIsVistaOrLater = FALSE;

    // Check for Windows Vista or later.
    bResult = GetVersionEx(&osvi);
    if(TRUE == bResult)
    {
        bIsVistaOrLater = (osvi.dwMajorVersion >= 6);
    }

    return bIsVistaOrLater;
}

