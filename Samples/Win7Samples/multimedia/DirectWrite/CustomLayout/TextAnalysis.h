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
#pragma once


// Helper source/sink class for text analysis.
class TextAnalysis
    :   public ComBase<
            QiList<IDWriteTextAnalysisSource,
            QiList<IDWriteTextAnalysisSink,
            QiList<IUnknown
        > > > >
{
public:
    // A single contiguous run of characters containing the same analysis results.
    struct Run
    {
        Run() throw()
        :   textStart(),
            textLength(),
            glyphStart(),
            glyphCount(),
            bidiLevel(),
            script(),
            isNumberSubstituted(),
            isSideways()
        { }

        UINT32 textStart;   // starting text position of this run
        UINT32 textLength;  // number of contiguous code units covered
        UINT32 glyphStart;  // starting glyph in the glyphs array
        UINT32 glyphCount;  // number of glyphs associated with this run of text
        DWRITE_SCRIPT_ANALYSIS script;
        UINT8 bidiLevel;
        bool isNumberSubstituted;
        bool isSideways;

        inline bool ContainsTextPosition(UINT32 desiredTextPosition) const throw()
        {
            return desiredTextPosition >= textStart
                && desiredTextPosition <  textStart + textLength;
        }

        inline bool operator==(UINT32 desiredTextPosition) const throw()
        {
            // Search by text position using std::find
            return ContainsTextPosition(desiredTextPosition);
        }
    };

    // Single text analysis run, which points to the next run.
    struct LinkedRun : Run
    {
        LinkedRun() throw()
        :   nextRunIndex(0)
        { }

        UINT32 nextRunIndex;  // index of next run
    };

public:
    TextAnalysis(
        const wchar_t* text,
        UINT32 textLength,
        const wchar_t* localeName,
        IDWriteNumberSubstitution* numberSubstitution,
        DWRITE_READING_DIRECTION readingDirection
        );

    STDMETHODIMP GenerateResults(
        IDWriteTextAnalyzer* textAnalyzer,
        OUT std::vector<Run>& runs,
        OUT std::vector<DWRITE_LINE_BREAKPOINT>& breakpoints_
        ) throw();

    // IDWriteTextAnalysisSource implementation

    IFACEMETHODIMP GetTextAtPosition(
        UINT32 textPosition,
        OUT WCHAR const** textString,
        OUT UINT32* textLength
        ) throw();

    IFACEMETHODIMP GetTextBeforePosition(
        UINT32 textPosition,
        OUT WCHAR const** textString,
        OUT UINT32* textLength
        ) throw();

    IFACEMETHODIMP_(DWRITE_READING_DIRECTION) GetParagraphReadingDirection() throw();

    IFACEMETHODIMP GetLocaleName(
        UINT32 textPosition,
        OUT UINT32* textLength,
        OUT WCHAR const** localeName
        ) throw();

    IFACEMETHODIMP GetNumberSubstitution(
        UINT32 textPosition,
        OUT UINT32* textLength,
        OUT IDWriteNumberSubstitution** numberSubstitution
        ) throw();

    // IDWriteTextAnalysisSink implementation

    IFACEMETHODIMP SetScriptAnalysis(
        UINT32 textPosition,
        UINT32 textLength,
        DWRITE_SCRIPT_ANALYSIS const* scriptAnalysis
        ) throw();

    IFACEMETHODIMP SetLineBreakpoints(
        UINT32 textPosition,
        UINT32 textLength,
        const DWRITE_LINE_BREAKPOINT* lineBreakpoints // [textLength]
        ) throw();

    IFACEMETHODIMP SetBidiLevel(
        UINT32 textPosition,
        UINT32 textLength,
        UINT8 explicitLevel,
        UINT8 resolvedLevel
        ) throw();

    IFACEMETHODIMP SetNumberSubstitution(
        UINT32 textPosition,
        UINT32 textLength,
        IDWriteNumberSubstitution* numberSubstitution
        ) throw();

protected:
    LinkedRun& FetchNextRun(IN OUT UINT32* textLength);

    void SetCurrentRun(UINT32 textPosition);

    void SplitCurrentRun(UINT32 splitPosition);

protected:
    // Input
    // (weak references are fine here, since this class is a transient
    //  stack-based helper that doesn't need to copy data)
    UINT32 textLength_;
    const wchar_t* text_; // [textLength_]
    const wchar_t* localeName_;
    IDWriteNumberSubstitution* numberSubstitution_;
    DWRITE_READING_DIRECTION readingDirection_;

    // Current processing state
    UINT32 currentPosition_;
    UINT32 currentRunIndex_;

    // Output
    std::vector<LinkedRun> runs_;
    std::vector<DWRITE_LINE_BREAKPOINT> breakpoints_;
};
