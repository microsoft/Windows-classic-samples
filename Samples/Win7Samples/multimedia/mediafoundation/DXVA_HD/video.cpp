//////////////////////////////////////////////////////////////////////
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <d3d9.h>
#include "video.h"

//-------------------------------------------------------------------
// DrawColorBars
//
// Draw SMPTE colors bars to a Direct3D surface. 
// The surface format is assumed to be YUY2.
//-------------------------------------------------------------------

HRESULT DrawColorBars(IDirect3DSurface9 *pSurf, UINT width, UINT height)
{
    HRESULT hr;

    // Draw the main stream (SMPTE color bars).

    D3DLOCKED_RECT lr;

    hr = pSurf->LockRect(&lr, NULL, D3DLOCK_NOSYSLOCK);
    if (FAILED(hr)) 
    { 
        return hr;
    }

    // YUY2 is two pixels per DWORD.
    const UINT dx = width / 2;

    // First row stripes.
    const UINT y1 = height * 2 / 3;

    FillRectangle(lr, dx * 0 / 7, 0, dx * 1 / 7, y1, RGBtoYUY2(RGB_WHITE_75pc));
    FillRectangle(lr, dx * 1 / 7, 0, dx * 2 / 7, y1, RGBtoYUY2(RGB_YELLOW_75pc));
    FillRectangle(lr, dx * 2 / 7, 0, dx * 3 / 7, y1, RGBtoYUY2(RGB_CYAN_75pc));
    FillRectangle(lr, dx * 3 / 7, 0, dx * 4 / 7, y1, RGBtoYUY2(RGB_GREEN_75pc));
    FillRectangle(lr, dx * 4 / 7, 0, dx * 5 / 7, y1, RGBtoYUY2(RGB_MAGENTA_75pc));
    FillRectangle(lr, dx * 5 / 7, 0, dx * 6 / 7, y1, RGBtoYUY2(RGB_RED_75pc));
    FillRectangle(lr, dx * 6 / 7, 0, dx * 7 / 7, y1, RGBtoYUY2(RGB_BLUE_75pc));

    // Second row stripes.
    const UINT y2 = height * 3 / 4;

    FillRectangle(lr, dx * 0 / 7, y1, dx * 1 / 7, y2, RGBtoYUY2(RGB_BLUE_75pc));
    FillRectangle(lr, dx * 1 / 7, y1, dx * 2 / 7, y2, RGBtoYUY2(RGB_BLACK));
    FillRectangle(lr, dx * 2 / 7, y1, dx * 3 / 7, y2, RGBtoYUY2(RGB_MAGENTA_75pc));
    FillRectangle(lr, dx * 3 / 7, y1, dx * 4 / 7, y2, RGBtoYUY2(RGB_BLACK));
    FillRectangle(lr, dx * 4 / 7, y1, dx * 5 / 7, y2, RGBtoYUY2(RGB_CYAN_75pc));
    FillRectangle(lr, dx * 5 / 7, y1, dx * 6 / 7, y2, RGBtoYUY2(RGB_BLACK));
    FillRectangle(lr, dx * 6 / 7, y1, dx * 7 / 7, y2, RGBtoYUY2(RGB_WHITE_75pc));

    // Third row stripes.
    const UINT y3  = height;

    FillRectangle(lr, dx *  0 / 28, y2, dx *  5 / 28, y3, RGBtoYUY2(RGB_I));
    FillRectangle(lr, dx *  5 / 28, y2, dx * 10 / 28, y3, RGBtoYUY2(RGB_WHITE));
    FillRectangle(lr, dx * 10 / 28, y2, dx * 15 / 28, y3, RGBtoYUY2(RGB_Q));
    FillRectangle(lr, dx * 15 / 28, y2, dx * 20 / 28, y3, RGBtoYUY2(RGB_BLACK));
    FillRectangle(lr, dx * 20 / 28, y2, dx * 16 / 21, y3, RGBtoYUY2(RGB_BLACK_n4pc));
    FillRectangle(lr, dx * 16 / 21, y2, dx * 17 / 21, y3, RGBtoYUY2(RGB_BLACK));
    FillRectangle(lr, dx * 17 / 21, y2, dx *  6 /  7, y3, RGBtoYUY2(RGB_BLACK_p4pc));
    FillRectangle(lr, dx *  6 /  7, y2, dx *  7 /  7, y3, RGBtoYUY2(RGB_BLACK));

    hr = pSurf->UnlockRect();

    return hr;
}


//-------------------------------------------------------------------
// LoadBitmapResourceToAYUVSurface
//
// Load a 24-bpp RGB bitmap resource onto a Direct3D surface.
// The surface format is assumed to be AYUV. 
//-------------------------------------------------------------------

HRESULT LoadBitmapResourceToAYUVSurface(
    IDirect3DSurface9 *pSurf, 
    LONG width,                 // Surface width
    LONG height,                // Surface height
    INT nIDBitmap,              // Resource ID          
    BYTE PixelAlphaValue        // Per-pixel alpha value
    )
{
    HRESULT hr = S_OK;

    D3DLOCKED_RECT lr = { 0 };
    HRSRC hrsrc = NULL;
    HGLOBAL hglob = NULL;

    // Load the bitmap resource.
    hrsrc = FindResource(NULL, MAKEINTRESOURCE(nIDBitmap), RT_BITMAP);
    if (hrsrc == NULL)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    hglob = LoadResource(NULL,hrsrc);
    if (hglob == NULL)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Get the bitmap bits.
    LPBITMAPINFOHEADER lpBitmap = (LPBITMAPINFOHEADER)LockResource(hglob);

    if (lpBitmap->biWidth != width || lpBitmap->biHeight != height)
    {
        hr = E_INVALIDARG;
        goto done;
    }


    // Lock the Direct3D surface.
    hr = pSurf->LockRect(&lr, NULL, D3DLOCK_NOSYSLOCK);

    if (FAILED(hr)) { goto done; }

    // Calculate source and target stride. 
    // The source bitmap is bottom-up, the target image is top-down.

    LPBYTE lpBits = (LPBYTE)(lpBitmap + 1);

    LONG lSrcStride = -1 * ((lpBitmap->biWidth * 3 + 3) & ~3);

    BYTE *pSrc = (BYTE*)lpBits + (-lSrcStride * (lpBitmap->biHeight - 1));
    BYTE* pDest = (BYTE*) lr.pBits;

    // Convert the RGB pixels to AYUV and write to the surface.
    for (LONG y = 0; y < lpBitmap->biHeight; y++)
    {
        RGBTRIPLE *pPel = (RGBTRIPLE*)pSrc;
        for (LONG x = 0; x < lpBitmap->biWidth; x++)
        {
            const D3DCOLOR rgb = D3DCOLOR_ARGB(
                PixelAlphaValue, 
                pPel[x].rgbtRed, 
                pPel[x].rgbtGreen, pPel[x].rgbtBlue
                );

            ((DWORD*) pDest)[x] = RGBtoYUV(rgb);
        }

        pSrc += lSrcStride;
        pDest += lr.Pitch;
    }

    hr = pSurf->UnlockRect();

done:
    DeleteObject(hglob);
    return hr;
}


//-------------------------------------------------------------------
// SetAYUVSurfacePixelAlpha
//
// Updates the per-pixel alpha values in an AYUV surface.
//-------------------------------------------------------------------

HRESULT SetAYUVSurfacePixelAlpha(
    IDirect3DSurface9 *pSurf, 
    UINT width, 
    UINT height, 
    BYTE PixelAlphaValue
    )
{
    HRESULT hr;

    D3DLOCKED_RECT lr;

    hr = pSurf->LockRect(&lr, NULL, D3DLOCK_NOSYSLOCK);
    if (FAILED(hr)) 
    { 
        return hr;
    }

    BYTE* pDest = (BYTE*) lr.pBits;

    for (UINT y = 0; y < height; y++)
    {
        for (UINT x = 0; x < width; x++)
        {
            pDest[ x * 4 + 3 ] = PixelAlphaValue;
        }

        pDest += lr.Pitch;
    }
    hr = pSurf->UnlockRect();

    return hr;
}


//-------------------------------------------------------------------
// FillRectangle
//
// Fills a subrectangle in a Direct3D surface with a solid color.
// 
// The caller must call LockSurface to lock the surface, and pass in
// the D3DLOCKED_RECT structure.
//-------------------------------------------------------------------

void FillRectangle(
    D3DLOCKED_RECT& lr,
    const UINT sx,          // Horizontal position
    const UINT sy,          // Vertical position
    const UINT ex,          // Horizontal extent.
    const UINT ey,          // Vertical extent.
    const DWORD color
    )
{
    BYTE* p = (BYTE*) lr.pBits;

    p += lr.Pitch * sy;

    for (UINT y = sy; y < ey; y++, p += lr.Pitch)
    {
        for (UINT x = sx; x < ex; x++)
        {
            ((DWORD*) p)[x] = color;
        }
    }
}


//-------------------------------------------------------------------
// ScaleRectangle
//
// Scales an input rectangle, using the following scaling factor:
// 
//      output = input * (dst / src)
//    
//-------------------------------------------------------------------

RECT ScaleRectangle(const RECT& input, const RECT& src, const RECT& dst)
{
    RECT rect;

    UINT src_dx = src.right - src.left;
    UINT src_dy = src.bottom - src.top;

    UINT dst_dx = dst.right - dst.left;
    UINT dst_dy = dst.bottom - dst.top;

    // Scale the input rectangle by dst / src.

    rect.left   = input.left   * dst_dx / src_dx;
    rect.right  = input.right  * dst_dx / src_dx;
    rect.top    = input.top    * dst_dy / src_dy;
    rect.bottom = input.bottom * dst_dy / src_dy;

    return rect;
}


//-------------------------------------------------------------------
// InflateRectBounded
//
// Resizes a rectangle by a specified amount, but clips the result 
// to a bounding rectangle.
//
// prc:     The rectangle to resize.
// cx, cy:  Amount to increase or decrease the rectangle.
// rcBound: Bounding rectangle.
//
//-------------------------------------------------------------------

void InflateRectBounded(RECT *prc, LONG cx, LONG cy, const RECT& rcBound)
{
    RECT rect = *prc;
    RECT intersect;

    InflateRect(&rect, cx, cy);
    IntersectRect(&intersect, &rect, &rcBound);

    if (!IsRectEmpty(&intersect))
    {
        *prc = intersect;
    }
}


//-------------------------------------------------------------------
// MoveRectBounded
//
// Moves a rectangle by a specified amount, but clips the result to
// a bounding rectangle.
//
// prc:     The rectangle to move.
// cx, cy:  Amount to move in the x and y directions.
// rcBound: Bounding rectangle.
//
//-------------------------------------------------------------------

void MoveRectBounded(RECT *prc, LONG cx, LONG cy, const RECT& rcBound)
{
    RECT rect = *prc;
    RECT intersect;

    OffsetRect(&rect, cx, cy);
    IntersectRect(&intersect, &rect, &rcBound);

    if (EqualRect(&rect, &intersect))
    {
        *prc = rect;
    }
}

