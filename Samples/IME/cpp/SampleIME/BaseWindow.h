// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#pragma once

#define RECT_INSIDE      (0x0001)
#define RECT_OVER_LEFT   (0x0002)
#define RECT_OVER_TOP    (0x0004)
#define RECT_OVER_RIGHT  (0x0008)
#define RECT_OVER_BOTTOM (0x0010)
#define RECT_TOO_WIDE    (0x0020)
#define RECT_TOO_TALL    (0x0040)
#define RECT_ERROR       (0x0080)

class CStringRange;

//+---------------------------------------------------------------------------
//
// CBaseWindow
//
//----------------------------------------------------------------------------

class CBaseWindow
{
public:
    CBaseWindow();
    virtual ~CBaseWindow();

    static BOOL _InitWindowClass(_In_ LPCWSTR lpwszClassName, _Out_ ATOM *patom);
    static void _UninitWindowClass(ATOM atom);

    virtual BOOL _Create(ATOM atom, DWORD dwExStyle, DWORD dwStyle, _In_opt_ CBaseWindow *pParentWnd = nullptr, int wndWidth = 0, int wndHeight = 0, _In_opt_ HWND parentWndHandle = nullptr);
    virtual void _Destroy();

    virtual void _Move(int x, int y);
    virtual void _Resize(int x, int y, int cx, int cy);
    virtual void _Show(BOOL isShowWnd);
    virtual BOOL _IsWindowVisible();
    virtual void _Enable(BOOL enableWindowReceiveInput);
    virtual BOOL _IsEnabled();
    virtual void _InvalidateRect();
    virtual BOOL _GetWindowRect(_Inout_ LPRECT lpRect);
    virtual BOOL _GetClientRect(_Inout_ LPRECT lpRect);

    virtual LRESULT CALLBACK _WindowProcCallback(_In_ HWND wndHandle, UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) = 0;
    virtual void _OnPaint(_In_ HDC dcHandle, _In_ PAINTSTRUCT *pps) { dcHandle; pps; }
    virtual void _OnLButtonDown(POINT pt) { pt; }
    virtual void _OnLButtonUp(POINT pt) { pt; }
    virtual void _OnMouseMove(POINT pt) { pt; }
    virtual void _OnTimer() { }

    CBaseWindow* _GetTopmostUIWnd();

    HRESULT _GetWindowExtent(_In_ const RECT *prcTextExtent, _In_opt_ RECT *prcCandidateExtent, _Inout_ POINT *pptCandidate);

    HWND _GetWnd() 
    { 
        return _wndHandle; 
    }

    CBaseWindow *_GetParent() 
    { 
        return _pParentWnd; 
    }

    void _SetUIWnd(_In_ CBaseWindow *pUIWnd) 
    { 
        _pUIWnd = pUIWnd; 
    }

    CBaseWindow* _GetUIWnd() 
    { 
        return _pUIWnd; 
    }

    CBaseWindow *_GetCaptureObject() 
    { 
        return _pUIObjCapture; 
    }

    CBaseWindow *_GetTimerObject()   
    { 
        return _pTimerUIObj; 
    }

    UINT _GetScrollDelay()
    {
        return (GetDoubleClickTime() * 4 / 5);
    }

    UINT _GetScrollSpeed()
    {
        return (_GetScrollDelay() / 8);
    }

protected:
    LRESULT _NotifyCommand(UINT uMsg, DWORD dwSB, int nPos);

    void _StartCapture() 
    { 
        _SetCaptureObject(this); 
    }

    void _EndCapture()   
    { 
        _SetCaptureObject(nullptr); 
    }

    BOOL _IsCapture();

    void _StartTimer(UINT uElapse) 
    { 
        _SetTimerObject(this, uElapse); 
    }

    void _EndTimer()    
    { 
        _SetTimerObject(nullptr); 
    }

    BOOL _IsTimer();

private:
    void _SetCaptureObject(_In_opt_ CBaseWindow *pUIObj);
    void _SetTimerObject(_In_opt_ CBaseWindow *pUIObj, UINT uElapse = 0);

    void GetWorkAreaFromPoint(_In_ const POINT& ptPoint, _Out_ LPRECT lprcWorkArea);
    void CalcFitPointAroundTextExtent(_In_ const RECT *prcTextExtent, _In_ const RECT *prcWorkArea, _In_ const RECT *prcWindow, _Out_ POINT *ppt);
    DWORD RectInRect(_In_ const RECT *prcLimit, _In_ const RECT *prcTarget);

    static LRESULT CALLBACK _WindowProc(_In_ HWND wndHandle, UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

    static CBaseWindow *_GetThis(_In_ HWND wndHandle)
    {
        return (CBaseWindow *)GetWindowLongPtr(wndHandle, GWLP_USERDATA);
    }

    static void _SetThis(_In_ HWND wndHandle, _In_opt_ void *lpv)
    {
        SetWindowLongPtr(wndHandle, GWLP_USERDATA, (LONG_PTR)lpv);
    }

private:
    HWND _wndHandle;

    CBaseWindow *_pParentWnd;
    CBaseWindow *_pUIWnd;

    CBaseWindow *_pTimerUIObj;
    CBaseWindow *_pUIObjCapture;

    BOOL _enableVirtualWnd;
    BOOL _visibleVirtualWnd;
    RECT _RectOfVirtualWnd;
};
