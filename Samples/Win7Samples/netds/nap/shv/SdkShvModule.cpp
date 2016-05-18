// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include <NapUtil.h>
#include "SdkShvModule.h"
#include "SdkCommon.h"

using namespace SDK_SAMPLE_COMMON;

/// Registers the SDK SHV with the NAP Server.
HRESULT CSdkShvModule::RegisterSdkShv() throw()
{
    HRESULT hr = S_OK;

    /// Pointer to the INapServerManagement interface
    CComPtr<INapServerManagement> pSHVMgmt = NULL;
    /// Registration Information
    NapComponentRegistrationInfo shvInfo;

    ZeroMemory (&shvInfo, sizeof(shvInfo));
    hr = pSHVMgmt.CoCreateInstance(CLSID_NapServerManagement,
                           NULL,
                           CLSCTX_INPROC_SERVER);
    if (FAILED(hr))
    {
        DebugPrintfW(L"RegisterSdkShv: CoCreateInstance Failed with %#x",hr);
        goto Cleanup;
    }

    hr = FillShvComponentRegistrationInfo(&shvInfo);
    if(FAILED(hr))
    {
        DebugPrintfW(L"RegisterSdkShv:: FillShvComponentRegistrationInfo Failed with %#x",hr);
        goto Cleanup;
    }

     hr = pSHVMgmt->RegisterSystemHealthValidator(&shvInfo,(CLSID *)&__uuidof(CSampleShv));
       if (FAILED(hr))
    {
        DebugPrintfW(L"RegisterSdkShv:: RegisterSystemHealthValidator failed %#x", hr); 
        goto Cleanup;
    }

    Cleanup:
    FreeComponentRegistration(&shvInfo);
    return hr;
}

/// Unregisters the SDK SHV with the NAP Server.
HRESULT CSdkShvModule::UnregisterSdkShv() throw()
{
    HRESULT hr = S_OK;
    CComPtr<INapServerManagement> pSHVMgmt = NULL;

    // Need to CoCreate again since the COM object created in
    // RegisterSdkShv has been discarded
    hr = pSHVMgmt.CoCreateInstance(CLSID_NapServerManagement,
                               NULL,
                               CLSCTX_INPROC_SERVER);
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
    return hr;
}

/// Fill the NapComponentRegistrationInfo structure that needs to be passed during registration.
HRESULT CSdkShvModule::FillShvComponentRegistrationInfo (NapComponentRegistrationInfo *shvInfo) throw()
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

Cleanup:
    return hr;
}

// Helper Function for releaseing ShaComponentRegistrationInfo members
void CSdkShvModule::FreeComponentRegistration(NapComponentRegistrationInfo *shvInfo) throw()
{
    EmptyCountedString(&shvInfo->friendlyName);
    EmptyCountedString(&shvInfo->description);
    EmptyCountedString(&shvInfo->version);
    EmptyCountedString(&shvInfo->vendorName);
}
