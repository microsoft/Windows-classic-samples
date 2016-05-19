//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  CandidateList.cpp
//
//          CCandidateList class
//
//////////////////////////////////////////////////////////////////////

#include "Globals.h"
#include "TextService.h"
#include "EditSession.h"
#include "CandidateWindow.h"
#include "CandidateList.h"

//+---------------------------------------------------------------------------
//
// CGetTextExtentEditSession
//
//----------------------------------------------------------------------------

class CGetTextExtentEditSession : public CEditSessionBase
{
public:
    CGetTextExtentEditSession(CTextService *pTextService, ITfContext *pContext, ITfContextView *pContextView, ITfRange *pRangeComposition, CCandidateWindow *pCandidateWindow) : CEditSessionBase(pTextService, pContext)
    {
        _pContextView = pContextView;
        _pRangeComposition = pRangeComposition;
        _pCandidateWindow = pCandidateWindow;
    }

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec);

private:
    ITfContextView *_pContextView;
    ITfRange *_pRangeComposition;
    CCandidateWindow *_pCandidateWindow;
};

//+---------------------------------------------------------------------------
//
// DoEditSession
//
//----------------------------------------------------------------------------

STDAPI CGetTextExtentEditSession::DoEditSession(TfEditCookie ec)
{
    RECT rc;
    BOOL fClipped;

    if (SUCCEEDED(_pContextView->GetTextExt(ec, _pRangeComposition, &rc, &fClipped)))
        _pCandidateWindow->_Move(rc.left, rc.bottom);
    return S_OK;
}


//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

CCandidateList::CCandidateList(CTextService *pTextService)
{
    _pTextService = pTextService;

    _hwndParent = NULL;
    _pCandidateWindow = NULL;
    _pRangeComposition = NULL;
    _pContextCandidateWindow = NULL;
    _pContextDocument = NULL;
    _pDocumentMgr = NULL;

    _dwCookieContextKeyEventSink = TF_INVALID_COOKIE;
    _dwCookieTextLayoutSink = TF_INVALID_COOKIE;

    _cRef = 1;

    DllAddRef();
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

CCandidateList::~CCandidateList()
{
    _EndCandidateList();
    DllRelease();
}

//+---------------------------------------------------------------------------
//
// QueryInterface
//
//----------------------------------------------------------------------------

STDAPI CCandidateList::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfContextKeyEventSink))
    {
        *ppvObj = (ITfContextKeyEventSink *)this;
    }
    else if (IsEqualIID(riid, IID_ITfTextLayoutSink))
    {
        *ppvObj = (ITfTextLayoutSink *)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}


//+---------------------------------------------------------------------------
//
// AddRef
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CCandidateList::AddRef()
{
    return ++_cRef;
}

//+---------------------------------------------------------------------------
//
// Release
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CCandidateList::Release()
{
    LONG cr = --_cRef;

    assert(_cRef >= 0);

    if (_cRef == 0)
    {
        delete this;
    }

    return cr;
}

//+---------------------------------------------------------------------------
//
// KeyDown
//
//----------------------------------------------------------------------------

STDAPI CCandidateList::OnKeyDown(WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    if (pfEaten == NULL)
        return E_INVALIDARG;

    *pfEaten = TRUE;
    _pCandidateWindow->_OnKeyDown((UINT)wParam);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnKeyUp
//
//----------------------------------------------------------------------------

STDAPI CCandidateList::OnKeyUp(WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    if (pfEaten == NULL)
        return E_INVALIDARG;

    *pfEaten = TRUE;

    // 
    // we eat VK_RETURN here to finish candidate list.
    // 
    if (wParam == VK_RETURN)
        _EndCandidateList();
    else
        _pCandidateWindow->_OnKeyUp((UINT)wParam);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// KeyTestDown
//
//----------------------------------------------------------------------------

STDAPI CCandidateList::OnTestKeyDown(WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    if (pfEaten == NULL)
        return E_INVALIDARG;

    *pfEaten = TRUE;

    return S_OK;
}


//+---------------------------------------------------------------------------
//
// OnTestKeyUp
//
//----------------------------------------------------------------------------

STDAPI CCandidateList::OnTestKeyUp(WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    if (pfEaten == NULL)
        return E_INVALIDARG;

    *pfEaten = TRUE;

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnLayoutChange
//
//----------------------------------------------------------------------------

STDAPI CCandidateList::OnLayoutChange(ITfContext *pContext, TfLayoutCode lcode, ITfContextView *pContextView)
{
    //
    // we're interested in only document context.
    //
    if (pContext != _pContextDocument)
        return S_OK;

    switch (lcode)
    {
        case TF_LC_CHANGE:
            if (_pCandidateWindow != NULL)
            {
                CGetTextExtentEditSession *pEditSession;

                if ((pEditSession = new CGetTextExtentEditSession(_pTextService, pContext, pContextView, _pRangeComposition, _pCandidateWindow)) != NULL)
                {
                    HRESULT hr;
                    // we need a lock to do our work
                    // nb: this method is one of the few places where it is legal to use
                    // the TF_ES_SYNC flag
                    pContext->RequestEditSession(_pTextService->_GetClientId(), pEditSession, TF_ES_SYNC | TF_ES_READ, &hr);

                    pEditSession->Release();
                 }
            }
            break;

        case TF_LC_DESTROY:
            _EndCandidateList();
            break;

    }
    return S_OK;
}


//+---------------------------------------------------------------------------
//
// _ShowCandidateList
//
//----------------------------------------------------------------------------

HRESULT CCandidateList::_StartCandidateList(TfClientId tfClientId, ITfDocumentMgr *pDocumentMgr, ITfContext *pContextDocument, TfEditCookie ec, ITfRange *pRangeComposition)
{
    TfEditCookie ecTmp;
    HRESULT hr = E_FAIL;
    BOOL fClipped;

    //
    // clear the previous candidate list.
    // we support only one candidate window.
    //
    _EndCandidateList();

    //
    // create a new context on the document manager object for
    // the candidate ui.
    //
    if (FAILED(pDocumentMgr->CreateContext(tfClientId, 0, NULL, &_pContextCandidateWindow, &ecTmp)))
        return E_FAIL;

    //
    // push the new context. 
    //
    if (FAILED(pDocumentMgr->Push(_pContextCandidateWindow)))
        goto Exit;

    _pDocumentMgr = pDocumentMgr;
    _pDocumentMgr->AddRef();

    _pContextDocument = pContextDocument;
    _pContextDocument->AddRef();

    _pRangeComposition = pRangeComposition;
    _pRangeComposition->AddRef();

    // 
    // advise ITfContextKeyEventSink to the new context.
    // 
    if (FAILED(_AdviseContextKeyEventSink()))
        goto Exit;

    // 
    // advise ITfTextLayoutSink to the document context.
    // 
    if (FAILED(_AdviseTextLayoutSink()))
        goto Exit;

    // 
    // create an instance of CCandidateWindow class.
    //
    if (_pCandidateWindow = new CCandidateWindow())
    {
        RECT rc;
        ITfContextView *pContextView;

        //
        // get an active view of the document context.
        //
        if (FAILED(pContextDocument->GetActiveView(&pContextView)))
            goto Exit;

        //
        // get text extent for the range of the composition.
        //
        if (FAILED(pContextView->GetTextExt(ec, pRangeComposition, &rc, &fClipped)))
            goto Exit;

        pContextView->Release();

        
        //
        // create the dummy candidate window
        //
        if (!_pCandidateWindow->_Create())
            goto Exit;

        _pCandidateWindow->_Move(rc.left, rc.bottom);
        _pCandidateWindow->_Show();

        hr = S_OK;
    }

Exit:
    if (FAILED(hr))
    {
        _EndCandidateList();
    }
    return hr;
}

//+---------------------------------------------------------------------------
//
// _EndCandidateList
//
//----------------------------------------------------------------------------

void CCandidateList::_EndCandidateList()
{
    if (_pCandidateWindow)
    {
        _pCandidateWindow->_Destroy();
        delete _pCandidateWindow;
        _pCandidateWindow = NULL;
    }

    if (_pRangeComposition)
    {
       _pRangeComposition->Release();
       _pRangeComposition = NULL;
    }

    if (_pContextCandidateWindow)
    {
       _UnadviseContextKeyEventSink();
       _pContextCandidateWindow->Release();
       _pContextCandidateWindow = NULL;
    }

    if (_pContextDocument)
    {
       _UnadviseTextLayoutSink();
       _pContextDocument->Release();
       _pContextDocument = NULL;
    }

    if (_pDocumentMgr)
    {
       _pDocumentMgr->Pop(0);
       _pDocumentMgr->Release();
       _pDocumentMgr = NULL;
    }
}

//+---------------------------------------------------------------------------
//
// _IsCandidateWindowShown
//
//----------------------------------------------------------------------------

BOOL CCandidateList::_IsContextCandidateWindow(ITfContext *pContext)
{
    return (_pContextCandidateWindow == pContext) ? TRUE : FALSE;
}

//+---------------------------------------------------------------------------
//
// AdviseContextKeyEventSink
//
//----------------------------------------------------------------------------

HRESULT CCandidateList::_AdviseContextKeyEventSink()
{
    HRESULT hr;
    ITfSource *pSource = NULL;

    hr = E_FAIL;

    if (FAILED(_pContextCandidateWindow->QueryInterface(IID_ITfSource, (void **)&pSource)))
        goto Exit;

    if (FAILED(pSource->AdviseSink(IID_ITfContextKeyEventSink, (ITfContextKeyEventSink *)this, &_dwCookieContextKeyEventSink)))
        goto Exit;

    hr = S_OK;

Exit:
    if (pSource != NULL)
        pSource->Release();
    return hr;
}

//+---------------------------------------------------------------------------
//
// UnadviseContextKeyEventSink
//
//----------------------------------------------------------------------------

HRESULT CCandidateList::_UnadviseContextKeyEventSink()
{
    HRESULT hr;
    ITfSource *pSource = NULL;

    hr = E_FAIL;

    if (_pContextCandidateWindow == NULL)
        goto Exit;

    if (FAILED(_pContextCandidateWindow->QueryInterface(IID_ITfSource, (void **)&pSource)))
        goto Exit;

    if (FAILED(pSource->UnadviseSink(_dwCookieContextKeyEventSink)))
        goto Exit;

    hr = S_OK;

Exit:
    if (pSource != NULL)
        pSource->Release();
    return hr;
}

//+---------------------------------------------------------------------------
//
// AdviseTextLayoutSink
//
//----------------------------------------------------------------------------

HRESULT CCandidateList::_AdviseTextLayoutSink()
{
    HRESULT hr;
    ITfSource *pSource = NULL;

    hr = E_FAIL;

    if (FAILED(_pContextDocument->QueryInterface(IID_ITfSource, (void **)&pSource)))
        goto Exit;

    if (FAILED(pSource->AdviseSink(IID_ITfTextLayoutSink, (ITfTextLayoutSink *)this, &_dwCookieTextLayoutSink)))
        goto Exit;

    hr = S_OK;

Exit:
    if (pSource != NULL)
        pSource->Release();
    return hr;
}

//+---------------------------------------------------------------------------
//
// UnadviseTextLayoutSink
//
//----------------------------------------------------------------------------

HRESULT CCandidateList::_UnadviseTextLayoutSink()
{
    HRESULT hr;
    ITfSource *pSource = NULL;

    hr = E_FAIL;

    if (_pContextDocument == NULL)
        goto Exit;

    if (FAILED(_pContextDocument->QueryInterface(IID_ITfSource, (void **)&pSource)))
        goto Exit;

    if (FAILED(pSource->UnadviseSink(_dwCookieTextLayoutSink)))
        goto Exit;

    hr = S_OK;

Exit:
    if (pSource != NULL)
        pSource->Release();
    return hr;
}
