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
//      MyDevicePropertySheet.cpp
//
//  Abstract:
//      Implementation of the property sheet extension for the My Device Sync
//      Center handler collection.
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resources.h"
#include "MyDevicePropertySheet.h"
#include <prsht.h>   // for PROPSHEETPAGE

//////////////////////////////////////////////////////////////////////////////
// class CMyDevicePropertySheet
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//
//  Description:
//      Create an instance of the Property sheet class.
//
//  Parameters:
//      punkOuter       - Outer IUnknown for aggregation.
//      ppunk           - IUnknown interface pointer returned to the caller.
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      E_OUTOFMEMORY   - Error allocating the object.
//      Other HRESULTs  - Error querying for requested interface.
//
//----------------------------------------------------------------------------
HRESULT CMyDevicePropertySheet_CreateInstance(
    __in_opt    IUnknown  *punkOuter,
    __deref_out IUnknown **ppunk)
{
    *ppunk = NULL;

    UNREFERENCED_PARAMETER(punkOuter);

    HRESULT hr = E_OUTOFMEMORY;
    CMyDevicePropertySheet *pmdps = new CMyDevicePropertySheet();
    if (pmdps != NULL)
    {
        hr = pmdps->QueryInterface(IID_PPV_ARGS(ppunk));
        pmdps->Release();
    }

    return hr;

} //*** CMyDevicePropertySheet_CreateInstance

//----------------------------------------------------------------------------
// IUnknown (CMyDeviceSyncHandler)
//----------------------------------------------------------------------------

STDMETHODIMP CMyDevicePropertySheet::QueryInterface(__in REFIID riid, __deref_out void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CMyDevicePropertySheet, IShellExtInit),
        QITABENT(CMyDevicePropertySheet, IShellPropSheetExt),
        { 0 },
    };

    return QISearch(this, qit, riid, ppv);

} //*** CMyDevicePropertySheet::QueryInterface

//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CMyDevicePropertySheet::Release()
{
    ULONG cRef = InterlockedDecrement(&_cRef);
    if (cRef == 0)
    {
        delete this;
    }

    return cRef;

} //*** CMyDevicePropertySheet::Release

//----------------------------------------------------------------------------
// IShellExtInit (CMyDevicePropertySheet)
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
//  Description:
//      Called by explorer to initialize the shell extension for the shell
//      item representing the sync handler.
//
//  Implements: IShellExtInit
//
//  Parameters:
//      pidlFolder      - PID list of the selected objects.
//      pdtobj          - Used to get the selected objects.
//      hkeyProgID      - Open registry key which can be used to access the
//                        DLL's registration data.
//
//  Return Values:
//      S_OK        - Operation completed successfully.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDevicePropertySheet::Initialize(
    __in PCIDLIST_ABSOLUTE   pidlFolder,
    __in IDataObject        *pdtobj,
    __in HKEY                hkeyProgID)
{
    UNREFERENCED_PARAMETER(pidlFolder);
    UNREFERENCED_PARAMETER(hkeyProgID);

    HRESULT hr = S_OK;

    // Hold our IDataObject.  We can use this to determine
    // which items are currently selected in the view.
    _pDataObject = pdtobj;
    _pDataObject->AddRef();

    return hr;

} //*** CMyDevicePropertySheet::Initialize

//----------------------------------------------------------------------------
// IShellPropSheetExt (CMyDevicePropertySheet)
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
//  Description:
//      Adds another property page to the handler's properties dialog.
//
//  Parameters:
//      lpfnAddPage         - Pointer to the function which we call to add the page.
//      lParam              - Reserved, must be passed to lpfnAddPage.
//
//  Return Values:
//      S_OK                - Page was successfully added.
//      Other HRESULTs      - Page was not added.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDevicePropertySheet::AddPages(
    __in LPFNADDPROPSHEETPAGE lpfnAddPage,
    __in LPARAM               lParam)
{
    HRESULT          hr = E_FAIL;
    PROPSHEETPAGEW   psp = {0};
    HPROPSHEETPAGE  hPage = NULL;;

    // Build the PROPSHEETPAGEW structure which
    // describes the page we will add
    psp.dwSize      = sizeof(psp);
    psp.dwFlags     = PSP_DEFAULT | PSP_USETITLE;
    psp.hInstance   = g_hmodThisDll;
    psp.pszTemplate = MAKEINTRESOURCEW(IDD_PROPPAGE);
    psp.pfnDlgProc  = _DlgProc;
    psp.lParam      = (LPARAM) this;
    psp.pszTitle    = MAKEINTRESOURCEW(IDS_PROPS_TITLE);

    hPage = CreatePropertySheetPageW(&psp);
    if (hPage != NULL)
    {
        // Add the page.  If it fails, then destroy the
        // page we have created.
        if (!lpfnAddPage(hPage, lParam))
        {
            DestroyPropertySheetPage(hPage);
            hPage = NULL;
        }
    }

    if (hPage != NULL)
    {
        hr = S_OK;
    }

    return hr;

} //*** CMyDevicePropertySheet::AddPages

//----------------------------------------------------------------------------
// CMyDevicePropertySheet Private Methods
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
//  Description:
//      The dialog procedure of the page.  It is called when the property
//      sheet is first initializing, at which point this method loads
//      a string from resource and sticks it on the page.
//
//  Parameters:
//      hDlg                - Handle to the dialog on the property sheet.
//      message             - The message to handle.
//      wParam              - Specifies additional message-specific information.
//      lParam              - Specifies additional message-specific information.
//
//  Return Values:
//      S_OK                - Page was successfully added.
//      Other HRESULTs      - Page was not added.
//
//----------------------------------------------------------------------------
INT_PTR CALLBACK CMyDevicePropertySheet::_DlgProc(
    __in HWND   hDlg,
    __in UINT   uMessage,
    __in WPARAM wParam,
    __in LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    if (uMessage == WM_INITDIALOG)
    {
        LPWSTR pszPropertyText;
        HRESULT hr = FormatString(g_hmodThisDll, IDS_PROPS_TEXT, &pszPropertyText);
        if (SUCCEEDED(hr))
        {
            SetDlgItemTextW(hDlg, IDC_PROPS_TEXT, pszPropertyText);
            LocalFree(pszPropertyText);
        }
    }

    return FALSE;

}//*** CMyDevicePropertySheet::_DlgProc

