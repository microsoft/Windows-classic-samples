// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// Contents:    Inline image for text layouts.
//
//----------------------------------------------------------------------------
#pragma once


struct IWICBitmapSource;


class DECLSPEC_UUID("1DE84D4E-1AD2-40ec-82B3-1B5B93471C65") InlineImage
    :   public ComBase<
            QiListSelf<InlineImage,
            QiList<IDWriteInlineObject
        > > >
{
public:
    InlineImage(
        IWICBitmapSource* image,
        unsigned int index = ~0
        );

    ~InlineImage()
    {
        SafeRelease(&image_);
    }

    IFACEMETHOD(Draw)(
        void* clientDrawingContext,
        IDWriteTextRenderer* renderer,
        FLOAT originX,
        FLOAT originY,
        BOOL isSideways,
        BOOL isRightToLeft,
        IUnknown* clientDrawingEffect
        );

    IFACEMETHOD(GetMetrics)(
        OUT DWRITE_INLINE_OBJECT_METRICS* metrics
        );

    IFACEMETHOD(GetOverhangMetrics)(
        OUT DWRITE_OVERHANG_METRICS* overhangs
        );

    IFACEMETHOD(GetBreakConditions)(
        OUT DWRITE_BREAK_CONDITION* breakConditionBefore,
        OUT DWRITE_BREAK_CONDITION* breakConditionAfter
        );

    static HRESULT LoadImageFromResource(
        const wchar_t* resourceName,
        const wchar_t* resourceType,
        IWICImagingFactory* wicFactory,
        OUT IWICBitmapSource** bitmap
        );

    static HRESULT LoadImageFromFile(
        const wchar_t* fileName,
        IWICImagingFactory* wicFactory,
        OUT IWICBitmapSource** bitmap
        );

protected:
    IWICBitmapSource* image_;
    RectF rect_; // coordinates in image, similar to index of HIMAGE_LIST
    float baseline_;
};
