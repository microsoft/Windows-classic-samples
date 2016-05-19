// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

#define MAX_RESOURCE_LENGTH     256
#include <uiribbon.h>

//
//  CLASS: CBorderSizeHandler : IUICommandHandler
//
//  PURPOSE: Implements interface IUICommandHandler. 
//
//  COMMENTS:
//
//    This is the command handler used by the Border Size gallery in this sample.
//    IUICommandHandler should be returned by the application during command creation.
//
class CBorderSizeHandler
    : public IUICommandHandler // Command handlers must implement IUICommandHandler.
{
public:
    STDMETHOD(Execute)(UINT nCmdID,
                       UI_EXECUTIONVERB verb, 
                       __in_opt const PROPERTYKEY* key,
                       __in_opt const PROPVARIANT* ppropvarValue,
                       __in_opt IUISimplePropertySet* pCommandExecutionProperties);

    STDMETHOD(UpdateProperty)(UINT nCmdID,
                              __in REFPROPERTYKEY key,
                              __in_opt const PROPVARIANT* ppropvarCurrentValue,
                              __out PROPVARIANT* ppropvarNewValue);

    static HRESULT CreateInstance(__deref_out CBorderSizeHandler **ppHandler);

    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID iid, void **ppv);

private:
    CBorderSizeHandler()
        : m_cRef(1)
    {
    }

    LONG m_cRef;
};