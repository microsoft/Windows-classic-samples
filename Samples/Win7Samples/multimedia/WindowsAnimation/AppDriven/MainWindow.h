// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#pragma once

#include <UIAnimation.h>
#include <D2D1.h>

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
    HRESULT CreateAnimationVariables();

    HRESULT CreateDeviceIndependentResources();
    HRESULT CreateDeviceResources();
    void DiscardDeviceResources();

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
    
    HRESULT OnLButtonDown();

    HRESULT OnResize(
        UINT width,
        UINT height
        );
    
    void OnDestroy();
    
    HRESULT DrawClientArea();
    
    HRESULT DrawBackground(
        const D2D1_RECT_F &rectPaint
        );

    HRESULT ChangeColor(
        DOUBLE red,
        DOUBLE green,
        DOUBLE blue
        );

    DOUBLE RandomFromRange(
        DOUBLE minimum,
        DOUBLE maximum
        );

private:

    HWND m_hwnd;
    
    // D2D components

    ID2D1Factory *m_pD2DFactory;
    ID2D1HwndRenderTarget *m_pRenderTarget;
    ID2D1SolidColorBrush *m_pBackgroundBrush;

    // Animation components

    IUIAnimationManager *m_pAnimationManager;
    IUIAnimationTimer *m_pAnimationTimer;
    IUIAnimationTransitionLibrary *m_pTransitionLibrary;

    // Animated Variables

    IUIAnimationVariable *m_pAnimationVariableRed;
    IUIAnimationVariable *m_pAnimationVariableGreen;
    IUIAnimationVariable *m_pAnimationVariableBlue;
};
