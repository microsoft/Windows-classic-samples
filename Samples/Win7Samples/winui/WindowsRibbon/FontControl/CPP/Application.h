// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

//
//  CLASS: CApplication : IUIApplication
//
//  PURPOSE: Implements interface IUIApplication that defines methods
//           required to manage framework events.
//
//  COMMENTS:
//
//    CApplication implements the IUIApplication interface which is required for any ribbon application.
//    IUIApplication contains callbacks made by the ribbon framework to the application
//    during various updates like command creation/destruction and view state changes.
//

#include <UIRibbon.h>
#include "RichEditMng.h"

class CApplication
    : public IUIApplication // Applications must implement IUIApplication.
{
public:
    // Static method to create an instance of the object.
    static HRESULT CreateInstance(__deref_out IUIApplication **ppApplication);

    // IUnknown methods.
    STDMETHOD_(ULONG, AddRef());
    STDMETHOD_(ULONG, Release());
    STDMETHOD(QueryInterface(REFIID iid, void** ppv));

    // IUIApplication methods
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

private:
    CApplication() 
        : m_cRef(1)
        , m_pCommandHandler(NULL)
    {
    }

    ~CApplication() 
    {
        if (m_pCommandHandler)
        {
            m_pCommandHandler->Release();
            m_pCommandHandler = NULL;
        }
    }

    LONG m_cRef;                            // Reference count.
    IUICommandHandler * m_pCommandHandler;  // Generic Command Handler.
};
