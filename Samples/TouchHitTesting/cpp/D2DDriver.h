// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#ifndef D2DDRIVER_H
#define D2DDRIVER_H

// D2D header files
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>

// Colors used for shapes and controls on the page
enum DrawingColor {Blue, Orange, DimGray, DarkGray};

//
// CD2DDriver is a helper class that encapsulates common D2D routines and contains
// allocated D2D resources.
//
class CD2DDriver
{
public:
    // Initialization
    CD2DDriver(HWND hwnd);
    ~CD2DDriver();
    HRESULT Initialize();

    // Methods and properties
    HRESULT CreateRenderTarget();
    HRESULT CreateDeviceIndependentResources();
    HRESULT CreateDeviceResources();
    void DiscardDeviceResources();
    HRESULT CreatePathGeometry(_Out_ ID2D1PathGeometry **pPathGeometry);
    HRESULT RenderBackground(_In_ float clientWidth, _In_ float clientHeight);
    ID2D1HwndRenderTarget *GetRenderTarget();
    IDWriteTextFormat *GetTextFormat() const;
    ID2D1LinearGradientBrush *GetBrush(_In_ DrawingColor color);

    // Drawing methods
    void BeginDraw();
    void EndDraw();

private:
    // Helper to create gradient resource
    HRESULT _CreateGradient(
        _Out_ ID2D1LinearGradientBrush **pplgBrush,
        _In_ D2D1::ColorF::Enum startColor,
        _In_ float startOpacity,
        _In_ float startPos,
        _In_ D2D1::ColorF::Enum endColor,
        _In_ float endOpacity,
        _In_ float endPos);

    // Handle to the main window
    HWND _hWnd;

    // D2D Factory
    ID2D1Factory *_pD2DFactory;

    // D2D Render Target
    ID2D1HwndRenderTarget *_pRenderTarget;

    // Gradient Brushes
    ID2D1LinearGradientBrush *_pBLBrush;
    ID2D1LinearGradientBrush *_pORBrush;
    ID2D1LinearGradientBrush *_pBGBrush;
    ID2D1LinearGradientBrush *_pDGBrush;
    ID2D1LinearGradientBrush *_pDKGBrush;

    // Solid Brushes
    ID2D1SolidColorBrush *_pWhiteBrush;

    // Text format
    IDWriteFactory *_pDWriteFactory;
    IDWriteTextFormat *_pTextFormat;
};
#endif
