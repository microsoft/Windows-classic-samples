// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#define TIME_RING_BUFFER_SIZE 4

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
    HRESULT CreateGeometries();

    HRESULT CreateDeviceResources();

    void DiscardDeviceResources();
    void DiscardGeometryData();

    HRESULT RenderMainContent(
        float time
        );

    HRESULT RenderTextInfo();

    HRESULT OnRender();

    void OnResize(
        UINT width,
        UINT height
        );

    void OnMouseMove(
        LPARAM lParam
        );

    void OnKeyDown(
        SHORT vkey
        );

    void OnWheel(
        WPARAM w
        );

    static LRESULT CALLBACK WndProc(
        HWND hWnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
        );

private:
    HWND m_hwnd;

    D2D1_ANTIALIAS_MODE m_antialiasMode;
    bool m_useRealizations;
    bool m_autoGeometryRegen;
    bool m_drawStroke;
    bool m_paused;
    bool m_updateRealization;
    UINT m_numSquares;
    D2D1_POINT_2F m_mousePos;
    LONGLONG m_pausedTime;
    LONGLONG m_timeDelta;
    float m_targetZoomFactor;
    float m_currentZoomFactor;
    RingBuffer<LONGLONG, TIME_RING_BUFFER_SIZE> m_times;

    ID2D1Factory *m_pD2DFactory;
    IWICImagingFactory *m_pWICFactory;
    IDWriteFactory *m_pDWriteFactory;
    ID2D1HwndRenderTarget *m_pRT;
    IDWriteTextFormat *m_pTextFormat;
    ID2D1SolidColorBrush *m_pSolidColorBrush;
    IGeometryRealization *m_pRealization;
    ID2D1Geometry *m_pGeometry;
};


