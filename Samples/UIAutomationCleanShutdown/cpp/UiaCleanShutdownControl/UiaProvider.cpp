// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <windows.h>
#include <ole2.h>
#include <uiautomation.h>
#include <strsafe.h>

#include "UiaCleanShutdownControl.h"
#include "UiaProvider.h"

CleanShutdownProvider::CleanShutdownProvider(_In_ HWND hwnd, _In_ UiaCleanShutdownControl *control)
    : _refCount(1)
{
    _hwnd = hwnd;
    _control = control;
    _control->IncrementProviderCount();
    _control->IncrementTotalRefCount();
}

CleanShutdownProvider::~CleanShutdownProvider()
{
    _control->DecrementProviderCount();
}

ULONG STDMETHODCALLTYPE CleanShutdownProvider::AddRef()
{
    _control->IncrementTotalRefCount();
    return InterlockedIncrement(&_refCount);
}

ULONG STDMETHODCALLTYPE CleanShutdownProvider::Release()
{
    _control->DecrementTotalRefCount();
    ULONG refCount = InterlockedDecrement(&_refCount);
    if (refCount == 0)
    {
        delete this;
        return 0;
    }
    return refCount;
}

HRESULT STDMETHODCALLTYPE CleanShutdownProvider::QueryInterface(_In_ REFIID riid, _Outptr_ void**ppInterface)
{
    if (riid == __uuidof(IUnknown))                              *ppInterface =(IUnknown*)((IRawElementProviderSimple*)this);
    else if (riid == __uuidof(IRawElementProviderSimple))        *ppInterface =(IRawElementProviderSimple*)this;
    else if (riid == __uuidof(IToggleProvider))                  *ppInterface =(IToggleProvider*)this;
    else
    {
        *ppInterface = NULL;
        return E_NOINTERFACE;
    }

    ((IUnknown*)(*ppInterface))->AddRef();
    return S_OK;
}

// IRawElementProviderSimple Implementation

HRESULT STDMETHODCALLTYPE CleanShutdownProvider::get_ProviderOptions(_Out_ ProviderOptions * retVal )
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }
    *retVal = ProviderOptions_ServerSideProvider | ProviderOptions_UseComThreading;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CleanShutdownProvider::GetPatternProvider(PATTERNID patternId, _Outptr_result_maybenull_ IUnknown ** retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }
    *retVal = NULL;

    if (patternId == UIA_TogglePatternId)
    {
        *retVal = (IToggleProvider *)this;
        AddRef();
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CleanShutdownProvider::GetPropertyValue(PROPERTYID idProp, _Out_ VARIANT * retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }
    retVal->vt = VT_EMPTY;

    // Returning the default will leave the property as the default
    // so we only really need to touch it for the properties we want to implement
    if (idProp == UIA_ControlTypePropertyId)
    {
        retVal->vt = VT_I4;
        retVal->lVal = UIA_CustomControlTypeId;
    }
    else if (idProp == UIA_LocalizedControlTypePropertyId)
    {
         // In a production application this should be loaded from a resource so it can be localized
         retVal->bstrVal = SysAllocString(L"CleanShutdownControl");
         if (retVal->bstrVal != NULL)
         {
            retVal->vt = VT_BSTR;
         } 
    }
    else if (idProp == UIA_NamePropertyId)
    {
         // In a production application this should be loaded from a resource so it can be localized
         retVal->bstrVal = SysAllocString(L"Clean Shutdown Control");
         if (retVal->bstrVal != NULL)
         {
            retVal->vt = VT_BSTR;
         } 
    }
    else if (idProp == UIA_AutomationIdPropertyId)
    {
         retVal->bstrVal = SysAllocString(L"CleanShutdownControl-0001");
         if (retVal->bstrVal != NULL)
         {
            retVal->vt = VT_BSTR;
         } 
    }
    else if (idProp == UIA_IsControlElementPropertyId)
    {
        retVal->vt = VT_BOOL;
        retVal->boolVal = VARIANT_TRUE;
    }
    else if (idProp == UIA_IsKeyboardFocusablePropertyId)
    {
        retVal->vt = VT_BOOL;
        retVal->boolVal = VARIANT_FALSE;
    }
    else if (idProp == UIA_HasKeyboardFocusPropertyId)
    {
        retVal->vt = VT_BOOL;
        retVal->boolVal = VARIANT_FALSE;
    }
    else if (idProp == UIA_ProviderDescriptionPropertyId)
    {
        retVal->bstrVal = SysAllocString(L"Microsoft Sample: Uia Clean Shutdown Control");
        if (retVal->bstrVal != NULL)
        {
            retVal->vt = VT_BSTR;
        }
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CleanShutdownProvider::get_HostRawElementProvider(_Outptr_result_maybenull_ IRawElementProviderSimple ** retVal)
{
    // Unlike other methods, specifically DON'T check whether _hwnd is still valid
    // It's not guaranteed to be valid after the window is destroyed, and that's
    // when UiaDisconnectProvider runs.
    return UiaHostProviderFromHwnd(_hwnd, retVal); 
}

HRESULT STDMETHODCALLTYPE CleanShutdownProvider::Toggle()
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    _control->Toggle();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CleanShutdownProvider::get_ToggleState (_Out_ ToggleState * pRetVal )
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    *pRetVal = _control->IsToggled() ? ToggleState_On : ToggleState_Off;
    return S_OK;
}

