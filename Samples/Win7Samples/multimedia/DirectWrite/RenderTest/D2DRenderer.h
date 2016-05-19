// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//----------------------------------------------------------------------------
#pragma once
#include "IRenderer.h"

class D2DRenderer : public IRenderer
{
public:
    D2DRenderer(
        HWND hwnd,
        UINT width,
        UINT height,
        IDWriteTextFormat* textFormat,
        wchar_t const* text
        );

    ~D2DRenderer()
    {
        SafeRelease(&textFormat_);
        SafeRelease(&textLayout_);
        SafeRelease(&renderingParams_);
        SafeRelease(&renderTarget_);
        SafeRelease(&bitmapRenderTarget_);
        SafeRelease(&backBrush_);
        SafeRelease(&textBrush_);
        SafeRelease(&borderBrush_);
    }

    virtual void SetFormat(IDWriteTextFormat* format);
    virtual void SetText(wchar_t const* text);
    virtual void SetWindowSize(UINT width, UINT height);
    virtual void SetMonitor(HMONITOR monitor);
    virtual void SetMeasuringMode(DWRITE_MEASURING_MODE measuringMode);
    virtual void SetTransform(DWRITE_MATRIX const& transform);
    virtual void SetMagnifier(MagnifierInfo const& magnifier);

    virtual HRESULT Draw(HDC hdc);

private:
    HRESULT DrawInternal();
    HRESULT DrawMagnifier();

    HRESULT InitializeDeviceDependentResources();
    void FreeDeviceDependentResources();

    HRESULT InitializeTextLayout();
    HRESULT UpdateTextOrigin();
    void FreeTextLayout();

    HWND hwnd_;
    UINT width_;
    UINT height_;
    DWRITE_MEASURING_MODE measuringMode_;
    DWRITE_MATRIX transform_;
    MagnifierInfo magnifier_;
    IDWriteTextFormat* textFormat_;
    wchar_t const* text_;

    // Device dependent resources.
    ID2D1HwndRenderTarget* renderTarget_;
    ID2D1BitmapRenderTarget* bitmapRenderTarget_;
    IDWriteRenderingParams* renderingParams_;
    ID2D1SolidColorBrush* backBrush_;
    ID2D1SolidColorBrush* textBrush_;
    ID2D1SolidColorBrush* borderBrush_;
    D2D1_COLOR_F backColor_;

    // Text layout.
    IDWriteTextLayout* textLayout_;
    D2D1_POINT_2F textOrigin_;
};
