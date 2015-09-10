// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <NapUtil.h>
#include "SdkShvModule.h"
#include "SdkCommon.h"

using namespace SDK_SAMPLE_COMMON;
extern HMODULE g_hModule;

/// Registers the SDK SHV with the NAP Server.
HRESULT CSdkShvModule::RegisterSdkShv()
{
    HRESULT hr = S_OK;

    /// Pointer to the INapServerManagement interface
    INapServerManagement* pSHVMgmt = NULL;

    /// Registration Information
    NapComponentRegistrationInfo shvInfo;
    ZeroMemory (&shvInfo, sizeof(shvInfo));

    hr = CoCreateInstance(CLSID_NapServerManagement,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_INapServerManagement,
                          reinterpret_cast<void**>(&pSHVMgmt));
    if (FAILED(hr))
    {
        DebugPrintfW(L"RegisterSdkShv: CoCreateInstance Failed with %#x",hr);
        goto Cleanup;
    }

    hr = FillShvComponentRegistrationInfo(&shvInfo);
    if(FAILED(hr))
    {
        DebugPrintfW(L"RegisterSdkShv:: FillShvComponentRegistrationInfo Failed with %#x",hr);
		ZeroMemory(&shvInfo, sizeof(shvInfo));
		goto Cleanup;
	}

    hr = pSHVMgmt->RegisterSystemHealthValidator(&shvInfo, &CLSID_SampleShv);
    if (FAILED(hr))
    {
        DebugPrintfW(L"RegisterSdkShv:: RegisterSystemHealthValidator failed %#x", hr); 
        goto Cleanup;
    }

Cleanup:
    FreeComponentRegistration(&shvInfo);
    ReleaseObject(pSHVMgmt);
    return hr;
}

/// Unregisters the SDK SHV with the NAP Server.
HRESULT CSdkShvModule::UnregisterSdkShv()
{
    HRESULT hr = S_OK;
    INapServerManagement* pSHVMgmt = NULL;

    // Need to CoCreate again since the COM object created in
    // RegisterSdkShv has been discarded
    hr = CoCreateInstance(CLSID_NapServerManagement,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_INapServerManagement,
                          reinterpret_cast<void**>(&pSHVMgmt));
    if (FAILED(hr))
    {
        DebugPrintfW(L"UnregisterSdkShv: CoCreateInstance Failed with %#x",hr);
        goto Cleanup;
    }

    hr = pSHVMgmt->UnregisterSystemHealthValidator(QuarSampleSystemHealthId);
    if (FAILED(hr))
    {
        DebugPrintfW(L"UnregisterSdkShv: UnregisterSystemHealthValidator Failed with %#x",hr);
        goto Cleanup;
    }
    
Cleanup:    
    ReleaseObject(pSHVMgmt);
    return hr;
}

/// Fill the NapComponentRegistrationInfo structure that needs to be passed during registration.
HRESULT CSdkShvModule::FillShvComponentRegistrationInfo(
    _Out_ NapComponentRegistrationInfo *shvInfo)
{
    HRESULT hr = S_OK;

    shvInfo->id = QuarSampleSystemHealthId;
    shvInfo->infoClsid = SHV_CLSID_INFO;
	shvInfo->configClsid = CLSID_SDK_SHV_UI;

    hr = FillCountedString(SDK_SHV_friendlyName, &(shvInfo->friendlyName));
    if (FAILED(hr))
    {
        DebugPrintfW(L"QShv::FillShvComponentRegistrationInfo(): AllocCountedString for friendlyName returned error %#x!",hr);
        goto Cleanup;
    }

    hr = FillCountedString(SDK_SHV_description, &(shvInfo->description));
    if (FAILED(hr))
    {
        DebugPrintfW(L"QShv::FillShvComponentRegistrationInfo(): AllocCountedString for description returned error %#x!",hr);
        goto Cleanup;
    }

    hr = FillCountedString(SDK_SHV_version, &(shvInfo->version));
    if (FAILED(hr))
    {
        DebugPrintfW(L"QShv::FillShvComponentRegistrationInfo(): AllocCountedString for version returned error %#x!",hr);
        goto Cleanup;
    }

    hr = FillCountedString(SDK_SHV_vendorName, &(shvInfo->vendorName));
    if (FAILED(hr))
    {
        DebugPrintfW(L"QShv::FillShvComponentRegistrationInfo(): AllocCountedString for vendorName returned error %#x!",hr);
        goto Cleanup;
    }

	return hr;

Cleanup:
	FreeComponentRegistration(shvInfo);
    return hr;
}

// Helper Function for releasing ShaComponentRegistrationInfo members
void CSdkShvModule::FreeComponentRegistration(
    _Inout_ NapComponentRegistrationInfo *shvInfo)
{
    EmptyCountedString(&shvInfo->friendlyName);
    EmptyCountedString(&shvInfo->description);
    EmptyCountedString(&shvInfo->version);
    EmptyCountedString(&shvInfo->vendorName);
}


// A binary data representing ACL of network service used for COM object launch permissions
const unsigned char LaunchPermission[] = {
    0x01, 0x00, 0x04, 0x80, 0x70, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x14, 0x00, 0x00, 0x00, 0x02, 0x00, 0x5c, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00,
    0x1f, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x12, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x18, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05,
    0x20, 0x00, 0x00, 0x00, 0x20, 0x02, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x1f, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00,
    0x0b, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x14, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x20, 0x00, 0x00, 0x00, 0x20, 0x02, 0x00, 0x00,
    0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x20, 0x00, 0x00, 0x00, 0x20, 0x02, 0x00, 0x00
};

// A binary data representing ACL of network service used for COM object access permissions
const unsigned char AccessPermission[] = {
    0x01, 0x00, 0x04, 0x80, 0x58, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x14, 0x00, 0x00, 0x00, 0x02, 0x00, 0x44, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 
    0x03, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x14, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x14, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05,
    0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x05, 0x12, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05,
    0x20, 0x00, 0x00, 0x00, 0x20, 0x02, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05,
    0x20, 0x00, 0x00, 0x00, 0x20, 0x02, 0x00, 0x00
};

HRESULT CSdkShvModule::RegisterServer()
{
    HRESULT hr = S_OK;

    // Write CLSID to a buffer
    wchar_t *lpClsid = NULL;
    hr = StringFromCLSID(
            CLSID_SampleShv,
            &lpClsid);
    if (FAILED(hr))
    {
        DebugPrintfW(L"CSdkShvModule::RegisterServer StringFromCLSID failed %#x", hr); 
		lpClsid = NULL;
        goto Cleanup;
    }

    wchar_t keyPath[MAX_KEY_LENGTH] = {0};

    // SOFTWARE\CLASSES\<progId>
    hr = CreateKeyPath(keyPath, MAX_KEY_LENGTH, SDK_keyRoot, SDK_SHV_progId, NULL);
    if (FAILED(hr))
    {
        DebugPrintfW(L"CSdkShvModule::RegisterServer CreateKeyPath (SDK_keyRoot, SDK_SHV_progId, NULL) failed %#x", hr); 
        goto Cleanup;
    }
    hr = SdkSetRegistryStringValue(keyPath, NULL, SDK_SHV_friendlyName);
    if (FAILED(hr))
    {
        DebugPrintfW(L"CSdkShvModule::RegisterServer SdkSetRegistryStringValue failed %#x", hr); 
        goto Cleanup;
    }

    // SOFTWARE\CLASSES\<progId>\CLSID
    hr = CreateKeyPath(keyPath, MAX_KEY_LENGTH, SDK_keyRoot, SDK_SHV_progId, L"\\CLSID");
    if (FAILED(hr))
    {
        DebugPrintfW(L"CSdkShvModule::RegisterServer CreateKeyPath (SDK_keyRoot, SDK_SHV_progId, \\CLSID) failed %#x", hr); 
        goto Cleanup;
    }
    hr = SdkSetRegistryStringValue(keyPath, NULL, lpClsid);
    if (FAILED(hr))
    {
        DebugPrintfW(L"CSdkShvModule::RegisterServer SdkSetRegistryStringValue failed %#x", hr); 
        goto Cleanup;
    }

    // SOFTWARE\CLASSES\CLSID\<clsid>
    hr = CreateKeyPath(keyPath, MAX_KEY_LENGTH, SDK_clsidKeyRoot, lpClsid, NULL);
    if (FAILED(hr))
    {
        DebugPrintfW(L"CSdkShvModule::RegisterServer CreateKeyPath (SDK_clsidKeyRoot, lpClsid, NULL) failed %#x", hr); 
        goto Cleanup;
    }
    hr = SdkSetRegistryStringValue(keyPath, NULL, SDK_SHV_friendlyName);
    if (FAILED(hr))
    {
        DebugPrintfW(L"CSdkShvModule::RegisterServer SdkSetRegistryStringValue failed %#x", hr); 
        goto Cleanup;
    }

    hr = SdkSetRegistryStringValue(keyPath, L"AppID", SDK_SHV_appID);
    if (FAILED(hr))
    {
        DebugPrintfW(L"CSdkShvModule::RegisterServer SdkSetRegistryStringValue failed %#x", hr); 
        goto Cleanup;
    }

    wchar_t buff[MAX_PATH] = {0};
    if (!GetModuleFileName(
        g_hModule,
        buff,
        MAX_PATH))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        DebugPrintfW(L"CSdkShvModule::RegisterServer GetModuleFileName failed %#x", hr); 
        goto Cleanup;
    }

    // SOFTWARE\CLASSES\CLSID\<clsid>\ProgId
    hr = CreateKeyPath(keyPath, MAX_KEY_LENGTH, SDK_clsidKeyRoot, lpClsid, L"\\ProgId");
    if (FAILED(hr))
    {
        DebugPrintfW(L"CSdkShvModule::RegisterServer CreateKeyPath (SDK_clsidKeyRoot, lpClsid, \\ProgId) failed %#x", hr); 
        goto Cleanup;
    }
    hr = SdkSetRegistryStringValue(keyPath, NULL, SDK_SHV_progId);
    if (FAILED(hr))
    {
        DebugPrintfW(L"CSdkShvModule::RegisterServer SdkSetRegistryStringValue failed %#x", hr); 
        goto Cleanup;
    }

    // SOFTWARE\CLASSES\CLSID\<clsid>\InprocServer32\(Default)
    hr = CreateKeyPath(keyPath, MAX_KEY_LENGTH, SDK_clsidKeyRoot, lpClsid, L"\\InprocServer32");
    if (FAILED(hr))
    {
        DebugPrintfW(L"CSdkShvModule::RegisterServer CreateKeyPath (SDK_clsidKeyRoot, lpClsid, \\InprocServer32) failed %#x", hr); 
        goto Cleanup;
    }
    hr = SdkSetRegistryStringValue(keyPath, NULL, buff);
    if (FAILED(hr))
    {
        DebugPrintfW(L"CSdkShvModule::RegisterServer SdkSetRegistryStringValue failed %#x", hr); 
        goto Cleanup;
    }

   // SOFTWARE\CLASSES\CLSID\<clsid>\InprocServer32\ThreadingModel
    hr = SdkSetRegistryStringValue(keyPath, L"ThreadingModel", L"Both");
    if (FAILED(hr))
    {
        DebugPrintfW(L"CSdkShvModule::RegisterServer SdkSetRegistryStringValue failed %#x", hr); 
        goto Cleanup;
    }

    // SOFTWARE\CLASSES\AppID\<appId>
    hr = CreateKeyPath(keyPath, MAX_KEY_LENGTH, SDK_appidKeyRoot, SDK_SHV_appID, NULL);
    if (FAILED(hr))
    {
        DebugPrintfW(L"CSdkShvModule::RegisterServer CreateKeyPath (SDK_appidKeyRoot, SDK_SHV_appID, NULL) failed %#x", hr); 
        goto Cleanup;
    }
    hr = SdkSetRegistryStringValue(keyPath, NULL, SDK_SHV_friendlyName);
    if (FAILED(hr))
    {
        DebugPrintfW(L"CSdkShvModule::RegisterServer SdkSetRegistryStringValue failed %#x", hr); 
        goto Cleanup;
    }
    hr = SdkSetRegistryValue(keyPath, L"DllSurrogate", REG_SZ, NULL, 0);
    if (FAILED(hr))
    {
        DebugPrintfW(L"CSdkShvModule::RegisterServer SdkSetRegistryValue failed %#x", hr); 
        goto Cleanup;
    }

    DWORD authLevel = 0;
    hr = SdkSetRegistryValue(keyPath, L"AuthenticationLevel", REG_DWORD, &authLevel, sizeof(DWORD));
    if (FAILED(hr))
    {
        DebugPrintfW(L"CSdkShvModule::RegisterServer SdkSetRegistryValue failed %#x", hr); 
        goto Cleanup;
    }

    hr = SdkSetRegistryStringValue(keyPath, L"RunAs", L"NT AUTHORITY\\networkservice");
    if (FAILED(hr))
    {
        DebugPrintfW(L"CSdkShvModule::RegisterServer SdkSetRegistryStringValue failed %#x", hr); 
        goto Cleanup;
    }

    // SOFTWARE\CLASSES\AppID\SdkShv.DLL
    hr = CreateKeyPath(keyPath, MAX_KEY_LENGTH, SDK_appidKeyRoot, SDK_SHV_dllName, NULL);
    if (FAILED(hr))
    {
        DebugPrintfW(L"CSdkShvModule::RegisterServer CreateKeyPath (SDK_appidKeyRoot, SDK_SHV_dllName, NULL) failed %#x", hr); 
        goto Cleanup;
    }
    hr = SdkSetRegistryStringValue(keyPath, L"AppID", SDK_SHV_appID);
    if (FAILED(hr))
    {
        DebugPrintfW(L"CSdkShvModule::RegisterServer SdkSetRegistryStringValue failed %#x", hr); 
        goto Cleanup;
    }

    // SOFTWARE\CLASSES\AppID\<UI clsid>
    hr = CreateKeyPath(keyPath, MAX_KEY_LENGTH, SDK_appidKeyRoot, SDK_SHV_sampleUIAppID, NULL);
    if (FAILED(hr))
    {
        DebugPrintfW(L"CSdkShvModule::RegisterServer CreateKeyPath (SDK_appidKeyRoot, SDK_SHV_sampleUIAppID, NULL) failed %#x", hr); 
        goto Cleanup;
    }
    hr = SdkSetRegistryStringValue(keyPath, NULL, SDK_SHV_configFriendlyName);
    if (FAILED(hr))
    {
        DebugPrintfW(L"CSdkShvModule::RegisterServer SdkSetRegistryStringValue failed %#x", hr); 
        goto Cleanup;
    }
    hr = SdkSetRegistryValue(keyPath, L"AccessPermission", REG_BINARY, static_cast<const void*>(AccessPermission), static_cast<DWORD>(sizeof(AccessPermission)));
    if (FAILED(hr))
    {
        DebugPrintfW(L"CSdkShvModule::RegisterServer SdkSetRegistryStringValue failed %#x", hr); 
        goto Cleanup;
    }

    hr = SdkSetRegistryValue(keyPath, L"LaunchPermission", REG_BINARY, static_cast<const void*>(LaunchPermission), static_cast<DWORD>(sizeof(LaunchPermission)));
    if (FAILED(hr))
    {
        DebugPrintfW(L"CSdkShvModule::RegisterServer SdkSetRegistryValue failed %#x", hr); 
        goto Cleanup;
    }

    // SOFTWARE\CLASSES\AppID\sampleshvui.exe
    hr = CreateKeyPath(keyPath, MAX_KEY_LENGTH, SDK_appidKeyRoot, SDK_SHV_exeName, NULL);
    if (FAILED(hr))
    {
        DebugPrintfW(L"CSdkShvModule::RegisterServer CreateKeyPath (SDK_appidKeyRoot, SDK_SHV_exeName, NULL) failed %#x", hr); 
        goto Cleanup;
    }
    hr = SdkSetRegistryStringValue(keyPath, L"AppID", SDK_SHV_sampleUIAppID);
    if (FAILED(hr))
    {
        DebugPrintfW(L"CSdkShvModule::RegisterServer SdkSetRegistryStringValue failed %#x", hr); 
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


HRESULT CSdkShvModule::UnregisterServer()
{
    HRESULT res = S_OK, hr = S_OK;

    // Write CLSID to a buffer
    wchar_t *lpClsid = NULL;
    hr = StringFromCLSID(
            CLSID_SampleShv,
            &lpClsid);
    if (FAILED(hr))
    {
        DebugPrintfW(L"CSdkShvModule::RegisterServer StringFromCLSID failed %#x", hr); 
		lpClsid = NULL;
        goto Cleanup;
    }

    // SOFTWARE\CLASSES\<progId>
    hr = DeleteRegistryTree(SDK_keyRoot, SDK_SHV_progId);
    if (FAILED(hr))
    {
        res = hr;
        DebugPrintfW(L"CSdkShvModule::RegisterServer DeleteRegistryTree (SDK_keyRoot, SDK_SHV_progId) failed %#x", hr); 
    }

    // SOFTWARE\CLASSES\CLSID\<clsid>
    hr = DeleteRegistryTree(SDK_clsidKeyRoot, lpClsid);
    if (FAILED(hr))
    {
        res = hr;
        DebugPrintfW(L"CSdkShvModule::RegisterServer DeleteRegistryTree (SDK_clsidKeyRoot, lpClsid) failed %#x", hr); 
    }

    // SOFTWARE\CLASSES\AppID\<clsid>
    hr = DeleteRegistryTree(SDK_appidKeyRoot, SDK_SHV_appID);
    if (FAILED(hr))
    {
        res = hr;
        DebugPrintfW(L"CSdkShvModule::RegisterServer DeleteRegistryTree (SDK_appidKeyRoot, SDK_SHV_appID) failed %#x", hr); 
    }

    // SOFTWARE\CLASSES\AppID\SdkShv.DLL
    hr = DeleteRegistryTree(SDK_appidKeyRoot, SDK_SHV_dllName);
    if (FAILED(hr))
    {
        res = hr;
        DebugPrintfW(L"CSdkShvModule::RegisterServer DeleteRegistryTree (SDK_appidKeyRoot, SDK_SHV_dllName) failed %#x", hr); 
    }

    // SOFTWARE\CLASSES\AppID\<UI clsid>
    hr = DeleteRegistryTree(SDK_appidKeyRoot, SDK_SHV_sampleUIAppID);
    if (FAILED(hr))
    {
        DebugPrintfW(L"CSdkShvModule::RegisterServer DeleteRegistryTree (SDK_appidKeyRoot, SDK_SHV_sampleUIAppID) failed %#x", hr); 
        res = hr;
    }

    // SOFTWARE\CLASSES\AppID\sampleshvui.exe
    hr = DeleteRegistryTree(SDK_appidKeyRoot, SDK_SHV_exeName);
    if (FAILED(hr))
    {
        DebugPrintfW(L"CSdkShvModule::RegisterServer DeleteRegistryTree (SDK_appidKeyRoot, SDK_SHV_exeName) failed %#x", hr); 
        res = hr;
    }

Cleanup:
    if (lpClsid != NULL)
    {
        CoTaskMemFree(lpClsid);
    }

    return res;
}

