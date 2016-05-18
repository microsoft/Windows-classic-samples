//------------------------------------------------------------------------------
// File: Bitmap.cpp
//
// Desc: DirectShow sample code - Bitmap manipulation routines for 
//       VMR alpha-blended bitmap
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <tchar.h>
#include <dshow.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>

#include "watermark.h"
#include "bitmap.h"
#include "resource.h"

//
// Constants
//
const float EDGE_BUFFER  = 0.04f;  // Pixel buffer between bitmap and window edge
                                   // (represented in composition space [0 - 1.0f])

const float SLIDE_VALUE  = 0.05f;  // Amount to slide image in composition space
const float STROBE_VALUE = 0.125f; // Amount to add to bitmap alpha value

const int ANIMATE_TIMER  = 2000;   // Timer IDs
const int SLIDE_TIMER    = 2001;
const int STROBE_TIMER   = 2002;
const int MAIN_TIMER     = 2003;

const int ANIMATE_TIMEOUT = 250;   // Timer delays in milliseconds
const int SLIDE_TIMEOUT   = 125;
const int STROBE_TIMEOUT  = 125;
const int MAIN_TIMEOUT    = 125;

const int NUM_IMAGES_IN_BITMAP = 5; // Five images in single wide bitmap (320 x 64)

//
// Global data
//
IVMRMixerBitmap9 *pBMP = NULL;
DWORD g_dwWatermarkFlags=0;
int gnTimer=0;

float g_fBitmapCompWidth=0;
int g_nBitmapWidth=0;               // Width of wide multi-image bitmap
int g_nImageWidth=0;                // Width of single image in wide multi-image bitmap
BOOL g_bRestoreFlip=0, g_bRestoreMirror=0;
HBITMAP g_hbmAnimate=0;
BOOL g_bWatermarkDisabled=FALSE;

// Rectangles used for alpha-blended watermark
RECT g_rSrc={0}, g_rSrcSingle={0};
VMR9NormalizedRect  g_rDest={0};



HRESULT BlendApplicationImage(HWND hwndApp)
{
    LONG cx, cy;
    HRESULT hr;
    RECT rc={0};

    // Read the default video size
    hr = pWC->GetNativeVideoSize(&cx, &cy, NULL, NULL);
    if (FAILED(hr))
      return hr;

    // Load the multi-image bitmap to alpha blend from the resource file
    HBITMAP hbm = LoadBitmap(ghInst, MAKEINTRESOURCE(IDR_VMR_WIDE));

    BITMAP bm;
    HBITMAP hbmOld;
    HDC hdc = GetDC(hwndApp);
    HDC hdcBmp = CreateCompatibleDC(hdc);
    ReleaseDC(hwndApp, hdc);

    GetObject(hbm, sizeof(bm), &bm);
    hbmOld = (HBITMAP)SelectObject(hdcBmp, hbm);

    // Configure the VMR's bitmap structure
    VMR9AlphaBitmap bmpInfo;
    ZeroMemory(&bmpInfo, sizeof(bmpInfo) );
    bmpInfo.dwFlags = VMRBITMAP_HDC;
    bmpInfo.hdc = hdcBmp;

    // The wide bitmap contains five similar images that can be
    // cycled to provide the illusion of animation.
    g_nBitmapWidth = bm.bmWidth;
    g_nImageWidth  = bm.bmWidth / NUM_IMAGES_IN_BITMAP;

    // Display the bitmap in the bottom right corner.
    // rSrc specifies the source rectangle in the GDI device context.
    // To enable animating between multiple single images within a wide 
    // bitmap, we must specify the entire rectangle of the wide image.
    // The VMR will convert this rectangle into a DirectDraw surface,
    // within which we can select a smaller source rectangle to display.
    SetRect(&rc, 0, 0, g_nBitmapWidth, bm.bmHeight);
    bmpInfo.rSrc = rc;

    // rDest specifies the destination rectangle in composition space (0.0f to 1.0f)
    bmpInfo.rDest.left   = (float)(cx - g_nImageWidth) / (float)cx - EDGE_BUFFER;
    bmpInfo.rDest.top    = (float)(cy - bm.bmHeight)   / (float)cy - EDGE_BUFFER;
    bmpInfo.rDest.right  = 1.0f - EDGE_BUFFER;
    bmpInfo.rDest.bottom = 1.0f - EDGE_BUFFER;

    // Copy initial settings to global memory for later modification
    g_rDest = bmpInfo.rDest;
    g_rSrc  = bmpInfo.rSrc;

    // Save the ratio of the bitmap's width to the width of the video file.
    // This value is used to reposition the bitmap in composition space.
    g_fBitmapCompWidth = (float)g_nImageWidth / (float)cx;

    // Transparency value 1.0 is opaque, 0.0 is transparent.
    // For initially setting the bitmap, we'll make it transparent
    // because we need to give the VMR the entire wide image.
    bmpInfo.fAlpha = 0.0;

    // Set the COLORREF so that the bitmap outline will be transparent
    SetColorRef(bmpInfo);

    // Give the bitmap to the VMR.  Since the alpha value is 0, nothing will
    // be displayed yet on the screen, but the VMR will have the information
    // that it needs to allow us to modify the bitmap.
    hr = pBMP->SetAlphaBitmap(&bmpInfo);
    if (FAILED(hr))
        Msg(TEXT("SetAlphaBitmap FAILED!  Bitmap operations will fail. hr=0x%x\r\n"), hr);

    // Clean up GDI resources
    DeleteObject(SelectObject(hdcBmp, hbmOld));
    DeleteObject(hbm);
    DeleteDC(hdcBmp);

    // If setting the alpha bitmap succeeded, update its parameters
    if (SUCCEEDED(hr))
    {
        //
        // Now change the size of the source rectangle to a single
        // image width.  Update the alpha so that the image will be
        // properly displayed.
        //
        SetRect(&rc, 0, 0, g_nImageWidth, bm.bmHeight);
        bmpInfo.rSrc = rc;

        // Save the single-image rectangle for later reference during animation
        g_rSrcSingle = rc;

        // Set the necessary flags to update the source rectangle
        bmpInfo.dwFlags = VMRBITMAP_SRCRECT | VMRBITMAP_SRCCOLORKEY;

        // Transparency value 1.0 is opaque, 0.0 is transparent.
        bmpInfo.fAlpha = TRANSPARENCY_VALUE;

        // Update the source rectangle and alpha values of the bitmap.
        // Now the image will appear properly on the screen.
        hr = pBMP->UpdateAlphaBitmapParameters(&bmpInfo);
        if (FAILED(hr))
            Msg(TEXT("UpdateAlphaBitmapParameters FAILED!  hr=0x%x\r\n"), hr);
    }

    return hr;
}


void ClearWatermarkState(void)
{
    KillTimer();

    // Clear current watermark flags
    g_dwWatermarkFlags = 0;

    // Enable all watermark menu items, which may have been
    // disabled if the watermark was currently disabled
    EnableWatermarkMenu(TRUE);

    // Clear menu check marks
    CheckMenuItem(ghMenu, ID_FLIP,    MF_UNCHECKED);
    CheckMenuItem(ghMenu, ID_MIRROR,  MF_UNCHECKED);
    CheckMenuItem(ghMenu, ID_ANIMATE, MF_UNCHECKED);
    CheckMenuItem(ghMenu, ID_SLIDE,   MF_UNCHECKED);
    CheckMenuItem(ghMenu, ID_STROBE,  MF_UNCHECKED);
    CheckMenuItem(ghMenu, ID_ALL_EFFECTS, MF_UNCHECKED);
    CheckMenuItem(ghMenu, ID_NO_EFFECTS,  MF_UNCHECKED);
}


void EnableWatermarkMenu(BOOL bEnable)
{
    EnableMenuItem(ghMenu, ID_FLIP,    bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(ghMenu, ID_MIRROR,  bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(ghMenu, ID_ANIMATE, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(ghMenu, ID_SLIDE,   bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(ghMenu, ID_STROBE,  bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(ghMenu, ID_ALL_EFFECTS, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(ghMenu, ID_NO_EFFECTS,  bEnable ? MF_ENABLED : MF_GRAYED);

    g_bWatermarkDisabled = !bEnable;
}


HRESULT MirrorWatermark(DWORD dwFlags)
{
    HRESULT hr;
    VMR9AlphaBitmap bmpInfo={0};

    hr = pBMP->GetAlphaBitmapParameters(&bmpInfo);

    // Configure the X axis
    if (dwFlags & MARK_MIRROR)
    {
        // Swap left/right coordinates
        float fLeft = bmpInfo.rDest.left;
        bmpInfo.rDest.left = bmpInfo.rDest.right;
        bmpInfo.rDest.right = fLeft;

        CheckMenuItem(ghMenu, ID_MIRROR, MF_CHECKED);
    }
    else
    {
        // If we're removing the mirror effect while sliding, then
        // it's not correct to reset the left/right coordinates to
        // the default saved global values.  Instead, just switch 
        // the left/right values.
        if (dwFlags & MARK_SLIDE)
        {
            // Swap left/right coordinates
            float fLeft = bmpInfo.rDest.left;
            bmpInfo.rDest.left = bmpInfo.rDest.right;
            bmpInfo.rDest.right = fLeft;
        }
        else
        {
            // We're not sliding, so just reset the left/right
            // coordinates to their original default values.
            bmpInfo.rDest.left  = g_rDest.left;
            bmpInfo.rDest.right = g_rDest.right;
        }

        CheckMenuItem(ghMenu, ID_MIRROR, MF_UNCHECKED);
    }

    // Set the COLORREF so that the bitmap outline will be transparent
    SetColorRef(bmpInfo);

    hr = pBMP->UpdateAlphaBitmapParameters(&bmpInfo);
    if (FAILED(hr))
        Msg(TEXT("UpdateAlphaBitmapParameters FAILED!  hr=0x%x\r\n"), hr);

    return hr;
}


HRESULT FlipWatermark(DWORD dwFlags)
{
    HRESULT hr;
    VMR9AlphaBitmap bmpInfo={0};

    hr = pBMP->GetAlphaBitmapParameters(&bmpInfo);

    // Configure the Y axis
    if (dwFlags & MARK_FLIP)
    {
        // Swap left/right coordinates
        float fTop = bmpInfo.rDest.top;
        bmpInfo.rDest.top = bmpInfo.rDest.bottom;
        bmpInfo.rDest.bottom = fTop;

        CheckMenuItem(ghMenu, ID_FLIP, MF_CHECKED);
    }
    else
    {
        bmpInfo.rDest.top = g_rDest.top;
        bmpInfo.rDest.bottom = g_rDest.bottom;

        CheckMenuItem(ghMenu, ID_FLIP, MF_UNCHECKED);
    }

    // Set the COLORREF so that the bitmap outline will be transparent
    SetColorRef(bmpInfo);

    hr = pBMP->UpdateAlphaBitmapParameters(&bmpInfo);
    if (FAILED(hr))
        Msg(TEXT("UpdateAlphaBitmapParameters FAILED!  hr=0x%x\r\n"), hr);

    return hr;
}


HRESULT DisableWatermark(DWORD dwFlags)
{
    HRESULT hr;
    VMR9AlphaBitmap bmpInfo={0};

    // Since the enable/disable menu will always be enabled,
    // verify that the VMR Alpha bitmap interface is set.
    if (!pBMP)
        return E_NOINTERFACE;

    // Read the current bitmap settings
    hr = pBMP->GetAlphaBitmapParameters(&bmpInfo);

    // If the request is to disable the bitmap, then disable it
    // and ignore the other flags for now
    if (dwFlags & MARK_DISABLE)
    {
        // Disable other watermark menu items
        EnableWatermarkMenu(FALSE);

        // Remember if the flip/mirror states are set for redrawing
        if (g_dwWatermarkFlags & MARK_FLIP)
            g_bRestoreFlip = TRUE;
        if (g_dwWatermarkFlags & MARK_MIRROR)
            g_bRestoreMirror = TRUE;

	// If animation is active, reset
	if (dwFlags & MARK_ANIMATE)
	{
		ResetAnimation();

		// Read the current bitmap settings again post-reset
		pBMP->GetAlphaBitmapParameters(&bmpInfo);
	}
        
	// Temporarily disable bitmap display
        bmpInfo.dwFlags = VMRBITMAP_DISABLE;

        // Update the bitmap settings
        hr = pBMP->UpdateAlphaBitmapParameters(&bmpInfo);
        if (FAILED(hr))
            Msg(TEXT("UpdateAlphaBitmapParameters FAILED to disable watermark!  hr=0x%x\r\n"), hr);
    }
    else
    {
        // Reset the bitmap with default values
        hr = BlendApplicationImage(ghApp);

        // Reenable other watermark menu items
        EnableWatermarkMenu(TRUE);

        // Preserve the previously-set mirror/flip settings
        if (g_bRestoreFlip)
        {
            FlipWatermark(g_dwWatermarkFlags);
            g_bRestoreFlip = FALSE;
        }
        if (g_bRestoreMirror)
        {
            MirrorWatermark(g_dwWatermarkFlags);
            g_bRestoreMirror = FALSE;
        }
    }

    return hr;
}


void FlipFlag(DWORD dwFlag)
{
    // If the flag is set, clear it
    if (g_dwWatermarkFlags & dwFlag)
        g_dwWatermarkFlags &= ~dwFlag;

    // Otherwise, set the flag
    else
        g_dwWatermarkFlags |= dwFlag;
}


void AnimateWatermark(DWORD dwFlags)
{
    if (dwFlags & MARK_ANIMATE)
    {
        StartTimer();
        CheckMenuItem(ghMenu, ID_ANIMATE, MF_CHECKED);
    }
    else
    {
        CheckMenuItem(ghMenu, ID_ANIMATE, MF_UNCHECKED);
        ResetAnimation();
    }
}


void SlideWatermark(DWORD dwFlags)
{
    if (dwFlags & MARK_SLIDE)
    {
        StartTimer();
        CheckMenuItem(ghMenu, ID_SLIDE, MF_CHECKED);
    }
    else
    {
        CheckMenuItem(ghMenu, ID_SLIDE, MF_UNCHECKED);
        ResetBitmapPosition();
    }
}


void StrobeWatermark(DWORD dwFlags)
{
    // Start cycling the bitmap's alpha value on a timer
    if (dwFlags & MARK_STROBE)
    {
        StartTimer();
        CheckMenuItem(ghMenu, ID_STROBE, MF_CHECKED);
    }
    else
    {
        CheckMenuItem(ghMenu, ID_STROBE, MF_UNCHECKED);
        ResetBitmapAlpha();
    }
}


void StartTimer(void)
{
    if (!gnTimer)
        gnTimer = (int) SetTimer(NULL, MAIN_TIMER, MAIN_TIMEOUT, TimerProc);
}

void KillTimer(void)
{
    if (gnTimer)
    {
        KillTimer(NULL, gnTimer);
        gnTimer = 0;
    }
}


VOID CALLBACK TimerProc(
  HWND hwnd,         // handle to window
  UINT uMsg,         // WM_TIMER message
  UINT_PTR idEvent,  // timer identifier
  DWORD dwTime       // current system time
)
{
    static int nToggle=0;

    // If the user has disabled the watermark, just exit.
    if (g_bWatermarkDisabled)
        return;

    // Use one timer for efficiency, instead of starting/stopping
    // three separate timers.  Since animation will run at half the
    // rate of slide/strobe, only process animation in every other callback.
    if (g_dwWatermarkFlags & MARK_SLIDE)
        HandleSlide();

    if (g_dwWatermarkFlags & MARK_STROBE)
        HandleStrobe();

    if (g_dwWatermarkFlags & MARK_ANIMATE)
    {
        nToggle ^= 1;
        if (nToggle)
            HandleAnimation();
    }
}


void HandleStrobe(void)
{
    HRESULT hr;
    VMR9AlphaBitmap bmpInfo={0};

    hr = pBMP->GetAlphaBitmapParameters(&bmpInfo);

    // Slowly increase the alpha value
    float fAlpha = bmpInfo.fAlpha + STROBE_VALUE;
    if (fAlpha > 1.0f)
        fAlpha = 0.0f;

    bmpInfo.fAlpha = fAlpha;

    // Set the COLORREF so that the bitmap outline will be transparent
    SetColorRef(bmpInfo);

    // If the bitmap is currently disabled, this call will fail
    hr = pBMP->UpdateAlphaBitmapParameters(&bmpInfo);
}


void ResetBitmapAlpha(void)
{
    HRESULT hr;
    VMR9AlphaBitmap bmpInfo={0};

    if (!pBMP)
        return;

    hr = pBMP->GetAlphaBitmapParameters(&bmpInfo);

    // Return bitmap alpha to its default value
    bmpInfo.fAlpha = TRANSPARENCY_VALUE;

    // Set the COLORREF so that the bitmap outline will be transparent
    SetColorRef(bmpInfo);

    hr = pBMP->UpdateAlphaBitmapParameters(&bmpInfo);
    if (FAILED(hr))
        Msg(TEXT("UpdateAlphaBitmapParameters FAILED!  hr=0x%x\r\n"), hr);
}


void HandleSlide(void)
{
    HRESULT hr;
    VMR9AlphaBitmap bmpInfo={0};

    hr = pBMP->GetAlphaBitmapParameters(&bmpInfo);

    // Slowly decrease the X coordinate
    bmpInfo.rDest.left  -= SLIDE_VALUE;   
    bmpInfo.rDest.right -= SLIDE_VALUE;

    // Once the bitmap disappears off the left side of the screen,
    // reset to the rightmost side of the window.
    // Take into account that the bitmap might be mirrored, in which case
    // the left/right coordinates are switched.

    if ((g_dwWatermarkFlags & MARK_MIRROR) == 0)
    {
        // NOT mirrored
        if (bmpInfo.rDest.right <= EDGE_BUFFER)
        {
            bmpInfo.rDest.left = 1.0f;
            bmpInfo.rDest.right = 1.0f + g_fBitmapCompWidth;
        }
    }
    else
    {
        // Mirrored
        if (bmpInfo.rDest.left <= EDGE_BUFFER)
        {
            bmpInfo.rDest.right = 1.0f;
            bmpInfo.rDest.left = 1.0f + g_fBitmapCompWidth;
        }
    }

    // Set the COLORREF so that the bitmap outline will be transparent
    SetColorRef(bmpInfo);

    // If the bitmap is currently disabled, this call will fail
    hr = pBMP->UpdateAlphaBitmapParameters(&bmpInfo);
}


void ResetBitmapPosition(void)
{
    HRESULT hr;
    VMR9AlphaBitmap bmpInfo={0};

    if (!pBMP)
        return;

    hr = pBMP->GetAlphaBitmapParameters(&bmpInfo);

    // Return bitmap position to its original value.
    // Since the image may be currently mirrored, swap the
    // left/right coordinates if necessary
    if (g_dwWatermarkFlags & MARK_MIRROR)
    {
        // Mirrored
        bmpInfo.rDest.left  = g_rDest.right;
        bmpInfo.rDest.right = g_rDest.left;
    }
    else
    {
        // NOT mirrored
        bmpInfo.rDest.left  = g_rDest.left;
        bmpInfo.rDest.right = g_rDest.right;
    }

    // Set the COLORREF so that the bitmap outline will be transparent
    SetColorRef(bmpInfo);

    hr = pBMP->UpdateAlphaBitmapParameters(&bmpInfo);
    if (FAILED(hr))
        Msg(TEXT("UpdateAlphaBitmapParameters FAILED!  hr=0x%x\r\n"), hr);
}


void HandleAnimation(void)
{
    HRESULT hr;
    VMR9AlphaBitmap bmpInfo={0};
    static int nCycle=0;

    // Fill the rDest and fAlpha values in bmpInfo
    hr = pBMP->GetAlphaBitmapParameters(&bmpInfo);

    // Move the image source to the right by one image width
    bmpInfo.rSrc.left   = g_nImageWidth * nCycle;
    bmpInfo.rSrc.right  = bmpInfo.rSrc.left + g_nImageWidth;
    bmpInfo.rSrc.top    = g_rSrc.top;
    bmpInfo.rSrc.bottom = g_rSrc.bottom;
    nCycle++;

    // If we have passed the last image in the wide bitmap,
    // then reset to the default source location (leftmost image)
    if (bmpInfo.rSrc.left >= g_nBitmapWidth)
    {
        bmpInfo.rSrc = g_rSrcSingle;
        nCycle = 1;
    }

    // Set the necessary flag to update the source rectangle
    bmpInfo.dwFlags = VMRBITMAP_SRCRECT;

    // Set the COLORREF so that the bitmap outline will be transparent
    SetColorRef(bmpInfo);

    // Give the VMR a new bitmap to display
    // If the bitmap is currently disabled, this call will fail
    hr = pBMP->UpdateAlphaBitmapParameters(&bmpInfo);
}


void ResetAnimation(void)
{
    HRESULT hr;
    VMR9AlphaBitmap bmpInfo={0};

    if (!pBMP)
        return;

    hr = pBMP->GetAlphaBitmapParameters(&bmpInfo);

    // Update the source rectangle to the original single-image default
    bmpInfo.rSrc = g_rSrcSingle;

    // Set the necessary flag to update the source rectangle
    bmpInfo.dwFlags = VMRBITMAP_SRCRECT;

    // Set the COLORREF so that the bitmap outline will be transparent
    SetColorRef(bmpInfo);

    hr = pBMP->UpdateAlphaBitmapParameters(&bmpInfo);
    if (FAILED(hr))
        Msg(TEXT("UpdateAlphaBitmapParameters FAILED!  hr=0x%x\r\n"), hr);
}


void SetColorRef(VMR9AlphaBitmap& bmpInfo)
{
    // Set the COLORREF so that the bitmap outline will be transparent
    bmpInfo.clrSrcKey = RGB(255, 255, 255);       // Pure white
    bmpInfo.dwFlags |= VMRBITMAP_SRCCOLORKEY;
}


void SetAllEffects(void)
{
    // Activate any effects that aren't currently active
    if ((g_dwWatermarkFlags & MARK_ANIMATE) == 0)
    {
        FlipFlag(MARK_ANIMATE);
        AnimateWatermark(g_dwWatermarkFlags);
    }
    if ((g_dwWatermarkFlags & MARK_STROBE) == 0)
    {
        FlipFlag(MARK_STROBE);
        StrobeWatermark(g_dwWatermarkFlags);
    }
    if ((g_dwWatermarkFlags & MARK_SLIDE) == 0)
    {
        FlipFlag(MARK_SLIDE);
        SlideWatermark(g_dwWatermarkFlags);
    }
}


void ClearAllEffects(void)
{
    KillTimer();

    // Deactivate all active effects
    if (g_dwWatermarkFlags & MARK_ANIMATE)
    {
        FlipFlag(MARK_ANIMATE);
        AnimateWatermark(g_dwWatermarkFlags);
    }
    if (g_dwWatermarkFlags & MARK_STROBE)
    {
        FlipFlag(MARK_STROBE);
        StrobeWatermark(g_dwWatermarkFlags);
    }
    if (g_dwWatermarkFlags & MARK_SLIDE)
    {
        FlipFlag(MARK_SLIDE);
        SlideWatermark(g_dwWatermarkFlags);
    }
}
