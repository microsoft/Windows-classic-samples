//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  EnumDisplayAttributeInfo.cpp
//
//          ITfEnumDisplayAttributeInfo implementation.
//
//////////////////////////////////////////////////////////////////////

#include "globals.h"
#include "TextService.h"
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

    _iIndex = 0;
    _cRef = 1;
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

STDAPI CEnumDisplayAttributeInfo::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

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
    return ++_cRef;
}

//+---------------------------------------------------------------------------
//
// Release
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CEnumDisplayAttributeInfo::Release()
{
    LONG cr = --_cRef;

    assert(_cRef >= 0);

    if (_cRef == 0)
    {
        delete this;
    }

    return cr;
}

//+---------------------------------------------------------------------------
//
// Clone
//
// Returns a copy of the object.
//----------------------------------------------------------------------------

STDAPI CEnumDisplayAttributeInfo::Clone(IEnumTfDisplayAttributeInfo **ppEnum)
{
    CEnumDisplayAttributeInfo *pClone;

    if (ppEnum == NULL)
        return E_INVALIDARG;

    *ppEnum = NULL;

    if ((pClone = new CEnumDisplayAttributeInfo) == NULL)
        return E_OUTOFMEMORY;

    // the clone should match this object's state
    pClone->_iIndex = _iIndex;

    *ppEnum = pClone;

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// Next
//
// Returns an array of display attribute info objects supported by this service.
//----------------------------------------------------------------------------

STDAPI CEnumDisplayAttributeInfo::Next(ULONG ulCount, ITfDisplayAttributeInfo **rgInfo, ULONG *pcFetched)
{
    ITfDisplayAttributeInfo *pDisplayAttributeInfo;
    ULONG cFetched;

    cFetched = 0;

    if (ulCount == 0)
        return S_OK;

    while (cFetched < ulCount)
    {
        if (_iIndex > 1)
            break;

        if (_iIndex == 0)
        {
            if ((pDisplayAttributeInfo = new CDisplayAttributeInfoInput()) == NULL)
                return E_OUTOFMEMORY;
        }
        else if (_iIndex == 1)
        {
            if ((pDisplayAttributeInfo = new CDisplayAttributeInfoConverted()) == NULL)
                return E_OUTOFMEMORY;
 
        }
        

        *rgInfo = pDisplayAttributeInfo;
        cFetched++;
        _iIndex++;
    }

    if (pcFetched != NULL)
    {
        // technically this is only legal if ulCount == 1, but we won't check
        *pcFetched = cFetched;
    }

    return (cFetched == ulCount) ? S_OK : S_FALSE;
}

//+---------------------------------------------------------------------------
//
// Reset
//
// Resets the enumeration.
//----------------------------------------------------------------------------

STDAPI CEnumDisplayAttributeInfo::Reset()
{
    _iIndex = 0;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// Skip
//
// Skips past objects in the enumeration.
//----------------------------------------------------------------------------

STDAPI CEnumDisplayAttributeInfo::Skip(ULONG ulCount)
{
    // we have only a single item to enum
    // so we can just skip it and avoid any overflow errors
    if (ulCount > 0 && _iIndex == 0)
    {
        _iIndex++;
    }
    return S_OK;
}

