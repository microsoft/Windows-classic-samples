//////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2005  Microsoft Corporation.  All rights reserved.
//
//  ScrollBarWindow.h
//
// CScrollBarWindow declaration.
//
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "BaseWindow.h"
#include "ButtonWindow.h"

class CScrollButtonWindow;

enum SCROLLBAR_DIRECTION
{
    SCROLL_NONE_DIR,
    SCROLL_PAGEDOWN,    // or page left
    SCROLL_PAGEUP       // or page right
};

enum SHELL_MODE
{
    STOREAPP,
    DESKTOP,
};

struct CScrollInfo
{
    CScrollInfo()
    {
        nMax = 0;
        nPage = 0;
        nPos = 0;
    }

    int  nMax;
    int  nPage;
    int  nPos;
};

class CScrollBarWindow;

class CScrollBarWindowFactory
{
public:
    static CScrollBarWindowFactory* Instance();
    CScrollBarWindow* MakeScrollBarWindow(SHELL_MODE shellMode);
    void Release();

protected:
    CScrollBarWindowFactory();

private:
    static CScrollBarWindowFactory* _instance;

};
//////////////////////////////////////////////////////////////////////
//
// CScrollBarWindow declaration.
//
//////////////////////////////////////////////////////////////////////

class CScrollBarWindow : public CBaseWindow
{
public:
    CScrollBarWindow();
    virtual ~CScrollBarWindow();

    BOOL _Create(ATOM atom, DWORD dwExStyle, DWORD dwStyle, CBaseWindow *pParent = nullptr, int wndWidth = 0, int wndHeight = 0);

    void _Resize(int x, int y, int cx, int cy);
    void _Show(BOOL isShowWnd);
    void _Enable(BOOL isEnable);

    LRESULT CALLBACK _WindowProcCallback(_In_ HWND wndHandle, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
    virtual void _OnPaint(_In_ HDC dcHandle, _In_ PAINTSTRUCT *pps);
    virtual void _OnLButtonDown(POINT pt);
    virtual void _OnLButtonUp(POINT pt);
    virtual void _OnMouseMove(POINT pt);
    virtual void _OnTimer() { };

    virtual void _OnOwnerWndMoved(BOOL isResized);

    virtual void _SetScrollInfo(_In_ CScrollInfo *lpsi);
    virtual void _GetScrollInfo(_Out_ CScrollInfo *lpsi);

    virtual BOOL _GetBtnUpRect(_Out_ RECT *prc);
    virtual BOOL _GetBtnDnRect(_Out_ RECT *prc);

    virtual void _ShiftLine(int nLine, BOOL isNotify)
    {
        DWORD dwSB = 0;

        dwSB = (nLine > 0 ? SB_LINEDOWN :
            nLine < 0 ? SB_LINEUP : 0);

        _SetCurPos(_scrollInfo.nPos + nLine, isNotify ? dwSB : -1);
    }

    virtual void _ShiftPage(int nPage, BOOL isNotify)
    {
        DWORD dwSB = 0;

        dwSB = (nPage > 0 ? SB_PAGEDOWN :
            nPage < 0 ? SB_PAGEUP : 0);

        _SetCurPos(_scrollInfo.nPos + _scrollInfo.nPage * nPage, isNotify ? dwSB : -1);
    }

    virtual void _ShiftPosition(int iPos, BOOL isNotify)
    {
        _SetCurPos(iPos, isNotify ? SB_THUMBPOSITION : -1);
    }

    virtual void _GetScrollArea(_Out_ RECT *prc);
    virtual void _SetCurPos(int nPos, int dwSB);

private:
    void _AdjustWindowPos();

    CScrollButtonWindow* _pBtnUp;
    CScrollButtonWindow* _pBtnDn;

    CScrollInfo _scrollInfo;
    SIZE _sizeOfScrollBtn;
    SCROLLBAR_DIRECTION _scrollDir;
};


//////////////////////////////////////////////////////////////////////
//
// CScrollBarNullWindow declaration.
//
//////////////////////////////////////////////////////////////////////

class CScrollBarNullWindow : public CScrollBarWindow
{
public:
    CScrollBarNullWindow() { };
    virtual ~CScrollBarNullWindow() { };

    virtual BOOL _Create(ATOM atom, DWORD dwExStyle, DWORD dwStyle, _In_opt_ CBaseWindow *pParent = nullptr, int wndWidth = 0, int wndHeight = 0)
    { 
        atom; dwExStyle; dwStyle; pParent; wndWidth; wndHeight;
        return TRUE; 
    };

    virtual void _Destroy() { };

    virtual void _Move(int x, int y) { x;y; };
    virtual void _Resize(int x, int y, int cx, int cy) {  x; y; cx; cy; };
    virtual void _Show(BOOL isShowWnd) { isShowWnd; };
    virtual BOOL _IsWindowVisible() { return TRUE; };
    virtual void _Enable(BOOL isEnable) { isEnable; };
    virtual BOOL _IsEnabled() { return TRUE; };
    virtual void _InvalidateRect() { };
    virtual BOOL _GetWindowRect(_Out_ LPRECT lpRect) { lpRect->bottom = 0; lpRect->left = 0; lpRect->right = 0; lpRect->top = 0; return TRUE; };
    virtual BOOL _GetClientRect(_Out_ LPRECT lpRect) { lpRect->bottom = 0; lpRect->left = 0; lpRect->right = 0; lpRect->top = 0; return TRUE; };

    virtual LRESULT CALLBACK _WindowProcCallback(_In_ HWND wndHandle, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) 
    { 
        wndHandle; uMsg; wParam; lParam;
        return 0; 
    };
    virtual void _OnPaint(_In_ HDC dcHandle, _In_ PAINTSTRUCT *pps) { dcHandle;pps; };
    virtual void _OnLButtonDown(POINT pt) { pt; };
    virtual void _OnLButtonUp(POINT pt) { pt; };
    virtual void _OnMouseMove(POINT pt) { pt; };
    virtual void _OnTimer() { };

    virtual void _OnOwnerWndMoved(BOOL isResized) { isResized; };

    virtual void _SetScrollInfo(_In_ CScrollInfo *lpsi) { lpsi; };
    virtual void _GetScrollInfo(_In_ CScrollInfo *lpsi) { lpsi; };

    virtual BOOL _GetBtnUpRect(_Out_ RECT *prc) { prc->bottom = 0; prc->left = 0; prc->right = 0; prc->top = 0; return TRUE; };
    virtual BOOL _GetBtnDnRect(_Out_ RECT *prc) { prc->bottom = 0; prc->left = 0; prc->right = 0; prc->top = 0; return TRUE; };

    virtual void _ShiftLine(int nLine, BOOL isNotify) { nLine; isNotify; };
    virtual void _ShiftPage(int nPage, BOOL isNotify) { nPage; isNotify; };
    virtual void _ShiftPosition(int iPos, BOOL isNotify) { iPos; isNotify; };
    virtual void _GetScrollArea(_Out_ RECT *prc) { prc->bottom = 0; prc->left = 0; prc->right = 0; prc->top = 0; };
    virtual void _SetCurPos(int nPos, DWORD dwSB) { nPos; dwSB; };
};

//////////////////////////////////////////////////////////////////////
//
// CScrollButtonWondow declaration.
//
//////////////////////////////////////////////////////////////////////

class CScrollButtonWindow : public CButtonWindow
{
public:
    CScrollButtonWindow(UINT uSubType)
    {
        subTypeOfControl = uSubType;
    }
    virtual ~CScrollButtonWindow()
    {
    }

    LRESULT CALLBACK _WindowProcCallback(_In_ HWND wndHandle, UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) 
    { 
        wndHandle;uMsg;wParam;lParam;
        return 0; 
    }
    virtual void _OnPaint(_In_ HDC dcHandle, _In_ PAINTSTRUCT *pps);
    virtual void _OnLButtonDown(POINT pt);
    virtual void _OnLButtonUp(POINT pt);
    virtual void _OnTimer();

private:
    UINT subTypeOfControl;        // DFCS_SCROLLxxx for DrawFrameControl
};
