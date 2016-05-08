// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// CommandHandler.h/cpp implements the IUICommandHandler interface.
// This interface should be returned by the application during command creation.
// Also the same command handler is returned for every command.
//

#pragma once

#include <UIRibbon.h>
#include <propvarutil.h>

//
//  CLASS: CCommandHandler : IUICommandHandler
//
//  PURPOSE: Implements interface IUICommandHandler. 
//
//  COMMENTS:
//
//    This is a generic command handler used by every command in this sample.
//    IUICommandHandler should be returned by the application during command creation.
//    The same command handler is returned for every command.
//
class CCommandHandler
    : public IUICommandHandler // Command handlers must implement IUICommandHandler.
{
public:

    // Static method to create an instance of this object.
    __checkReturn static HRESULT CreateInstance(__deref_out CCommandHandler **ppCommandHandler);

    // IUnknown methods.
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID iid, void** ppv);

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
    };

    ULONG m_cRef;
};
