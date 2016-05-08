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

class DWriteRenderer : public IRenderer, private IDWriteTextRenderer
{
public:
    DWriteRenderer(
        HWND hwnd,
        UINT width,
        UINT height,
        IDWriteTextFormat* textFormat,
        wchar_t const* text
        );

    ~DWriteRenderer()
    {
        SafeRelease(&textFormat_);
        SafeRelease(&textLayout_);
        SafeRelease(&renderingParams_);
        SafeRelease(&renderTarget_);
        SafeRelease(&magnifierTarget_);
        DeleteObject(borderPen_);
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

    // IUnknown methods
    IFACEMETHOD(QueryInterface)( 
        REFIID riid,
        void** ppvObject
        );
    IFACEMETHOD_(ULONG, AddRef)();
    IFACEMETHOD_(ULONG, Release)();

    // IDWritePixelSnapping methods
    IFACEMETHOD(IsPixelSnappingDisabled)(
        void* clientDrawingContext,
        OUT BOOL* isDisabled
        );
    IFACEMETHOD(GetCurrentTransform)(
        void* clientDrawingContext,
        OUT DWRITE_MATRIX* transform
        );
    IFACEMETHOD(GetPixelsPerDip)(
        void* clientDrawingContext,
        OUT FLOAT* pixelsPerDip
        );

    // IDWriteTextRenderer methods
    IFACEMETHOD(DrawGlyphRun)(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_MEASURING_MODE measuringMode,
        DWRITE_GLYPH_RUN const* glyphRun,
        DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
        IUnknown* clientDrawingEffect
        );

    IFACEMETHOD(DrawUnderline)(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_UNDERLINE const* underline,
        IUnknown* clientDrawingEffect
        );

    IFACEMETHOD(DrawStrikethrough)(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_STRIKETHROUGH const* strikethrough,
        IUnknown* clientDrawingEffect
        );

    IFACEMETHOD(DrawInlineObject)(
        void* clientDrawingContext,
        FLOAT originX,
        FLOAT originY,
        IDWriteInlineObject* inlineObject,
        BOOL isSideways,
        BOOL isRightToLeft,
        IUnknown* clientDrawingEffect
        );

private:
    HRESULT PrepareMagnifier(HDC hdc);
    void DrawMagnifier();
    void SubpixelZoom();

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
    __nullterminated wchar_t const* text_;

    // Rendering interfaces.
    IDWriteBitmapRenderTarget* renderTarget_;
    IDWriteBitmapRenderTarget* magnifierTarget_;
    IDWriteRenderingParams* renderingParams_;
    HPEN borderPen_;

    // Text layout.
    IDWriteTextLayout* textLayout_;
    float textOriginX_;
    float textOriginY_;
};
