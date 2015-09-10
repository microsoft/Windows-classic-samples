// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "BaseWindow.h"
#include "ShadowWindow.h"

#define SHADOW_ALPHANUMBER (5)
#define ALPHA(x) (255 - (x))

//// Gray scale values for the shadow grades
#define GS01 255
#define GS02 254
#define GS03 253
#define GS04 252
#define GS05 250
#define GS06 246
#define GS07 245
#define GS08 242
#define GS09 241
#define GS10 227
#define GS11 217
#define GS12 213
#define GS13 212
#define GS14 199
#define GS15 180
#define GS16 172
#define GS17 171
#define GS18 155
#define GS19 144
#define GS20 142

// pre-computed alpha values for the shadow
const BYTE AlphaTable[SHADOW_ALPHANUMBER] = {
    ALPHA(GS04), ALPHA(GS09), ALPHA(GS13), ALPHA(GS17), ALPHA(GS20)
};

const BYTE CornerShadowAlphaMetric[SHADOW_ALPHANUMBER][SHADOW_ALPHANUMBER] = {
    ALPHA(GS08), ALPHA(GS06), ALPHA(GS05), ALPHA(GS03), ALPHA(GS02),
    ALPHA(GS11), ALPHA(GS10), ALPHA(GS09), ALPHA(GS05), ALPHA(GS02),
    ALPHA(GS15), ALPHA(GS14), ALPHA(GS10), ALPHA(GS07), ALPHA(GS03),
    ALPHA(GS18), ALPHA(GS15), ALPHA(GS11), ALPHA(GS08), ALPHA(GS03),
    ALPHA(GS19), ALPHA(GS16), ALPHA(GS12), ALPHA(GS09), ALPHA(GS04),
};


//+---------------------------------------------------------------------------
//
// _Create
//
//----------------------------------------------------------------------------

BOOL CShadowWindow::_Create(ATOM atom, DWORD dwExStyle, DWORD dwStyle, _In_opt_ CBaseWindow *pParent, int wndWidth, int wndHeight)
{
    if (!CBaseWindow::_Create(atom, dwExStyle, dwStyle, pParent, wndWidth, wndHeight))
    {
        return FALSE;
    }

    return _Initialize();
}

//+---------------------------------------------------------------------------
//
// _WindowProcCallback
//
// Shadow window proc.
//----------------------------------------------------------------------------

LRESULT CALLBACK CShadowWindow::_WindowProcCallback(_In_ HWND wndHandle, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_PAINT:
        {
            HDC dcHandle;
            PAINTSTRUCT ps;

            dcHandle = BeginPaint(wndHandle, &ps);

            HBRUSH hBrush = CreateSolidBrush(_color);
            FillRect(dcHandle, &ps.rcPaint, hBrush);
            DeleteObject(hBrush);

            EndPaint(wndHandle, &ps);
        }
        return 0;

    case WM_SETTINGCHANGE:
        _OnSettingChange();
        break;
    }

    return DefWindowProc(wndHandle, uMsg, wParam, lParam);
}

//+---------------------------------------------------------------------------
//
// _OnSettingChange
//
//----------------------------------------------------------------------------

void CShadowWindow::_OnSettingChange()
{
    _InitSettings();

    DWORD dwWndStyleEx = GetWindowLong(_GetWnd(), GWL_EXSTYLE);

    if (_isGradient)
    {
        SetWindowLong(_GetWnd(), GWL_EXSTYLE, (dwWndStyleEx | WS_EX_LAYERED));
    }
    else
    {
        SetWindowLong(_GetWnd(), GWL_EXSTYLE, (dwWndStyleEx & ~WS_EX_LAYERED));
    }

    _AdjustWindowPos();
    _InitShadow();
}

//+---------------------------------------------------------------------------
//
// _OnOwnerWndMoved
//
//----------------------------------------------------------------------------

void CShadowWindow::_OnOwnerWndMoved(BOOL isResized)
{
    if (IsWindow(_GetWnd()) && _IsWindowVisible())
    {
        _AdjustWindowPos();
        if (isResized)
        {
            _InitShadow();
        }
    }
}


//+---------------------------------------------------------------------------
//
// _Show
//
//----------------------------------------------------------------------------

void CShadowWindow::_Show(BOOL isShowWnd)
{
    _OnOwnerWndMoved(TRUE);
    CBaseWindow::_Show(isShowWnd);
}

//+---------------------------------------------------------------------------
//
// _Initialize
//
//----------------------------------------------------------------------------

BOOL CShadowWindow::_Initialize()
{
    _InitSettings();

    return TRUE;
}

//+---------------------------------------------------------------------------
//
// _InitSettings
//
//----------------------------------------------------------------------------

void CShadowWindow::_InitSettings()
{
    HDC dcHandle = GetDC(nullptr);

    // device caps
    int cBitsPixelScreen = GetDeviceCaps(dcHandle, BITSPIXEL);

    _isGradient = cBitsPixelScreen > 8;

    ReleaseDC(nullptr, dcHandle);

    if (_isGradient)
    {
        _color = RGB(0, 0, 0);
        _sizeShift.cx = SHADOW_ALPHANUMBER;
        _sizeShift.cy = SHADOW_ALPHANUMBER;
    }
    else
    {
        _color = RGB(128, 128, 128);
        _sizeShift.cx = 2;
        _sizeShift.cy = 2;
    }
}

//+---------------------------------------------------------------------------
//
// _AdjustWindowPos
//
//----------------------------------------------------------------------------

void CShadowWindow::_AdjustWindowPos()
{
    if (!IsWindow(_GetWnd()))
    {
        return;
    }

    HWND hWndOwner = _pWndOwner->_GetWnd();
    RECT rc = {0, 0, 0, 0};

    GetWindowRect(hWndOwner, &rc);
    SetWindowPos(_GetWnd(), hWndOwner,
        rc.left + _sizeShift.cx,
        rc.top  + _sizeShift.cy,
        rc.right - rc.left,
        rc.bottom - rc.top,
        SWP_NOOWNERZORDER | SWP_NOACTIVATE);
}

//+---------------------------------------------------------------------------
//
// _InitShadow
//
//----------------------------------------------------------------------------

#define GETRGBALPHA(_x_, _y_) ((RGBALPHA *)pDIBits + (_y_) * size.cx + (_x_))

void CShadowWindow::_InitShadow()
{
    typedef struct _RGBAPLHA {
        BYTE rgbBlue;
        BYTE rgbGreen;
        BYTE rgbRed;
        BYTE rgbAlpha;
    } RGBALPHA;

    HDC dcScreenHandle = nullptr;
    HDC dcLayeredHandle = nullptr;
    RECT rcWindow = {0, 0, 0, 0};
    SIZE size = {0, 0};
    BITMAPINFO bitmapInfo;
    HBITMAP bitmapMemHandle = nullptr;
    HBITMAP bitmapOldHandle = nullptr;
    void* pDIBits = nullptr;
    int i = 0;
    int j = 0;
    POINT ptSrc = {0, 0};
    POINT ptDst = {0, 0};
    BLENDFUNCTION Blend;

    if (!_isGradient)
    {
        return;
    }

    SetWindowLong(_GetWnd(), GWL_EXSTYLE, (GetWindowLong(_GetWnd(), GWL_EXSTYLE) | WS_EX_LAYERED));

    _GetWindowRect(&rcWindow);
    size.cx = rcWindow.right - rcWindow.left;
    size.cy = rcWindow.bottom - rcWindow.top;

    dcScreenHandle = GetDC(nullptr);
    if (dcScreenHandle == nullptr) {
        return;
    }

    dcLayeredHandle = CreateCompatibleDC(dcScreenHandle);
    if (dcLayeredHandle == nullptr) {
        ReleaseDC(nullptr, dcScreenHandle);
        return;
    }

    // create bitmap
    bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo.bmiHeader.biWidth = size.cx;
    bitmapInfo.bmiHeader.biHeight = size.cy;
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;
    bitmapInfo.bmiHeader.biSizeImage = 0;
    bitmapInfo.bmiHeader.biXPelsPerMeter = 100;
    bitmapInfo.bmiHeader.biYPelsPerMeter = 100;
    bitmapInfo.bmiHeader.biClrUsed = 0;
    bitmapInfo.bmiHeader.biClrImportant = 0;

    bitmapMemHandle = CreateDIBSection(dcScreenHandle, &bitmapInfo, DIB_RGB_COLORS, &pDIBits, nullptr, 0);
    if (pDIBits == nullptr || bitmapMemHandle == nullptr) {
        ReleaseDC(nullptr, dcScreenHandle);
        DeleteDC(dcLayeredHandle);
        return;
    }

    memset(pDIBits, 0, ((((32 * size.cx) + 31) & ~31) / 8) * size.cy);

    // edges
    for (i = 0; i < SHADOW_ALPHANUMBER; i++) {
        RGBALPHA *ppxl;
        BYTE bAlpha = AlphaTable[i];

        // bottom
        if (i <= (size.cy + 1)/2) {
            for (j = SHADOW_ALPHANUMBER; j < size.cx - SHADOW_ALPHANUMBER; j++) {
                ppxl = GETRGBALPHA(j, i);
                ppxl->rgbAlpha = bAlpha;
            }
        }

        // right
        if (i <= (size.cx + 1)/2) {
            for (j = SHADOW_ALPHANUMBER; j < size.cy - SHADOW_ALPHANUMBER; j++) {
                ppxl = GETRGBALPHA(size.cx - 1 - i, j);
                ppxl->rgbAlpha = bAlpha;
            }
        }
    }

    // corners
    for (i = 0; i < SHADOW_ALPHANUMBER; i++) {
        for (j = 0; j < SHADOW_ALPHANUMBER; j++) {
            RGBALPHA *ppxl;
            BYTE bAlpha;
            bAlpha = CornerShadowAlphaMetric[i][SHADOW_ALPHANUMBER - j - 1];

            // top-right
            if ((i <= (size.cy + 1)/2) && (j <= (size.cx + 1)/2)) {
                ppxl = GETRGBALPHA(size.cx - 1 - j, size.cy - 1 - i);
                ppxl->rgbAlpha = bAlpha;
            }

            // bottom-left
            if ((i <= (size.cy + 1)/2) && (j <= (size.cx + 1)/2)) {
                ppxl = GETRGBALPHA(j, i + 1);
                ppxl->rgbAlpha = bAlpha;
            }

            // bottom-right
            if ((i <= (size.cy + 1)/2) && (j <= (size.cx + 1)/2)) {
                ppxl = GETRGBALPHA(size.cx - 1 - j, i + 1);
                ppxl->rgbAlpha = bAlpha;
            }
        }
    }

    ptSrc.x = 0;
    ptSrc.y = 0;
    ptDst.x = rcWindow.left;
    ptDst.y = rcWindow.top;
    Blend.BlendOp = AC_SRC_OVER;
    Blend.BlendFlags = 0;
    Blend.SourceConstantAlpha = 255;
    Blend.AlphaFormat = AC_SRC_ALPHA;

    bitmapOldHandle = (HBITMAP)SelectObject(dcLayeredHandle, bitmapMemHandle);

    UpdateLayeredWindow(_GetWnd(), dcScreenHandle, nullptr, &size, dcLayeredHandle, &ptSrc, 0, &Blend, ULW_ALPHA);

    SelectObject(dcLayeredHandle, bitmapOldHandle);

    // done
    ReleaseDC(nullptr, dcScreenHandle);
    DeleteDC(dcLayeredHandle);
    DeleteObject(bitmapMemHandle);
}
