//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  DisplayAttribure.cpp
//
//          apply the display attribute to the composition range.
//
//////////////////////////////////////////////////////////////////////

#include "globals.h"
#include "TextService.h"

//+---------------------------------------------------------------------------
//
// _ClearCompositionDisplayAttributes
//
//----------------------------------------------------------------------------

void CTextService::_ClearCompositionDisplayAttributes(TfEditCookie ec, ITfContext *pContext)
{
    ITfRange *pRangeComposition;
    ITfProperty *pDisplayAttributeProperty;

    // get the compositon range.
    if (_pComposition->GetRange(&pRangeComposition) != S_OK)
        return;

    // get our the display attribute property
    if (pContext->GetProperty(GUID_PROP_ATTRIBUTE, &pDisplayAttributeProperty) == S_OK)
    {
        // clear the value over the range
        pDisplayAttributeProperty->Clear(ec, pRangeComposition);

        pDisplayAttributeProperty->Release();
    }

    pRangeComposition->Release();
}

//+---------------------------------------------------------------------------
//
// _SetCompositionDisplayAttributes
//
//----------------------------------------------------------------------------

BOOL CTextService::_SetCompositionDisplayAttributes(TfEditCookie ec, ITfContext *pContext, TfGuidAtom gaDisplayAttribute)
{
    ITfRange *pRangeComposition;
    ITfProperty *pDisplayAttributeProperty;
    HRESULT hr;

    // we need a range and the context it lives in
    if (_pComposition->GetRange(&pRangeComposition) != S_OK)
        return FALSE;

    hr = E_FAIL;

    // get our the display attribute property
    if (pContext->GetProperty(GUID_PROP_ATTRIBUTE, &pDisplayAttributeProperty) == S_OK)
    {
        VARIANT var;
        // set the value over the range
        // the application will use this guid atom to lookup the acutal rendering information
        var.vt = VT_I4; // we're going to set a TfGuidAtom
        var.lVal = gaDisplayAttribute; 

        hr = pDisplayAttributeProperty->SetValue(ec, pRangeComposition, &var);

        pDisplayAttributeProperty->Release();
    }

    pRangeComposition->Release();
    return (hr == S_OK);
}

//+---------------------------------------------------------------------------
//
// _InitDisplayAttributeGuidAtom
//
// Because it's expensive to map our display attribute GUID to a TSF
// TfGuidAtom, we do it once when Activate is called.
//----------------------------------------------------------------------------

BOOL CTextService::_InitDisplayAttributeGuidAtom()
{
    ITfCategoryMgr *pCategoryMgr;
    HRESULT hr;

    if (CoCreateInstance(CLSID_TF_CategoryMgr,
                         NULL, 
                         CLSCTX_INPROC_SERVER, 
                         IID_ITfCategoryMgr, 
                         (void**)&pCategoryMgr) != S_OK)
    {
        return FALSE;
    }

    // register the display attribute for input text.
    hr = pCategoryMgr->RegisterGUID(c_guidDisplayAttributeInput, &_gaDisplayAttributeInput);

    // register the display attribute for the converted text.
    hr = pCategoryMgr->RegisterGUID(c_guidDisplayAttributeConverted, &_gaDisplayAttributeConverted);

    pCategoryMgr->Release();
        
    return (hr == S_OK);
}
