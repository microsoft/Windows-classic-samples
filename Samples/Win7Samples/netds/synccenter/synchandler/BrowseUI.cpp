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
//      BrowseUI.cpp
//
//  Abstract:
//      Implementation for an object which implements ISyncMgrUIOperation.  It
//      opens explorer to a particular folder when the user browses on an
//      item or partnership.
//
//////////////////////////////////////////////////////////////////////////////

#include "Pch.h"
#include "BrowseUI.h"
#include "Resources.h"
#include "Helpers.h"
#include <knownfolders.h>  // for KNOWNFOLDERID

//////////////////////////////////////////////////////////////////////////////
// class CBrowseUI
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//
//  Description:
//      Create an instance of the CBrowseUI class.
//
//  Parameters:
//      folderID        - KNOWNFOLDERID of the folder to be viewed.
//      riid            - ID of interface to return pointer to.
//      ppv             - Interface pointer returned to caller.
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      E_OUTOFMEMORY   - Insufficient memory.
//      Other HRESULTs  - Error querying for requested interface.
//
//----------------------------------------------------------------------------
HRESULT CBrowseUI_CreateInstance(__in KNOWNFOLDERID folderID, __in REFIID riid, __deref_out void **ppv)
{
    *ppv = NULL;

    HRESULT hr = E_OUTOFMEMORY;
    CBrowseUI *pBrowseUI = new CBrowseUI(folderID);
    if (pBrowseUI != NULL)
    {
        hr = pBrowseUI->QueryInterface(riid, ppv);
        pBrowseUI->Release();
    }

    return hr;

} //*** CBrowseUI_CreateInstance

//----------------------------------------------------------------------------
//
//  Description:
//      Constructor.
//
//  Parameters:
//      folderID       - KNOWNFOLDERID of the folder to be viewed.
//
//----------------------------------------------------------------------------
CBrowseUI::CBrowseUI(__in KNOWNFOLDERID folderID)
    :
    _cRef(1),
    _folderID(folderID)
{
    DllAddRef();

} //*** CBrowseUI::CBrowseUI

//----------------------------------------------------------------------------
//
//  Description:
//      Destructor.
//
//----------------------------------------------------------------------------
CBrowseUI::~CBrowseUI()
{
    DllRelease();

} //*** Destructor CBrowseUI::~CBrowseUI

//----------------------------------------------------------------------------
// IUnknown (CBrowseUI)
//----------------------------------------------------------------------------

STDMETHODIMP CBrowseUI::QueryInterface(__in REFIID riid, __deref_out void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CBrowseUI, ISyncMgrUIOperation),
        { 0 },
    };

    return QISearch(this, qit, riid, ppv);

} //*** CBrowseUI::QueryInterface

//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CBrowseUI::Release()
{
    ULONG cRef = InterlockedDecrement(&_cRef);
    if (cRef == 0)
    {
        delete this;
    }

    return cRef;

} //*** CBrowseUI::Release

//----------------------------------------------------------------------------
// ISyncMgrUIOperation (CBrowseUI)
//----------------------------------------------------------------------------

STDMETHODIMP CBrowseUI::Run(__in HWND hwndOwner)
{
    return _BrowseInExplorer(hwndOwner);

} //*** CMyDeviceSetupUI::Run

//----------------------------------------------------------------------------
//
//  Description:
//      Opens Explorer to a specified known folder.
//
//  Parameters:
//      cFolderID   - KNOWNFOLDERID of the folder to open.
//      hwndOwner   - Handle to the owner window.
//
//  Return Values:
//      S_OK            - Operation completed successfully
//      Other HRESULTs  - Operation failed
//
//----------------------------------------------------------------------------
HRESULT CBrowseUI::_BrowseInExplorer(__in HWND hwndOwner)
{
    LPWSTR pszPath = NULL;
    HRESULT hr = SHGetKnownFolderPath(_folderID, 0, NULL, &pszPath);
    if (SUCCEEDED(hr))
    {
        ShellExecuteW(hwndOwner, L"open", pszPath, NULL, NULL, SW_SHOWNORMAL);

        CoTaskMemFree(pszPath);
    }

    return hr;

}//*** CBrowseUI::_BrowseInExplorer

