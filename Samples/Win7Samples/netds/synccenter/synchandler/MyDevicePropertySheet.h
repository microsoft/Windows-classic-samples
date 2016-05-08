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
//      MyDevicePropertySheet.h
//
//  Abstract:
//      Include file for the property sheet extension for the My Device Sync
//      Center handler collection.
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

//----------------------------------------------------------------------------
// Forward Class Declarations
//----------------------------------------------------------------------------

class CMyDevicePropertySheet;

//----------------------------------------------------------------------------
// Global Function Prototypes
//----------------------------------------------------------------------------

HRESULT CMyDevicePropertySheet_CreateInstance(__in_opt IUnknown *punkOuter, __deref_out IUnknown **ppunk);

//////////////////////////////////////////////////////////////////////////////
//
// Class which adds a page to the properties dialog for the Sync Center device
//
//////////////////////////////////////////////////////////////////////////////
class CMyDevicePropertySheet : public IShellExtInit, public IShellPropSheetExt
{
public:
    CMyDevicePropertySheet() :
        _cRef(1),
        _pDataObject(NULL)
    {
        DllAddRef();
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(__in REFIID riid, __deref_out void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef()     { return InterlockedIncrement(&_cRef); }
    IFACEMETHODIMP_(ULONG) Release();

    // IShellExtInit
    IFACEMETHODIMP Initialize(__in PCIDLIST_ABSOLUTE pidlFolder, __in IDataObject *pdtobj, __in HKEY hkeyProgID);

    // IShellPropSheetExt
    STDMETHODIMP AddPages(__in LPFNADDPROPSHEETPAGE lpfnAddPage, __in LPARAM lParam);
    STDMETHODIMP ReplacePage(__in UINT uPageID, __in LPFNADDPROPSHEETPAGE lpfnReplacePage, __in LPARAM lParam)
    {
        UNREFERENCED_PARAMETER(uPageID);
        UNREFERENCED_PARAMETER(lpfnReplacePage);
        UNREFERENCED_PARAMETER(lParam);
        return E_NOTIMPL;
    }

private:
    ~CMyDevicePropertySheet()
    {
        if (_pDataObject != NULL)
        {
            _pDataObject->Release();
        }

        DllRelease();
    }

    static INT_PTR CALLBACK _DlgProc(__in HWND hDlg, __in UINT uMessage, __in WPARAM wParam, __in LPARAM lParam);

    /////////////////////
    // Member Variables
    /////////////////////
    LONG                 _cRef;
    IDataObject         *_pDataObject;

}; //*** class CMyDeviceContextMenu

