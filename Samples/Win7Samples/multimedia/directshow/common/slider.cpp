//////////////////////////////////////////////////////////////////////////
// Slider.cpp: Custom slider control.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#include "wincontrol.h"
#include "Slider.h"

#include <windowsx.h>

namespace SliderControl
{

	LPCTSTR ClassName = TEXT("SLIDER_CLASS");

	LPCTSTR InstanceData = TEXT("SLIDER_PROP");

	const LONG DEFAULT_MIN = 0;
	const LONG DEFAULT_MAX = 100;
	const LONG DEFAULT_THUMB = 0;


	// GOALS of the slider class:
    // 
    // - Application can get/set logical position.
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

	struct Slider_Info
	{
		// Logical units
		LONG	posMin;		// minimum logical position
		LONG	posMax;		// maximum logical position
		LONG	posThumb;	// current logical position

		// Client area
		SIZE    pxThumbSize;	// real size of thumb bitmap (constant until bitmap changes
		Rect	rcThumb;		// client area of the thumb (changes with position)

		// state	
		BOOL	bThumbDown;		// User is dragging the thumb?

		// GDI objects
		HBRUSH	hBackground;
		HBITMAP	hbmThumb;		// Thumb bitmap
	};


	LRESULT CALLBACK Slider_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// Message handlers
	LRESULT OnCreate(HWND hwnd); 
	LRESULT OnNcDestroy(HWND hwnd, Slider_Info *pInfo); 
	LRESULT OnPaint(HWND hwnd, Slider_Info *pInfo); 
	LRESULT OnLButtonDown(HWND hwnd, LONG x, LONG y, Slider_Info *pInfo);
	LRESULT OnLButtonUp(HWND hwnd, LONG x, LONG y, Slider_Info *pInfo);
	LRESULT OnMouseMove(HWND hwnd, LONG x, LONG y, Slider_Info *pInfo);
    LRESULT OnReleaseSlider(HWND hwnd, Slider_Info *pInfo);

	// Private message handlers
	LRESULT OnSetThumbBitmap(HWND hwnd, WORD nID, Slider_Info *pInfo);
	LRESULT OnSetBackground(HWND hwnd, HBRUSH hBrush, Slider_Info *pInfo);
	LRESULT OnSetMinMax(HWND hwnd, LONG posMin, LONG posMax, Slider_Info *pInfo);	
	LRESULT OnSetPosition(HWND hwnd, LONG pos, Slider_Info *pInfo);
	LRESULT OnGetPosition(HWND hwnd, Slider_Info *pInfo);


    //--------------------------------------------------------------------------------------
    // SetInfo
    // Description: Store the control's instance information.
    //--------------------------------------------------------------------------------------

    inline BOOL SetInfo(HWND hwnd, Slider_Info *pInfo)
	{
		return SetProp(hwnd, InstanceData, pInfo);
	}

    //--------------------------------------------------------------------------------------
    // GetInfo
    // Description: Get the control's instance information.
    //--------------------------------------------------------------------------------------

    inline Slider_Info * GetInfo(HWND hwnd)
	{
		return (Slider_Info*)GetProp(hwnd, InstanceData);
	}

    //--------------------------------------------------------------------------------------
    // GetThumbRect
    // Description: Get the current position of the thumb rectangle.
    // 
    // The position is updated in the rcThumb member of pInfo.
    //--------------------------------------------------------------------------------------

    void GetThumbRect(HWND hwnd, Slider_Info *pInfo)
	{
		// 1. Convert logical position to pixels:
		//		logical width = max position - min position
		//		pixel width = client area - thumb width
		//		logical thumb position = current position - min position
		//		pixel thumb position = (logical thumb position / logical width) * pixel width

		Rect rc;
		GetClientRect(hwnd, &rc);

		LONG logWidth = pInfo->posMax - pInfo->posMin;
		LONG pixWidth = rc.Width() - pInfo->pxThumbSize.cx;
		LONG logPosition = pInfo->posThumb - pInfo->posMin;
		LONG left = MulDiv(logPosition, pixWidth, logWidth);	

		// 2. Center vertically
		LONG top = (rc.Height() - pInfo->pxThumbSize.cy) / 2;
	
		pInfo->rcThumb.Set(
			left,
			top,
			left + pInfo->pxThumbSize.cx,
			top + pInfo->pxThumbSize.cy
			);
	}

    //--------------------------------------------------------------------------------------
    // PixelToLogical
    // Description: Convert a pixel position to a logical position.
    // 
    // Returns the logical position.
    //--------------------------------------------------------------------------------------

    LONG PixelToLogical(HWND hwnd, LONG x, Slider_Info *pInfo)
	{
		// % of pixel width * logical width, max = logical max

		// (x / pixel width) * logical width + logical min

		Rect rc;
		GetClientRect(hwnd, &rc);

		LONG pixWidth = rc.Width();
		LONG logWidth = pInfo->posMax - pInfo->posMin;
		LONG pos = MulDiv(x, logWidth, pixWidth) + pInfo->posMin;

		// clamp to slider min and max
		return max(pInfo->posMin, min(pos, pInfo->posMax));
	}

    //--------------------------------------------------------------------------------------
    // NotifyParent
    // Description: Send the parent window a WM_NOTIFY message with our current status.
    // 
    // hwnd: Control window.
    // code: WM_NOTIFY code. (One of the SLIDER_NOTIFY_xxx constants.)
    // pInfo: Instance data.
    //--------------------------------------------------------------------------------------
	void NotifyParent(HWND hwnd, UINT code, Slider_Info *pInfo)
	{
		HWND hParent = GetParent(hwnd);

		if (hParent)
		{	
			NMSLIDER_INFO nminfo;

			nminfo.hdr.hwndFrom = hwnd;
			nminfo.hdr.idFrom = (UINT_PTR)GetMenu(hwnd);  
			nminfo.hdr.code = code;
			nminfo.bSelected = pInfo->bThumbDown;
			nminfo.position = pInfo->posThumb;

			SendMessage(hParent, WM_NOTIFY, (WPARAM)nminfo.hdr.idFrom, (LPARAM)&nminfo);
		}
	}

    //--------------------------------------------------------------------------------------
    // Slider_WndProc
    // Description: Window proc for the control.
    //--------------------------------------------------------------------------------------

	LRESULT CALLBACK Slider_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		Slider_Info * const pInfo = GetInfo(hwnd);

		switch (uMsg)
		{
		case WM_CREATE:
			return OnCreate(hwnd);

		case WM_PAINT:
			return OnPaint(hwnd, pInfo);

		case WM_NCDESTROY:
			return OnNcDestroy(hwnd, pInfo);

		case WM_LBUTTONDOWN:
			return OnLButtonDown(hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), pInfo);

		case WM_LBUTTONUP:
			return OnLButtonUp(hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), pInfo);

		case WM_MOUSEMOVE:
			return OnMouseMove(hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), pInfo);

        case WM_ENABLE:
            if (wParam == FALSE) // Window is disabled. Stop tracking.
            {
                return OnReleaseSlider(hwnd, pInfo);
            }
            break;

        case WM_CAPTURECHANGED:
           // The window lost focus while the slider was tracking the mouse OR 
           // the slider released the mouse capture itself.
           return OnReleaseSlider(hwnd, pInfo);


		// Custom messages
		case WM_SLIDER_SET_THUMB_BITMAP:
			return OnSetThumbBitmap(hwnd, (WORD)wParam, pInfo);

		case WM_SLIDER_SET_BACKGROUND:
			return OnSetBackground(hwnd, (HBRUSH)wParam, pInfo);

		case WM_SLIDER_SET_MIN_MAX:
			return OnSetMinMax(hwnd, (LONG)wParam, (LONG)lParam, pInfo);

		case WM_SLIDER_SET_POSITION:
			return OnSetPosition(hwnd, (LONG)wParam, pInfo);

		case WM_SLIDER_GET_POSITION:
			return OnGetPosition(hwnd, pInfo);

		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}

		return 0;
	};

    
	LRESULT OnCreate(HWND hwnd)
	{
		Slider_Info *pInfo = new Slider_Info();
        if (!pInfo)
        {
            return (LRESULT)-1;
        }

		ZeroMemory(pInfo, sizeof(Slider_Info));

		pInfo->posMin = DEFAULT_MIN;
		pInfo->posMax = DEFAULT_MAX;
		pInfo->posThumb = DEFAULT_THUMB;

		pInfo->bThumbDown = FALSE;

		pInfo->hBackground = CreateSolidBrush(RGB(0xFF, 0x80, 0x80));

        if (SetInfo(hwnd, pInfo))
        {
            return 0;
        }
        else
        {
            delete pInfo;
            return -1;
        }
	}

	LRESULT OnNcDestroy(HWND /*hwnd*/, Slider_Info *pInfo)
	{
        if (pInfo)
        {
		    DeleteObject(pInfo->hBackground);
		    DeleteObject(pInfo->hbmThumb);
		    delete pInfo;
        }
		return 0;
	}


	LRESULT OnPaint(HWND hwnd, Slider_Info *pInfo)
	{
		PAINTSTRUCT ps;
		HDC hdc;

		hdc = BeginPaint(hwnd, &ps);

		// Draw the background
		if (pInfo->hBackground)
		{
			FillRect(hdc, &ps.rcPaint, pInfo->hBackground);
		}

		// Draw the thumb

		if (pInfo->hbmThumb)
		{
			HDC hdcCompat = CreateCompatibleDC(hdc); 
			SelectObject(hdcCompat, pInfo->hbmThumb); 

			BOOL bResult = BitBlt(
				hdc, 
				pInfo->rcThumb.left, 
				pInfo->rcThumb.top, 
				pInfo->pxThumbSize.cx,
				pInfo->pxThumbSize.cy,
				hdcCompat,
				0, 0,
				SRCCOPY
				);

			assert(bResult);

			DeleteDC(hdcCompat);
		}

		EndPaint(hwnd, &ps);
		return 0;
	}


	void SetSliderPosition(HWND hwnd, LONG pos, Slider_Info *pInfo)
	{
		// Invalidate the old thumb rect
		InvalidateRect(hwnd, &pInfo->rcThumb, FALSE);

		pInfo->posThumb = pos;

		GetThumbRect(hwnd, pInfo);

		// Invalidate the new thumb rect
		InvalidateRect(hwnd, &pInfo->rcThumb, FALSE);
	}



	LRESULT OnLButtonDown(HWND hwnd, LONG x, LONG /*y*/, Slider_Info *pInfo)
	{
        // Move the slider to the mouse position.
		SetSliderPosition(hwnd, PixelToLogical(hwnd, x, pInfo), pInfo);

        // Set the thumb-down flag.
		pInfo->bThumbDown = TRUE;

        // Start capturing mouse moves so we can update the slider position.
		SetCapture(hwnd);

        // Notify the owner window that the control was selected.
		NotifyParent(hwnd, SLIDER_NOTIFY_SELECT, pInfo);

		return 0;
	}

    LRESULT OnLButtonUp(HWND hwnd, LONG /*x*/, LONG /*y*/, Slider_Info *pInfo)
    {
        return OnReleaseSlider(hwnd, pInfo);
    }

	LRESULT OnMouseMove(HWND hwnd, LONG x, LONG /*y*/, Slider_Info *pInfo)
	{
        // If the control is selected, update the slider position
        // and notify the owner window.
		if (pInfo->bThumbDown)
		{
			SetSliderPosition(hwnd, PixelToLogical(hwnd, x, pInfo), pInfo);

			NotifyParent(hwnd, SLIDER_NOTIFY_DRAG, pInfo);

		}
		return 0;
	}

    // OnReleaseSlider: Stop tracking the slider movement.
    LRESULT OnReleaseSlider(HWND hwnd, Slider_Info *pInfo)
    {
        if (pInfo->bThumbDown)
        {
            // Reset the thumb-down flag.
            pInfo->bThumbDown = FALSE;

            InvalidateRect(hwnd, &pInfo->rcThumb, FALSE);

            // Stop capturing mouse moves.
            ReleaseCapture();

            // Notify the owner window that the control was deselected.
            NotifyParent(hwnd, SLIDER_NOTIFY_RELEASE, pInfo);
        }
        return 0;
    }
    //--------------------------------------------------------------------------------------
    // OnSetThumbBitmap
    // Description: Sets the bitmap image for the slider thumb.
    // 
    // Handler for WM_SLIDER_SET_THUMB_BITMAP message.
    //--------------------------------------------------------------------------------------

	LRESULT OnSetThumbBitmap(HWND hwnd, WORD nID, Slider_Info *pInfo)
	{
		HBITMAP hbm = LoadBitmap(GetInstance(), MAKEINTRESOURCE(nID));

		if (hbm == NULL)
		{
			return FALSE;
		}

		BITMAP bm;
		GetObject(hbm, sizeof(BITMAP), &bm);

		pInfo->pxThumbSize.cx = bm.bmWidth;
		pInfo->pxThumbSize.cy = bm.bmHeight;

		if (pInfo->hbmThumb)
		{
			DeleteObject(pInfo->hbmThumb);
		}

		pInfo->hbmThumb = hbm;

		GetThumbRect(hwnd, pInfo);

		return TRUE;
	}

    //--------------------------------------------------------------------------------------
    // OnSetBackground
    // Description: Sets the background brush.
    // 
    // Handler for WM_SLIDER_SET_BACKGROUND message.
    //--------------------------------------------------------------------------------------

    LRESULT OnSetBackground(HWND /*hwnd*/, HBRUSH hBrush, Slider_Info *pInfo)
	{
		if (pInfo->hBackground)
		{
			DeleteObject(pInfo->hBackground);
		}

		pInfo->hBackground = hBrush;
	
		return TRUE;
	}


    //--------------------------------------------------------------------------------------
    // OnSetMinMax
    // Description: Sets the slider range.
    // 
    // Handler for WM_SLIDER_SET_MIN_MAX message.
    //--------------------------------------------------------------------------------------
	LRESULT OnSetMinMax(HWND hwnd, LONG posMin, LONG posMax, Slider_Info *pInfo)
	{
		pInfo->posMin = posMin;
		pInfo->posMax = posMax;

		if (pInfo->posThumb < posMin)
		{
			SetSliderPosition(hwnd, posMin, pInfo);
		}
		else if (pInfo->posThumb > posMax)
		{
			SetSliderPosition(hwnd, posMax, pInfo);
		}
		return TRUE;
	}

    //--------------------------------------------------------------------------------------
    // OnSetPosition
    // Description: Sets the slider range.
    // 
    // Handler for WM_SLIDER_SET_POSITION message.
    //--------------------------------------------------------------------------------------

    LRESULT OnSetPosition(HWND hwnd, LONG pos, Slider_Info *pInfo)
	{
        if (pos < pInfo->posMin || pos > pInfo->posMax)
        {
            return FALSE;
        }
        else
        {
    		SetSliderPosition(hwnd, pos, pInfo);
	    	return TRUE;
        }
	}

    //--------------------------------------------------------------------------------------
    // OnSetPosition
    // Description: Sets the slider range.
    // 
    // Handler for WM_SLIDER_GET_POSITION message.
    //--------------------------------------------------------------------------------------

    LRESULT OnGetPosition(HWND /*hwnd*/, Slider_Info *pInfo)
	{
		return pInfo->posThumb;
	}

}  // namespace SliderControl




//--------------------------------------------------------------------------------------
// Slider_Init
// Description: Initializes the slider window class.
// 
// Call this function before using the slider control.
//--------------------------------------------------------------------------------------

HRESULT Slider_Init()
{
	WNDCLASSEX wce;
	ZeroMemory(&wce, sizeof(wce));

	wce.cbSize = sizeof(WNDCLASSEX);
	wce.lpfnWndProc = SliderControl::Slider_WndProc;
	wce.hInstance = GetInstance();
	wce.lpszClassName = SliderControl::ClassName;
	wce.cbWndExtra = sizeof(SliderControl::Slider_Info);		// Reserve space for slider instance data


	ATOM a = RegisterClassEx(&wce);

	if (a == 0)
	{
		return __HRESULT_FROM_WIN32(GetLastError());
	}
	else
	{
		return S_OK;
	}
}

//--------------------------------------------------------------------------------------
// Slider_Create
// Description: Creates an instance of the slider control.
//--------------------------------------------------------------------------------------

HRESULT Slider_Create(HWND hParent, const Rect rc, DWORD_PTR id, HWND *pHwnd)
{
	HWND hwnd = CreateWindowEx(
		WS_EX_WINDOWEDGE,
		SliderControl::ClassName,
		NULL,
		WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		rc.left, rc.top, rc.Width(), rc.Height(),
		hParent,
		(HMENU)id,
		GetInstance(),
		NULL
		);

	if (hwnd == 0)
	{
		return E_FAIL;
	}
	else
	{
		*pHwnd = hwnd;
		return S_OK;
	}
}


// Wrapper class for the flat C functions.

HRESULT Slider::Create(HWND hParent, const Rect& rcSize, DWORD_PTR id)
{
	if (m_hwnd != 0)
	{
		return E_FAIL;
	}

	return Slider_Create(hParent, rcSize, id, &m_hwnd);
}


HRESULT Slider::SetThumbBitmap(UINT nId)
{
	if (SendMessage(WM_SLIDER_SET_THUMB_BITMAP, nId, 0))
    {
    	return S_OK; 
    }
    else
    {
        return E_FAIL;
    }
}

HRESULT Slider::SetBackground(HBRUSH hBackground)
{
	if (SendMessage(WM_SLIDER_SET_BACKGROUND, (WPARAM)hBackground, 0))
    {
    	return S_OK; 
    }
    else
    {
        return E_FAIL;
    }
}

LONG Slider::GetPosition() const
{
	return (LONG)SendMessage(WM_SLIDER_GET_POSITION, 0, 0);
}

HRESULT Slider::SetPosition(LONG pos)
{
	if (SendMessage(WM_SLIDER_SET_POSITION, pos, 0))
    {
    	return S_OK; 
    }
    else
    {
        return E_FAIL;
    }
}

HRESULT Slider::SetRange(LONG min, LONG max)
{
	if (SendMessage(WM_SLIDER_SET_MIN_MAX, min, max))
    {
    	return S_OK; 
    }
    else
    {
        return E_FAIL;
    }
}


