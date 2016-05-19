// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// Contents:    Implementation of text analyzer source and sink.
//
//----------------------------------------------------------------------------
#include "common.h"
#include "TextAnalysis.h"


TextAnalysis::TextAnalysis(
    const wchar_t* text,
    UINT32 textLength,
    const wchar_t* localeName,
    IDWriteNumberSubstitution* numberSubstitution,
    DWRITE_READING_DIRECTION readingDirection
    )
:   text_(text),
    textLength_(textLength),
    localeName_(localeName),
    readingDirection_(readingDirection),
    numberSubstitution_(numberSubstitution),
    currentPosition_(0),
    currentRunIndex_(0)
{
}


STDMETHODIMP TextAnalysis::GenerateResults(
    IDWriteTextAnalyzer* textAnalyzer,
    OUT std::vector<TextAnalysis::Run>& runs,
    OUT std::vector<DWRITE_LINE_BREAKPOINT>& breakpoints
    )
{
    // Analyzes the text using each of the analyzers and returns
    // their results as a series of runs.

    HRESULT hr = S_OK;

    try
    {
        // Initially start out with one result that covers the entire range.
        // This result will be subdivided by the analysis processes.
        runs_.resize(1);
        LinkedRun& initialRun   = runs_[0];
        initialRun.nextRunIndex = 0;
        initialRun.textStart    = 0;
        initialRun.textLength   = textLength_;
        initialRun.bidiLevel    = (readingDirection_ == DWRITE_READING_DIRECTION_RIGHT_TO_LEFT);

        // Allocate enough room to have one breakpoint per code unit.
        breakpoints_.resize(textLength_);

        // Call each of the analyzers in sequence, recording their results.
        if (SUCCEEDED(hr = textAnalyzer->AnalyzeLineBreakpoints(   this, 0, textLength_, this))
        &&  SUCCEEDED(hr = textAnalyzer->AnalyzeBidi(              this, 0, textLength_, this))
        &&  SUCCEEDED(hr = textAnalyzer->AnalyzeScript(            this, 0, textLength_, this))
        &&  SUCCEEDED(hr = textAnalyzer->AnalyzeNumberSubstitution(this, 0, textLength_, this))
        )
        {
            // Exchange our results with the caller's.
            breakpoints.swap(breakpoints_);

            // Resequence the resulting runs in order before returning to caller.
            size_t totalRuns = runs_.size();
            runs.resize(totalRuns);

            UINT32 nextRunIndex = 0;
            for (size_t i = 0; i < totalRuns; ++i)
            {
                runs[i]         = runs_[nextRunIndex];
                nextRunIndex    = runs_[nextRunIndex].nextRunIndex;
            }
        }
    }
    catch (...)
    {
        return ExceptionToHResult();
    }

    return hr;
}


////////////////////////////////////////////////////////////////////////////////
// IDWriteTextAnalysisSource source implementation

IFACEMETHODIMP TextAnalysis::GetTextAtPosition(
    UINT32 textPosition,
    OUT WCHAR const** textString,
    OUT UINT32* textLength
    ) throw()
{
    if (textPosition >= textLength_)
    {
        // Return no text if a query comes for a position at the end of
        // the range. Note that is not an error and we should not return
        // a failing HRESULT. Although the application is not expected
        // to return any text that is outside of the given range, the
        // analyzers may probe the ends to see if text exists.
        *textString = NULL;
        *textLength = 0;
    }
    else
    {
        *textString = &text_[textPosition];
        *textLength = textLength_ - textPosition;
    }
    return S_OK;
}


IFACEMETHODIMP TextAnalysis::GetTextBeforePosition(
    UINT32 textPosition,
    OUT WCHAR const** textString,
    OUT UINT32* textLength
    ) throw()
{
    if (textPosition == 0 || textPosition > textLength_)
    {
        // Return no text if a query comes for a position at the end of
        // the range. Note that is not an error and we should not return
        // a failing HRESULT. Although the application is not expected
        // to return any text that is outside of the given range, the
        // analyzers may probe the ends to see if text exists.
        *textString = NULL;
        *textLength = 0;
    }
    else
    {
        *textString = &text_[0];
        *textLength = textPosition - 0; // text length is valid from current position backward
    }
    return S_OK;
}


DWRITE_READING_DIRECTION STDMETHODCALLTYPE TextAnalysis::GetParagraphReadingDirection() throw()
{
    return readingDirection_;
}


IFACEMETHODIMP TextAnalysis::GetLocaleName(
    UINT32 textPosition,
    OUT UINT32* textLength,
    OUT WCHAR const** localeName
    ) throw()
{
    // The pointer returned should remain valid until the next call,
    // or until analysis ends. Since only one locale name is supported,
    // the text length is valid from the current position forward to
    // the end of the string.

    *localeName = localeName_;
    *textLength = textLength_ - textPosition;

    return S_OK;
}


IFACEMETHODIMP TextAnalysis::GetNumberSubstitution(
    UINT32 textPosition,
    OUT UINT32* textLength,
    OUT IDWriteNumberSubstitution** numberSubstitution
    ) throw()
{
    if (numberSubstitution_ != NULL)
        numberSubstitution_->AddRef();

    *numberSubstitution = numberSubstitution_;
    *textLength = textLength_ - textPosition;

    return S_OK;
}


////////////////////////////////////////////////////////////////////////////////
// IDWriteTextAnalysisSink implementation

IFACEMETHODIMP TextAnalysis::SetLineBreakpoints(
    UINT32 textPosition,
    UINT32 textLength,
    DWRITE_LINE_BREAKPOINT const* lineBreakpoints   // [textLength]
    ) throw()
{
    if (textLength > 0)
    {
        memcpy(&breakpoints_[textPosition], lineBreakpoints, textLength * sizeof(lineBreakpoints[0]));
    }
    return S_OK;
}


IFACEMETHODIMP TextAnalysis::SetScriptAnalysis(
    UINT32 textPosition,
    UINT32 textLength,
    DWRITE_SCRIPT_ANALYSIS const* scriptAnalysis
    ) throw()
{
    try
    {
        SetCurrentRun(textPosition);
        SplitCurrentRun(textPosition);
        while (textLength > 0)
        {
            LinkedRun& run  = FetchNextRun(&textLength);
            run.script      = *scriptAnalysis;
        }
    }
    catch (...)
    {
        return E_FAIL; // Unknown error, probably out of memory.
    }

    return S_OK;
}


IFACEMETHODIMP TextAnalysis::SetBidiLevel(
    UINT32 textPosition,
    UINT32 textLength,
    UINT8 explicitLevel,
    UINT8 resolvedLevel
    ) throw()
{
    try
    {
        SetCurrentRun(textPosition);
        SplitCurrentRun(textPosition);
        while (textLength > 0)
        {
            LinkedRun& run  = FetchNextRun(&textLength);
            run.bidiLevel   = resolvedLevel;
        }
    }
    catch (...)
    {
        return E_FAIL; // Unknown error, probably out of memory.
    }

    return S_OK;
}


IFACEMETHODIMP TextAnalysis::SetNumberSubstitution(
    UINT32 textPosition,
    UINT32 textLength,
    IDWriteNumberSubstitution* numberSubstitution
    ) throw()
{
    try
    {
        SetCurrentRun(textPosition);
        SplitCurrentRun(textPosition);
        while (textLength > 0)
        {
            LinkedRun& run          = FetchNextRun(&textLength);
            run.isNumberSubstituted = (numberSubstitution != NULL);
        }
    }
    catch (...)
    {
        return E_FAIL; // Unknown error, probably out of memory.
    }

    return S_OK;
}


////////////////////////////////////////////////////////////////////////////////
// Run modification.

TextAnalysis::LinkedRun& TextAnalysis::FetchNextRun(
    IN OUT UINT32* textLength
    )
{
    // Used by the sink setters, this returns a reference to the next run.
    // Position and length are adjusted to now point after the current run
    // being returned.

    UINT32 runIndex      = currentRunIndex_;
    UINT32 runTextLength = runs_[currentRunIndex_].textLength;

    // Split the tail if needed (the length remaining is less than the
    // current run's size).
    if (*textLength < runTextLength)
    {
        runTextLength       = *textLength; // Limit to what's actually left.
        UINT32 runTextStart = runs_[currentRunIndex_].textStart;

        SplitCurrentRun(runTextStart + runTextLength);
    }
    else
    {
        // Just advance the current run.
        currentRunIndex_ = runs_[currentRunIndex_].nextRunIndex;
    }
    *textLength -= runTextLength;


    // Return a reference to the run that was just current.
    return runs_[runIndex];
}


void TextAnalysis::SetCurrentRun(UINT32 textPosition)
{
    // Move the current run to the given position.
    // Since the analyzers generally return results in a forward manner,
    // this will usually just return early. If not, find the
    // corresponding run for the text position.

    if (currentRunIndex_ < runs_.size()
    &&  runs_[currentRunIndex_].ContainsTextPosition(textPosition))
    {
        return;
    }

    currentRunIndex_ = static_cast<UINT32>(
                            std::find(runs_.begin(), runs_.end(), textPosition)
                            - runs_.begin()
                            );
}


void TextAnalysis::SplitCurrentRun(UINT32 splitPosition)
{
    // Splits the current run and adjusts the run values accordingly.

    UINT32 runTextStart = runs_[currentRunIndex_].textStart;

    if (splitPosition <= runTextStart)
        return; // no change

    // Grow runs by one.
    size_t totalRuns = runs_.size();
    try
    {
        runs_.resize(totalRuns + 1);
    }
    catch (...)
    {
        return; // Can't increase size. Return same run.
    }

    // Copy the old run to the end.
    LinkedRun& frontHalf = runs_[currentRunIndex_];
    LinkedRun& backHalf  = runs_.back();
    backHalf             = frontHalf;

    // Adjust runs' text positions and lengths.
    UINT32 splitPoint       = splitPosition - runTextStart;
    backHalf.textStart     += splitPoint;
    backHalf.textLength    -= splitPoint;
    frontHalf.textLength    = splitPoint;
    frontHalf.nextRunIndex  = static_cast<UINT32>(totalRuns);
    currentRunIndex_        = static_cast<UINT32>(totalRuns);
}
