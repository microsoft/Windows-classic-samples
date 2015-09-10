// Copyright (c) Microsoft Corporation
//
#pragma once

SAFEARRAY * BuildIntSafeArray(_In_reads_(length) const int * data, _In_ int length);

// A Class representing the top level frame of the AnnotatedTextControl
class FrameProvider : public IRawElementProviderSimple,
                      public IRawElementProviderFragment,
                      public IRawElementProviderFragmentRoot
{
public:
    FrameProvider(_In_ HWND hwnd, _In_ AnnotatedTextControl *control);
    virtual ~FrameProvider();

    // IUnknown methods
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    HRESULT STDMETHODCALLTYPE QueryInterface(_In_ REFIID riid, _Outptr_ void**ppInterface);
    
    // IRawElementProviderSimple methods
    HRESULT STDMETHODCALLTYPE get_ProviderOptions(_Out_ ProviderOptions * retVal);
    HRESULT STDMETHODCALLTYPE GetPatternProvider(PATTERNID iid, _Outptr_result_maybenull_ IUnknown * * retVal );
    HRESULT STDMETHODCALLTYPE GetPropertyValue(PROPERTYID idProp, _Out_ VARIANT * retVal );
    HRESULT STDMETHODCALLTYPE get_HostRawElementProvider(_Outptr_result_maybenull_ IRawElementProviderSimple ** retVal );

    // IRawElementProviderFragment methods
    HRESULT STDMETHODCALLTYPE Navigate(NavigateDirection direction, _Outptr_result_maybenull_ IRawElementProviderFragment ** retVal );
    HRESULT STDMETHODCALLTYPE GetRuntimeId(_Outptr_result_maybenull_ SAFEARRAY ** retVal );
    HRESULT STDMETHODCALLTYPE get_BoundingRectangle(_Out_ UiaRect * retVal );
    HRESULT STDMETHODCALLTYPE GetEmbeddedFragmentRoots(_Outptr_result_maybenull_ SAFEARRAY ** retVal );
    HRESULT STDMETHODCALLTYPE SetFocus();
    HRESULT STDMETHODCALLTYPE get_FragmentRoot(_Outptr_result_maybenull_ IRawElementProviderFragmentRoot * * retVal);

    // IRawElementProviderFragmenRoot methods
    HRESULT STDMETHODCALLTYPE ElementProviderFromPoint(double x, double y, _Outptr_result_maybenull_ IRawElementProviderFragment ** retVal);
    HRESULT STDMETHODCALLTYPE GetFocus(_Outptr_result_maybenull_ IRawElementProviderFragment ** retVal );

private:
    // Ref Counter for this COM object
    ULONG _refCount;

    // Internal Implementation Stuff;
    HWND _hwnd; // The HWND for this object
    AnnotatedTextControl *_control;
};
