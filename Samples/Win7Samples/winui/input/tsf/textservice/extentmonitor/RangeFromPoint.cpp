//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  ExtentVisual.cpp
//
//          CRangeFromPointViewer
//
//////////////////////////////////////////////////////////////////////

#include "Globals.h"
#include "TextService.h"
#include "RangeFromPoint.h"
#include "EditSession.h"

TCHAR CRangeFromPointViewer::_szWndClass[] = TEXT("RangeFromPointViewer");

//+---------------------------------------------------------------------------
//
// CRangeFromPointViewer
//
//----------------------------------------------------------------------------

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

CRangeFromPointViewer::CRangeFromPointViewer(CExtentMonitorTextService *pTextService, WCHAR *psz, COLORREF cr)
{
    _hwnd = NULL;
    StringCchCopy(_sz, ARRAYSIZE(_sz), psz);
    _cr = cr;
    _pTextService = pTextService;
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

CRangeFromPointViewer::~CRangeFromPointViewer()
{
    if (IsWindow(_hwnd))
        DestroyWindow(_hwnd);
}


//+---------------------------------------------------------------------------
//
// StaticInit
//
//----------------------------------------------------------------------------

BOOL CRangeFromPointViewer::StaticInit()
{

    WNDCLASSEX wcex;

    memset(&wcex, 0, sizeof(wcex));
    wcex.cbSize        = sizeof(wcex);
    wcex.style         = CS_HREDRAW | CS_VREDRAW ;
    wcex.hInstance     = g_hInst;
    wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground    = (HBRUSH)(COLOR_WINDOW+1);

    wcex.lpfnWndProc   = _WndProc;
    wcex.lpszClassName = _szWndClass;
    RegisterClassEx(&wcex);

    return TRUE;
}

//+---------------------------------------------------------------------------
//
// CreateWnd
//
//----------------------------------------------------------------------------

HWND CRangeFromPointViewer::CreateWnd()
{
    if (_hwnd)
        return _hwnd;

    _hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED, _szWndClass, TEXT(""),
                           WS_POPUP | WS_DISABLED,
                           0, 0, 0, 0,
                           NULL, 0, g_hInst, this);

    // SetLayeredWindowAttributes(_hwnd, _cr, 240, LWA_COLORKEY | LWA_ALPHA);
    SetLayeredWindowAttributes(_hwnd, 0, 128, LWA_ALPHA);

    return _hwnd;
}

//+---------------------------------------------------------------------------
//
// _OwnerWndProc
//
//----------------------------------------------------------------------------

LRESULT CALLBACK CRangeFromPointViewer::_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CRangeFromPointViewer *_this;
    HDC hdc;
    PAINTSTRUCT ps;
  
    _this = _GetThis(hwnd);

    switch (uMsg)
    {
        case WM_CREATE:
            _SetThis(hwnd, lParam);
            return 0;

        case WM_SETCURSOR:
            if (_this)
                _this->OnSetCursor(hwnd, wParam, lParam);
            break;

        case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);
            if (_this)
                _this->OnPaint(hwnd, hdc);
            EndPaint(hwnd, &ps);
            break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}


//+---------------------------------------------------------------------------
//
// Show
//
//----------------------------------------------------------------------------

void CRangeFromPointViewer::Show(RECT *prc)
{
    if (!IsWindow(_hwnd))
        return;

    InvalidateRect(_hwnd, NULL, TRUE);
    SetWindowPos(_hwnd, 
                 HWND_TOPMOST, 
                 prc->left,
                 prc->top,
                 prc->right - prc->left > 2 ? prc->right - prc->left : 2,
                 prc->bottom - prc->top > 2 ? prc->bottom - prc->top : 2,
                 SWP_SHOWWINDOW | SWP_NOACTIVATE);

}


//+---------------------------------------------------------------------------
//
// Hide
//
//----------------------------------------------------------------------------

void CRangeFromPointViewer::Hide()
{
    if (!IsWindow(_hwnd))
        return;

    ShowWindow(_hwnd, SW_HIDE);
}


//+---------------------------------------------------------------------------
//
// OnPaint
//
//----------------------------------------------------------------------------

void CRangeFromPointViewer::OnPaint(HWND hwnd, HDC hdc)
{
    HFONT hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    HFONT hfontOld = (HFONT)SelectObject(hdc, hfont);

    RECT rc;
    GetClientRect(hwnd, &rc);
    SetBkColor(hdc, _cr);
    // ExtTextOutW(hdc, 2, 2, ETO_OPAQUE, &rc, _sz, lstrlenW(_sz), NULL);
    ExtTextOutW(hdc, 2, 2, ETO_OPAQUE, &rc, _sz, 0, NULL);

    if (_fIsRectNearest)
        FrameRect(hdc, &_rcNearest, (HBRUSH)GetStockObject(BLACK_BRUSH));

    SelectObject(hdc, hfontOld);
}

//+---------------------------------------------------------------------------
//
// OnSetCursor
//
//----------------------------------------------------------------------------

void CRangeFromPointViewer::OnSetCursor(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    POINT pt;

    switch (HIWORD(lParam))
    {
        case WM_LBUTTONDOWN:
            _fIsRect = FALSE;
            _fIsRectRoundNearest = FALSE;
            _fIsRectNearest = FALSE;
            GetCursorPos( &pt );
            _pTextService->GetRectFromPointOnFocusDocument(pt);
            break;

        case WM_RBUTTONDOWN:
            _fIsRect = FALSE;
            _fIsRectRoundNearest = FALSE;
            _fIsRectNearest = FALSE;
            InvalidateRect(_hwnd, NULL, TRUE);
            break;
    }
}

//+---------------------------------------------------------------------------
//
// _EnsurePopupWindow
//
//----------------------------------------------------------------------------

void CExtentMonitorTextService::_EnsureRangeFromPointViewer()
{
    if (!_pRangeFromPointViewer)
         _pRangeFromPointViewer = new CRangeFromPointViewer(this, L"View", 0xD00000);
    if (_pRangeFromPointViewer)
        _pRangeFromPointViewer->CreateWnd();
}

//+---------------------------------------------------------------------------
//
// _UpdatePopupWindow
//
//----------------------------------------------------------------------------

void CExtentMonitorTextService::_UpdateRangeFromPointViewer()
{
    if (_pRangeFromPointViewer)
    {
        _pRangeFromPointViewer->Show(&_rcView);
    }

    _fIsShownRangeFromPointViewer = TRUE;
}

//+---------------------------------------------------------------------------
//
// _HidePopupWindow
//
//----------------------------------------------------------------------------

void CExtentMonitorTextService::_HideRangeFromPointViewer()
{
    if (_pRangeFromPointViewer)
        _pRangeFromPointViewer->Hide();

    _fIsShownRangeFromPointViewer = FALSE;
}

//+---------------------------------------------------------------------------
//
// CGetRectFromPointEditSession
//
//----------------------------------------------------------------------------

class CGetRectFromPointEditSession : public CEditSessionBase
{
public:
    CGetRectFromPointEditSession(CExtentMonitorTextService *pTextService, ITfContext *pContext, POINT pt) : CEditSessionBase(pTextService, pContext)
    {
        _pt = pt;
    }

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec);

    POINT _pt;

};

//+---------------------------------------------------------------------------
//
// DoEditSession
//
//----------------------------------------------------------------------------

STDAPI CGetRectFromPointEditSession::DoEditSession(TfEditCookie ec)
{
    _pTextService->_GetRectFromPoint(ec, _pContext, _pt);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// GetRectFromPointOnFocusDocument
//
//----------------------------------------------------------------------------

void CExtentMonitorTextService::GetRectFromPointOnFocusDocument(POINT pt)
{
    ITfDocumentMgr *pDocMgrFocus;
    if ((_pThreadMgr->GetFocus(&pDocMgrFocus) == S_OK) &&
        (pDocMgrFocus != NULL))
    {
        ITfContext *pContext;
        if (SUCCEEDED(pDocMgrFocus->GetBase(&pContext)))
        {
            CGetRectFromPointEditSession *pEditSession;
            HRESULT hr = E_FAIL;

            if ((pEditSession = new CGetRectFromPointEditSession(this, pContext, pt)) != NULL)
            {
                pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READ, &hr);
                pEditSession->Release();
            }

            pContext->Release();
        }
        pDocMgrFocus->Release();
    }
}

//+---------------------------------------------------------------------------
//
// _GetRectFromPoint
//
//----------------------------------------------------------------------------

void CExtentMonitorTextService::_GetRectFromPoint(TfEditCookie ec, ITfContext *pContext, POINT pt)
{
    ITfContextView *pContextView;

    if (!_pRangeFromPointViewer)
        return;

    if (SUCCEEDED(pContext->GetActiveView(&pContextView)))
    {
        ITfRange *pRange;
        ITfRange *pRangeRoundNearest;
        ITfRange *pRangeNearest;
        BOOL fClipped;
        RECT rcScreen = {0};

        if (SUCCEEDED(pContextView->GetScreenExt(&rcScreen)))
        {
            _pRangeFromPointViewer->_fIsRect = FALSE;
            if (SUCCEEDED(pContextView->GetRangeFromPoint(ec, &pt, 0, &pRange)))
            {     
                if (SUCCEEDED(pContextView->GetTextExt(ec, pRange, &_pRangeFromPointViewer->_rc, &fClipped)))
                {
                    OffsetRect(&_pRangeFromPointViewer->_rc, 0 - rcScreen.left, 0 - rcScreen.top);
                    _pRangeFromPointViewer->_fIsRect = TRUE;
                }
                pRange->Release();
            }

            _pRangeFromPointViewer->_fIsRectRoundNearest = FALSE;
            if (SUCCEEDED(pContextView->GetRangeFromPoint(ec, &pt, GXFPF_ROUND_NEAREST, &pRangeRoundNearest)))
            { 
                if (SUCCEEDED(pContextView->GetTextExt(ec, pRangeRoundNearest, &_pRangeFromPointViewer->_rcRoundNearest, &fClipped)))
                {
                    OffsetRect(&_pRangeFromPointViewer->_rcRoundNearest, 0 - rcScreen.left, 0 - rcScreen.top);
                    _pRangeFromPointViewer->_fIsRectRoundNearest = TRUE;
                }
                pRangeRoundNearest->Release();
            }

            _pRangeFromPointViewer->_fIsRectNearest = FALSE;
            if (SUCCEEDED(pContextView->GetRangeFromPoint(ec, &pt, GXFPF_NEAREST, &pRangeNearest)))
            {     
                if (SUCCEEDED(pContextView->GetTextExt(ec, pRangeNearest, &_pRangeFromPointViewer->_rcNearest, &fClipped)))
                {
                    OffsetRect(&_pRangeFromPointViewer->_rcNearest, 0 - rcScreen.left, 0 - rcScreen.top);
                    _pRangeFromPointViewer->_fIsRectNearest = TRUE;
                }
                pRangeNearest->Release();
            }
        }

        pContextView->Release();
    }


    if (IsShownRangeFromPointViewer())
        _UpdateRangeFromPointViewer();

}


