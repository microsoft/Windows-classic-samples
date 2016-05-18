// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#pragma once

#include <UIAnimation.h>
#include <GDIPlus.h>

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
    
    void OnDestroy();

    HRESULT DrawClientArea(
        HDC hdc,
        const RECT &rcPaint
        );

    HRESULT DrawBackground(
        Gdiplus::Graphics &graphics,
        const Gdiplus::RectF &rectPaint
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
        
    HRESULT HrFromStatus(
        Gdiplus::Status status
        );

private:

    HWND m_hwnd;

    // Animation components

    IUIAnimationManager *m_pAnimationManager;
    IUIAnimationTimer *m_pAnimationTimer;
    IUIAnimationTransitionLibrary *m_pTransitionLibrary;

    // Animated Variables

    IUIAnimationVariable *m_pAnimationVariableRed;
    IUIAnimationVariable *m_pAnimationVariableGreen;
    IUIAnimationVariable *m_pAnimationVariableBlue;
};
