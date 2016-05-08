// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include <UIRibbon.h>
#include <UIRibbonPropertyHelpers.h>
#include "Renderer.h"

extern CRenderer g_renderer;
extern IUIFramework* g_pFramework;

//
//  CLASS: CColorPickerHandler : IUICommandHandler
//
//  PURPOSE: Implements interface IUICommandHandler. 
//
//  COMMENTS:
//
//    This is a command handler used by color picker in this sample.
//    IUICommandHandler should be returned by the application during command creation.
//
class CColorPickerHandler
    : public IUICommandHandler
{
public:
    // Static method to create an instance of this object.
    __checkReturn static HRESULT CreateInstance(__deref_out CColorPickerHandler **ppCommandHandler);

    // IUnknown methods.
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID iid, void** ppv);

    // IUICommandHandler methods.
    STDMETHOD(Execute)(UINT nCmdID,
                       UI_EXECUTIONVERB verb, 
                       __in_opt const PROPERTYKEY* key,
                       __in_opt const PROPVARIANT* ppropvarValue,
                       __in_opt IUISimplePropertySet* pCommandExecutionProperties);

    STDMETHOD(UpdateProperty)(UINT nCmdID,
                              __in REFPROPERTYKEY key,
                              __in_opt const PROPVARIANT* ppropvarCurrentValue,
                              __out PROPVARIANT* ppropvarNewValue);
private:
    CColorPickerHandler() : m_fInitialized(FALSE), m_cRef(1) { };

    ULONG m_cRef;
    BOOL m_fInitialized;
};

//
//  CLASS: CButtonHandler : IUICommandHandler
//
//  PURPOSE: Implements interface IUICommandHandler. 
//
//  COMMENTS:
//
//    This is a command handler used by button commands in this sample.
//    IUICommandHandler should be returned by the application during command creation.
//
class CButtonHandler
    : public IUICommandHandler
{
public:  
    // Static method to create an instance of this object.
    __checkReturn static HRESULT CreateInstance(__deref_out CButtonHandler **ppCommandHandler);

    // IUnknown methods.
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID iid, void** ppv);

    // IUICommandHandler methods.
    STDMETHOD(Execute)(UINT nCmdID,
                       UI_EXECUTIONVERB verb, 
                       __in_opt const PROPERTYKEY* key,
                       __in_opt const PROPVARIANT* ppropvarValue,
                       __in_opt IUISimplePropertySet* pCommandExecutionProperties);

    STDMETHOD(UpdateProperty)(UINT nCmdID,
                              __in REFPROPERTYKEY key,
                              __in_opt const PROPVARIANT* ppropvarCurrentValue,
                              __out PROPVARIANT* ppropvarNewValue);
private:
    CButtonHandler() : m_cRef(1) { };

    ULONG m_cRef;
};