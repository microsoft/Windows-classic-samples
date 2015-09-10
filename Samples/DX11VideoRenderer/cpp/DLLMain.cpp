#include <initguid.h>
#include "Common.h"
#include "Activate.h"
#include "ClassFactory.h"
#include "MediaSink.h"

HMODULE g_hModule = NULL;
volatile long DX11VideoRenderer::CBase::s_lObjectCount = 0;
volatile long DX11VideoRenderer::CClassFactory::s_lLockCount = 0;

// helper functions

///////////////////////////////////////////////////////////////////////
// Name: CreateObjectKeyName
// Desc: Converts a CLSID into a string with the form "CLSID\{clsid}"
///////////////////////////////////////////////////////////////////////

HRESULT CreateObjectKeyName(const GUID& guid, _Out_writes_(cchMax) TCHAR* pszName, DWORD cchMax)
{
    pszName[0] = _T('\0');

    // convert CLSID to string
    OLECHAR pszCLSID[CHARS_IN_GUID];
    HRESULT hr = StringFromGUID2(guid, pszCLSID, CHARS_IN_GUID);
    if (SUCCEEDED(hr))
    {
        // create a string of the form "CLSID\{clsid}"
        hr = StringCchPrintf(pszName, cchMax - 1, TEXT("CLSID\\%ls"), pszCLSID);
    }
    return hr;
}

///////////////////////////////////////////////////////////////////////
// Name: SetKeyValue
// Desc: Sets a string value (REG_SZ) for a registry key
//
// hKey:   Handle to the registry key.
// sName:  Name of the value. Use NULL for the default value.
// sValue: The string value.
///////////////////////////////////////////////////////////////////////

HRESULT SetKeyValue(HKEY hKey, const TCHAR* pszName, const TCHAR* pszValue)
{
    size_t cch = 0;
    DWORD cbData = 0;
    HRESULT hr = StringCchLength(pszValue, MAXLONG, &cch);
    if (SUCCEEDED(hr))
    {
        cbData = (DWORD) (sizeof(TCHAR) * (cch + 1)); // add 1 for the NULL character
        hr = __HRESULT_FROM_WIN32(RegSetValueEx(hKey, pszName, 0, REG_SZ, reinterpret_cast<const BYTE*>(pszValue), cbData));
    }
    return hr;
}

///////////////////////////////////////////////////////////////////////
// Name: RegisterObject
// Desc: Creates the registry entries for a COM object.
//
// guid: The object's CLSID
// sDescription: Description of the object
// sThreadingMode: Threading model. e.g., "Both"
///////////////////////////////////////////////////////////////////////

HRESULT RegisterObject(GUID guid, const TCHAR* pszDescription, const TCHAR* pszThreadingModel)
{
    HRESULT hr = S_OK;
    TCHAR pszTemp[MAX_PATH];
    HKEY hKey = NULL;
    HKEY hSubkey = NULL;
    DWORD dwRet = 0;

    do
    {
        hr = CreateObjectKeyName(guid, pszTemp, MAX_PATH);
        if (FAILED(hr))
        {
            break;
        }

        hr = __HRESULT_FROM_WIN32(RegCreateKeyEx(HKEY_CLASSES_ROOT, pszTemp, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL));
        if (FAILED(hr))
        {
            break;
        }

        hr = SetKeyValue(hKey, NULL, pszDescription);
        if (FAILED(hr))
        {
            break;
        }

        hr = __HRESULT_FROM_WIN32(RegCreateKeyEx(hKey, L"InprocServer32", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hSubkey, NULL));
        if (FAILED(hr))
        {
            break;
        }

        dwRet = GetModuleFileName(g_hModule, pszTemp, MAX_PATH);
        if (dwRet == 0)
        {
            hr = __HRESULT_FROM_WIN32(GetLastError());
            break;
        }
        if (dwRet == MAX_PATH)
        {
            hr = E_FAIL; // buffer too small
            break;
        }

        hr = SetKeyValue(hSubkey, NULL, pszTemp);
        if (FAILED(hr))
        {
            break;
        }

        hr = SetKeyValue(hSubkey, L"ThreadingModel", pszThreadingModel);
    }
    while (FALSE);

    if (hSubkey != NULL)
    {
        RegCloseKey(hSubkey);
    }

    if (hKey != NULL)
    {
        RegCloseKey(hKey);
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////
// Name: UnregisterObject
// Desc: Deletes the registry entries for a COM object.
//
// guid: The object's CLSID
///////////////////////////////////////////////////////////////////////

HRESULT UnregisterObject(GUID guid)
{
    HRESULT hr = S_OK;
    TCHAR pszTemp[MAX_PATH];

    do
    {
        hr = CreateObjectKeyName(guid, pszTemp, MAX_PATH);
        if (FAILED(hr))
        {
            break;
        }

        hr = __HRESULT_FROM_WIN32(RegDeleteTree(HKEY_CLASSES_ROOT, pszTemp));
    }
    while (FALSE);

    return hr;
}

// DLL Exports

STDAPI CreateDX11VideoRenderer(REFIID riid, void** ppvObject)
{
    return DX11VideoRenderer::CMediaSink::CreateInstance(riid, ppvObject);
}

STDAPI CreateDX11VideoRendererActivate(HWND hwnd, IMFActivate** ppActivate)
{
    return DX11VideoRenderer::CActivate::CreateInstance(hwnd, ppActivate);
}

STDAPI DllCanUnloadNow(void)
{
    return (DX11VideoRenderer::CBase::GetObjectCount() == 0 && DX11VideoRenderer::CClassFactory::IsLocked() == FALSE) ? S_OK : S_FALSE;
}

STDAPI DllGetClassObject(_In_ REFCLSID clsid, _In_ REFIID riid, _Outptr_ void** ppvObject)
{
    if (clsid != CLSID_DX11VideoRenderer)
    {
        return CLASS_E_CLASSNOTAVAILABLE;
    }

    DX11VideoRenderer::CClassFactory* pFactory = new DX11VideoRenderer::CClassFactory();
    if (pFactory == NULL)
    {
        return E_OUTOFMEMORY;
    }

    pFactory->AddRef();

    HRESULT hr = pFactory->QueryInterface(riid, ppvObject);

    SafeRelease(pFactory);

    return hr;
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {
            g_hModule = static_cast<HMODULE>(hModule);
            break;
        }
        default:
        {
            break;
        }
    }

    return TRUE;
}

STDAPI DllRegisterServer(void)
{
    HRESULT hr = S_OK;

    do
    {
        hr = RegisterObject(CLSID_DX11VideoRenderer, L"DX11 Video Renderer", L"Both");
        if (FAILED(hr))
        {
            break;
        }

        hr = RegisterObject(CLSID_DX11VideoRendererActivate, L"DX11 Video Renderer Activate", L"Both");
        if (FAILED(hr))
        {
            break;
        }

        hr = MFTRegister(
            CLSID_DX11VideoRenderer,    // CLSID
            MFT_CATEGORY_OTHER,         // Category
            L"DX11 Video Renderer",     // Friendly name
            0,                          // Reserved, must be zero.
            0,
            NULL,
            0,
            NULL,
            NULL
            );
    }
    while (FALSE);

    return hr;
}

STDAPI DllUnregisterServer(void)
{
    HRESULT hr = UnregisterObject(CLSID_DX11VideoRenderer);
    HRESULT hrTemp = MFTUnregister(CLSID_DX11VideoRenderer);
    if (SUCCEEDED(hr))
    {
        hr = hrTemp;
    }
    return hr;
}
