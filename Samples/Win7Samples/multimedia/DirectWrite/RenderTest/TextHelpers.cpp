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

//
// CreateTextFormatFromLOGFONT
//
//      Helper function that creates a DWrite text format object from
//      the specified LOGFONT.
//
//      We use LOGFONT in this application simply because we rely on the
//      Win32 font common dialog. If we were implementing our own font
//      selection user interface, a more natural approach would be to use
//      the DWrite font collection API and not use LOGFONT at all.
//
HRESULT CreateTextFormatFromLOGFONT(
    LOGFONT const& logFont,
    float fontSize,
    OUT IDWriteTextFormat** textFormat
    )
{
    HRESULT hr = S_OK;

    IDWriteGdiInterop*       gdiInterop = NULL;
    IDWriteFont*             font = NULL;
    IDWriteFontFamily*       fontFamily = NULL;
    IDWriteLocalizedStrings* localizedFamilyNames = NULL;

    // Conversion to and from LOGFONT uses the IDWriteGdiInterop interface.
    if (SUCCEEDED(hr))
    {
        hr = g_dwriteFactory->GetGdiInterop(&gdiInterop);
    }

    // Find the font object that best matches the specified LOGFONT.
    if (SUCCEEDED(hr))
    {
        hr = gdiInterop->CreateFontFromLOGFONT(&logFont, &font);
    }

    // Get the font family to which this font belongs.
    if (SUCCEEDED(hr))
    {
        hr = font->GetFontFamily(&fontFamily);
    }

    // Get the family names. This returns an object that encapsulates one or
    // more names with the same meaning but in different languages.
    if (SUCCEEDED(hr))
    {
        hr = fontFamily->GetFamilyNames(&localizedFamilyNames);
    }

    // Get the family name at index zero. If we were going to display the name
    // we'd want to try to find one that matched the use locale, but for purposes
    // of creating a text format object any language will do.

    wchar_t familyName[100];
    if (SUCCEEDED(hr))
    {
        hr = localizedFamilyNames->GetString(0, familyName, ARRAYSIZE(familyName));
    }

    if (SUCCEEDED(hr))
    {
        // If no font size was passed in use the lfHeight of the LOGFONT.
        if (fontSize == 0)
        {
            // Convert from pixels to DIPs.
            fontSize = PixelsToDipsY(logFont.lfHeight);
            if (fontSize < 0)
            {
                // Negative lfHeight represents the size of the em unit.
                fontSize = -fontSize;
            }
            else
            {
                // Positive lfHeight represents the cell height (ascent + descent).
                DWRITE_FONT_METRICS fontMetrics;
                font->GetMetrics(&fontMetrics);

                // Convert the cell height (ascent + descent) from design units to ems.
                float cellHeight = static_cast<float>(fontMetrics.ascent + fontMetrics.descent) / 
                    fontMetrics.designUnitsPerEm;

                // Divide the font size by the cell height to get the font em size.
                fontSize /= cellHeight;
            }
        }
    }

    // The text format includes a locale name. Ideally, this would be the language
    // of the text, which may or may not be the same as the primary language of the
    // user. However, for our purposes the user locale will do.
    wchar_t localeName[LOCALE_NAME_MAX_LENGTH];
    if (SUCCEEDED(hr))
    {
        if (GetUserDefaultLocaleName(localeName, LOCALE_NAME_MAX_LENGTH) == 0)
            hr = HRESULT_FROM_WIN32(GetLastError());
    }

    if (SUCCEEDED(hr))
    {
        // Create the text format object.
        hr = g_dwriteFactory->CreateTextFormat(
            familyName,
            NULL, // no custom font collection
            font->GetWeight(),
            font->GetStyle(),
            font->GetStretch(),
            fontSize,
            localeName,
            textFormat
            );
    }

    SafeRelease(&localizedFamilyNames);
    SafeRelease(&fontFamily);
    SafeRelease(&font);
    SafeRelease(&gdiInterop);

    return hr;
}


DWRITE_MATRIX MakeRotateTransform(
    float angle,    // angle in degrees
    float x,        // x coordinate of the center of rotation
    float y         // y coordinate of the center of rotation
    )
{
    DWRITE_MATRIX matrix;

    // Convert degrees to radians.
    float radians = (angle * 3.141593f * 2) / 360;

    // Initialize the first four members, which comprise a 2x2 matrix.
    float sinA = sinf(radians);
    float cosA = cosf(radians);

    // If the angle is axis aligned, we'll make it a clean matrix.
    if (fmod(angle, 90.0f) == 0)
    {
        sinA = floorf(sinA + .5f);
        cosA = floorf(cosA + .5f);
    }

    matrix.m11 = cosA;
    matrix.m12 = sinA;
    matrix.m21 = -sinA;
    matrix.m22 = cosA;

    // Transform the given point.
    float xT = (x * matrix.m11) + (y * matrix.m21);
    float yT = (x * matrix.m12) + (y * matrix.m22);

    // Initialize the displacement to compensate such that the given
    // point will be the center of rotation.
    matrix.dx = x - xT;
    matrix.dy = y - yT;

    return matrix;
}

//
// Conversions between pixels and DIPs.
//
float PixelsToDipsX(int x)
{
    return x * 96.0f / g_dpiX;
}

float PixelsToDipsY(int y)
{
    return y * 96.0f / g_dpiY;
}

int DipsToPixelsX(float x)
{
    float pixels = x * g_dpiX * (1/96.0f);
    return static_cast<int>(floorf(pixels + 0.5f));
}

int DipsToPixelsY(float y)
{
    float pixels = y * g_dpiY * (1/96.0f);
    return static_cast<int>(floorf(pixels + 0.5f));
}
