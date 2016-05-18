//------------------------------------------------------------------------------
// File: DibHelper.cpp
//
// Desc: DirectShow sample code - In-memory push mode source filter
//       Helper routines for manipulating bitmaps.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <windows.h>

#include "dibhelper.h"


HBITMAP CopyScreenToBitmap(LPRECT lpRect, BYTE *pData, BITMAPINFO *pHeader)
{
    HDC         hScrDC, hMemDC;         // screen DC and memory DC
    HBITMAP     hBitmap, hOldBitmap;    // handles to deice-dependent bitmaps
    int         nX, nY, nX2, nY2;       // coordinates of rectangle to grab
    int         nWidth, nHeight;        // DIB width and height
    int         xScrn, yScrn;           // screen resolution

    // check for an empty rectangle
    if (IsRectEmpty(lpRect))
      return NULL;

    // create a DC for the screen and create
    // a memory DC compatible to screen DC   
    hScrDC = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
    hMemDC = CreateCompatibleDC(hScrDC);

    // get points of rectangle to grab
    nX  = lpRect->left;
    nY  = lpRect->top;
    nX2 = lpRect->right;
    nY2 = lpRect->bottom;

    // get screen resolution
    xScrn = GetDeviceCaps(hScrDC, HORZRES);
    yScrn = GetDeviceCaps(hScrDC, VERTRES);

    //make sure bitmap rectangle is visible
    if (nX < 0)
        nX = 0;
    if (nY < 0)
        nY = 0;
    if (nX2 > xScrn)
        nX2 = xScrn;
    if (nY2 > yScrn)
        nY2 = yScrn;

    nWidth  = nX2 - nX;
    nHeight = nY2 - nY;

    // create a bitmap compatible with the screen DC
    hBitmap = CreateCompatibleBitmap(hScrDC, nWidth, nHeight);

    // select new bitmap into memory DC
    hOldBitmap = (HBITMAP) SelectObject(hMemDC, hBitmap);

    // bitblt screen DC to memory DC
    BitBlt(hMemDC, 0, 0, nWidth, nHeight, hScrDC, nX, nY, SRCCOPY);

    // select old bitmap back into memory DC and get handle to
    // bitmap of the screen   
    hBitmap = (HBITMAP)  SelectObject(hMemDC, hOldBitmap);

    // Copy the bitmap data into the provided BYTE buffer
    GetDIBits(hScrDC, hBitmap, 0, nHeight, pData, pHeader, DIB_RGB_COLORS);

    // clean up
    DeleteDC(hScrDC);
    DeleteDC(hMemDC);

    // return handle to the bitmap
    return hBitmap;
}

