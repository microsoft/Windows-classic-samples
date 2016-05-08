// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <windows.h>
#include <new>
#include <shlwapi.h>
#include <mfapi.h>
#include <uuids.h>      // MPEG-1 subtypes
#include <initguid.h>

#define SZ_DECODER_CLSID L"{df20ddfa-0d19-463a-ab46-e5d8ef6efd69}"

const WCHAR SZ_DECODER_NAME[] = L"Sample MPEG-1 Decoder";

// {df20ddfa-0d19-463a-ab46-e5d8ef6efd69}
DEFINE_GUID(CLSID_MPEG1SampleDecoder,
0xdf20ddfa, 0x0d19, 0x463a, 0xab, 0x46, 0xe5, 0xd8, 0xef, 0x6e, 0xfd, 0x69);

HRESULT Decoder_CreateInstance(REFIID riid, void **ppv);

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
            if (IsEqualCLSID(m_clsid, CLSID_MPEG1SampleDecoder))
            {
                hr = Decoder_CreateInstance(riid, ppv);
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

// Creates a registry key (if needed) and sets the default value of the key
HRESULT CreateRegKeyAndSetValue(const REGISTRY_ENTRY *pRegistryEntry)
{
    HRESULT hr;
    HKEY hKey;

    LONG lRet = RegCreateKeyExW(pRegistryEntry->hkeyRoot, pRegistryEntry->pszKeyName, 
                                0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
    if (lRet != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(lRet);
    }
    else
    {
        lRet = RegSetValueExW(hKey, pRegistryEntry->pszValueName, 0, REG_SZ,
                            (LPBYTE) pRegistryEntry->pszData,
                            ((DWORD) wcslen(pRegistryEntry->pszData) + 1) * sizeof(WCHAR));

        hr = HRESULT_FROM_WIN32(lRet);

        RegCloseKey(hKey);
    }

    return hr;
}

// Registers this COM server
STDAPI DllRegisterServer()
{
    HRESULT hr;
 
    WCHAR szModuleName[MAX_PATH];

    MFT_REGISTER_TYPE_INFO aDecoderInputTypes[] = 
    {
        { MFMediaType_Video, MEDIASUBTYPE_MPEG1Payload },
    };

    MFT_REGISTER_TYPE_INFO aDecoderOutputTypes[] = 
    {
        { MFMediaType_Video, MFVideoFormat_RGB32 }
    };
    

    if (!GetModuleFileNameW(g_hModule, szModuleName, ARRAYSIZE(szModuleName)))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        // List of registry entries we want to create

        const REGISTRY_ENTRY rgRegistryEntries[] = 
        {
            // RootKey             KeyName                                                              ValueName           Data
            {HKEY_LOCAL_MACHINE,   L"Software\\Classes\\CLSID\\" SZ_DECODER_CLSID,                      NULL,               SZ_DECODER_NAME},
            {HKEY_LOCAL_MACHINE,   L"Software\\Classes\\CLSID\\" SZ_DECODER_CLSID L"\\InProcServer32",  NULL,               szModuleName},
            {HKEY_LOCAL_MACHINE,   L"Software\\Classes\\CLSID\\" SZ_DECODER_CLSID L"\\InProcServer32",  L"ThreadingModel",  L"Both"},
        };

        hr = S_OK;

        for (int i = 0; i < ARRAYSIZE(rgRegistryEntries) && SUCCEEDED(hr); i++)
        {
            hr = CreateRegKeyAndSetValue(&rgRegistryEntries[i]);

            if (FAILED(hr))
            {
                break;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        // Register the decoder for MFTEnum(Ex).
        hr = MFTRegister(
            CLSID_MPEG1SampleDecoder,                   // CLSID
            MFT_CATEGORY_VIDEO_DECODER,                 // Category
            const_cast<LPWSTR>(SZ_DECODER_NAME),        // Friendly name
            0,                                          // Flags
            ARRAYSIZE(aDecoderInputTypes),              // Number of input types
            aDecoderInputTypes,                         // Input types
            ARRAYSIZE(aDecoderOutputTypes),             // Number of output types
            aDecoderOutputTypes,                        // Output types
            NULL                                        // Attributes (optional)
            );
    }

    return hr;
}
 
// Unregisters this COM server
STDAPI DllUnregisterServer()
{
    HRESULT hr = S_OK;

    const PCWSTR rgpszKey = L"Software\\Classes\\CLSID\\" SZ_DECODER_CLSID;

    // Delete the registry entry
    DWORD dwError = RegDeleteTree(HKEY_LOCAL_MACHINE, rgpszKey);
    if (ERROR_FILE_NOT_FOUND == dwError)
    {
        // The registry entry has already been deleted.
        hr = S_OK;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(dwError);
    }

    if (SUCCEEDED(hr))
    {
        hr = MFTUnregister(CLSID_MPEG1SampleDecoder);
    }

    return hr;
}
