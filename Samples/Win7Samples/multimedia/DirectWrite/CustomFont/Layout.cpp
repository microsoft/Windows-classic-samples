// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//----------------------------------------------------------------------------

#include "Common.h"
#include "CustomFont.h"
#include "Layout.h"
#include "resource.h"

float const Layout::leftMargin_     = 0.25f * 96;
float const Layout::rightMargin_    = 0.25f * 96;
float const Layout::minColumnWidth_ = 3.0f  * 96;

struct Layout::Format
{
    float pointSize;
    float spaceBefore;
    wchar_t const* familyName;
};

Layout::Format const Layout::formats_[] =
{
    { 20.0f, 12.0f, L"Pericles" },
    { 11.0f, 11.0f, L"Kootenay" }
};

UINT const Layout::paragraphs_[] =
{
    IDS_HEADING,
    IDS_BODY1,
    IDS_BODY2,
    IDS_BODY3
};

HRESULT Layout::Draw(float pageWidthInDips, ID2D1RenderTarget* renderTarget, ID2D1Brush* textBrush)
{
    HRESULT hr = S_OK;

    static UINT const fontResourceIDs[] = {  IDR_FONT_PERICLES, IDR_FONT_KOOTENAY };

    // Create a custom font collection comprising our two font resources. We could have done this
    // in the constructor rather than every time. However, if you set break points on the loader 
    // callbacks you'll find they're only called the first time the font collection is created. 
    // Thereafter the font collection data is cached so recreating it is quite fast.
    hr = fontContext_.Initialize();
    if (FAILED(hr))
        return hr;

    IDWriteFontCollection* fontCollection = NULL;
    hr = fontContext_.CreateFontCollection(
            fontResourceIDs,
            sizeof(fontResourceIDs),
            &fontCollection
            );
    if (FAILED(hr))
        return hr;

    // Set up for first paragraph.
    float const columnWidth = std::max<float>(minColumnWidth_, pageWidthInDips - leftMargin_ - rightMargin_);
    float y = 0;

    float spaceBefore = 0;
    IDWriteTextFormat* textFormat = NULL;

    size_t const formatCount = sizeof(formats_) / sizeof(formats_[0]);
    size_t const paragraphCount = sizeof(paragraphs_) / sizeof(paragraphs_[0]);

    // Iterate over all the paragraphs.
    for (size_t i = 0; i < paragraphCount; ++i)
    {
        // We create a different text format object for the first formatCount
        // paragraphs. After that we reuse the last text format object.
        if (i < formatCount)
        {
            // Create the text format object, specifying both the family name and the
            // custom font collection in which to look for the family name. DirectWrite
            // will only look for the family name in the specified collection so there
            // is no ambiguity even if the system font collection happens to have a font
            // with the same family name.
            SafeRelease(&textFormat);
            hr = g_dwriteFactory->CreateTextFormat(
                    formats_[i].familyName,
                    fontCollection,
                    DWRITE_FONT_WEIGHT_NORMAL,
                    DWRITE_FONT_STYLE_NORMAL,
                    DWRITE_FONT_STRETCH_NORMAL,
                    formats_[i].pointSize * (96.0f / 72),
                    L"en-us",
                    &textFormat
                    );

            spaceBefore = formats_[i].spaceBefore * (96.0f / 72);
        }

        IDWriteTextLayout* textLayout = NULL;
        if (SUCCEEDED(hr))
        {
            // Load the string.
            int const maxLength = 512;
            wchar_t charBuffer[maxLength];
            int stringLength = LoadString(g_instance, paragraphs_[i], charBuffer, maxLength);

            // Create the text layout object.
            hr = g_dwriteFactory->CreateTextLayout(
                    charBuffer,
                    stringLength,
                    textFormat,
                    columnWidth,
                    0,
                    &textLayout
                    );
        }

        if (SUCCEEDED(hr))
        {
            // Draw the text and update the y coordinate.
            y += spaceBefore;

            renderTarget->DrawTextLayout(
                D2D1::Point2F(leftMargin_, y),
                textLayout,
                textBrush
                );

            DWRITE_TEXT_METRICS metrics;
            hr = textLayout->GetMetrics(&metrics);
            y += metrics.height;
        }
        SafeRelease(&textLayout);

        if (FAILED(hr))
            break;
    }

    SafeRelease(&textFormat);
    SafeRelease(&fontCollection);

    return hr;
}
