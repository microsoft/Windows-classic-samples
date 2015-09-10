// Copyright (c) Microsoft Corporation
//
#pragma once

// Class representing each individual annotation
class  __declspec(uuid("87304f6a-0602-4b92-9efb-ed9da88f147c"))
AnnotationProvider : public IRawElementProviderSimple,
                     public IRawElementProviderFragment,
                     public IAnnotationProvider
{
public:
    AnnotationProvider(_In_ HWND hwnd, _In_ AnnotatedTextControl *control, _In_ int annotationId);
    virtual ~AnnotationProvider();

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

    // IAnnotationProvider methods
    HRESULT STDMETHODCALLTYPE get_AnnotationTypeId(_Out_ int *retVal);
    HRESULT STDMETHODCALLTYPE get_AnnotationTypeName(_Outptr_result_maybenull_ BSTR *retVal);
    HRESULT STDMETHODCALLTYPE get_Author(_Outptr_result_maybenull_ BSTR *retVal);
    HRESULT STDMETHODCALLTYPE get_DateTime(_Outptr_result_maybenull_ BSTR *retVal);
    HRESULT STDMETHODCALLTYPE get_Target(_Outptr_result_maybenull_ IRawElementProviderSimple **retVal);

    // Used internally for RangeFromAnnotation
    int GetAnnotationId();
    HWND GetControlHwnd();

private:
    bool CheckAnnotation();

    // Ref Counter for this COM object
    ULONG _refCount;

    // Internal Implementation Stuff;
    HWND _hwnd; // The HWND for this object
    AnnotatedTextControl *_control;
    int _annotationId;
    Annotation *_annotation;
};