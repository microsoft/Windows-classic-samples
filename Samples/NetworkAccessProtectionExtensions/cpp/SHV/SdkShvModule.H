// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#ifndef SDK_SHV_MODULE_H
#define SDK_SHV_MODULE_H

#include "DebugHelper.h"
#include <NapTypes.h>
#include <NapServerManagement.h>
#include "SampleShv.h"
#include <Strsafe.h>
#include "SdkCommon.h"

using namespace SDK_SAMPLE_COMMON;
using namespace SDK_SAMPLE_SHV;

static const wchar_t SDK_SHV_friendlyName[] = L"SDK SHV Sample";
static const wchar_t SDK_SHV_configFriendlyName[] = L"Sdk Shv Configuration UI COM Server";

static const wchar_t SDK_SHV_progId[] = L"SdkShv.SampleShv";
static const wchar_t SDK_SHV_dllName[] = L"SdkShv.DLL";
static const wchar_t SDK_SHV_exeName[] = L"sampleshvui.exe";
static const wchar_t SDK_SHV_version[] = L"1.0.0.1";
static const wchar_t SDK_SHV_description[] = L"System Health Validator(SHV)";
static const wchar_t SDK_SHV_vendorName[] = L"Microsoft";

static const wchar_t SDK_SHV_appID[] = L"{EEDF28CE-CC5D-4251-A114-33DC950091F4}";
static const wchar_t SDK_SHV_sampleUIAppID[] = L"{AD310CB9-8B4B-46ae-93BB-2D3C4DBE35DD}";


// SHV_CLSID_INFO is assigned a dummy value since INapComponentInfo interface
// is not implemented for the SDK SHV sample 
static const GUID SHV_CLSID_INFO =
{ 0x00000000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };

class CSdkShvModule
{
public:

    /// Add entries to registry
    HRESULT RegisterServer();

    /// Remove entries from registry
    HRESULT UnregisterServer();

    /// Registers the SDK SHV with the NAP Server.
    HRESULT RegisterSdkShv();

    /// Unregisters the SDK SHV with the NAP Server.
    HRESULT UnregisterSdkShv();

private:

    /// Fill the NapComponentRegistrationInfo structure that needs to be passed during registration.
    HRESULT FillShvComponentRegistrationInfo(
        _Out_ NapComponentRegistrationInfo *shvInfo);

    // Helper Function for releasing ShvComponentRegistrationInfo members
    void FreeComponentRegistration(
        _Inout_ NapComponentRegistrationInfo *shvInfo);
};

#endif
