//
// editsink.cpp
//
// ITfTextEditSink implementation.
//

#include "globals.h"
#include "mark.h"

//+---------------------------------------------------------------------------
//
// OnEndEdit
//
// Called by the system whenever anyone releases a write-access document lock.
//----------------------------------------------------------------------------

STDAPI CMarkTextService::OnEndEdit(ITfContext *pContext, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord)
{
    ITfRange *pRangeComposition;
    IEnumTfRanges *pEnumRanges;
    ITfRange *pRange;
    ITfContext *pCompositionContext;
    TF_SELECTION tfSelection;
    BOOL fResult;
    BOOL fCancelComposition;
    ULONG cFetched;

    if (_pComposition == NULL)
        return S_OK;

    // are we responsible for the edit?
    if (pContext->InWriteSession(_tfClientId, &fResult) == S_OK && fResult)
        return S_OK;

    // is this the context our composition lives in?
    if (_pComposition->GetRange(&pRangeComposition) != S_OK)
        return S_OK;

    if (pRangeComposition->GetContext(&pCompositionContext) != S_OK)
        goto Exit;

    fResult = IsEqualUnknown(pCompositionContext, pContext);

    pCompositionContext->Release();

    if (!fResult)
        goto Exit; // different context

    fCancelComposition = FALSE;

    // we're composing in this context, cancel the composition if anything suspicious happened

    // did the selection move outside the composition?
    if (pEditRecord->GetSelectionStatus(&fResult) == S_OK && fResult)
    {
        if (pContext->GetSelection(ecReadOnly, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) == S_OK &&
            cFetched == 1)
        {
            if (_pComposition->GetRange(&pRangeComposition) == S_OK)
            {
                fResult = IsRangeCovered(ecReadOnly, tfSelection.range, pRangeComposition);

                pRangeComposition->Release();

                if (!fResult)
                {
                    fCancelComposition = TRUE;
                }
            }
            tfSelection.range->Release();
        }
    }

    if (fCancelComposition)
        goto CancelComposition;

    // did someone else edit the document text?
    if (pEditRecord->GetTextAndPropertyUpdates(TF_GTP_INCL_TEXT, NULL, 0, &pEnumRanges) == S_OK)
    {
        // is the enumerator empty?
        if (pEnumRanges->Next(1, &pRange, NULL) == S_OK)
        {
            pRange->Release();
            fCancelComposition = TRUE;
        }
        pEnumRanges->Release();
    }

    if (fCancelComposition)
    {
CancelComposition:
        // we need a write edit session to cancel the composition
        _TerminateCompositionInContext(pContext);
    }

Exit:
    pRangeComposition->Release();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _InitTextEditSink
//
// Init a text edit sink on the topmost context of the document.
// Always release any previous sink.
//----------------------------------------------------------------------------

BOOL CMarkTextService::_InitTextEditSink(ITfDocumentMgr *pDocMgr)
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
