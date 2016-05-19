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
//      BrowseUI.h
//
//  Abstract:
//      Header file for an object which implements ISyncMgrUIOperation.  It
//      opens explorer to a particular folder when the user browses on an
//      item or partnership.
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

//----------------------------------------------------------------------------
// Global Function Prototypes
//----------------------------------------------------------------------------

HRESULT CBrowseUI_CreateInstance(__in KNOWNFOLDERID folderID, __in REFIID riid, __deref_out void **ppv);

//////////////////////////////////////////////////////////////////////////////
//
// Class which provides UI to allow the browsing of the sync items
// and the sync handler
//
//////////////////////////////////////////////////////////////////////////////

class CBrowseUI : public ISyncMgrUIOperation
{
public:
    CBrowseUI(__in KNOWNFOLDERID folderID);

    // IUnknown
    IFACEMETHODIMP QueryInterface(__in REFIID riid, __deref_out void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef()  { return InterlockedIncrement(&_cRef); }
    IFACEMETHODIMP_(ULONG) Release();

    // ISyncMgrUIOperation
    IFACEMETHODIMP Run(__in HWND hwndOwner);

private:
    ~CBrowseUI();

    HRESULT _BrowseInExplorer(__in HWND hwndOwner);

    /////////////////////
    // Member Variables
    /////////////////////
    LONG             _cRef;
    KNOWNFOLDERID    _folderID;

};  //*** class CBrowseUI

