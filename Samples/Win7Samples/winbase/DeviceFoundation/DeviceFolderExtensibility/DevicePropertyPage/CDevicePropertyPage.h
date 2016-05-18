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
#include <shobjidl.h>

//------------------------------------------------------------------------------
// CDevicePropertyPage
//
//      Prototype for class that implements IShellPropSheetExt and
//      IShellExtInit to support property page extensibility in the shell 
//      (for this sample we're after extending the Devices and Printers folder
//      exclusively). 
//------------------------------------------------------------------------------
class CDevicePropertyPage:
    public IShellPropSheetExt,
    public IShellExtInit
{
public:

    CDevicePropertyPage();

    //
    // IShellPropSheetExt
    //
    IFACEMETHODIMP AddPages(
        __in LPFNADDPROPSHEETPAGE pfnAddPage,
        __in LPARAM lParam
        );
    IFACEMETHODIMP ReplacePage(
        __in UINT uPageID,
        __in LPFNADDPROPSHEETPAGE pfnReplacePage,
        __in LPARAM lParam
        );
    
    //
    // IShellExtInit
    //
    IFACEMETHODIMP Initialize(
        __in PCIDLIST_ABSOLUTE pidlFolder,
        __in IDataObject* pdtobj,
        __in HKEY hkeyProgID
        );

    //
    // IUnknown
    //
    IFACEMETHODIMP QueryInterface(
        __in REFIID riid, 
        __deref_out void** ppvObject
        );
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

private:

    ~CDevicePropertyPage();

    HRESULT GetIconFromItem(
        __in IShellItem* pShellItem, 
        __in int iImageList, 
        __out HICON* phIcon
        );

    static INT_PTR CALLBACK PropPageDlgProc(
        __in HWND hWndDlg,
        __in UINT uMsg,
        __in WPARAM wParam,
        __in LPARAM lParam
        );

    static UINT CALLBACK PropPageDlgCleanup(
        __in HWND hwnd,
        __in UINT uMsg,
        __in LPPROPSHEETPAGE ppsp
        );

    static HRESULT PopulateShellProperties(
        __in HWND hWndDlg,
        __in IShellItem2* pShellItem
        );

    static HRESULT PopulateDevnodeProperties(
        __in HWND hWndDlg,
        __in IShellItem2* pShellItem
        );

    LONG m_cRef;
    IShellItem2* m_pShellItem;
        
};// CDevicePropertyPage

