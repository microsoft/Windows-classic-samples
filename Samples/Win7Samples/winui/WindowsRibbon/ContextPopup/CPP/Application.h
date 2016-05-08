// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// Application.h/cpp implements the IUIApplication interface which is required for any ribbon application.
// This interface contains callbacks made by the ribbon framework to the application
// during various updates like command creation/destruction and view state changes.
//

#pragma once

#include <UIRibbon.h>
#include <propvarutil.h>

#include "CommandHandler.h"
#include "ids.h"

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

    __checkReturn static HRESULT CreateInstance(__deref_out_opt CApplication** ppApplication);

    // IUnknown methods.
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID iid, void** ppv);

    // IUIApplication methods.
    STDMETHOD(OnCreateUICommand)(UINT nCmdID,
        __in UI_COMMANDTYPE typeID,
        __deref_out IUICommandHandler** ppCommandHandler);

    STDMETHOD(OnViewChanged)(UINT viewId,
    __in UI_VIEWTYPE typeId,
    __in IUnknown* pView,
    UI_VIEWVERB verb,
    INT uReasonCode);

    STDMETHOD(OnDestroyUICommand)(UINT32 commandId, 
        __in UI_COMMANDTYPE typeID,
        __in_opt IUICommandHandler* commandHandler);

    // Helper functions.
    __checkReturn int GetCurrentContext();

    void SetCurrentContext(int newContext);

private:
    CApplication()
        : m_cRef(1)
        , m_iCurrentContext(IDC_CMD_CONTEXTMAP1)
        , m_pCommandHandler(NULL)
    {
    };

    ~CApplication()
    {
        if (m_pCommandHandler != NULL)
        {
            m_pCommandHandler->Release();
        }
    };

    int m_iCurrentContext;
    CCommandHandler* m_pCommandHandler; // Generic Command Handler.
    ULONG m_cRef;
};
