// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __SdkQecModule_H__
#define __SdkQecModule_H__

#include "stdafx.h"
#include "DebugHelper.h"
#include <NapTypes.h>
#include <NapManagement.h>
#include <Napenforcementclient.h>
#include <Strsafe.h>
#include "SdkCommon.h"


using namespace SDK_SAMPLE_COMMON;

static const wchar_t  QEC_FRIENDLY_NAME[] = L"SDKQec";
static const wchar_t  QEC_DESCRIPTION[] = L"Microsoft SDKQec";
static const wchar_t  QEC_VERSION[] = L"1.0.0.1";
static const wchar_t  QEC_VENDOR_NAME[] = L"Microsoft";

static const UINT32 NapSdkQecId = 0x000137F1;

// CLSID_INFO is assigned a dummy value since INapComponentInfo interface
// is not implemented for the SDK QEC sample 
static const GUID CLSID_INFO =
{ 0x00000000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };


class CSdkQecModule /* : public CAtlDllModuleT< CSdkQecModule > */
{
public:

    /// Registers the SDK QEC with the NAP Agent.
    HRESULT RegisterSdkQec() throw();

    /// Unregisters the SDK QEC with the NAP Agent.
    HRESULT UnregisterSdkQec() throw();

private:

    /// Fill the NapComponentRegistrationInfo structure that needs to be passed during registration.
    HRESULT FillQecComponentRegistrationInfo (NapComponentRegistrationInfo *agentInfo) throw();

    /// Helper Function to free memory used by SDK QEC.
    void FreeComponentRegistration(NapComponentRegistrationInfo *agentInfo) throw();

};

#endif

