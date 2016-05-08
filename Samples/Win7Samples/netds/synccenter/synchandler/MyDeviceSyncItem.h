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
//      MyDeviceSyncItem.h
//
//  Abstract:
//      Header file for the classes implementing a sync item for the device
//      handler.
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "BrowseUI.h"

//----------------------------------------------------------------------------
// Forward Class Declarations
//----------------------------------------------------------------------------

class CMyDeviceSyncItem;

//----------------------------------------------------------------------------
// Global Function Prototypes
//----------------------------------------------------------------------------

HRESULT CMyDeviceSyncItem_CreateInstance(
    __in        LPCWSTR               pszItemID,
    __in        LPCWSTR               pszItemName,
    __in        LPCWSTR               pszItemLabel,
    __in        LPCWSTR               pszHandlerID,
    __in        KNOWNFOLDERID         folderID,
    __in        UINT                  uSyncTextID,
    __deref_out CMyDeviceSyncItem   **ppSyncItem);

//////////////////////////////////////////////////////////////////////////////
//
// Class which implements a sync handler for a device.
//
//////////////////////////////////////////////////////////////////////////////
class CMyDeviceSyncItem
    :
    public ISyncMgrSyncItem,
    public ISyncMgrSyncItemInfo
{
public:
    CMyDeviceSyncItem(
        __in LPCWSTR       pszItemID,
        __in LPCWSTR       pszItemName,
        __in LPCWSTR       pszItemLabel,
        __in LPCWSTR       pszHandlerID,
        __in KNOWNFOLDERID folderID,
        __in UINT          uSyncTextID);

    // IUnknown
    IFACEMETHODIMP QueryInterface(__in REFIID riid, __deref_out void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef()     { return InterlockedIncrement(&_cRef); }
    IFACEMETHODIMP_(ULONG) Release();

    // ISyncMgrSyncItem
    IFACEMETHODIMP GetItemID(__deref_out LPWSTR *ppszItemID);
    IFACEMETHODIMP GetName(__deref_out LPWSTR *ppszName);
    IFACEMETHODIMP GetItemInfo(__deref_out ISyncMgrSyncItemInfo **ppItemInfo);
    IFACEMETHODIMP GetObject(__in REFGUID rguidObjectID, __in REFIID riid, __deref_out void **ppv);
    IFACEMETHODIMP GetCapabilities(__out SYNCMGR_ITEM_CAPABILITIES *pmCapabilities);
    IFACEMETHODIMP GetPolicies(__out SYNCMGR_ITEM_POLICIES *pmPolicies);
    IFACEMETHODIMP Enable(BOOL fEnable);
    IFACEMETHODIMP Delete();

    // ISyncMgrSyncItemInfo
    IFACEMETHODIMP GetTypeLabel(__deref_out LPWSTR *ppszTypeLabel);
    IFACEMETHODIMP GetComment(__deref_out LPWSTR *ppszComment);
    IFACEMETHODIMP GetLastSyncTime(__out FILETIME *pftLastSync);
    IFACEMETHODIMP IsEnabled();
    IFACEMETHODIMP IsConnected();

    HRESULT RegisterIconLocation();

private:
    ~CMyDeviceSyncItem()
    {
        DllRelease();
    } //*** destructor ~CMyDeviceSyncItem

    // Accessor methods for accessing private data directly so it doesn't
    // have to be copied.
    LPCWSTR GetItemIDPointer()      { return _szItemID; }
    LPCWSTR GetNamePointer()    { return _szItemName; }

    UINT GetSyncTextID()  { return _uSyncTextID; }
    HRESULT GetShellItemObject(__in REFIID riid, __deref_out void **ppv);

    HRESULT _OpenIconLocationRegKey(__out HKEY *phkey);
    HRESULT _QueryIconLocationRegValue(__in HKEY hkey, __out_ecount(cchLocation) LPWSTR pszLocation, __in size_t cchLocation);
    HRESULT _GetIconLocation(__out_ecount(cchLocation) LPWSTR pszLocation, __in size_t cchLocation);
    HRESULT _WriteIconLocationRegValue(__in HKEY hkey, __in LPCWSTR pszLocation);

    /////////////////////
    // Member Variables
    /////////////////////
    LONG            _cRef;
    WCHAR           _szItemID[MAX_SYNCMGR_ID];
    WCHAR           _szItemName[MAX_PATH];
    WCHAR           _szItemLabel[MAX_SYNCMGR_NAME];
    WCHAR           _szHandlerID[MAX_SYNCMGR_ID];
    UINT            _uSyncTextID;
    KNOWNFOLDERID   _folderID;

    friend class CMyDeviceSyncHandler;

}; //*** class CMyDeviceSyncItem
