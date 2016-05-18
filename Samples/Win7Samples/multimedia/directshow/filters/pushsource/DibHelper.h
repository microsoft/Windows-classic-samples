//------------------------------------------------------------------------------
// File: DibHelper.H
//
// Desc: DirectShow sample code - Helper code for bitmap manipulation
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#define HDIB HANDLE

/* DIB macros */
#define IS_WIN30_DIB(lpbi)  ((*(LPDWORD)(lpbi)) == sizeof(BITMAPINFOHEADER))
#define RECTWIDTH(lpRect)   ((lpRect)->right - (lpRect)->left)
#define RECTHEIGHT(lpRect)  ((lpRect)->bottom - (lpRect)->top)

// Function prototypes
HDIB BitmapToDIB (HBITMAP hBitmap, HPALETTE hPal);
HDIB ChangeBitmapFormat (HBITMAP	hBitmap,
                         WORD     wBitCount,
                         DWORD    dwCompression,
                         HPALETTE hPal);
HDIB ChangeDIBFormat (HDIB hDIB, WORD wBitCount, DWORD dwCompression);

HBITMAP CopyScreenToBitmap(LPRECT lpRect, BYTE *pData, BITMAPINFO *pHeader);
HDIB CopyScreenToDIB (LPRECT);
HBITMAP CopyWindowToBitmap (HWND, WORD);
HDIB CopyWindowToDIB (HWND, WORD);

HPALETTE CreateDIBPalette (HDIB);
HDIB CreateDIB(DWORD, DWORD, WORD);
WORD DestroyDIB (HDIB);

void DIBError (int ErrNo);
DWORD DIBHeight (LPSTR lpDIB);
WORD DIBNumColors (LPSTR lpDIB);
HBITMAP DIBToBitmap (HDIB hDIB, HPALETTE hPal);
DWORD DIBWidth (LPSTR lpDIB);

LPSTR FindDIBBits (LPSTR lpDIB);
HPALETTE GetSystemPalette (void);
HDIB LoadDIB (LPSTR);

BOOL PaintBitmap (HDC, LPRECT, HBITMAP, LPRECT, HPALETTE);
BOOL PaintDIB (HDC, LPRECT, HDIB, LPRECT, HPALETTE);

int PalEntriesOnDevice (HDC hDC);
WORD PaletteSize (LPSTR lpDIB);
WORD SaveDIB (HDIB, LPSTR);
