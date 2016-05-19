
/************************************************************************
 *
 * File: GdiTextRenderer.cpp
 *
 * Description: 
 * 
 * 
 *  This file is part of the Microsoft Windows SDK Code Samples.
 * 
 *  Copyright (C) Microsoft Corporation.  All rights reserved.
 * 
 * This source code is intended only as a supplement to Microsoft
 * Development Tools and/or on-line documentation.  See these other
 * materials for detailed information regarding Microsoft code samples.
 * 
 * THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 * 
 ************************************************************************/

#include "GdiInterop.h"

/******************************************************************
*                                                                 *
*  GdiTextRenderer::GdiTextRenderer                               *
*                                                                 *
*  The constructor stores render target and the rendering params. *
*                                                                 *
******************************************************************/

GdiTextRenderer::GdiTextRenderer(
    IDWriteBitmapRenderTarget* bitmapRenderTarget,
    IDWriteRenderingParams* renderingParams
    )
:
cRefCount_(0), 
pRenderTarget_(bitmapRenderTarget),
pRenderingParams_(renderingParams)
{
    pRenderTarget_->AddRef();
    pRenderingParams_->AddRef();
}

/******************************************************************
*                                                                 *
*  GdiTextRenderer::GdiTextRenderer                               *
*                                                                 *
*  Destructor releases the interfaces passed when the class was   *
*  created.                                                       *
*                                                                 *
******************************************************************/

GdiTextRenderer::~GdiTextRenderer()
{
    SafeRelease(&pRenderTarget_);
    SafeRelease(&pRenderingParams_);
}


/******************************************************************
*                                                                 *
*  GdiTextRenderer::DrawGlyphRun                                  *
*                                                                 *
*  Gets GlyphRun outlines via IDWriteFontFace::GetGlyphRunOutline *
*  and then draws and fills them using Direct2D path geometries   *
*                                                                 *
******************************************************************/
STDMETHODIMP GdiTextRenderer::DrawGlyphRun(
    __maybenull void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    DWRITE_MEASURING_MODE measuringMode,
    __in DWRITE_GLYPH_RUN const* glyphRun,
    __in DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
    IUnknown* clientDrawingEffect
    )
{
    HRESULT hr = S_OK;

    // Pass on the drawing call to the render target to do the real work.
    RECT dirtyRect = {0};

    hr = pRenderTarget_->DrawGlyphRun(
        baselineOriginX,
        baselineOriginY,
        measuringMode,
        glyphRun,
        pRenderingParams_,
        RGB(0,200,255),
        &dirtyRect
        );
    

    return hr;
}

/******************************************************************
*                                                                 *
*  GdiTextRenderer::DrawUnderline                                 *
*                                                                 *
*  This function is not implemented for the purposes of this      *
*  sample.                                                        *
*                                                                 *
******************************************************************/

STDMETHODIMP GdiTextRenderer::DrawUnderline(
    __maybenull void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    __in DWRITE_UNDERLINE const* underline,
    IUnknown* clientDrawingEffect
    )
{
// Not implemented
    return E_NOTIMPL;
}

/******************************************************************
*                                                                 *
*  GdiTextRenderer::DrawStrikethrough                             *
*                                                                 *
*  This function is not implemented for the purposes of this      *
*  sample.                                                        *
*                                                                 *
******************************************************************/

STDMETHODIMP GdiTextRenderer::DrawStrikethrough(
    __maybenull void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    __in DWRITE_STRIKETHROUGH const* strikethrough,
    IUnknown* clientDrawingEffect
    )
{
// Not implemented
    return E_NOTIMPL;
}

/******************************************************************
*                                                                 *
*  GdiTextRenderer::DrawInlineObject                              *
*                                                                 *
*  This function is not implemented for the purposes of this      *
*  sample.                                                        *
*                                                                 *
******************************************************************/

STDMETHODIMP GdiTextRenderer::DrawInlineObject(
    __maybenull void* clientDrawingContext,
    FLOAT originX,
    FLOAT originY,
    IDWriteInlineObject* inlineObject,
    BOOL isSideways,
    BOOL isRightToLeft,
    IUnknown* clientDrawingEffect
    )
{
    // Not implemented
    return E_NOTIMPL;
}

/******************************************************************
*                                                                 *
*  GdiTextRenderer::AddRef                                        *
*                                                                 *
*  Increments the ref count                                       *
*                                                                 *
******************************************************************/

STDMETHODIMP_(unsigned long) GdiTextRenderer::AddRef()
{
    return InterlockedIncrement(&cRefCount_);
}

/******************************************************************
*                                                                 *
*  GdiTextRenderer::Release                                       *
*                                                                 *
*  Decrements the ref count and deletes the instance if the ref   *
*  count becomes 0                                                *
*                                                                 *
******************************************************************/

STDMETHODIMP_(unsigned long) GdiTextRenderer::Release()
{
    long newCount = InterlockedDecrement(&cRefCount_);

    if (newCount == 0)
    {
        delete this;
        return 0;
    }
    return newCount;
}

/******************************************************************
*                                                                 *
*  GdiTextRenderer::IsPixelSnappingDisabled                       *
*                                                                 *
*  Determines whether pixel snapping is disabled. The recommended *
*  default is FALSE, unless doing animation that requires         *
*  subpixel vertical placement.                                   *
*                                                                 *
******************************************************************/

STDMETHODIMP GdiTextRenderer::IsPixelSnappingDisabled(
    __maybenull void* clientDrawingContext,
    __out BOOL* isDisabled
    )
{
    *isDisabled = FALSE;
    return S_OK;
}

/******************************************************************
*                                                                 *
*  GdiTextRenderer::GetCurrentTransform                           *
*                                                                 *
*  Returns the current transform applied to the render target..   *
*                                                                 *
******************************************************************/

STDMETHODIMP GdiTextRenderer::GetCurrentTransform(
    __maybenull void* clientDrawingContext,
    __out DWRITE_MATRIX* transform
    )
{
    //forward the render target's transform
    pRenderTarget_->GetCurrentTransform(transform);
    return S_OK;
}

/******************************************************************
*                                                                 *
*  GdiTextRenderer::GetPixelsPerDip                               *
*                                                                 *
*  This returns the number of pixels per DIP.                     *
*                                                                 *
******************************************************************/

STDMETHODIMP GdiTextRenderer::GetPixelsPerDip(
    __maybenull void* clientDrawingContext,
    __out FLOAT* pixelsPerDip
    )
{
    *pixelsPerDip = pRenderTarget_->GetPixelsPerDip();
    return S_OK;
}

/******************************************************************
*                                                                 *
*  GdiTextRenderer::QueryInterface                                *
*                                                                 *
*  Query interface implementation                                 *
*                                                                 *
******************************************************************/

STDMETHODIMP GdiTextRenderer::QueryInterface(
    IID const& riid,
    void** ppvObject
    )
{
    if (__uuidof(IDWriteTextRenderer) == riid)
    {
        *ppvObject = this;
    }
    else if (__uuidof(IDWritePixelSnapping) == riid)
    {
        *ppvObject = this;
    }
    else if (__uuidof(IUnknown) == riid)
    {
        *ppvObject = this;
    }
    else
    {
        *ppvObject = NULL;
        return E_FAIL;
    }

    return S_OK;
}

