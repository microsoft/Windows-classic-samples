//////////////////////////////////////////////////////////////////////
// 
// video.h
//
// Contains the following:
// - RGB-YUV conversion functions.
// - Functions for drawing the video images to Direct3D surfaces.
// - Functions for calculating source/destination rectangles.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////

#pragma once

//
// Studio RGB [16...235] colors.
//

// 100%
const D3DCOLOR RGB_WHITE        = D3DCOLOR_XRGB(0xEB, 0xEB, 0xEB);
const D3DCOLOR RGB_RED          = D3DCOLOR_XRGB(0xEB, 0x10, 0x10);
const D3DCOLOR RGB_YELLOW       = D3DCOLOR_XRGB(0xEB, 0xEB, 0x10);
const D3DCOLOR RGB_GREEN        = D3DCOLOR_XRGB(0x10, 0xEB, 0x10);
const D3DCOLOR RGB_CYAN         = D3DCOLOR_XRGB(0x10, 0xEB, 0xEB);
const D3DCOLOR RGB_BLUE         = D3DCOLOR_XRGB(0x10, 0x10, 0xEB);
const D3DCOLOR RGB_MAGENTA      = D3DCOLOR_XRGB(0xEB, 0x10, 0xEB);
const D3DCOLOR RGB_BLACK        = D3DCOLOR_XRGB(0x10, 0x10, 0x10);

// 75%
const D3DCOLOR RGB_WHITE_75pc   = D3DCOLOR_XRGB(0xB4, 0xB4, 0xB4);
const D3DCOLOR RGB_YELLOW_75pc  = D3DCOLOR_XRGB(0xB4, 0xB4, 0x10);
const D3DCOLOR RGB_CYAN_75pc    = D3DCOLOR_XRGB(0x10, 0xB4, 0xB4);
const D3DCOLOR RGB_GREEN_75pc   = D3DCOLOR_XRGB(0x10, 0xB4, 0x10);
const D3DCOLOR RGB_MAGENTA_75pc = D3DCOLOR_XRGB(0xB4, 0x10, 0xB4);
const D3DCOLOR RGB_RED_75pc     = D3DCOLOR_XRGB(0xB4, 0x10, 0x10);
const D3DCOLOR RGB_BLUE_75pc    = D3DCOLOR_XRGB(0x10, 0x10, 0xB4);

// -4% / +4%
const D3DCOLOR RGB_BLACK_n4pc   = D3DCOLOR_XRGB(0x07, 0x07, 0x07);
const D3DCOLOR RGB_BLACK_p4pc   = D3DCOLOR_XRGB(0x18, 0x18, 0x18);

// -Inphase / +Quadrature
const D3DCOLOR RGB_I            = D3DCOLOR_XRGB(0x00, 0x1D, 0x42);
const D3DCOLOR RGB_Q            = D3DCOLOR_XRGB(0x2C, 0x00, 0x5C);

inline DWORD RGBtoYUV(const D3DCOLOR rgb)
{
    const INT A = HIBYTE(HIWORD(rgb));
    const INT R = LOBYTE(HIWORD(rgb)) - 16;
    const INT G = HIBYTE(LOWORD(rgb)) - 16;
    const INT B = LOBYTE(LOWORD(rgb)) - 16;

    //
    // studio RGB [16...235] to SDTV ITU-R BT.601 YCbCr
    //
    INT Y = ( 77 * R + 150 * G +  29 * B + 128) / 256 + 16;
    INT U = (-44 * R -  87 * G + 131 * B + 128) / 256 + 128;
    INT V = (131 * R - 110 * G -  21 * B + 128) / 256 + 128;

    return D3DCOLOR_AYUV(A, Y, U, V);
}


inline DWORD RGBtoYUY2(const D3DCOLOR rgb)
{
    const D3DCOLOR yuv = RGBtoYUV(rgb);

    const BYTE Y = LOBYTE(HIWORD(yuv));
    const BYTE U = HIBYTE(LOWORD(yuv));
    const BYTE V = LOBYTE(LOWORD(yuv));

    return MAKELONG(MAKEWORD(Y, U), MAKEWORD(Y, V));
}

void FillRectangle(
    D3DLOCKED_RECT& lr,
    const UINT sx,
    const UINT sy,
    const UINT ex,
    const UINT ey,
    const DWORD color
    );

HRESULT DrawColorBars(IDirect3DSurface9 *pSurf, UINT width, UINT height);
HRESULT LoadBitmapResourceToAYUVSurface(IDirect3DSurface9 *pSurf, LONG width, LONG height, INT nIDBitmap, BYTE PixelAlphaValue);
HRESULT SetAYUVSurfacePixelAlpha(IDirect3DSurface9 *pSurf, UINT width, UINT height, BYTE PixelAlphaValue);

RECT    ScaleRectangle(const RECT& input, const RECT& src, const RECT& dst);
void    InflateRectBounded(RECT *prc, LONG cx, LONG cy, const RECT& rcBound);
void    MoveRectBounded(RECT *prc, LONG cx, LONG cy, const RECT& rcBound);
