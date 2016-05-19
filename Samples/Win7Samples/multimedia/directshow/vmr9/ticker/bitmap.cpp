//------------------------------------------------------------------------------
// File: Bitmap.cpp
//
// Desc: DirectShow sample code - Bitmap manipulation routines for 
//       VMR alpha-blended bitmap
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

//#include <atlbase.h>
#include <tchar.h>
#include <dshow.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>
#include <strsafe.h>

#include "ticker.h"
#include "bitmap.h"
#include "resource.h"

//
// Constants
//
const float EDGE_BUFFER=0.04f; // Pixel buffer between bitmap and window edge
                               // (represented in composition space [0 - 1.0f])

const float SLIDE_VALUE = 0.05f;  // Amount to slide image in composition space

const int SLIDE_TIMER   = 2001;
const int SLIDE_TIMEOUT = 125;   // 125 ms between ticker movements

//
// Global data
//
IVMRMixerBitmap9 *pBMP = NULL;
DWORD g_dwTickerFlags=0;
int gnSlideTimer=0;
TCHAR g_szAppText[DYNAMIC_TEXT_SIZE]={0};

float g_fBitmapCompWidth=0;  // Width of bitmap in composition space units
int g_nImageWidth=0;         // Width of text bitmap

// Text font information
HFONT g_hFont=0;
LONG g_lFontPointSize   = DEFAULT_FONT_SIZE;
TCHAR g_szFontName[100] = {DEFAULT_FONT_NAME};
TCHAR g_szFontStyle[32] = {DEFAULT_FONT_STYLE};
COLORREF g_rgbColors    = DEFAULT_FONT_COLOR;

// Destination rectangle used for alpha-blended text
VMR9NormalizedRect  g_rDest={0};


HRESULT BlendApplicationImage(HWND hwndApp)
{
    LONG cx, cy;
    HRESULT hr;

    // Read the default video size
    hr = pWC->GetNativeVideoSize(&cx, &cy, NULL, NULL);
    if (FAILED(hr))
    {
        Msg(TEXT("GetNativeVideoSize FAILED!  hr=0x%x\r\n"), hr);
        return hr;
    }

    // Load the bitmap to alpha blend from the resource file
    HBITMAP hbm = LoadBitmap(ghInst, MAKEINTRESOURCE(IDR_TICKER));

    // Create a device context compatible with the current window
    HDC hdc = GetDC(hwndApp);
    HDC hdcBmp = CreateCompatibleDC(hdc);
    ReleaseDC(hwndApp, hdc);

    // Select our bitmap into the device context and save the old one
    BITMAP bm;
    HBITMAP hbmOld;
    GetObject(hbm, sizeof(bm), &bm);
    hbmOld = (HBITMAP)SelectObject(hdcBmp, hbm);

    // Configure the VMR's bitmap structure
    VMR9AlphaBitmap bmpInfo;
    ZeroMemory(&bmpInfo, sizeof(bmpInfo) );
    bmpInfo.dwFlags = VMRBITMAP_HDC;
    bmpInfo.hdc = hdcBmp;

    // Remember the width of this new bitmap
    g_nImageWidth = bm.bmWidth;

    // Save the ratio of the bitmap's width to the width of the video file.
    // This value is used to reposition the bitmap in composition space.
    g_fBitmapCompWidth = (float)g_nImageWidth / (float)cx;

    // Display the bitmap in the bottom right corner.
    // rSrc specifies the source rectangle in the GDI device context 
    // rDest specifies the destination rectangle in composition space (0.0f to 1.0f)
    SetRect(&bmpInfo.rSrc, 0, 0, g_nImageWidth, bm.bmHeight);
    bmpInfo.rDest.left   = 1.0f;
    bmpInfo.rDest.right  = 1.0f + g_fBitmapCompWidth;
    bmpInfo.rDest.top    = (float)(cy - bm.bmHeight) / (float)cy - EDGE_BUFFER;
    bmpInfo.rDest.bottom = 1.0f - EDGE_BUFFER;

    // Copy initial settings to global memory for later modification
    g_rDest = bmpInfo.rDest;

    // Transparency value 1.0 is opaque, 0.0 is transparent.
    bmpInfo.fAlpha = TRANSPARENCY_VALUE;

    // Set the COLORREF so that the bitmap outline will be transparent
    SetColorRef(bmpInfo);

    // Give the bitmap to the VMR for display
    hr = pBMP->SetAlphaBitmap(&bmpInfo);
    if (FAILED(hr))
    {
        Msg(TEXT("SetAlphaBitmap FAILED!  hr=0x%x\r\n\r\n%s\0"),
            hr, STR_VMR_DISPLAY_WARNING);
    }

    // Select the initial object back into our device context
    DeleteObject(SelectObject(hdcBmp, hbmOld));

    // Clean up resources
    DeleteObject(hbm);
    DeleteDC(hdcBmp);

    return hr;
}


void ClearTickerState(void)
{
    if (gnSlideTimer)
    {
        KillTimer(NULL, gnSlideTimer);
        gnSlideTimer = 0;
    }

    // Clear current ticker flags
    g_dwTickerFlags = 0;

    // Enable all Ticker menu items, which may have been
    // disabled if the ticker was currently disabled
    EnableMenuItem(ghMenu, ID_SLIDE,   MF_ENABLED);
    EnableMenuItem(ghMenu, ID_TICKER_STATIC_IMAGE, MF_ENABLED);
    EnableMenuItem(ghMenu, ID_TICKER_DYNAMIC_TEXT, MF_ENABLED);
    EnableMenuItem(ghMenu, ID_SET_FONT, MF_ENABLED);
    EnableMenuItem(ghMenu, ID_SET_TEXT, MF_ENABLED);

    // Clear menu check marks
    CheckMenuItem(ghMenu, ID_SLIDE,   MF_UNCHECKED);
}


HRESULT DisableTicker(DWORD dwFlags)
{
    HRESULT hr;
    VMR9AlphaBitmap bmpInfo={0};

    // Read the current bitmap settings
    hr = pBMP->GetAlphaBitmapParameters(&bmpInfo);
    if (FAILED(hr))
        Msg(TEXT("GetAlphaBitmapParameters FAILED!  hr=0x%x\r\n"), hr);

    // If the request is to disable the bitmap, then disable it
    // and ignore the other flags for now
    if (dwFlags & MARK_DISABLE)
    {
        // Temporarily disable bitmap display
        bmpInfo.dwFlags = VMRBITMAP_DISABLE;

        // Disable other ticker menu items
        EnableMenuItem(ghMenu, ID_SLIDE,   MF_GRAYED);
        EnableMenuItem(ghMenu, ID_TICKER_STATIC_IMAGE, MF_GRAYED);
        EnableMenuItem(ghMenu, ID_TICKER_DYNAMIC_TEXT, MF_GRAYED);
        EnableMenuItem(ghMenu, ID_SET_FONT, MF_GRAYED);
        EnableMenuItem(ghMenu, ID_SET_TEXT, MF_GRAYED);

        // Update the bitmap settings
        hr = pBMP->UpdateAlphaBitmapParameters(&bmpInfo);
        if (FAILED(hr))
            Msg(TEXT("UpdateAlphaBitmapParameters FAILED to disable ticker!  hr=0x%x\r\n"), hr);
    }
    else
    {
        // Reset the bitmap with default values
        if (g_dwTickerFlags & MARK_STATIC_IMAGE)
            hr = BlendApplicationImage(ghApp);
        else
            hr = BlendApplicationText(ghApp, g_szAppText);

        // Reenable other ticker menu items
        EnableMenuItem(ghMenu, ID_SLIDE,   MF_ENABLED);
        EnableMenuItem(ghMenu, ID_TICKER_STATIC_IMAGE, MF_ENABLED);
        EnableMenuItem(ghMenu, ID_TICKER_DYNAMIC_TEXT, MF_ENABLED);
        EnableMenuItem(ghMenu, ID_SET_FONT, MF_ENABLED);
        EnableMenuItem(ghMenu, ID_SET_TEXT, MF_ENABLED);
     }

    return hr;
}


void FlipFlag(DWORD dwFlag)
{
    // If the flag is set, clear it
    if (g_dwTickerFlags & dwFlag)
        g_dwTickerFlags &= ~dwFlag;

    // Otherwise, set the flag
    else
        g_dwTickerFlags |= dwFlag;
}


void SlideTicker(DWORD dwFlags)
{
    if (dwFlags & MARK_SLIDE)
    {
        gnSlideTimer = (int) SetTimer(NULL, SLIDE_TIMER, SLIDE_TIMEOUT, TimerProc);
        CheckMenuItem(ghMenu, ID_SLIDE, MF_CHECKED);
    }
    else
    {
        if (gnSlideTimer)
        {
            KillTimer(NULL, gnSlideTimer);
            gnSlideTimer=0;
        }
        CheckMenuItem(ghMenu, ID_SLIDE, MF_UNCHECKED);
        ResetBitmapPosition();
    }
}


VOID CALLBACK TimerProc(
  HWND hwnd,         // handle to window
  UINT uMsg,         // WM_TIMER message
  UINT_PTR idEvent,  // timer identifier
  DWORD dwTime       // current system time
)
{
    HandleSlide();
}


void HandleSlide(void)
{
    HRESULT hr;
    VMR9AlphaBitmap bmpInfo={0};

    hr = pBMP->GetAlphaBitmapParameters(&bmpInfo);
    if (FAILED(hr))
        Msg(TEXT("GetAlphaBitmapParameters FAILED!  hr=0x%x\r\n"), hr);

    // Slowly decrease the X coordinate
    bmpInfo.rDest.left  -= SLIDE_VALUE;   
    bmpInfo.rDest.right -= SLIDE_VALUE;

    // Once the bitmap disappears off the left side of the screen,
    // reset to the rightmost side of the window.
    if ((bmpInfo.rDest.right <= EDGE_BUFFER))
    {
        bmpInfo.rDest.left = 1.0f;
        bmpInfo.rDest.right = 1.0f + g_fBitmapCompWidth;
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

    hr = pBMP->GetAlphaBitmapParameters(&bmpInfo);
    if (FAILED(hr))
        Msg(TEXT("GetAlphaBitmapParameters FAILED!  hr=0x%x\r\n"), hr);

    // Return bitmap position to its original value.
    bmpInfo.rDest.left  = g_rDest.left;
    bmpInfo.rDest.right = g_rDest.right;

    // Set the COLORREF so that the bitmap outline will be transparent
    SetColorRef(bmpInfo);

    // If the bitmap is currently disabled, this call will fail
    hr = pBMP->UpdateAlphaBitmapParameters(&bmpInfo);
}


void SetColorRef(VMR9AlphaBitmap& bmpInfo)
{
    // Set the COLORREF so that the bitmap outline will be transparent
    bmpInfo.clrSrcKey = RGB(255, 255, 255);  // Pure white
    bmpInfo.dwFlags |= VMRBITMAP_SRCCOLORKEY;
}


void EnableTickerMenu(BOOL bEnable)
{
    CheckMenuItem( ghMenu, ID_SLIDE,    bEnable ? MF_CHECKED : MF_UNCHECKED);

    EnableMenuItem(ghMenu, ID_SLIDE,    bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(ghMenu, ID_DISABLE,  bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(ghMenu, ID_SET_FONT, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(ghMenu, ID_SET_TEXT, bEnable ? MF_ENABLED : MF_GRAYED);

    EnableMenuItem(ghMenu, ID_TICKER_STATIC_IMAGE, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(ghMenu, ID_TICKER_DYNAMIC_TEXT, bEnable ? MF_ENABLED : MF_GRAYED);
}


HFONT UserSelectFont( void ) 
{
    // Allow the user to specify the text font to use with
    // dynamic text scrolling.  Display the Windows ChooseFont() dialog.
    return (SetTextFont(TRUE));
}


HFONT SetTextFont(BOOL bShowDialog) 
{ 
    CHOOSEFONT cf={0}; 
    LOGFONT lf={0}; 
    HFONT hfont; 
    HDC hdc;
    LONG lHeight;

    // Convert requested font point size to logical units
    hdc = GetDC( ghApp );
    lHeight = -MulDiv( g_lFontPointSize, GetDeviceCaps(hdc, LOGPIXELSY), 72 );
    ReleaseDC( ghApp, hdc );

    // Initialize members of the LOGFONT structure. 
    StringCchCopy(lf.lfFaceName, LF_FACESIZE, g_szFontName);
    lf.lfHeight = lHeight;      // Logical units

    // Prevent font smoothing, which could distort the text and leave
    // white pixels on the edges.  Disabling antialiasing leads to 
    // smoother text in this context.
    lf.lfQuality = NONANTIALIASED_QUALITY;

    // Initialize members of the CHOOSEFONT structure. 
    cf.lStructSize = sizeof(CHOOSEFONT); 
    cf.hwndOwner   = ghApp; 
    cf.hDC         = (HDC)NULL; 
    cf.lpLogFont   = &lf; 
    cf.iPointSize  = g_lFontPointSize * 10; 
    cf.rgbColors   = g_rgbColors; 
    cf.lCustData   = 0L; 
    cf.lpfnHook    = (LPCFHOOKPROC)NULL; 
    cf.hInstance   = (HINSTANCE) NULL; 
    cf.lpszStyle   = g_szFontStyle; 
    cf.nFontType   = SCREEN_FONTTYPE; 
    cf.nSizeMin    = 0; 
    cf.lpTemplateName = NULL; 
    cf.Flags = CF_SCREENFONTS | CF_SCALABLEONLY | CF_INITTOLOGFONTSTRUCT | 
               CF_EFFECTS     | CF_USESTYLE     | CF_LIMITSIZE; 

    // Limit font size to prevent bitmap from becoming too wide
    cf.nSizeMax = MAX_FONT_SIZE; 
 
    // If we previously changed a pure white font to 'almost white'
    // to support writing white text over a white colorkey, then
    // configure the font dialog for pure white text.
    if (cf.rgbColors == ALMOST_WHITE)
        cf.rgbColors = PURE_WHITE;

    // Display the CHOOSEFONT common-dialog box.  When it closes,
    // the CHOOSEFONT structure members will be updated.
    if (bShowDialog)
        ChooseFont(&cf); 

    // Save the user's selections for configuring the dialog box next time.
    // The style is automatically saved in g_szFontStyle (cf.lpszStyle)
    (void)StringCchCopy(g_szFontName, NUMELMS(g_szFontName), lf.lfFaceName);
    g_lFontPointSize = cf.iPointSize / 10;  // Specified in 1/10 point units
    g_rgbColors = cf.rgbColors;

    // Because we use a white colorkey to introduce transparency behind
    // our text, drawing white text will cause it to be transparent.
    // Therefore, filter out pure white (RGB(255,255,255)).
    if (g_rgbColors == PURE_WHITE)
        g_rgbColors = ALMOST_WHITE;

    // Create a logical font based on the user's selection and 
    // return a handle identifying that font.  
    hfont = CreateFontIndirect(cf.lpLogFont); 
    return (hfont); 
} 


HRESULT BlendApplicationText(HWND hwndApp, TCHAR *szNewText)
{
    LONG cx, cy;
    HRESULT hr;

    // Read the default video size
    hr = pWC->GetNativeVideoSize(&cx, &cy, NULL, NULL);
    if (FAILED(hr))
      return hr;

    // Create a device context compatible with the current window
    HDC hdc = GetDC(hwndApp);
    HDC hdcBmp = CreateCompatibleDC(hdc);

    // Write with a known font by selecting it into our HDC
    HFONT hOldFont = (HFONT) SelectObject(hdcBmp, g_hFont);

    // Determine the length of the string, then determine the
    // dimensions (in pixels) of the character string using the
    // currently selected font.  These dimensions are used to create
    // a bitmap below.
    int nLength, nTextBmpWidth, nTextBmpHeight;
    SIZE sz={0};
    nLength = (int) _tcslen(szNewText);
    GetTextExtentPoint32(hdcBmp, szNewText, nLength, &sz);
    nTextBmpHeight = sz.cy;
    nTextBmpWidth  = sz.cx;

    // Create a new bitmap that is compatible with the current window
    HBITMAP hbm = CreateCompatibleBitmap(hdc, nTextBmpWidth, nTextBmpHeight);
    ReleaseDC(hwndApp, hdc);

    // Select our bitmap into the device context and save the old one
    BITMAP bm;
    HBITMAP hbmOld;
    GetObject(hbm, sizeof(bm), &bm);
    hbmOld = (HBITMAP)SelectObject(hdcBmp, hbm);

    // Set initial bitmap settings
    RECT rcText;
    SetRect(&rcText, 0, 0, nTextBmpWidth, nTextBmpHeight);
    SetBkColor(hdcBmp, RGB(255, 255, 255)); // Pure white background
    SetTextColor(hdcBmp, g_rgbColors);      // Write text with requested color

    // Draw the requested text string onto the bitmap
    TextOut(hdcBmp, 0, 0, szNewText, nLength);

    // Configure the VMR's bitmap structure
    VMR9AlphaBitmap bmpInfo;
    ZeroMemory(&bmpInfo, sizeof(bmpInfo) );
    bmpInfo.dwFlags = VMRBITMAP_HDC;
    bmpInfo.hdc = hdcBmp;  // DC which has selected our bitmap

    // Remember the width of this new bitmap
    g_nImageWidth = bm.bmWidth;

    // Save the ratio of the bitmap's width to the width of the video file.
    // This value is used to reposition the bitmap in composition space.
    g_fBitmapCompWidth = (float)g_nImageWidth / (float)cx;

    // Display the bitmap in the bottom right corner.
    // rSrc specifies the source rectangle in the GDI device context 
    // rDest specifies the destination rectangle in composition space (0.0f to 1.0f)
    bmpInfo.rDest.left  = 1.0f;
    bmpInfo.rDest.right = 1.0f + g_fBitmapCompWidth;
    bmpInfo.rDest.top = (float)(cy - bm.bmHeight) / (float)cy - EDGE_BUFFER;
    bmpInfo.rDest.bottom = 1.0f - EDGE_BUFFER;
    bmpInfo.rSrc = rcText;

    // Copy initial settings to global memory for later modification
    g_rDest = bmpInfo.rDest;

    // Transparency value 1.0 is opaque, 0.0 is transparent.
    bmpInfo.fAlpha = TRANSPARENCY_VALUE;

    // Set the COLORREF so that the bitmap outline will be transparent
    SetColorRef(bmpInfo);

    // Give the bitmap to the VMR for display
    hr = pBMP->SetAlphaBitmap(&bmpInfo);
    if (FAILED(hr))
        Msg(TEXT("SetAlphaBitmap FAILED!  hr=0x%x\r\n\r\n%s\0"), hr,
            STR_VMR_DISPLAY_WARNING);

    // Select the initial objects back into our device context
    DeleteObject(SelectObject(hdcBmp, hbmOld));
    SelectObject(hdc, hOldFont);

    // Clean up resources
    DeleteObject(hbm);
    DeleteDC(hdcBmp);

    return hr;
}


