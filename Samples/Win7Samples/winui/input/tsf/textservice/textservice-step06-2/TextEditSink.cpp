//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  TextEditSink.cpp
//
//          ITfTextEditSink implementation.
//
//////////////////////////////////////////////////////////////////////

#include "globals.h"
#include "TextService.h"

BOOL IsRangeCovered(TfEditCookie ec, ITfRange *pRangeTest, ITfRange *pRangeCover);

//+---------------------------------------------------------------------------
//
// OnEndEdit
//
// Called by the system whenever anyone releases a write-access document lock.
//----------------------------------------------------------------------------

STDAPI CTextService::OnEndEdit(ITfContext *pContext, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord)
{
    BOOL fSelectionChanged;
    IEnumTfRanges *pEnumTextChanges;
    ITfRange *pRange;

    //
    // did the selection change?
    // The selection change includes the movement of caret as well. 
    // The caret position is represent as the empty selection range when
    // there is no selection.
    //
    if (pEditRecord->GetSelectionStatus(&fSelectionChanged) == S_OK &&
        fSelectionChanged)
    {
        // If the selection is moved to out side of the current composition,
        // we terminate the composition. This TextService supports only one
        // composition in one context object.
        if (_IsComposing())
        {
            TF_SELECTION tfSelection;
            ULONG cFetched;

            if (pContext->GetSelection(ecReadOnly, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) == S_OK && cFetched == 1)
            {
                ITfRange *pRangeComposition;
                // is the insertion point covered by a composition?
                if (_pComposition->GetRange(&pRangeComposition) == S_OK)
                {
                    if (!IsRangeCovered(ecReadOnly, tfSelection.range, pRangeComposition))
                    {
                       _EndComposition(pContext);
                    }

                    pRangeComposition->Release();
                }
            }
        }
    }

    // text modification?
    if (pEditRecord->GetTextAndPropertyUpdates(TF_GTP_INCL_TEXT, NULL, 0, &pEnumTextChanges) == S_OK)
    {
        if (pEnumTextChanges->Next(1, &pRange, NULL) == S_OK)
        {
            //
            // pRange is the updated range.
            //

            pRange->Release();
        }

        pEnumTextChanges->Release();
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _InitTextEditSink
//
// Init a text edit sink on the topmost context of the document.
// Always release any previous sink.
//----------------------------------------------------------------------------

BOOL CTextService::_InitTextEditSink(ITfDocumentMgr *pDocMgr)
{
    ITfSource *pSource;
    BOOL fRet;

    // clear out any previous sink first

    if (_dwTextEditSinkCookie != TF_INVALID_COOKIE)
    {
        if (_pTextEditSinkContext->QueryInterface(IID_ITfSource, (void **)&pSource) == S_OK)
        {
            pSource->UnadviseSink(_dwTextEditSinkCookie);
            pSource->Release();
        }

        _pTextEditSinkContext->Release();
        _pTextEditSinkContext = NULL;
        _dwTextEditSinkCookie = TF_INVALID_COOKIE;
    }

    if (pDocMgr == NULL)
        return TRUE; // caller just wanted to clear the previous sink

    // setup a new sink advised to the topmost context of the document

    if (pDocMgr->GetTop(&_pTextEditSinkContext) != S_OK)
        return FALSE;

    if (_pTextEditSinkContext == NULL)
        return TRUE; // empty document, no sink possible

    fRet = FALSE;

    if (_pTextEditSinkContext->QueryInterface(IID_ITfSource, (void **)&pSource) == S_OK)
    {
        if (pSource->AdviseSink(IID_ITfTextEditSink, (ITfTextEditSink *)this, &_dwTextEditSinkCookie) == S_OK)
        {
            fRet = TRUE;
        }
        else
        {
            _dwTextEditSinkCookie = TF_INVALID_COOKIE;
        }
        pSource->Release();
    }

    if (fRet == FALSE)
    {
        _pTextEditSinkContext->Release();
        _pTextEditSinkContext = NULL;
    }

    return fRet;
}
