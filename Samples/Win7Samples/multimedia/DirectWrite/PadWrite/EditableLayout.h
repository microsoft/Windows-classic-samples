// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// Contents:    Extended TextLayout that permits editing.
//
// Remarks:     Internally, a new DirectWrite layout is recreated when the
//              text is edited, but the caller can safely hold the same
//              reference, since the adapter forwards all the calls onward.
//
//----------------------------------------------------------------------------
#pragma once


////////////////////////////////////////
// Helper to construct text ranges when calling setters.
//
// Example: textLayout_->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD, MakeDWriteTextRange(20, 10));

struct MakeDWriteTextRange : public DWRITE_TEXT_RANGE
{
    inline MakeDWriteTextRange(UINT32 startPosition, UINT32 length)
    {
        this->startPosition = startPosition;
        this->length = length;
    }

    // Overload to extend to end of text.
    inline MakeDWriteTextRange(UINT32 startPosition)
    {
        this->startPosition = startPosition;
        this->length = UINT32_MAX - startPosition;
    }
};


////////////////////////////////////////
// Layouts were optimized for mostly static UI text, so a layout's text is
// immutable upon creation. This means that when we modify the text, we must
// create a new layout, copying the old properties over to the new one. This
// class assists with that.

class EditableLayout
{
public:
    struct CaretFormat
    {
        // The important range based properties for the current caret.
        // Note these are stored outside the layout, since the current caret
        // actually has a format, independent of the text it lies between.
        wchar_t fontFamilyName[100];
        wchar_t localeName[LOCALE_NAME_MAX_LENGTH];
        FLOAT fontSize;
        DWRITE_FONT_WEIGHT fontWeight;
        DWRITE_FONT_STRETCH fontStretch;
        DWRITE_FONT_STYLE fontStyle;
        UINT32 color;
        BOOL hasUnderline;
        BOOL hasStrikethrough;
    };

public:
    EditableLayout(IDWriteFactory* factory)
        :   factory_(SafeAcquire(factory))
    {
    }

    ~EditableLayout()
    {
        SafeRelease(&factory_);
    }

public:
    IDWriteFactory* GetFactory() { return factory_; }

    /// Inserts a given string in the text layout's stored string at a certain text postion;
    HRESULT STDMETHODCALLTYPE InsertTextAt(
        IN OUT IDWriteTextLayout*& currentLayout,
        IN OUT std::wstring& text,
        UINT32 position,
        WCHAR const* textToInsert,  // [lengthToInsert]
        UINT32 textToInsertLength,
        CaretFormat* caretFormat = NULL
        );

    /// Deletes a specified amount characters from the layout's stored string.
    HRESULT STDMETHODCALLTYPE RemoveTextAt(
        IN OUT IDWriteTextLayout*& currentLayout,
        IN OUT std::wstring& text,
        UINT32 position,
        UINT32 lengthToRemove
        );

    HRESULT STDMETHODCALLTYPE Clear(
        IN OUT IDWriteTextLayout*& currentLayout,
        IN OUT std::wstring& text
        );

private:
    HRESULT STDMETHODCALLTYPE RecreateLayout(
        IN OUT IDWriteTextLayout*& currentLayout,
        const std::wstring& text
        );

    static void CopyGlobalProperties(
        IDWriteTextLayout* oldLayout,
        IDWriteTextLayout* newLayout
        );

    static void CopyRangedProperties(
        IDWriteTextLayout* oldLayout,
        UINT32 startPos,
        UINT32 endPos,
        UINT32 newLayoutTextOffset,
        IDWriteTextLayout* newLayout,
        bool isOffsetNegative = false
        );

    static void CopySinglePropertyRange(
        IDWriteTextLayout* oldLayout,
        UINT32 startPosForOld,
        IDWriteTextLayout* newLayout,
        UINT32 startPosForNew,
        UINT32 length,
        EditableLayout::CaretFormat* caretFormat = NULL
        );

public:
    IDWriteFactory* factory_;
};
