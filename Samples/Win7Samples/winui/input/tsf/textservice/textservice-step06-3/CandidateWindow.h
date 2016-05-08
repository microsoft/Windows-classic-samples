//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  CandidateWindow.h
//
//          CCandidateWindow declaration.
//
//////////////////////////////////////////////////////////////////////

#ifndef CANDIDATEWINDOW_H
#define CANDIDATEWINDOW_H

//+---------------------------------------------------------------------------
//
// CCandidateWindow
//
//----------------------------------------------------------------------------

class CCandidateWindow 
{
public:
    CCandidateWindow();

    static BOOL _InitWindowClass();
    static void _UninitWindowClass();

    BOOL _Create();
    void _Destroy();

    void _Move(int x, int y);
    void _Show();
    void _Hide();

    HRESULT _OnKeyDown(UINT uVKey);
    HRESULT _OnKeyUp(UINT uVKey);

private:
    static LRESULT CALLBACK _WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    static void _SetThis(HWND hwnd, LPARAM lParam)
    {
        SetWindowLongPtr(hwnd, GWLP_USERDATA, 
                         (LONG_PTR)((CREATESTRUCT *)lParam)->lpCreateParams);
    }

    static CCandidateWindow *_GetThis(HWND hwnd)
    {
        return (CCandidateWindow *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    static ATOM _atomWndClass;

    HWND _hwnd;
};

#endif // CANDIDATEWINDOW_H
