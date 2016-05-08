//
// keys.cpp
//
// ITfKeyEventSink implementation.
//

#include "globals.h"
#include "case.h"
#include "editsess.h"

class CKeystrokeEditSession : public CEditSessionBase
{
public:
    CKeystrokeEditSession(ITfContext *pContext, WPARAM wParam) : CEditSessionBase(pContext)
    {
        _wParam = wParam;
    }

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec);

private:
    WPARAM _wParam;
};


/* 5d6d1b1e-64f2-47cd-9fe1-4e032c2dae77 */
static const GUID GUID_PRESERVEDKEY_FLIPCASE = { 0x5d6d1b1e, 0x64f2, 0x47cd, {0x9f, 0xe1, 0x4e, 0x03, 0x2c, 0x2d, 0xae, 0x77} };
// arbitrary hotkey: ctl-f
static const TF_PRESERVEDKEY c_FlipCaseKey = { 'F', TF_MOD_CONTROL };


//+---------------------------------------------------------------------------
//
// IsKeyEaten
//
//----------------------------------------------------------------------------

inline BOOL IsKeyEaten(BOOL fFlipKeys, WPARAM wParam)
{
    // we're only interested in VK_A - VK_Z, when the "Flip Keys" menu option
    // is on
    return fFlipKeys && (wParam >= 'A') && (wParam <= 'Z');
}

//+---------------------------------------------------------------------------
//
// _Menu_FlipKeys
//
// Advise or unadvise a keystroke sink.
//----------------------------------------------------------------------------

/* static */
void CCaseTextService::_Menu_FlipKeys(CCaseTextService *_this)
{
    _this->_fFlipKeys = !_this->_fFlipKeys;
}

//+---------------------------------------------------------------------------
//
// _InitKeystrokeSink
//
// Advise a keystroke sink.
//----------------------------------------------------------------------------

BOOL CCaseTextService::_InitKeystrokeSink()
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

void CCaseTextService::_UninitKeystrokeSink()
{
    ITfKeystrokeMgr *pKeystrokeMgr;

    if (_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr) != S_OK)
        return;

    pKeystrokeMgr->UnadviseKeyEventSink(_tfClientId);

    pKeystrokeMgr->Release();
}

//+---------------------------------------------------------------------------
//
// _InitPreservedKey
//
// Register a hot key.
//----------------------------------------------------------------------------

BOOL CCaseTextService::_InitPreservedKey()
{
    const WCHAR wchToggleCase[] = L"Toggle Case";
    ITfKeystrokeMgr *pKeystrokeMgr;
    HRESULT hr;

    if (_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr) != S_OK)
        return FALSE;

    hr = pKeystrokeMgr->PreserveKey(_tfClientId, GUID_PRESERVEDKEY_FLIPCASE,
                                    &c_FlipCaseKey, wchToggleCase,
                                    ARRAYSIZE(wchToggleCase)-1);

    pKeystrokeMgr->Release();

    return (hr == S_OK);
}

//+---------------------------------------------------------------------------
//
// _UninitPreservedKey
//
// Uninit a hot key.
//----------------------------------------------------------------------------

void CCaseTextService::_UninitPreservedKey()
{
    ITfKeystrokeMgr *pKeystrokeMgr;

    if (_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr) != S_OK)
        return;

    pKeystrokeMgr->UnpreserveKey(GUID_PRESERVEDKEY_FLIPCASE, &c_FlipCaseKey);

    pKeystrokeMgr->Release();
}

//+---------------------------------------------------------------------------
//
// OnSetFocus
//
// Called by the system whenever this service gets the keystroke device focus.
//----------------------------------------------------------------------------

STDAPI CCaseTextService::OnSetFocus(BOOL fForeground)
{
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnTestKeyDown
//
// Called by the system to query this service wants a potential keystroke.
//----------------------------------------------------------------------------

STDAPI CCaseTextService::OnTestKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    *pfEaten = IsKeyEaten(_fFlipKeys, wParam);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnKeyDown
//
// Called by the system to offer this service a keystroke.  If *pfEaten == TRUE
// on exit, the application will not handle the keystroke.
//----------------------------------------------------------------------------

STDAPI CCaseTextService::OnKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    CKeystrokeEditSession *pEditSession;
    HRESULT hr = S_OK;

    *pfEaten = IsKeyEaten(_fFlipKeys, wParam);

    if (*pfEaten)
    {
        // we'll insert a char ourselves in place of this keystroke
        if ((pEditSession = new CKeystrokeEditSession(pContext, wParam)) == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

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
    if (hr != S_OK)
    {
        *pfEaten = FALSE;
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
    WCHAR wc;

    // we want to toggle the english case of the keystroke
    // nb: this is quick-and-dirty code, not intended to demonstrate the
    // correct way to flip capitalization!

    if (GetKeyState(VK_SHIFT) & 0x8000)
    {
        // shift-key, make it lowercase
        wc = (WCHAR)(_wParam | 32);
    }
    else
    {
        // else make it capital
        wc = (WCHAR)_wParam;
    }

    InsertTextAtSelection(ec, _pContext, &wc, 1);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnTestKeyUp
//
// Called by the system to query this service wants a potential keystroke.
//----------------------------------------------------------------------------

STDAPI CCaseTextService::OnTestKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    *pfEaten = IsKeyEaten(_fFlipKeys, wParam);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnKeyUp
//
// Called by the system to offer this service a keystroke.  If *pfEaten == TRUE
// on exit, the application will not handle the keystroke.
//----------------------------------------------------------------------------

STDAPI CCaseTextService::OnKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    *pfEaten = IsKeyEaten(_fFlipKeys, wParam);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnPreservedKey
//
// Called when a hotkey (registered by us, or by the system) is typed.
//----------------------------------------------------------------------------

STDAPI CCaseTextService::OnPreservedKey(ITfContext *pContext, REFGUID rguid, BOOL *pfEaten)
{
    if (IsEqualGUID(rguid, GUID_PRESERVEDKEY_FLIPCASE))
    {
        _Menu_FlipDoc(this);
        *pfEaten = TRUE;
    }
    else
    {
        *pfEaten = FALSE;
    }

    return S_OK;
}
