//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  DisplayAttribureProvider.cpp
//
//          ITfDisplayAttributeProvider implementation.
//
//////////////////////////////////////////////////////////////////////

#include "globals.h"
#include "TextService.h"
#include "DisplayAttributeInfo.h"
#include "EnumDisplayAttributeInfo.h"

//+---------------------------------------------------------------------------
//
// EnumDisplayAttributeInfo
//
//----------------------------------------------------------------------------

STDAPI CTextService::EnumDisplayAttributeInfo(IEnumTfDisplayAttributeInfo **ppEnum)
{
    CEnumDisplayAttributeInfo *pAttributeEnum;

    if (ppEnum == NULL)
        return E_INVALIDARG;

    *ppEnum = NULL;

    if ((pAttributeEnum = new CEnumDisplayAttributeInfo) == NULL)
        return E_OUTOFMEMORY;

    *ppEnum = pAttributeEnum;

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// GetDisplayAttributeInfo
//
//----------------------------------------------------------------------------

STDAPI CTextService::GetDisplayAttributeInfo(REFGUID guidInfo, ITfDisplayAttributeInfo **ppInfo)
{
    if (ppInfo == NULL)
        return E_INVALIDARG;

    *ppInfo = NULL;

    // Which display attribute GUID?
    if (IsEqualGUID(guidInfo, c_guidDisplayAttributeInput))
    {
        if ((*ppInfo = new CDisplayAttributeInfoInput()) == NULL)
            return E_OUTOFMEMORY;
    }
    else if (IsEqualGUID(guidInfo, c_guidDisplayAttributeConverted))
    {
        if ((*ppInfo = new CDisplayAttributeInfoConverted()) == NULL)
            return E_OUTOFMEMORY;
    }
    else
    {
        return E_INVALIDARG;
    }


    return S_OK;
}
