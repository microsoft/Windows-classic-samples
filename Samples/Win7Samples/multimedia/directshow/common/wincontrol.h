//////////////////////////////////////////////////////////////////////////
// WinControl.h: Base control class
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <commctrl.h>
#include <assert.h>
#include "utils.h"

struct Rect : RECT
{
	Rect()
	{
		SetEmpty();
	}
	Rect(LONG left, LONG top, LONG right, LONG bottom)
	{
		this->left = left;
		this->top = top;
		this->right = right;
		this->bottom = bottom;
	}

	BOOL IsEmpty() const { return IsRectEmpty(this); }
	BOOL SetEmpty() { return SetRectEmpty(this); }
	BOOL Set(LONG left, LONG top, LONG right, LONG bottom)
	{
		return SetRect(this, left, top, right, bottom);
	}

	BOOL PointInRect(LONG x, LONG y)
	{
		POINT pt = { x, y };
		return PtInRect(this, pt);
	}
	

	LONG Width() const { return right - left; }
	LONG Height() const { return bottom - top; }
};

struct Size :  SIZE
{
	Size()
	{
		this->cx = 0;
		this->cy = 0;
	}
	Size(LONG x, LONG y)
	{
		this->cx = x;
		this->cy = y; 
	}
};

class Control
{
public:
	Control() : m_hwnd(0) {}
    
	void SetWindow(HWND hwnd) { m_hwnd = hwnd; }
    HWND Window() const { return m_hwnd; }

	void Enable(BOOL bEnable) 
    { 
        if (!bEnable &&  m_hwnd == GetFocus())
        {
            // If we're being disabled and this control has focus,
            // set the focus to the next control.

            ::SendMessage(GetParent(m_hwnd), WM_NEXTDLGCTL, 0, FALSE);
        }

        EnableWindow(m_hwnd, bEnable); 
    }

    // SendMessage: Send a message to the control.
    LRESULT SendMessage(UINT msg, WPARAM wParam, LPARAM lParam) const
    {
        assert(m_hwnd);
        return ::SendMessage(m_hwnd, msg, wParam, lParam);
    }

    bool HasStyle(LONG style) const
    {
        return (GetWindowLong(m_hwnd, GWL_STYLE) & style) == style;
    }

    LONG AddStyle(LONG style)
    {
        LONG old_style = GetWindowLong(m_hwnd, GWL_STYLE);
        return SetWindowLong(m_hwnd, GWL_STYLE,  old_style | style);
    }

    LONG RemoveStyle(LONG style)
    {
        LONG old_style = GetWindowLong(m_hwnd, GWL_STYLE);
        old_style = old_style & (~style);
        return SetWindowLong(m_hwnd, GWL_STYLE, old_style);
    }

    LONG EnableStyle(bool bEnable, LONG style)
    {
        return (bEnable ? AddStyle(style) : RemoveStyle(style));
    }

protected:
	HRESULT Create(const CREATESTRUCT& create);
    HWND m_hwnd;
};


// CreateStruct: Thin wrapper for CREATESTRUCT structure.
class CreateStruct : public CREATESTRUCT
{
public:
	CreateStruct();
	void SetBoundingRect(const Rect& rc);
};


HINSTANCE	GetInstance();
HBITMAP		SetBitmapImg(HINSTANCE hinst, WORD nID, HWND hwnd);

#include "button.h"
#include "combobox.h"
#include "listbox.h"
#include "slider.h"
#include "toolbar.h"
#include "BaseWindow.h"
#include "Dialog.h"