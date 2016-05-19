//
// mark.cpp
//
// IUnknown, ITfTextInputProcessor implementation.
//

#include "globals.h"
#include "mark.h"

//+---------------------------------------------------------------------------
//
// CreateInstance
//
//----------------------------------------------------------------------------

/* static */
HRESULT CMarkTextService::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj)
{
    CMarkTextService *pMark;
    HRESULT hr;

    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (NULL != pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    if ((pMark = new CMarkTextService) == NULL)
        return E_OUTOFMEMORY;

    hr = pMark->QueryInterface(riid, ppvObj);

    pMark->Release(); // caller still holds ref if hr == S_OK

    return hr;
}

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

CMarkTextService::CMarkTextService()
{
    DllAddRef();

    _pThreadMgr = NULL;
    _tfClientId = TF_CLIENTID_NULL;

    _pComposition = NULL;

    _fCleaningUp = FALSE;

    _gaDisplayAttribute = TF_INVALID_GUIDATOM;

    _pLangBarItem = NULL;

    _dwThreadMgrEventSinkCookie = TF_INVALID_COOKIE;
    _dwThreadFocusSinkCookie = TF_INVALID_COOKIE;
    _dwTextEditSinkCookie = TF_INVALID_COOKIE;
    _dwGlobalCompartmentEventSinkCookie = TF_INVALID_COOKIE;

    _pTextEditSinkContext = NULL;

    _hWorkerWnd = NULL;

    _cRef = 1;
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

CMarkTextService::~CMarkTextService()
{
    DllRelease();
}

//+---------------------------------------------------------------------------
//
// QueryInterface
//
//----------------------------------------------------------------------------

STDAPI CMarkTextService::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfTextInputProcessor))
    {
        *ppvObj = (ITfTextInputProcessor *)this;
    }
    else if (IsEqualIID(riid, IID_ITfDisplayAttributeProvider))
    {
        *ppvObj = (ITfDisplayAttributeProvider *)this;
    }
    else if (IsEqualIID(riid, IID_ITfCreatePropertyStore))
    {
        *ppvObj = (ITfCreatePropertyStore *)this;
    }
    else if (IsEqualIID(riid, IID_ITfThreadMgrEventSink))
    {
        *ppvObj = (ITfThreadMgrEventSink *)this;
    }
    else if (IsEqualIID(riid, IID_ITfTextEditSink))
    {
        *ppvObj = (ITfTextEditSink *)this;
    }
    else if (IsEqualIID(riid, IID_ITfCleanupContextSink))
    {
        *ppvObj = (ITfCleanupContextSink *)this;
    }
    else if (IsEqualIID(riid, IID_ITfCleanupContextDurationSink))
    {
        *ppvObj = (ITfCleanupContextDurationSink *)this;
    }
    else if (IsEqualIID(riid, IID_ITfCompartmentEventSink))
    {
        *ppvObj = (ITfCompartmentEventSink *)this;
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

STDAPI_(ULONG) CMarkTextService::AddRef()
{
    return ++_cRef;
}

//+---------------------------------------------------------------------------
//
// Release
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CMarkTextService::Release()
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

STDAPI CMarkTextService::Activate(ITfThreadMgr *pThreadMgr, TfClientId tfClientId)
{
    _pThreadMgr = pThreadMgr;
    _pThreadMgr->AddRef();

    _tfClientId = tfClientId;

    if (!_InitLanguageBar())
        goto ExitError;

    if (!_InitThreadMgrSink())
        goto ExitError;

    if (!_InitDisplayAttributeGuidAtom())
        goto ExitError;

    if (!_InitCleanupContextDurationSink())
        goto ExitError;

    if (!_InitGlobalCompartment())
        goto ExitError;

    if (!_InitWorkerWnd())
        goto ExitError;

    if (!_InitKeystrokeSink())
        goto ExitError;

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

STDAPI CMarkTextService::Deactivate()
{
    _UninitThreadMgrSink();
    _UninitLanguageBar();
    _UninitCleanupContextDurationSink();
    _UninitGlobalCompartment();
    _UninitWorkerWnd();
    _UninitKeystrokeSink();
    _InitTextEditSink(NULL);

    // we MUST release all refs to _pThreadMgr in Deactivate
    SafeReleaseClear(_pThreadMgr);

    _tfClientId = TF_CLIENTID_NULL;

    return S_OK;
}
