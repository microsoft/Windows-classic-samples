//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  ThreadMgrEventSink.cpp
//
//          ITfThreadMgrEventSink implementation.
//
//////////////////////////////////////////////////////////////////////

#include "Globals.h"
#include "TextService.h"

//+---------------------------------------------------------------------------
//
// OnInitDocumentMgr
//
// Sink called by the framework just before the first context is pushed onto
// a document.
//----------------------------------------------------------------------------

STDAPI CExtentMonitorTextService::OnInitDocumentMgr(ITfDocumentMgr *pDocMgr)
{
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnUninitDocumentMgr
//
// Sink called by the framework just after the last context is popped off a
// document.
//----------------------------------------------------------------------------

STDAPI CExtentMonitorTextService::OnUninitDocumentMgr(ITfDocumentMgr *pDocMgr)
{
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnSetFocus
//
// Sink called by the framework when focus changes from one document to
// another.  Either document may be NULL, meaning previously there was no
// focus document, or now no document holds the input focus.
//----------------------------------------------------------------------------

STDAPI CExtentMonitorTextService::OnSetFocus(ITfDocumentMgr *pDocMgrFocus, ITfDocumentMgr *pDocMgrPrevFocus)
{

    //
    // Whenever focus is changed, we initialize the TextEditSink.
    //
    ITfDocumentMgr *pDocMgrTarget = NULL;
    if (GetNonTransitoryDim(pDocMgrFocus, &pDocMgrTarget) != S_OK)
    {
        if (pDocMgrFocus)
        {
            pDocMgrTarget = pDocMgrFocus;
            pDocMgrTarget->AddRef();
        }
    }

    _InitTextEditSink(pDocMgrTarget);

    if (pDocMgrTarget)
    {
        ITfContext *pContext;
        if (SUCCEEDED(pDocMgrTarget->GetBase(&pContext)))
        {
            DumpExtent(pContext, DE_EVENTID_ONSETFOCUS);
            pContext->Release();
        }
        pDocMgrTarget->Release();
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnPushContext
//
// Sink called by the framework when a context is pushed.
//----------------------------------------------------------------------------

STDAPI CExtentMonitorTextService::OnPushContext(ITfContext *pContext)
{
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnPopContext
//
// Sink called by the framework when a context is popped.
//----------------------------------------------------------------------------

STDAPI CExtentMonitorTextService::OnPopContext(ITfContext *pContext)
{
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _InitThreadMgrEventSink
//
// Advise our sink.
//----------------------------------------------------------------------------

BOOL CExtentMonitorTextService::_InitThreadMgrEventSink()
{
    ITfSource *pSource;
    BOOL fRet;

    if (_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource) != S_OK)
        return FALSE;

    fRet = FALSE;

    if (pSource->AdviseSink(IID_ITfThreadMgrEventSink, (ITfThreadMgrEventSink *)this, &_dwThreadMgrEventSinkCookie) != S_OK)
    {
        // make sure we don't try to Unadvise _dwThreadMgrEventSinkCookie later
        _dwThreadMgrEventSinkCookie = TF_INVALID_COOKIE;
        goto Exit;
    }

    fRet = TRUE;

Exit:
    pSource->Release();
    return fRet;
}

//+---------------------------------------------------------------------------
//
// _UninitThreadMgrEventSink
//
// Unadvise our sink.
//----------------------------------------------------------------------------

void CExtentMonitorTextService::_UninitThreadMgrEventSink()
{
    ITfSource *pSource;

    if (_dwThreadMgrEventSinkCookie == TF_INVALID_COOKIE)
        return; // never Advised

    if (_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource) == S_OK)
    {
        pSource->UnadviseSink(_dwThreadMgrEventSinkCookie);
        pSource->Release();
    }

    _dwThreadMgrEventSinkCookie = TF_INVALID_COOKIE;
}
