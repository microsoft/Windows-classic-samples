//////////////////////////////////////////////////////////////////////////////
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Module Name:
//      SetupUI.h
//
//  Abstract:
//      Header file for the setup UI object that displays UI when the user
//      chooses to activate a device's partnership.
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

//----------------------------------------------------------------------------
// Forward Class Declarations
//----------------------------------------------------------------------------

class CMyDeviceSetupUI;

//----------------------------------------------------------------------------
// External Class Declarations
//----------------------------------------------------------------------------

class CMyDeviceSyncHandler;

//----------------------------------------------------------------------------
// Global Function Prototypes
//----------------------------------------------------------------------------

HRESULT CMyDeviceSetupUI_CreateInstance(__in CMyDeviceSyncHandler *pSyncHandler, __in REFIID riid, __deref_out void **ppv);

//////////////////////////////////////////////////////////////////////////////
//
// Class which provides UI to setup a device's sync partnership.
//
//////////////////////////////////////////////////////////////////////////////
class CMyDeviceSetupUI : public ISyncMgrUIOperation
{
public:
    CMyDeviceSetupUI(__in CMyDeviceSyncHandler *pSyncHandler);

    // IUnknown
    IFACEMETHODIMP QueryInterface(__in REFIID riid, __deref_out void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef()     { return InterlockedIncrement(&_cRef); }
    IFACEMETHODIMP_(ULONG) Release();

    // ISyncMgrUIOperation
    IFACEMETHODIMP Run(__in HWND hwndOwner);

private:
    ~CMyDeviceSetupUI();

    /////////////////////
    // Member Variables
    /////////////////////
    LONG                     _cRef;
    CMyDeviceSyncHandler    *_pSyncHandler;

}; //*** class CMyDeviceSetupUI
