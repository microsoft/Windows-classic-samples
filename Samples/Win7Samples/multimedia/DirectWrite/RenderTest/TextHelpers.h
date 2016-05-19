// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//----------------------------------------------------------------------------
#pragma once

//
// CreateTextFormatFromLOGFONT
//
//      Helper function that creates a DWrite text format object from
//      the specified LOGFONT.
//
HRESULT CreateTextFormatFromLOGFONT(
    LOGFONT const& logFont,
    float fontSize,
    OUT IDWriteTextFormat** textFormat
    );

//
// MakeRotateTransform
//
//      Given an angle and the center of rotatation, returns a matrix
//      to be used as a world transform.
//
DWRITE_MATRIX MakeRotateTransform(
    float angle,    // angle in degrees
    float x,        // x coordinate of the center of rotation
    float y         // y coordinate of the center of rotation
    );

//
// Conversions between pixels and DIPs.
//
//      Note: In this sample program, floating point coordinates are
//      always in DIPs and integer coordinates are always in pixels.
//
float PixelsToDipsX(int x);
float PixelsToDipsY(int y);
int DipsToPixelsX(float x);
int DipsToPixelsY(float y);

