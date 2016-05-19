// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

#include "splitterbar.h"

#include "tedobj.h"

LPCTSTR CSplitterBar::ms_strNextCursor = NULL;
bool CSplitterBar::ms_fVert = false;

CSplitterBar::CSplitterBar(CDock* pDock, bool vert, HWND parent) 
    : m_pDock(pDock), m_bSelected(false), m_fVert(vert), m_hParent(parent) 
{

    ms_fVert = vert;
}

LRESULT CSplitterBar::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HCURSOR hCursor;
    if(m_fVert) 
    {
        hCursor = LoadCursor(NULL, IDC_SIZEWE);
    }
    else 
    {
        hCursor = LoadCursor(NULL, IDC_SIZENS);
    }
    
    SetClassLongPtr(m_hWnd, GCLP_HCURSOR, (LONG_PTR) hCursor);


    return 0;
}

LRESULT CSplitterBar::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    PAINTSTRUCT ps;

    HDC dc = ::BeginPaint(m_hWnd, &ps);
    
    RECT drawArea;
    GetClientRect(&drawArea);

    HPEN grayPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DFACE));
    HPEN darkPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DDKSHADOW));
    HBRUSH grayBrush = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
    
    if(drawArea.bottom - drawArea.top > drawArea.right - drawArea.left) 
    {
        HPEN oldPen = (HPEN) SelectObject(dc, darkPen);

        MoveToEx(dc, drawArea.left, drawArea.top, NULL);
        LineTo(dc, drawArea.left, drawArea.bottom);

        MoveToEx(dc, drawArea.right, drawArea.top, NULL);
        LineTo(dc, drawArea.right, drawArea.bottom);

        SelectObject(dc, grayPen);

        drawArea.left += 1;
        drawArea.right -= 1;

        FillRect(dc, &drawArea, grayBrush);

        SelectObject(dc, oldPen);
    }
    else 
    {
        HPEN oldPen = (HPEN) SelectObject(dc, darkPen);

        MoveToEx(dc, drawArea.left, drawArea.top, NULL);
        LineTo(dc, drawArea.right, drawArea.top);

        MoveToEx(dc, drawArea.left, drawArea.bottom, NULL);
        LineTo(dc, drawArea.right, drawArea.bottom);

        SelectObject(dc, grayPen);

        drawArea.top += 1;
        drawArea.bottom -= 1;

        FillRect(dc, &drawArea, grayBrush);

        SelectObject(dc, oldPen);
    }

    DeleteObject(grayPen);
    DeleteObject(darkPen);
    DeleteObject(grayBrush);

    ::EndPaint(m_hWnd, &ps);

    return 0;
}

LRESULT CSplitterBar::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if(m_bSelected) 
    {
        m_pDock->MoveSplitter(this, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

        ::SendMessage(m_hParent, WM_SPLITTERSIZE, 0, 0);
    }

    return 0;
}

LRESULT CSplitterBar::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    m_bSelected = true;
    SetCapture();

    return 0;
}

LRESULT CSplitterBar::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    m_bSelected = false;
    ReleaseCapture();

    return 0;
}
