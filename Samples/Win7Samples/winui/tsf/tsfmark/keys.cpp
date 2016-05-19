//
// keys.cpp
//
// ITfKeyEventSink implementation.
//

#include "globals.h"
#include "mark.h"
#include "editsess.h"

class CKeystrokeEditSession : public CEditSessionBase
{
public:
    CKeystrokeEditSession(CMarkTextService *pMark, ITfContext *pContext, WPARAM wParam) : CEditSessionBase(pContext)
    {
        _pMark = pMark;
        _pMark->AddRef();
        _wParam = wParam;
    }
    ~CKeystrokeEditSession()
    {
        _pMark->Release();
    }

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec);

private:
    CMarkTextService *_pMark;
    WPARAM _wParam;
};

//+---------------------------------------------------------------------------
//
// _HandleReturn
//
// Returns S_OK to eat the keystroke, S_FALSE otherwise.
//----------------------------------------------------------------------------

HRESULT CMarkTextService::_HandleReturn(TfEditCookie ec, ITfContext *pContext)
{
    // just terminate the composition
    _TerminateComposition(ec);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleArrowKey
//
// Update the selection within a composition.
// Returns S_OK to eat the keystroke, S_FALSE otherwise.
//----------------------------------------------------------------------------

HRESULT CMarkTextService::_HandleArrowKey(TfEditCookie ec, ITfContext *pContext, WPARAM wParam)
{
    ITfRange *pRangeComposition;
    LONG cch;
    BOOL fEqual;
    TF_SELECTION tfSelection;
    ULONG cFetched;

    // get the selection
    if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK ||
        cFetched != 1)
    {
        // no selection?
        return S_OK; // eat the keystroke
    }

    // get the composition range
    if (_pComposition->GetRange(&pRangeComposition) != S_OK)
        goto Exit;

    // adjust the selection, we won't do anything fancy

    if (wParam == VK_LEFT)
    {
        if (tfSelection.range->IsEqualStart(ec, pRangeComposition, TF_ANCHOR_START, &fEqual) == S_OK &&
            !fEqual)
        {
            tfSelection.range->ShiftStart(ec, -1, &cch, NULL);
        }
        tfSelection.range->Collapse(ec, TF_ANCHOR_START);
    }
    else
    {
        // VK_RIGHT
        if (tfSelection.range->IsEqualEnd(ec, pRangeComposition, TF_ANCHOR_END, &fEqual) == S_OK &&
            !fEqual)
        {
            tfSelection.range->ShiftEnd(ec, +1, &cch, NULL);
        }
        tfSelection.range->Collapse(ec, TF_ANCHOR_END);
    }

    pContext->SetSelection(ec, 1, &tfSelection);

    pRangeComposition->Release();

Exit:
    tfSelection.range->Release();
    return S_OK; // eat the keystroke
}

//+---------------------------------------------------------------------------
//
// _HandleKeyDown
//
// If the keystroke happens within a composition, eat the key and return S_OK.
// Otherwise, do nothing and return S_FALSE.
//----------------------------------------------------------------------------

HRESULT CMarkTextService::_HandleKeyDown(TfEditCookie ec, ITfContext *pContext, WPARAM wParam)
{
    ITfRange *pRangeComposition;
    TF_SELECTION tfSelection;
    ULONG cFetched;
    HRESULT hr;
    WCHAR ch;
    BOOL fCovered;

    if (wParam < 'A' || wParam > 'Z')
        return S_OK; // just eat the key if it's not in a range we know how to handle

    hr = S_OK; // return S_FALSE to NOT eat the key

    // convert the wParam to a WCHAR
    if (GetKeyState(VK_SHIFT) & 0x8000)
    {
        // shift-key, leave it uppercase
        ch = (WCHAR)wParam;
    }
    else
    {
        // else make it lowercase
        ch = (WCHAR)(wParam | 32);
    }

    // first, test where a keystroke would go in the document if we did an insert
    if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK || cFetched != 1)
        return S_FALSE;

    // is the insertion point covered by a composition?
    if (_pComposition->GetRange(&pRangeComposition) == S_OK)
    {
        fCovered = IsRangeCovered(ec, tfSelection.range, pRangeComposition);

        pRangeComposition->Release();

        if (!fCovered)
        {
            hr = S_FALSE; // don't eat the key, it's outside our composition
            goto Exit;
        }
    }

    // insert the text
    // we use SetText here instead of InsertTextAtSelection because we've already started a composition
    // we don't want to the app to adjust the insertion point inside our composition
    if (tfSelection.range->SetText(ec, 0, &ch, 1) != S_OK)
        goto Exit;

    // update the selection, we'll make it an insertion point just past
    // the inserted text.
    tfSelection.range->Collapse(ec, TF_ANCHOR_END);

    pContext->SetSelection(ec, 1, &tfSelection);

    // apply our dislay attribute property to the inserted text
    // we need to apply it to the entire composition, since the
    // display attribute property is static, not static compact
    _SetCompositionDisplayAttributes(ec);

Exit:
    tfSelection.range->Release();
    return hr;
}

//+---------------------------------------------------------------------------
//
// _InitKeystrokeSink
//
// Advise a keystroke sink.
//----------------------------------------------------------------------------

BOOL CMarkTextService::_InitKeystrokeSink()
{
    ITfKeystrokeMgr *pKeystrokeMgr;
    HRESULT hr;

    if (_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr) != S_OK)
        return FALSE;

    hr = pKeystrokeMgr->AdviseKeyEventSink(_tfClientId, (ITfKeyEventSink *)this, TRUE);

    pKeystrokeMgr->Release();

    return (hr == S_OK);
}

//+---------------------------------------------------------------------------
//
// _UninitKeystrokeSink
//
// Unadvise a keystroke sink.  Assumes we have advised one already.
//----------------------------------------------------------------------------

void CMarkTextService::_UninitKeystrokeSink()
{
    ITfKeystrokeMgr *pKeystrokeMgr;

    if (_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr) != S_OK)
        return;

    pKeystrokeMgr->UnadviseKeyEventSink(_tfClientId);

    pKeystrokeMgr->Release();
}

//+---------------------------------------------------------------------------
//
// OnSetFocus
//
// Called by the system whenever this service gets the keystroke device focus.
//----------------------------------------------------------------------------

STDAPI CMarkTextService::OnSetFocus(BOOL fForeground)
{
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnTestKeyDown
//
// Called by the system to query if this service wants a potential keystroke.
//----------------------------------------------------------------------------

STDAPI CMarkTextService::OnTestKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    *pfEaten = (_pComposition != NULL);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnKeyDown
//
// Called by the system to offer this service a keystroke.  If *pfEaten == TRUE
// on exit, the application will not handle the keystroke.
//
// This text service is interested in handling keystrokes to demonstrate the
// use the compositions.  Some apps will cancel compositions if they receive
// keystrokes while a compositions is ongoing.
//----------------------------------------------------------------------------

STDAPI CMarkTextService::OnKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    CKeystrokeEditSession *pEditSession;
    HRESULT hr;

    hr = E_FAIL;
    *pfEaten = FALSE;

    if (_pComposition != NULL) // only eat keys while composing
    {
        // we'll insert a char ourselves in place of this keystroke
        if ((pEditSession = new CKeystrokeEditSession(this, pContext, wParam)) == NULL)
            goto Exit;

        // we need a lock to do our work
        // nb: this method is one of the few places where it is legal to use
        // the TF_ES_SYNC flag
        if (pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_SYNC | TF_ES_READWRITE, &hr) != S_OK)
        {
            hr = E_FAIL;
        }

        pEditSession->Release();
    }

Exit:
    // if we made it all the way to the RequestEditSession, then hr is ultimately the
    // return code from CKeystrokeEditSession::DoEditSession.  Our DoEditSession method
    // return S_OK to signal that the keystroke should be eaten, S_FALSE otherwise.
    if (hr == S_OK)
    {
        *pfEaten = TRUE;
    }
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// DoEditSession
//
//----------------------------------------------------------------------------

STDAPI CKeystrokeEditSession::DoEditSession(TfEditCookie ec)
{
    switch (_wParam)
    {
        case VK_LEFT:
        case VK_RIGHT:
            return _pMark->_HandleArrowKey(ec, _pContext, _wParam);

        case VK_RETURN:
            return _pMark->_HandleReturn(ec, _pContext);

        case VK_SPACE:
            return S_OK;
    }

    return _pMark->_HandleKeyDown(ec, _pContext, _wParam);
}

//+---------------------------------------------------------------------------
//
// OnTestKeyUp
//
// Called by the system to query this service wants a potential keystroke.
//----------------------------------------------------------------------------

STDAPI CMarkTextService::OnTestKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    *pfEaten = FALSE;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnKeyUp
//
// Called by the system to offer this service a keystroke.  If *pfEaten == TRUE
// on exit, the application will not handle the keystroke.
//----------------------------------------------------------------------------

STDAPI CMarkTextService::OnKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    *pfEaten = FALSE;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnPreservedKey
//
// Called when a hotkey (registered by us, or by the system) is typed.
//----------------------------------------------------------------------------

STDAPI CMarkTextService::OnPreservedKey(ITfContext *pContext, REFGUID rguid, BOOL *pfEaten)
{
    *pfEaten = FALSE;
    return S_OK;
}
