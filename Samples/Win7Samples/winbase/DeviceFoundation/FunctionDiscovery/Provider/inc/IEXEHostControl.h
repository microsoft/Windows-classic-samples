// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Abstract:
//
//      This module declares the IEXEHostControl interface.
//      This interface is used when the provider is hosted in the EXE server 
//      to notify the provider of Logoff notifications and manage the lifetime
//      of the EXE host.

#pragma once

typedef  VOID (__stdcall *PFNProviderLifetimeNotificationCallback)(
    bool fDestructed);

typedef interface IEXEHostControl IEXEHostControl;

MIDL_INTERFACE("1e988915-5501-4013-a5fd-f73e2be9307c")
IEXEHostControl:
    public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE RegisterProviderLifetimeNotificationCallback(
        PFNProviderLifetimeNotificationCallback pfnProviderLifetimeNotificationCallback) = 0;

    virtual HRESULT STDMETHODCALLTYPE LogoffNotification(
        DWORD hSessionId) = 0;
};