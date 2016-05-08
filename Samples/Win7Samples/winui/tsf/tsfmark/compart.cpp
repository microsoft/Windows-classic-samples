//
// compart.cpp
//
// Compartment example.
//

#include "globals.h"
#include "mark.h"

//+---------------------------------------------------------------------------
//
// _InitCompartment
//
// Initialize a compartment on a particular context.
//
// The Mark sample doesn't really do any with its context compartment, this
// code is purely for demonstration purposes.
//----------------------------------------------------------------------------

BOOL CMarkTextService::_InitContextCompartment(ITfContext *pContext)
{
    ITfCompartmentMgr *pCompartmentMgr;
    ITfCompartment *pCompartment;
    VARIANT varValue;
    HRESULT hr;

    // we want the mgr associated with pContext
    if (pContext->QueryInterface(IID_ITfCompartmentMgr, (void **)&pCompartmentMgr) != S_OK)
        return FALSE;

    hr = E_FAIL;

    if (pCompartmentMgr->GetCompartment(c_guidMarkContextCompartment, &pCompartment) != S_OK)
        goto Exit;

    // if we don't initialize the value, it will be VT_EMPTY
    // but let's initialize it to 0

    // NB: to keep things simple, we use a VT_I4
    // but you could use VT_UNKNOWN and store a pointer to anything
    varValue.vt = VT_I4;
    varValue.lVal = 0; // arbitrary value

    hr = pCompartment->SetValue(_tfClientId, &varValue);

    pCompartment->Release();

Exit:
    pCompartmentMgr->Release();

    return (hr == S_OK);
}

//+---------------------------------------------------------------------------
//
// _UninitCompartment
//
// Uninitialize a compartment on a particular context.
//
// The Mark sample doesn't really do any with its context compartment, this
// code is purely for demonstration purposes.
//----------------------------------------------------------------------------

void CMarkTextService::_UninitCompartment(ITfContext *pContext)
{
    ITfCompartmentMgr *pCompartmentMgr;

    // we want the mgr associated with pContext
    if (pContext->QueryInterface(IID_ITfCompartmentMgr, (void **)&pCompartmentMgr) != S_OK)
        return;

    pCompartmentMgr->ClearCompartment(_tfClientId, c_guidMarkContextCompartment);

    pCompartmentMgr->Release();
}

//+---------------------------------------------------------------------------
//
// _Menu_OnSetGlobalCompartment
//
// Callback for the "Set Global Compartment" menu item.
// Set the value of our global compartment.  This will trigger a callback
// on our compartment change sinks in every thread/instance of this service.
//----------------------------------------------------------------------------

/* static */
void CMarkTextService::_Menu_OnSetGlobalCompartment(CMarkTextService *_this)
{
    ITfCompartmentMgr *pCompartmentMgr;
    ITfCompartment *pCompartment;
    VARIANT varValue;

    // we want the global mgr
    if (_this->_pThreadMgr->GetGlobalCompartment(&pCompartmentMgr) != S_OK)
        return;

    if (pCompartmentMgr->GetCompartment(c_guidMarkGlobalCompartment, &pCompartment) != S_OK)
    {
        pCompartment = NULL;
        goto Exit;
    }

    // let's toggle the value
    // notice that global compartments are persisted, unlike all others
    if (FAILED(pCompartment->GetValue(&varValue))) // will return S_FALSE if varValue.vt == VT_EMPTY
        goto Exit;

    if (varValue.vt == VT_EMPTY)
    {
        // if we get here, the compartment has never been initialized
        varValue.vt = VT_I4;
        varValue.lVal = 0;
    }

    // toggle value
    varValue.lVal = ~varValue.lVal;

    pCompartment->SetValue(_this->_tfClientId, &varValue);

Exit:
    SafeRelease(pCompartment);
    pCompartmentMgr->Release();
}

//+---------------------------------------------------------------------------
//
// _InitGlobalCompartment
//
// Ininit a change sink on our global compartment.  The system will call us
// back anytime the compartment is modified from any thread in the desktop.
//
// NB: ITfCompartmentEventSink's attached to thread local compartments will
// only get callbacks to changes that occur within a single thread.  Global
// compartments are different.
//----------------------------------------------------------------------------

BOOL CMarkTextService::_InitGlobalCompartment()
{
    ITfCompartmentMgr *pCompartmentMgr;
    ITfCompartment *pCompartment;
    BOOL fRet;

    // we want the global mgr
    if (_pThreadMgr->GetGlobalCompartment(&pCompartmentMgr) != S_OK)
        return FALSE;

    fRet = FALSE;

    if (pCompartmentMgr->GetCompartment(c_guidMarkGlobalCompartment, &pCompartment) != S_OK)
        goto Exit;

    fRet = AdviseSink(pCompartment, (ITfCompartmentEventSink *)this,
                      IID_ITfCompartmentEventSink, &_dwGlobalCompartmentEventSinkCookie);

    pCompartment->Release();

    if (!fRet)
    {
        // don't try to unadvise a bogus cookie later
        _dwGlobalCompartmentEventSinkCookie = TF_INVALID_COOKIE;
    }

Exit:
    pCompartmentMgr->Release();
    return fRet;
}

//+---------------------------------------------------------------------------
//
// _UninitCompartment
//
// Unitialize the global compartment if we have previously accessed it.
// This method only frees resources the system has allocated in this thread.
// Other threads can still access the global compartment, and the value (which
// is persisted across the desktop) does not change.
//
// Also, uninit the change sink we attached to the compartment.
//----------------------------------------------------------------------------

void CMarkTextService::_UninitGlobalCompartment()
{
    ITfCompartmentMgr *pCompartmentMgr;
    ITfCompartment *pCompartment;

    // we want the global mgr
    if (_pThreadMgr->GetGlobalCompartment(&pCompartmentMgr) != S_OK)
        return;

    // unadvise our event sink
    if (pCompartmentMgr->GetCompartment(c_guidMarkGlobalCompartment, &pCompartment) == S_OK)
    {
        UnadviseSink(pCompartment, &_dwGlobalCompartmentEventSinkCookie);
        pCompartment->Release();
    }

    // let the system free resources associated with the compartment on this
    // thread
    pCompartmentMgr->ClearCompartment(_tfClientId, c_guidMarkGlobalCompartment);

    pCompartmentMgr->Release();
}

//+---------------------------------------------------------------------------
//
// ITfCompartmentEventSink::OnChange
//
// TSF calls this method anytime our private global compartment is modified,
// even from other threads/processes.
//----------------------------------------------------------------------------

STDAPI CMarkTextService::OnChange(REFGUID rguidCompartment)
{
    // nothing to do in this sample
    return S_OK;
}
