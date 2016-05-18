//
// snoop.cpp
//
// CSnoopWnd implementation.
//

#include "globals.h"
#include "snoop.h"
#include "case.h"
#include "editsess.h"

class CUpdateTextEditSession : public CEditSessionBase
{
public:
    CUpdateTextEditSession(ITfContext *pContext, ITfRange *pRange, CSnoopWnd *pSnoopWnd) : CEditSessionBase(pContext)
    {
        _pSnoopWnd = pSnoopWnd;
        _pRange = pRange;
        if (_pRange != NULL)
        {
            _pRange->AddRef();
        }
    }
    ~CUpdateTextEditSession()
    {
        SafeRelease(_pRange);
    }

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec);

private:
    CSnoopWnd *_pSnoopWnd;
    ITfRange *_pRange;
};


#define SNOOP_X_POS     0
#define SNOOP_Y_POS     0

#define SNOOP_WIDTH     300
#define SNOOP_HEIGHT    (SNOOP_WIDTH / 3)

ATOM CSnoopWnd::_atomWndClass = 0;

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

CSnoopWnd::CSnoopWnd(CCaseTextService *pCase)
{
    _pCase = pCase; // no AddRef because CSnoopWnd is contained in the
                    // pCase lifetime
    _hWnd = NULL;
    _cchText = 0;
}

//+---------------------------------------------------------------------------
//
// _InitClass
//
//----------------------------------------------------------------------------

/* static */
BOOL CSnoopWnd::_InitClass()
{
    WNDCLASS wc;

    wc.style = 0;
    wc.lpfnWndProc = CSnoopWnd::_WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = g_hInst;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = TEXT("SnoopWndClass");

    _atomWndClass = RegisterClass(&wc);

    return (_atomWndClass != 0);
}

//+---------------------------------------------------------------------------
//
// _UninitClass
//
//----------------------------------------------------------------------------

/* static */
void CSnoopWnd::_UninitClass()
{
    if (_atomWndClass != 0)
    {
        UnregisterClass((LPCTSTR)_atomWndClass, g_hInst);
    }
}


//+---------------------------------------------------------------------------
//
// _Init
//
//----------------------------------------------------------------------------

BOOL CSnoopWnd::_Init()
{
    // nb: on windows 2000, you can use WS_EX_NOACTIVATE to prevent windows
    // from taking the foreground.  We don't use that here for compatibility.
    // Instead, we use WS_DISABLED, which can be burdensome for more complex
    // ui.

    _hWnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
                           (LPCTSTR)_atomWndClass,
                           TEXT("Snoop Window"),
                           WS_BORDER | WS_DISABLED | WS_POPUP,
                           SNOOP_X_POS, SNOOP_Y_POS,
                           SNOOP_WIDTH, SNOOP_HEIGHT,
                           NULL,
                           NULL,
                           g_hInst,
                           this);

    return (_hWnd != NULL);
}

//+---------------------------------------------------------------------------
//
// _Uninit
//
//----------------------------------------------------------------------------

void CSnoopWnd::_Uninit()
{
    if (_hWnd != NULL)
    {
        DestroyWindow(_hWnd);
        _hWnd = NULL;
    }
}

//+---------------------------------------------------------------------------
//
// _Show
//
//----------------------------------------------------------------------------

void CSnoopWnd::_Show()
{
    ShowWindow(_hWnd, SW_SHOWNA);
}

//+---------------------------------------------------------------------------
//
// _Hide
//
//----------------------------------------------------------------------------

void CSnoopWnd::_Hide()
{
    ShowWindow(_hWnd, SW_HIDE);
}

//+---------------------------------------------------------------------------
//
// _UpdateText
//
//----------------------------------------------------------------------------

void CSnoopWnd::_UpdateText(ITfRange *pRange)
{
    ITfDocumentMgr *pdmFocus;
    ITfContext *pContext;
    CUpdateTextEditSession *pEditSession;
    HRESULT hr;

    if (pRange == NULL)
    {
        // caller wants us to just use the selection in the focus doc
        if (_pCase->_GetThreadMgr()->GetFocus(&pdmFocus) != S_OK)
            return;

        hr = pdmFocus->GetTop(&pContext);

        pdmFocus->Release();

        if (hr != S_OK)
            return;
    }
    else if (pRange->GetContext(&pContext) != S_OK)
        return;

    if (pEditSession = new CUpdateTextEditSession(pContext, pRange, this))
    {
        // we need a document read lock to scan text
        // the CUpdateTextEditSession will do all the work when the
        // CUpdateTextEditSession::DoEditSession method is called by the context
        pContext->RequestEditSession(_pCase->_GetClientId(), pEditSession, TF_ES_READ | TF_ES_ASYNCDONTCARE, &hr);

        pEditSession->Release();
    }

    pContext->Release();
}

//+---------------------------------------------------------------------------
//
// DoEditSession
//
//----------------------------------------------------------------------------

STDAPI CUpdateTextEditSession::DoEditSession(TfEditCookie ec)
{
    _pSnoopWnd->_UpdateText(ec, _pContext, _pRange);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _UpdateText
//
//----------------------------------------------------------------------------

void CSnoopWnd::_UpdateText(TfEditCookie ec, ITfContext *pContext, ITfRange *pRange)
{
    LONG cchBefore;
    LONG cchAfter;
    TF_SELECTION tfSelection;
    ULONG cFetched;
    BOOL fReleaseRange = FALSE;

    if (pRange == NULL)
    {
        // caller wants us to use the selection
        if (pContext->GetSelection(ec, TS_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK ||
            cFetched != 1)
        {
            return;
        }

        pRange = tfSelection.range; // no AddRef, take ownership of the pointer
        fReleaseRange = TRUE;
    }

    // arbitrarily grab some text before and after the range start anchor

    pRange->Collapse(ec, TF_ANCHOR_START);

    pRange->ShiftStart(ec, -MAX_SNOOP_TEXT / 2, &cchBefore, NULL);

    cchBefore = -cchBefore; // we shifted backwards, so make count a positive number

    pRange->GetText(ec, 0, _achText, cchBefore, (ULONG *)&cchBefore);

    pRange->Collapse(ec, TF_ANCHOR_END);

    pRange->ShiftEnd(ec, MAX_SNOOP_TEXT - cchBefore, &cchAfter, NULL);

    pRange->GetText(ec, 0, _achText + cchBefore, cchAfter, (ULONG *)&cchAfter);

    _cchText = cchBefore + cchAfter;

    // force a repaint

    InvalidateRect(_hWnd, NULL, TRUE);

    if (fReleaseRange)
    {
        pRange->Release();
    }
}

//+---------------------------------------------------------------------------
//
// _WndProc
//
// Snoop window proc.
//----------------------------------------------------------------------------

/* static */
LRESULT CALLBACK CSnoopWnd::_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;

    switch (uMsg)
    {
        case WM_CREATE:
            _SetThis(hWnd, lParam);
            return 0;

        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            _GetThis(hWnd)->_OnPaint(hWnd, hdc);
            EndPaint(hWnd, &ps);
            return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//+---------------------------------------------------------------------------
//
// _OnPaint
//
// WM_PAINT handler for CSnoopWnd.
//----------------------------------------------------------------------------

void CSnoopWnd::_OnPaint(HWND hWnd, HDC hdc)
{
    RECT rc;

    // background
    GetClientRect(hWnd, &rc);
    FillRect(hdc, &rc, (HBRUSH)GetStockObject(LTGRAY_BRUSH));

    // text
    TextOutW(hdc, 0, 0, _achText, _cchText);
}

//+---------------------------------------------------------------------------
//
// _InitSnoopWnd
//
// Create and init the snoop window.
//----------------------------------------------------------------------------

BOOL CCaseTextService::_InitSnoopWnd()
{
    BOOL fThreadfocus;
    ITfSource *pSource = NULL;

    // create a snoop window

    if ((_pSnoopWnd = new CSnoopWnd(this)) == NULL)
        return FALSE;

    if (!_pSnoopWnd->_Init())
        goto ExitError;

    // we also need a thread focus sink

    if (_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource) != S_OK)
    {
        pSource = NULL;
        goto ExitError;
    }

    if (pSource->AdviseSink(IID_ITfThreadFocusSink, (ITfThreadFocusSink *)this, &_dwThreadFocusSinkCookie) != S_OK)
    {
        // make sure we don't try to Unadvise _dwThreadFocusSinkCookie later
        _dwThreadFocusSinkCookie = TF_INVALID_COOKIE;
        goto ExitError;
    }

    pSource->Release();

    // we may need to display the snoop window right now
    // our thread focus sink won't be called until something changes,
    // so we need to check the current state.

    if (_pThreadMgr->IsThreadFocus(&fThreadfocus) == S_OK && fThreadfocus)
    {
        OnSetThreadFocus();
    }

    return TRUE;

ExitError:
    SafeRelease(pSource);
    _UninitSnoopWnd();
    return FALSE;
}

//+---------------------------------------------------------------------------
//
// _UninitSnoopWnd
//
// Uninit and free the snoop window, unadvise the thread focus sink.
//----------------------------------------------------------------------------

void CCaseTextService::_UninitSnoopWnd()
{
    ITfSource *pSource;

    if (_pSnoopWnd != NULL)
    {
        _pSnoopWnd->_Uninit();
        delete _pSnoopWnd;
    }

    if (_dwThreadFocusSinkCookie != TF_INVALID_COOKIE)
    {
        if (_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource) == S_OK)
        {
            pSource->UnadviseSink(_dwThreadFocusSinkCookie);
            pSource->Release();
        }

        _dwThreadFocusSinkCookie = TF_INVALID_COOKIE;
    }
}

//+---------------------------------------------------------------------------
//
// _Menu_ShowSnoopWnd
//
// Show or hide the snoop window.
//----------------------------------------------------------------------------

void CCaseTextService::_Menu_ShowSnoopWnd(CCaseTextService *_this)
{
    _this->_fShowSnoop = !_this->_fShowSnoop;

    if (_this->_fShowSnoop)
    {
        _this->_pSnoopWnd->_Show();
    }
    else
    {
        _this->_pSnoopWnd->_Hide();
    }
}

//+---------------------------------------------------------------------------
//
// OnSetThreadFocus
//
// Called by the system when the thread/appartment of this text service gains
// the ui focus.
//----------------------------------------------------------------------------

STDAPI CCaseTextService::OnSetThreadFocus()
{
    if (_fShowSnoop)
    {
        _pSnoopWnd->_Show();
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnKillThreadFocus
//
// Called by the system when the thread/appartment of this text service loses
// the ui focus.
//----------------------------------------------------------------------------

STDAPI CCaseTextService::OnKillThreadFocus()
{
    // only show our snoop window when our thread has the focus.
    if (_fShowSnoop)
    {
        _pSnoopWnd->_Hide();
    }

    return S_OK;
}
