//
// compose.cpp
//
// Composition code.
//

#include "globals.h"
#include "mark.h"
#include "editsess.h"

class CCompositionEditSession : public CEditSessionBase
{
public:
    CCompositionEditSession(ITfContext *pContext, CMarkTextService *pMark) : CEditSessionBase(pContext)
    {
        _pMark = pMark;
        _pMark->AddRef();
    }
    ~CCompositionEditSession()
    {
        _pMark->Release();
    }

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec);

private:
    CMarkTextService *_pMark;
};

class CTerminateCompositionEditSession : public CEditSessionBase
{
public:
    CTerminateCompositionEditSession(CMarkTextService *pMark, ITfContext *pContext) : CEditSessionBase(pContext)
    {
        _pMark = pMark;
        _pMark->AddRef();
    }
    ~CTerminateCompositionEditSession()
    {
        _pMark->Release();
    }

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec)
    {
        _pMark->_TerminateComposition(ec);
        return S_OK;
    }

private:
    CMarkTextService *_pMark;
};

//+---------------------------------------------------------------------------
//
// _TerminateCompositionInContext
//
//----------------------------------------------------------------------------

void CMarkTextService::_TerminateCompositionInContext(ITfContext *pContext)
{
    CTerminateCompositionEditSession *pEditSession;
    HRESULT hr;

    if (pEditSession = new CTerminateCompositionEditSession(this, pContext))
    {
        pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
        pEditSession->Release();
    }
}

//+---------------------------------------------------------------------------
//
// _Menu_OnComposition
//
// Callback for the "Start/End Composition" menu item.
// If we have a composition, end it.  Otherwise start a new composition over
// the selection of the current focus context.
//----------------------------------------------------------------------------

/* static */
void CMarkTextService::_Menu_OnComposition(CMarkTextService *_this)
{
    ITfDocumentMgr *pFocusDoc;
    ITfContext *pContext;
    CCompositionEditSession *pCompositionEditSession;
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

    if (pCompositionEditSession = new CCompositionEditSession(pContext, _this))
    {
        // we need a document write lock
        // the CCompositionEditSession will do all the work when the
        // CCompositionEditSession::DoEditSession method is called by the context
        pContext->RequestEditSession(_this->_tfClientId, pCompositionEditSession, TF_ES_READWRITE | TF_ES_ASYNCDONTCARE, &hr);

        pCompositionEditSession->Release();
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

STDAPI CCompositionEditSession::DoEditSession(TfEditCookie ec)
{
    ITfInsertAtSelection *pInsertAtSelection;
    ITfContextComposition *pContextComposition;
    ITfComposition *pComposition;
    ITfRange *pRangeComposition;
    ITfRange *pRangeInsert;
    ITfContext *pCompositionContext;
    HRESULT hr;
    BOOL fEqualContexts;

    // get an interface on the context we can use to deal with compositions
    if (_pContext->QueryInterface(IID_ITfContextComposition, (void **)&pContextComposition) != S_OK)
        return E_FAIL;

    hr = E_FAIL;

    pInsertAtSelection = NULL;

    if (_pMark->_IsComposing())
    {
        // we have a composition, let's terminate it
        
        // it's possible our current composition is in another context...let's find out
        fEqualContexts = TRUE;
        if (_pMark->_GetComposition()->GetRange(&pRangeComposition) == S_OK)
        {
            if (pRangeComposition->GetContext(&pCompositionContext) == S_OK)
            {
                fEqualContexts = IsEqualUnknown(pCompositionContext, _pContext);
                if (!fEqualContexts)
                {
                    // need an edit session in the composition context
                    _pMark->_TerminateCompositionInContext(pCompositionContext);
                }
                pCompositionContext->Release();
            }
            pRangeComposition->Release();
        }

        // if the composition is in pContext, we already have an edit cookie
        if (fEqualContexts)
        {
            _pMark->_TerminateComposition(ec);
        }
    }
    else
    {
        // let's start a new composition over the current selection
        // this is totally contrived, a real text service would have
        // some meaningful logic to trigger this

        // first, test where a keystroke would go in the document if we did an insert
        // we need a special interface to insert text at the selection
        if (_pContext->QueryInterface(IID_ITfInsertAtSelection, (void **)&pInsertAtSelection) != S_OK)
        {
            pInsertAtSelection = NULL;
            goto Exit;
        }

        if (pInsertAtSelection->InsertTextAtSelection(ec, TF_IAS_QUERYONLY, NULL, 0, &pRangeInsert) != S_OK)
            goto Exit;

        // start the composition
        if (pContextComposition->StartComposition(ec, pRangeInsert, _pMark, &pComposition) != S_OK)
        {
            pComposition = NULL;
        }

        pRangeInsert->Release();

        // _pComposition may be NULL even if StartComposition return S_OK, this mean the application
        // rejected the composition

        if (pComposition != NULL)
        {
            _pMark->_SetComposition(pComposition);
            // underline the composition text to give the user some feedback UI
            _pMark->_SetCompositionDisplayAttributes(ec);
        }
    }

    // if we make it here, we've succeeded
    hr = S_OK;

Exit:
    SafeRelease(pInsertAtSelection);
    pContextComposition->Release();

    return hr;
}

//+---------------------------------------------------------------------------
//
// OnCompositionTerminated
//
// Callback for ITfCompositionSink.  The system calls this method whenever
// someone other than this service ends a composition.
//----------------------------------------------------------------------------

STDAPI CMarkTextService::OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition *pComposition)
{
    // we already have the composition cached, so we can ignore pComposition...

    // all this service wants to do is clear the display property
    _ClearCompositionDisplayAttributes(ecWrite);

    // releae our cached composition
    SafeReleaseClear(_pComposition);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _ClearCompositionDisplayAttributes
//
//----------------------------------------------------------------------------

void CMarkTextService::_ClearCompositionDisplayAttributes(TfEditCookie ec)
{
    ITfRange *pRangeComposition;
    ITfContext *pContext;
    ITfProperty *pDisplayAttributeProperty;

    // we need a range and the context it lives in
    if (_pComposition->GetRange(&pRangeComposition) != S_OK)
        return;

    if (pRangeComposition->GetContext(&pContext) != S_OK)
    {
        pContext = NULL;
        goto Exit;
    }

    // get our the display attribute property
    if (pContext->GetProperty(GUID_PROP_ATTRIBUTE, &pDisplayAttributeProperty) != S_OK)
        goto Exit;

    // clear the value over the range
    pDisplayAttributeProperty->Clear(ec, pRangeComposition);

    pDisplayAttributeProperty->Release();

Exit:
    pRangeComposition->Release();
    SafeRelease(pContext);
}

//+---------------------------------------------------------------------------
//
// _SetCompositionDisplayAttributes
//
//----------------------------------------------------------------------------

BOOL CMarkTextService::_SetCompositionDisplayAttributes(TfEditCookie ec)
{
    ITfRange *pRangeComposition;
    ITfContext *pContext;
    ITfProperty *pDisplayAttributeProperty;
    VARIANT var;
    HRESULT hr;

    // we need a range and the context it lives in
    if (_pComposition->GetRange(&pRangeComposition) != S_OK)
        return FALSE;

    hr = E_FAIL;

    if (pRangeComposition->GetContext(&pContext) != S_OK)
    {
        pContext = NULL;
        goto Exit;
    }

    // get our the display attribute property
    if (pContext->GetProperty(GUID_PROP_ATTRIBUTE, &pDisplayAttributeProperty) != S_OK)
        goto Exit;

    // set the value over the range
    // the application will use this guid atom to lookup the acutal rendering information
    var.vt = VT_I4; // we're going to set a TfGuidAtom
    var.lVal = _gaDisplayAttribute; // our cached guid atom for c_guidMarkDisplayAttribute

    hr = pDisplayAttributeProperty->SetValue(ec, pRangeComposition, &var);

    pDisplayAttributeProperty->Release();

Exit:
    pRangeComposition->Release();
    SafeRelease(pContext);
    return (hr == S_OK);
}

//+---------------------------------------------------------------------------
//
// _InitDisplayAttributeGuidAtom
//
// Because it's expensive to map our display attribute GUID to a TSF
// TfGuidAtom, we do it once when Activate is called.
//----------------------------------------------------------------------------

BOOL CMarkTextService::_InitDisplayAttributeGuidAtom()
{
    ITfCategoryMgr *pCategoryMgr;
    HRESULT hr;

    if (CoCreateInstance(CLSID_TF_CategoryMgr,
                         NULL, 
                         CLSCTX_INPROC_SERVER, 
                         IID_ITfCategoryMgr, 
                         (void**)&pCategoryMgr) != S_OK)
    {
        return FALSE;
    }

    hr = pCategoryMgr->RegisterGUID(c_guidMarkDisplayAttribute, &_gaDisplayAttribute);

    pCategoryMgr->Release();
        
    return (hr == S_OK);
}
