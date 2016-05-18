// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// This interface should be returned by the application during command creation.
// For the purpose of this sample, it has a barebone implementation.
// Also the same command handler is returned for every command.
//

#pragma once

// This is a generic command handler used by every command in this sample.

//
//  CLASS: CCommandHandler : IUICommandHandler
//
//  PURPOSE: Implements interface IUICommandHandler. 
//
//  COMMENTS:
//
//    This is a generic command handler used by every command in this sample.
//    IUICommandHandler should be returned by the application during command creation.
//    For the purpose of this sample, it has a barebone implementation.
//    Also the same command handler is returned for every command.
//

#include <UIRibbon.h>
#include "RichEditMng.h"

class CCommandHandler
    : public IUICommandHandler // Command handlers must implement IUICommandHandler.
{
public:
    // Static method to create an instance of the object.
    __checkReturn static HRESULT CreateInstance(__deref_out IUICommandHandler **ppCommandHandler);

    // IUnknown methods.
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);

    // IUICommandHandler methods.
    STDMETHOD(UpdateProperty)(UINT nCmdID,
        __in REFPROPERTYKEY key,
        __in_opt const PROPVARIANT* ppropvarCurrentValue,
        __out PROPVARIANT* ppropvarNewValue);

    STDMETHOD(Execute)(UINT nCmdID,
        UI_EXECUTIONVERB verb, 
        __in_opt const PROPERTYKEY* key,
        __in_opt const PROPVARIANT* ppropvarValue,
        __in_opt IUISimplePropertySet* pCommandExecutionProperties);

private:
    CCommandHandler()
        : m_cRef(1)
    {
    }

    LONG m_cRef;                        // Reference count.
};
