// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <time.h>
#include "D2DDriver.h"

CD2DDriver::CD2DDriver(HWND hwnd):
    _hWnd(hwnd),
    _pRenderTarget(nullptr),
    _pD2DFactory(nullptr),
    _pBLBrush(nullptr),
    _pORBrush(nullptr),
    _pBGBrush(nullptr),
    _pDGBrush(nullptr),
    _pDKGBrush(nullptr),
    _pWhiteBrush(nullptr)
{
}

CD2DDriver::~CD2DDriver()
{
    DiscardDeviceResources();
    _pD2DFactory->Release();
}

HRESULT CD2DDriver::Initialize()
{
    HRESULT hr = CreateDeviceIndependentResources();

    if (SUCCEEDED(hr))
    {
        hr = CreateDeviceResources();
    }

    return hr;
}

HRESULT CD2DDriver::CreateDeviceIndependentResources()
{
    return D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &_pD2DFactory);
}

// Path geometry is used for drawing polygonal shapes
HRESULT CD2DDriver::CreatePathGeometry(_Out_ ID2D1PathGeometry **pPathGeometry)
{
    return _pD2DFactory->CreatePathGeometry(pPathGeometry);
}


HRESULT CD2DDriver::CreateDeviceResources()
{
    HRESULT hr = S_OK;

    if (!_pRenderTarget)
    {
        hr = CreateRenderTarget();

        if (SUCCEEDED(hr))
        {
            // Create white brush
            hr = _pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::White),
                &_pWhiteBrush
                );
        }

        if (SUCCEEDED(hr))
        {
            // Create Blue gradient brush
            hr = _CreateGradient(&_pBLBrush,
                D2D1::ColorF::Aqua,
                1.0f,
                1.0f,
                D2D1::ColorF::DarkBlue,
                1.0f,
                0.0f);
        }

        if (SUCCEEDED(hr))
        {
            // Create Orange gradient brush
            _CreateGradient(&_pORBrush,
                D2D1::ColorF::Yellow,
                1.0f,
                1.0f,
                D2D1::ColorF::OrangeRed,
                1.0f,
                0.0f);
        }

        if (SUCCEEDED(hr))
        {
            // Create bg gradient brush
            hr = _CreateGradient(&_pBGBrush,
                D2D1::ColorF::LightSlateGray,
                1.0f,
                1.0f,
                D2D1::ColorF::Black,
                1.0f,
                0.0f);
        }

        if (SUCCEEDED(hr))
        {
            // Create dim gray gradient brush
            hr = _CreateGradient(&_pDGBrush,
                D2D1::ColorF::DimGray,
                1.0f,
                1.0f,
                D2D1::ColorF::Black,
                1.0f,
                0.0f);
        }

        if (SUCCEEDED(hr))
        {
            // Create dark gray gradient brush
            hr = _CreateGradient(&_pDKGBrush,
                D2D1::ColorF::DarkGray,
                1.0f,
                1.0f,
                D2D1::ColorF::Gray,
                1.0f,
                0.0f);
        }

        if (SUCCEEDED(hr))
        {
            // create text format
            static wchar_t const mscFontName[] = L"Verdana";
            static float const mscFontSize = 20;

            // Create a DirectWrite factory.
            hr = DWriteCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED,
                __uuidof(_pDWriteFactory),
                reinterpret_cast<IUnknown **>(&_pDWriteFactory)
                );

            if (SUCCEEDED(hr))
            {
                // Create a DirectWrite text format object.
                hr = _pDWriteFactory->CreateTextFormat(
                    mscFontName,
                    nullptr,
                    DWRITE_FONT_WEIGHT_NORMAL,
                    DWRITE_FONT_STYLE_NORMAL,
                    DWRITE_FONT_STRETCH_NORMAL,
                    mscFontSize,
                    L"", //locale
                    &_pTextFormat
                    );
            }

            if (SUCCEEDED(hr))
            {
                // Center the text horizontally and vertically.
                _pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
                _pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
            }
        }

    }

    return hr;
}

HRESULT CD2DDriver::RenderBackground(_In_ float clientWidth, _In_ float clientHeight)
{
    // Randomly generate transparent shapes for
    // added effect to the background
    static unsigned int seed = static_cast<unsigned int>(time(nullptr));
    srand(seed);

    _pBGBrush->SetStartPoint(D2D1::Point2F(clientWidth / 2, 0.0f));
    _pBGBrush->SetEndPoint(D2D1::Point2F(clientWidth / 2, clientHeight));

    // Create background rectangle
    D2D1_RECT_F background = D2D1::RectF(0, 0, clientWidth, clientHeight);
    _pRenderTarget->FillRectangle(&background, _pBGBrush);

    _pWhiteBrush->SetOpacity(0.015f);
    D2D1_RECT_F square;
    D2D1_ROUNDED_RECT roundedSquare;
    D2D_MATRIX_3X2_F rotateMatrix;

    for (int i = 0; i < 12; i ++)
    {
        int randomDimension = rand() % 500 + 200;
        int randomPositionX = rand() % static_cast<int>(clientWidth);
        int randomPositionY = rand() % static_cast<int>(clientHeight);
        int randomAngle = rand() % 360;

        // Apply rotate transform
        rotateMatrix = D2D1::Matrix3x2F::Rotation(static_cast<float>(randomAngle),
        D2D1::Point2F(static_cast<float>(randomPositionX + randomDimension/2),
                static_cast<float>(randomPositionY + randomDimension/2)
            )
        );

        _pRenderTarget->SetTransform(&rotateMatrix);

        square = D2D1::RectF(
            static_cast<float>(randomPositionX),
            static_cast<float>(randomPositionY),
            static_cast<float>(randomDimension),
            static_cast<float>(randomDimension)
        );

        roundedSquare = D2D1::RoundedRect(square, 10.0f, 10.0f);
        _pRenderTarget->FillRoundedRectangle(&roundedSquare, _pWhiteBrush);
    }

    _pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

    return S_OK;
}

void CD2DDriver::DiscardDeviceResources()
{
    _pRenderTarget->Release();
    _pBLBrush->Release();
    _pORBrush->Release();
    _pBGBrush->Release();
    _pWhiteBrush->Release();
    _pDGBrush->Release();
    _pDKGBrush->Release();
}

HRESULT CD2DDriver::CreateRenderTarget()
{
    HRESULT hr = S_OK;

    // Get the size of our target area
    RECT rc;

    GetClientRect(_hWnd, &rc);
    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

    // Create a Direct2D render target
    hr = _pD2DFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(_hWnd, size), &_pRenderTarget);
    _pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);

    return hr;
}

void CD2DDriver::BeginDraw()
{
    _pRenderTarget->BeginDraw();
    _pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
}

void CD2DDriver::EndDraw()
{
    HRESULT hr = _pRenderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET)
    {
        DiscardDeviceResources();
    }
}

HRESULT CD2DDriver::_CreateGradient(
    _Out_ ID2D1LinearGradientBrush **pplgBrush,
    _In_ D2D1::ColorF::Enum startColor,
    _In_ float startOpacity,
    _In_ float startPos,
    _In_ D2D1::ColorF::Enum endColor,
    _In_ float endOpacity,
    _In_ float endPos)
{
    HRESULT hr = S_OK;

    ID2D1GradientStopCollection *pStops;
    D2D1_GRADIENT_STOP stops[2];
    stops[0].color = D2D1::ColorF(startColor, startOpacity);
    stops[0].position = startPos;
    stops[1].color = D2D1::ColorF(endColor, endOpacity);
    stops[1].position = endPos;

    hr = _pRenderTarget->CreateGradientStopCollection(stops, 2, &pStops);

    if(SUCCEEDED(hr))
    {
        hr = _pRenderTarget->CreateLinearGradientBrush(
            D2D1::LinearGradientBrushProperties(
            D2D1::Point2F(0.0f, 0.0f),
            D2D1::Point2F(0.0f, 0.0f)),
            D2D1::BrushProperties(),
            pStops,
            pplgBrush);
    }

    return hr;
}

ID2D1HwndRenderTarget *CD2DDriver::GetRenderTarget()
{
    return _pRenderTarget;
}

IDWriteTextFormat *CD2DDriver::GetTextFormat() const
{
    return _pTextFormat;
}

ID2D1LinearGradientBrush *CD2DDriver::GetBrush(_In_ DrawingColor color)
{
    ID2D1LinearGradientBrush *brush = nullptr;

    // Determines what brush to use for drawing this object and
    // get the brush from the D2DDriver class.
    switch (color)
    {
        case Blue:
            brush = _pBLBrush;
            break;
        case Orange:
            brush = _pORBrush;
            break;
        case DimGray:
            brush = _pDGBrush;
            break;
        case DarkGray:
            brush = _pDKGBrush;
            break;
        default:
            brush = _pDGBrush;
    }

    return brush;
}
