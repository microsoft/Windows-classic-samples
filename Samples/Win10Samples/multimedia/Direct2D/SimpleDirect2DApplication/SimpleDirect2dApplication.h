// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

#pragma once

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER              // Allow use of features specific to Windows 7 or later.
#define WINVER NTDDI_WIN10 // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT        // Allow use of features specific to Windows 7 or later.
#define _WIN32_WINNT _WIN32_WINNT_WIN10 // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <wchar.h>
#include <math.h>

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

/******************************************************************
*                                                                 *
*  Macros                                                         *
*                                                                 *
******************************************************************/

template<class Interface>
inline void
SafeRelease(
    Interface **ppInterfaceToRelease
    )
{
    if (*ppInterfaceToRelease != NULL)
    {
        (*ppInterfaceToRelease)->Release();

        (*ppInterfaceToRelease) = NULL;
    }
}

#ifndef Assert
#if defined( DEBUG ) || defined( _DEBUG )
#define Assert(b) do {if (!(b)) {OutputDebugStringA("Assert: " #b "\n");}} while(0)
#else
#define Assert(b)
#endif //DEBUG || _DEBUG
#endif


#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

/******************************************************************
*                                                                 *
*  DemoApp                                                        *
*                                                                 *
******************************************************************/

class DemoApp
{
public:
    DemoApp();
    ~DemoApp();

    HRESULT Initialize();

    void RunMessageLoop();

private:
    HRESULT CreateDeviceIndependentResources();
    HRESULT CreateDeviceResources();

    HRESULT CreateGridPatternBrush(
        ID2D1RenderTarget *pRenderTarget,
        ID2D1BitmapBrush **ppBitmapBrush
        );

    void DiscardDeviceResources();

    HRESULT OnRender();

    void OnResize(
        UINT width,
        UINT height
        );

    static LRESULT CALLBACK WndProc(
        HWND hWnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
        );

    HRESULT LoadResourceBitmap(
        ID2D1RenderTarget *pRenderTarget,
        IWICImagingFactory *pIWICFactory,
        PCWSTR resourceName,
        PCWSTR resourceType,
        UINT destinationWidth,
        UINT destinationHeight,
        ID2D1Bitmap **ppBitmap
        );

    HRESULT LoadBitmapFromFile(
        ID2D1RenderTarget *pRenderTarget,
        IWICImagingFactory *pIWICFactory,
        PCWSTR uri,
        UINT destinationWidth,
        UINT destinationHeight,
        ID2D1Bitmap **ppBitmap
        );

private:
    HWND m_hwnd;
    ID2D1Factory *m_pD2DFactory;
    IWICImagingFactory *m_pWICFactory;
    IDWriteFactory *m_pDWriteFactory;
    ID2D1HwndRenderTarget *m_pRenderTarget;
    IDWriteTextFormat *m_pTextFormat;
    ID2D1PathGeometry *m_pPathGeometry;
    ID2D1LinearGradientBrush *m_pLinearGradientBrush;
    ID2D1SolidColorBrush *m_pBlackBrush;
    ID2D1BitmapBrush *m_pGridPatternBitmapBrush;
    ID2D1Bitmap *m_pBitmap;
    ID2D1Bitmap *m_pAnotherBitmap;
};



