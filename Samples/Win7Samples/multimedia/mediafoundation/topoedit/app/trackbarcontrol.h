// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

typedef void (*HANDLESCROLLPROC)(WORD wPos);

class CTrackbarControl 
    : public CWindowImpl<CTrackbarControl>
{
public:
    CTrackbarControl();
    
    HRESULT Init(HWND hParentWnd, RECT& rect, bool fHoriz, bool fAutoTicks);

    DWORD GetMaxRange() const;

    WORD GetPos();
    
    void SetPos(LONG lPos);
    void SetRange(int minValue, int maxValue);
    void SetScrollCallback(HANDLESCROLLPROC scrollCallback);
    void HandleScroll(WORD wMsg, WORD wPos);
    bool IsTracking();
    
    DECLARE_WND_SUPERCLASS(NULL, TRACKBAR_CLASS)
protected:
    LRESULT OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    
    BEGIN_MSG_MAP(CTrackbarControl)
        MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
        MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
        MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
        MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
        MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
    END_MSG_MAP()

protected:
    bool m_fTracking;
    HANDLESCROLLPROC m_scrollCallback;
};

//////////////////////////////////////////////////////////////////////////
//

class CSeekerTrackbarControl : public CTrackbarControl
{
public:
    CSeekerTrackbarControl();
    
protected:
    LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
private:
    LRESULT m_LastClickPos;
    static const int m_iControlLeftMarginSize = 10;
    static const int m_iControlMarginSize = 20;
};
