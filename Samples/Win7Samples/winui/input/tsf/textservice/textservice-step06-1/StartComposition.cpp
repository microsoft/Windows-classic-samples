//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  StartComposition.cpp
//
//          the rountins to start a new composition object.
//
//////////////////////////////////////////////////////////////////////

#include "Globals.h"
#include "EditSession.h"
#include "TextService.h"

//+---------------------------------------------------------------------------
//
// CStartCompositinoEditSession
//
//----------------------------------------------------------------------------

class CStartCompositionEditSession : public CEditSessionBase
{
public:
    CStartCompositionEditSession(CTextService *pTextService, ITfContext *pContext) : CEditSessionBase(pTextService, pContext)
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

STDAPI CStartCompositionEditSession::DoEditSession(TfEditCookie ec)
{
    ITfInsertAtSelection *pInsertAtSelection = NULL;
    ITfRange *pRangeInsert = NULL;
    ITfContextComposition *pContextComposition = NULL;
    ITfComposition *pComposition = NULL;
    HRESULT hr = E_FAIL;

    // we need a special interface to insert text at the selection
    if (_pContext->QueryInterface(IID_ITfInsertAtSelection, (void **)&pInsertAtSelection) != S_OK)
    {
        goto Exit;
    }

    // insert the text
    if (pInsertAtSelection->InsertTextAtSelection(ec, TF_IAS_QUERYONLY, NULL, 0, &pRangeInsert) != S_OK)
    {
        goto Exit;
    }

    // get an interface on the context we can use to deal with compositions
    if (_pContext->QueryInterface(IID_ITfContextComposition, (void **)&pContextComposition) != S_OK)
    {
        goto Exit;
    }

    // start the new composition
    if ((pContextComposition->StartComposition(ec, pRangeInsert, _pTextService, &pComposition) == S_OK) && (pComposition != NULL))
    {
        // Store the pointer of this new composition object in the instance 
        // of the CTextService class. So this instance of the CTextService 
        // class can know now it is in the composition stage.
        _pTextService->_SetComposition(pComposition);

        // 
        //  set selection to the adjusted range
        // 
        TF_SELECTION tfSelection;
        tfSelection.range = pRangeInsert;
        tfSelection.style.ase = TF_AE_NONE;
        tfSelection.style.fInterimChar = FALSE;
        _pContext->SetSelection(ec, 1, &tfSelection);
    }

Exit:
    if (pContextComposition != NULL)
        pContextComposition->Release();

    if (pRangeInsert != NULL)
        pRangeInsert->Release();

    if (pInsertAtSelection != NULL)
        pInsertAtSelection->Release();

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _StartComposition
//
// this starts the new composition at the selection of the current 
// focus context.
//----------------------------------------------------------------------------

void CTextService::_StartComposition(ITfContext *pContext)
{
    CStartCompositionEditSession *pStartCompositionEditSession;

    if (pStartCompositionEditSession = new CStartCompositionEditSession(this, pContext))
    {
        HRESULT hr;
        // we need a synchronus document write lock.
        // the CStartCompositionEditSession will do all the work when the
        // CStartCompositionEditSession::DoEditSession method is called by the context
        pContext->RequestEditSession(_tfClientId, pStartCompositionEditSession, TF_ES_SYNC | TF_ES_READWRITE, &hr);

        pStartCompositionEditSession->Release();
    }
}
