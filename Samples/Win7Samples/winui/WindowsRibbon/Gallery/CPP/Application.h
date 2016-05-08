// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

#include <uiribbon.h>

//
//  CLASS: CApplication : IUIApplication
//
//  PURPOSE: Implements interface IUIApplication that defines methods
//           required to manage Framework events.
//
//  COMMENTS:
//
//    CApplication implements the IUIApplication interface which is required for any ribbon application.
//    IUIApplication contains callbacks made by the ribbon framework to the application
//    during various updates like command creation/destruction and view state changes.
//
class CApplication
    : public IUIApplication // Applications must implement IUIApplication.
{
public:
    STDMETHOD(OnViewChanged)(UINT32 nViewID, __in UI_VIEWTYPE typeID,
    __in IUnknown* pView, UI_VIEWVERB verb, INT32 uReasonCode);

    STDMETHOD(OnCreateUICommand)(UINT32 nCmdID, __in UI_COMMANDTYPE typeID,
    __deref_out IUICommandHandler** ppCommandHandler);

    STDMETHOD(OnDestroyUICommand)(UINT32 commandId, __in UI_COMMANDTYPE typeID,
    __in_opt IUICommandHandler* pCommandHandler);

    static HRESULT CreateInstance(__deref_out CApplication **ppHandler, HWND hwnd);

    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID iid, void **ppv);

private:
    CApplication()
        : m_cRef(1), m_hwnd(NULL)
    {
    }

    LONG m_cRef;
    HWND m_hwnd;
};