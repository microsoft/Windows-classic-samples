//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  TextService.cpp
//
//          IUnknown, ITfTextInputProcessor implementation.
//
//////////////////////////////////////////////////////////////////////

#include "globals.h"
#include "TextService.h"
#include "MemoryStream.h"
#include "PopupWindow.h"

//+---------------------------------------------------------------------------
//
// CreateInstance
//
//----------------------------------------------------------------------------

/* static */
HRESULT CPropertyMonitorTextService::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj)
{
    CPropertyMonitorTextService *pCase;
    HRESULT hr;

    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (NULL != pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    if ((pCase = new CPropertyMonitorTextService) == NULL)
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

CPropertyMonitorTextService::CPropertyMonitorTextService()
{
    DllAddRef();

    //
    // Initialize the thread manager pointer.
    //
    _pThreadMgr = NULL;

    //
    // Initialize the numbers for ThreadMgrEventSink.
    //
    _dwThreadMgrEventSinkCookie = TF_INVALID_COOKIE;

    //
    // Initialize the numbers for TextEditSink.
    //
    _pTextEditSinkContext = NULL;
    _dwTextEditSinkCookie = TF_INVALID_COOKIE;

    _pDisplayAttributeMgr = NULL;
    _pCategoryMgr = NULL;

    _dwThreadFocusCookie = TF_INVALID_COOKIE;
    _pPopupWindow = NULL;

    _cRef = 1;
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

CPropertyMonitorTextService::~CPropertyMonitorTextService()
{
    DllRelease();
}

//+---------------------------------------------------------------------------
//
// QueryInterface
//
//----------------------------------------------------------------------------

STDAPI CPropertyMonitorTextService::QueryInterface(REFIID riid, void **ppvObj)
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
    else if (IsEqualIID(riid, IID_ITfTextEditSink))
    {
        *ppvObj = (ITfTextEditSink *)this;
    }
    else if (IsEqualIID(riid, IID_ITfThreadFocusSink))
    {
        *ppvObj = (ITfThreadFocusSink *)this;
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

STDAPI_(ULONG) CPropertyMonitorTextService::AddRef()
{
    return ++_cRef;
}

//+---------------------------------------------------------------------------
//
// Release
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CPropertyMonitorTextService::Release()
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

STDAPI CPropertyMonitorTextService::Activate(ITfThreadMgr *pThreadMgr, TfClientId tfClientId)
{
    _pThreadMgr = pThreadMgr;
    _pThreadMgr->AddRef();
    _tfClientId = tfClientId;

    //
    // Initialize ThreadMgrEventSink.
    //
    if (!_InitThreadMgrEventSink())
        goto ExitError;

    // 
    //  If there is the focus document manager already,
    //  we advise the TextEditSink.
    // 
    ITfDocumentMgr *pDocMgrFocus;
    if ((_pThreadMgr->GetFocus(&pDocMgrFocus) == S_OK) &&
        (pDocMgrFocus != NULL))
    {
        _InitTextEditSink(pDocMgrFocus);
        pDocMgrFocus->Release();
    }

    //
    // Initialize Language Bar.
    //
    if (!_InitLanguageBar())
        goto ExitError;

    //
    // Initialize Thread focus sink.
    //
    if (!_InitThreadFocusSink())
        goto ExitError;

    if (CoCreateInstance(CLSID_TF_DisplayAttributeMgr,
                         NULL,
                         CLSCTX_INPROC_SERVER,
                         IID_ITfDisplayAttributeMgr,
                         (void**)&_pDisplayAttributeMgr) != S_OK)
    {
        goto ExitError;
    }

    if (CoCreateInstance(CLSID_TF_CategoryMgr,
                         NULL,
                         CLSCTX_INPROC_SERVER,
                         IID_ITfCategoryMgr,
                         (void**)&_pCategoryMgr) != S_OK)
    {
        goto ExitError;
    }

    _pMemStream = CreateMemoryStream();
    if (_pMemStream == NULL)
    {
        goto ExitError;
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

STDAPI CPropertyMonitorTextService::Deactivate()
{
    //
    // Unadvise TextEditSink if it is advised.
    //
    _InitTextEditSink(NULL);

    //
    // Uninitialize ThreadMgrEventSink.
    //
    _UninitThreadMgrEventSink();

    //
    // Uninitialize Language Bar.
    //
    _UninitLanguageBar();

    //
    // Uninitialize thread focus sink.
    //
    _UninitThreadFocusSink();

    if (_pPopupWindow != NULL)
    {
        delete _pPopupWindow;
        _pPopupWindow = NULL;
    }

    if (_pMemStream != NULL)
    {
        _pMemStream->Release();
        _pMemStream = NULL;
    }

    if (_pDisplayAttributeMgr != NULL)
    {
        _pDisplayAttributeMgr->Release();
        _pDisplayAttributeMgr = NULL;
    }

    if (_pCategoryMgr != NULL)
    {
        _pCategoryMgr->Release();
        _pCategoryMgr = NULL;
    }

    // we MUST release all refs to _pThreadMgr in Deactivate
    if (_pThreadMgr != NULL)
    {
        _pThreadMgr->Release();
        _pThreadMgr = NULL;
    }

    _tfClientId = TF_CLIENTID_NULL;

    return S_OK;
}
