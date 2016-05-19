// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#pragma once

#include "Thumbnail.h"
#include "LayoutManager.h"

#include <UIAnimation.h>
#include <D2D1.h>

#include <D2D1helper.h>
#include <DWrite.h>
#include <WinCodec.h>

#include <ThumbCache.h>
#include <ShlObj.h>

// Main application window

class CMainWindow
{
public:

    CMainWindow();
    ~CMainWindow();

    HRESULT Initialize(
        HINSTANCE hInstance
        );
    
    HRESULT Invalidate();

protected:

    HRESULT InitializeAnimation();

    HRESULT CreateDeviceIndependentResources();
    HRESULT CreateDeviceResources();
    void DiscardDeviceResources();

    HRESULT FindImages();

    HRESULT DecodeImageFromThumbCache(
        IShellItem *pShellItem,
        ID2D1Bitmap **ppBitmap
        );

    static LRESULT CALLBACK WndProc(
        HWND hwnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );

    HRESULT OnPaint(
        HDC hdc,
        const RECT &rcPaint
        );

    HRESULT OnResize(
        UINT width,
        UINT height
        );

    HRESULT OnKeyDown(
        UINT uVirtKey
        );

    void OnDestroy();

    HRESULT DrawClientArea();

private:

    HWND m_hwnd;
    
    // D2D components

    ID2D1Factory *m_pD2DFactory;
    ID2D1HwndRenderTarget *m_pRenderTarget;
    ID2D1LinearGradientBrush *m_pBackgroundBrush;
    ID2D1SolidColorBrush *m_pOutlineBrush;

    // Animation components

    IUIAnimationManager *m_pAnimationManager;
    IUIAnimationTimer *m_pAnimationTimer;
    IUIAnimationTransitionLibrary *m_pTransitionLibrary;
    IUIAnimationTransitionFactory *m_pTransitionFactory;

    // Thumbnail array

    UINT m_uThumbCount;
    CThumbnail *m_thumbs;

    // Layout Manager

    CLayoutManager *m_pLayoutManager;
};
