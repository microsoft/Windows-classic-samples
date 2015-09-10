// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "BaseWindow.h"
#include "ScrollBarWindow.h"

//////////////////////////////////////////////////////////////////////
//
// CScrollBarWindowFactory class.
//
//////////////////////////////////////////////////////////////////////
CScrollBarWindowFactory* CScrollBarWindowFactory::_instance;

CScrollBarWindowFactory::CScrollBarWindowFactory()
{
    _instance = nullptr;
}

CScrollBarWindowFactory* CScrollBarWindowFactory::Instance()
{
    if (nullptr == _instance)
    {
        _instance = new (std::nothrow) CScrollBarWindowFactory();
    }

    return _instance;
}

CScrollBarWindow* CScrollBarWindowFactory::MakeScrollBarWindow(SHELL_MODE shellMode)
{
    CScrollBarWindow* pScrollBarWindow = nullptr;

    switch (shellMode)
    {
    case STOREAPP:
        pScrollBarWindow = new (std::nothrow) CScrollBarWindow();
        break;

    case DESKTOP:
        pScrollBarWindow = new (std::nothrow) CScrollBarNullWindow();
        break;

    default:
        pScrollBarWindow = new (std::nothrow) CScrollBarNullWindow();
        break;
    }
    return pScrollBarWindow;
}

void CScrollBarWindowFactory::Release()
{
    if (_instance)
    {
        delete _instance;
        _instance = nullptr;
    }
}

//////////////////////////////////////////////////////////////////////
//
// CScrollBarWindow class.
//
//////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

CScrollBarWindow::CScrollBarWindow()
{
    _pBtnUp = nullptr;
    _pBtnDn = nullptr;

    _scrollDir = SCROLL_NONE_DIR;

    _sizeOfScrollBtn.cx = GetSystemMetrics(SM_CXVSCROLL) * 2;
    _sizeOfScrollBtn.cy = GetSystemMetrics(SM_CYVSCROLL) * 2;
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

CScrollBarWindow::~CScrollBarWindow()
{
    if (_pBtnUp)
    {
        delete _pBtnUp;
        _pBtnUp = nullptr;
    }
    if (_pBtnDn)
    {
        delete _pBtnDn;
        _pBtnDn = nullptr;
    }
}

//+---------------------------------------------------------------------------
//
// _Create
//
//----------------------------------------------------------------------------

BOOL CScrollBarWindow::_Create(ATOM atom, DWORD dwExStyle, DWORD dwStyle, CBaseWindow *pParent, int wndWidth, int wndHeight)
{
    if (!CBaseWindow::_Create(atom, dwExStyle, dwStyle, pParent, wndWidth, wndHeight))
    {
        return FALSE;
    }

    _pBtnUp = new (std::nothrow) CScrollButtonWindow(dwStyle & WS_HSCROLL ? DFCS_SCROLLLEFT : DFCS_SCROLLUP);
    if (_pBtnUp == nullptr)
    {
        return FALSE;
    }

    _pBtnUp->_Create(NULL, 0, 0, this);
    _pBtnUp->_SetUIWnd(_GetUIWnd());

    _pBtnDn = new (std::nothrow) CScrollButtonWindow(dwStyle & WS_HSCROLL ? DFCS_SCROLLRIGHT : DFCS_SCROLLDOWN);
    if (_pBtnDn == nullptr)
    {
        delete _pBtnUp;
        _pBtnUp = nullptr;
        return FALSE;
    }

    _pBtnDn->_Create(NULL, 0, 0, this);
    _pBtnDn->_SetUIWnd(_GetUIWnd());

    return TRUE;
}

//+---------------------------------------------------------------------------
//
// _WindowProcCallback
//
// Scrollbar window proc.
//----------------------------------------------------------------------------

LRESULT CALLBACK CScrollBarWindow::_WindowProcCallback(_In_ HWND wndHandle, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_PAINT:
        {
            HDC dcHandle = nullptr;
            PAINTSTRUCT ps;

            dcHandle = BeginPaint(wndHandle, &ps);

            // paint itself at first
            _OnPaint(dcHandle, &ps);

            // paint all children
            _pBtnUp->_OnPaint(dcHandle, &ps);
            _pBtnDn->_OnPaint(dcHandle, &ps);

            EndPaint(wndHandle, &ps);
        }
        return 0;
    }

    return DefWindowProc(wndHandle, uMsg, wParam, lParam);
}

//+---------------------------------------------------------------------------
//
// _OnPaint
//
//----------------------------------------------------------------------------

void CScrollBarWindow::_OnPaint(_In_ HDC dcHandle, _In_ PAINTSTRUCT *pps)
{
    HBRUSH hBrush = nullptr;
    CBaseWindow* pUIWnd = _GetUIWnd();

    if (pUIWnd != nullptr && pUIWnd->_GetWnd())
    {
        hBrush = (HBRUSH)DefWindowProc(pUIWnd->_GetWnd(), WM_CTLCOLORSCROLLBAR, (WPARAM)dcHandle, (LPARAM)pUIWnd->_GetWnd());
    }

    if (hBrush == nullptr)
    {
        hBrush = GetSysColorBrush(COLOR_SCROLLBAR);
    }

    FillRect(dcHandle, &pps->rcPaint, hBrush);
}

//+---------------------------------------------------------------------------
//
// _OnLButtonDown
//
//----------------------------------------------------------------------------

void CScrollBarWindow::_OnLButtonDown(POINT pt)
{
    _StartCapture();

    RECT rc = {0, 0, 0, 0};

    _pBtnUp->_GetClientRect(&rc);
    if (PtInRect(&rc, pt))
    {
        _pBtnUp->_OnLButtonDown(pt);
    }
    else
    {
        _pBtnDn->_GetClientRect(&rc);
        if (PtInRect(&rc, pt))
        {
            _pBtnDn->_OnLButtonDown(pt);
        }
    }
}

//+---------------------------------------------------------------------------
//
// _OnLButtonUp
//
//----------------------------------------------------------------------------

void CScrollBarWindow::_OnLButtonUp(POINT pt)
{
    if (_IsCapture())
    {
        CBaseWindow* pUIWnd = _GetTopmostUIWnd();
        if (pUIWnd)
        {
            CBaseWindow *pCapture = pUIWnd->_GetCaptureObject();
            if (pCapture && pCapture != this)
            {
                pCapture->_OnLButtonUp(pt);
            }
        }
    }
    else
    {
        RECT rc = {0, 0, 0, 0};

        _pBtnUp->_GetClientRect(&rc);
        if (PtInRect(&rc, pt))
        {
            _pBtnUp->_OnLButtonUp(pt);
        }
        else
        {
            _pBtnDn->_GetClientRect(&rc);
            if (PtInRect(&rc, pt))
            {
                _pBtnDn->_OnLButtonUp(pt);
            }
        }
    }

    if (_IsCapture())
    {
        _EndCapture();
    }
    if (_IsTimer())
    {
        _EndTimer();
    }

    _scrollDir = SCROLL_NONE_DIR;
    _InvalidateRect();
}

//+---------------------------------------------------------------------------
//
// _OnMouseMove
//
//----------------------------------------------------------------------------

void CScrollBarWindow::_OnMouseMove(POINT pt)
{
    RECT rc = {0, 0, 0, 0};

    _pBtnUp->_GetClientRect(&rc);
    if (PtInRect(&rc, pt))
    {
        _pBtnUp->_OnMouseMove(pt);
    }
    else
    {
        _pBtnDn->_GetClientRect(&rc);
        if (PtInRect(&rc, pt))
        {
            _pBtnDn->_OnMouseMove(pt);
        }
    }
}

//+---------------------------------------------------------------------------
//
// _OnOwnerWndMoved
//
//----------------------------------------------------------------------------

void CScrollBarWindow::_OnOwnerWndMoved(BOOL isResized)
{
    isResized;

    if (IsWindow(_GetWnd()) && IsWindowVisible(_GetWnd()))
    {
        _AdjustWindowPos();
    }
}

//+---------------------------------------------------------------------------
//
// _Resize
//
//----------------------------------------------------------------------------

void CScrollBarWindow::_Resize(int x, int y, int cx, int cy)
{
    CBaseWindow::_Resize(x, y, cx, cy);

    RECT rc = {0, 0, 0, 0};

    _GetBtnUpRect(&rc);
    _pBtnUp->_Resize(rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top);

    _GetBtnDnRect(&rc);
    _pBtnDn->_Resize(rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top);
}

//+---------------------------------------------------------------------------
//
// _Show
//
//----------------------------------------------------------------------------

void CScrollBarWindow::_Show(BOOL isShowWnd)
{
    if (_IsWindowVisible() != isShowWnd)
    {
        CBaseWindow::_Show(isShowWnd);

        _pBtnUp->_Show(isShowWnd);
        _pBtnDn->_Show(isShowWnd);
    }
}

//+---------------------------------------------------------------------------
//
// _Enable
//
//----------------------------------------------------------------------------

void CScrollBarWindow::_Enable(BOOL isEnable)
{
    if (_IsEnabled() != isEnable)
    {
        CBaseWindow::_Enable(isEnable);

        _pBtnUp->_Enable(isEnable);
        _pBtnDn->_Enable(isEnable);
    }
}

//+---------------------------------------------------------------------------
//
// _AdjustWindowPos
//
//----------------------------------------------------------------------------

void CScrollBarWindow::_AdjustWindowPos()
{
    if (!IsWindow(_GetWnd()))
    {
        return;
    }

    RECT rc = {0, 0, 0, 0};
    CBaseWindow* pParent = _GetParent();
    if (pParent == nullptr)
    {
        return;
    }

    GetWindowRect(pParent->_GetWnd(), &rc);
    SetWindowPos(_GetWnd(), pParent->_GetWnd(),
        rc.left,
        rc.top,
        rc.right - rc.left,
        rc.bottom - rc.top,
        SWP_NOOWNERZORDER | SWP_NOACTIVATE);
}

//+---------------------------------------------------------------------------
//
// _SetScrollInfo
//
//----------------------------------------------------------------------------

void CScrollBarWindow::_SetScrollInfo(_In_ CScrollInfo *lpsi)
{
    _scrollInfo = *lpsi;

    BOOL isEnable = (_scrollInfo.nMax > _scrollInfo.nPage);

    _Enable(isEnable);

    _scrollDir = SCROLL_NONE_DIR;

    _SetCurPos(_scrollInfo.nPos, -1);
}

//+---------------------------------------------------------------------------
//
// _GetScrollInfo
//
//----------------------------------------------------------------------------

void CScrollBarWindow::_GetScrollInfo(_Out_ CScrollInfo *lpsi)
{
    *lpsi = _scrollInfo;
}

//+---------------------------------------------------------------------------
//
// _GetBtnUpRect
//
//----------------------------------------------------------------------------

BOOL CScrollBarWindow::_GetBtnUpRect(_Out_ RECT *prc)
{
    RECT rc = {0, 0, 0, 0};

    _GetClientRect(&rc);

    if (prc == nullptr)
    {
        return FALSE;
    }

    prc->left = rc.left;
    prc->top = rc.top;
    prc->right = rc.right;
    prc->bottom = rc.top + min(_sizeOfScrollBtn.cy, (rc.bottom - rc.top)/2);

    return TRUE;
}

//+---------------------------------------------------------------------------
//
// _GetBtnDnRect
//
//----------------------------------------------------------------------------

BOOL CScrollBarWindow::_GetBtnDnRect(_Out_ RECT *prc)
{
    RECT rc = {0, 0, 0, 0};

    _GetClientRect(&rc);

    if (prc == nullptr)
    {
        return FALSE;
    }

    prc->left = rc.left;
    prc->top = rc.bottom - min(_sizeOfScrollBtn.cy, (rc.bottom - rc.top)/2);
    prc->right = rc.right;
    prc->bottom = rc.bottom;

    return TRUE;
}

//+---------------------------------------------------------------------------
//
// _GetScrollArea
//
//----------------------------------------------------------------------------

void CScrollBarWindow::_GetScrollArea(_Out_ RECT *prc)
{
    RECT rcBtnUp = {0, 0, 0, 0};
    RECT rcBtnDn = {0, 0, 0, 0};

    _GetBtnUpRect(&rcBtnUp);
    _GetBtnDnRect(&rcBtnDn);

    RECT rc = {0, 0, 0, 0};

    _GetClientRect(&rc);

    if (prc == nullptr)
    {
        return;
    }

    prc->left = rc.left;
    prc->top = rc.top + (rcBtnUp.bottom - rcBtnUp.top);
    prc->right = rc.right;
    prc->bottom = rc.bottom - (rcBtnDn.bottom - rcBtnDn.top);
}

//+---------------------------------------------------------------------------
//
// _SetCurPos
//
// param: dwSB - SB_xxx for WM_VSCROLL or WM_HSCROLL.
//    if -1 is specified, function doesn't send WM_xSCROLL message
//----------------------------------------------------------------------------

void CScrollBarWindow::_SetCurPos(int nPos, int dwSB)
{
    int posMax = (_scrollInfo.nMax <= _scrollInfo.nPage) ? 0 : _scrollInfo.nMax - _scrollInfo.nPage;

    nPos = min(nPos, posMax);
    nPos = max(nPos, 0);

    _scrollInfo.nPos = nPos;

    if (_IsWindowVisible()) {
        _InvalidateRect();
    }

    if ((_IsCapture() && dwSB != -1) || (dwSB == SB_THUMBPOSITION))
    {
        _NotifyCommand(WM_VSCROLL, dwSB, nPos);
    }
}

//////////////////////////////////////////////////////////////////////
//
// CScrollButtonWindow class.
//
//////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
//
// _OnPaint
//
//----------------------------------------------------------------------------

void CScrollButtonWindow::_OnPaint(_In_ HDC dcHandle, _In_ PAINTSTRUCT *pps)
{
    pps;

    RECT rc = {0, 0, 0, 0};

    _GetClientRect(&rc);

    DrawFrameControl(dcHandle, &rc, DFC_SCROLL, subTypeOfControl | typeOfControl | (!_IsEnabled() ? DFCS_INACTIVE : 0));
}

//+---------------------------------------------------------------------------
//
// _OnLButtonDown
//
//----------------------------------------------------------------------------

void CScrollButtonWindow::_OnLButtonDown(POINT pt)
{
    CButtonWindow::_OnLButtonDown(pt);

    CScrollBarWindow* pParent = (CScrollBarWindow*)_GetParent();    // more secure w/ dynamic_cast
    if (pParent == nullptr)
    {
        return;
    }

    switch (subTypeOfControl)
    {
    case DFCS_SCROLLDOWN:
        _NotifyCommand(WM_VSCROLL, SB_LINEDOWN, 0);
        pParent->_ShiftLine(+1, FALSE);
        break;
    case DFCS_SCROLLUP:
        _NotifyCommand(WM_VSCROLL, SB_LINEUP, 0);
        pParent->_ShiftLine(-1, FALSE);
        break;
    }

    _StartTimer(_GetScrollDelay());
}

//+---------------------------------------------------------------------------
//
// _OnLButtonUp
//
//----------------------------------------------------------------------------

void CScrollButtonWindow::_OnLButtonUp(POINT pt)
{
    CButtonWindow::_OnLButtonUp(pt);

    _InvalidateRect();

    if (_IsTimer())
    {
        _EndTimer();
    }
}

//+---------------------------------------------------------------------------
//
// _OnTimer : Speed up page Down/Up while holding Down/Up Button
//
//----------------------------------------------------------------------------

void CScrollButtonWindow::_OnTimer()
{
    POINT pt = {0, 0};
    CScrollBarWindow* pParent = (CScrollBarWindow*)_GetParent();    // more secure w/ dynamic_cast
    if (pParent == nullptr)
    {
        return;
    }

    // start another faster timer
    _StartTimer(_GetScrollSpeed());

    GetCursorPos(&pt);
    MapWindowPoints(HWND_DESKTOP, pParent->_GetWnd(), &pt, 1);

    RECT rc = {0, 0, 0, 0};

    _GetClientRect(&rc);

    if (PtInRect(&rc, pt))
    {
        switch (subTypeOfControl)
        {
        case DFCS_SCROLLDOWN:
            _NotifyCommand(WM_VSCROLL, SB_LINEDOWN, 0);
            pParent->_ShiftLine(+1, FALSE);
            break;
        case DFCS_SCROLLUP:
            _NotifyCommand(WM_VSCROLL, SB_LINEUP, 0);
            pParent->_ShiftLine(-1, FALSE);
            break;
        }
    }
}