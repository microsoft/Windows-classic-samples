//
// case.cpp
//
// IUnknown, ITfTextInputProcessor implementation.
//

#include "globals.h"
#include "case.h"

//+---------------------------------------------------------------------------
//
// CreateInstance
//
//----------------------------------------------------------------------------

/* static */
HRESULT CCaseTextService::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj)
{
    CCaseTextService *pCase;
    HRESULT hr;

    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (NULL != pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    if ((pCase = new CCaseTextService) == NULL)
        return E_OUTOFMEMORY;

    hr = pCase->QueryInterface(riid, ppvObj);

    pCase->Release(); // caller still holds ref if hr == S_OK

    return hr;
}

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

CCaseTextService::CCaseTextService()
{
    DllAddRef();

    _pThreadMgr = NULL;
    _tfClientId = TF_CLIENTID_NULL;

    _fShowSnoop = FALSE;
    _pSnoopWnd = NULL;

    _pLangBarItem = NULL;

    _fFlipKeys = FALSE;

    _dwThreadMgrEventSinkCookie = TF_INVALID_COOKIE;
    _dwThreadFocusSinkCookie = TF_INVALID_COOKIE;
    _dwTextEditSinkCookie = TF_INVALID_COOKIE;
    _pTextEditSinkContext = NULL;

    _cRef = 1;
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

CCaseTextService::~CCaseTextService()
{
    DllRelease();
}

//+---------------------------------------------------------------------------
//
// QueryInterface
//
//----------------------------------------------------------------------------

STDAPI CCaseTextService::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfTextInputProcessor))
    {
        *ppvObj = (ITfTextInputProcessor *)this;
    }
    else if (IsEqualIID(riid, IID_ITfThreadMgrEventSink))
    {
        *ppvObj = (ITfThreadMgrEventSink *)this;
    }
    else if (IsEqualIID(riid, IID_ITfThreadFocusSink))
    {
        *ppvObj = (ITfThreadFocusSink *)this;
    }
    else if (IsEqualIID(riid, IID_ITfTextEditSink))
    {
        *ppvObj = (ITfTextEditSink *)this;
    }
    else if (IsEqualIID(riid, IID_ITfKeyEventSink))
    {
        *ppvObj = (ITfKeyEventSink *)this;
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

STDAPI_(ULONG) CCaseTextService::AddRef()
{
    return ++_cRef;
}

//+---------------------------------------------------------------------------
//
// Release
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CCaseTextService::Release()
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
// Activate
//
//----------------------------------------------------------------------------

STDAPI CCaseTextService::Activate(ITfThreadMgr *pThreadMgr, TfClientId tfClientId)
{
    ITfDocumentMgr *pFocusDoc;

    _pThreadMgr = pThreadMgr;
    _pThreadMgr->AddRef();

    _tfClientId = tfClientId;

    if (!_InitLanguageBar())
        goto ExitError;

    if (!_InitThreadMgrSink())
        goto ExitError;

    if (!_InitSnoopWnd())
        goto ExitError;

    if (!_InitKeystrokeSink())
        goto ExitError;

    if (!_InitPreservedKey())
        goto ExitError;

    // start tracking the focus doc
    if (_pThreadMgr->GetFocus(&pFocusDoc) == S_OK)
    {
        // The system will call OnSetFocus only for focus events after Activate
        // is called.
        OnSetFocus(pFocusDoc, NULL);
        pFocusDoc->Release();
    }

    return S_OK;

ExitError:
    Deactivate(); // cleanup any half-finished init
    return E_FAIL;
}

//+---------------------------------------------------------------------------
//
// Deactivate
//
//----------------------------------------------------------------------------

STDAPI CCaseTextService::Deactivate()
{
    _UninitSnoopWnd();
    _UninitThreadMgrSink();
    _UninitLanguageBar();
    _UninitKeystrokeSink();
    _UninitPreservedKey();

    // we MUST release all refs to _pThreadMgr in Deactivate
    SafeReleaseClear(_pThreadMgr);

    _tfClientId = TF_CLIENTID_NULL;

    return S_OK;
}
