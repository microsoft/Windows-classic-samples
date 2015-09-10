// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

SAFEARRAY * BuildIntSafeArray(_In_reads_(length) const int * data, _In_ int length);

// A Class representing the top level frame of the AnnotatedTextControl
class CleanShutdownProvider : public IRawElementProviderSimple,
                              public IToggleProvider
{
public:
    CleanShutdownProvider(_In_ HWND hwnd, _In_ UiaCleanShutdownControl *control);
    virtual ~CleanShutdownProvider();

    // IUnknown methods
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    HRESULT STDMETHODCALLTYPE QueryInterface(_In_ REFIID riid, _Outptr_ void**ppInterface);
    
    // IRawElementProviderSimple methods
    HRESULT STDMETHODCALLTYPE get_ProviderOptions(_Out_ ProviderOptions * retVal);
    HRESULT STDMETHODCALLTYPE GetPatternProvider(PATTERNID iid, _Outptr_result_maybenull_ IUnknown * * retVal );
    HRESULT STDMETHODCALLTYPE GetPropertyValue(PROPERTYID idProp, _Out_ VARIANT * retVal );
    HRESULT STDMETHODCALLTYPE get_HostRawElementProvider(_Outptr_result_maybenull_ IRawElementProviderSimple ** retVal );

    // IToggleProvider methods
    HRESULT STDMETHODCALLTYPE Toggle();
    HRESULT STDMETHODCALLTYPE get_ToggleState (_Out_ ToggleState * pRetVal );

private:
    // Ref Counter for this COM object
    ULONG _refCount;

    // Internal Implementation Stuff;
    HWND _hwnd; // The HWND for this object
    UiaCleanShutdownControl *_control;
};
