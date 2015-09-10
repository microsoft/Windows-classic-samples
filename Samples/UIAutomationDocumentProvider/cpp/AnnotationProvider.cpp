// Copyright (c) Microsoft Corporation
//
#include <windows.h>
#include <ole2.h>
#include <uiautomation.h>
#include <strsafe.h>

#include "AnnotatedTextControl.h"
#include "FrameProvider.h"
#include "AnnotationProvider.h"
#include "TextAreaProvider.h"

// Implementation for the AnnotationProvider
AnnotationProvider::AnnotationProvider(_In_ HWND hwnd, _In_ AnnotatedTextControl *control, _In_ int annotationId)
    : _refCount(1)
{
    _hwnd = hwnd;
    _control = control;
     _annotationId = annotationId;
     _annotation = NULL;
}

AnnotationProvider::~AnnotationProvider()
{
}

ULONG STDMETHODCALLTYPE AnnotationProvider::AddRef()
{
    return ++_refCount;
}

ULONG STDMETHODCALLTYPE AnnotationProvider::Release()
{
    if (--_refCount <= 0)
    {
        delete this;
        return 0;
    }
    return _refCount;
}

HRESULT STDMETHODCALLTYPE AnnotationProvider::QueryInterface(_In_ REFIID riid, _Outptr_ void** ppInterface)
{
    if (riid == __uuidof(IUnknown))                              *ppInterface =(IUnknown*)((IRawElementProviderSimple*)this);
    else if (riid == __uuidof(IRawElementProviderSimple))        *ppInterface =(IRawElementProviderSimple*)this;
    else if (riid == __uuidof(IRawElementProviderFragment))      *ppInterface =(IRawElementProviderFragment*)this;
    else if (riid == __uuidof(IAnnotationProvider))              *ppInterface =(IAnnotationProvider*)this;
    else if (riid == __uuidof(AnnotationProvider))               *ppInterface =this;
    else
    {
        *ppInterface = NULL;
        return E_NOINTERFACE;
    }

    ((IUnknown*)(*ppInterface))->AddRef();
    return S_OK;
}

// Verifies that the control and the specific annotation still exist
bool AnnotationProvider::CheckAnnotation()
{
    _annotation = NULL;

    if (!IsWindow(_hwnd))
    {
        return false;
    }

    _annotation = _control->GetAnnotation(_annotationId);
    return (_annotation != NULL);
}


// IRawElementProviderSimple Implementation
HRESULT STDMETHODCALLTYPE AnnotationProvider::get_ProviderOptions(_Out_ ProviderOptions * retVal)
{
    if (!CheckAnnotation())
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    *retVal = ProviderOptions_ServerSideProvider | ProviderOptions_UseComThreading;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE AnnotationProvider::GetPatternProvider(PATTERNID patternId, _Outptr_result_maybenull_ IUnknown ** retVal)
{
    if (!CheckAnnotation())
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    *retVal = NULL;
    if (patternId == UIA_AnnotationPatternId)
    {
        *retVal = static_cast<IAnnotationProvider *>(this);
        (*retVal)->AddRef();
        return S_OK;
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE AnnotationProvider::GetPropertyValue(PROPERTYID idProp, _Out_ VARIANT * retVal)
{
    if (!CheckAnnotation())
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    retVal->vt = VT_EMPTY;
    if (idProp == UIA_ControlTypePropertyId)
    {
        // Since the annotations are static text labels, the Text controltype is used
        // if they were editable, the Edit controltype would be more appropriate.
        retVal->vt = VT_I4;
        retVal->lVal = UIA_TextControlTypeId;
    }
    else if (idProp == UIA_NamePropertyId)
    {
        // Since they are Text controls, the name should be the text they show
        retVal->bstrVal = SysAllocString(_annotation->text);
        if (retVal->bstrVal != NULL)
        {
            retVal->vt = VT_BSTR;
        } 
    }
    else if (idProp == UIA_AutomationIdPropertyId)
    {
         WCHAR autoId[100];
         StringCchPrintf(autoId, ARRAYSIZE(autoId), L"Annotation_%d", _annotation->id);
         retVal->bstrVal = SysAllocString(autoId);
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
    else if (idProp == UIA_IsOffscreenPropertyId)
    {
        UiaRect boundingRect;
        if (SUCCEEDED(get_BoundingRectangle(&boundingRect)))
        {
            retVal->vt = VT_BOOL;
            retVal->boolVal = UiaRectIsEmpty(boundingRect) ? VARIANT_TRUE : VARIANT_FALSE;
        }
    }
    else if (idProp == UIA_ProviderDescriptionPropertyId)
    {
        retVal->bstrVal = SysAllocString(L"Microsoft Sample: Uia Document Content Annotation");
        if (retVal->bstrVal != NULL)
        {
            retVal->vt = VT_BSTR;
        }
    }

    return S_OK;
}


HRESULT STDMETHODCALLTYPE AnnotationProvider::get_HostRawElementProvider(_Outptr_result_maybenull_ IRawElementProviderSimple ** retVal)
{
    if (!CheckAnnotation())
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    *retVal = NULL;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE AnnotationProvider::Navigate(NavigateDirection direction, _Outptr_result_maybenull_ IRawElementProviderFragment ** retVal)
{
    if (!CheckAnnotation())
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }
    *retVal = NULL;

    HRESULT hr = S_OK;
    int destination = -2;
    switch(direction)
    {
    case NavigateDirection_Parent:
        {
            *retVal = new FrameProvider(_hwnd, _control);
            if (*retVal == NULL)
            {
                hr = E_OUTOFMEMORY;
            }
            break;
        }
    case NavigateDirection_NextSibling:       destination = _annotationId + 1; break;
    case NavigateDirection_PreviousSibling:   destination = _annotationId - 1; break;
    case NavigateDirection_FirstChild:        break;
    case NavigateDirection_LastChild:         break;
    }

    if (destination == -1)
    {
        // it has walked before the first annotation, which puts as at the text area
        *retVal = new TextAreaProvider(_hwnd, _control);
        if (*retVal == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (destination >= 0 && static_cast<unsigned int>(destination) < _control->GetAnnotationCount())
    {
        *retVal = new AnnotationProvider(_hwnd, _control, destination);
        if (*retVal == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE AnnotationProvider::GetRuntimeId(_Outptr_result_maybenull_ SAFEARRAY ** retVal)
{
    if (!CheckAnnotation())
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }
    // AppendRuntimeId is a magic Number that tells UIAutomation to Append its own Runtime ID(From the HWND)
    int rId[] = { UiaAppendRuntimeId, _annotationId};

    // BuildIntSafeArray is a custom function to hide the SafeArray creation
    HRESULT hr = S_OK;
    *retVal = BuildIntSafeArray(rId, 2);
    if (*retVal == NULL)
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE AnnotationProvider::get_BoundingRectangle(_Out_ UiaRect * retVal)
{
    if (!CheckAnnotation())
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }
    ZeroMemory(retVal, sizeof(*retVal));

    Graphics graphics(_hwnd);
    RectF rectClient = _control->GetAnnotationPosition(_annotationId, &graphics);
    RECT rc = {static_cast<int>(rectClient.X),
               static_cast<int>(rectClient.Y),
               static_cast<int>(rectClient.X + rectClient.Width),
               static_cast<int>(rectClient.Y + rectClient.Height)};
    
    // Limit the bounding rect to inside the control
    HRESULT hr = S_OK;
    RECT winRect;
    if (!GetClientRect(_hwnd, &winRect))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    if (SUCCEEDED(hr))
    {
        RECT intersection;
        if (IntersectRect(&intersection, &winRect, &rc))
        {
            // Map to screen coordinates
            MapWindowPoints(_hwnd, NULL, reinterpret_cast<POINT *>(&intersection), 2);

            retVal->left = intersection.left;
            retVal->top = intersection.top;
            retVal->width = intersection.right - intersection.left;
            retVal->height = intersection.bottom - intersection.top;
        }
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE AnnotationProvider::GetEmbeddedFragmentRoots(_Outptr_result_maybenull_ SAFEARRAY ** retVal)
{
    if (!CheckAnnotation())
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    *retVal = NULL;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE AnnotationProvider::SetFocus()
{
    if (!CheckAnnotation())
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE AnnotationProvider::get_FragmentRoot(_Outptr_result_maybenull_ IRawElementProviderFragmentRoot ** retVal)
{
    if (!CheckAnnotation())
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    *retVal = new FrameProvider(_hwnd, _control);
    return (*retVal == NULL) ? E_OUTOFMEMORY : S_OK;
}

HRESULT STDMETHODCALLTYPE AnnotationProvider::get_AnnotationTypeId(_Out_ int *retVal)
{
    if (!CheckAnnotation())
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    // All the annotations in this sample are comments
    *retVal= AnnotationType_Comment;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE AnnotationProvider::get_AnnotationTypeName(_Outptr_result_maybenull_ BSTR *retVal)
{
    if (!CheckAnnotation())
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    // UI Automation core will fill this in for standard annotation types like comment
    *retVal = NULL;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE AnnotationProvider::get_Author(_Outptr_result_maybenull_ BSTR *retVal)
{
    if (!CheckAnnotation())
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    *retVal = SysAllocString(_annotation->author);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE AnnotationProvider::get_DateTime(_Outptr_result_maybenull_ BSTR *retVal)
{
    if (!CheckAnnotation())
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    *retVal = NULL;

    // Format the SYSTEM_TIME into a locale appropriate string
    WCHAR formattedDate[100];
    WCHAR formattedTime[100];
    WCHAR dateTime[200];
    HRESULT hr = S_OK;
    if (GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, TIME_NOSECONDS,
        &_annotation->time, NULL, formattedTime, ARRAYSIZE(formattedTime)) != 0 &&
        GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, DATE_LONGDATE,
        &_annotation->time, NULL, formattedDate, ARRAYSIZE(formattedDate), NULL) != 0 &&
        SUCCEEDED(StringCchPrintf(dateTime, ARRAYSIZE(dateTime), L"%s %s", formattedDate, formattedTime)))
    {
        *retVal = SysAllocString(dateTime);
        if (*retVal == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE AnnotationProvider::get_Target(_Outptr_result_maybenull_ IRawElementProviderSimple **retVal)
{
    if (!CheckAnnotation())
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    // All the annotations point to text inside the text area
    HRESULT hr = S_OK;
    *retVal = new TextAreaProvider(_hwnd, _control);
    if (*retVal == NULL)
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

int AnnotationProvider::GetAnnotationId()
{
    return _annotationId;
}

HWND AnnotationProvider::GetControlHwnd()
{
    return _hwnd;
}
