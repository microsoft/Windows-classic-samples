//////////////////////////////////////////////////////////////////////////
//
// dllmain.cpp : Implements DLL exports and COM class factory
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Note: This source file implements the class factory for the sample
//       media source, plus the following DLL functions:
//       - DllMain
//       - DllCanUnloadNow
//       - DllRegisterServer
//       - DllUnregisterServer
//       - DllGetClassObject
//
//////////////////////////////////////////////////////////////////////////

#include "MPEG1Source.h"
#include "MPEG1ByteStreamHandler.h"

#include <assert.h>
#include <strsafe.h>

#include "ClassFactory.h"   // Implements IClassFactory
#include "registry.h"       // Helpers to register COM objects.

#include <initguid.h>

// {EFE6208A-0A2C-49fa-8A01-3768B559B6DA}
DEFINE_GUID(CLSID_MFSampleMPEG1ByteStreamHandler, 
0xefe6208a, 0xa2c, 0x49fa, 0x8a, 0x1, 0x37, 0x68, 0xb5, 0x59, 0xb6, 0xda);

HMODULE g_hModule;      // DLL module handle

// Defines the class factory lock variable:
DEFINE_CLASSFACTORY_SERVER_LOCK;

// g_ClassFactories: Array of class factory data.
// Defines a look-up table of CLSIDs and corresponding creation functions.

ClassFactoryData g_ClassFactories[] =
{
    {   &CLSID_MFSampleMPEG1ByteStreamHandler,      MPEG1ByteStreamHandler::CreateInstance }
};
      
DWORD g_numClassFactories = ARRAYSIZE(g_ClassFactories);

// Text strings

// Description string for the bytestream handler.
const TCHAR* sByteStreamHandlerDescription = TEXT("MPEG1 Source ByteStreamHandler");

// File extension for WAVE files.
const TCHAR* sFileExtension = TEXT(".mpg");

// Registry location for bytestream handlers.
const TCHAR* REGKEY_MF_BYTESTREAM_HANDLERS 
                = TEXT("Software\\Microsoft\\Windows Media Foundation\\ByteStreamHandlers");


// Forward declarations

// Functions to register and unregister the byte stream handler.

HRESULT RegisterByteStreamHandler(const GUID& guid, const TCHAR *sFileExtension, const TCHAR *sDescription);
HRESULT UnregisterByteStreamHandler(const GUID& guid, const TCHAR *sFileExtension);

// Misc Registry helpers
HRESULT SetKeyValue(HKEY hKey, const TCHAR *sName, const TCHAR *sValue);

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        g_hModule = (HMODULE)hModule;
        TRACE_INIT();
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
        TRACE_CLOSE();
        break;
    }
    return TRUE;
}

STDAPI DllCanUnloadNow()
{
    if (!ClassFactory::IsLocked())
    {
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}


STDAPI DllRegisterServer()
{
    HRESULT hr = S_OK;

    // Register the bytestream handler's CLSID as a COM object.
    CHECK_HR(hr = RegisterObject(
            g_hModule,                              // Module handle
            CLSID_MFSampleMPEG1ByteStreamHandler,   // CLSID 
            sByteStreamHandlerDescription,          // Description
            TEXT("Both")                            // Threading model
            ));

    // Register the bytestream handler as the handler for the MPEG-1 file extension.
    CHECK_HR(hr = RegisterByteStreamHandler(
        CLSID_MFSampleMPEG1ByteStreamHandler,       // CLSID 
        sFileExtension,                             // Supported file extension
        sByteStreamHandlerDescription               // Description
        ));

done:
    return hr;
}

STDAPI DllUnregisterServer()
{
    // Unregister the CLSIDs
    UnregisterObject(CLSID_MFSampleMPEG1ByteStreamHandler);

    // Unregister the bytestream handler for the file extension.
    UnregisterByteStreamHandler(CLSID_MFSampleMPEG1ByteStreamHandler, sFileExtension);

    return S_OK;
}

STDAPI DllGetClassObject(REFCLSID clsid, REFIID riid, void** ppv)
{
    ClassFactory *pFactory = NULL;

    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE; // Default to failure

    // Find an entry in our look-up table for the specified CLSID.
    for (DWORD index = 0; index < g_numClassFactories; index++)
    {
        if (*g_ClassFactories[index].pclsid == clsid)
        {
            // Found an entry. Create a new class factory object.
            pFactory = new ClassFactory(g_ClassFactories[index].pfnCreate);
            if (pFactory)
            {
                hr = S_OK;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
            break;
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = pFactory->QueryInterface(riid, ppv);
    }
    SAFE_RELEASE(pFactory);

    return hr;
}

///////////////////////////////////////////////////////////////////////
// Name: CreateRegistryKey
// Desc: Creates a new registry key. (Thin wrapper just to encapsulate
//       all of the default options.)
///////////////////////////////////////////////////////////////////////

HRESULT CreateRegistryKey(HKEY hKey, LPCTSTR subkey, HKEY *phKey)
{
    assert(phKey != NULL);

    LONG lreturn = RegCreateKeyEx(
        hKey,                 // parent key
        subkey,               // name of subkey
        0,                    // reserved
        NULL,                 // class string (can be NULL)
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        NULL,                 // security attributes
        phKey,
        NULL                  // receives the "disposition" (is it a new or existing key)
        );

    return HRESULT_FROM_WIN32(lreturn);
}


///////////////////////////////////////////////////////////////////////
// Name: RegisterByteStreamHandler
// Desc: Register a bytestream handler for the Media Foundation
//       source resolver.
//
// guid:            CLSID of the bytestream handler.
// sFileExtension:  File extension.
// sDescription:    Description.
//
// Note: sFileExtension can also be a MIME type although that is not
//       illustrated in this sample.
///////////////////////////////////////////////////////////////////////

HRESULT RegisterByteStreamHandler(const GUID& guid, const TCHAR *sFileExtension, const TCHAR *sDescription)
{
    HRESULT hr = S_OK;

    // Open HKCU/<byte stream handlers>/<file extension>
    
    // Create {clsid} = <description> key

    HKEY    hKey = NULL;
    HKEY    hSubKey = NULL;

    OLECHAR szCLSID[CHARS_IN_GUID];

    size_t  cchDescription = 0;
    hr = StringCchLength(sDescription, STRSAFE_MAX_CCH, &cchDescription);
    
    if (SUCCEEDED(hr))
    {
        hr = StringFromGUID2(guid, szCLSID, CHARS_IN_GUID);
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateRegistryKey(HKEY_LOCAL_MACHINE, REGKEY_MF_BYTESTREAM_HANDLERS, &hKey);
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateRegistryKey(hKey, sFileExtension, &hSubKey);
    }

    if (SUCCEEDED(hr))
    {
        hr = RegSetValueEx(
            hSubKey,
            szCLSID,
            0,
            REG_SZ,
            (BYTE*)sDescription,
            static_cast<DWORD>((cchDescription + 1) * sizeof(TCHAR))
            );
    }

    if (hSubKey != NULL)
    {
        RegCloseKey( hSubKey );
    }

    if (hKey != NULL)
    {
        RegCloseKey( hKey );
    }

    return hr;
}


HRESULT UnregisterByteStreamHandler(const GUID& guid, const TCHAR *sFileExtension)
{
    TCHAR szKey[MAX_PATH];
    OLECHAR szCLSID[CHARS_IN_GUID];

    DWORD result = 0;
    HRESULT hr = S_OK;

    // Create the subkey name.
    CHECK_HR(hr = StringCchPrintf(
        szKey, MAX_PATH, TEXT("%s\\%s"), REGKEY_MF_BYTESTREAM_HANDLERS, sFileExtension));

    // Create the CLSID name in canonical form.
    CHECK_HR(hr = StringFromGUID2(guid, szCLSID, CHARS_IN_GUID));

    // Delete the CLSID entry under the subkey. 
    // Note: There might be multiple entries for this file extension, so we should not delete 
    // the entire subkey, just the entry for this CLSID.
    result = RegDeleteKeyValue(HKEY_LOCAL_MACHINE, szKey, szCLSID);
    if (result != ERROR_SUCCESS)
    {
        CHECK_HR(hr = HRESULT_FROM_WIN32(result));
    }

done:
    return hr;
}

