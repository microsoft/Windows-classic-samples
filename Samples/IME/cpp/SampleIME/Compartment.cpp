// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Compartment.h"
#include "Globals.h"

//////////////////////////////////////////////////////////////////////
//
// CCompartment
//
//////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
// ctor
//----------------------------------------------------------------------------

CCompartment::CCompartment(_In_ IUnknown* punk, TfClientId tfClientId, _In_ REFGUID guidCompartment)
{
    _guidCompartment = guidCompartment;

    _punk = punk;
    _punk->AddRef();

    _tfClientId = tfClientId;
}

//+---------------------------------------------------------------------------
// dtor
//----------------------------------------------------------------------------

CCompartment::~CCompartment()
{
    _punk->Release();
}

//+---------------------------------------------------------------------------
// _GetCompartment
//----------------------------------------------------------------------------

HRESULT CCompartment::_GetCompartment(_Outptr_ ITfCompartment **ppCompartment)
{
    HRESULT hr = S_OK;
    ITfCompartmentMgr* pCompartmentMgr = nullptr;

    hr = _punk->QueryInterface(IID_ITfCompartmentMgr, (void **)&pCompartmentMgr);
    if (SUCCEEDED(hr))
    {
        hr = pCompartmentMgr->GetCompartment(_guidCompartment, ppCompartment);
        pCompartmentMgr->Release();
    }

    return hr;
}

//+---------------------------------------------------------------------------
// _GetCompartmentBOOL
//----------------------------------------------------------------------------

HRESULT CCompartment::_GetCompartmentBOOL(_Out_ BOOL &flag)
{
    HRESULT hr = S_OK;
    ITfCompartment* pCompartment = nullptr;
    flag = FALSE;

    if ((hr = _GetCompartment(&pCompartment)) == S_OK)
    {
        VARIANT var;
        if ((hr = pCompartment->GetValue(&var)) == S_OK)
        {
            if (var.vt == VT_I4) // Even VT_EMPTY, GetValue() can succeed
            {
                flag = (BOOL)var.lVal;
            }
            else
            {
                hr = S_FALSE;
            }
        }
        pCompartment->Release();
    }

    return hr;
}

//+---------------------------------------------------------------------------
// _SetCompartmentBOOL
//----------------------------------------------------------------------------

HRESULT CCompartment::_SetCompartmentBOOL(_In_ BOOL flag)
{
    HRESULT hr = S_OK;
    ITfCompartment* pCompartment = nullptr;

    hr = _GetCompartment(&pCompartment);
    if (SUCCEEDED(hr))
    {
        VARIANT var;
        var.vt = VT_I4;
        var.lVal = flag;
        hr = pCompartment->SetValue(_tfClientId, &var);
        pCompartment->Release();
    }

    return hr;
}

//+---------------------------------------------------------------------------
// _GetCompartmentDWORD
//----------------------------------------------------------------------------

HRESULT CCompartment::_GetCompartmentDWORD(_Out_ DWORD &dw)
{
    HRESULT hr = S_OK;
    ITfCompartment* pCompartment = nullptr;
    dw = 0;

    hr = _GetCompartment(&pCompartment);
    if (SUCCEEDED(hr))
    {
        VARIANT var;
        if ((hr = pCompartment->GetValue(&var)) == S_OK)
        {
            if (var.vt == VT_I4) // Even VT_EMPTY, GetValue() can succeed
            {
                dw = (DWORD)var.lVal;
            }
            else
            {
                hr = S_FALSE;
            }
        }
        pCompartment->Release();
    }

    return hr;
}

//+---------------------------------------------------------------------------
// _SetCompartmentDWORD
//----------------------------------------------------------------------------

HRESULT CCompartment::_SetCompartmentDWORD(_In_ DWORD dw)
{
    HRESULT hr = S_OK;
    ITfCompartment* pCompartment = nullptr;

    hr = _GetCompartment(&pCompartment);
    if (SUCCEEDED(hr))
    {
        VARIANT var;
        var.vt = VT_I4;
        var.lVal = dw;
        hr = pCompartment->SetValue(_tfClientId, &var);
        pCompartment->Release();
    }

    return hr;
}

//+---------------------------------------------------------------------------
//
// _ClearCompartment
//
//----------------------------------------------------------------------------

HRESULT CCompartment::_ClearCompartment()
{
    if (IsEqualGUID(_guidCompartment, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE))
    {
        return S_FALSE;
    }

    HRESULT hr = S_OK;
    ITfCompartmentMgr* pCompartmentMgr = nullptr;

    if ((hr = _punk->QueryInterface(IID_ITfCompartmentMgr, (void **)&pCompartmentMgr)) == S_OK)
    {
        hr = pCompartmentMgr->ClearCompartment(_tfClientId, _guidCompartment);
        pCompartmentMgr->Release();
    }

    return hr;
}

//////////////////////////////////////////////////////////////////////
//
// CCompartmentEventSink
//
//////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
// ctor
//----------------------------------------------------------------------------

CCompartmentEventSink::CCompartmentEventSink(_In_ CESCALLBACK pfnCallback, _In_ void *pv)
{
    _pfnCallback = pfnCallback;
    _pv = pv;
    _refCount = 1;
}

//+---------------------------------------------------------------------------
// dtor
//----------------------------------------------------------------------------

CCompartmentEventSink::~CCompartmentEventSink()
{
}

//+---------------------------------------------------------------------------
//
// QueryInterface
//
//----------------------------------------------------------------------------

STDAPI CCompartmentEventSink::QueryInterface(REFIID riid, _Outptr_ void **ppvObj)
{
    if (ppvObj == nullptr)
        return E_INVALIDARG;

    *ppvObj = nullptr;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfCompartmentEventSink))
    {
        *ppvObj = (CCompartmentEventSink *)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}


//+---------------------------------------------------------------------------
//
// AddRef
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CCompartmentEventSink::AddRef()
{
    return ++_refCount;
}

//+---------------------------------------------------------------------------
//
// Release
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CCompartmentEventSink::Release()
{
    LONG cr = --_refCount;

    assert(_refCount >= 0);

    if (_refCount == 0)
    {
        delete this;
    }

    return cr;
}

//+---------------------------------------------------------------------------
//
// OnChange
//
//----------------------------------------------------------------------------

STDAPI CCompartmentEventSink::OnChange(_In_ REFGUID guidCompartment)
{
    return _pfnCallback(_pv, guidCompartment);
}

//+---------------------------------------------------------------------------
//
// _Advise
//
//----------------------------------------------------------------------------

HRESULT CCompartmentEventSink::_Advise(_In_ IUnknown *punk, _In_ REFGUID guidCompartment)
{
    HRESULT hr = S_OK;
    ITfCompartmentMgr* pCompartmentMgr = nullptr;
    ITfSource* pSource = nullptr;

    hr = punk->QueryInterface(IID_ITfCompartmentMgr, (void **)&pCompartmentMgr);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = pCompartmentMgr->GetCompartment(guidCompartment, &_pCompartment);
    if (SUCCEEDED(hr))
    {
        hr = _pCompartment->QueryInterface(IID_ITfSource, (void **)&pSource);
        if (SUCCEEDED(hr))
        {
            hr = pSource->AdviseSink(IID_ITfCompartmentEventSink, this, &_dwCookie);
            pSource->Release();
        }
    }

    pCompartmentMgr->Release();

    return hr;
}

//+---------------------------------------------------------------------------
//
// _Unadvise
//
//----------------------------------------------------------------------------

HRESULT CCompartmentEventSink::_Unadvise()
{
    HRESULT hr = S_OK;
    ITfSource* pSource = nullptr;

    hr = _pCompartment->QueryInterface(IID_ITfSource, (void **)&pSource);
    if (SUCCEEDED(hr))
    {
        hr = pSource->UnadviseSink(_dwCookie);
        pSource->Release();
    }

    _pCompartment->Release();
    _pCompartment = nullptr;
    _dwCookie = 0;

    return hr;
}
