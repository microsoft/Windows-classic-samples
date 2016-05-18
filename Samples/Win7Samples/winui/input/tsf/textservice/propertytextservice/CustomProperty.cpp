//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  CustomProperty.cpp
//
//          Attach custom compact property
//
//////////////////////////////////////////////////////////////////////

#include "globals.h"
#include "TextService.h"
#include "EditSession.h"
#include "CustomPropertyStore.h"

//+---------------------------------------------------------------------------
//
// CCustomPropertyEditSession
//
//----------------------------------------------------------------------------

class CCustomPropertyEditSession : public CEditSessionBase
{
public:
    CCustomPropertyEditSession(CTextService *pTextService, ITfContext *pContext) : CEditSessionBase(pTextService, pContext)
    {
    }

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec);
};

//+---------------------------------------------------------------------------
//
// DoEditSession
//
//----------------------------------------------------------------------------

STDAPI CCustomPropertyEditSession::DoEditSession(TfEditCookie ec)
{
    ITfProperty *pProperty;
    TF_SELECTION tfSelection;
    ULONG cFetched;

    if (_pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK || cFetched != 1)
        return S_FALSE;

    if (_pContext->GetProperty(c_guidPropCustom, &pProperty) == S_OK)
    {
        CCustomPropertyStore *pCustomPropertyStore = new CCustomPropertyStore();
        if (pCustomPropertyStore != NULL)
            pProperty->SetValueStore(ec, tfSelection.range, pCustomPropertyStore);
        pProperty->Release();
    }

    tfSelection.range->Release();
    return S_OK;
}


//----------------------------------------------------------------------------
//
// _AttachCustomProperty
//
//----------------------------------------------------------------------------

void CTextService::_AttachCustomProperty()
{
    ITfDocumentMgr *pDocMgrFocus;
    ITfContext *pContext;

    if ((_pThreadMgr->GetFocus(&pDocMgrFocus) == S_OK) &&
        (pDocMgrFocus != NULL))
    {
        if (pDocMgrFocus->GetTop(&pContext) == S_OK)
        {
            CCustomPropertyEditSession *pEditSession;
            if (pEditSession = new CCustomPropertyEditSession(this, pContext))
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
