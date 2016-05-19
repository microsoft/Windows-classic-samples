// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// Contents:    Custom layout, demonstrating usage of shaping and glyph
//              results.
//
//----------------------------------------------------------------------------
#include "common.h"
#include "TextAnalysis.h"
#include "FlowSource.h"
#include "FlowSink.h"
#include "FlowLayout.h"


namespace
{
    // Estimates the maximum number of glyph indices needed to hold a string of 
    // a given length.  This is the formula given in the Uniscribe SDK and should
    // cover most cases. Degenerate cases will require a reallocation.
    UINT32 EstimateGlyphCount(UINT32 textLength)
    {
        return 3 * textLength / 2 + 16;
    }
}


STDMETHODIMP FlowLayout::SetTextFormat(IDWriteTextFormat* textFormat)
{
    // Initializes properties using a text format, like font family, font size,
    // and reading direction. For simplicity, this custom layout supports
    // minimal formatting. No mixed formatting or alignment modes are supported.

    HRESULT hr = S_OK;

    IDWriteFontCollection*  fontCollection  = NULL;
    IDWriteFontFamily*      fontFamily      = NULL;
    IDWriteFont*            font            = NULL;

    wchar_t fontFamilyName[100];

    readingDirection_   = textFormat->GetReadingDirection();
    fontEmSize_         = textFormat->GetFontSize();

    hr = textFormat->GetLocaleName(localeName_, ARRAYSIZE(localeName_));

    ////////////////////
    // Map font and style to fontFace.

    if (SUCCEEDED(hr))
    {
        // Need the font collection to map from font name to actual font.
        textFormat->GetFontCollection(&fontCollection);
        if (fontCollection == NULL)
        {
            // No font collection was set in the format, so use the system default.
            hr = dwriteFactory_->GetSystemFontCollection(&fontCollection);
        }
    }

    // Find matching family name in collection.
    if (SUCCEEDED(hr))
    {
        hr = textFormat->GetFontFamilyName(fontFamilyName, ARRAYSIZE(fontFamilyName));
    }

    UINT32 fontIndex  = 0;
    if (SUCCEEDED(hr))
    {
        BOOL fontExists = false;
        hr = fontCollection->FindFamilyName(fontFamilyName, &fontIndex, &fontExists);
        if (!fontExists)
        {
            // If the given font does not exist, take what we can get
            // (displaying something instead nothing), choosing the foremost
            // font in the collection.
            fontIndex = 0;
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = fontCollection->GetFontFamily(fontIndex, &fontFamily);
    }

    if (SUCCEEDED(hr))
    {
        hr = fontFamily->GetFirstMatchingFont(
                textFormat->GetFontWeight(),
                textFormat->GetFontStretch(),
                textFormat->GetFontStyle(),
                &font
                );
    }

    if (SUCCEEDED(hr))
    {
        SafeRelease(&fontFace_);
        hr = font->CreateFontFace(&fontFace_);
    }

    SafeRelease(&font);
    SafeRelease(&fontFamily);
    SafeRelease(&fontCollection);

    return S_OK;
}


STDMETHODIMP FlowLayout::SetNumberSubstitution(IDWriteNumberSubstitution* numberSubstitution)
{
    SafeSet(&numberSubstitution_, numberSubstitution);

    return S_OK;
}


STDMETHODIMP FlowLayout::AnalyzeText(
    const wchar_t* text,                // [textLength]
    UINT32 textLength
    ) throw()
{
    // Analyzes the given text and keeps the results for later reflow.

    isTextAnalysisComplete_ = false;

    if (fontFace_ == NULL)
        return E_FAIL; // Need a font face to determine metrics.

    HRESULT hr = S_OK;

    try
    {
        text_.assign(text, textLength);
    }
    catch (...)
    {
        hr = ExceptionToHResult();
    }

    // Query for the text analyzer's interface.
    IDWriteTextAnalyzer* textAnalyzer = NULL;
    if (SUCCEEDED(hr))
    {
        hr = dwriteFactory_->CreateTextAnalyzer(&textAnalyzer);
    }

    // Record the analyzer's results.
    if (SUCCEEDED(hr))
    {
        TextAnalysis textAnalysis(text, textLength, localeName_, numberSubstitution_, readingDirection_);
        hr = textAnalysis.GenerateResults(textAnalyzer, runs_, breakpoints_);
    }

    // Convert the entire text to glyphs.
    if (SUCCEEDED(hr))
    {
        hr = ShapeGlyphRuns(textAnalyzer);
    }

    if (SUCCEEDED(hr))
    {
        isTextAnalysisComplete_ = true;
    }

    SafeRelease(&textAnalyzer);

    return hr;
}


STDMETHODIMP FlowLayout::ShapeGlyphRuns(IDWriteTextAnalyzer* textAnalyzer)
{
    // Shapes all the glyph runs in the layout.

    HRESULT hr = S_OK;

    UINT32 textLength = static_cast<UINT32>(text_.size());

    // Estimate the maximum number of glyph indices needed to hold a string.
    UINT32 estimatedGlyphCount = EstimateGlyphCount(textLength);

    try
    {
        glyphIndices_.resize(estimatedGlyphCount);
        glyphOffsets_.resize(estimatedGlyphCount);
        glyphAdvances_.resize(estimatedGlyphCount);
        glyphClusters_.resize(textLength);

        UINT32 glyphStart = 0;

        // Shape each run separately. This is needed whenever script, locale,
        // or reading direction changes.
        for (UINT32 runIndex = 0; runIndex < runs_.size(); ++runIndex)
        {
            hr = ShapeGlyphRun(textAnalyzer, runIndex, glyphStart);
            if (FAILED(hr))
                break;
        }

        glyphIndices_.resize(glyphStart);
        glyphOffsets_.resize(glyphStart);
        glyphAdvances_.resize(glyphStart);
    }
    catch (...)
    {
        hr = ExceptionToHResult();
    }

    return hr;
}


STDMETHODIMP FlowLayout::ShapeGlyphRun(
    IDWriteTextAnalyzer* textAnalyzer,
    UINT32 runIndex,
    IN OUT UINT32& glyphStart
    )
{
    // Shapes a single run of text into glyphs.
    // Alternately, you could iteratively interleave shaping and line
    // breaking to reduce the number glyphs held onto at once. It's simpler
    // for this demostration to just do shaping and line breaking as two
    // separate processes, but realize that this does have the consequence that
    // certain advanced fonts containing line specific features (like Gabriola)
    // will shape as if the line is not broken.

    HRESULT hr = S_OK;

    try
    {
        TextAnalysis::Run& run  = runs_[runIndex];
        UINT32 textStart        = run.textStart;
        UINT32 textLength       = run.textLength;
        UINT32 maxGlyphCount    = static_cast<UINT32>(glyphIndices_.size() - glyphStart);
        UINT32 actualGlyphCount = 0;

        run.glyphStart          = glyphStart;
        run.glyphCount          = 0;

        if (textLength == 0)
            return S_OK; // Nothing to do..

        HRESULT hr = S_OK;

        ////////////////////
        // Allocate space for shaping to fill with glyphs and other information,
        // with about as many glyphs as there are text characters. We'll actually
        // need more glyphs than codepoints if they are decomposed into separate
        // glyphs, or fewer glyphs than codepoints if multiple are substituted
        // into a single glyph. In any case, the shaping process will need some
        // room to apply those rules to even make that determintation.

        if (textLength > maxGlyphCount)
        {
            maxGlyphCount = EstimateGlyphCount(textLength);
            UINT32 totalGlyphsArrayCount = glyphStart + maxGlyphCount;
            glyphIndices_.resize(totalGlyphsArrayCount);
        }

        std::vector<DWRITE_SHAPING_TEXT_PROPERTIES>  textProps(textLength);
        std::vector<DWRITE_SHAPING_GLYPH_PROPERTIES> glyphProps(maxGlyphCount);

        ////////////////////
        // Get the glyphs from the text, retrying if needed.

        int tries = 0;
        do
        {
            hr = textAnalyzer->GetGlyphs(
                &text_[textStart],
                textLength,
                fontFace_,
                run.isSideways,         // isSideways,
                (run.bidiLevel & 1),    // isRightToLeft
                &run.script,
                localeName_,
                (run.isNumberSubstituted) ? numberSubstitution_ : NULL,
                NULL,                   // features
                NULL,                   // featureLengths
                0,                      // featureCount
                maxGlyphCount,          // maxGlyphCount
                &glyphClusters_[textStart],
                &textProps[0],
                &glyphIndices_[glyphStart],
                &glyphProps[0],
                &actualGlyphCount
                );
            tries++;

            if (hr == E_NOT_SUFFICIENT_BUFFER)
            {
                // Try again using a larger buffer.
                maxGlyphCount                = EstimateGlyphCount(maxGlyphCount);
                UINT32 totalGlyphsArrayCount = glyphStart + maxGlyphCount;

                glyphProps.resize(maxGlyphCount);
                glyphIndices_.resize(totalGlyphsArrayCount);
            }
            else
            {
                break;
            }
        } while (tries < 2); // We'll give it two chances.

        if (FAILED(hr))
            return hr;

        ////////////////////
        // Get the placement of the all the glyphs.

        glyphAdvances_.resize(std::max(static_cast<size_t>(glyphStart + actualGlyphCount), glyphAdvances_.size()));
        glyphOffsets_.resize( std::max(static_cast<size_t>(glyphStart + actualGlyphCount), glyphOffsets_.size()));

        hr = textAnalyzer->GetGlyphPlacements(
            &text_[textStart],
            &glyphClusters_[textStart],
            &textProps[0],
            textLength,
            &glyphIndices_[glyphStart],
            &glyphProps[0],
            actualGlyphCount,
            fontFace_,
            fontEmSize_,
            run.isSideways,
            (run.bidiLevel & 1),    // isRightToLeft
            &run.script,
            localeName_,
            NULL,                   // features
            NULL,                   // featureRangeLengths
            0,                      // featureRanges
            &glyphAdvances_[glyphStart],
            &glyphOffsets_[glyphStart]
            );

        if (FAILED(hr))
            return hr;

        ////////////////////
        // Certain fonts, like Batang, contain glyphs for hidden control
        // and formatting characters. So we'll want to explicitly force their
        // advance to zero.
        if (run.script.shapes & DWRITE_SCRIPT_SHAPES_NO_VISUAL)
        {
            std::fill(glyphAdvances_.begin() + glyphStart,
                      glyphAdvances_.begin() + glyphStart + actualGlyphCount,
                      0.0f
                      );
        }

        ////////////////////
        // Set the final glyph count of this run and advance the starting glyph.
        run.glyphCount = actualGlyphCount;
        glyphStart    += actualGlyphCount;
    }
    catch (...)
    {
        hr = ExceptionToHResult();
    }

    return hr;
}


STDMETHODIMP FlowLayout::FlowText(
    FlowLayoutSource* flowSource,
    FlowLayoutSink* flowSink
    )
{
    // Reflow all the text, from source to sink.

    if (!isTextAnalysisComplete_)
        return E_FAIL;

    HRESULT hr = S_OK;

    // Determine the font line height, needed by the flow source.
    DWRITE_FONT_METRICS fontMetrics = {};
    fontFace_->GetMetrics(&fontMetrics);

    float fontHeight = (fontMetrics.ascent + fontMetrics.descent + fontMetrics.lineGap)
                     * fontEmSize_ / fontMetrics.designUnitsPerEm;

    // Get ready for series of glyph runs.
    hr = flowSink->Prepare(static_cast<UINT32>(glyphIndices_.size()));

    if (SUCCEEDED(hr))
    {
        // Set initial cluster position to beginning of text.
        ClusterPosition cluster, nextCluster;
        SetClusterPosition(cluster, 0);

        FlowLayoutSource::RectF rect;
        UINT32 textLength = static_cast<UINT32>(text_.size());

        // Iteratively pull rect's from the source,
        // and push as much text will fit to the sink.
        while (cluster.textPosition < textLength)
        {
            // Pull the next rect from the source.
            if (FAILED(flowSource->GetNextRect(fontHeight, &rect)))
                break;

            if (rect.right - rect.left <= 0)
                break; // Stop upon reaching zero sized rects.

            // Fit as many clusters between breakpoints that will go in.
            if (FAILED(FitText(cluster, textLength, rect.right - rect.left, &nextCluster)))
                break;

            // Push the glyph runs to the sink.
            if (FAILED(ProduceGlyphRuns(flowSink, rect, cluster, nextCluster)))
                break;

            cluster = nextCluster;
        }
    }

    return hr;
}


STDMETHODIMP FlowLayout::FitText(
    const ClusterPosition& clusterStart,
    UINT32 textEnd,
    float maxWidth,
    OUT ClusterPosition* clusterEnd
    )
{
    // Fits as much text as possible into the given width,
    // using the clusters and advances returned by DWrite.

    ////////////////////////////////////////
    // Set the starting cluster to the starting text position,
    // and continue until we exceed the maximum width or hit
    // a hard break.
    ClusterPosition cluster(clusterStart);
    ClusterPosition nextCluster(clusterStart);
    UINT32 validBreakPosition   = cluster.textPosition;
    UINT32 bestBreakPosition    = cluster.textPosition;
    float textWidth             = 0;

    while (cluster.textPosition < textEnd)
    {
        // Use breakpoint information to find where we can safely break words.
        AdvanceClusterPosition(nextCluster);
        const DWRITE_LINE_BREAKPOINT breakpoint = breakpoints_[nextCluster.textPosition - 1];

        // Check whether we exceeded the amount of text that can fit,
        // unless it's whitespace, which we allow to flow beyond the end.

        textWidth += GetClusterRangeWidth(cluster, nextCluster);
        if (textWidth > maxWidth && !breakpoint.isWhitespace)
        {
            // Want a minimum of one cluster.
            if (validBreakPosition > clusterStart.textPosition)
                break;
        }

        validBreakPosition = nextCluster.textPosition;

        // See if we can break after this character cluster, and if so,
        // mark it as the new potential break point.
        if (breakpoint.breakConditionAfter != DWRITE_BREAK_CONDITION_MAY_NOT_BREAK)
        {
            bestBreakPosition = validBreakPosition;
            if (breakpoint.breakConditionAfter == DWRITE_BREAK_CONDITION_MUST_BREAK)
                break; // we have a hard return, so we've fit all we can.
        }
        cluster = nextCluster;
    }

    ////////////////////////////////////////
    // Want last best position that didn't break a word, but if that's not available,
    // fit at least one cluster (emergency line breaking).
    if (bestBreakPosition == clusterStart.textPosition)
        bestBreakPosition =  validBreakPosition;

    SetClusterPosition(cluster, bestBreakPosition);

    *clusterEnd = cluster;

    return S_OK;
}


STDMETHODIMP FlowLayout::ProduceGlyphRuns(
    FlowLayoutSink* flowSink,
    const FlowLayoutSource::RectF& rect,
    const ClusterPosition& clusterStart,
    const ClusterPosition& clusterEnd
    ) const throw()
{
    // Produce a series of glyph runs from the given range
    // and send them to the sink. If the entire text fit
    // into the rect, then we'll only pass on a single glyph
    // run.

    HRESULT hr = S_OK;

    ////////////////////////////////////////
    // Figure out how many runs we cross, because this is the number
    // of distinct glyph runs we'll need to reorder visually.

    UINT32 runIndexEnd = clusterEnd.runIndex;
    if (clusterEnd.textPosition > runs_[runIndexEnd].textStart)
        ++runIndexEnd; // Only partially cover the run, so round up.

    // Fill in mapping from visual run to logical sequential run.
    UINT32 bidiOrdering[100];
    UINT32 totalRuns = runIndexEnd - clusterStart.runIndex;
    totalRuns = std::min(totalRuns, static_cast<UINT32>(ARRAYSIZE(bidiOrdering)));

    ProduceBidiOrdering(
        clusterStart.runIndex,
        totalRuns,
        &bidiOrdering[0]
        );

    ////////////////////////////////////////
    // Ignore any trailing whitespace

    // Look backwards from end until we find non-space.
    UINT32 trailingWsPosition = clusterEnd.textPosition;
    for ( ; trailingWsPosition > clusterStart.textPosition; --trailingWsPosition)
    {
        if (!breakpoints_[trailingWsPosition-1].isWhitespace)
            break; // Encountered last significant character.
    }
    // Set the glyph run's ending cluster to the last whitespace.
    ClusterPosition clusterWsEnd(clusterStart);
    SetClusterPosition(clusterWsEnd, trailingWsPosition);


    ////////////////////////////////////////
    // Produce justified advances to reduce the jagged edge.

    std::vector<float> justifiedAdvances;
    hr = ProduceJustifiedAdvances(rect, clusterStart, clusterWsEnd, justifiedAdvances);
    UINT32 justificationGlyphStart = GetClusterGlyphStart(clusterStart);


    ////////////////////////////////////////
    // Determine starting point for alignment.

    float x = rect.left;
    float y = rect.bottom;

    if (SUCCEEDED(hr))
    {
        DWRITE_FONT_METRICS fontMetrics;
        fontFace_->GetMetrics(&fontMetrics);

        float descent   = (fontMetrics.descent * fontEmSize_ / fontMetrics.designUnitsPerEm);
        y -= descent;

        if (readingDirection_ == DWRITE_READING_DIRECTION_RIGHT_TO_LEFT)
        {
            // For RTL, we neeed the run width to adjust the origin
            // so it starts on the right side.
            UINT32 glyphStart = GetClusterGlyphStart(clusterStart);
            UINT32 glyphEnd   = GetClusterGlyphStart(clusterWsEnd);

            if (glyphStart < glyphEnd)
            {
                float lineWidth = GetClusterRangeWidth(
                    glyphStart - justificationGlyphStart,
                    glyphEnd   - justificationGlyphStart,
                    &justifiedAdvances.front()
                    );
                x = rect.right - lineWidth;
            }
        }
    }

    ////////////////////////////////////////
    // Send each glyph run to the sink.

    if (SUCCEEDED(hr))
    {
        for (size_t i = 0; i < totalRuns; ++i)
        {
            const TextAnalysis::Run& run    = runs_[bidiOrdering[i]];
            UINT32 glyphStart               = run.glyphStart;
            UINT32 glyphEnd                 = glyphStart + run.glyphCount;

            // If the run is only partially covered, we'll need to find
            // the subsection of glyphs that were fit.
            if (clusterStart.textPosition > run.textStart)
            {
                glyphStart = GetClusterGlyphStart(clusterStart);
            }
            if (clusterWsEnd.textPosition < run.textStart + run.textLength)
            {
                glyphEnd = GetClusterGlyphStart(clusterWsEnd);
            }
            if ((glyphStart >= glyphEnd)
            || (run.script.shapes & DWRITE_SCRIPT_SHAPES_NO_VISUAL))
            {
                // The itemizer told us not to draw this character,
                // either because it was a formatting, control, or other hidden character.
                continue;
            }

            // The run width is needed to know how far to move forward,
            // and to flip the origin for right-to-left text.
            float runWidth = GetClusterRangeWidth(
                                glyphStart - justificationGlyphStart,
                                glyphEnd   - justificationGlyphStart,
                                &justifiedAdvances.front()
                                );

            // Flush this glyph run.
            hr = flowSink->SetGlyphRun(
                (run.bidiLevel & 1) ? (x + runWidth) : (x), // origin starts from right if RTL
                y,
                glyphEnd - glyphStart,
                &glyphIndices_[glyphStart],
                &justifiedAdvances[glyphStart - justificationGlyphStart],
                &glyphOffsets_[glyphStart],
                fontFace_,
                fontEmSize_,
                run.bidiLevel,
                run.isSideways
                ); 
            if (FAILED(hr))
                break;

            x += runWidth;
        }
    }

    return hr;
}


void FlowLayout::ProduceBidiOrdering(
    UINT32 spanStart,
    UINT32 spanCount,
    OUT UINT32* spanIndices     // [spanCount]
    ) const throw()
{
    // Produces an index mapping from sequential order to visual bidi order.
    // The function progresses forward, checking the bidi level of each
    // pair of spans, reversing when needed.
    //
    // See the Unicode technical report 9 for an explanation.
    // http://www.unicode.org/reports/tr9/tr9-17.html 

    // Fill all entries with initial indices
    for (UINT32 i = 0; i < spanCount; ++i)
    {
        spanIndices[i] = spanStart + i;
    }

    if (spanCount <= 1)
        return;

    size_t runStart = 0;
    UINT32 currentLevel = runs_[spanStart].bidiLevel;

    // Rearrange each run to produced reordered spans.
    for (size_t i = 0; i < spanCount; ++i )
    {
        size_t runEnd       = i + 1;
        UINT32 nextLevel    = (runEnd < spanCount)
                            ? runs_[spanIndices[runEnd]].bidiLevel
                            : 0; // past last element

        // We only care about transitions, particularly high to low,
        // because that means we have a run behind us where we can
        // do something.

        if (currentLevel <= nextLevel) // This is now the beginning of the next run.
        {
            if (currentLevel < nextLevel)
            {
                currentLevel = nextLevel;
                runStart     = i + 1;
            }
            continue; // Skip past equal levels, or increasing stairsteps.
        }

        do // currentLevel > nextLevel
        {
            // Recede to find start of the run and previous level.
            UINT32 previousLevel;
            for (;;)
            {
                if (runStart <= 0) // reached front of index list
                {
                    previousLevel = 0; // position before string has bidi level of 0
                    break;
                }
                if (runs_[spanIndices[--runStart]].bidiLevel < currentLevel)
                {
                    previousLevel = runs_[spanIndices[runStart]].bidiLevel;
                    ++runStart; // compensate for going one element past
                    break;
                }
            }

            // Reverse the indices, if the difference between the current and
            // next/previous levels is odd. Otherwise, it would be a no-op, so
            // don't bother.
            if ((std::min(currentLevel - nextLevel, currentLevel - previousLevel) & 1) != 0)
            {
                std::reverse(spanIndices + runStart, spanIndices + runEnd);
            }

            // Descend to the next lower level, the greater of previous and next
            currentLevel = std::max(previousLevel, nextLevel);
        }
        while (currentLevel > nextLevel); // Continue until completely flattened.
    }
}


STDMETHODIMP FlowLayout::ProduceJustifiedAdvances(
    const FlowLayoutSource::RectF& rect,
    const ClusterPosition& clusterStart,
    const ClusterPosition& clusterEnd,
    OUT std::vector<float>& justifiedAdvances
    ) const throw()
{
    // Performs simple inter-word justification
    // using the breakpoint analysis whitespace property.

    // Copy out default, unjustified advances.
    UINT32 glyphStart   = GetClusterGlyphStart(clusterStart);
    UINT32 glyphEnd     = GetClusterGlyphStart(clusterEnd);

    try
    {
        justifiedAdvances.assign(glyphAdvances_.begin() + glyphStart, glyphAdvances_.begin() + glyphEnd);
    }
    catch (...)
    {
        return ExceptionToHResult();
    }

    if (glyphEnd - glyphStart == 0)
        return S_OK; // No glyphs to modify.

    float maxWidth = rect.right - rect.left;
    if (maxWidth <= 0)
        return S_OK; // Text can't fit anyway.


    ////////////////////////////////////////
    // First, count how many spaces there are in the text range.

    ClusterPosition cluster(clusterStart);
    UINT32 whitespaceCount = 0;

    while (cluster.textPosition < clusterEnd.textPosition)
    {
        if (breakpoints_[cluster.textPosition].isWhitespace)
            ++whitespaceCount;
        AdvanceClusterPosition(cluster);
    }
    if (whitespaceCount <= 0)
        return S_OK; // Can't justify using spaces, since none exist.


    ////////////////////////////////////////
    // Second, determine the needed contribution to each space.

    float lineWidth             = GetClusterRangeWidth(glyphStart, glyphEnd, &glyphAdvances_.front());
    float justificationPerSpace = (maxWidth - lineWidth) / whitespaceCount;

    if (justificationPerSpace  <= 0)
        return S_OK; // Either already justified or would be negative justification.

    if (justificationPerSpace > maxSpaceWidth_)
        return S_OK; // Avoid justification if it would space the line out awkwardly far.


    ////////////////////////////////////////
    // Lastly, adjust the advance widths, adding the difference to each space character.

    cluster = clusterStart;
    while (cluster.textPosition < clusterEnd.textPosition)
    {
        if (breakpoints_[cluster.textPosition].isWhitespace)
            justifiedAdvances[GetClusterGlyphStart(cluster) - glyphStart] += justificationPerSpace;

        AdvanceClusterPosition(cluster);
    }

    return S_OK;
}


////////////////////////////////////////////////////////////////////////////////
// Text/cluster navigation.
//
// Since layout should never split text clusters, we want to move ahead whole
// clusters at a time.

void FlowLayout::SetClusterPosition(
    IN OUT ClusterPosition& cluster,
    UINT32 textPosition
    ) const throw()
{
    // Updates the current position and seeks its matching text analysis run.

    cluster.textPosition = textPosition;

    // If the new text position is outside the previous analysis run,
    // find the right one.

    if (textPosition >= cluster.runEndPosition
    ||  !runs_[cluster.runIndex].ContainsTextPosition(textPosition))
    {
        // If we can resume the search from the previous run index,
        // (meaning the new text position comes after the previously
        // seeked one), we can save some time. Otherwise restart from
        // the beginning.

        UINT32 newRunIndex = 0;
        if (textPosition >= runs_[cluster.runIndex].textStart)
        {
            newRunIndex = cluster.runIndex;
        }

        // Find new run that contains the text position.
        newRunIndex = static_cast<UINT32>(
                            std::find(runs_.begin() + newRunIndex, runs_.end(), textPosition)
                            - runs_.begin()
                            );

        // Keep run index within the list, rather than pointing off the end.
        if (newRunIndex >= runs_.size())
        {
            newRunIndex  = static_cast<UINT32>(runs_.size() - 1);
        }

        // Cache the position of the next analysis run to efficiently
        // move forward in the clustermap.
        const TextAnalysis::Run& matchingRun    = runs_[newRunIndex];
        cluster.runIndex                        = newRunIndex;
        cluster.runEndPosition                  = matchingRun.textStart + matchingRun.textLength;
    }
}


void FlowLayout::AdvanceClusterPosition(
    IN OUT ClusterPosition& cluster
    ) const throw()
{
    // Looks forward in the cluster map until finding a new cluster,
    // or until we reach the end of a cluster run returned by shaping.
    //
    // Glyph shaping can produce a clustermap where a:
    //  - A single codepoint maps to a single glyph (simple Latin and precomposed CJK)
    //  - A single codepoint to several glyphs (diacritics decomposed into distinct glyphs)
    //  - Multiple codepoints are coalesced into a single glyph.
    //
    UINT32 textPosition = cluster.textPosition;
    UINT32 clusterId    = glyphClusters_[textPosition];

    for (++textPosition; textPosition < cluster.runEndPosition; ++textPosition)
    {
        if (glyphClusters_[textPosition] != clusterId)
        {
            // Now pointing to the next cluster.
            cluster.textPosition = textPosition;
            return;
        }
    }
    if (textPosition == cluster.runEndPosition)
    {
        // We crossed a text analysis run.
        SetClusterPosition(cluster, textPosition);
    }
}


UINT32 FlowLayout::GetClusterGlyphStart(const ClusterPosition& cluster) const throw()
{
    // Maps from text position to corresponding starting index in the glyph array.
    // This is needed because there isn't a 1:1 correspondence between text and
    // glyphs produced.

    UINT32 glyphStart = runs_[cluster.runIndex].glyphStart;

    return (cluster.textPosition < glyphClusters_.size())
        ? glyphStart + glyphClusters_[cluster.textPosition]
        : glyphStart + runs_[cluster.runIndex].glyphCount;
}


float FlowLayout::GetClusterRangeWidth(
    const ClusterPosition& clusterStart,
    const ClusterPosition& clusterEnd
    ) const throw()
{
    // Sums the glyph advances between two cluster positions,
    // useful for determining how long a line or word is.
    return GetClusterRangeWidth(
                GetClusterGlyphStart(clusterStart),
                GetClusterGlyphStart(clusterEnd),
                &glyphAdvances_.front()
                );
}


float FlowLayout::GetClusterRangeWidth(
    UINT32 glyphStart,
    UINT32 glyphEnd,
    const float* glyphAdvances          // [glyphEnd]
    ) const throw()
{
    // Sums the glyph advances between two glyph offsets, given an explicit
    // advances array - useful for determining how long a line or word is.
    return std::accumulate(glyphAdvances + glyphStart, glyphAdvances + glyphEnd, 0.0f);
}
