//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  InsertHello.cpp
//
//          Insert Hello edit session
//
//////////////////////////////////////////////////////////////////////

#include "globals.h"
#include "TextService.h"
#include "EditSession.h"

void InsertTextAtSelection(TfEditCookie ec, ITfContext *pContext, const WCHAR *pchText, ULONG cchText);

//+---------------------------------------------------------------------------
//
// CInsertHelloEditSession
//
//----------------------------------------------------------------------------

class CInsertHelloEditSession : public CEditSessionBase
{
public:
    CInsertHelloEditSession(ITfContext *pContext) : CEditSessionBase(pContext) 
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

STDAPI CInsertHelloEditSession::DoEditSession(TfEditCookie ec)
{
    InsertTextAtSelection(ec, _pContext, L"Hello world!", (ULONG)wcslen(L"Hello world!"));
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// InsertTextAtSelection
//
//----------------------------------------------------------------------------

void InsertTextAtSelection(TfEditCookie ec, ITfContext *pContext, const WCHAR *pchText, ULONG cchText)
{
    ITfInsertAtSelection *pInsertAtSelection;
    ITfRange *pRange;
    TF_SELECTION tfSelection;

    // we need a special interface to insert text at the selection
    if (pContext->QueryInterface(IID_ITfInsertAtSelection, (void **)&pInsertAtSelection) != S_OK)
        return;

    // insert the text
    if (pInsertAtSelection->InsertTextAtSelection(ec, 0, pchText, cchText, &pRange) != S_OK)
        goto Exit;

    // update the selection, we'll make it an insertion point just past
    // the inserted text.
    pRange->Collapse(ec, TF_ANCHOR_END);

    tfSelection.range = pRange;
    tfSelection.style.ase = TF_AE_NONE;
    tfSelection.style.fInterimChar = FALSE;

    pContext->SetSelection(ec, 1, &tfSelection);

    pRange->Release();

Exit:
    pInsertAtSelection->Release();
}

//+---------------------------------------------------------------------------
//
// InsertHello
//
// Insert the string "Hello world!" to the focus context.
//----------------------------------------------------------------------------

void CTextService::InsertHello()
{
    ITfDocumentMgr *pDocMgrFocus;
    ITfContext *pContext;
    CInsertHelloEditSession *pInsertHelloEditSession;
    HRESULT hr;

    // get the focus document
    if (_pThreadMgr->GetFocus(&pDocMgrFocus) != S_OK)
        return;

    // we want the topmost context, since the main doc context could be
    // superceded by a modal tip context
    if (pDocMgrFocus->GetTop(&pContext) != S_OK)
    {
        pContext = NULL;
        goto Exit;
    }

    if (pInsertHelloEditSession = new CInsertHelloEditSession(pContext))
    {
        // we need a document write lock to insert text
        // the CInsertHelloEditSession will do all the work when the
        // CInsertHelloEditSession::DoEditSession method is called by the context
        pContext->RequestEditSession(_tfClientId, pInsertHelloEditSession, TF_ES_READWRITE | TF_ES_ASYNCDONTCARE, &hr);

        pInsertHelloEditSession->Release();
    }

Exit:
    if (pContext)
        pContext->Release();    

    pDocMgrFocus->Release();    
}
