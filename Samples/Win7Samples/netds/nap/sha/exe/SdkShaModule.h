// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.


#ifndef __SdkShaModule_H__
#define __SdkShaModule_H__

#include "stdafx.h"
#include "DebugHelper.h"
#include <NapTypes.h>
#include <NapManagement.h>
#include <NapSystemHealthAgent.h>
#include <Strsafe.h>
#include "SdkCommon.h"

static const wchar_t SHA_FRIENDLY_NAME[] = L"SHA SDK Sample";
static const wchar_t SHA_DESCRIPTION[] = L"Microsoft SDK SHA Sample";
static const wchar_t SHA_VERSION[] = L"1.0.0.1";
static const wchar_t SHA_VENDOR_NAME[] = L"Microsoft";

namespace SDK_SAMPLE_SHA
{
    static const UINT16 NUMBER_OF_HRESULTS = 1;
    typedef CComPtr<INapSystemHealthAgentCallback> IShaCallbackPtr;
    enum SDK_SHA_FIXUPSTATE {NOFIXESNEEDED, FIXESNEEDED, FIXESINPROGRESS};
}
//using namespace SDK_SAMPLE_COMMON;

// {E19DDEC2-3FBE-4c3b-9317-679760C13AAE}
static const GUID CLSID_INFO =
{ 0xe19ddec2, 0x3fbe, 0x4c3b, 0x93, 0x17, 0x67, 0x97, 0x60, 0xc1, 0x3a, 0xae };


class CSdkShaModule /*: public CAtlDllModuleT< CSdkShaModule >*/
{
public:

    /// Registers the SDK SHA with the NAP Server.
    HRESULT RegisterSdkSha();

    /// Unregisters the SDK SHA with the NAP Server.
    HRESULT UnregisterSdkSha();

private:

    /// Fill the NapComponentRegistrationInfo structure that needs to be passed during registration.
    HRESULT FillShaComponentRegistrationInfo (NapComponentRegistrationInfo *agentInfo);

    /// Helper Function to free memory used by SDK SHA.
    void FreeComponentRegistration(NapComponentRegistrationInfo *agentInfo);

};

#endif

