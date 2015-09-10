// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// dllmain.cpp : Implementation of DllMain.

#include "Windows.h"
#include "resource.h"
#include "dllmain.h"
#include "SdkCommon.h"
#include "ComponentInfo.h"
#include "Strsafe.h"
using namespace SDK_SAMPLE_COMMON;

HMODULE g_hModule=NULL;
LONG g_nComObjsInUse = 0;

CSdkShaInfoModule sdkModule;

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(
    _In_ HINSTANCE hInstance, 
    _In_ DWORD reason, 
    _In_opt_ LPVOID /*lpReserved*/)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
		g_hModule = (HMODULE)hInstance ;
    }

    return TRUE ;
}


HRESULT CSdkShaInfoModule::RegisterServer()
{
    HRESULT hr = S_OK;

    // Write CLSID to a buffer
    wchar_t *lpClsid = NULL;
    hr = StringFromCLSID(
            CLSID_ComponentInfo,
            &lpClsid);
    if (FAILED(hr))
    {
		lpClsid = NULL;
		goto Cleanup;
    }

    wchar_t keyPath[MAX_KEY_LENGTH] = {0};

    // SOFTWARE\CLASSES\<progId>
    hr = CreateKeyPath(keyPath, MAX_KEY_LENGTH, SDK_keyRoot, SDK_SHA_progId, NULL);
    if (FAILED(hr))
    {
        goto Cleanup;
    }
    hr = SdkSetRegistryStringValue(keyPath, NULL, SDK_SHA_friendlyName);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    // SOFTWARE\CLASSES\<progId>\CLSID
    hr = CreateKeyPath(keyPath, MAX_KEY_LENGTH, SDK_keyRoot, SDK_SHA_progId, L"\\CLSID");
    if (FAILED(hr))
    {
        goto Cleanup;
    }
    hr = SdkSetRegistryStringValue(keyPath, NULL, lpClsid);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    wchar_t appVer[MAX_KEY_LENGTH] = {0};
    hr = StringCchPrintf(appVer, MAX_KEY_LENGTH, L"%ws.1", SDK_SHA_progId);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    // SOFTWARE\CLASSES\<progId>\CurVer
    hr = CreateKeyPath(keyPath, MAX_KEY_LENGTH, SDK_keyRoot, SDK_SHA_progId, L"\\CurVer");
    if (FAILED(hr))
    {
        goto Cleanup;
    }
    hr = SdkSetRegistryStringValue(keyPath, NULL, appVer);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    // SOFTWARE\CLASSES\<progId>.1
    hr = CreateKeyPath(keyPath, MAX_KEY_LENGTH, SDK_keyRoot, appVer, NULL);
    if (FAILED(hr))
    {
        goto Cleanup;
    }
    hr = SdkSetRegistryStringValue(keyPath, NULL, SDK_SHA_friendlyName);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    // SOFTWARE\CLASSES\<progId>.1\CLSID
    hr = CreateKeyPath(keyPath, MAX_KEY_LENGTH, SDK_keyRoot, appVer, L"\\CLSID");
    if (FAILED(hr))
    {
        goto Cleanup;
    }
    hr = SdkSetRegistryStringValue(keyPath, NULL, lpClsid);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    // SOFTWARE\CLASSES\CLSID\<clsid>
    hr = CreateKeyPath(keyPath, MAX_KEY_LENGTH, SDK_clsidKeyRoot, lpClsid, NULL);
    if (FAILED(hr))
    {
        goto Cleanup;
    }
    hr = SdkSetRegistryStringValue(keyPath, NULL, SDK_SHA_friendlyName);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    hr = SdkSetRegistryStringValue(keyPath, L"AppID", SDK_SHA_appID);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    wchar_t buff[MAX_PATH] = {0};
    if (!GetModuleFileName(
        g_hModule,
        buff,
        MAX_PATH))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }

    // SOFTWARE\CLASSES\CLSID\<clsid>\ProgId
    hr = CreateKeyPath(keyPath, MAX_KEY_LENGTH, SDK_clsidKeyRoot, lpClsid, L"\\ProgId");
    if (FAILED(hr))
    {
        goto Cleanup;
    }
    hr = SdkSetRegistryStringValue(keyPath, NULL, appVer);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    // SOFTWARE\CLASSES\CLSID\<clsid>\Programmable
    hr = CreateKeyPath(keyPath, MAX_KEY_LENGTH, SDK_clsidKeyRoot, lpClsid, L"\\Programmable");
    if (FAILED(hr))
    {
        goto Cleanup;
    }
    hr = SdkSetRegistryValue(keyPath, NULL, REG_SZ, NULL, 0);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    // SOFTWARE\CLASSES\CLSID\<clsid>\InprocServer32
    hr = CreateKeyPath(keyPath, MAX_KEY_LENGTH, SDK_clsidKeyRoot, lpClsid, L"\\InprocServer32");
    if (FAILED(hr))
    {
        goto Cleanup;
    }
    hr = SdkSetRegistryStringValue(keyPath, NULL, buff);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    hr = SdkSetRegistryStringValue(keyPath, L"ThreadingModel", L"Both");
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    // SOFTWARE\CLASSES\CLSID\<clsid>\VersionIndependentProgID
    hr = CreateKeyPath(keyPath, MAX_KEY_LENGTH, SDK_clsidKeyRoot, lpClsid, L"\\VersionIndependentProgID");
    if (FAILED(hr))
    {
        goto Cleanup;
    }
    hr = SdkSetRegistryStringValue(keyPath, NULL, SDK_SHA_progId);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    // SOFTWARE\CLASSES\CLSID\<clsid>\TypeLib
    hr = CreateKeyPath(keyPath, MAX_KEY_LENGTH, SDK_clsidKeyRoot, lpClsid, L"\\TypeLib");
    if (FAILED(hr))
    {
        goto Cleanup;
    }
    hr = SdkSetRegistryStringValue(keyPath, NULL, SDK_SHA_typeLib);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    // SOFTWARE\CLASSES\AppID\<appId>
    hr = CreateKeyPath(keyPath, MAX_KEY_LENGTH, SDK_appidKeyRoot, SDK_SHA_appID, NULL);
    if (FAILED(hr))
    {
        goto Cleanup;
    }
    hr = SdkSetRegistryStringValue(keyPath, NULL, SDK_SHA_friendlyName);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    hr = SdkSetRegistryValue(keyPath, L"DllSurrogate", REG_SZ, NULL, 0);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

Cleanup:

    if (lpClsid != NULL)
    {
        CoTaskMemFree(lpClsid);
    }

    if (FAILED(hr))
    {
        UnregisterServer();
    }

    return hr;
}


HRESULT CSdkShaInfoModule::UnregisterServer()
{
    HRESULT res = S_OK, hr = S_OK;

    // Write CLSID to a buffer
    wchar_t *lpClsid = NULL;
    hr = StringFromCLSID(
            CLSID_ComponentInfo,
            &lpClsid);
    if (FAILED(hr))
    {
		lpClsid = NULL;
		goto Cleanup;
    }

    // SOFTWARE\CLASSES\<progId>
    hr = DeleteRegistryTree(SDK_keyRoot, SDK_SHA_progId);
    if (FAILED(hr))
    {
        res = hr;
    }

    wchar_t appVer[MAX_KEY_LENGTH] = {0};
    hr = StringCchPrintf(appVer, MAX_KEY_LENGTH, L"%ws.1", SDK_SHA_progId);
    if (SUCCEEDED(hr))
    {
        // SOFTWARE\CLASSES\<progId>.1
        hr = DeleteRegistryTree(SDK_keyRoot, appVer);
        if (FAILED(hr))
        {
            res = hr;
        }
    }

    // SOFTWARE\CLASSES\CLSID\<clsid>
    hr = DeleteRegistryTree(SDK_clsidKeyRoot, lpClsid);
    if (FAILED(hr))
    {
        res = hr;
    }

    // SOFTWARE\CLASSES\AppID\<clsid>
    hr = DeleteRegistryTree(SDK_appidKeyRoot, SDK_SHA_appID);
    if (FAILED(hr))
    {
        res = hr;
    }

Cleanup:
    if (lpClsid != NULL)
    {
        CoTaskMemFree(lpClsid);
    }

    return res;
}