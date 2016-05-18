//
// flipdoc.cpp
//
// "Flip Doc" menu item handler.
//

#include "globals.h"
#include "case.h"
#include "editsess.h"

class CFlipDocEditSession : public CEditSessionBase
{
public:
    CFlipDocEditSession(ITfContext *pContext) : CEditSessionBase(pContext) {}

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec);
};

//+---------------------------------------------------------------------------
//
// _Menu_FlipDoc
//
// Toggle the case of the entire document.
//----------------------------------------------------------------------------

void CCaseTextService::_Menu_FlipDoc(CCaseTextService *_this)
{
    ITfDocumentMgr *pFocusDoc;
    ITfContext *pContext;
    CFlipDocEditSession *pFlipEditSession;
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

    if (pFlipEditSession = new CFlipDocEditSession(pContext))
    {
        // we need a document write lock to insert text
        // the CHelloEditSession will do all the work when the
        // CFlipDocEditSession::DoEditSession method is called by the context
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

STDAPI CFlipDocEditSession::DoEditSession(TfEditCookie ec)
{
    ITfRange *pRangeStart;

    // get the head of the doc
    if (_pContext->GetStart(ec, &pRangeStart) != S_OK)
        return E_FAIL;

    // do the work
    ToggleCase(ec, pRangeStart, TRUE);

    pRangeStart->Release();

    return S_OK;
}
