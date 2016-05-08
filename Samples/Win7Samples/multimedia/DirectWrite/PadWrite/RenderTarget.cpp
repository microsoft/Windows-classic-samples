// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// Contents:    Adapter render target draws using D2D or DirectWrite.
//              This demonstrates how to implement your own render target
//              for layout drawing callbacks.
//
//----------------------------------------------------------------------------
#include "Common.h"
#include "DrawingEffect.h"
#include "RenderTarget.h"


inline bool operator== (const RenderTargetD2D::ImageCacheEntry& entry, const IWICBitmapSource* original)
{
    return entry.original == original;
}

inline bool operator== (const RenderTargetDW::ImageCacheEntry& entry, const IWICBitmapSource* original)
{
    return entry.original == original;
}


////////////////////////////////////////////////////////////////////////////////
// Direct2D render target.


HRESULT RenderTargetD2D::Create(ID2D1Factory* d2dFactory, IDWriteFactory* dwriteFactory, HWND hwnd, OUT RenderTarget** renderTarget)
{
    *renderTarget = NULL;
    HRESULT hr    = S_OK;

    RenderTargetD2D* newRenderTarget = SafeAcquire(new(std::nothrow) RenderTargetD2D(d2dFactory, dwriteFactory, hwnd));
    if (newRenderTarget == NULL)
    {
        return E_OUTOFMEMORY;
    }

    hr = newRenderTarget->CreateTarget();
    if (FAILED(hr))
        SafeRelease(&newRenderTarget);

    *renderTarget = SafeDetach(&newRenderTarget);

    return hr;
}


RenderTargetD2D::RenderTargetD2D(ID2D1Factory* d2dFactory, IDWriteFactory* dwriteFactory, HWND hwnd)
:   hwnd_(hwnd),
    hmonitor_(NULL),
    d2dFactory_(SafeAcquire(d2dFactory)),
    dwriteFactory_(SafeAcquire(dwriteFactory)),
    target_(),
    brush_()
{
}


RenderTargetD2D::~RenderTargetD2D()
{
    SafeRelease(&brush_);
    SafeRelease(&target_);
    SafeRelease(&d2dFactory_);
    SafeRelease(&dwriteFactory_);
}


HRESULT RenderTargetD2D::CreateTarget()
{
    // Creates a D2D render target set on the HWND.

    HRESULT hr = S_OK;

    // Get the window's pixel size.
    RECT rect = {};
    GetClientRect(hwnd_, &rect);
    D2D1_SIZE_U d2dSize = D2D1::SizeU(rect.right, rect.bottom);

    // Create a D2D render target.
    ID2D1HwndRenderTarget* target = NULL;

    hr = d2dFactory_->CreateHwndRenderTarget(
                    D2D1::RenderTargetProperties(),
                    D2D1::HwndRenderTargetProperties(hwnd_, d2dSize),
                    &target
                    );

    if (SUCCEEDED(hr))
    {
        SafeSet(&target_, target);

        // Any scaling will be combined into matrix transforms rather than an
        // additional DPI scaling. This simplifies the logic for rendering
        // and hit-testing. If an application does not use matrices, then
        // using the scaling factor directly is simpler.
        target->SetDpi(96.0, 96.0);

        // Create a reusable scratch brush, rather than allocating one for
        // each new color.
        SafeRelease(&brush_);
        hr = target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &brush_);
    }

    if (SUCCEEDED(hr))
    {
        // Update the initial monitor rendering parameters.
        UpdateMonitor();
    }

    SafeRelease(&target);

    return hr;
}


void RenderTargetD2D::Resize(UINT width, UINT height)
{
    D2D1_SIZE_U size;
    size.width = width;
    size.height = height;
    target_->Resize(size);
}


void RenderTargetD2D::UpdateMonitor()
{
    // Updates rendering parameters according to current monitor.

    HMONITOR monitor = MonitorFromWindow(hwnd_, MONITOR_DEFAULTTONEAREST);
    if (monitor != hmonitor_)
    {
        // Create based on monitor settings, rather than the defaults of
        // gamma=1.8, contrast=.5, and clearTypeLevel=.5

        IDWriteRenderingParams* renderingParams = NULL;

        dwriteFactory_->CreateMonitorRenderingParams(
                            monitor,
                            &renderingParams
                            );
        target_->SetTextRenderingParams(renderingParams);

        hmonitor_ = monitor;
        InvalidateRect(hwnd_, NULL, FALSE);

        SafeRelease(&renderingParams);
    }
}


void RenderTargetD2D::BeginDraw()
{
    target_->BeginDraw();
    target_->SetTransform(D2D1::Matrix3x2F::Identity());
}


void RenderTargetD2D::EndDraw()
{
    HRESULT hr = target_->EndDraw();

    // If the device is lost for any reason, we need to recreate it.
    if (hr == D2DERR_RECREATE_TARGET)
    {
        // Flush resources and recreate them.
        // This is very rare for a device to be lost,
        // but it can occur when connecting via Remote Desktop.
        imageCache_.clear();
        hmonitor_ = NULL;

        CreateTarget();
    }
}


void RenderTargetD2D::Clear(UINT32 color)
{
    target_->Clear(D2D1::ColorF(color));
}


ID2D1Bitmap* RenderTargetD2D::GetCachedImage(IWICBitmapSource* image)
{
    // Maps a WIC image source to an aready cached D2D bitmap.
    // If not already cached, it creates the D2D bitmap from WIC.

    if (image == NULL)
        return NULL;

    // Find an existing match
    std::vector<ImageCacheEntry>::iterator match = std::find(imageCache_.begin(), imageCache_.end(), image);
    if (match != imageCache_.end())
        return match->converted; // already cached

    // Convert the WIC image to a ready-to-use device-dependent D2D bitmap.
    // This avoids needing to recreate a new texture every draw call, but
    // allows easy reconstruction of textures if the device changes and
    // resources need recreation (also lets callers be D2D agnostic).

    ID2D1Bitmap* bitmap = NULL;
    target_->CreateBitmapFromWicBitmap(image, NULL, &bitmap);
    if (bitmap == NULL)
        return NULL;

    // Save for later calls.
    try
    {
        imageCache_.push_back(ImageCacheEntry(image, bitmap));
    }
    catch (...)
    {
        // Out of memory
        SafeRelease(&bitmap);
        return NULL;
    }

    // Release it locally and return the pointer.
    // The bitmap is now referenced by the bitmap cache.
    bitmap->Release();
    return bitmap;
}


void RenderTargetD2D::FillRectangle(
    const RectF& destRect,
    const DrawingEffect& drawingEffect
    )
{
    ID2D1Brush* brush = GetCachedBrush(&drawingEffect);
    if (brush == NULL)
        return;

    // We will always get a strikethrough as a LTR rectangle
    // with the baseline origin snapped.
    target_->FillRectangle(destRect, brush);
}


void RenderTargetD2D::DrawImage(
    IWICBitmapSource* image,
    const RectF& sourceRect,  // where in source atlas texture
    const RectF& destRect     // where on display to draw it
    )
{
    // Ignore zero size source rects.
    // Draw nothing if the destination is zero size.
    if (&sourceRect    == NULL
    || sourceRect.left >= sourceRect.right
    || sourceRect.top  >= sourceRect.bottom
    || destRect.left   >= destRect.right
    || destRect.top    >= destRect.bottom)
    {
        return;
    }

    ID2D1Bitmap* bitmap = GetCachedImage(image);
    if (bitmap == NULL)
        return;

    target_->DrawBitmap(
            bitmap,
            destRect,
            1.0, // opacity
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
            sourceRect
            );
}


void RenderTargetD2D::DrawTextLayout(
    IDWriteTextLayout* textLayout,
    const RectF& rect
    )
{
    if (textLayout == NULL)
        return;

    Context context(this, NULL);
    textLayout->Draw(
        &context,
        this,
        rect.left,
        rect.top
        );
}


ID2D1Brush* RenderTargetD2D::GetCachedBrush(
    const DrawingEffect* effect
    )
{
    if (effect == NULL || brush_ == NULL)
        return NULL;

    // Update the D2D brush to the new effect color.
    UINT32 bgra = effect->GetColor();
    float alpha = (bgra >> 24) / 255.0f;
    brush_->SetColor(D2D1::ColorF(bgra, alpha));

    return brush_;
}


void RenderTargetD2D::SetTransform(DWRITE_MATRIX const& transform)
{
    target_->SetTransform(reinterpret_cast<const D2D1_MATRIX_3X2_F*>(&transform));
}


void RenderTargetD2D::GetTransform(DWRITE_MATRIX& transform)
{
    target_->GetTransform(reinterpret_cast<D2D1_MATRIX_3X2_F*>(&transform));
}


void RenderTargetD2D::SetAntialiasing(bool isEnabled)
{
    target_->SetAntialiasMode(isEnabled ? D2D1_ANTIALIAS_MODE_PER_PRIMITIVE : D2D1_ANTIALIAS_MODE_ALIASED);
}


HRESULT STDMETHODCALLTYPE RenderTargetD2D::DrawGlyphRun(
    void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    DWRITE_MEASURING_MODE measuringMode,
    const DWRITE_GLYPH_RUN* glyphRun,
    const DWRITE_GLYPH_RUN_DESCRIPTION* glyphRunDescription,
    IUnknown* clientDrawingEffect
    )
{
    // If no drawing effect is applied to run, but a clientDrawingContext
    // is passed, use the one from that instead. This is useful for trimming
    // signs, where they don't have a color of their own.
    clientDrawingEffect = GetDrawingEffect(clientDrawingContext, clientDrawingEffect);

    // Since we use our own custom renderer and explicitly set the effect
    // on the layout, we know exactly what the parameter is and can
    // safely cast it directly.
    DrawingEffect* effect = static_cast<DrawingEffect*>(clientDrawingEffect);
    ID2D1Brush* brush = GetCachedBrush(effect);
    if (brush == NULL)
        return E_FAIL;

    target_->DrawGlyphRun(
        D2D1::Point2(baselineOriginX, baselineOriginY),
        glyphRun,
        brush,
        measuringMode
    );

    return S_OK;
}


HRESULT STDMETHODCALLTYPE RenderTargetD2D::DrawUnderline(
    void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    const DWRITE_UNDERLINE* underline,
    IUnknown* clientDrawingEffect
    )
{
    clientDrawingEffect = GetDrawingEffect(clientDrawingContext, clientDrawingEffect);

    DrawingEffect* effect = static_cast<DrawingEffect*>(clientDrawingEffect);
    ID2D1Brush* brush = GetCachedBrush(effect);
    if (brush == NULL)
        return E_FAIL;

    // We will always get a strikethrough as a LTR rectangle
    // with the baseline origin snapped.
    D2D1_RECT_F rectangle =
    {
        baselineOriginX,
        baselineOriginY + underline->offset,
        baselineOriginX + underline->width,
        baselineOriginY + underline->offset + underline->thickness
   };

    // Draw this as a rectangle, rather than a line.
    target_->FillRectangle(&rectangle, brush);

    return S_OK;
}


HRESULT STDMETHODCALLTYPE RenderTargetD2D::DrawStrikethrough(
    void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    const DWRITE_STRIKETHROUGH* strikethrough,
    IUnknown* clientDrawingEffect
    )
{
    clientDrawingEffect = GetDrawingEffect(clientDrawingContext, clientDrawingEffect);

    DrawingEffect* effect = static_cast<DrawingEffect*>(clientDrawingEffect);
    ID2D1Brush* brush = GetCachedBrush(effect);
    if (brush == NULL)
        return E_FAIL;

    // We will always get an underline as a LTR rectangle
    // with the baseline origin snapped.
    D2D1_RECT_F rectangle =
    {
        baselineOriginX,
        baselineOriginY + strikethrough->offset,
        baselineOriginX + strikethrough->width,
        baselineOriginY + strikethrough->offset + strikethrough->thickness
   };

    // Draw this as a rectangle, rather than a line.
    target_->FillRectangle(&rectangle, brush);

    return S_OK;
}


HRESULT STDMETHODCALLTYPE RenderTargetD2D::DrawInlineObject(
    void* clientDrawingContext,
    FLOAT originX,
    FLOAT originY,
    IDWriteInlineObject* inlineObject,
    BOOL isSideways,
    BOOL isRightToLeft,
    IUnknown* clientDrawingEffect
    )
{
    // Inline objects inherit the drawing effect of the text
    // they are in, so we should pass it down (if none is set
    // on this range, use the drawing context's effect instead).
    Context subContext(*reinterpret_cast<RenderTarget::Context*>(clientDrawingContext));

    if (clientDrawingEffect != NULL)
        subContext.drawingEffect = clientDrawingEffect;

    inlineObject->Draw(
        &subContext,
        this,
        originX,
        originY,
        false,
        false,
        subContext.drawingEffect
        );

    return S_OK;
}


HRESULT STDMETHODCALLTYPE RenderTargetD2D::IsPixelSnappingDisabled(
    void* clientDrawingContext,
    OUT BOOL* isDisabled
    )
{
    // Enable pixel snapping of the text baselines,
    // since we're not animating and don't want blurry text.
    *isDisabled = FALSE;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE RenderTargetD2D::GetCurrentTransform(
    void* clientDrawingContext,
    OUT DWRITE_MATRIX* transform
    )
{
    // Simply forward what the real renderer holds onto.
    target_->GetTransform(reinterpret_cast<D2D1_MATRIX_3X2_F*>(transform));
    return S_OK;
}


HRESULT STDMETHODCALLTYPE RenderTargetD2D::GetPixelsPerDip(
    void* clientDrawingContext,
    OUT FLOAT* pixelsPerDip
    )
{
    // Any scaling will be combined into matrix transforms rather than an
    // additional DPI scaling. This simplifies the logic for rendering
    // and hit-testing. If an application does not use matrices, then
    // using the scaling factor directly is simpler.
    *pixelsPerDip = 1;
    return S_OK;
}


////////////////////////////////////////////////////////////////////////////////
// DirectWrite render target.


HRESULT RenderTargetDW::Create(IDWriteFactory* dwriteFactory, HWND hwnd, OUT RenderTarget** renderTarget)
{
    *renderTarget = NULL;
    HRESULT hr    = S_OK;

    RenderTargetDW* newRenderTarget = SafeAcquire(new(std::nothrow) RenderTargetDW(dwriteFactory, hwnd));
    if (newRenderTarget == NULL)
    {
        return E_OUTOFMEMORY;
    }

    hr = newRenderTarget->Initialize();
    if (FAILED(hr))
        SafeRelease(&newRenderTarget);

    *renderTarget = SafeDetach(&newRenderTarget);

    return hr;
}



RenderTargetDW::RenderTargetDW(IDWriteFactory* dwriteFactory, HWND hwnd)
:   hwnd_(hwnd),
    hmonitor_(NULL),
    dwriteFactory_(SafeAcquire(dwriteFactory)),
    gdiInterop_(),
    renderingParams_(),
    target_()
{
}


HRESULT RenderTargetDW::Initialize()
{
    // Creates a DirectWrite bitmap render target.

    HRESULT hr = S_OK;

    hr = dwriteFactory_->GetGdiInterop(&gdiInterop_);
    if (FAILED(hr))
        return hr;

    RECT rect = {};
    HDC hdc = GetDC(hwnd_);
    GetClientRect(hwnd_, &rect);
    hr = gdiInterop_->CreateBitmapRenderTarget(hdc, rect.right, rect.bottom, &target_);
    ReleaseDC(hwnd_, hdc);

    if (FAILED(hr))
        return hr;

    // Any scaling will be combined into matrix transforms rather than an
    // additional DPI scaling. This simplifies the logic for rendering
    // and hit-testing. If an application does not use matrices, then
    // using the scaling factor directly is simpler.
    target_->SetPixelsPerDip(1);

    // Update the initial monitor rendering parameters.
    UpdateMonitor();

    if (renderingParams_ == NULL)
        return E_FAIL;

    return hr;
}


RenderTargetDW::~RenderTargetDW()
{
    SafeRelease(&renderingParams_);
    SafeRelease(&gdiInterop_);
    SafeRelease(&target_);
    SafeRelease(&dwriteFactory_);

    for (size_t i = 0, ci = imageCache_.size(); i < ci; ++i)
    {
        DeleteObject(imageCache_[i].converted);
    }
}


void RenderTargetDW::Resize(UINT width, UINT height)
{
    target_->Resize(width, height);
}


void RenderTargetDW::UpdateMonitor()
{
    // Updates rendering parameters according to current monitor.

    HMONITOR monitor = MonitorFromWindow(hwnd_, MONITOR_DEFAULTTONEAREST);
    if (monitor != hmonitor_)
    {
        // Create based on monitor settings, rather than the defaults of
        // gamma=1.8, contrast=.5, and clearTypeLevel=.5

        SafeRelease(&renderingParams_);
        dwriteFactory_->CreateMonitorRenderingParams(
                            monitor,
                            &renderingParams_
                            );
        hmonitor_ = monitor;
        InvalidateRect(hwnd_, NULL, FALSE);
    }
}


void RenderTargetDW::BeginDraw()
{
    memoryHdc_ = target_->GetMemoryDC();

    // Explicitly disable mirroring of content, otherwise
    // elements may be drawn reversed.
    SetLayout(memoryHdc_, LAYOUT_BITMAPORIENTATIONPRESERVED);
    SetGraphicsMode(memoryHdc_, GM_ADVANCED);
}


void RenderTargetDW::EndDraw()
{
    SIZE size;
    target_->GetSize(&size);

    HDC hdc = GetDC(hwnd_);

    SetGraphicsMode(memoryHdc_, GM_COMPATIBLE);

    // Transfer from DWrite's rendering target to the actual display
    // Also, explicitly disable mirroring of bitmaps, otherwise the
    // text can be literally drawn backwards, and ClearType is incorrect.
    // Note that we set a raster operation rather than calling SetLayout
    // here.
    BitBlt(
        hdc,
        0, 0,
        size.cx, size.cy,
        memoryHdc_,
        0, 0,
        SRCCOPY | NOMIRRORBITMAP
        );

    ReleaseDC(hwnd_, hdc);
}


void RenderTargetDW::Clear(UINT32 color)
{
    SIZE size;
    target_->GetSize(&size);

    SetDCBrushColor(memoryHdc_, color);
    SelectObject(memoryHdc_, GetStockObject(NULL_PEN));
    SelectObject(memoryHdc_, GetStockObject(DC_BRUSH));
    Rectangle(memoryHdc_, 0,0, size.cx + 1, size.cy + 1);
}


HBITMAP RenderTargetDW::GetCachedImage(IWICBitmapSource* image)
{
    // Maps a WIC image source to an aready cached HBITMAP.
    // If not already cached, it creates the HBITMAP from WIC.

    if (image == NULL)
        return NULL;

    // Find an existing cached match for this image.
    std::vector<ImageCacheEntry>::iterator match = std::find(imageCache_.begin(), imageCache_.end(), image);
    if (match != imageCache_.end())
        return match->converted; // already cached

    // Convert the WIC image to a ready-to-use device-dependent GDI bitmap.
    // so that we don't recreate a new bitmap every draw call.
    //
    // * Note this expects BGRA pixel format.

    UINT width, height;
    if (FAILED(image->GetSize(&width, &height)))
        return NULL;

    UINT stride = width * 4;
    UINT pixelBufferSize = stride * height;
    std::vector<UINT8> pixelBuffer(pixelBufferSize);

    if (FAILED(image->CopyPixels(NULL, stride, pixelBufferSize, &pixelBuffer[0])))
        return NULL;

    // Initialize bitmap information.
    BITMAPINFO bmi = {
        sizeof(BITMAPINFOHEADER),   // biSize
        width,                      // biWidth
        -int(height),               // biHeight
        1,                          // biPlanes
        32,                         // biBitCount
        BI_RGB,                     // biCompression
        pixelBufferSize,            // biSizeImage
        1,                          // biXPelsPerMeter
        1,                          // biYPelsPerMeter
        0,                          // biClrUsed
        0,                          // biClrImportant
        0                           // RGB QUAD
    };

    HBITMAP bitmap = CreateCompatibleBitmap(memoryHdc_, width, height);
    if (bitmap == NULL)
        return NULL;

    SetDIBits(
        memoryHdc_,
        bitmap,
        0,              // starting line
        height,         // total scanlines
        &pixelBuffer[0],
        &bmi,
        DIB_RGB_COLORS
        );

    // Save for later calls.
    try
    {
        imageCache_.push_back(ImageCacheEntry(image, bitmap));
    }
    catch (...)
    {
        // Out of memory
        DeleteObject(bitmap);
        return NULL;
    }

    return bitmap;
}


void RenderTargetDW::FillRectangle(
    const RectF& destRect,
    const DrawingEffect& drawingEffect
    )
{
    // Convert D2D/GDI+ color order to GDI's COLORREF,
    // which expects the lowest byte to be red.
    // The alpha channel must also be cleared.

    // GDI's Rectangle() does not support translucency,
    // so draw nothing. Alternately, you could draw a hash.
    if (drawingEffect.GetColor() < 0xE0000000)
        return;

    COLORREF gdiColor = drawingEffect.GetColorRef();
    SetDCBrushColor(memoryHdc_, gdiColor);

    SelectObject(memoryHdc_, GetStockObject(NULL_PEN));
    SelectObject(memoryHdc_, GetStockObject(DC_BRUSH));
    Rectangle(
        memoryHdc_,
        int(floor(destRect.left   + .5f)),
        int(floor(destRect.top    + .5f)),
        int(floor(destRect.right  + .5f)),
        int(floor(destRect.bottom + .5f))
        );
}


void RenderTargetDW::DrawImage(
    IWICBitmapSource* image,
    const RectF& sourceRect,  // where in source atlas texture
    const RectF& destRect     // where on display to draw it
    )
{
    // Ignore zero size source rects.
    // Draw nothing if the destination is zero size.
    if (&sourceRect == NULL
    || sourceRect.left >= sourceRect.right
    || sourceRect.top  >= sourceRect.bottom
    || destRect.left   >= destRect.right
    || destRect.top    >= destRect.bottom)
    {
        return;
    }

    HBITMAP bitmap = GetCachedImage(image);
    if (bitmap == NULL)
        return;

    HDC tempHdc = CreateCompatibleDC(memoryHdc_);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(tempHdc, bitmap);

    const static BLENDFUNCTION blend = {
        AC_SRC_OVER, // blend-op
        0, // flags
        255, // alpha
        AC_SRC_ALPHA
    };
    // Notice that since GDI doesn't accept floating point coordinates,
    // we round them to the nearest integer. This helps align the image's
    // baseline to that of nearby text, since round-nearest is the
    // same method generally used by D2D and DirectWrite internally.
    AlphaBlend(
        memoryHdc_,
        int(floor(destRect.left   + .5f)),
        int(floor(destRect.top    + .5f)),
        int(floor(destRect.right  - destRect.left + .5f)),
        int(floor(destRect.bottom - destRect.top  + .5f)),
        tempHdc,
        int(sourceRect.left),
        int(sourceRect.top),
        int(sourceRect.right - sourceRect.left),
        int(sourceRect.bottom - sourceRect.top),
        blend
        );

    SelectObject(tempHdc, oldBitmap);
    DeleteDC(tempHdc);
}


void RenderTargetDW::DrawTextLayout(
    IDWriteTextLayout* textLayout,
    const RectF& rect
    )
{
    if (textLayout == NULL)
        return;

    Context context(this, NULL);
    textLayout->Draw(
        &context,
        this,
        rect.left,
        rect.top
        );
}


void RenderTargetDW::SetTransform(DWRITE_MATRIX const& transform)
{
    target_->SetCurrentTransform(&transform);
    SetGraphicsMode(memoryHdc_, GM_ADVANCED);
    SetWorldTransform(memoryHdc_, (XFORM*)&transform);
}


void RenderTargetDW::GetTransform(DWRITE_MATRIX& transform)
{
    target_->GetCurrentTransform(&transform);
}


HRESULT STDMETHODCALLTYPE RenderTargetDW::DrawGlyphRun(
    void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    DWRITE_MEASURING_MODE measuringMode,
    const DWRITE_GLYPH_RUN* glyphRun,
    const DWRITE_GLYPH_RUN_DESCRIPTION* glyphRunDescription,
    IUnknown* clientDrawingEffect
    )
{
    // If no drawing effect is applied to run, but a clientDrawingContext
    // is passed, use the one from that instead. This is useful for trimming
    // signs, where they don't have a color of their own.
    clientDrawingEffect = GetDrawingEffect(clientDrawingContext, clientDrawingEffect);
    
    // Since we use our own custom renderer and explicitly set the effect
    // on the layout, we know exactly what the parameter is and can
    // safely cast it directly.
    DrawingEffect* effect = static_cast<DrawingEffect*>(clientDrawingEffect);
    if (effect == NULL)
        return E_FAIL;

    // Pass on the drawing call to the render target to do the real work.
    target_->DrawGlyphRun(
        baselineOriginX,
        baselineOriginY,
        measuringMode,
        glyphRun,
        renderingParams_,
        effect->GetColorRef(),
        NULL
        );

    return S_OK;
}


HRESULT STDMETHODCALLTYPE RenderTargetDW::DrawUnderline(
    void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    const DWRITE_UNDERLINE* underline,
    IUnknown* clientDrawingEffect
    )
{
    clientDrawingEffect = GetDrawingEffect(clientDrawingContext, clientDrawingEffect);

    return DrawLine(
        baselineOriginX,
        baselineOriginY,
        underline->width,
        underline->offset,
        underline->thickness,
        clientDrawingEffect
        );
}


HRESULT STDMETHODCALLTYPE RenderTargetDW::DrawStrikethrough(
    void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    const DWRITE_STRIKETHROUGH* strikethrough,
    IUnknown* clientDrawingEffect
    )
{
    clientDrawingEffect = GetDrawingEffect(clientDrawingContext, clientDrawingEffect);

    return DrawLine(
        baselineOriginX,
        baselineOriginY,
        strikethrough->width,
        strikethrough->offset,
        strikethrough->thickness,
        clientDrawingEffect
        );
}


HRESULT RenderTargetDW::DrawLine(
    float baselineOriginX,
    float baselineOriginY,
    float width,
    float offset,
    float thickness,
    IUnknown* clientDrawingEffect
    )
{
    // Get solid color from drawing effect.
    DrawingEffect* effect = static_cast<DrawingEffect*>(clientDrawingEffect);
    if (effect == NULL)
        return E_FAIL;

    HDC hdc = target_->GetMemoryDC();
    RECT rect = {
        LONG(baselineOriginX),
        LONG(baselineOriginY + offset),
        LONG(baselineOriginX + width),
        LONG(baselineOriginY + offset + thickness),
        };

    // Account for the possibility that the line became zero height,
    // which can occur at small font sizes.
    if (rect.bottom == rect.top)
        rect.bottom++;

    // Draw the line
    // Note that GDI wants RGB, not BGRA like D2D.

    SetBkColor(hdc, effect->GetColorRef());
    ExtTextOut(
        hdc,
        0, 0,
        ETO_OPAQUE,
        &rect,
        L"",
        0,
        NULL
        );

    return S_OK;
}


HRESULT STDMETHODCALLTYPE RenderTargetDW::DrawInlineObject(
    void* clientDrawingContext,
    FLOAT originX,
    FLOAT originY,
    IDWriteInlineObject* inlineObject,
    BOOL isSideways,
    BOOL isRightToLeft,
    IUnknown* clientDrawingEffect
    )
{
    // Inline objects inherit the drawing effect of the text
    // they are in, so we should pass it down (if none is set
    // on this range, use the drawing context's effect instead).
    Context subContext(*reinterpret_cast<RenderTarget::Context*>(clientDrawingContext));

    if (clientDrawingEffect != NULL)
        subContext.drawingEffect = clientDrawingEffect;

    inlineObject->Draw(
        &subContext,
        this,
        originX,
        originY,
        false,
        false,
        subContext.drawingEffect
        );

    return S_OK;
}


HRESULT STDMETHODCALLTYPE RenderTargetDW::IsPixelSnappingDisabled(
    void* clientDrawingContext,
    OUT BOOL* isDisabled
    )
{
    // Enable pixel snapping of the text baselines,
    // since we're not animating and don't want blurry text.
    *isDisabled = FALSE;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE RenderTargetDW::GetCurrentTransform(
    void* clientDrawingContext,
    OUT DWRITE_MATRIX* transform
    )
{
    // Simply forward what the real renderer holds onto.
    target_->GetCurrentTransform(transform);
    return S_OK;
}


HRESULT STDMETHODCALLTYPE RenderTargetDW::GetPixelsPerDip(
    void* clientDrawingContext,
    OUT FLOAT* pixelsPerDip
    )
{
    // Simply forward what the real renderer holds onto.
    *pixelsPerDip = target_->GetPixelsPerDip();
    return S_OK;
}
