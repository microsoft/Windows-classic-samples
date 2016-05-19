//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  EndComposition.cpp
//
//          terminate the compositon object
//
//////////////////////////////////////////////////////////////////////

#include "Globals.h"
#include "EditSession.h"
#include "TextService.h"

//+---------------------------------------------------------------------------
//
// CEndCompositionEditSession
//
//----------------------------------------------------------------------------

class CEndCompositionEditSession : public CEditSessionBase
{
public:
    CEndCompositionEditSession(CTextService *pTextService, ITfContext *pContext) : CEditSessionBase(pTextService, pContext)
    {
    }

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec)
    {
        _pTextService->_TerminateComposition(ec);
        return S_OK;
    }

};

//+---------------------------------------------------------------------------
//
// _TerminateComposition
//
//----------------------------------------------------------------------------

void CTextService::_TerminateComposition(TfEditCookie ec)
{
    if (_pComposition != NULL)
    {
        _pComposition->EndComposition(ec);
        _pComposition->Release();
        _pComposition = NULL;
    }
}

//+---------------------------------------------------------------------------
//
// _EndComposition
//
//----------------------------------------------------------------------------

void CTextService::_EndComposition(ITfContext *pContext)
{
    CEndCompositionEditSession *pEditSession;
    HRESULT hr;

    if (pEditSession = new CEndCompositionEditSession(this, pContext))
    {
        pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
        pEditSession->Release();
    }
}

