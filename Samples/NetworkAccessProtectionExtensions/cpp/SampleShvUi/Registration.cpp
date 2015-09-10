// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Regutil.h"
#include "shvuicf.h"
#include "Registration.h"
#include "DebugHelper.h"
#include "strsafe.h"


#define CLSIDSTR_MS_SHVUI L"{230b2a03-bbb3-4d50-839b-74f095e2b53e}"
// This AppID has been created by 'regsvr32 SdkShv.dll' earlier, so we simply use it.
#define STR_APPID_SDK_SHV_CONFIG L"{AD310CB9-8B4B-46ae-93BB-2D3C4DBE35DD}"
#define CLSID_MS_SHVUI_FRIENDLY_NAME L"Sample SHV Configuration UI"
#define MAX_LENGTH 256

#define REGCLSID  L"CLSID"
#define REGAPPID  L"AppID"
#define LOCALSERVER32  L"LocalServer32"


STDMETHODIMP ShvUIRegisterServer()
{
    HRESULT hr = S_OK;
    WCHAR  module[MAX_PATH];                     // path name of server
    WCHAR clsidKey[MAX_LENGTH];                  
    DWORD result = ERROR_SUCCESS;

    result = GetModuleFileName(
                    ShvUIClassFactory::s_hModule, 
                    module, 
                    ARRAYSIZE(module));

    if (result == 0)
    {
        // Trace error message
        hr = HRESULT_FROM_WIN32(GetLastError());
        DebugPrintfW(L" --- ShvUIRegisterServer - GetModuleFileName failed %#x!",hr);
        goto cleanup;
    }
  
    // create entries under CLSID.
    // Description
    hr = ShvuiSetRegistryValue(
                    HKEY_CLASSES_ROOT,
                    REGCLSID,
                    CLSIDSTR_MS_SHVUI,
                    NULL,
                    REG_SZ,
                    reinterpret_cast<const BYTE*>(CLSID_MS_SHVUI_FRIENDLY_NAME),
                    static_cast<DWORD>((wcslen(CLSID_MS_SHVUI_FRIENDLY_NAME)+1) * sizeof(wchar_t)));
	if (FAILED(hr))
    {
        DebugPrintfW(L" --- ShvUIRegisterServer - ShvuiSetRegistryValue failed %#x!",hr);
		goto cleanup;
    }

    // Add AppID value under CLSIDSTR_MS_SHVUI. 
    hr = ShvuiSetRegistryValue(
                    HKEY_CLASSES_ROOT,
                    REGCLSID,
                    CLSIDSTR_MS_SHVUI,
                    REGAPPID,
                    REG_SZ,
                    reinterpret_cast<const BYTE*>(STR_APPID_SDK_SHV_CONFIG),
                    static_cast<DWORD>((wcslen(STR_APPID_SDK_SHV_CONFIG)+1) * sizeof(wchar_t)));
    if (FAILED(hr))
    {
        DebugPrintfW(L" --- ShvUIRegisterServer - ShvuiSetRegistryValue failed %#x!",hr);
		goto cleanup;
    }

    // get the class ID strings.
    hr = StringCchPrintf(clsidKey, MAX_LENGTH, L"%ws\\%ws", REGCLSID, CLSIDSTR_MS_SHVUI);
    if (FAILED(hr))
    {
        DebugPrintfW(L" --- ShvUIRegisterServer - StringCchPrintf failed %#x!",hr);
	    goto cleanup;
    }
   

    // set the server path.
    hr = ShvuiSetRegistryValue(
                    HKEY_CLASSES_ROOT,
                    clsidKey, 
                    LOCALSERVER32,
                    NULL,
                    REG_SZ,
                    reinterpret_cast<const BYTE*>(module),
                    static_cast<DWORD>((wcslen(module)+1) * sizeof(wchar_t)));
    if (FAILED(hr))
    {
        DebugPrintfW(L" --- ShvUIRegisterServer - ShvuiSetRegistryValue failed %#x!",hr);
    }

cleanup:
    return hr;    
}

STDMETHODIMP ShvUIUnRegisterServer()
{
    HRESULT hr = S_OK;

    wchar_t clsidKey[MAX_LENGTH];
    
    // get the class ID strings.

    hr = StringCchPrintf(clsidKey, MAX_LENGTH, L"%ws\\%ws", REGCLSID, CLSIDSTR_MS_SHVUI);
	if (FAILED(hr))
    {
        DebugPrintfW(L" --- ShvUIUnRegisterServer - StringCchPrintf failed %#x!",hr);
		goto cleanup;
    }


    // delete Class ID key
    hr = ShvuiDeleteRegistryKey(
                    HKEY_CLASSES_ROOT,
                    clsidKey);
	if (FAILED(hr))
    {
        DebugPrintfW(L" --- ShvUIUnRegisterServer - ShvuiDeleteRegistryKey failed %#x!",hr);
		goto cleanup;
    }

cleanup:
    return hr;       
}

