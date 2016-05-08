// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// Contents:    Drawing effect that holds a single color.
//
//----------------------------------------------------------------------------
#pragma once


class DECLSPEC_UUID("1CD7C44F-526B-492a-B780-EF9C4159B653") DrawingEffect
:   public ComBase<QiList<IUnknown> >
{
public:
    DrawingEffect(UINT32 color)
    :   color_(color)
    { }

    inline UINT32 GetColor() const throw()
    {
        // Returns the BGRA value for D2D.
        return color_;
    }

    inline COLORREF GetColorRef() const throw()
    {
        // Returns color as COLORREF.
        return GetColorRef(color_);
    }

    static inline COLORREF GetColorRef(UINT32 bgra) throw()
    {
        // Swaps color order (bgra <-> rgba) from D2D/GDI+'s to a COLORREF.
        // This also leaves the top byte 0, since alpha is ignored anyway.
        return RGB(GetBValue(bgra), GetGValue(bgra), GetRValue(bgra));
    }

    static inline COLORREF GetBgra(COLORREF rgb) throw()
    {
        // Swaps color order (bgra <-> rgba) from COLORREF to D2D/GDI+'s.
        // Sets alpha to full opacity.
        return RGB(GetBValue(rgb), GetGValue(rgb), GetRValue(rgb)) | 0xFF000000;
    }

protected:
    // The color is stored as BGRA, with blue in the lowest byte,
    // then green, red, alpha; which is what D2D, GDI+, and GDI DIBs use.
    // GDI's COLORREF stores red as the lowest byte.
    UINT32 color_;
};
