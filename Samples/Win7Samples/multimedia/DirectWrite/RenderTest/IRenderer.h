// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//----------------------------------------------------------------------------
#pragma once

struct MagnifierInfo;

class IRenderer
{
public:
    virtual ~IRenderer() throw()
    {
    }

    virtual void SetFormat(IDWriteTextFormat* textFormat) = 0;
    virtual void SetText(wchar_t const* text) = 0;
    virtual void SetWindowSize(UINT width, UINT height) = 0;
    virtual void SetMonitor(HMONITOR monitor) = 0;
    virtual void SetMeasuringMode(DWRITE_MEASURING_MODE measuringMode) = 0;
    virtual void SetTransform(DWRITE_MATRIX const& transform) = 0;
    virtual void SetMagnifier(MagnifierInfo const& magnifier) = 0;

    virtual HRESULT Draw(HDC hdc) = 0;
};

struct MagnifierInfo
{
    enum Type
    {
        Vector,
        Pixel,
        Subpixel
    };

    bool visible;
    Type type;
    int scale;

    // Note: all integer coordinates are in pixels rather than DIPs.
    POINT focusPos;
    POINT magnifierPos;
    SIZE  magnifierSize;
};

IRenderer* CreateD2DRenderer(
    HWND hwnd,
    UINT width,
    UINT height,
    IDWriteTextFormat* textFormat,
    wchar_t const* text
    );

IRenderer* CreateDWriteRenderer(
    HWND hwnd,
    UINT width,
    UINT height,
    IDWriteTextFormat* textFormat,
    wchar_t const* text
    );
