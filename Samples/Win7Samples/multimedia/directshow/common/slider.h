//////////////////////////////////////////////////////////////////////////
// Slider.h: Custom slider control.
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

HRESULT Slider_Init();
HRESULT Slider_Create(HWND hParent, const Rect rc, DWORD_PTR id, HWND *pHwnd);

// Messages
const UINT WM_SLIDER_SET_THUMB_BITMAP = WM_USER + 1;		// wparam = resource id
const UINT WM_SLIDER_SET_BACKGROUND = WM_USER + 2;			// wparam = hbrush
const UINT WM_SLIDER_SET_MIN_MAX = WM_USER + 3;				// wparam = min, lparam = max
const UINT WM_SLIDER_GET_POSITION = WM_USER + 4;			// returns position
const UINT WM_SLIDER_SET_POSITION = WM_USER + 5;			// wparam = position

// Notifications

// lparam = LPNMSLIDER_INFO

const int SLIDER_NOTIFY_SELECT = 0;	
const int SLIDER_NOTIFY_RELEASE = 1;	
const int SLIDER_NOTIFY_DRAG = 2;	

// NMSLIDER_INFO:
// Custom notification structure.

typedef struct tag_NMSLIDER_INFO {
    NMHDR hdr;
    BOOL  bSelected;    
	LONG  position;
} NMSLIDER_INFO, *LPNMSLIDER_INFO;



class Slider : public Control
{
public:

	HRESULT Create(HWND hParent, const Rect& rcSize, DWORD_PTR id);
	HRESULT SetThumbBitmap(UINT nId);
	HRESULT SetBackground(HBRUSH hBackground);
    LONG GetPosition() const;
    HRESULT SetPosition(LONG pos);
    HRESULT SetRange(LONG min, LONG max);
};



