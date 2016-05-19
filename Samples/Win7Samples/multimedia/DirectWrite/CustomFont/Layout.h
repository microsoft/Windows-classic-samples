// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//----------------------------------------------------------------------------

#pragma once
#include "ResourceFontContext.h"

class Layout
{
public:
    HRESULT Draw(float pageWidthInDips, ID2D1RenderTarget* renderTarget, ID2D1Brush* textBrush);

private:
    static float const leftMargin_;
    static float const rightMargin_;
    static float const minColumnWidth_;

    struct Format;
    static Format const formats_[];
    static UINT const paragraphs_[];

    ResourceFontContext fontContext_;
};
