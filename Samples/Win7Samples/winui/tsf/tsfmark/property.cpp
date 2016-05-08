//
// property.cpp
//
// Property code.
//

#include "globals.h"
#include "mark.h"
#include "editsess.h"
#include "pstore.h"
#include <wchar.h>

// callback code for CPropertyEditSession
#define VIEW_CASE_PROPERTY   0
#define SET_CASE_PROPERTY    1
#define VIEW_CUSTOM_PROPERTY 2
#define SET_CUSTOM_PROPERTY  3

const TCHAR c_szWorkerWndClass[] = TEXT("Mark Worker Wnd Class");

class CPropertyEditSession : public CEditSessionBase
{
public:
    CPropertyEditSession(CMarkTextService *pMark, ITfContext *pContext, ULONG ulCallback) : CEditSessionBase(pContext)
    {
        _pMark = pMark;
        _pMark->AddRef();
        _ulCallback = ulCallback;
    }
    ~CPropertyEditSession()
    {
        _pMark->Release();
    }

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec)
    {
        switch (_ulCallback)
        {
            case VIEW_CASE_PROPERTY:
                _pMark->_ViewCaseProperty(ec, _pContext);
                break;
            case SET_CASE_PROPERTY:
                _pMark->_SetCaseProperty(ec, _pContext);
                break;
            case VIEW_CUSTOM_PROPERTY:
                _pMark->_ViewCustomProperty(ec, _pContext);
                break;
            case SET_CUSTOM_PROPERTY:
                _pMark->_SetCustomProperty(ec, _pContext);
                break;
        }
        return S_OK;
    }

private:
    CMarkTextService *_pMark;
    ULONG _ulCallback;
};

//+---------------------------------------------------------------------------
//
// _RequestEditSession
//
// Helper function.  Schedules an edit session for a particular property
// related callback.
//----------------------------------------------------------------------------

void CMarkTextService::_RequestPropertyEditSession(ULONG ulCallback)
{
    ITfDocumentMgr *pFocusDoc;
    ITfContext *pContext;
    CPropertyEditSession *pPropertyEditSession;
    HRESULT hr;

    // get the focus document
    if (_pThreadMgr->GetFocus(&pFocusDoc) != S_OK)
        return;

    if (pFocusDoc == NULL)
        return; // no focus

    // we want the topmost context, since the main doc context could be
    // superceded by a modal tip context
    if (pFocusDoc->GetTop(&pContext) != S_OK)
    {
        pContext = NULL;
        goto Exit;
    }

    if (pPropertyEditSession = new CPropertyEditSession(this, pContext, ulCallback))
    {
        // we need a document write lock
        // the CPropertyEditSession will do all the work when the
        // CPropertyEditSession::DoEditSession method is called by the context
        pContext->RequestEditSession(_tfClientId, pPropertyEditSession, TF_ES_READWRITE | TF_ES_ASYNCDONTCARE, &hr);

        pPropertyEditSession->Release();
    }

Exit:
    SafeRelease(pContext);
    pFocusDoc->Release();
}

//+---------------------------------------------------------------------------
//
// _SetCaseProperty
//
//----------------------------------------------------------------------------

void CMarkTextService::_SetCaseProperty(TfEditCookie ec, ITfContext *pContext)
{
    TF_SELECTION tfSelection;
    ITfProperty *pCaseProperty;
    ITfRange *pRangeChar;
    WCHAR ch;
    ULONG cchRead;
    ULONG cFetched;
    VARIANT varValue;

    // get the case property
    if (pContext->GetProperty(c_guidCaseProperty, &pCaseProperty) != S_OK)
        return;

    // get the selection
    if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK ||
        cFetched != 1)
    {
        // no selection or something went wrong
        tfSelection.range = NULL;
        goto Exit;
    }

    // get a helper range ready for the loop
    if (tfSelection.range->Clone(&pRangeChar) != S_OK)
        goto Exit;

    // set the value char-by-char over the selection
    while (TRUE)
    {
        // read one char, the TF_TF_MOVESTART flag will advance the start anchor
        if (tfSelection.range->GetText(ec, TF_TF_MOVESTART, &ch, 1, &cchRead) != S_OK)
            break;

        // any more text to read?
        if (cchRead != 1)
            break;

        // make pRange cover just the one char we read
        if (pRangeChar->ShiftEndToRange(ec, tfSelection.range, TF_ANCHOR_START) != S_OK)
            break;

        // set the value
        varValue.vt = VT_I4;
        varValue.lVal = (ch >= 'A' && ch <= 'Z');

        if (pCaseProperty->SetValue(ec, pRangeChar, &varValue) != S_OK)
            break;

        // advance pRange for next iteration
        if (pRangeChar->Collapse(ec, TF_ANCHOR_END) != S_OK)
            break;
    }
    
    pRangeChar->Release();

Exit:
    SafeRelease(tfSelection.range);
    pCaseProperty->Release();
}

//+---------------------------------------------------------------------------
//
// _Menu_OnSetCaseProperty
//
// Callback for the "Set Case Property" menu item.
// Set the value for a private "case" property over the text covered by the
// selection.  The case property is private to this text service, which defines
// it as:
//
//      static compact, per character
//      VT_I4, !0 => character is within 'A' - 'Z', 0 => anything else.
//      
//----------------------------------------------------------------------------

/* static */
void CMarkTextService::_Menu_OnSetCaseProperty(CMarkTextService *_this)
{
    _this->_RequestPropertyEditSession(SET_CASE_PROPERTY);
}

//+---------------------------------------------------------------------------
//
// _ViewCaseProperty
//
//----------------------------------------------------------------------------

void CMarkTextService::_ViewCaseProperty(TfEditCookie ec, ITfContext *pContext)
{
    TF_SELECTION tfSelection;
    ITfProperty *pCaseProperty;
    ULONG cchRead;
    LONG cch;
    ULONG cFetched;
    ULONG i;
    VARIANT varValue;

    // get the case property
    if (pContext->GetProperty(c_guidCaseProperty, &pCaseProperty) != S_OK)
        return;

    // get the selection
    if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK ||
        cFetched != 1)
    {
        // no selection or something went wrong
        tfSelection.range = NULL;
        goto Exit;
    }

    // grab the text
    if (tfSelection.range->GetText(ec, 0, _achDisplayText, ARRAYSIZE(_achDisplayText)-1, &cchRead) != S_OK)
        goto Exit;

    // prepare for the loop
    if (tfSelection.range->Collapse(ec, TF_ANCHOR_START) != S_OK)
        goto Exit;

    // get the property value char-by-char over the selection
    for (i=0; i < cchRead; i++)
    {
        // advance pRange for next iteration, cover the next char
        if (tfSelection.range->ShiftStartToRange(ec, tfSelection.range, TF_ANCHOR_END) != S_OK)
            break;
        if (tfSelection.range->ShiftEnd(ec, 1, &cch, NULL) != S_OK)
            break;
        if (cch != 1) // hit a region boundary?
            break;

        switch (pCaseProperty->GetValue(ec, tfSelection.range, &varValue))
        {
            case S_OK:
                // the property value has been set, use it
                // 'U' --> uppercase
                // 'L' --> lowercase
                _achDisplayPropertyText[i] = varValue.lVal ? 'U' : 'L';
                break;
            case S_FALSE:
                // no property value set, varValue.vt == VT_EMPTY
                // '?' --> no value
                _achDisplayPropertyText[i] = '?';
                break;
            default:
                // error
                // '!' --> error
                _achDisplayPropertyText[i] = '!';
                break;
        }
    }
    for (; i<cchRead; i++) // error case
    {
        _achDisplayPropertyText[i] = '!';
    }

    _achDisplayPropertyText[cchRead] = '\0';
    _achDisplayText[cchRead] = '\0';

    // we can't change the focus while holding a lock
    // so postpone the UI until we've released our lock
    PostMessage(_hWorkerWnd, CMarkTextService::WM_DISPLAY_PROPERTY, 0, 0);

Exit:
    SafeRelease(tfSelection.range);
    pCaseProperty->Release();
}

//+---------------------------------------------------------------------------
//
// _Menu_OnViewCaseProperty
//
// Menu callback.  Displays a popup with "case" property values over the
// current selection.
//----------------------------------------------------------------------------

/* static */
void CMarkTextService::_Menu_OnViewCaseProperty(CMarkTextService *_this)
{
    _this->_RequestPropertyEditSession(VIEW_CASE_PROPERTY);
}

//+---------------------------------------------------------------------------
//
// _ViewCustomProperty
//
// Display the value of this text service's custom property over the text
// covered by the selection.
//----------------------------------------------------------------------------

void CMarkTextService::_ViewCustomProperty(TfEditCookie ec, ITfContext *pContext)
{
    TF_SELECTION tfSelection;
    ITfProperty *pCustomProperty;
    ITfRange *pSelRange;
    ITfRange *pPropertySpanRange;
    ULONG cchRead;
    ULONG cFetched;
    LONG cch;
    VARIANT varValue;
    HRESULT hr;

    // get the case property
    if (pContext->GetProperty(c_guidCustomProperty, &pCustomProperty) != S_OK)
        return;

    // get the selection
    if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK ||
        cFetched != 1)
    {
        // no selection or something went wrong
        pSelRange = NULL;
        goto Exit;
    }
    // free up tfSelection so we can re-use it below
    pSelRange = tfSelection.range;

    // the selection may not exactly match a span of text covered by the
    // custom property....so we'll return the value over the start anchor of
    // the selection.

    // we need to collapse the range because GetValue will return VT_EMPTY
    // if the query range is not completely covered by the property span
    if (pSelRange->Collapse(ec, TF_ANCHOR_START) != S_OK)
        goto Exit;
    // the query range must also cover at least one char
    if (pSelRange->ShiftEnd(ec, 1, &cch, NULL) != S_OK)
        goto Exit;

    hr = pCustomProperty->GetValue(ec, pSelRange, &varValue);

    switch (hr)
    {
        case S_OK:
            // there's a value at the selection start anchor
            // let's find out exactly what text is covered
            _achDisplayText[0] = '\0';
            if (pCustomProperty->FindRange(ec, pSelRange, &pPropertySpanRange, TF_ANCHOR_START) == S_OK)
            {
                if (pPropertySpanRange->GetText(ec, 0, _achDisplayText, ARRAYSIZE(_achDisplayText)-1, &cchRead) != S_OK)
                {
                    cchRead = 0;
                }
                _achDisplayText[cchRead] = '\0';
                // let's update the selection to give the user feedback
                tfSelection.range = pPropertySpanRange;
                pContext->SetSelection(ec, 1, &tfSelection);
                pPropertySpanRange->Release();
            }
            // write the value
            swprintf_s(_achDisplayPropertyText, ARRAYSIZE(_achDisplayPropertyText), L"%i", varValue.lVal);
            break;

        case S_FALSE:
            // the property has no value, varValue.vt == VT_EMPTY
            _achDisplayText[0] = '\0';
            wcscpy_s(_achDisplayPropertyText, ARRAYSIZE(_achDisplayPropertyText), L"- No Value -");
            break;
        default:
            goto Exit; // error
    }

    // we can't change the focus while holding a lock
    // so postpone the UI until we've released our lock
    PostMessage(_hWorkerWnd, CMarkTextService::WM_DISPLAY_PROPERTY, 0, 0);

Exit:
    SafeRelease(pSelRange);
    pCustomProperty->Release();
}

//+---------------------------------------------------------------------------
//
// _Menu_OnViewCustomProperty
//
// Menu callback for "View Custom Property".
//----------------------------------------------------------------------------

/* static */
void CMarkTextService::_Menu_OnViewCustomProperty(CMarkTextService *_this)
{
    _this->_RequestPropertyEditSession(VIEW_CUSTOM_PROPERTY);
}

//+---------------------------------------------------------------------------
//
// _InitWorkerWnd
//
// Called from Activate.  Create a worker window to receive private windows
// messages.
//----------------------------------------------------------------------------

BOOL CMarkTextService::_InitWorkerWnd()
{
    WNDCLASS wc;

    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = _WorkerWndProc;
    wc.hInstance = g_hInst;
    wc.lpszClassName = c_szWorkerWndClass;

    if (RegisterClass(&wc) == 0)
        return FALSE;

    _hWorkerWnd = CreateWindow(c_szWorkerWndClass, TEXT("Mark Worker Wnd"),
                               0, 0, 0, 0, 0, NULL, NULL, g_hInst, this);

    return (_hWorkerWnd != NULL);
}

//+---------------------------------------------------------------------------
//
// _UninitWorkerWnd
//
// Called from Deactivate.  Destroy the worker window.
//----------------------------------------------------------------------------

void CMarkTextService::_UninitWorkerWnd()
{
    if (_hWorkerWnd != NULL)
    {
        DestroyWindow(_hWorkerWnd);
        _hWorkerWnd = NULL;
    }
    UnregisterClass(c_szWorkerWndClass, g_hInst);
}

//+---------------------------------------------------------------------------
//
// _WorkerWndProc
//
//----------------------------------------------------------------------------

/* static */
LRESULT CALLBACK CMarkTextService::_WorkerWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CMarkTextService *_this;
    int cch;
    char achText[128];

    switch (uMsg)
    {
        case WM_CREATE:
            // save the this pointer we originally passed into CreateWindow
            SetWindowLongPtr(hWnd, GWLP_USERDATA, 
                             (LONG_PTR)((CREATESTRUCT *)lParam)->lpCreateParams);
            return 0;

        case WM_DISPLAY_PROPERTY:
            _this = (CMarkTextService *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

            // bring up a message box with the contents of _achDisplayText
            
            // first, convert from unicode
            cch = WideCharToMultiByte(CP_ACP, 0, _this->_achDisplayText, wcslen(_this->_achDisplayText),
                                      achText, ARRAYSIZE(achText)-1, NULL, NULL);

            if (cch < ARRAYSIZE(achText) - 1)
            {
                achText[cch++] = '\n';
            }
            if (cch < ARRAYSIZE(achText) - 1)
            {
                cch += WideCharToMultiByte(CP_ACP, 0, _this->_achDisplayPropertyText, wcslen(_this->_achDisplayPropertyText),
                                           achText+cch, ARRAYSIZE(achText)-cch-1, NULL, NULL);
            }
            achText[cch] = '\0';

            // bring up the display
            MessageBoxA(NULL, achText, "Property View", MB_OK);

            return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//+---------------------------------------------------------------------------
//
// _Menu_OnSetCustomProperty
//
// Callback for the "Set Custom Property" menu item.
//----------------------------------------------------------------------------

/* static */
void CMarkTextService::_Menu_OnSetCustomProperty(CMarkTextService *_this)
{
    _this->_RequestPropertyEditSession(SET_CUSTOM_PROPERTY);
}

//+---------------------------------------------------------------------------
//
// _SetCustomProperty
//
// Assign a custom property to the text covered by the selection.
//----------------------------------------------------------------------------

void CMarkTextService::_SetCustomProperty(TfEditCookie ec, ITfContext *pContext)
{
    TF_SELECTION tfSelection;
    ITfProperty *pCustomProperty;
    CCustomPropertyStore *pCustomPropertyStore;
    ULONG cFetched;

    // get the case property
    if (pContext->GetProperty(c_guidCustomProperty, &pCustomProperty) != S_OK)
        return;

    // get the selection
    if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK ||
        cFetched != 1)
    {
        // no selection or something went wrong
        tfSelection.range = NULL;
        goto Exit;
    }

    if ((pCustomPropertyStore = new CCustomPropertyStore) == NULL)
        goto Exit;

    pCustomProperty->SetValueStore(ec, tfSelection.range, pCustomPropertyStore);

    // TSF will hold a reference to pCustomPropertyStore is the SetValueStore succeeded
    // but we need to release ours
    pCustomPropertyStore->Release();

Exit:
    pCustomProperty->Release();
    SafeRelease(tfSelection.range);
}
