// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// Contents:    Sink interface for where text is placed.
//
//----------------------------------------------------------------------------
#include "common.h"
#include "FlowSink.h"


STDMETHODIMP FlowLayoutSink::Reset()
{
    glyphRuns_.clear();
    glyphIndices_.clear();
    glyphAdvances_.clear();
    glyphOffsets_.clear();

    return S_OK;
}


STDMETHODIMP FlowLayoutSink::Prepare(UINT32 glyphCount)
{
    try
    {
        // Reserve a known glyph count up front.
        glyphIndices_.reserve (glyphCount);
        glyphAdvances_.reserve(glyphCount);
        glyphOffsets_.reserve (glyphCount);
    }
    catch (...)
    {
        return ExceptionToHResult();
    }

    return S_OK;
}


STDMETHODIMP FlowLayoutSink::SetGlyphRun(
    float x,
    float y,
    UINT32 glyphCount,
    const UINT16* glyphIndices,                 // [glyphCount]
    const float* glyphAdvances,                 // [glyphCount]
    const DWRITE_GLYPH_OFFSET* glyphOffsets,    // [glyphCount]
    IDWriteFontFace* fontFace,
    float fontEmSize,
    UINT8 bidiLevel,
    bool isSideways
    )
{
    // Append this glyph run to the list.

    try
    {
        glyphRuns_.resize(glyphRuns_.size() + 1);
        CustomGlyphRun& glyphRun = glyphRuns_.back();
        UINT32 glyphStart = static_cast<UINT32>(glyphAdvances_.size());

        glyphIndices_.insert (glyphIndices_.end(),  glyphIndices,  glyphIndices  + glyphCount);
        glyphAdvances_.insert(glyphAdvances_.end(), glyphAdvances, glyphAdvances + glyphCount);
        glyphOffsets_.insert (glyphOffsets_.end(),  glyphOffsets,  glyphOffsets  + glyphCount);

        glyphRun.x          = x;
        glyphRun.y          = y;
        glyphRun.bidiLevel  = bidiLevel;
        glyphRun.glyphStart = glyphStart;
        glyphRun.isSideways = isSideways;
        glyphRun.glyphCount = glyphCount;
        glyphRun.fontEmSize = fontEmSize;
        SafeSet(&glyphRun.fontFace, fontFace);
    }
    catch (...)
    {
        return ExceptionToHResult();
    }

    return S_OK;
}


STDMETHODIMP FlowLayoutSink::DrawGlyphRuns(
    IDWriteBitmapRenderTarget* renderTarget,
    IDWriteRenderingParams* renderingParams,
    COLORREF textColor
    ) const
{
    // Just iterate through all the saved glyph runs
    // and have DWrite to draw each one.

    HRESULT hr = S_OK;

    for (size_t i = 0; i < glyphRuns_.size(); ++i)
    {
        DWRITE_GLYPH_RUN glyphRun;
        const CustomGlyphRun& customGlyphRun = glyphRuns_[i];

        if (customGlyphRun.glyphCount == 0)
            continue;

        // Massage the custom glyph run to something directly
        // digestable by DrawGlyphRun.
        customGlyphRun.Convert(
            &glyphIndices_ [0],
            &glyphAdvances_[0],
            &glyphOffsets_ [0],
            &glyphRun
            );

        hr = renderTarget->DrawGlyphRun(
                customGlyphRun.x,
                customGlyphRun.y,
                DWRITE_MEASURING_MODE_NATURAL,
                &glyphRun,
                renderingParams,
                textColor,
                NULL // don't need blackBoxRect
                );
        if (FAILED(hr))
            break;
    }

    return hr;
}


void FlowLayoutSink::CustomGlyphRun::Convert(
    const UINT16* glyphIndices,                 // [this->glyphCount]
    const float* glyphAdvances,                 // [this->glyphCount]
    const DWRITE_GLYPH_OFFSET* glyphOffsets,    // [this->glyphCount]
    OUT DWRITE_GLYPH_RUN* glyphRun
    ) const throw()
{
    // Populate the DWrite glyph run.

    glyphRun->glyphIndices  = &glyphIndices [glyphStart];
    glyphRun->glyphAdvances = &glyphAdvances[glyphStart];
    glyphRun->glyphOffsets  = &glyphOffsets [glyphStart];
    glyphRun->glyphCount    = glyphCount;
    glyphRun->fontEmSize    = fontEmSize;
    glyphRun->fontFace      = fontFace;
    glyphRun->bidiLevel     = bidiLevel;
    glyphRun->isSideways    = isSideways;
}
