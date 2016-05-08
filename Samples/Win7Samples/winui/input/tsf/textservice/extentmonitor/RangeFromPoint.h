//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  PopupWindow.h
//
//          CProeprtyPopupWindow
//
//////////////////////////////////////////////////////////////////////

#ifndef RANGEFROMPOINT_H
#define RANGEFROMPOINT_H


class CExtentMonitorTextService;

class CRangeFromPointViewer
{
public:
    CRangeFromPointViewer(CExtentMonitorTextService *pTextService, WCHAR *psz, COLORREF cr);
    ~CRangeFromPointViewer();

    HWND CreateWnd();
    void Show(RECT *prc);
    void Hide();
    void OnPaint(HWND hwnd, HDC hdc);
    void OnSetCursor(HWND hwnd, WPARAM wParam, LPARAM lParam);
    BOOL IsShown()
    {
        if (!IsWindow(_hwnd))
            return FALSE;

        return IsWindowVisible(_hwnd);
    }

    static BOOL StaticInit();

    static LRESULT CALLBACK _WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    static void _SetThis(HWND hwnd, LPARAM lParam)
    {
        SetWindowLongPtr(hwnd, GWLP_USERDATA, 
                         (LONG_PTR)((CREATESTRUCT *)lParam)->lpCreateParams);
    }

    static CRangeFromPointViewer *_GetThis(HWND hwnd)
    {
        return (CRangeFromPointViewer *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    HWND _hwnd;
    static TCHAR _szWndClass[];

    CExtentMonitorTextService *_pTextService;

    WCHAR _sz[80];
    COLORREF _cr;

    RECT _rc;
    BOOL _fIsRect;
    RECT _rcRoundNearest;
    BOOL _fIsRectRoundNearest;
    RECT _rcNearest;
    BOOL _fIsRectNearest;
};

#endif RANGEFROMPOINT_H
