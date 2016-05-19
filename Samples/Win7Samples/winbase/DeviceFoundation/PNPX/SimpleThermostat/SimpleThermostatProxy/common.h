////////////////////////////////////////////////////////////////////////////////
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
////////////////////////////////////////////////////////////////////////////////
#pragma once

// Public Headers
#include <FunctionDiscoveryProvider.h>

enum DEVICE_PROTOCOL_TYPE
{
    DEVICE_PROTOCOL_TYPE_UPNP,
    DEVICE_PROTOCOL_TYPE_WSD
};

VOID DllIncLockCount();
VOID DllDecLockCount();

HRESULT GetDeviceType(
    __in IPropertyStore* pPropStore,
    __deref_out DEVICE_PROTOCOL_TYPE* pDeviceType
    );