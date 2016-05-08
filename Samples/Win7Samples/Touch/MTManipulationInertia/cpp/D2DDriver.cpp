// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "D2DDriver.h"

CD2DDriver::CD2DDriver(HWND hwnd):
    m_hWnd(hwnd) {

}

CD2DDriver::~CD2DDriver() {
    DiscardDeviceResources();
}

HRESULT CD2DDriver::Initialize() {
    HRESULT hr = CreateDeviceIndependentResources();

    if(SUCCEEDED(hr))
    {
        hr = CreateDeviceResources();
    }

    return hr;
}

HRESULT CD2DDriver::CreateDeviceIndependentResources() {
    // Create D2D factory
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
        &m_spD2DFactory);

    return hr;
}

HRESULT CD2DDriver::CreateGeometryRoundedRect(D2D1_ROUNDED_RECT rect, 
                                              ID2D1RoundedRectangleGeometry** spRoundedRectGeometry) {
    HRESULT hr = m_spD2DFactory->CreateRoundedRectangleGeometry(
        rect, 
        spRoundedRectGeometry );

    return hr;
}

HRESULT CD2DDriver::CreateDeviceResources() {
    HRESULT hr = S_OK;

    if(!m_spRT) 
    {
        hr = CreateRenderTarget();

        if(SUCCEEDED(hr))
        {
            // Create white brush
            hr = m_spRT->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::White),
                &m_spWhiteBrush
                );
        }

        if(SUCCEEDED(hr))
        {
            // Create glossy gradient brush
            hr = CreateGradient(pGlossyStops, 
                &m_spGLBrush, 
                D2D1::ColorF::White, 
                0.5f, 
                0.3f, 
                D2D1::ColorF::White, 
                0.0f, 
                1.0f);
        }

        if(SUCCEEDED(hr))
        {
            // Create Blue gradient brush
            hr = CreateGradient(pGradientStops, 
                &m_spBLBrush, 
                D2D1::ColorF::Aqua, 
                1.0f, 
                1.0f, 
                D2D1::ColorF::DarkBlue, 
                1.0f, 
                0.0f);
        }

        if(SUCCEEDED(hr))
        {
            // Create Orange gradient brush
            CreateGradient(pGradientStops2, 
                &m_spORBrush, 
                D2D1::ColorF::Yellow, 
                1.0f, 
                1.0f, 
                D2D1::ColorF::OrangeRed, 
                1.0f, 
                0.0f);
        }

        if(SUCCEEDED(hr))
        {
            // Create yellow gradient brush
            hr = CreateGradient(pGradientStops3, 
                &m_spREBrush, 
                D2D1::ColorF::Red, 
                1.0f, 
                1.0f, 
                D2D1::ColorF::Maroon, 
                1.0f, 
                0.0f);
        }
        if(SUCCEEDED(hr))
        {
            // Create Green gradient brush
            hr = CreateGradient(pGradientStops4, 
                &m_spGRBrush, 
                D2D1::ColorF::GreenYellow, 
                1.0f, 
                1.0f, 
                D2D1::ColorF::Green, 
                1.0f, 
                0.0f);
        }

        if(SUCCEEDED(hr))
        {
            // Create bg gradient brush
            hr = CreateGradient(pGradientStops5, 
                &m_spBGBrush, 
                D2D1::ColorF::LightSlateGray, 
                1.0f, 
                1.0f, 
                D2D1::ColorF::Black, 
                1.0f, 
                0.0f);
        }
    }

    return hr;
}
HRESULT CD2DDriver::RenderBackground(FLOAT clientWidth, FLOAT clientHeight) {

    m_spBGBrush->SetStartPoint(
        D2D1::Point2F(
            clientWidth/2, 
            0.0f)
        );
    
    m_spBGBrush->SetEndPoint(
        D2D1::Point2F(
            clientWidth/2, 
            clientHeight)
         );
    
    // Create background rectangle
    D2D1_RECT_F background = D2D1::RectF(
        0,
        0,
        clientWidth,
        clientHeight
        );

    m_spRT->FillRectangle(
        &background,
        m_spBGBrush
        );

    // Randomly generate transparent shapes for
    // added effect to the background
    srand(75);
    m_spWhiteBrush->SetOpacity(0.015f);
    D2D1_RECT_F square;
    D2D1_ROUNDED_RECT roundedSquare;
    D2D_MATRIX_3X2_F rotateMatrix;

    for(int i = 0; i < 12; i ++)
    {
        int randomDimension = rand()%500 + 200;
        int randomPositionX = rand()%(int)clientWidth;
        int randomPositionY = rand()%(int)clientHeight;
        int randomAngle = rand()%360;

        // Apply rotate transform
        rotateMatrix = D2D1::Matrix3x2F::Rotation((FLOAT)randomAngle, 
        D2D1::Point2F((FLOAT)(randomPositionX + randomDimension/2), 
                (FLOAT)(randomPositionY + randomDimension/2)
            )
        );
    
        m_spRT->SetTransform(&rotateMatrix);
        
        square = D2D1::RectF(
            (FLOAT)randomPositionX,
            (FLOAT)randomPositionY,
            (FLOAT)randomDimension,
            (FLOAT)randomDimension
        );
        
        roundedSquare = D2D1::RoundedRect(
            square, 
            10.0f, 
            10.0f
        );

        m_spRT->FillRoundedRectangle(
            &roundedSquare,
            m_spWhiteBrush
        );
    }
    
    return S_OK;
}

VOID CD2DDriver::DiscardDeviceResources() {
    m_spRT.Release();
    m_spBLBrush.Release();
    m_spORBrush.Release();
    m_spGRBrush.Release();
    m_spREBrush.Release();
    m_spGLBrush.Release();
    m_spBGBrush.Release();
    m_spWhiteBrush.Release();
}	

HRESULT CD2DDriver::CreateRenderTarget() {
    HRESULT hr = S_OK;

    // Get the size of our target area
    RECT rc;

    GetClientRect(m_hWnd, &rc);
    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left,
            rc.bottom - rc.top);

    // Create a Direct2D render target
    hr = m_spD2DFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(m_hWnd, size), &m_spRT);

    return hr;
}

VOID CD2DDriver::BeginDraw() {
    m_spRT->BeginDraw();
    m_spRT->Clear(D2D1::ColorF(D2D1::ColorF::White));
}

VOID CD2DDriver::EndDraw() {
    HRESULT hr = m_spRT->EndDraw();
    if(hr == D2DERR_RECREATE_TARGET)
    {
        DiscardDeviceResources();
    }
}

HRESULT CD2DDriver::CreateGradient(ID2D1GradientStopCollection* pStops, 
    ID2D1LinearGradientBrush** pplgBrush, 
    D2D1::ColorF::Enum startColor, 
    FLOAT startOpacity, 
    FLOAT startPos, 
    D2D1::ColorF::Enum endColor, 
    FLOAT endOpacity, 
    FLOAT endPos) {
    
    HRESULT hr = S_OK;

    D2D1_GRADIENT_STOP stops[2];
    stops[0].color = D2D1::ColorF(startColor, startOpacity);
    stops[0].position = startPos;
    stops[1].color = D2D1::ColorF(endColor, endOpacity);
    stops[1].position = endPos;

    hr = m_spRT->CreateGradientStopCollection(
        stops,
        2,
        &pStops
        );

    if(SUCCEEDED(hr))
    {
        hr = m_spRT->CreateLinearGradientBrush(
            D2D1::LinearGradientBrushProperties(
            D2D1::Point2F(0.0f, 0.0f),
            D2D1::Point2F(0.0f, 0.0f)),
            D2D1::BrushProperties(),
            pStops,
            pplgBrush);
    }

    return hr;
}

ID2D1HwndRenderTargetPtr CD2DDriver::GetRenderTarget() {
    return m_spRT;
}

ID2D1LinearGradientBrushPtr CD2DDriver::get_GradBrush(unsigned int uBrushType) {
    switch(uBrushType)
    {
    case GRB_Glossy:
        return m_spGLBrush;
        break;
    case GRB_Blue:
        return m_spBLBrush;
        break;
    case GRB_Orange:
        return m_spORBrush;
        break;
    case GRB_Red:
        return m_spREBrush;
        break;
    case GRB_Green:
        return m_spGRBrush;
        break;
    default:
        return m_spGRBrush;
    }
}