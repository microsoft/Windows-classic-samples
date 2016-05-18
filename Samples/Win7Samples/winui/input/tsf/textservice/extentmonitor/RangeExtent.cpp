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
//          CRangeExtentViewer
//
//////////////////////////////////////////////////////////////////////

#include "Globals.h"
#include "TextService.h"
#include "RangeExtent.h"

TCHAR CRangeExtentViewer::_szWndClass[] = TEXT("RangeExtentViewer");

//+---------------------------------------------------------------------------
//
// CRangeExtentViewer
//
//----------------------------------------------------------------------------

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

CRangeExtentViewer::CRangeExtentViewer(WCHAR *psz, COLORREF cr)
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

CRangeExtentViewer::~CRangeExtentViewer()
{
    if (IsWindow(_hwnd))
        DestroyWindow(_hwnd);
}


//+---------------------------------------------------------------------------
//
// StaticInit
//
//----------------------------------------------------------------------------

BOOL CRangeExtentViewer::StaticInit()
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

HWND CRangeExtentViewer::CreateWnd()
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

LRESULT CALLBACK CRangeExtentViewer::_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CRangeExtentViewer *_this;
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

void CRangeExtentViewer::Show(RECT *prc)
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

void CRangeExtentViewer::Hide()
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

void CRangeExtentViewer::OnPaint(HWND hwnd, HDC hdc)
{
    HFONT hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    HFONT hfontOld = (HFONT)SelectObject(hdc, hfont);

    RECT rc;
    GetClientRect(hwnd, &rc);
    SetBkColor(hdc, _cr);
    // ExtTextOutW(hdc, 2, 2, ETO_OPAQUE, &rc, _sz, lstrlenW(_sz), NULL);
    ExtTextOutW(hdc, 2, 2, ETO_OPAQUE, &rc, _sz, 0, NULL);

    for (int i = 0; i < ARRAYSIZE(_rcRanges); i++)
    {
        FrameRect(hdc, &_rcRanges[i], (HBRUSH)GetStockObject(BLACK_BRUSH));
    }
    SelectObject(hdc, hfontOld);
}

//+---------------------------------------------------------------------------
//
// _EnsurePopupWindow
//
//----------------------------------------------------------------------------

void CExtentMonitorTextService::_EnsureRangeExtentViewer()
{
    if (!_pRangeExtentViewer)
         _pRangeExtentViewer = new CRangeExtentViewer(L"View", 0x00D0D0);
    if (_pRangeExtentViewer)
        _pRangeExtentViewer->CreateWnd();
}

//+---------------------------------------------------------------------------
//
// _UpdatePopupWindow
//
//----------------------------------------------------------------------------

void CExtentMonitorTextService::_UpdateRangeExtentViewer()
{
    if (_pRangeExtentViewer)
    {
        _pRangeExtentViewer->Show(&_rcView);
        memcpy(_pRangeExtentViewer->_rcRanges, _rcRanges, sizeof(_rcRanges));
    }

    _fIsShownRangeExtentViewer = TRUE;
}

//+---------------------------------------------------------------------------
//
// _HidePopupWindow
//
//----------------------------------------------------------------------------

void CExtentMonitorTextService::_HideRangeExtentViewer()
{
    if (_pRangeExtentViewer)
        _pRangeExtentViewer->Hide();

    _fIsShownRangeExtentViewer = FALSE;
}
