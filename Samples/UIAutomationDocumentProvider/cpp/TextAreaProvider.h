// Copyright (c) Microsoft Corporation
//
#pragma once


// A Class representing the top level frame of the AnnotatedTextControl
class TextAreaProvider : public IRawElementProviderSimple,
                         public IRawElementProviderFragment,
                         public ITextProvider2
{
public:
    TextAreaProvider(_In_ HWND hwnd, _In_ AnnotatedTextControl *control);
    virtual ~TextAreaProvider();

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

    // TextProvider methods
    HRESULT STDMETHODCALLTYPE GetSelection(_Outptr_result_maybenull_ SAFEARRAY** retVal);
    HRESULT STDMETHODCALLTYPE GetVisibleRanges(_Outptr_result_maybenull_ SAFEARRAY ** retVal);
    HRESULT STDMETHODCALLTYPE RangeFromChild(_In_opt_ IRawElementProviderSimple * childElement, _Outptr_result_maybenull_ ITextRangeProvider ** retVal);
    HRESULT STDMETHODCALLTYPE RangeFromPoint(UiaPoint screenLocation, _Outptr_result_maybenull_ ITextRangeProvider ** retVal);
    HRESULT STDMETHODCALLTYPE get_DocumentRange(_Outptr_result_maybenull_ ITextRangeProvider ** retVal);
    HRESULT STDMETHODCALLTYPE get_SupportedTextSelection(_Out_ SupportedTextSelection * retVal);

    // ITextProvider2 methods
    HRESULT STDMETHODCALLTYPE RangeFromAnnotation(_In_opt_ IRawElementProviderSimple *annotationElement, _Outptr_result_maybenull_ ITextRangeProvider **retVal);
    HRESULT STDMETHODCALLTYPE GetCaretRange(_Out_ BOOL *isActive, _Outptr_result_maybenull_ ITextRangeProvider **retVal);

private:
    // Ref Counter for this COM object
    ULONG _refCount;

    HWND _hwnd; // The HWND for this object
    AnnotatedTextControl *_control; // The control object that this UIA object is referring to
};

// A text range for the TextArea control
class __declspec(uuid("d6558c5c-71dc-4b83-8ee6-06cab27612fd"))
TextAreaTextRange:
    public ITextRangeProvider
{
public:
    TextAreaTextRange(_In_ HWND hwnd, _In_ AnnotatedTextControl *control, _In_ Range range);
    virtual ~TextAreaTextRange();

    // IUnknown methods
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    HRESULT STDMETHODCALLTYPE QueryInterface(_In_ REFIID riid, _Outptr_ void**ppInterface);

    // ITextRangeProvider methods
    HRESULT STDMETHODCALLTYPE Clone(_Outptr_result_maybenull_ ITextRangeProvider ** retVal);
    HRESULT STDMETHODCALLTYPE Compare(_In_opt_ ITextRangeProvider * range, _Out_ BOOL *retVal);
    HRESULT STDMETHODCALLTYPE CompareEndpoints(TextPatternRangeEndpoint endpoint, _In_opt_ ITextRangeProvider *targetRange, _In_ TextPatternRangeEndpoint targetEndpoint, _Out_ int *retVal);
    HRESULT STDMETHODCALLTYPE ExpandToEnclosingUnit(_In_ TextUnit unit);
    HRESULT STDMETHODCALLTYPE FindAttribute(_In_ TEXTATTRIBUTEID textAttributeId, _In_ VARIANT val, _In_ BOOL searchBackward, _Outptr_result_maybenull_ ITextRangeProvider **retVal);
    HRESULT STDMETHODCALLTYPE FindText(_In_ BSTR text, BOOL searchBackward, BOOL ignoreCase, _Outptr_result_maybenull_ ITextRangeProvider **retVal);
    HRESULT STDMETHODCALLTYPE GetAttributeValue(_In_ TEXTATTRIBUTEID textAttributeId, _Out_ VARIANT *retVal);
    HRESULT STDMETHODCALLTYPE GetBoundingRectangles(_Outptr_result_maybenull_ SAFEARRAY ** retVal);
    HRESULT STDMETHODCALLTYPE GetEnclosingElement(_Outptr_result_maybenull_ IRawElementProviderSimple **retVal);
    HRESULT STDMETHODCALLTYPE GetText(_In_ int maxLength, _Out_ BSTR * retVal);
    HRESULT STDMETHODCALLTYPE Move(_In_ TextUnit unit, _In_ int count, _Out_ int *retVal);
    HRESULT STDMETHODCALLTYPE MoveEndpointByUnit(_In_ TextPatternRangeEndpoint endpoint, _In_ TextUnit unit, _In_ int count, _Out_ int *retVal);
    HRESULT STDMETHODCALLTYPE MoveEndpointByRange(_In_ TextPatternRangeEndpoint endpoint, _In_opt_ ITextRangeProvider *targetRange, _In_ TextPatternRangeEndpoint targetEndpoint);
    HRESULT STDMETHODCALLTYPE Select();
    HRESULT STDMETHODCALLTYPE AddToSelection();
    HRESULT STDMETHODCALLTYPE RemoveFromSelection();
    HRESULT STDMETHODCALLTYPE ScrollIntoView(_In_ BOOL alignToTop);
    HRESULT STDMETHODCALLTYPE GetChildren(_Outptr_result_maybenull_ SAFEARRAY ** retVal);
private:
    // Helper function to convert Rects
    HRESULT SafeArrayInsertRect(_In_ SAFEARRAY * sa, _In_ RectF rect, _In_ long index);

    // Helper functions for walking/searching
    bool CheckEndPointIsUnitEndpoint(_In_ EndPoint check, _In_ TextUnit unit, _In_ TEXTATTRIBUTEID specificAttribute);
    EndPoint Walk(_In_ EndPoint start, _In_ bool forward, _In_ TextUnit unit, _In_ TEXTATTRIBUTEID specificAttribute, _In_ int count, _Out_ int *walked);
    bool GetAnnotationsAtPoint(_In_ EndPoint endPoint, _Outptr_result_buffer_(*annotationCount) int **annotationIds, _Out_ int *annotationCount);
    bool IsWhiteSpace(_In_ EndPoint check);

    // Ref Counter for this COM object
    ULONG _refCount;

    HWND _hwnd; // The HWND for this object
    AnnotatedTextControl *_control; // The control object that this UIA object is referring to
    Range _range; // The range for this instance of TextAreaTextRange
};
