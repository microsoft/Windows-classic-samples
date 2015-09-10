// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "globals.h"
#include "SampleIME.h"
#include "DisplayAttributeInfo.h"
#include "EnumDisplayAttributeInfo.h"

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

CEnumDisplayAttributeInfo::CEnumDisplayAttributeInfo()
{
    DllAddRef();

    _index = 0;
    _refCount = 1;
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

CEnumDisplayAttributeInfo::~CEnumDisplayAttributeInfo()
{
    DllRelease();
}

//+---------------------------------------------------------------------------
//
// QueryInterface
//
//----------------------------------------------------------------------------

STDAPI CEnumDisplayAttributeInfo::QueryInterface(REFIID riid, _Outptr_ void **ppvObj)
{
    if (ppvObj == nullptr)
        return E_INVALIDARG;

    *ppvObj = nullptr;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IEnumTfDisplayAttributeInfo))
    {
        *ppvObj = (IEnumTfDisplayAttributeInfo *)this;
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

STDAPI_(ULONG) CEnumDisplayAttributeInfo::AddRef()
{
    return ++_refCount;
}

//+---------------------------------------------------------------------------
//
// Release
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CEnumDisplayAttributeInfo::Release()
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
// IEnumTfDisplayAttributeInfo::Clone
//
// Returns a copy of the object.
//----------------------------------------------------------------------------

STDAPI CEnumDisplayAttributeInfo::Clone(_Out_ IEnumTfDisplayAttributeInfo **ppEnum)
{
    CEnumDisplayAttributeInfo* pClone = nullptr;

    if (ppEnum == nullptr)
    {
        return E_INVALIDARG;
    }

    *ppEnum = nullptr;

    pClone = new (std::nothrow) CEnumDisplayAttributeInfo();
    if ((pClone) == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    // the clone should match this object's state
    pClone->_index = _index;

    *ppEnum = pClone;

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// IEnumTfDisplayAttributeInfo::Next
//
// Returns an array of display attribute info objects supported by this service.
//----------------------------------------------------------------------------

const int MAX_DISPLAY_ATTRIBUTE_INFO = 2;

STDAPI CEnumDisplayAttributeInfo::Next(ULONG ulCount, __RPC__out_ecount_part(ulCount, *pcFetched) ITfDisplayAttributeInfo **rgInfo, __RPC__out ULONG *pcFetched)
{
    ULONG fetched;

    fetched = 0;

    if (ulCount == 0)
    {
        return S_OK;
    }
    if (rgInfo == nullptr)
    {
        return E_INVALIDARG;
    }
    *rgInfo = nullptr;

    while (fetched < ulCount)
    {
        ITfDisplayAttributeInfo* pDisplayAttributeInfo = nullptr;

        if (_index == 0)
        {   
            pDisplayAttributeInfo = new (std::nothrow) CDisplayAttributeInfoInput();
            if ((pDisplayAttributeInfo) == nullptr)
            {
                return E_OUTOFMEMORY;
            }
        }
        else if (_index == 1)
        {
            pDisplayAttributeInfo = new (std::nothrow) CDisplayAttributeInfoConverted();
            if ((pDisplayAttributeInfo) == nullptr)
            {
                return E_OUTOFMEMORY;
            }

        }
        else
        {
            break;
        }

        *rgInfo = pDisplayAttributeInfo;
        rgInfo++;
        fetched++;
        _index++;
    }

    if (pcFetched != nullptr)
    {
        // technically this is only legal if ulCount == 1, but we won't check
        *pcFetched = fetched;
    }

    return (fetched == ulCount) ? S_OK : S_FALSE;
}

//+---------------------------------------------------------------------------
//
// IEnumTfDisplayAttributeInfo::Reset
//
// Resets the enumeration.
//----------------------------------------------------------------------------

STDAPI CEnumDisplayAttributeInfo::Reset()
{
    _index = 0;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// IEnumTfDisplayAttributeInfo::Skip
//
// Skips past objects in the enumeration.
//----------------------------------------------------------------------------

STDAPI CEnumDisplayAttributeInfo::Skip(ULONG ulCount)
{
    if ((ulCount + _index) > MAX_DISPLAY_ATTRIBUTE_INFO || (ulCount + _index) < ulCount)
    {
        _index = MAX_DISPLAY_ATTRIBUTE_INFO;
        return S_FALSE;
    }
    _index += ulCount;
    return S_OK;
}
