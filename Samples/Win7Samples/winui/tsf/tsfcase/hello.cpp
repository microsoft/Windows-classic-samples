//
// hello.cpp
//
// "Hello World" menu item handler.
//

#include "globals.h"
#include "case.h"
#include "editsess.h"

class CHelloEditSession : public CEditSessionBase
{
public:
    CHelloEditSession(ITfContext *pContext) : CEditSessionBase(pContext) {}

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec);
};

//+---------------------------------------------------------------------------
//
// _Menu_HelloWord
//
// Insert the string "Hello world!" to the focus context.
//----------------------------------------------------------------------------

void CCaseTextService::_Menu_HelloWord(CCaseTextService *_this)
{
    ITfDocumentMgr *pFocusDoc;
    ITfContext *pContext;
    CHelloEditSession *pHelloEditSession;
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

    if (pHelloEditSession = new CHelloEditSession(pContext))
    {
        // we need a document write lock to insert text
        // the CHelloEditSession will do all the work when the
        // CHelloEditSession::DoEditSession method is called by the context
        pContext->RequestEditSession(_this->_tfClientId, pHelloEditSession, TF_ES_READWRITE | TF_ES_ASYNCDONTCARE, &hr);

        pHelloEditSession->Release();
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

STDAPI CHelloEditSession::DoEditSession(TfEditCookie ec)
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
