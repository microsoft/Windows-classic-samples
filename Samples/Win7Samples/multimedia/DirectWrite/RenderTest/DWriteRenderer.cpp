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
#include "DWriteRenderer.h"

namespace
{
    DWRITE_MATRIX const g_identityTransform =
    {
        1, 0,
        0, 1,
        0, 0
    };
}

IRenderer* CreateDWriteRenderer(
    HWND hwnd,
    UINT width,
    UINT height,
    IDWriteTextFormat* textFormat,
    wchar_t const* text
    )
{
    return new(std::nothrow) DWriteRenderer(
        hwnd,
        width,
        height,
        textFormat,
        text
        );
}

DWriteRenderer::DWriteRenderer(
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
    borderPen_(NULL),
    textFormat_(SafeAcquire(textFormat)),
    textLayout_(),
    renderTarget_(),
    magnifierTarget_(),
    renderingParams_()
{
    magnifier_.visible = false;
}

void DWriteRenderer::SetFormat(IDWriteTextFormat* format)
{
    SafeSet(&textFormat_, format);
    FreeTextLayout();
}

void DWriteRenderer::SetText(wchar_t const* text)
{
    text_ = text;
    FreeTextLayout();
}

void DWriteRenderer::SetMeasuringMode(DWRITE_MEASURING_MODE measuringMode)
{
    measuringMode_ = measuringMode;
    FreeTextLayout();
}

void DWriteRenderer::SetTransform(DWRITE_MATRIX const& transform)
{
    transform_ = transform;

    // GDI-compatible layouts depend on the DPI and transform.
    if (measuringMode_ != DWRITE_MEASURING_MODE_NATURAL)
        FreeTextLayout();
}

void DWriteRenderer::SetMagnifier(MagnifierInfo const& magnifier)
{
    magnifier_ = magnifier;
    SafeRelease(&magnifierTarget_);
}

void DWriteRenderer::SetWindowSize(UINT width, UINT height)
{
    width_ = width;
    height_ = height;
    SafeRelease(&renderTarget_);

    UpdateTextOrigin();
}

void DWriteRenderer::SetMonitor(HMONITOR monitor)
{
    g_dwriteFactory->CreateMonitorRenderingParams(monitor, &renderingParams_);
}

HRESULT DWriteRenderer::Draw(HDC hdc)
{
    HRESULT hr = S_OK;

    // Create the bitmap render target if we don't already have it.
    if (renderTarget_ == NULL)
    {
        IDWriteGdiInterop* gdiInterop = NULL;

        if (SUCCEEDED(hr))
        {
            hr = g_dwriteFactory->GetGdiInterop(&gdiInterop);
        }
        if (SUCCEEDED(hr))
        {
            hr = gdiInterop->CreateBitmapRenderTarget(hdc, width_, height_, &renderTarget_);
        }
        if (SUCCEEDED(hr))
        {
            hr = renderTarget_->SetPixelsPerDip(g_dpiY / 96.0f);
        }

        SafeRelease(&gdiInterop);
    }

    // Create the rendering params object if we haven't already.
    if (SUCCEEDED(hr) && renderingParams_ == NULL)
    {
        hr = g_dwriteFactory->CreateRenderingParams(&renderingParams_);
    }

    HDC hdcMem = NULL;
    if (SUCCEEDED(hr))
    {
        // Clear the background.
        hdcMem = renderTarget_->GetMemoryDC();
        SelectObject(hdcMem, GetSysColorBrush(COLOR_WINDOW));
        PatBlt(hdcMem, 0, 0, width_, height_, PATCOPY);

        // Set the rendering transform.
        renderTarget_->SetCurrentTransform(&transform_);

        // Prepare the render target we use for the magnifier.
        PrepareMagnifier(hdc);
    }

    if (SUCCEEDED(hr))
    {
        hr = InitializeTextLayout();
    }

    if (SUCCEEDED(hr))
    {
        // Render the text. The Draw method will call back to the IDWriteTextRenderer
        // methods implemented by this class.
        hr = textLayout_->Draw(
            NULL,           // optional client drawing context
            this,           // renderer callback
            textOriginX_,
            textOriginY_
            );
    }

    if (SUCCEEDED(hr))
    {
        DrawMagnifier();

        // Do the final BitBlt to the specified HDC.
        BitBlt(hdc, 0, 0, width_, height_, hdcMem, 0, 0, SRCCOPY | NOMIRRORBITMAP);
    }

    return hr;
}

HRESULT DWriteRenderer::PrepareMagnifier(HDC hdc)
{
    HRESULT hr = S_OK;

    if (!magnifier_.visible)
    {
        SafeRelease(&magnifierTarget_);
        return hr;
    }

    // Determine the size and scale factor for the magnifier render target. In vector
    // mode we render using a scale transform. In all other modes we render at normal
    // size and then scale up the pixels afterwards.
    SIZE targetSize = magnifier_.magnifierSize;
    int targetScale = magnifier_.scale;
    if (magnifier_.type != MagnifierInfo::Vector)
    {
        targetSize.cx /= targetScale;
        targetSize.cy /= targetScale;
        targetScale = 1;
    }

    // Create a separate render target for the magnifier if we haven't already.
    if (SUCCEEDED(hr) && magnifierTarget_ == NULL)
    {
        IDWriteGdiInterop* gdiInterop = NULL;

        if (SUCCEEDED(hr))
        {
            hr = g_dwriteFactory->GetGdiInterop(&gdiInterop);
        }
        if (SUCCEEDED(hr))
        {
            hr = gdiInterop->CreateBitmapRenderTarget(hdc, targetSize.cx, targetSize.cy, &magnifierTarget_);
        }

        SafeRelease(&gdiInterop);
    }

    DWRITE_MATRIX zoomTransform;;
    if (SUCCEEDED(hr))
    {
        // Clear the background.
        HDC hdcMagnifier = magnifierTarget_->GetMemoryDC();
        SelectObject(hdcMagnifier, GetSysColorBrush(COLOR_WINDOW));
        PatBlt(hdcMagnifier, 0, 0, magnifier_.magnifierSize.cx, magnifier_.magnifierSize.cy, PATCOPY);

        // Create a transform that translates and scales the focus rect to the origin of the magnifier target.
        float focusLeft = PixelsToDipsX(magnifier_.focusPos.x);
        float focusTop = PixelsToDipsY(magnifier_.focusPos.y);

        zoomTransform.m11 = transform_.m11 * targetScale;
        zoomTransform.m12 = transform_.m12 * targetScale;
        zoomTransform.m21 = transform_.m21 * targetScale;
        zoomTransform.m22 = transform_.m22 * targetScale;
        zoomTransform.dx = (transform_.dx - focusLeft) * targetScale;
        zoomTransform.dy = (transform_.dy - focusTop) * targetScale;
    }

    if (SUCCEEDED(hr))
    {
        hr = magnifierTarget_->SetCurrentTransform(&zoomTransform);
    }

    return hr;
}

void DWriteRenderer::DrawMagnifier()
{
    if (magnifierTarget_ == NULL)
        return;

    HDC memoryDC = renderTarget_->GetMemoryDC();

    // Copy the text from the magnifier render target to the main render target.
    switch (magnifier_.type)
    {
    case MagnifierInfo::Vector:
        // We rendered the text at the larger scale; just copy it.
        BitBlt(
            memoryDC, 
            magnifier_.magnifierPos.x, 
            magnifier_.magnifierPos.y,
            magnifier_.magnifierSize.cx,
            magnifier_.magnifierSize.cy,
            magnifierTarget_->GetMemoryDC(),
            0,
            0,
            SRCCOPY | NOMIRRORBITMAP
            );
        break;

    case MagnifierInfo::Pixel:
        // We rendered the text at normal size; copy and scale up.
        StretchBlt(
            memoryDC, 
            magnifier_.magnifierPos.x, 
            magnifier_.magnifierPos.y,
            magnifier_.magnifierSize.cx,
            magnifier_.magnifierSize.cy,
            magnifierTarget_->GetMemoryDC(),
            0,
            0,
            magnifier_.magnifierSize.cx / magnifier_.scale,
            magnifier_.magnifierSize.cy / magnifier_.scale,
            SRCCOPY | NOMIRRORBITMAP
            );
        break;

    case MagnifierInfo::Subpixel:
        SubpixelZoom();
        break;
    }

    // Draw the borders around the magnifier and focus rectangle.
    if (borderPen_ == NULL)
    {
        borderPen_ = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
    }

    HGDIOBJ oldBrush = SelectObject(memoryDC, GetStockObject(NULL_BRUSH));
    HGDIOBJ oldPen = SelectObject(memoryDC, borderPen_);

    Rectangle(
        memoryDC, 
        magnifier_.magnifierPos.x, 
        magnifier_.magnifierPos.y, 
        magnifier_.magnifierPos.x + magnifier_.magnifierSize.cx,
        magnifier_.magnifierPos.y + magnifier_.magnifierSize.cy
        );

    Rectangle(
        memoryDC, 
        magnifier_.focusPos.x,
        magnifier_.focusPos.y, 
        magnifier_.focusPos.x + magnifier_.magnifierSize.cx / magnifier_.scale,
        magnifier_.focusPos.y + magnifier_.magnifierSize.cy / magnifier_.scale
        );

    SelectObject(memoryDC, oldPen);
    SelectObject(memoryDC, oldBrush);
}

void DWriteRenderer::SubpixelZoom()
{
    bool bgr = renderingParams_->GetPixelGeometry() == DWRITE_PIXEL_GEOMETRY_BGR;

    // Get the DIB selection selected into each IDWriteBitmapRenderTarget's memory DC.
    DIBSECTION srcDib;
    if (GetObject(GetCurrentObject(magnifierTarget_->GetMemoryDC(), OBJ_BITMAP), sizeof(srcDib), &srcDib) != sizeof(srcDib))
        return;

    DIBSECTION dstDib;
    if (GetObject(GetCurrentObject(renderTarget_->GetMemoryDC(), OBJ_BITMAP), sizeof(dstDib), &dstDib) != sizeof(dstDib))
        return;

    // Point to the pixels. Each DIB section is a 32-bit per pixel top-down DIB.
    int const srcWidth = srcDib.dsBm.bmWidth;
    int const srcHeight = srcDib.dsBm.bmHeight;
    UINT32 const* const srcBits = static_cast<UINT32*>(srcDib.dsBm.bmBits);

    int const dstWidth = dstDib.dsBm.bmWidth;
    int const dstHeight = dstDib.dsBm.bmHeight;
    UINT32* const dstBits = static_cast<UINT32*>(dstDib.dsBm.bmBits);

    // Number of target pixels per source pixel and source subpixel.
    int const scale = magnifier_.scale;
    int const subpixelScale = scale / 3;
    int const pixelGap = scale % 3;

    // Mask of colors for left, center, and right subpixels.
    UINT32 const maskL = bgr ? 0x0000FF : 0xFF0000;
    UINT32 const maskC = 0x00FF00;
    UINT32 const maskR = bgr ? 0xFF0000 : 0x0000FF;

    // Iterate over all the source scan lines.
    for (int y = 0; y < srcHeight; ++y)
    {
        UINT32 const* srcRow = srcBits + (y * srcWidth);

        // Determine the corresponding range of Y values in the destination bitmap.
        int minDstY = (y * scale) + magnifier_.magnifierPos.y;
        int limDstY = minDstY + scale;

        // Consrain the destination Y values to fit in the destination bitmap.
        if (minDstY < 0)
            minDstY = 0;

        if (limDstY > dstHeight)
            limDstY = dstHeight;

        // Are any of the destination scan lines visible?
        if (minDstY < limDstY)
        {
            UINT32* const firstDstRow = dstBits + (minDstY * dstWidth);

            int dstX = magnifier_.magnifierPos.x;

            // Iterate over all the pixels in the source scan line.
            for (int x = 0; x < srcWidth; ++x)
            {
                UINT32 const color = srcRow[x];

                // Fill in the destination pixels for the left, center,
                // and right color stripes.
                for (int i = 0; i < subpixelScale; ++i, ++dstX)
                {
                    if (dstX >= 0 && dstX < dstWidth)
                        firstDstRow[dstX] = color & maskL;
                }
                for (int i = 0; i < subpixelScale; ++i, ++dstX)
                {
                    if (dstX >= 0 && dstX < dstWidth)
                        firstDstRow[dstX] = color & maskC;
                }
                for (int i = 0; i < subpixelScale; ++i, ++dstX)
                {
                    if (dstX >= 0 && dstX < dstWidth)
                        firstDstRow[dstX] = color & maskR;
                }

                // If the scale is not a multiple if three, we'll have a black
                // gap between the pixels.
                for (int i = 0; i < pixelGap; ++i, ++dstX)
                {
                    if (dstX >= 0 && dstX < dstWidth)
                        firstDstRow[dstX] = 0;
                }
            }

            // Copy the destination row we just initialized to the remaining 
            // destination rows for this scan line.
            UINT32* dstRow = firstDstRow + dstWidth;

            for (int y2 = minDstY + 1; y2 < limDstY; ++y2, dstRow += dstWidth)
            {
                memcpy(
                    dstRow + magnifier_.magnifierPos.x,
                    firstDstRow + magnifier_.magnifierPos.x,
                    (dstX - magnifier_.magnifierPos.x) * sizeof(UINT32)
                    );
            }
        }
    }
}

HRESULT DWriteRenderer::InitializeTextLayout()
{
    HRESULT hr = S_OK;

    if (textLayout_ == NULL)
    {
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
    }

    return hr;
}

HRESULT DWriteRenderer::UpdateTextOrigin()
{
    HRESULT hr = S_OK;

    if (textLayout_ != NULL)
    {
        // Get the text layout size.
        DWRITE_TEXT_METRICS metrics = {};
        hr = textLayout_->GetMetrics(&metrics);

        // Center the text.
        textOriginX_ = (PixelsToDipsX(width_) - metrics.width) * 0.5f;
        textOriginY_ = (PixelsToDipsY(height_) - metrics.height) * 0.5f;
    }

    return hr;
}

void DWriteRenderer::FreeTextLayout()
{
    SafeRelease(&textLayout_);
}

//
// IUnknown methods
//
//      These methods are never called in this scenario so we just use stub
//      implementations.
//
HRESULT STDMETHODCALLTYPE DWriteRenderer::QueryInterface( 
    REFIID riid,
    void** ppvObject
    )
{
    *ppvObject = NULL;
    return E_NOTIMPL;
}

ULONG STDMETHODCALLTYPE DWriteRenderer::AddRef()
{
    return 0;
}

ULONG STDMETHODCALLTYPE DWriteRenderer::Release()
{
    return 0;
}

//
// IDWritePixelSnapping::IsPixelSnappingDisabled
//
HRESULT STDMETHODCALLTYPE DWriteRenderer::IsPixelSnappingDisabled(
    void* clientDrawingContext,
    OUT BOOL* isDisabled
    )
{
    *isDisabled = FALSE;
    return S_OK;
}

//
// IDWritePixelSnapping::GetCurrentTransform
//
HRESULT STDMETHODCALLTYPE DWriteRenderer::GetCurrentTransform(
    void* clientDrawingContext,
    OUT DWRITE_MATRIX* transform
    )
{
    *transform = transform_;
    return S_OK;
}

//
// IDWritePixelSnapping::GetPixelsPerDip
//
HRESULT STDMETHODCALLTYPE DWriteRenderer::GetPixelsPerDip(
    void* clientDrawingContext,
    OUT FLOAT* pixelsPerDip
    )
{
    *pixelsPerDip = g_dpiY / 96.0f;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE DWriteRenderer::DrawGlyphRun(
    void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    DWRITE_MEASURING_MODE measuringMode,
    DWRITE_GLYPH_RUN const* glyphRun,
    DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
    IUnknown* clientDrawingEffect
    )
{
    HRESULT hr = S_OK;

    hr = renderTarget_->DrawGlyphRun(
            baselineOriginX,
            baselineOriginY,
            measuringMode,
            glyphRun,
            renderingParams_,
            GetSysColor(COLOR_WINDOWTEXT)
            );

    if (SUCCEEDED(hr) && magnifierTarget_ != NULL)
    {
        hr = magnifierTarget_->DrawGlyphRun(
                baselineOriginX,
                baselineOriginY,
                measuringMode,
                glyphRun,
                renderingParams_,
                GetSysColor(COLOR_WINDOWTEXT)
                );
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE DWriteRenderer::DrawUnderline(
    void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    DWRITE_UNDERLINE const* underline,
    IUnknown* clientDrawingEffect
    )
{
    // We don't use underline in this application.
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE DWriteRenderer::DrawStrikethrough(
    void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    DWRITE_STRIKETHROUGH const* strikethrough,
    IUnknown* clientDrawingEffect
    )
{
    // We don't use strikethrough in this application.
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE DWriteRenderer::DrawInlineObject(
    void* clientDrawingContext,
    FLOAT originX,
    FLOAT originY,
    IDWriteInlineObject* inlineObject,
    BOOL isSideways,
    BOOL isRightToLeft,
    IUnknown* clientDrawingEffect
    )
{
    // We don't use inline objects in this application.
    return E_NOTIMPL;
}
