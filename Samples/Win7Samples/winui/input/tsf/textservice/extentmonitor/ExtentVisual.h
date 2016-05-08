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

#ifndef EXTENTVISUALWINDOW_H
#define EXTENTVISUALWINDOW_H


class CExtentVisualWindow
{
public:
    CExtentVisualWindow(WCHAR *psz, COLORREF cr);
    ~CExtentVisualWindow();

    HWND CreateWnd();
    void Show(RECT *prc);
    void Hide();
    void OnPaint(HWND hwnd, HDC hdc);
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

    static CExtentVisualWindow *_GetThis(HWND hwnd)
    {
        return (CExtentVisualWindow *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    HWND _hwnd;
    static TCHAR _szWndClass[];

    WCHAR _sz[80];
    COLORREF _cr;
};

#endif EXTENTVISUALWINDOW_H
