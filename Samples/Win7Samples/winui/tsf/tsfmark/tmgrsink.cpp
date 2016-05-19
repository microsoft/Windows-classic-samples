//
// tmgrsink.cpp
//
// ITfThreadMgrEventSink implementation.
//

#include "globals.h"
#include "mark.h"

//+---------------------------------------------------------------------------
//
// OnInitDocumentMgr
//
// Sink called by the framework just before the first context is pushed onto
// a document.
//----------------------------------------------------------------------------

STDAPI CMarkTextService::OnInitDocumentMgr(ITfDocumentMgr *pDocMgr)
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

STDAPI CMarkTextService::OnUninitDocumentMgr(ITfDocumentMgr *pDocMgr)
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

STDAPI CMarkTextService::OnSetFocus(ITfDocumentMgr *pDocMgrFocus, ITfDocumentMgr *pDocMgrPrevFocus)
{
    // we'll track edit changes in the focus document, the only place we start compositions
    _InitTextEditSink(pDocMgrFocus);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnPushContext
//
// Sink called by the framework when a context is pushed.
//----------------------------------------------------------------------------

STDAPI CMarkTextService::OnPushContext(ITfContext *pContext)
{
    // ignore new contexts that appear while were uninitializing
    if (!_fCleaningUp)
    {
        _InitCleanupContextSink(pContext);
        _InitContextCompartment(pContext);
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnPopContext
//
// Sink called by the framework when a context is popped.
//----------------------------------------------------------------------------

STDAPI CMarkTextService::OnPopContext(ITfContext *pContext)
{
    _UninitCleanupContextSink(pContext);
    _UninitCompartment(pContext);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _InitThreadMgrSink
//
// Advise our sink.
//----------------------------------------------------------------------------

BOOL CMarkTextService::_InitThreadMgrSink()
{
    return AdviseSink(_pThreadMgr, (ITfThreadMgrEventSink *)this,
                      IID_ITfThreadMgrEventSink, &_dwThreadMgrEventSinkCookie);
}

//+---------------------------------------------------------------------------
//
// _UninitThreadMgrSink
//
// Unadvise our sink.
//----------------------------------------------------------------------------

void CMarkTextService::_UninitThreadMgrSink()
{
    UnadviseSink(_pThreadMgr, &_dwThreadMgrEventSinkCookie);
}
