//
// flipsel.cpp
//
// "Flip Selection" menu item handler.
//

#include "globals.h"
#include "case.h"
#include "editsess.h"

class CFlipEditSession : public CEditSessionBase
{
public:
    CFlipEditSession(ITfContext *pContext) : CEditSessionBase(pContext) {}

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec);
};

//+---------------------------------------------------------------------------
//
// _Menu_FlipSel
//
// Toggle the case of the selected text in the focus context.
//----------------------------------------------------------------------------

void CCaseTextService::_Menu_FlipSel(CCaseTextService *_this)
{
    ITfDocumentMgr *pFocusDoc;
    ITfContext *pContext;
    CFlipEditSession *pFlipEditSession;
    HRESULT hr;

    // get the focus document
    if (_this->_pThreadMgr->GetFocus(&pFocusDoc) != S_OK)
        return;

    // we want the topmost context, since the main doc context could be
    // superceded by a modal tip context
    if (pFocusDoc->GetTop(&pContext) != S_OK)
    {
        pContext = NULL;
        goto Exit;
    }

    if (pFlipEditSession = new CFlipEditSession(pContext))
    {
        // we need a document write lock to insert text
        // the CHelloEditSession will do all the work when the
        // CFlipEditSession::DoEditSession method is called by the context
        pContext->RequestEditSession(_this->_tfClientId, pFlipEditSession, TF_ES_READWRITE | TF_ES_ASYNCDONTCARE, &hr);

        pFlipEditSession->Release();
    }

Exit:
    SafeRelease(pContext);
    pFocusDoc->Release();    
}

//+---------------------------------------------------------------------------
//
// DoEditSession
//
//----------------------------------------------------------------------------

STDAPI CFlipEditSession::DoEditSession(TfEditCookie ec)
{
    TF_SELECTION tfSelection;
    ULONG cFetched;

    // get the selection
    if (_pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK ||
        cFetched == 0)
    {
        // no selection
        return S_OK;
    }

    // do the work
    ToggleCase(ec, tfSelection.range, FALSE);

    // release the range
    tfSelection.range->Release();

    return S_OK;
}
