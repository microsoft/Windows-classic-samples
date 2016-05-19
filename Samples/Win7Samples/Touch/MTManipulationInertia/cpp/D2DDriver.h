// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#ifndef D2DDRIVER_H
#define D2DDRIVER_H

// D2D
#include <d2d1.h>
#include <d2d1helper.h>	
#include <comdef.h>

// D2D Smart Pointers
_COM_SMARTPTR_TYPEDEF(ID2D1Factory, __uuidof(ID2D1Factory));
_COM_SMARTPTR_TYPEDEF(ID2D1HwndRenderTarget, __uuidof(ID2D1HwndRenderTarget));
_COM_SMARTPTR_TYPEDEF(ID2D1LinearGradientBrush, __uuidof(ID2D1LinearGradientBrush));
_COM_SMARTPTR_TYPEDEF(ID2D1SolidColorBrush, __uuidof(ID2D1SolidColorBrush));
_COM_SMARTPTR_TYPEDEF(ID2D1RectangleGeometry, __uuidof(ID2D1RectangleGeometry));
_COM_SMARTPTR_TYPEDEF(ID2D1RoundedRectangleGeometry, __uuidof(ID2D1RoundedRectangleGeometry));

class CD2DDriver {
public:
    CD2DDriver(HWND hwnd);
    ~CD2DDriver();
    HRESULT Initialize();

    // D2D Methods

    HRESULT CreateRenderTarget();
    HRESULT CreateDeviceIndependentResources();
    HRESULT CreateDeviceResources();
    VOID DiscardDeviceResources();
    
    HRESULT CreateGeometryRoundedRect(D2D1_ROUNDED_RECT rect, ID2D1RoundedRectangleGeometry** spRoundedRectGeometry);
    HRESULT RenderBackground(FLOAT clientWidth, FLOAT clientHeight);
    ID2D1HwndRenderTargetPtr GetRenderTarget();
    ID2D1LinearGradientBrushPtr get_GradBrush(unsigned int uBrushType);

    VOID BeginDraw();
    VOID EndDraw();

    enum {GRB_Glossy, GRB_Blue, GRB_Orange, GRB_Red, GRB_Green};

private:
    // Helper to create gradient resource
    HRESULT CreateGradient(ID2D1GradientStopCollection* pStops, 
        ID2D1LinearGradientBrush** pplgBrush, 
        D2D1::ColorF::Enum startColor, 
        FLOAT startOpacity, 
        FLOAT startPos, 
        D2D1::ColorF::Enum endColor, 
        FLOAT endOpacity, 
        FLOAT endPos);

    // Handle to the main window
    HWND m_hWnd;

    // D2D Factory
    ID2D1FactoryPtr m_spD2DFactory;

    // D2D Render Target
    ID2D1HwndRenderTargetPtr m_spRT;

    // Gradient Brushes
    ID2D1LinearGradientBrushPtr m_spGLBrush;
    ID2D1LinearGradientBrushPtr m_spBLBrush;
    ID2D1LinearGradientBrushPtr m_spORBrush;
    ID2D1LinearGradientBrushPtr m_spGRBrush;
    ID2D1LinearGradientBrushPtr m_spREBrush;
    ID2D1LinearGradientBrushPtr m_spBGBrush;

    // Solid Brushes
    ID2D1SolidColorBrushPtr m_spWhiteBrush;

    // Gradient Stops for Gradient Brushes
    ID2D1GradientStopCollection *pGradientStops;
    ID2D1GradientStopCollection *pGradientStops2;
    ID2D1GradientStopCollection *pGradientStops3;
    ID2D1GradientStopCollection *pGradientStops4;
    ID2D1GradientStopCollection *pGradientStops5;
    ID2D1GradientStopCollection *pGlossyStops;

};
#endif