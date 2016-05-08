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
//      MyDeviceSyncItem.cpp
//
//  Abstract:
//      Implementation of the Sync Center sync item for a device.
//
//////////////////////////////////////////////////////////////////////////////

#include "Pch.h"
#include "MyDeviceSyncItem.h"
#include "MyDeviceSyncHandler.h"
#include "BrowseUI.h"
#include "Guids.h"              // Declare the GUIDs used by this component.
#include "Helpers.h"
#include "Resources.h"

//////////////////////////////////////////////////////////////////////////////
// class CMyDeviceSyncItem
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//
//  Description:
//      Create an instance of the sync item class.
//
//  Parameters:
//      pszItemID       - ID of the item.
//      pszItemName     - Name of the item.
//      pszItemLabel    - Label of the item.
//      pszHandlerID    - ID of the handler creating the item
//      folderID        - KNOWNFOLDERID of the folder which the item represents
//      uSyncTextID     - String to be displayed during item syncronization
//      ppSyncItem      - Sync item object being returned.
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      E_OUTOFMEMORY   - Error allocating the object.
//      Other HRESULTs  - Error querying for requested interface.
//
//----------------------------------------------------------------------------
HRESULT CMyDeviceSyncItem_CreateInstance(
    __in        LPCWSTR                  pszItemID,
    __in        LPCWSTR                  pszItemName,
    __in        LPCWSTR                  pszItemLabel,
    __in        LPCWSTR                  pszHandlerID,
    __in        KNOWNFOLDERID            folderID,
    __in        UINT                     uSyncTextID,
    __deref_out CMyDeviceSyncItem      **ppSyncItem)
{
    HRESULT hr = S_OK;
    *ppSyncItem = new CMyDeviceSyncItem(pszItemID, pszItemName, pszItemLabel, pszHandlerID, folderID, uSyncTextID);
    if (*ppSyncItem == NULL)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        hr = (*ppSyncItem)->RegisterIconLocation();
    }

    return hr;

} //*** CMyDeviceSyncItem_CreateInstance

//----------------------------------------------------------------------------
//
//  Description:
//      Constructor.
//
//  Parameters:
//      pszItemID       - ID of the item.
//      pszItemName     - Name of the item.
//      pszItemLabel    - Label of the item.
//      pszHandlerID    - ID of the handler creating the item.
//      folderID        - KNOWNFOLDERID of the folder which the item represents.
//      uSyncTextID     - String to be displayed during item syncronization.
//
//----------------------------------------------------------------------------
CMyDeviceSyncItem::CMyDeviceSyncItem(
    __in LPCWSTR         pszItemID,
    __in LPCWSTR         pszItemName,
    __in LPCWSTR         pszItemLabel,
    __in LPCWSTR         pszHandlerID,
    __in KNOWNFOLDERID   folderID,
    __in UINT            uSyncTextID)
    :
    _cRef(1),
    _folderID(folderID),
    _uSyncTextID(uSyncTextID)
{
    DllAddRef();

    // Truncation should NEVER happen.
    assert(lstrlenW(pszItemID) < ARRAYSIZE(_szItemID));
    StringCchCopyW(_szItemID, ARRAYSIZE(_szItemID), pszItemID);
    assert(lstrlenW(pszHandlerID) < ARRAYSIZE(_szHandlerID));
    StringCchCopyW(_szHandlerID, ARRAYSIZE(_szHandlerID), pszHandlerID);

    // Truncation is okay.
    StringCchCopyW(_szItemName, ARRAYSIZE(_szItemName), pszItemName);
    StringCchCopyW(_szItemLabel, ARRAYSIZE(_szItemLabel), pszItemLabel);

} //*** constructor CMyDeviceSyncItem::CMyDeviceSyncItem

//----------------------------------------------------------------------------
// IUnknown (CMyDeviceSyncItem)
//----------------------------------------------------------------------------

STDMETHODIMP CMyDeviceSyncItem::QueryInterface(__in REFIID riid, __deref_out void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CMyDeviceSyncItem, ISyncMgrSyncItem),
        QITABENT(CMyDeviceSyncItem, ISyncMgrSyncItemInfo),
        { 0 },
    };

    return QISearch(this, qit, riid, ppv);

} //*** CMyDeviceSyncSyncItem::QueryInterface

//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CMyDeviceSyncItem::Release()
{
    ULONG cRef = InterlockedDecrement(&_cRef);
    if (cRef == 0)
    {
        delete this;
    }

    return cRef;

} //*** CMyDeviceSyncItem::Release

//----------------------------------------------------------------------------
// ISyncMgrSyncItem (CMyDeviceSyncItem)
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center to get the ID of the Item.
//
//  Implements: ISyncMgrSyncItem
//
//  Parameters:
//      ppszItemID       - ID of item being returned.
//
//  Return Values:
//      S_OK             - Operation completed successfully.
//      Other HRESULTs.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncItem::GetItemID(__deref_out LPWSTR *ppszItemID)
{
    // Duplicate the item ID string for the caller.
    HRESULT hr = SHStrDupW(_szItemID, ppszItemID);

    return hr;

} //*** CMyDeviceSyncItem::GetItemID

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center to get the name of the item.  This name will be
//      displayed in the handler's folder.
//
//  Parameters:
//      ppszName    - Name of the item being returned.
//
//  Return Values:
//      S_OK        - Operation completed successfully.
//      Other HRESULTs.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncItem::GetName(__deref_out LPWSTR *ppszName)
{
    *ppszName = NULL;

    // Duplicate the name for the caller.
    HRESULT hr = SHStrDupW(_szItemName, ppszName);

    return hr;

} //*** CMyDeviceSyncItem::GetName

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center to get information about the sync item.
//
//  Implements: ISyncMgrSyncItem
//
//  Parameters:
//      ppItemInfo  - Interface for getting sync item info to return to Sync Center.
//
//  Return Values:
//      S_OK        - Operation completed successfully.
//      Other HRESULTs.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncItem::GetItemInfo(__deref_out ISyncMgrSyncItemInfo **ppItemInfo)
{
    HRESULT hr = QueryInterface(IID_PPV_ARGS(ppItemInfo));
    return hr;

} //*** CMyDeviceSyncItem::GetItemInfo

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center to get an interface pointer for a specific
//      type of object.
//
//  Implements: ISyncMgrSyncItem
//
//  Parameters:
//      rguidObjectID   - GUID for the item to get the object for.
//      riid            - Interface to get.
//      ppv             - Interface returned to Sync Center.
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      E_NOTIMPL       - Not implemented by the sync item.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncItem::GetObject(
    __in        REFGUID    rguidObjectID,
    __in        REFIID     riid,
    __deref_out void     **ppv)
{
    HRESULT hr = E_NOTIMPL;
    *ppv = NULL;

    if (rguidObjectID == SYNCMGR_OBJECTID_BrowseContent)
    {
        hr = CBrowseUI_CreateInstance(_folderID, riid, ppv);
    }

    return hr;

} //*** CMyDeviceSyncItem::GetObject

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center to get the mask of capabilities of the item.
//      The following capabilities are defined:
//
//      SYNCMGR_ICM_NONE
//          No capabilities are specified.
//
//      SYNCMGR_ICM_PROVIDES_ICON
//          Indicates that the item will return a valid object from the
//          GetObject() method when SYNCMGR_OBJECTID_Icon is specified.
//
//      SYNCMGR_ICM_EVENT_STORE
//          Indicates that the item will return a valid object from the
//          GetObject() method when SYNCMGR_OBJECTID_EventStore is specified.
//
//      SYNCMGR_ICM_CONFLICT_STORE
//          Indicates that the item will return a valid object from the
//          GetObject() method when SYNCMGR_OBJECTID_ConflictStore is
//          specified.
//
//      SYNCMGR_ICM_CAN_DELETE
//          Indicates that the item wants to allow the user to be able to
//          delete it from the handler's folder.  This can be used by an item
//          to remove that item from the handler's sync set (e.g. remove
//          a folder from the set of Offline Files).
//
//          If this value is set the Delete task will be shown in the
//          handler's folder when this item is selected.
//
//      SYNCMGR_ICM_BROWSE_CONTENT
//          Indicates that the item will return a valid object from the
//          GetObject() method when SYNCMGR_OBJECTID_BrowseContent is
//          specified.  If this value is set, the Browse Content task will be
//          added to the context menu for the item.
//
//      SYNCMGR_ICM_QUERY_BEFORE_ENABLE
//          Indicates that the item will return a valid object from the
//          GetObject() method when SYNCMGR_OBJECTID_QueryBeforeEnable is
//          specified.
//
//      SYNCMGR_ICM_QUERY_BEFORE_DISABLE
//          Indicates that the item will return a valid object from the
//          GetObject() method when SYNCMGR_OBJECTID_QueryBeforeDisable is
//          specified.
//
//      SYNCMGR_ICM_QUERY_BEFORE_DELETE
//          Indicates that the item will return a valid object from the
//          GetObject() method when SYNCMGR_OBJECTID_QueryBeforeDelete is
//          specified.
//
//  Implements: ISyncMgrSyncItem
//
//  Parameters:
//      pmCapabilities  - Capabilities mask being returned.
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncItem::GetCapabilities(__out SYNCMGR_ITEM_CAPABILITIES *pmCapabilities)
{
    *pmCapabilities = SYNCMGR_ICM_CAN_BROWSE_CONTENT;
    return S_OK;

} //*** CMyDeviceSyncItem::GetCapabilities

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center to get the mask of policies of the item.
//      The following policies are defined:
//
//      SYNCMGR_IPM_NONE
//          No policies are specified.
//
//      SYNCMGR_IPM_PREVENT_ENABLE
//          Indicates that enabling the item is not supported.  This can be
//          used by an item to implement support for group policy that
//          prevents an item from being enabled.  If this value is set the
//          Enable task will not be shown in the handler's folder when this
//          item is selected.  The item should provide a comment (returned
//          from its implement of the GetComment() method on
//          ISyncMgrSyncItemInfo) to let the user know why the Enable task is
//          not available.  Most items should never set this value.
//
//      SYNCMGR_IC_PREVENT_DISABLE
//          Indicates that disabling the item is not supported.  This can be
//          used by an item to implement support for group policy that forces
//          an item to always be able to sync.  If this value is set the
//          Disable task will not be shown in the handler's folder when this
//          item is selected.  The item should provide a comment (returned
//          from its implementation of the GetComment() method on
//          ISyncMgrSyncItemInfo) to let the user know why the Disable task is
//          not available.  Most items should never set this value.
//
//      SYNCMGR_IPM_PREVENT_START_SYNC
//          Indicates that starting a sync through the user interface or
//          through the APIs is not supported.  Sync can only be started by
//          an external application that creates a session creator to report
//          progress.  If this value is set, the Start Sync task will not be
//          shown in the handler's folder when the sync item is selected.
//          Note that Start Sync must be supported on a handler in order for
//          it to be supported on a sync item.  Most sync items should not set
//          this value.
//
//      SYNCMGR_IPM_PREVENT_STOP_SYNC
//          Indicates that stopping a sync through the user interface or
//          through the APIs is not supported.  If this value is set, the
//          Stop Sync task will not be shown in the handler's folder when the
//          sync item is selected.  Note that Stop Sync must be supported on a
//          handler in order for it to be supported on a sync item.  Most sync
//          items should not set this value.
//
//      SYNCMGR_IPM_DISABLE_ENABLE
//          Indicates that the enable task should be disabled when it is
//          shown for this sync item.  With this policy set, the Enable option
//          will appear in the context menu (if SYNCMGR_IPM_PREVENT_ENABLE is
//          not set) but will be disabled.
//
//      SYNCMGR_IPM_DISABLE_DISABLE
//          Indicates that the disable task should be disabled when it is
//          shown for this sync item.  With this policy set, the Disable option
//          will appear in the context menu (if SYNCMGR_IPM_PREVENT_DISABLE is
//          not set) but will be disabled.
//
//      SYNCMGR_IPM_DISABLE_START_SYNC
//          Indicates that the Start Sync task should be disabled when it is
//          shown for this sync item.  With this policy set, the Start Sync
//          option will appear in the context menu (if
//          SYNCMGR_IPM_PREVENT_START_SYNC is not set and if
//          SYNCMGR_HPM_PREVENT_START_SYNC is not set on the handler) but will
//          be disabled.
//
//      SYNCMGR_IPM_DISABLE_STOP_SYNC
//          Indicates that the Stop Sync task should be disabled when it is
//          shown for this sync item.  With this policy set, the Stop Sync
//          option will appear in the context menu (if
//          SYNCMGR_IPM_PREVENT_STOP_SYNC is not set and if
//          SYNCMGR_HPM_PREVENT_STOP_SYNC is not set on the handler) but will
//          be disabled.
//
//      SYNCMGR_IPM_DISABLE_BROWSE
//          Indicates that the Browse task should be disabled when it is shown
//          for this sync item.  The Browse task will only be shown if the
//          SYNCMGR_ICM_CAN_BROWSE_CONTENT value is returned from the
//          GetCapabilities() method.
//
//      SYNCMGR_IPM_HIDDEN_BY_DEFAULT
//          Indicates that the item should be hidden from the user unless
//          the Show Hidden Files option has been enabled.  This policy only
//          applies the first time the item is loaded.  After that, the hidden
//          state is maintained by Sync Center and can be changed by the user
//          through the property sheet.
//
//  Implements: ISyncMgrSyncItem
//
//  Parameters:
//      pmPolicies  - Policy mask being returned.
//
//  Return Values:
//      S_OK        - Operation completed successfully.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncItem::GetPolicies(__out SYNCMGR_ITEM_POLICIES *pmPolicies)
{
    *pmPolicies = SYNCMGR_IPM_NONE;
    return S_OK;

} //*** CMyDeviceSyncItem::GetPolicies

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center when the user chooses to enable or disable the
//      sync item from the handler's folder.
//
//  Implements: ISyncMgrSyncItem
//
//  Return Values:
//      S_OK                - Sync item changed enabled state successfully.
//      Failure HRESULTs    - Sync item failed to change the enabled state.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncItem::Enable(BOOL fEnable)
{
    UNREFERENCED_PARAMETER(fEnable);
    return S_OK;

} //*** CMyDeviceSyncItem::Enable

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center when the user chooses to delete the sync item
//      from the handler's folder.
//
//  Implements: ISyncMgrSyncItem
//
//  Return Values:
//      S_OK                - Sync item was deleted successfully.
//      Failure HRESULT     - Sync item failed to delete the sync item.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncItem::Delete()
{
    return E_NOTIMPL;

} //*** CMyDeviceSyncItem::Delete

//----------------------------------------------------------------------------
// ISyncMgrSyncItemInfo (CMyDeviceSyncItem)
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center to get a label for the item type.  This is
//      usually used to display the model of the device or some other item-
//      specific identifying string.
//
//  Implements: ISyncMgrSyncItemInfo
//
//  Parameters:
//      ppszLabel
//          Pointer to fill with a pointer to a string.  Must be allocated
//          with CoTaskMemAlloc().  Sync Center will free this using
//          CoTaskMemFree().
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      E_OUTOFMEMORY   - Error allocating memory for the string buffer.
//      Other HRESULTs.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncItem::GetTypeLabel(__deref_out LPWSTR *ppszLabel)
{
    *ppszLabel = NULL;

    // Duplicate the name for the caller.
    HRESULT hr = SHStrDupW(_szItemLabel, ppszLabel);

    return hr;

} //*** CMyDeviceSyncItem::GetTypeLabel

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center to get a string to display in the folder in
//      the far right column when a synchronization is not being performed.
//      If no string is provided, a blank string will be displayed.
//
//  Parameters:
//      ppszComment
//          Pointer to fill with a pointer to a string.  Must be allocated
//          with CoTaskMemAlloc().  Sync Center will free this using
//          CoTaskMemFree().
//
//  Implements: ISyncMgrSyncItemInfo
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      E_OUTOFMEMORY   - Error allocating memory for the string buffer.
//      E_NOTIMPL       - No comment is provided by the handler.
//      Other HRESULTs.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncItem::GetComment(__deref_out LPWSTR *ppszComment)
{
    *ppszComment = NULL;
    return E_NOTIMPL;

} //*** CMyDeviceSyncItem::GetComment

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center to get the date and time the item was last
//      synchronized.  If a failure is returned, the value calculated from the
//      last synchronization will be used.
//
//  Implements: ISyncMgrSyncItemInfo
//
//  Parameters:
//      pftLastSync - Last sync time.
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      E_NOTIMPL       - No last sync time is provided by the handler.
//      Other HRESULTs.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncItem::GetLastSyncTime(__out FILETIME *pftLastSync)
{
    UNREFERENCED_PARAMETER(pftLastSync);
    return E_NOTIMPL;

} //*** CMyDeviceSyncItem::GetLastSyncTime

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center to determine whether the sync item is enabled
//      or not.
//
//      If a sync item is disabled it will not be synchronized by Sync Center
//      and many of the actions will be removed or disabled in the UI.
//
//      If a sync item doesn't want to maintain its enabled state, it can
//      return E_NOTIMPL and Sync Center will maintain it for the sync item.
//
//  Implements: ISyncMgrSyncItemInfo
//
//  Return Values:
//      S_OK        - Sync item is enabled.
//      S_FALSE     - Sync item is not enabled.
//      E_NOTIMPL   - Let Sync Center maintain the enabled state.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncItem::IsEnabled()
{
    return E_NOTIMPL;

} //*** CMyDeviceSyncItem::IsEnabled

//----------------------------------------------------------------------------
//
//  Description:
//      Called by Sync Center to determine whether the sync item is in a
//      connected state or not.
//
//      If an item is in a disconnected state it will not be synchronized by
//      Sync Center and many of the actions will be removed or disabled in
//      the UI.
//
//  Implements: ISyncMgrSyncItemInfo
//
//  Return Values:
//      S_OK        - Item is connected.
//      S_FALSE     - Item is disconnected.
//      E_NOTIMPL   - Item doesn't support this state.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceSyncItem::IsConnected()
{
    return E_NOTIMPL;

} //*** CMyDeviceSyncItem::IsConnected

//----------------------------------------------------------------------------
//
//  Description:
//      Called at sync item creation to register the item's icon in the Windows
//      registry.  If this fails, Sync Center will use the GetObject() method
//      to query for its icon.
//
//  Return Values:
//      S_OK            - Icon was successfully registered.
//      Other HRESULTs  - Icon registration failed.
//
//----------------------------------------------------------------------------
HRESULT CMyDeviceSyncItem::RegisterIconLocation()
{
    HKEY hkey;
    HRESULT hr = _OpenIconLocationRegKey(&hkey);
    if (SUCCEEDED(hr))
    {
        // First check if the value on the DefaultIcon key exists.
        // If it does, check the type and length.  If it is a
        // string value and is not empty we consider it a valid
        // icon location.  Otherwise, we will attempt to write
        // the icon path there.

        // MAX_PATH+12 to make room for the resource
        // number that is stored after the path
        WCHAR szIconLocation[MAX_PATH+12] = { 0 };

        hr = _QueryIconLocationRegValue(hkey, szIconLocation, ARRAYSIZE(szIconLocation));
        if (FAILED(hr))
        {
            // Icon location not yet registered or we found invalid data.
            // [Re]register the icon location.
            hr = _GetIconLocation(szIconLocation, ARRAYSIZE(szIconLocation));
            if (SUCCEEDED(hr))
            {
                hr = _WriteIconLocationRegValue(hkey, szIconLocation);
            }
        }

        RegCloseKey(hkey);
    }

    return hr;

}//*** CMyDeviceSyncItem::RegisterIconLocation

//----------------------------------------------------------------------------
//
//  Description:
//      Binds to the shell item underlying the item's known folder and calls
//      GetUIObjectOf.  This is used to get an IExtractIconW interface for the
//      sync item's known folder.
//
//  Parameters:
//      riid        - IID of the interface to be passed to GetUIObjectOf.
//      ppv         - Pointer to the interface to be returned.
//
//  Return Values:
//      S_OK           - Pointer was successfully found.
//      Other HRESULTs - Pointer was not found.
//
//----------------------------------------------------------------------------
HRESULT CMyDeviceSyncItem::GetShellItemObject(__in REFIID riid, __out void **ppv)
{
    LPWSTR pszPath = NULL;

    HRESULT hr = SHGetKnownFolderPath(
        _folderID,
        0,           /* dwFlags */
        NULL,        /* hToken */
        &pszPath);

    if (SUCCEEDED(hr))
    {
        PIDLIST_ABSOLUTE pidlFull;
        ULONG ulAttr = 0;
        hr = SHILCreateFromPath(pszPath, &pidlFull, &ulAttr);
        if (SUCCEEDED(hr))
        {
            // Bind to the shell item's parent and then call GetUIObjectOf
            IShellFolder *psf;
            PCIDLIST_RELATIVE pidlItem;
            hr = ::SHBindToFolderIDListParent(NULL, pidlFull, IID_PPV_ARGS(&psf), &pidlItem);
            if (SUCCEEDED(hr))
            {
                hr = psf->GetUIObjectOf(NULL, 1, &pidlItem, riid, NULL, ppv);
                psf->Release();
            }

            ILFree(pidlFull);
        }

        CoTaskMemFree(pszPath);
    }

    return hr;

}//*** CMyDeviceSyncItem::GetShellItemObject

//----------------------------------------------------------------------------
//
//  Description:
//      Opens the registry key used by Sync Center for registering sync item
//      icons with read/write privileges .  If the key does not
//      exist, it is created.
//
//  Parameters:
//      phkey       - Open registry key to be returned.
//
//  Return Values:
//      S_OK                - Key was successfully opened.
//      Other HRESULTs      - Key was not opened.
//
//----------------------------------------------------------------------------
HRESULT CMyDeviceSyncItem::_OpenIconLocationRegKey(__out HKEY *phkey)
{
    *phkey = NULL;

    LPWSTR pszKey = NULL;
    HRESULT hr = FormatString(L"Software\\Microsoft\\Windows\\CurrentVersion\\SyncMgr\\HandlerInstances\\%1\\SyncItems\\%2\\DefaultIcon",
                              &pszKey,
                              _szHandlerID,
                              _szItemID);

    if (SUCCEEDED(hr))
    {
        DWORD nStatus = RegCreateKeyExW(HKEY_CURRENT_USER,
                                        pszKey,
                                        0,
                                        NULL,
                                        0,
                                        (KEY_WRITE | KEY_READ),
                                        NULL,
                                        phkey,
                                        NULL);

        if (nStatus != ERROR_SUCCESS)
        {
            hr = HRESULT_FROM_WIN32(nStatus);
        }

        LocalFree(pszKey);
    }

    return hr;

}//*** CMyDeviceSyncItem::_OpenIconLocationRegKey

//----------------------------------------------------------------------------
//
//  Description:
//      Querys the value of the specified registry key.  This is used to
//      check the value of the key containing the path to the icon.  If
//      the value is not a string, or if it is empty, the value is certainly
//      invalid and the method fails.
//
//  Parameters:
//      hkey        - The open registry key to be queried.
//      pszLocation - The buffer which will be filled with the value of the registry key.
//      cchLocation - The size of the buffer pointed to by pszLocation, in characters.
//
//  Return Values:
//      S_OK                - Value was successfully queried.
//      ERROR_INVALID_DATA  - Registry key contains invalid data
//      ERROR_NOT_FOUND     - Registry key contains no data
//      Other HRESULTs      - The value queried was invalid.
//
//----------------------------------------------------------------------------
HRESULT CMyDeviceSyncItem::_QueryIconLocationRegValue(
    __in                       HKEY      hkey,
    __out_ecount(cchLocation)  LPWSTR    pszLocation,
    __in                       size_t    cchLocation)
{
    DWORD dwType;
    DWORD cbIconLocation = (DWORD)(cchLocation * sizeof(WCHAR));
    DWORD dwResult = RegQueryValueExW(hkey,
                                      NULL,
                                      NULL,
                                      &dwType,
                                      (LPBYTE)pszLocation,
                                      &cbIconLocation);

    if (dwResult == ERROR_SUCCESS)
    {
        if (!(REG_SZ == dwType || REG_EXPAND_SZ == dwType))
        {
            // If the value is not a string type we want to write a new value.
            dwResult = ERROR_INVALID_DATA;
        }
        else if (pszLocation[0] == L'\0')
        {
            // If the value is empty we want to write a new value.
            dwResult = ERROR_NOT_FOUND;
        }

        // Ensure the output string is null-terminated.
        pszLocation[cchLocation - 1] = 0;
    }

    return HRESULT_FROM_WIN32(dwResult);

}//*** CMyDeviceSyncItem::_QueryIconLocationRegValue

//----------------------------------------------------------------------------
//
//  Description:
//      Retrieves the path to the file which contains the icon used in
//      Explorer to represent the folder which this Item synchronizes
//      (My Music, My Documents, etc).
//
//  Parameters:
//      pszLocation - Buffer to be filled with the path to the file which holds the icon.
//      cchLocation - Size of the buffer pointed to by pszLocation, in characters.
//
//  Return Values:
//      S_OK             - Path to icon was found.
//      Other HRESULTs   - Path to icon was not found.
//
//----------------------------------------------------------------------------
HRESULT CMyDeviceSyncItem::_GetIconLocation(
    __out_ecount(cchLocation)     LPWSTR      pszLocation,
    __in                          size_t      cchLocation
    )
{
    IExtractIconW *pei = NULL;
    HRESULT hr = GetShellItemObject(IID_PPV_ARGS(&pei));
    if (SUCCEEDED(hr))
    {
        int iIconIndex;
        UINT uFlags;
        hr = pei->GetIconLocation(GIL_FORSHELL, pszLocation, (UINT)cchLocation, &iIconIndex, &uFlags);
        if (SUCCEEDED(hr))
        {
            // Format the string in the normal icon location syntax:
            //
            //    c:\dir1\dir2\resfile.dll,<icon index>
            //
            // Note that the icon index normally comes back from the shell as
            // a negative number indicating an icon resource ID, not an index.
            // Our code here will automatically include that minus sign which
            // is required when the number represents a resource ID.
            hr = StringCchPrintfW(pszLocation, MAX_PATH+12, L"%s,%d", pszLocation, iIconIndex);
        }

        pei->Release();
    }

    return hr;

}//*** CMyDeviceSyncItem::_GetIconLocation

//----------------------------------------------------------------------------
//
//  Description:
//      Used to write the path of the icon to the registry.
//
//  Parameters:
//      hkey        - Key (opened with write privilges) to write the path to.
//      pszLocation - String containing the path to the icon.
//
//  Return Values:
//      S_OK           - Value was successfully written.
//      Other HRESULTs - Value was not written.
//
//----------------------------------------------------------------------------
HRESULT CMyDeviceSyncItem::_WriteIconLocationRegValue(__in HKEY hkey, __in LPCWSTR pszLocation)
{
    // Write the icon location string.  Include the nul-terminator in the
    // size of the written buffer.
    HRESULT hr = S_OK;
    DWORD cbLocation = (lstrlenW(pszLocation) + 1) * sizeof(WCHAR);
    DWORD dwResult = RegSetValueExW(hkey,
                                    NULL,
                                    0,
                                    REG_EXPAND_SZ,
                                    (const BYTE *)pszLocation,
                                    cbLocation);

    if (dwResult != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(dwResult);
    }

    return hr;

}//*** CMyDeviceSyncItem::_WriteIconLocationRegValue


