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

// A helper function to create a SafeArray Version of an int array of a specified length
SAFEARRAY * BuildIntSafeArray(_In_reads_(length) const int * data, _In_ int length) 
{
    SAFEARRAY *sa = SafeArrayCreateVector(VT_I4, 0, length);
    if (sa == NULL)
        return NULL;

    for (long i = 0; i < length; i++)
    {
        if (FAILED(SafeArrayPutElement(sa, &i,(void *)&(data[i]))))
        {
            SafeArrayDestroy(sa);
            sa = NULL;
            break;
        }
    }

    return sa;
}

FrameProvider::FrameProvider(_In_ HWND hwnd, _In_ AnnotatedTextControl *control)
    : _refCount(1)
{
    _hwnd = hwnd;
    _control = control;
}

FrameProvider::~FrameProvider()
{
}

ULONG STDMETHODCALLTYPE FrameProvider::AddRef()
{
    return ++_refCount;
}

ULONG STDMETHODCALLTYPE FrameProvider::Release()
{
    if (--_refCount <= 0)
    {
        delete this;
        return 0;
    }
    return _refCount;
}

HRESULT STDMETHODCALLTYPE FrameProvider::QueryInterface(_In_ REFIID riid, _Outptr_ void**ppInterface)
{
    if (riid == __uuidof(IUnknown))                              *ppInterface =(IUnknown*)((IRawElementProviderSimple*)this);
    else if (riid == __uuidof(IRawElementProviderSimple))        *ppInterface =(IRawElementProviderSimple*)this;
    else if (riid == __uuidof(IRawElementProviderFragment))      *ppInterface =(IRawElementProviderFragment*)this;
    else if (riid == __uuidof(IRawElementProviderFragmentRoot))  *ppInterface =(IRawElementProviderFragmentRoot*)this;
    else
    {
        *ppInterface = NULL;
        return E_NOINTERFACE;
    }

    ((IUnknown*)(*ppInterface))->AddRef();
    return S_OK;
}

// IRawElementProviderSimple Implementation

HRESULT STDMETHODCALLTYPE FrameProvider::get_ProviderOptions(_Out_ ProviderOptions * retVal )
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }
    *retVal = ProviderOptions_ServerSideProvider | ProviderOptions_UseComThreading;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE FrameProvider::GetPatternProvider(PATTERNID patternId, _Outptr_result_maybenull_ IUnknown ** retVal)
{
    UNREFERENCED_PARAMETER(patternId);
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    *retVal = NULL;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE FrameProvider::GetPropertyValue(PROPERTYID idProp, _Out_ VARIANT * retVal)
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
        retVal->lVal = UIA_PaneControlTypeId;
    }
    else if (idProp == UIA_AutomationIdPropertyId)
    {
         retVal->bstrVal = SysAllocString(L"AnnotatedTextControl");
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
        retVal->bstrVal = SysAllocString(L"Microsoft Sample: Uia Document Content Control");
        if (retVal->bstrVal != NULL)
        {
            retVal->vt = VT_BSTR;
        }
    }

    return S_OK;
}


HRESULT STDMETHODCALLTYPE FrameProvider::get_HostRawElementProvider(_Outptr_result_maybenull_ IRawElementProviderSimple ** retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    return UiaHostProviderFromHwnd(_hwnd, retVal); 
}

HRESULT STDMETHODCALLTYPE FrameProvider::Navigate(NavigateDirection direction, _Outptr_result_maybenull_ IRawElementProviderFragment ** retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }
    *retVal = NULL;

    if (direction == NavigateDirection_FirstChild ||
        (direction == NavigateDirection_LastChild && _control->GetAnnotationCount() == 0))
    {
        *retVal = new TextAreaProvider(_hwnd, _control);
        if (*retVal == NULL)
        {
            return E_OUTOFMEMORY;
        }
    }
    else if (direction == NavigateDirection_LastChild)
    {
        *retVal = new AnnotationProvider(_hwnd, _control, _control->GetAnnotationCount() - 1);
        if (*retVal == NULL)
        {
            return E_OUTOFMEMORY;
        }
    }

    // For the other directions (parent, next, previous) the default of NULL is correct
    return S_OK;
}

HRESULT STDMETHODCALLTYPE FrameProvider::GetRuntimeId(_Outptr_result_maybenull_ SAFEARRAY ** retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }
    // Root defers this to host, others must implement it...
    *retVal = NULL;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE FrameProvider::get_BoundingRectangle(_Out_ UiaRect * retVal)
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
    retVal->width = rc.right - rc.left;
    retVal->height = rc.bottom - rc.top;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE FrameProvider::GetEmbeddedFragmentRoots(_Outptr_result_maybenull_ SAFEARRAY ** retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    *retVal = NULL;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE FrameProvider::SetFocus()
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE FrameProvider::get_FragmentRoot(_Outptr_result_maybenull_ IRawElementProviderFragmentRoot ** retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    *retVal = this;
    AddRef();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE FrameProvider::ElementProviderFromPoint(double x, double y, _Outptr_result_maybenull_ IRawElementProviderFragment ** retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    *retVal = NULL;

    // Map from screen coordinates to hwnd coordinates
    POINT screenPt = {static_cast<int>(x), static_cast<int>(y)};
    MapWindowPoints(NULL, _hwnd, &screenPt, 1);

    HRESULT hr = S_OK;
    PointF pt(static_cast<float>(screenPt.x), static_cast<float>(screenPt.y));
    Graphics graphics(_hwnd);

    // First check for annotations
    for (unsigned int i = 0; i < _control->GetAnnotationCount(); i++)
    {
        RectF boundingRect = _control->GetAnnotationPosition(i, &graphics);
        if (boundingRect.Contains(pt))
        {
            *retVal = new AnnotationProvider(_hwnd, _control, i);
            if (*retVal == NULL)
            {
                hr = E_OUTOFMEMORY;
            }
            break;
        }
    }

    if (SUCCEEDED(hr) && *retVal == NULL)
    {
        if (pt.X < _control->GetTextAreaWidth())
        {
            *retVal = new TextAreaProvider(_hwnd, _control);
            if (*retVal == NULL)
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE FrameProvider::GetFocus(_Outptr_result_maybenull_ IRawElementProviderFragment ** retVal)
{
    if (!IsWindow(_hwnd))
    {
        return UIA_E_ELEMENTNOTAVAILABLE;
    }

    *retVal = NULL;
    return S_OK;
}
