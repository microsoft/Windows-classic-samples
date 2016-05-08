//
// cleanup.cpp
//
// ITfCleanupContextDurationSink, ITfCleanupContextSink implementation.
//

#include "globals.h"
#include "mark.h"

//+---------------------------------------------------------------------------
//
// ITfCleanupContextDurationSink::OnStartCleanupContext
//
// TSF calls this method before it starts making OnCleanupContext callbacks.
// When this happens, we know we're just about to be shut down, and this will
// be our last chance to modify text in the document.  So we'll set a flag so
// we know not to initialize any contexts that might get created during the
// shutdown.
//----------------------------------------------------------------------------

STDAPI CMarkTextService::OnStartCleanupContext()
{
    _fCleaningUp = TRUE;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfCleanupContextDurationSink::OnEndCleanupContext
//
// Clear the cleanup flag.
//----------------------------------------------------------------------------

STDAPI CMarkTextService::OnEndCleanupContext()
{
    _fCleaningUp = FALSE;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfCleanupContextSink::OnCleanupContext
//
// The system calls this method if we're about to be deactivated.  It's our
// last chance to modify text in the context.  This is the time to abort any
// ongoing compositions.
//
// You won't see this callback shutting down the app if the app destroys all
// contexts before deactivating TSF.  Another way to get here is by installing
// a non-english keyboard layout or text service.  If you switch languages
// from english, the mark text service will be shut down before the context,
// and this method will be called.
//
// There's intentionally no way to get an edit cookie when a context is popped.
// In this case, it is assumed the context state is going to be thrown away.
// It's the owner's responsibility to terminate any compositions/text services
// before a Pop if they care about the contents afterwards.
//----------------------------------------------------------------------------

STDAPI CMarkTextService::OnCleanupContext(TfEditCookie ecWrite, ITfContext *pContext)
{
    // this sample has nothing to do here...a real text service wouldn't bother
    // to advise the sink in the first place if it took no action
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _InitCleanupContextSink
//
//----------------------------------------------------------------------------

BOOL CMarkTextService::_InitCleanupContextDurationSink()
{
    return AdviseSingleSink(_tfClientId, _pThreadMgr, (ITfCleanupContextDurationSink *)this,
                            IID_ITfCleanupContextDurationSink);
}

//+---------------------------------------------------------------------------
//
// _UninitCleanupContextSink
//
//----------------------------------------------------------------------------

void CMarkTextService::_UninitCleanupContextDurationSink()
{
    UnadviseSingleSink(_tfClientId, _pThreadMgr, IID_ITfCleanupContextDurationSink);
}

//+---------------------------------------------------------------------------
//
// _InitCleanupContextSink
//
//----------------------------------------------------------------------------

BOOL CMarkTextService::_InitCleanupContextSink(ITfContext *pContext)
{
    return AdviseSingleSink(_tfClientId, pContext, (ITfCleanupContextSink *)this,
                            IID_ITfCleanupContextSink);
}

//+---------------------------------------------------------------------------
//
// _UninitCleanupContextSink
//
//----------------------------------------------------------------------------

void CMarkTextService::_UninitCleanupContextSink(ITfContext *pContext)
{
    UnadviseSingleSink(_tfClientId, pContext, IID_ITfCleanupContextSink);
}
