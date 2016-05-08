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
//      SetupUI.cpp
//
//  Abstract:
//      Implementation for the setup UI object that displays UI when the user
//      chooses to activate a device's partnership.
//
//////////////////////////////////////////////////////////////////////////////

#include "Pch.h"
#include "SetupUI.h"
#include "MyDeviceSyncHandler.h"
#include "Resources.h"

//////////////////////////////////////////////////////////////////////////////
// class CMyDeviceSetupUI
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//
//  Description:
//      Create an instance of the setup UI class.
//
//  Parameters:
//      punkOuter       - Outer IUnknown for aggregation.
//      riid            - ID of interface to return pointer to.
//      pDeviceInfo     - Info about the device to create the handler for.
//      ppv             - Interface pointer.
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      E_OUTOFMEMORY   - Error allocating the object.
//      Other HRESULTs  - Error querying for requested interface.
//
//----------------------------------------------------------------------------
HRESULT CMyDeviceSetupUI_CreateInstance(__in CMyDeviceSyncHandler *pSyncHandler, __in REFIID riid, __deref_out void **ppv)
{
    *ppv = NULL;

    HRESULT hr = E_OUTOFMEMORY;
    CMyDeviceSetupUI *pSetupUI = new CMyDeviceSetupUI(pSyncHandler);
    if (pSetupUI != NULL)
    {
        hr = pSetupUI->QueryInterface(riid, ppv);
        pSetupUI->Release();
    }

    return hr;

} //*** CMyDeviceSetupUI_CreateInstance

//----------------------------------------------------------------------------
//
//  Description:
//      Constructor.
//
//  Parameters:
//      pSyncHandler    - Handler being setup.
//
//----------------------------------------------------------------------------
CMyDeviceSetupUI::CMyDeviceSetupUI(__in CMyDeviceSyncHandler *pSyncHandler)
    :
    _cRef(1),
    _pSyncHandler(pSyncHandler)
{
    DllAddRef();
    _pSyncHandler->AddRef();

} //*** CMyDeviceSetupUI::CMyDeviceSetupUI

//----------------------------------------------------------------------------
//
//  Description:
//      Destructor.
//
//----------------------------------------------------------------------------
CMyDeviceSetupUI::~CMyDeviceSetupUI()
{
    if (_pSyncHandler != NULL)
    {
        _pSyncHandler->Release();
    }
    DllRelease();

} //*** destructor CMyDeviceSetupUI::~CMyDeviceSetupUI

//----------------------------------------------------------------------------
// IUnknown (CMyDeviceSetupUI)
//----------------------------------------------------------------------------

STDMETHODIMP CMyDeviceSetupUI::QueryInterface(__in REFIID riid, __deref_out void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CMyDeviceSetupUI, ISyncMgrUIOperation),
        { 0 },
    };

    return QISearch(this, qit, riid, ppv);

} //*** CMyDeviceSetupUI::QueryInterface

//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CMyDeviceSetupUI::Release()
{
    ULONG cRef = InterlockedDecrement(&_cRef);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;

} //*** CMyDeviceSetupUI::Release

//----------------------------------------------------------------------------
// ISyncMgrUIOperation (CMyDeviceSetupUI)
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center to display UI to allow the user to setup the
//      sync partnership.
//
//  Implements: ISyncMgrUIOperation
//
//  Parameters:
//      hwndOwner   - Owner window to host the UI on.
//
//  Return Values:
//      S_OK                - Operation completed successfully.
//      Failure HRESULT     - Operation failed.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSetupUI::Run(__in HWND hwndOwner)
{
    LPWSTR pszDeviceName = NULL;
    HRESULT hr = _pSyncHandler->GetName(&pszDeviceName);
    if (SUCCEEDED(hr))
    {
        LPWSTR pszTitle = NULL;
        hr = FormatString(g_hmodThisDll, IDS_SETUP_TITLE, &pszTitle);
        if (SUCCEEDED(hr))
        {
            LPWSTR pszQuestion = NULL;
            hr = FormatString(g_hmodThisDll, IDS_SETUP_QUESTION, &pszQuestion, pszDeviceName);
            if (SUCCEEDED(hr))
            {
                int nResult = MessageBoxW(hwndOwner, pszQuestion, pszTitle, (MB_YESNO | MB_ICONQUESTION));
                if (nResult != IDYES)
                {
                    hr = E_ABORT;
                }
                LocalFree(pszQuestion);
            }
            LocalFree(pszTitle);
        } // if: title string loaded successfully
        CoTaskMemFree(pszDeviceName);
    } // if: device name retrieved from the handler successfully

    return hr;

} //*** CMyDeviceSetupUI::Run
