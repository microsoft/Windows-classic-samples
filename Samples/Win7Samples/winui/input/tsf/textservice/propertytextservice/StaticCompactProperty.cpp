//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  StaticCompactProperty.cpp
//
//          Attach static compact property
//
//////////////////////////////////////////////////////////////////////

#include "globals.h"
#include "TextService.h"
#include "EditSession.h"

//+---------------------------------------------------------------------------
//
// CStaticCompactPropertyEditSession
//
//----------------------------------------------------------------------------

class CStaticCompactPropertyEditSession : public CEditSessionBase
{
public:
    CStaticCompactPropertyEditSession(CTextService *pTextService, ITfContext *pContext, DWORD dwValue) : CEditSessionBase(pTextService, pContext)
    {
        _dwValue = dwValue;
    }

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec);

private:
    DWORD _dwValue;
};

//+---------------------------------------------------------------------------
//
// DoEditSession
//
//----------------------------------------------------------------------------

STDAPI CStaticCompactPropertyEditSession::DoEditSession(TfEditCookie ec)
{
    ITfProperty *pProperty;
    TF_SELECTION tfSelection;
    ULONG cFetched;

    if (_pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK || cFetched != 1)
        return S_FALSE;

    if (_pContext->GetProperty(c_guidPropStaticCompact, &pProperty) == S_OK)
    {
        VARIANT var;
        var.vt = VT_I4; 
        var.lVal = _dwValue;
        pProperty->SetValue(ec, tfSelection.range, &var);
        pProperty->Release();
    }

    tfSelection.range->Release();
    return S_OK;
}


//----------------------------------------------------------------------------
//
// _AttachStaticCompactProperty
//
//----------------------------------------------------------------------------

void CTextService::_AttachStaticCompactProperty(DWORD dwValue)
{
    ITfDocumentMgr *pDocMgrFocus;
    ITfContext *pContext;

    if ((_pThreadMgr->GetFocus(&pDocMgrFocus) == S_OK) &&
        (pDocMgrFocus != NULL))
    {
        if (pDocMgrFocus->GetTop(&pContext) == S_OK)
        {
            CStaticCompactPropertyEditSession *pEditSession;
            if (pEditSession = new CStaticCompactPropertyEditSession(this, pContext, dwValue))
            {
                HRESULT hr;
                pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
                pEditSession->Release();
            }
            pContext->Release();
        }
        pDocMgrFocus->Release();
    }

    return;
}
