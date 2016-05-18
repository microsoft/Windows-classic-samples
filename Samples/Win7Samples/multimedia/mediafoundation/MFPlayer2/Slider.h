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


// GOALS of the slider class:
// 
// - Application can get/set the logical position.
// - When the usser clicks on the client area, the slider jumps to the clicked position.
//   (This behavior is different from the slider control in the Windows common controls,
//    and is more like the behavior of the seek bar in Windows Media Player.)

// Window messages:

// set thumb position (the "thumb" is the part of the slider that the user drags)
// get thumb position
// set background brush
// set thumb image

// Window notifications:

// User clicked on thumb
// User dragged thumb
// User click on non-thumb

// data:
// image of thumb


#pragma once

HRESULT Slider_Init();

// Messages
const UINT WM_SLIDER_SET_THUMB_BITMAP = WM_USER + 1;        // wparam = resource id
const UINT WM_SLIDER_SET_BACKGROUND = WM_USER + 2;          // wparam = hbrush
const UINT WM_SLIDER_SET_MIN_MAX = WM_USER + 3;             // wparam = min, lparam = max
const UINT WM_SLIDER_GET_POSITION = WM_USER + 4;            // returns position
const UINT WM_SLIDER_SET_POSITION = WM_USER + 5;            // wparam = position

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


inline BOOL Slider_SetBackground(HWND hwnd, HBRUSH hBackground)
{
    return (BOOL)SendMessage(hwnd, WM_SLIDER_SET_BACKGROUND, (WPARAM)hBackground, 0);
}

inline BOOL Slider_SetThumbBitmap(HWND hwnd, UINT nId)
{
    return (BOOL)SendMessage(hwnd, WM_SLIDER_SET_THUMB_BITMAP, nId, 0);
}

inline BOOL Slider_SetPosition(HWND hwnd, LONG pos)
{
    return (BOOL)SendMessage(hwnd, WM_SLIDER_SET_POSITION, pos, 0);
}

inline BOOL Slider_SetRange(HWND hwnd, LONG min, LONG max)
{
    return (BOOL)SendMessage(hwnd, WM_SLIDER_SET_MIN_MAX, min, max);
}



