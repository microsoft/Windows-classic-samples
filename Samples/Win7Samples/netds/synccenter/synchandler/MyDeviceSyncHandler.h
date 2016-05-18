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
//      MyDeviceSyncHandler.h
//
//  Abstract:
//      Header file for the device handler classes loaded by Sync Center.
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "MyDeviceHandlerCollection.h"
#include "MyDeviceSyncItem.h"
#include "BrowseUI.h"

//----------------------------------------------------------------------------
// Forward Class Declarations
//----------------------------------------------------------------------------

class CMyDeviceSyncHandler;
class CEnumSyncMgrItems;

//----------------------------------------------------------------------------
// Global Function Prototypes
//----------------------------------------------------------------------------

HRESULT CMyDeviceSyncHandler_CreateInstance(
    __in_opt    IUnknown        *punkOuter,
    __in        REFIID           riid,
    __in        SYNCDEVICEINFO  *pDeviceInfo,
    __deref_out void           **ppv);
HRESULT CEnumSyncMgrItems_CreateInstance(
    __inout     CMyDeviceSyncHandler *pDeviceHandler,
    __in        REFIID                riid,
    __deref_out void                **ppv);


//////////////////////////////////////////////////////////////////////////////
//
// Class which implements a sync handler for a device.
//
//////////////////////////////////////////////////////////////////////////////
class CMyDeviceSyncHandler
    :
    public ISyncMgrHandler,
    public ISyncMgrSyncItemContainer,
    public ISyncMgrHandlerInfo
{
public:
    CMyDeviceSyncHandler(__in const SYNCDEVICEINFO *pDeviceInfo);

    // IUnknown
    IFACEMETHODIMP QueryInterface(__in REFIID riid, __deref_out void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef()     { return InterlockedIncrement(&_cRef); }
    IFACEMETHODIMP_(ULONG) Release();

    // ISyncMgrHandler
    IFACEMETHODIMP GetName(__deref_out LPWSTR *ppszName);
    IFACEMETHODIMP GetHandlerInfo(__deref_out ISyncMgrHandlerInfo **ppHandlerInfo);
    IFACEMETHODIMP GetObject(__in REFGUID rguidObjectID, __in REFIID riid, __deref_out void **ppv);
    IFACEMETHODIMP GetCapabilities(__out SYNCMGR_HANDLER_CAPABILITIES *pmCapabilities);
    IFACEMETHODIMP GetPolicies(__out SYNCMGR_HANDLER_POLICIES *pmPolicies);
    IFACEMETHODIMP Activate(__in BOOL fActivate);
    IFACEMETHODIMP Enable(__in BOOL fEnable);
    IFACEMETHODIMP Synchronize(
        __in_ecount(cItems) LPCWSTR                 *ppszItemIDs,
        __in                ULONG                    cItems,
        __in                HWND                     hwndOwner,
        __in                ISyncMgrSessionCreator  *pCreator,
        __in_opt            IUnknown                *punk);

    // ISyncMgrSyncItemContainer
    IFACEMETHODIMP GetSyncItem(__in LPCWSTR pszItemID, __deref_out ISyncMgrSyncItem **ppItem);
    IFACEMETHODIMP GetSyncItemEnumerator(__deref_out IEnumSyncMgrSyncItems **ppenum);
    IFACEMETHODIMP GetSyncItemCount(__out ULONG *pcItems);

    // ISyncMgrHandlerInfo
    IFACEMETHODIMP GetType(__out SYNCMGR_HANDLER_TYPE *pnType);
    IFACEMETHODIMP GetTypeLabel(__deref_out LPWSTR *ppszLabel);
    IFACEMETHODIMP GetComment(__deref_out LPWSTR *ppszComment);
    IFACEMETHODIMP GetLastSyncTime(__out FILETIME *pftLastSync);
    IFACEMETHODIMP IsActive();
    IFACEMETHODIMP IsEnabled();
    IFACEMETHODIMP IsConnected();

private:
    ~CMyDeviceSyncHandler();

    HRESULT _LoadItems();
    CMyDeviceSyncItem *_FindItem(__in LPCWSTR pszItemID);

    static void _ReportItemProgress(
        __in  ISyncMgrSyncCallback       *pCallback,
        __in  LPCWSTR                     pszItemID,
        __in  UINT                        nFormatStringID,
        __in  SYNCMGR_PROGRESS_STATUS     nStatus,
        __in  ULONG                       uCurrentStep,
        __in  ULONG                       uMaxStep,
        __out SYNCMGR_CANCEL_REQUEST     *pnCancelRequest,
        ...);
    static void _SetHandlerProgressText(
        __in  ISyncMgrSyncCallback       *pCallback,
        __in  UINT                        nProgressTextID,
        __out SYNCMGR_CANCEL_REQUEST     *pnCancelRequest,
        ...);
    static void CMyDeviceSyncHandler::_ReportEvent(
        __in     ISyncMgrSyncCallback   *pCallback,
        __in     LPCWSTR                 pszItemID,
        __in     UINT                    nNameStringID,
        __in     UINT                    nFormatStringID,
        __in     SYNCMGR_EVENT_LEVEL     nLevel,
        __in     SYNCMGR_EVENT_FLAGS     nFlags,
        __in_opt LPCWSTR                 pszLinkText,
        __in_opt LPCWSTR                 pszLinkReference,
        __in_opt LPCWSTR                 pszContext,
        __out    GUID                   *pguidEventID,
        ...);

    /////////////////////
    // Member Variables
    /////////////////////
    LONG                          _cRef;
    DWORD                         _cItems;
    CMyDeviceSyncItem           **_ppItems;

    DWORD                         _nPartnerID;
    WCHAR                         _szHandlerID[MAX_SYNCMGR_ID];
    WCHAR                         _szDeviceName[MAX_SYNCMGR_NAME];
    LPWSTR                        _pszPartnershipName;

    // Friend Declarations
    friend class CEnumSyncMgrItems;

}; //*** class CMyDeviceSyncHandler

//////////////////////////////////////////////////////////////////////////////
//
// Enumeration of all the items managed by devices.
//
//////////////////////////////////////////////////////////////////////////////
class CEnumSyncMgrItems : public IEnumSyncMgrSyncItems
{
public:
    CEnumSyncMgrItems(__inout CMyDeviceSyncHandler *pDeviceHandler) :
        _cRef(1),
        _iCur(0),
        _pDeviceHandler(pDeviceHandler)
    {
        DllAddRef();
        _pDeviceHandler->AddRef();
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(__in REFIID riid, __deref_out void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&_cRef); }
    IFACEMETHODIMP_(ULONG) Release();

    // IEnumSyncMgrSyncItems
    IFACEMETHODIMP Next(__in ULONG celt, __deref_out_ecount(celt) ISyncMgrSyncItem *rgelt[], __out_opt ULONG *pceltFetched);
    IFACEMETHODIMP Skip(__in ULONG celt)
    {
        UNREFERENCED_PARAMETER(celt);
        return E_NOTIMPL;
    }
    IFACEMETHODIMP Reset()
    {
        _iCur = 0;
        return S_OK;
    }
    IFACEMETHODIMP Clone(__deref_out IEnumSyncMgrSyncItems **ppenum)
    {
        *ppenum = NULL;
        return E_NOTIMPL;
    }

private:
    ~CEnumSyncMgrItems()
    {
        if (_pDeviceHandler != NULL)
        {
            _pDeviceHandler->Release();
        }
        DllRelease();
    }

    /////////////////////
    // Member Variables
    /////////////////////
    LONG                   _cRef;
    DWORD                  _iCur;
    CMyDeviceSyncHandler  *_pDeviceHandler;

}; //*** class CEnumSyncMgrSyncItems
