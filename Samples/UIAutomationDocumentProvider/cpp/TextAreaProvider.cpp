// Copyright (c) Microsoft Corporation
//
#include <windows.h>
#include <ole2.h>
#include <uiautomation.h>
#include <strsafe.h>
#include <wchar.h>

#include "AnnotatedTextControl.h"
#include "FrameProvider.h"
#include "AnnotationProvider.h"
#include "TextAreaProvider.h"

const TEXTATTRIBUTEID lineVariableAttributes[] =
{
    UIA_FontSizeAttributeId,
    UIA_FontWeightAttributeId,
    UIA_IndentationFirstLineAttributeId,
    UIA_IndentationLeadingAttributeId,
    UIA_IsItalicAttributeId,
    UIA_UnderlineStyleAttributeId,
    UIA_StyleNameAttributeId,
    UIA_StyleIdAttributeId
};

const TEXTATTRIBUTEID charVariableAttributes[] =
{
    UIA_AnnotationTypesAttributeId,
    UIA_AnnotationObjectsAttributeId
};

void NotifyCaretPositionChanged(_In_ HWND hwnd, _In_ AnnotatedTextControl *control)
{
    TextAreaProvider *eventControl = new TextAreaProvider(hwnd, control);
    if (eventControl == NULL)
    {
        // This is an error, but there's not good place to report it in this sample,
        // so just proceed
    }
    else
    {
        UiaRaiseAutomationEvent(eventControl, UIA_Text_TextSelectionChangedEventId);
        eventControl->Release();
    }
}

TextAreaProvider::TextAreaProvider(_In_ HWND hwnd, _In_ AnnotatedTextControl *control)
    : _refCount(1)
{
    _hwnd = hwnd;
    _control = control;
}

TextAreaProvider::~TextAreaProvider()
{
}

ULONG STDMETHODCALLTYPE TextAreaProvider::AddRef()
{
    return ++_refCount;
}

ULONG STDMETHODCALLTYPE TextAreaProvider::Release()
{
    if (--_refCount <= 0)
    {
        delete this;
        return 0;
    }
    return _refCount;
}

HRESULT STDMETHODCALLTYPE TextAreaProvider::QueryInterface(_In_ REFIID riid, _Outptr_ void** ppInterface)
{
    if (riid == __uuidof(IUnknown))                              *ppInterface =(IUnknown*)((IRawElementProviderSimple*)this);
    else if (riid == __uuidof(IRawElementProviderSimple))        *ppInterface =(IRawElementProviderSimple*)this;
    else if (riid == __uuidof(IRawElementProviderFragment))      *ppInterface =(IRawElementProviderFragment*)this;
    else if (riid == __uuidof(ITextProvider))                    *ppInterface =(ITextProvider*)this;
    else if (riid == __uuidof(ITextProvider2))                   *ppInterface =(ITextProvider2*)this;
    else
    {
        *ppInterface = NULL;
        return E_NOINTERFACE;
    }

    ((IUnknown*)(*ppInterface))->AddRef();
    return S_OK;
}

// IRawElementProviderSimple Implementation

HRESULT STDMETHODCALLTYPE TextAreaProvider::get_ProviderOptions(_Out_ ProviderOptions * retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    *retVal = ProviderOptions_ServerSideProvider | ProviderOptions_UseComThreading;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE TextAreaProvider::GetPatternProvider(PATTERNID patternId, _Outptr_result_maybenull_ IUnknown ** retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    *retVal = NULL;
    if (patternId == UIA_TextPatternId || patternId == UIA_TextPattern2Id)
    {
        *retVal = static_cast<ITextProvider2 *>(this);
        (*retVal)->AddRef();
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE TextAreaProvider::GetPropertyValue(PROPERTYID idProp, _Out_ VARIANT * retVal)
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
        // This control is the Document control type, implying that it is
        // a complex document that supports text pattern
        retVal->vt = VT_I4;
        retVal->lVal = UIA_DocumentControlTypeId;
    }
    else if (idProp == UIA_NamePropertyId)
    {
         // In a production application, this would be localized text.
         retVal->bstrVal = SysAllocString(L"Text Area");
         if (retVal->bstrVal != NULL)
         {
            retVal->vt = VT_BSTR;
         } 
    }
    else if (idProp == UIA_AutomationIdPropertyId)
    {
         retVal->bstrVal = SysAllocString(L"Text Area");
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
    else if (idProp == UIA_IsContentElementPropertyId)
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
        retVal->bstrVal = SysAllocString(L"Microsoft Sample: Uia Document Content Text Area");
        if (retVal->bstrVal != NULL)
        {
            retVal->vt = VT_BSTR;
        }
    }

    return S_OK;
}


HRESULT STDMETHODCALLTYPE TextAreaProvider::get_HostRawElementProvider(_Outptr_result_maybenull_ IRawElementProviderSimple ** retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    *retVal = NULL;

    return S_OK; 
}

HRESULT STDMETHODCALLTYPE TextAreaProvider::Navigate(NavigateDirection direction, _Outptr_result_maybenull_ IRawElementProviderFragment ** retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }
    *retVal = NULL;

    HRESULT hr = S_OK;
    if (direction == NavigateDirection_Parent)
    {
        *retVal = new FrameProvider(_hwnd, _control);
        if (*retVal == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else if (direction == NavigateDirection_NextSibling && _control->GetAnnotationCount() > 0)
    {
        *retVal = new AnnotationProvider(_hwnd, _control, 0);
        if (*retVal == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    // For the other directions (firstchild, lastchild, previous) the default of NULL is correct
    return hr;
}

HRESULT STDMETHODCALLTYPE TextAreaProvider::GetRuntimeId(_Outptr_result_maybenull_ SAFEARRAY ** retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }
    // AppendRuntimeId is a magic Number that tells UIAutomation to Append its own Runtime ID(From the HWND)
    int rId[] = { UiaAppendRuntimeId, -1 };
    // BuildIntSafeArray is a custom function to hide the SafeArray creation
    HRESULT hr = S_OK;
    *retVal = BuildIntSafeArray(rId, 2);
    if (*retVal == NULL)
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE TextAreaProvider::get_BoundingRectangle(_Out_ UiaRect * retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    RECT rc;
    if (!GetWindowRect(_hwnd, &rc))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    retVal->left = rc.left;
    retVal->top = rc.top;
    retVal->width = _control->GetTextAreaWidth();
    retVal->height = rc.bottom - rc.top;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE TextAreaProvider::GetEmbeddedFragmentRoots(_Outptr_result_maybenull_ SAFEARRAY ** retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    *retVal = NULL;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE TextAreaProvider::SetFocus()
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE TextAreaProvider::get_FragmentRoot(_Outptr_result_maybenull_ IRawElementProviderFragmentRoot ** retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    *retVal = new FrameProvider(_hwnd, _control);
    return (*retVal == NULL) ? E_OUTOFMEMORY : S_OK;
}

// TextProvider methods
HRESULT STDMETHODCALLTYPE TextAreaProvider::GetSelection(_Outptr_result_maybenull_ SAFEARRAY** retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    // The control does not support selection
    *retVal = NULL;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE TextAreaProvider::GetVisibleRanges(_Outptr_result_maybenull_ SAFEARRAY ** retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    RECT winRect;
    if (!GetClientRect(_hwnd, &winRect))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    EndPoint start = _control->SearchForClosestEndPoint(static_cast<float>(winRect.left), static_cast<float>(winRect.top));
    EndPoint end = _control->SearchForClosestEndPoint(static_cast<float>(winRect.right), static_cast<float>(winRect.bottom));
    Range visibleRange = {start, end};

    HRESULT hr = S_OK;
    ITextRangeProvider *visibleRangeProvider = new TextAreaTextRange(_hwnd, _control, visibleRange);
    if (visibleRangeProvider == NULL)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        *retVal = SafeArrayCreateVector(VT_UNKNOWN, 0, 1);
        if (*retVal == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            long index = 0;
            hr = SafeArrayPutElement(*retVal, &index, visibleRangeProvider);
            if (FAILED(hr))
            {
                SafeArrayDestroy(*retVal);
                *retVal = NULL;
            }
        }
        visibleRangeProvider->Release();
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE TextAreaProvider::RangeFromChild(_In_opt_ IRawElementProviderSimple * childElement, _Outptr_result_maybenull_ ITextRangeProvider ** retVal)
{
    UNREFERENCED_PARAMETER(childElement);
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }
    
    // There are no children of this text control
    *retVal = NULL;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE TextAreaProvider::RangeFromPoint(UiaPoint screenLocation, _Outptr_result_maybenull_ ITextRangeProvider ** retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    POINT screenPt = {static_cast<int>(screenLocation.x), static_cast<int>(screenLocation.y)};
    MapWindowPoints(NULL, _hwnd, &screenPt, 1);

    EndPoint closest = _control->SearchForClosestEndPoint(static_cast<float>(screenPt.x), static_cast<float>(screenPt.y));
    Range closestRange = {closest, closest};

    *retVal = new TextAreaTextRange(_hwnd, _control, closestRange);
    return (*retVal == NULL) ? E_OUTOFMEMORY : S_OK;
}

HRESULT STDMETHODCALLTYPE TextAreaProvider::get_DocumentRange(_Outptr_result_maybenull_ ITextRangeProvider ** retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    Range full = {{0, 0}, _control->GetEnd()};
    *retVal = new TextAreaTextRange(_hwnd, _control, full);
    return (*retVal == NULL) ? E_OUTOFMEMORY : S_OK;
}

HRESULT STDMETHODCALLTYPE TextAreaProvider::get_SupportedTextSelection(_Out_ SupportedTextSelection * retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    // The control does not support selection
    *retVal = SupportedTextSelection_None;
    return S_OK;
}

// ITextProvider2 methods
HRESULT STDMETHODCALLTYPE TextAreaProvider::RangeFromAnnotation(_In_opt_ IRawElementProviderSimple *annotationElement, _Outptr_result_maybenull_ ITextRangeProvider **retVal)
{
    *retVal = NULL;

    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    if (annotationElement == NULL)
    {
        return E_INVALIDARG;
    }

    AnnotationProvider *annotationInternal;
    // Can't use IID_PPV_ARGS because of IUnknown ambiguity
    if (FAILED(annotationElement->QueryInterface(__uuidof(AnnotationProvider), reinterpret_cast<void **>(&annotationInternal))))
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    HWND hwnd = annotationInternal->GetControlHwnd();
    int annotationId = annotationInternal->GetAnnotationId();
    annotationInternal->Release();

    // Tried to input an annotation from a different instance of the control
    if (hwnd != _hwnd)
    {
        hr = E_INVALIDARG;
    }

    Annotation *annotation = NULL;
    if (SUCCEEDED(hr))
    {
        annotation = _control->GetAnnotation(annotationId);

        // Points to a non-existant annotation 
        if (annotation == NULL)
        {
            hr = E_INVALIDARG;
        }
    }

    if (SUCCEEDED(hr))
    {
        Range annotationRange = {{annotation->line, annotation->start_char}, {annotation->line, annotation->start_char + annotation->length}};
        int lineLength = _control->GetLineLength(annotation->line);

        if (annotation->length == -1 || annotationRange.end.character > lineLength)
        {
            annotationRange.end.character = lineLength;
        }
    
        *retVal = new TextAreaTextRange(_hwnd, _control, annotationRange);
        hr = (*retVal == NULL) ? E_OUTOFMEMORY : S_OK;
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE TextAreaProvider::GetCaretRange(_Out_ BOOL *isActive, _Outptr_result_maybenull_ ITextRangeProvider **retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    *isActive = static_cast<BOOL>(_control->IsActive());
    EndPoint caret = _control->GetCaretPosition();
    Range caretRange = {caret, caret};
    *retVal = new TextAreaTextRange(_hwnd, _control, caretRange);
    return (*retVal == NULL) ? E_OUTOFMEMORY : S_OK;
}

TextAreaTextRange::TextAreaTextRange(_In_ HWND hwnd, _In_ AnnotatedTextControl *control, _In_ Range range)
    : _refCount(1)
{
    _hwnd = hwnd;
    _control = control;
    _range = range;
}

TextAreaTextRange::~TextAreaTextRange()
{
}

// IUnknown methods
ULONG STDMETHODCALLTYPE TextAreaTextRange::AddRef()
{
    return ++_refCount;
}

ULONG STDMETHODCALLTYPE TextAreaTextRange::Release()
{
    if (--_refCount <= 0)
    {
        delete this;
        return 0;
    }
    return _refCount;
}

HRESULT STDMETHODCALLTYPE TextAreaTextRange::QueryInterface(_In_ REFIID riid, _Outptr_ void** ppInterface)
{
    if (riid == __uuidof(IUnknown))                    *ppInterface =(IUnknown*)((ITextRangeProvider*)this);
    else if (riid == __uuidof(ITextRangeProvider))     *ppInterface =(ITextRangeProvider*)this;
    else if (riid == __uuidof(TextAreaTextRange))      *ppInterface =this;
    else
    {
        *ppInterface = NULL;
        return E_NOINTERFACE;
    }

    ((IUnknown*)(*ppInterface))->AddRef();
    return S_OK;
}

// ITextRangeProvider methods
HRESULT STDMETHODCALLTYPE TextAreaTextRange::Clone(_Outptr_result_maybenull_ ITextRangeProvider ** retVal)
{
    HRESULT hr = S_OK;
    *retVal = new TextAreaTextRange(_hwnd, _control, _range);
    if (*retVal == NULL)
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}


HRESULT STDMETHODCALLTYPE TextAreaTextRange::Compare(_In_opt_ ITextRangeProvider * range, _Out_ BOOL *retVal)
{
    *retVal = FALSE;
    if (range != NULL)
    {
        TextAreaTextRange *rangeInternal;
        if (SUCCEEDED(range->QueryInterface(IID_PPV_ARGS(&rangeInternal))))
        {
            if (_hwnd == rangeInternal->_hwnd &&
                QuickCompareEndpoints(rangeInternal->_range.begin, _range.begin) == 0 &&
                QuickCompareEndpoints(rangeInternal->_range.end, _range.end) == 0)
            {
                *retVal = TRUE;
            }
            rangeInternal->Release();
        }
    }
    return S_OK;
}


HRESULT STDMETHODCALLTYPE TextAreaTextRange::CompareEndpoints(TextPatternRangeEndpoint endpoint, _In_opt_ ITextRangeProvider *targetRange, _In_ TextPatternRangeEndpoint targetEndpoint, _Out_ int *retVal)
{
    if (targetRange == NULL)
    {
        return E_INVALIDARG;
    }

    TextAreaTextRange *rangeInternal;
    if (FAILED(targetRange->QueryInterface(IID_PPV_ARGS(&rangeInternal))))
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    if (_hwnd != rangeInternal->_hwnd)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        Range target = rangeInternal->_range;

        EndPoint thisEnd = (endpoint == TextPatternRangeEndpoint_Start) ? _range.begin : _range.end;
        EndPoint targetEnd = (targetEndpoint == TextPatternRangeEndpoint_Start) ? target.begin : target.end;

        *retVal = QuickCompareEndpoints(thisEnd, targetEnd);
    }
    rangeInternal->Release();


    return hr;
}


HRESULT STDMETHODCALLTYPE TextAreaTextRange::ExpandToEnclosingUnit(_In_ TextUnit unit)
{
    // This control supports the units:  TextUnit_Character, TextUnit_Format, TextUnit_Word, TextUnit_Paragraph, TextUnit_Document
    // This control does not support: TextUnit_Line, TextUnit_Page
    HRESULT hr = S_OK;
    if (unit == TextUnit_Character)
    {
        _range.end = _range.begin;
        _range.end.character++;
        
        if (_range.end.character > _control->GetLineLength(_range.begin.line))
        {
            if (_range.end.line + 1 >= _control->GetLineCount())
            {
                _range.end = _range.begin;
                _range.begin.character--;
            }
            else
            {
                _range.end.line = _range.begin.line + 1;
                _range.end.character = 0;
            }
        }
    }
    else if (unit == TextUnit_Format || unit == TextUnit_Word)
    {
        int walked;
        _range.begin = Walk(_range.begin, false, unit, 0, 0, &walked);
        _range.end = Walk(_range.begin, true, unit, 0, 1, &walked);
        if ( walked < 1 )
        {
            _range.begin = Walk(_range.end, false, unit, 0, 1, &walked);
        }
    }
    else if (unit == TextUnit_Line || unit == TextUnit_Paragraph)
    {
        // As stated above, this sample does not support line-by-line navigation. It does support paragraph-by-paragraph
        // navigation, and in that case, check if the starting position of the current range is already at the end of a
        // paragraph. If it is, move to the next paragraph.
        if ((unit == TextUnit_Paragraph) && (_range.begin.character == _control->GetLineLength(_range.begin.line)))
        {
            _range.begin.line++;
        }

        _range.begin.character = 0;
        _range.end.line = _range.begin.line;
        _range.end.character = _control->GetLineLength(_range.end.line);
    }
    else if (unit == TextUnit_Page || unit == TextUnit_Document)
    {
        _range.begin.character = 0;
        _range.begin.line = 0;
        _range.end =  _control->GetEnd();
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


HRESULT STDMETHODCALLTYPE TextAreaTextRange::FindAttribute(_In_ TEXTATTRIBUTEID textAttributeId, _In_ VARIANT val, _In_ BOOL searchBackward, _Outptr_result_maybenull_ ITextRangeProvider **retVal)
{
    HRESULT hr = S_OK;
    *retVal = NULL;

    EndPoint start = searchBackward ? _range.end : _range.begin;
    EndPoint finish   = searchBackward ? _range.begin : _range.end;
    EndPoint current = start;

    // This will loop until 'current' passes or is equal to the end
    while (QuickCompareEndpoints(searchBackward ? finish : current, searchBackward ? current : finish) < 0)
    {
        int walked;
        EndPoint next = Walk(current, !searchBackward, TextUnit_Format, textAttributeId, 1, &walked);
        VARIANT curValue = _control->GetAttributeAtPoint(searchBackward ? current : next, textAttributeId);

        hr = VarCmp(&val, &curValue, LOCALE_NEUTRAL);

        if (hr == VARCMP_EQ)
        {
            Range found;
            found.begin = searchBackward ? next : current;

            // For line based attributes, the start character will be the end of the previous line
            // bump it up one to make it an even line
            for (int i = 0; i < ARRAYSIZE(lineVariableAttributes); i++)
            {
                if (textAttributeId == lineVariableAttributes[i])
                {
                    if (found.begin.character > 0)
                    {
                        found.begin.character = 0;
                        found.begin.line++;
                    }
                    break;
                }
            }

            // If next is past the end of the current range, end at the end of the current range instead
            if (QuickCompareEndpoints(searchBackward ? finish : next, searchBackward ? next : finish) > 0)
            {
                found.end = finish;
            }
            else
            {
                found.end = searchBackward ? current : next;
            }
            *retVal = new TextAreaTextRange(_hwnd, _control, found);
            if (*retVal == NULL)
            {
                hr = E_OUTOFMEMORY;
            }
            break;
        }

        current = next;
    }
    return hr;
}


HRESULT STDMETHODCALLTYPE TextAreaTextRange::FindText(_In_ BSTR text, BOOL searchBackward, BOOL ignoreCase, _Outptr_result_maybenull_ ITextRangeProvider **retVal)
{
    UNREFERENCED_PARAMETER(text);
    UNREFERENCED_PARAMETER(searchBackward);
    UNREFERENCED_PARAMETER(ignoreCase);

    // The FindText operation is not implemented in this sample.
    *retVal = NULL;
    return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE TextAreaTextRange::GetAttributeValue(_In_ TEXTATTRIBUTEID textAttributeId, _Out_ VARIANT *retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }
    HRESULT hr = S_OK;
    VariantInit(retVal);

    int walked;
    EndPoint endOfAttribute = Walk(_range.begin, true, TextUnit_Format, textAttributeId, 1, &walked);
    if (QuickCompareEndpoints(endOfAttribute, _range.end) < 0)
    {
        hr = UiaGetReservedMixedAttributeValue(&retVal->punkVal);
        if (SUCCEEDED(hr))
        {
            retVal->vt = VT_UNKNOWN;
        }
    }
    else
    {
        if ( textAttributeId == UIA_AnnotationTypesAttributeId ||
             textAttributeId == UIA_AnnotationObjectsAttributeId )
        {
            int *annotationIds;
            int annotationCount;
            if (!GetAnnotationsAtPoint(_range.begin, &annotationIds, &annotationCount))
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                if (textAttributeId == UIA_AnnotationTypesAttributeId)
                {
                    int *annotationTypes = new int[annotationCount];
                    if (annotationTypes == NULL)
                    {
                        hr = E_OUTOFMEMORY;
                    }
                    else
                    {
                        for (int i = 0; i < annotationCount; i++)
                        {
                            annotationTypes[i] = AnnotationType_Comment;
                        }
                        retVal->parray = BuildIntSafeArray(annotationTypes, annotationCount);
                        if (retVal->parray == NULL)
                        {
                            hr = E_OUTOFMEMORY;
                        }
                        else
                        {
                            retVal->vt = VT_I4|VT_ARRAY;
                        }
                        delete [] annotationTypes;
                    }
                }
                else if (textAttributeId == UIA_AnnotationObjectsAttributeId)
                {
                    retVal->parray = SafeArrayCreateVector(VT_UNKNOWN, 0, annotationCount);
                    if (retVal->parray == NULL)
                    {
                        hr = E_OUTOFMEMORY;
                    }
                    else
                    {
                        for (long i = 0; SUCCEEDED(hr) && i < annotationCount; i++)
                        {
                            IRawElementProviderSimple *provider = new AnnotationProvider(_hwnd, _control, annotationIds[i]);
                            if (provider == NULL || FAILED(SafeArrayPutElement(retVal->parray, &i, provider)))
                            {
                                SafeArrayDestroy(retVal->parray);
                                retVal->parray = NULL;
                                hr = E_OUTOFMEMORY;
                            }
                            
                            if (provider != NULL)
                            {
                                provider->Release();
                            }

                            if (FAILED(hr))
                            {
                                break;
                            }
                        }
                    }

                    if (SUCCEEDED(hr))
                    {
                        retVal->vt = VT_UNKNOWN|VT_ARRAY;
                    }
                }
                delete [] annotationIds;
            }
        }
        else
        {
            *retVal = _control->GetAttributeAtPoint(_range.begin, textAttributeId);
        }
    }

    return hr;
}

HRESULT TextAreaTextRange::SafeArrayInsertRect(_In_ SAFEARRAY * sa, _In_ RectF rect, _In_ long index)
{
    HRESULT hr = S_OK;
    RECT rc = {static_cast<int>(rect.X),
               static_cast<int>(rect.Y),
               static_cast<int>(rect.GetRight()),
               static_cast<int>(rect.GetBottom())};

    // Convert from Client coordinates to desktop coordinates
    MapWindowPoints(_hwnd, NULL, reinterpret_cast<POINT *>(&rc), 2);

    double data[4];
    data[0] = rc.left;
    data[1] = rc.top;
    data[2] = rc.right - rc.left;
    data[3] = rc.bottom - rc.top;
    for (int i = 0; SUCCEEDED(hr) && i < 4; i++)
    {
        hr = SafeArrayPutElement(sa, &index,(void *)&(data[i]));
        index++;
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE TextAreaTextRange::GetBoundingRectangles(_Outptr_result_maybenull_ SAFEARRAY ** retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    HRESULT hr = S_OK;
    *retVal = NULL;

    int numLines = _range.end.line - _range.begin.line + 1;
    Region ** lineRegions = new Region*[numLines];
    if (lineRegions == NULL)
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        for (int i = 0; i < numLines; i++)
        {
            lineRegions[i] = NULL;
        }

        Graphics graphics(_hwnd);
        Matrix identity;

        EndPoint current = _range.begin;
        int rectCount = 0;
        for (int i = 0; i < numLines; i++)
        {
            if (QuickCompareEndpoints(current, _range.end) >= 0)
            {
                break;
            }
    
            if (current.line < _range.end.line)
            {
                int length = _control->GetLineLength(current.line);
                // Add 1 for the implied newline
                int getLength = length - current.character;
                lineRegions[i] = _control->GetLineCharactersPosition(current.line, current.character, getLength, &graphics);
                current.line++;
                current.character = 0;
            }
            else
            {
                int getLength =  _range.end.character - current.character;
                lineRegions[i] = _control->GetLineCharactersPosition(current.line, current.character, getLength, &graphics);
                current.character = _range.end.character;
            }
            if (lineRegions[i] != NULL)
            {
                rectCount += lineRegions[i]->GetRegionScansCount(&identity);
            }
        }

        if (rectCount > 0)
        {
            RectF *boundingRects = new RectF[rectCount];
            if (boundingRects != NULL)
            {
                int currentRect = 0;
                for (int i = 0; i < numLines; i++)
                {
                    if (lineRegions[i] != NULL)
                    {
                        int count;
                        if (lineRegions[i]->GetRegionScans(&identity, &boundingRects[currentRect], &count) == Ok && count > 0)
                        {
                            currentRect += count;
                        }
                    }
                }

                if (currentRect == rectCount)
                {
                    *retVal = SafeArrayCreateVector(VT_R8, 0, rectCount * 4);
                    if (*retVal != NULL)
                    {
                        for (int i = 0; i < rectCount; i++)
                        {
                            if (FAILED(SafeArrayInsertRect(*retVal, boundingRects[i], i * 4)))
                            {
                                SafeArrayDestroy(*retVal);
                                *retVal = NULL;
                                break;
                            }
                        }
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
                delete [] boundingRects;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

        // Cleanup
        for (int i = 0; i < numLines; i++)
        {
            if (lineRegions[i] != NULL)
            {
                delete lineRegions[i];
                lineRegions[i] = NULL;
            }
        }
        delete [] lineRegions;
    }

    return hr;
}


HRESULT STDMETHODCALLTYPE TextAreaTextRange::GetEnclosingElement(_Outptr_result_maybenull_ IRawElementProviderSimple **retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    *retVal = new TextAreaProvider(_hwnd, _control);
    return (*retVal == NULL) ? E_OUTOFMEMORY : S_OK;
}


HRESULT STDMETHODCALLTYPE TextAreaTextRange::GetText(_In_ int maxLength, _Out_ BSTR * retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    HRESULT hr = S_OK;
    int builderSize;
    PWSTR textBuilder;
    if (maxLength >= 0)
    {
        builderSize = maxLength + 1;
    }
    else
    {
        builderSize = 0;
        EndPoint current = _range.begin;
        while (QuickCompareEndpoints(current, _range.end) < 0)
        {
            if (current.line < _range.end.line)
            {
                int length = _control->GetLineLength(current.line);
                // Add 1 for the implied newline
                builderSize += length - current.character + 1;
                current.line++;
                current.character = 0;
            }
            else
            {
                builderSize += _range.end.character - current.character;
                current.character = _range.end.character;
            }
        }

        // Always allow space for the final string terminator.
        builderSize++;
    }

    textBuilder = new WCHAR[builderSize];
    if (textBuilder != NULL)
    {
        int writePos = 0;
        EndPoint current = _range.begin;
        while (QuickCompareEndpoints(current, _range.end) < 0 && writePos < (builderSize - 1))
        {
            int copyLength = 0;
            EndPoint next;
            bool trailingNewline = false;
            if (current.line < _range.end.line)
            {
                int length = _control->GetLineLength(current.line);
                // Add 1 for the implied newline
                copyLength = length - current.character;
                trailingNewline = true;
                next.line = current.line + 1;
                next.character = 0;
            }
            else
            {
                copyLength = _range.end.character - current.character;
                next.line = current.line;
                next.character = _range.end.character;
            }

            if (writePos + copyLength >= (builderSize - 1))
            {
                copyLength = (builderSize - 1) - writePos;
            }

            TextLine *textLine = _control->GetLine(current.line);
            StringCchCopyN(&textBuilder[writePos], copyLength + 1, &textLine->text[current.character], copyLength);
            writePos += copyLength;
            current = next;

            if (trailingNewline && writePos < (builderSize - 1))
            {
                textBuilder[writePos++] = '\n';
            }
        }
        // Ensure the string is null-terminated
        textBuilder[writePos] = '\0';

        *retVal = SysAllocString(textBuilder);
        if (*retVal == NULL)
        {
            hr = E_OUTOFMEMORY;
        }

        delete [] textBuilder;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
        
    return hr;
}


HRESULT STDMETHODCALLTYPE TextAreaTextRange::Move(_In_ TextUnit unit, _In_ int count, _Out_ int *retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }
    *retVal = 0;

    bool isDegenerate = (QuickCompareEndpoints(_range.begin, _range.end) == 0);

    int walked;
    EndPoint back = Walk(_range.begin, false, unit, 0, 0, &walked);

    EndPoint destination = Walk(back, count > 0, unit, 0, abs(count), retVal);

    _range.begin = destination;
    _range.end = destination;

    if (!isDegenerate)
    {
        return ExpandToEnclosingUnit(unit);
    }

    return S_OK;
}


HRESULT STDMETHODCALLTYPE TextAreaTextRange::MoveEndpointByUnit(_In_ TextPatternRangeEndpoint endpoint, _In_ TextUnit unit, _In_ int count, _Out_ int *retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }
    *retVal = 0;

    if (endpoint == TextPatternRangeEndpoint_Start)
    {
        _range.begin = Walk(_range.begin, count > 0, unit, 0, abs(count), retVal);
        if (QuickCompareEndpoints(_range.begin, _range.end) > 0)
        {
            _range.end = _range.begin;
        }
    }
    else
    {
        _range.end = Walk(_range.end, count > 0, unit, 0, abs(count), retVal);
        if (QuickCompareEndpoints(_range.begin, _range.end) > 0)
        {
            _range.begin = _range.end;
        }
    }


    return S_OK;
}


HRESULT STDMETHODCALLTYPE TextAreaTextRange::MoveEndpointByRange(_In_ TextPatternRangeEndpoint endpoint, _In_opt_ ITextRangeProvider *targetRange, _In_ TextPatternRangeEndpoint targetEndpoint)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    if (targetRange == NULL)
    {
        return E_INVALIDARG;
    }

    TextAreaTextRange *rangeInternal;
    if (FAILED(targetRange->QueryInterface(IID_PPV_ARGS(&rangeInternal))))
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    if (_hwnd != rangeInternal->_hwnd)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        EndPoint src;
        if (targetEndpoint == TextPatternRangeEndpoint_Start)
        {
            src = rangeInternal->_range.begin;
        }
        else
        {
            src = rangeInternal->_range.end;
        }

        if (endpoint == TextPatternRangeEndpoint_Start)
        {
            _range.begin = src;
            if (QuickCompareEndpoints(_range.begin, _range.end) > 0)
            {
                _range.end = _range.begin;
            }
        }
        else
        {
            _range.end = src;
            if (QuickCompareEndpoints(_range.begin, _range.end) > 0)
            {
                _range.begin = _range.end;
            }
        }
    }
    rangeInternal->Release();
    return hr;
}

// This control does not support selection
HRESULT STDMETHODCALLTYPE TextAreaTextRange::Select()
{
    return UIA_E_INVALIDOPERATION;
}


// This control does not support selection
HRESULT STDMETHODCALLTYPE TextAreaTextRange::AddToSelection()
{
    return UIA_E_INVALIDOPERATION;
}


// This control does not support selection
HRESULT STDMETHODCALLTYPE TextAreaTextRange::RemoveFromSelection()
{
    return UIA_E_INVALIDOPERATION;
}


HRESULT STDMETHODCALLTYPE TextAreaTextRange::ScrollIntoView(_In_ BOOL alignToTop)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    // The ScrollInView operation is not implemented in this sample
    UNREFERENCED_PARAMETER(alignToTop);
    return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE TextAreaTextRange::GetChildren(_Outptr_result_maybenull_ SAFEARRAY ** retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    // No children
    *retVal = NULL;
    return S_OK;
}

bool TextAreaTextRange::CheckEndPointIsUnitEndpoint(_In_ EndPoint check, _In_ TextUnit unit, _In_ TEXTATTRIBUTEID specificAttribute)
{
    if (unit == TextUnit_Character)
    {
        return true;
    }

    EndPoint next;
    EndPoint prev;
    if (!_control->StepCharacter(check, true, &next) ||
        !_control->StepCharacter(check, false, &prev))
    {
        // If we're at the beginning or end, we're at an endpoint
        return true;
    }

    else if (unit == TextUnit_Word)
    {
        if (IsWhiteSpace(prev) && !IsWhiteSpace(check))
        {
            return true;
        }
        return false;
    }

    else if (unit == TextUnit_Line || unit == TextUnit_Paragraph)
    {
        return check.line != next.line;
    }

    // TextUnit_Page and TextUnit_Document are covered by the initial beginning/end check
    else if (unit == TextUnit_Page || unit == TextUnit_Document)
    {
        return false;
    }

    else if (unit == TextUnit_Format)
    {
        bool matching = true;
        bool checkedLineBoundary = false;
        
        // There are limited attributes that vary in this control
        // If its not one of those attributes, then it is not an Endpoint
        // unless it's the document start or end, which is checked above
        for (int i = 0; i < ARRAYSIZE(lineVariableAttributes); i++)
        {
            if (specificAttribute == 0 || specificAttribute == lineVariableAttributes[i])
            {
                if (!checkedLineBoundary)
                {
                    if (!CheckEndPointIsUnitEndpoint(check, TextUnit_Paragraph, 0))
                    {
                        break;
                    }
                    checkedLineBoundary = true;
                }

                VARIANT varC = _control->GetAttributeAtPoint(check, lineVariableAttributes[i]);
                VARIANT varN = _control->GetAttributeAtPoint(next, lineVariableAttributes[i]);
                HRESULT hr = VarCmp(&varC, &varN, LOCALE_NEUTRAL);
                VariantClear(&varC);
                VariantClear(&varN);
                if (hr != VARCMP_EQ)
                {
                    matching = false;
                    break;
                }
            }
        }

        for (int i = 0; i < ARRAYSIZE(charVariableAttributes); i++)
        {
            if (specificAttribute == 0 || specificAttribute == charVariableAttributes[i])
            {
                int *annotationIds;
                int annotationCount;
                if (GetAnnotationsAtPoint(check, &annotationIds, &annotationCount))
                {
                    int *prevAnnotationIds;
                    int prevAnnotationCount;
                    if (GetAnnotationsAtPoint(prev, &prevAnnotationIds, &prevAnnotationCount))
                    {
                        if (annotationCount != prevAnnotationCount)
                        {
                            matching = false;
                        }
                        // Since all our annotations are the same type, if the number matches,
                        // then the UIA_AnnotationTypesAttributeId all match
                        else if (charVariableAttributes[i] == UIA_AnnotationObjectsAttributeId)
                        {
                            for (int j = 0; j < annotationCount; j++)
                            {
                                if (annotationIds[j] != prevAnnotationIds[j])
                                {
                                    matching = false;
                                }
                            }
                        }
                        delete [] prevAnnotationIds;
                    }
                    delete [] annotationIds;
                }
            }
        }
        return !matching;
    }

    return false;
}

EndPoint TextAreaTextRange::Walk(_In_ EndPoint start, _In_ bool forward, _In_ TextUnit unit, _In_ TEXTATTRIBUTEID specificAttribute, _In_ int count, _Out_ int *walked)
{
    *walked = 0;

    // Use count of zero to normalize
    if (count == 0)
    {
        if (CheckEndPointIsUnitEndpoint(start, unit, specificAttribute))
        {
            return start;
        }
        count = 1;
    }

    TextUnit walkUnit = unit;
    if (unit == TextUnit_Word)
    {
        walkUnit = TextUnit_Character;
    }

    if (unit == TextUnit_Line)
    {
        walkUnit = TextUnit_Paragraph;
    }

    if (unit == TextUnit_Page)
    {
        walkUnit = TextUnit_Document;
    }

    if (unit == TextUnit_Format)
    {
        walkUnit = TextUnit_Character;
        for (int i = 0; i < ARRAYSIZE(lineVariableAttributes); i++)
        {
            if (specificAttribute == lineVariableAttributes[i])
            {
                walkUnit = TextUnit_Paragraph;
                break;
            }
        }
    }

    EndPoint current = start;
    for (int i = 0; i < count; i++)
    {
        EndPoint checkNext;
        if (!_control->StepCharacter(current, forward, &checkNext))
        {
            // If we're at the beginning or end... so stop now and return
            break;
        }

        do
        {
            if (walkUnit == TextUnit_Character)
            {
                EndPoint next;
                if (_control->StepCharacter(current, forward, &next))
                {
                    current = next;
                }
            }
            else if (walkUnit == TextUnit_Paragraph)
            {
                EndPoint next;
                if (_control->StepLine(current, forward, &next))
                {
                    current = next;
                }
            }
            else if (walkUnit == TextUnit_Document)
            {
                if (forward)
                {
                    current = _control->GetEnd();
                }
                else
                {
                    current.line = 0;
                    current.character = 0;
                }
            }
        }
        while (!CheckEndPointIsUnitEndpoint(current, unit, specificAttribute));
        (*walked)++;
    }

    // When walking backwards, clients expect negative values for distance walked
    if (!forward)
    {
        *walked = -*walked;
    }
    
    return current;
}

bool TextAreaTextRange::GetAnnotationsAtPoint(_In_ EndPoint endPoint, _Outptr_result_buffer_(*annotationCount) int **annotationIds, _Out_ int *annotationCount)
{
    *annotationCount = 0;
    *annotationIds = NULL;

    unsigned int totalCount = _control->GetAnnotationCount();
    unsigned int atPointCount = 0;
    for (unsigned int i = 0; i < totalCount; i++)
    {
        Annotation *annotation = _control->GetAnnotation(i);
        if (annotation != NULL)
        {
            if (annotation->line == endPoint.line &&
                endPoint.character >= annotation->start_char &&
                (endPoint.character < annotation->start_char + annotation->length ||
                 annotation->length == -1))
            {
                atPointCount++;
            }
        }
    }

    *annotationIds = new int[atPointCount];
    if (*annotationIds == NULL)
    {
        return false;
    }

    for (unsigned int i = 0; i < totalCount && (*annotationCount) < (int)atPointCount; i++)
    {
        Annotation *annotation = _control->GetAnnotation(i);
        if (annotation != NULL)
        {
            if (annotation->line == endPoint.line &&
                endPoint.character >= annotation->start_char &&
                (endPoint.character < annotation->start_char + annotation->length ||
                 annotation->length == -1))
            {
                (*annotationIds)[*annotationCount] = i;
                (*annotationCount)++;
            }
        }
    }
    return true;
}

bool TextAreaTextRange::IsWhiteSpace(_In_ EndPoint check)
{
    if (check.character >= _control->GetLineLength(check.line))
    {
        return true;
    }

    TextLine *line = _control->GetLine(check.line);

    int isSpace = iswspace(line->text[check.character]);
    return isSpace != 0;
}