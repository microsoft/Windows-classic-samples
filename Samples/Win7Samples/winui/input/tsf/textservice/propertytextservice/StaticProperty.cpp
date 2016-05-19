//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  StaticProperty.cpp
//
//          Attach static property
//
//////////////////////////////////////////////////////////////////////

#include "globals.h"
#include "TextService.h"
#include "EditSession.h"

//+---------------------------------------------------------------------------
//
// CStaticPropertyEditSession
//
//----------------------------------------------------------------------------

class CStaticPropertyEditSession : public CEditSessionBase
{
public:
    CStaticPropertyEditSession(CTextService *pTextService, ITfContext *pContext, WCHAR *psz) : CEditSessionBase(pTextService, pContext)
    {
        StringCchCopy(_sz, ARRAYSIZE(_sz), psz);
    }

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec);

private:
    WCHAR _sz[32];
};

//+---------------------------------------------------------------------------
//
// DoEditSession
//
//----------------------------------------------------------------------------

STDAPI CStaticPropertyEditSession::DoEditSession(TfEditCookie ec)
{
    ITfProperty *pProperty;
    TF_SELECTION tfSelection;
    ULONG cFetched;

    if (_pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK || cFetched != 1)
        return S_FALSE;

    if (_pContext->GetProperty(c_guidPropStatic, &pProperty) == S_OK)
    {
        VARIANT var;
        var.vt = VT_BSTR; 
        var.bstrVal = SysAllocString(_sz);
        pProperty->SetValue(ec, tfSelection.range, &var);
        pProperty->Release();
    }

    tfSelection.range->Release();
    return S_OK;
}


//----------------------------------------------------------------------------
//
// _AttachStaticProperty
//
//----------------------------------------------------------------------------

void CTextService::_AttachStaticProperty(WCHAR *psz)
{
    ITfDocumentMgr *pDocMgrFocus;
    ITfContext *pContext;

    if ((_pThreadMgr->GetFocus(&pDocMgrFocus) == S_OK) &&
        (pDocMgrFocus != NULL))
    {
        if (pDocMgrFocus->GetTop(&pContext) == S_OK)
        {
            CStaticPropertyEditSession *pEditSession;
            if (pEditSession = new CStaticPropertyEditSession(this, pContext, psz))
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
