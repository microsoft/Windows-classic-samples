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
//          CExtentVisualWindow
//
//////////////////////////////////////////////////////////////////////

#include "Globals.h"
#include "TextService.h"
#include "ExtentVisual.h"

TCHAR CExtentVisualWindow::_szWndClass[] = TEXT("ExtentVisualWindow");

//+---------------------------------------------------------------------------
//
// CExtentVisualWindow
//
//----------------------------------------------------------------------------

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

CExtentVisualWindow::CExtentVisualWindow(WCHAR *psz, COLORREF cr)
{
    _hwnd = NULL;
    StringCchCopy(_sz, ARRAYSIZE(_sz), psz);
    _cr = cr;
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

CExtentVisualWindow::~CExtentVisualWindow()
{
    if (IsWindow(_hwnd))
        DestroyWindow(_hwnd);
}


//+---------------------------------------------------------------------------
//
// StaticInit
//
//----------------------------------------------------------------------------

BOOL CExtentVisualWindow::StaticInit()
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

HWND CExtentVisualWindow::CreateWnd()
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

LRESULT CALLBACK CExtentVisualWindow::_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CExtentVisualWindow *_this;
    HDC hdc;
    PAINTSTRUCT ps;
  
    _this = _GetThis(hwnd);

    switch (uMsg)
    {
        case WM_CREATE:
            _SetThis(hwnd, lParam);
            return 0;

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

void CExtentVisualWindow::Show(RECT *prc)
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

void CExtentVisualWindow::Hide()
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

void CExtentVisualWindow::OnPaint(HWND hwnd, HDC hdc)
{
    HFONT hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    HFONT hfontOld = (HFONT)SelectObject(hdc, hfont);

    RECT rc;
    GetClientRect(hwnd, &rc);
    SetBkColor(hdc, _cr);
    // ExtTextOutW(hdc, 2, 2, ETO_OPAQUE, &rc, _sz, lstrlenW(_sz), NULL);
    ExtTextOutW(hdc, 2, 2, ETO_OPAQUE, &rc, _sz, 0, NULL);

    SelectObject(hdc, hfontOld);
}

//+---------------------------------------------------------------------------
//
// _EnsurePopupWindow
//
//----------------------------------------------------------------------------

void CExtentMonitorTextService::_EnsureExtentVisualWindows()
{
    if (!_pExtentVisualWinodowStartPos)
         _pExtentVisualWinodowStartPos = new CExtentVisualWindow(L"Start", 0x0000FF);
    if (_pExtentVisualWinodowStartPos)
        _pExtentVisualWinodowStartPos->CreateWnd();

    if (!_pExtentVisualWinodowEndPos)
         _pExtentVisualWinodowEndPos = new CExtentVisualWindow(L"End", 0xFF0000);
    if (_pExtentVisualWinodowEndPos)
        _pExtentVisualWinodowEndPos->CreateWnd();

    if (!_pExtentVisualWinodowSelection)
         _pExtentVisualWinodowSelection = new CExtentVisualWindow(L"Selection", 0x00FF00);
    if (_pExtentVisualWinodowSelection)
        _pExtentVisualWinodowSelection->CreateWnd();
}

//+---------------------------------------------------------------------------
//
// _UpdatePopupWindow
//
//----------------------------------------------------------------------------

void CExtentMonitorTextService::_UpdateExtentVisualWindows()
{
    if (_pExtentVisualWinodowStartPos)
        _pExtentVisualWinodowStartPos->Show(&_rcStartPos);

    if (_pExtentVisualWinodowEndPos)
        _pExtentVisualWinodowEndPos->Show(&_rcEndPos);

    if (_pExtentVisualWinodowSelection)
        _pExtentVisualWinodowSelection->Show(&_rcSelection);

    _fIsShownExtentVisualWindows = TRUE;
}

//+---------------------------------------------------------------------------
//
// _HidePopupWindow
//
//----------------------------------------------------------------------------

void CExtentMonitorTextService::_HideExtentVisualWindows()
{
    if (_pExtentVisualWinodowStartPos)
        _pExtentVisualWinodowStartPos->Hide();

    if (_pExtentVisualWinodowEndPos)
        _pExtentVisualWinodowEndPos->Hide();

    if (_pExtentVisualWinodowSelection)
        _pExtentVisualWinodowSelection->Hide();

    _fIsShownExtentVisualWindows = FALSE;
}
