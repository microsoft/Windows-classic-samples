// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER              // Allow use of features specific to Windows XP or later.
#define WINVER 0x0501       // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT        // Allow use of features specific to Windows XP or later.
#define _WIN32_WINNT 0x0501 // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINDOWS      // Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE           // Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0700    // Change this to the appropriate value to target other versions of IE.
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

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

#include <shobjidl.h>
#include <objbase.h>
#include <strsafe.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include <new> // std::nothrow

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
#if defined( DEBUG ) || defined( _DEBUG ) || DBG
#define Assert(b) if (!(b)) {OutputDebugStringA("Assert: " #b "\n");}
#else
#define Assert(b)
#endif //DEBUG || _DEBUG
#endif

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

class ListViewApp
{
public:
    ListViewApp();
    ~ListViewApp();

    HRESULT Initialize();

    void RunMessageLoop();

private:
    HRESULT CreateDeviceIndependentResources();
    HRESULT CreateDeviceResources();
    void DiscardDeviceResources();

    HRESULT OnRender();

    void OnResize();

    void OnChar(
        SHORT aChar
        );

    void OnVScroll(
        WPARAM wParam,
        LPARAM lParam
        );

    UINT GetScrollRange();
    UINT GetScrollPos();

    void OnMouseWheel(
        WPARAM wParam,
        LPARAM lParam
        );

    void OnLeftButtonDown(
        D2D1_POINT_2F diPosition
        );

    D2D1_SIZE_U CalculateD2DWindowSize();

    static LRESULT CALLBACK ChildWndProc(
        HWND hWnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
        );

    static LRESULT CALLBACK ParentWndProc(
        HWND hWnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
        );

    HRESULT LoadDirectory();

    static FLOAT GetFancyAccelerationInterpolatedValue(
        FLOAT linearFactor,
        FLOAT p1,
        FLOAT p2
        );

    FLOAT GetAnimatingItemInterpolationFactor();
    FLOAT GetAnimatingScrollInterpolationFactor();
    FLOAT GetInterpolatedScrollPosition();

    D2D1_POINT_2F GetScrolledDIPositionFromPixelPosition(
        D2D1_POINT_2U
        );


    /******************************************************************
    *                                                                 *
    *  Comparator functions (for sorting)                             *
    *                                                                 *
    ******************************************************************/

    static int __cdecl CompareAToZ(
        const void *a,
        const void *b
        );

    static int __cdecl CompareZToA(
        const void *a,
        const void *b
        );

    static int __cdecl CompareDirFirstAToZ(
        const void *a,
        const void *b
        );

private:
    HWND m_d2dHwnd;
    HWND m_parentHwnd;

    ID2D1Factory *m_pD2DFactory;
    IWICImagingFactory *m_pWICFactory;
    IDWriteFactory *m_pDWriteFactory;
    ID2D1HwndRenderTarget *m_pRT;
    IDWriteTextFormat *m_pTextFormat;
    ID2D1SolidColorBrush *m_pBlackBrush;

    IBindCtx *m_pBindContext;

    ID2D1Bitmap *m_pBitmapAtlas;


    // Size of bitmap atlas (in pixels)
    static const UINT msc_atlasWidth = 2048;
    static const UINT msc_atlasHeight = 2048;

    // Width/Height of each icon
    static const UINT msc_iconSize = 48;

    // Space between each item
    static const UINT msc_lineSpacing = 10;

    // Number of frames to show while animating item repositioning
    static const UINT msc_totalAnimatingItemFrames = 60;

    // Number of frames to show while animating scrolls
    static const UINT msc_totalAnimatingScrollFrames = 10;

    // Static size of item info array
    static const UINT msc_maxItemInfos = msc_atlasHeight * msc_atlasWidth / (msc_iconSize * msc_iconSize);


    /******************************************************************
    *                                                                 *
    *  ItemInfo                                                       *
    *                                                                 *
    ******************************************************************/

    class ItemInfo
    {
    public:
        ItemInfo()
        {
            szFilename[0] = L'\0';
            ZeroMemory(&placement, sizeof(placement));
            currentPosition = 0.0f;
            previousPosition = 0.0f;
            isDirectory = false;
        }

        D2D1_RECT_U placement;
        WCHAR szFilename[MAX_PATH];
        FLOAT currentPosition;
        FLOAT previousPosition;
        bool isDirectory;
    };

    ItemInfo m_pFiles[msc_maxItemInfos];

    // Number of item infos actually loaded (<= msc_maxItemInfos)
    UINT m_numItemInfos;

    // Maximum scroll amount
    UINT m_scrollRange;

    // m_currentScrollPos is the current scroll position. We animate to the
    // current scroll position from the previous scroll position,
    // m_previousScrollPos, interpolating between the two based on the factor
    // m_animatingItems / msc_totalAnimatingScrollFrames.
    INT m_previousScrollPos;
    INT m_currentScrollPos;
    UINT m_animatingScroll;

    // m_animatingItems / msc_totalAnimatingItemFrames is the interpolation
    // factor for animating between the previousPosition and currentPosition of
    // each ItemInfo
    UINT m_animatingItems;
};
