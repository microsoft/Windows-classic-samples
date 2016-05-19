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

#include <windows.h>
#include <new>
#include <shlwapi.h>
#include <mfapi.h>

#include <initguid.h>
#include "WavSourceGuids.h"

HRESULT WavByteStreamHandler_CreateInstance(REFIID riid, void **ppv);

#define SZ_HANDLER_CLSID L"{0CB50FD9-BB56-4ea6-8D8E-CD02769521B0}"
#define SZ_HANDLER_NAME L"WAVE Source ByteStreamHandler"

#if NTDDI_VERSION < 0x06010000
#define SZ_FILE_EXT L".wav"
#else
#define SZ_FILE_EXT L".xyz"
#endif

// Handle to the DLL's module
HMODULE g_hModule = NULL;

// Module Ref count
long c_cRefModule = 0;

void DllAddRef()
{
    InterlockedIncrement(&c_cRefModule);
}

void DllRelease()
{
    InterlockedDecrement(&c_cRefModule);
}

class CClassFactory : public IClassFactory
{
public:
    CClassFactory(REFCLSID clsid) : m_cRef(1), m_clsid(clsid)
    {
        DllAddRef();
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void ** ppv)
    {
        static const QITAB qit[] = 
        {
            QITABENT(CClassFactory, IClassFactory),
            { 0 }
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&m_cRef);
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        long cRef = InterlockedDecrement(&m_cRef);
        if (cRef == 0)
        {
            delete this;
        }
        return cRef;
    }

    // IClassFactory
    IFACEMETHODIMP CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
    {
        *ppv = NULL;
        HRESULT hr;
        if (punkOuter)
        {
            hr = CLASS_E_NOAGGREGATION;
        }
        else
        {
            if (IsEqualCLSID(m_clsid, CLSID_MFSampleWavSourceByteStreamHandler))
            {
                hr = WavByteStreamHandler_CreateInstance(riid, ppv);
            }
            else
            {
                hr = CLASS_E_CLASSNOTAVAILABLE;
            }
        }
        return hr;
    }

    IFACEMETHODIMP LockServer(BOOL bLock) 
    { 
        if (bLock)
        {
            DllAddRef();
        }
        else
        {
            DllRelease();
        }
        return S_OK;
    }

private:
    ~CClassFactory()
    {
        DllRelease();
    }

    long m_cRef;
    CLSID m_clsid;
};

// Standard DLL functions
STDAPI_(BOOL) DllMain(HINSTANCE hInstance, DWORD dwReason, void *)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hModule = (HMODULE)hInstance;
        DisableThreadLibraryCalls(hInstance);
    }
    return TRUE;
}

STDAPI DllCanUnloadNow()
{
    return (c_cRefModule == 0) ? S_OK : S_FALSE;
}

STDAPI DllGetClassObject(REFCLSID clsid, REFIID riid, void **ppv)
{
    *ppv = NULL;
    CClassFactory *pClassFactory = new (std::nothrow) CClassFactory(clsid);
    HRESULT hr = pClassFactory ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        hr = pClassFactory->QueryInterface(riid, ppv);
        pClassFactory->Release();
    }
    return hr;
}

// A struct to hold the information required for a registry entry
struct REGISTRY_ENTRY
{
    HKEY   hkeyRoot;
    PCWSTR pszKeyName;
    PCWSTR pszValueName;
    PCWSTR pszData;
};

#include <sfc.h>
#pragma comment(lib, "sfc")

// Creates a registry key (if needed) and sets the default value of the key
HRESULT CreateRegKeyAndSetValue(const REGISTRY_ENTRY *pRegistryEntry)
{
    HRESULT hr;
    HKEY hKey;

    LONG lRet = RegCreateKeyExW(pRegistryEntry->hkeyRoot, pRegistryEntry->pszKeyName, 
                                0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
    if (lRet != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(lRet);
    }
    else
    {
        // Registry keys for Media Foundation byte-stream handlers
        // might be protected by Windows Resource Protection (WRP), 
        // which might cause RegSetValueEx to fail silently.

        BOOL bIsKeyProtected = SfcIsKeyProtected(
            pRegistryEntry->hkeyRoot,
            pRegistryEntry->pszKeyName,
            0
            );

        if (bIsKeyProtected)
        {
            RegCloseKey(hKey);

            return E_ACCESSDENIED;
        }


        lRet = RegSetValueEx(hKey, pRegistryEntry->pszValueName, 0, REG_SZ,
                            (LPBYTE) pRegistryEntry->pszData,
                            ((DWORD) wcslen(pRegistryEntry->pszData) + 1) * sizeof(WCHAR));

        hr = HRESULT_FROM_WIN32(lRet);

        RegCloseKey(hKey);
    }

    return hr;
}


HRESULT DeleteRegKey(const REGISTRY_ENTRY *pRegistryEntry)
{
    HRESULT hr = S_OK;

    // Delete the registry entry
    DWORD dwError = RegDeleteTree(pRegistryEntry->hkeyRoot, pRegistryEntry->pszKeyName);
    if (ERROR_FILE_NOT_FOUND == dwError)
    {
        // The registry entry has already been deleted.
        hr = S_OK;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(dwError);
    }
    return hr;    
}

// Registers this COM server
STDAPI DllRegisterServer()
{
    HRESULT hr = S_OK;
 
    WCHAR szModuleName[MAX_PATH];

    // List of registry entries to create.
    const REGISTRY_ENTRY rgRegistryEntries[] = 
    {
        // RootKey             KeyName                                                              ValueName           Data
        {HKEY_LOCAL_MACHINE,   L"Software\\Classes\\CLSID\\" SZ_HANDLER_CLSID,                      NULL,               SZ_HANDLER_NAME},
        {HKEY_LOCAL_MACHINE,   L"Software\\Classes\\CLSID\\" SZ_HANDLER_CLSID L"\\InProcServer32",  NULL,               szModuleName},
        {HKEY_LOCAL_MACHINE,   L"Software\\Classes\\CLSID\\" SZ_HANDLER_CLSID L"\\InProcServer32",  L"ThreadingModel",  L"Both"},
        {HKEY_LOCAL_MACHINE,   L"Software\\Microsoft\\Windows Media Foundation\\ByteStreamHandlers\\" SZ_FILE_EXT,  SZ_HANDLER_CLSID,  SZ_HANDLER_NAME},
    };


    if (!GetModuleFileNameW(g_hModule, szModuleName, ARRAYSIZE(szModuleName)))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        for (int i = 0; i < ARRAYSIZE(rgRegistryEntries) && SUCCEEDED(hr); i++)
        {
            hr = CreateRegKeyAndSetValue(&rgRegistryEntries[i]);

            if (FAILED(hr))
            {
                break;
            }
        }
    }

    return hr;
}


// Unregisters this COM server
STDAPI DllUnregisterServer()
{
    HRESULT hr = S_OK;

    // List of registry entries to create.
    const REGISTRY_ENTRY rgRegistryEntries[] = 
    {
        // RootKey             KeyName                                                              ValueName           Data
        {HKEY_LOCAL_MACHINE,   L"Software\\Classes\\CLSID\\" SZ_HANDLER_CLSID,                      NULL,               SZ_HANDLER_NAME},
        {HKEY_LOCAL_MACHINE,   L"Software\\Microsoft\\Windows Media Foundation\\ByteStreamHandlers\\" SZ_FILE_EXT,  SZ_HANDLER_CLSID,  SZ_HANDLER_NAME},
    };

    for (int i = 0; i < ARRAYSIZE(rgRegistryEntries) && SUCCEEDED(hr); i++)
    {
        hr = DeleteRegKey(&rgRegistryEntries[i]);

        if (FAILED(hr))
        {
            break;
        }
    }

    return hr;
}
