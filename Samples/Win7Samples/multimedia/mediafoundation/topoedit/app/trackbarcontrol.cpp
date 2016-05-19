// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

#include "trackbarcontrol.h"

#include <commctrl.h>
#include <assert.h>

CTrackbarControl::CTrackbarControl() 
    : m_scrollCallback(NULL)
    , m_fTracking(false)
{
}

HRESULT CTrackbarControl::Init(HWND hParentWnd, RECT& rect, bool fHoriz, bool fAutoTicks) 
{
    HRESULT hr = S_OK;

    DWORD style = WS_CHILD | WS_VISIBLE;
    if(fHoriz) 
    {
        style |= TBS_HORZ;
    }
    else 
    {
        style |= TBS_VERT;
    }

    if(fAutoTicks) 
    {
        style |= TBS_AUTOTICKS;
    }
    
    if(Create(hParentWnd, _U_RECT(rect), LoadAtlString(IDS_SLIDER), style, 0, 0U, NULL) == NULL) 
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }

    SendMessage(m_hWnd, TBM_SETPAGESIZE, 0, (LPARAM) 1);                  
Cleanup:
    return hr;
}

DWORD CTrackbarControl::GetMaxRange() const 
{
    return (DWORD) SendMessage(m_hWnd, TBM_GETRANGEMAX, 0, 0);
}

WORD CTrackbarControl::GetPos() 
{
    return (WORD) SendMessage(m_hWnd, TBM_GETPOS, 0, 0);
}

void CTrackbarControl::SetPos(LONG lPos) 
{
    SendMessage(m_hWnd, TBM_SETPOS, TRUE, lPos);
}

void CTrackbarControl::SetRange(int minValue, int maxValue)
{
    SendMessage(TBM_SETRANGE, (WPARAM) TRUE, MAKELONG(minValue, maxValue)); 
}

void CTrackbarControl::SetScrollCallback(HANDLESCROLLPROC scrollCallback)
{
    m_scrollCallback = scrollCallback;
}

bool CTrackbarControl::IsTracking()
{
    return m_fTracking;
}

LRESULT CTrackbarControl::OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
{
    HandleScroll(LOWORD(wParam), HIWORD(wParam));

    return 0;
}

LRESULT CTrackbarControl::OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
{
    HandleScroll(LOWORD(wParam), HIWORD(wParam));

    return 0;  
}

LRESULT CTrackbarControl::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
{
    bHandled= false;
    return 0;
}

LRESULT CTrackbarControl::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
{
   bHandled = false;
   return 0;
}

LRESULT CTrackbarControl::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
{
    bHandled = false;
    return 0;
}

void CTrackbarControl::HandleScroll(WORD wMsg, WORD wPos) {
    switch(wMsg)
    {
    case TB_PAGEDOWN: // fallthrough
    case TB_PAGEUP: 
    case TB_LINEDOWN:
    case TB_LINEUP:
    {
        LRESULT pos = SendMessage(m_hWnd, TBM_GETPOS, 0, 0);
        if(m_scrollCallback) m_scrollCallback((WORD) pos);
        break;
    }
    case TB_THUMBTRACK:
        m_fTracking = true;
        break;
    case TB_THUMBPOSITION:
        m_fTracking = false;
        if(m_scrollCallback) m_scrollCallback(wPos);
        break;
    }
}

/////////////////////////////////////////////////////////////////
//

CSeekerTrackbarControl::CSeekerTrackbarControl()
    : CTrackbarControl()
    , m_LastClickPos(0)
{
}

LRESULT CSeekerTrackbarControl::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
{
    POINT pt;
    pt.x = GET_X_LPARAM(lParam); 
    pt.y = GET_Y_LPARAM(lParam); 

    pt.x -= 10;
    
    if(pt.x < 0) return 0;

    RECT rect;
    GetClientRect(&rect);
    
    if(pt.x > rect.right - rect.left - 20) return 0;

    LRESULT pos = SendMessage(m_hWnd, TBM_GETRANGEMAX, 0, 0);

    m_LastClickPos = pos * pt.x / (rect.right - rect.left - 20);

    SetCapture();
    m_fTracking = true;
    return 0;
}

LRESULT CSeekerTrackbarControl::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
{
    ReleaseCapture();
    m_fTracking = false;
    
    POINT pt;
    pt.x = GET_X_LPARAM(lParam); 
    pt.y = GET_Y_LPARAM(lParam); 

    pt.x -= m_iControlLeftMarginSize;
    if(pt.x < 0) return 0;

    RECT rect;
    GetClientRect(&rect);

    if(pt.x > rect.right - rect.left - m_iControlMarginSize) return 0;
    
    LRESULT pos = SendMessage(m_hWnd, TBM_GETRANGEMAX, 0, 0);

    m_LastClickPos = pos * pt.x / (rect.right - rect.left - m_iControlMarginSize);

    SendMessage(m_hWnd, TBM_SETPOS, TRUE, m_LastClickPos);
    if(m_scrollCallback) m_scrollCallback((WORD) m_LastClickPos);
    
   return 0;
}

LRESULT CSeekerTrackbarControl::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
{
    if(m_fTracking)
    {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam); 
        pt.y = GET_Y_LPARAM(lParam); 

        pt.x -= m_iControlLeftMarginSize;
        
        if(pt.x < 0) return 0;
        
        RECT rect;
        GetClientRect(&rect);

        LRESULT pos = SendMessage(m_hWnd, TBM_GETRANGEMAX, 0, 0);

        m_LastClickPos = pos * pt.x / (rect.right - rect.left - m_iControlMarginSize);

        SendMessage(m_hWnd, TBM_SETPOS, TRUE, m_LastClickPos);

    }
    
    return 0;
}