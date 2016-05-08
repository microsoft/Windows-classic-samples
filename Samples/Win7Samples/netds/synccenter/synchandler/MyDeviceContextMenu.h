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
//      MyDeviceContextMenu.h
//
//  Abstract:
//      Include file for the context menu extension for the My Device Sync
//      Center handler collection.
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

//----------------------------------------------------------------------------
// Forward Class Declarations
//----------------------------------------------------------------------------

class CMyDeviceContextMenu;

//----------------------------------------------------------------------------
// Global Function Prototypes
//----------------------------------------------------------------------------

HRESULT CMyDeviceContextMenu_CreateInstance(__in_opt IUnknown *punkOuter, __deref_out IUnknown **ppunk);

//////////////////////////////////////////////////////////////////////////////
//
// Class which extends the context menu in Sync Center folders for
// My Device.
//
//////////////////////////////////////////////////////////////////////////////
class CMyDeviceContextMenu : public IShellExtInit, public IContextMenu
{
public:
    CMyDeviceContextMenu() :
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

    // IContextMenu
    IFACEMETHODIMP QueryContextMenu(__in HMENU hMenu, __in UINT indexMenu, __in UINT idCmdFirst, __in UINT idCmdLast, __in UINT uFlags);
    IFACEMETHODIMP InvokeCommand(__in LPCMINVOKECOMMANDINFO pici);
    IFACEMETHODIMP GetCommandString(
        __in                                                UINT_PTR idCommand,
        __in                                                UINT     uFlags,
        __reserved                                          LPUINT   lpReserved,
        __out_awcount(!(uFlags & GCS_UNICODE), uMaxNameLen) LPSTR    pszName,
        __in                                                UINT     uMaxNameLen);

private:
    ~CMyDeviceContextMenu()
    {
        if (_pDataObject != NULL)
        {
            _pDataObject->Release();
        }

        DllRelease();
    }

    HRESULT _GetSelectedItemProperty(__in REFPROPERTYKEY pkey, __out PROPVARIANT *pPropVar);

    /////////////////////
    // Member Variables
    /////////////////////
    LONG                 _cRef;
    IDataObject         *_pDataObject;

}; //*** class CMyDeviceContextMenu
