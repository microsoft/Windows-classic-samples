// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//----------------------------------------------------------------------------

#include "Common.h"
#include "RenderTest.h"
#include "TextHelpers.h"
#include "D2DRenderer.h"

namespace
{
    DWRITE_MATRIX const g_identityTransform =
    {
        1, 0,
        0, 1,
        0, 0
    };

    D2D1_MATRIX_3X2_F const g_identityMatrix =
    {
        1, 0,
        0, 1,
        0, 0
    };
}

IRenderer* CreateD2DRenderer(
    HWND hwnd,
    UINT width,
    UINT height,
    IDWriteTextFormat* textFormat,
    wchar_t const* text
    )
{
    return new(std::nothrow) D2DRenderer(
        hwnd,
        width,
        height,
        textFormat,
        text
        );
}

D2DRenderer::D2DRenderer(
    HWND hwnd,
    UINT width,
    UINT height,
    IDWriteTextFormat* textFormat,
    wchar_t const* text
    ) : 
    hwnd_(hwnd),
    width_(width), 
    height_(height),
    measuringMode_(DWRITE_MEASURING_MODE_NATURAL),
    transform_(g_identityTransform),
    text_(text),
    textFormat_(SafeAcquire(textFormat)),
    textLayout_(),
    renderTarget_(),
    bitmapRenderTarget_(),
    renderingParams_(),
    backBrush_(),
    textBrush_(),
    borderBrush_()
{
    magnifier_.visible = false;
}

void D2DRenderer::SetFormat(IDWriteTextFormat* format)
{
    SafeSet(&textFormat_, format);
    FreeTextLayout();
}

void D2DRenderer::SetText(wchar_t const* text)
{
    text_ = text;
    FreeTextLayout();
}

void D2DRenderer::SetMeasuringMode(DWRITE_MEASURING_MODE measuringMode)
{
    measuringMode_ = measuringMode;
    FreeTextLayout();
}

void D2DRenderer::SetTransform(DWRITE_MATRIX const& transform)
{
    transform_ = transform;

    // GDI-compatible layouts depend on the DPI and transform.
    if (measuringMode_ != DWRITE_MEASURING_MODE_NATURAL)
        FreeTextLayout();
}

void D2DRenderer::SetMagnifier(MagnifierInfo const& magnifier)
{
    magnifier_ = magnifier;
    SafeRelease(&bitmapRenderTarget_);
}

void D2DRenderer::SetWindowSize(UINT width, UINT height)
{
    width_ = width;
    height_ = height;

    if (renderTarget_ != NULL)
    {
        if (FAILED(renderTarget_->Resize(D2D1::SizeU(width, height))))
            SafeRelease(&renderTarget_);
    }

    FreeDeviceDependentResources();
    UpdateTextOrigin();
}

void D2DRenderer::SetMonitor(HMONITOR monitor)
{
    IDWriteRenderingParams* renderingParams = NULL;
    if (SUCCEEDED(g_dwriteFactory->CreateMonitorRenderingParams(monitor, &renderingParams)))
    {
        SafeAttach(&renderingParams_, SafeDetach(&renderingParams));

        if (renderTarget_ != NULL)
        {
            renderTarget_->SetTextRenderingParams(renderingParams_);
        }

        if (bitmapRenderTarget_ != NULL)
        {
            bitmapRenderTarget_->SetTextRenderingParams(renderingParams_);
        }
    }
    SafeRelease(&renderingParams);
}

HRESULT D2DRenderer::Draw(HDC hdc)
{
    HRESULT hr = DrawInternal();

    // If D2D returns D2DERR_RECREATE_TARGET, the client should recreate
    // the render target and try rendering the entire frame again.
    if (hr == D2DERR_RECREATE_TARGET)
    {
        FreeDeviceDependentResources();
        hr = DrawInternal();
    }

    return hr;
}

HRESULT D2DRenderer::DrawInternal()
{
    HRESULT hr = S_OK;

    if (SUCCEEDED(hr))
    {
        hr = InitializeDeviceDependentResources();
    }
    if (SUCCEEDED(hr))
    {
        hr = InitializeTextLayout();
    }

    renderTarget_->BeginDraw();

    renderTarget_->Clear(backColor_);

    // Set the render target transform. D2D has its own 3x2 matrix type, but
    // it has the same layout as DWRITE_MATRIX.
    D2D1_MATRIX_3X2_F matrix;
    memcpy(&matrix, &transform_, sizeof(matrix));
    renderTarget_->SetTransform(matrix);

    // Draw the text layout.
    renderTarget_->DrawTextLayout(
        textOrigin_,
        textLayout_,
        textBrush_
        );

    // Restore the original identity transform.
    renderTarget_->SetTransform(g_identityMatrix);

    // Draw the magnifier.
    if (SUCCEEDED(hr))
    {
        hr = DrawMagnifier();
    }

    if (SUCCEEDED(hr))
    {
        hr = renderTarget_->EndDraw();
    }

    return hr;
}

// MakeRectF
// Converts left, top, right, bottom in pixels to a D2D rectangle in DIPs.
D2D1_RECT_F MakeRectF(int left, int top, int right, int bottom)
{
    D2D1_RECT_F result;

    result.left   = PixelsToDipsX(left);
    result.top    = PixelsToDipsY(top);
    result.right  = PixelsToDipsX(right);
    result.bottom = PixelsToDipsY(bottom);

    return result;
}

HRESULT D2DRenderer::DrawMagnifier()
{
    HRESULT hr = S_OK;

    if (!magnifier_.visible)
        return hr;

    // Get the bounding rectangles, in DIPs, of the magnifier and the focus area.
    D2D1_RECT_F magnifierRect = MakeRectF(
        magnifier_.magnifierPos.x,
        magnifier_.magnifierPos.y,
        magnifier_.magnifierPos.x + magnifier_.magnifierSize.cx,
        magnifier_.magnifierPos.y + magnifier_.magnifierSize.cy
        );

    int const focusWidth  = magnifier_.magnifierSize.cx / magnifier_.scale;
    int const focusHeight = magnifier_.magnifierSize.cy / magnifier_.scale;

    D2D1_RECT_F focusRect = MakeRectF(
        magnifier_.focusPos.x,
        magnifier_.focusPos.y,
        magnifier_.focusPos.x + focusWidth,
        magnifier_.focusPos.y + focusHeight
        );

    // Branch depending on the magnifier type:
    //
    //   *  Vector means we scale up the text using a render transform. This is enabled only
    //      for natural layout because GDI-compatible layouts are not resolution-independent.
    //
    //   *  Pixel means we render at normal size to an intermediate bitmap and then scale up
    //      the pixels. This is the only other magnifier type implemented by the D2D renderer.
    //
    if (magnifier_.type == MagnifierInfo::Vector && measuringMode_ == DWRITE_MEASURING_MODE_NATURAL)
    {
        renderTarget_->FillRectangle(magnifierRect, backBrush_);

        // Clip to the magnifier rectangle.
        renderTarget_->PushAxisAlignedClip(magnifierRect, D2D1_ANTIALIAS_MODE_ALIASED);

        // Create a transform that translates and scales the focus rect to the magnifier rect.
        D2D1_MATRIX_3X2_F zoomTransform;;
        zoomTransform._11 = transform_.m11 * magnifier_.scale;
        zoomTransform._12 = transform_.m12 * magnifier_.scale;
        zoomTransform._21 = transform_.m21 * magnifier_.scale;
        zoomTransform._22 = transform_.m22 * magnifier_.scale;
        zoomTransform._31 = ((transform_.dx - focusRect.left) * magnifier_.scale) + magnifierRect.left;
        zoomTransform._32 = ((transform_.dy - focusRect.top) * magnifier_.scale) + magnifierRect.top;

        // Redraw the text layout using the zoom transform.
        renderTarget_->SetTransform(zoomTransform);
        renderTarget_->DrawTextLayout(textOrigin_, textLayout_, textBrush_);
        renderTarget_->SetTransform(g_identityMatrix);

        renderTarget_->PopAxisAlignedClip();
    }
    else
    {
        // Create the bitmap render target if we haven't already.
        if (bitmapRenderTarget_ == NULL)
        {
            hr = renderTarget_->CreateCompatibleRenderTarget(
                    D2D1::SizeF(PixelsToDipsX(focusWidth), PixelsToDipsY(focusHeight)),
                    D2D1::SizeU(focusWidth, focusHeight),
                    D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_IGNORE),
                    D2D1_COMPATIBLE_RENDER_TARGET_OPTIONS_NONE,
                    &bitmapRenderTarget_
                    );
        }

        if (SUCCEEDED(hr))
        {
            if (renderingParams_ != NULL)
            {
                bitmapRenderTarget_->SetTextRenderingParams(renderingParams_);
            }

            D2D1_MATRIX_3X2_F bitmapTransform;
            memcpy(&bitmapTransform, &transform_, sizeof(bitmapTransform));
            bitmapTransform._31 -= focusRect.left;
            bitmapTransform._32 -= focusRect.top;

            bitmapRenderTarget_->BeginDraw();

            bitmapRenderTarget_->Clear(backColor_);
            bitmapRenderTarget_->SetTransform(bitmapTransform);
            bitmapRenderTarget_->DrawTextLayout(textOrigin_, textLayout_, textBrush_);

            hr = bitmapRenderTarget_->EndDraw();
        }

        ID2D1Bitmap* bitmap = NULL;
        if (SUCCEEDED(hr))
        {
            hr = bitmapRenderTarget_->GetBitmap(&bitmap);
        }

        if (SUCCEEDED(hr))
        {
            renderTarget_->DrawBitmap(
                bitmap, 
                magnifierRect, 
                1.0f, 
                D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR
                );
        }

        SafeRelease(&bitmap);
    }

    renderTarget_->DrawRectangle(focusRect, borderBrush_, 2.0f);
    renderTarget_->DrawRectangle(magnifierRect, borderBrush_, 2.0f);

    return hr;
}

HRESULT D2DRenderer::InitializeDeviceDependentResources()
{
    HRESULT hr = S_OK;

    if (renderTarget_ != NULL)
        return hr;

    ID2D1HwndRenderTarget* renderTarget = NULL;
    ID2D1SolidColorBrush*  backBrush    = NULL;
    ID2D1SolidColorBrush*  textBrush    = NULL;
    ID2D1SolidColorBrush*  borderBrush  = NULL;

    D2D1_RENDER_TARGET_PROPERTIES renderTargetProperties = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(),
        static_cast<float>(g_dpiX),
        static_cast<float>(g_dpiY)
        );

    if (SUCCEEDED(hr))
    {
        hr = g_d2dFactory->CreateHwndRenderTarget(
                renderTargetProperties,
                D2D1::HwndRenderTargetProperties(hwnd_, D2D1::SizeU(width_, height_)),
                &renderTarget
                );
    }

    if (renderingParams_ != NULL)
    {
        renderTarget->SetTextRenderingParams(renderingParams_);
    }

    backColor_ = D2D1::ColorF(GetSysColor(COLOR_WINDOW));

    if (SUCCEEDED(hr))
    {
        hr = renderTarget->CreateSolidColorBrush(
                backColor_,
                &backBrush
                );
    }

    if (SUCCEEDED(hr))
    {
        hr = renderTarget->CreateSolidColorBrush(
                D2D1::ColorF(GetSysColor(COLOR_WINDOWTEXT)),
                &textBrush
                );
    }

    if (SUCCEEDED(hr))
    {
        hr = renderTarget->CreateSolidColorBrush(
                D2D1::ColorF(1.0f, 0, 0, 0.5f),
                &borderBrush
                );
    }

    // Commit changes.
    SafeAttach(&renderTarget_, SafeDetach(&renderTarget));
    SafeAttach(&backBrush_,    SafeDetach(&backBrush));
    SafeAttach(&textBrush_,    SafeDetach(&textBrush));
    SafeAttach(&borderBrush_,  SafeDetach(&borderBrush));

    return hr;
}

void D2DRenderer::FreeDeviceDependentResources()
{
    SafeRelease(&renderTarget_);
    SafeRelease(&bitmapRenderTarget_);
    SafeRelease(&backBrush_);
    SafeRelease(&textBrush_);
    SafeRelease(&borderBrush_);
}

HRESULT D2DRenderer::InitializeTextLayout()
{
    HRESULT hr = S_OK;

    if (textLayout_ != NULL)
        return hr;

    if (measuringMode_ == DWRITE_MEASURING_MODE_NATURAL)
    {
        hr = g_dwriteFactory->CreateTextLayout(
                text_,
                lstrlenW(text_),
                textFormat_,
                g_formatWidth,
                0, // max height
                &textLayout_
                );
    }
    else
    {
        BOOL useGdiNatural = (measuringMode_ == DWRITE_MEASURING_MODE_GDI_NATURAL);

        hr = g_dwriteFactory->CreateGdiCompatibleTextLayout(
                text_,
                lstrlenW(text_),
                textFormat_,
                g_formatWidth,
                0, // max height
                g_dpiY / 96.0f, // pixels per DIP
                &transform_,
                useGdiNatural,
                &textLayout_
                );
    }

    if (SUCCEEDED(hr))
    {
        hr = UpdateTextOrigin();
    }

    return hr;
}

HRESULT D2DRenderer::UpdateTextOrigin()
{
    HRESULT hr = S_OK;

    if (textLayout_ == NULL)
        return hr;

    // Get the text layout size.
    DWRITE_TEXT_METRICS metrics = {};
    hr = textLayout_->GetMetrics(&metrics);

    // Center the text.
    textOrigin_ = D2D1::Point2F(
        (PixelsToDipsX(width_) - metrics.width) * 0.5f,
        (PixelsToDipsY(height_) - metrics.height) * 0.5f
        );

    return hr;
}

void D2DRenderer::FreeTextLayout()
{
    SafeRelease(&textLayout_);
}
