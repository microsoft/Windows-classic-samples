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
#include "Common.h"
#include "RenderTarget.h"
#include "InlineImage.h"


InlineImage::InlineImage(
    IWICBitmapSource* image,
    unsigned int index
    )
    :   image_(SafeAcquire(image))
{
    // Pass the index of the image in the sequence of concatenated sequence
    // (just like toolbar images).
    UINT imageWidth = 0, imageHeight = 0;

    if (image != NULL)
        image->GetSize(&imageWidth, &imageHeight);

    if (index == ~0)
    {
        // No index. Use entire image.
        rect_.left      = 0;
        rect_.top       = 0;
        rect_.right     = float(imageWidth);
        rect_.bottom    = float(imageHeight);
    }
    else
    {
        // Use index.
        float size = float(imageHeight);
        float offset    = index * size;
        rect_.left      = offset;
        rect_.top       = 0;
        rect_.right     = offset + size;
        rect_.bottom    = size;
    }

    baseline_ = float(imageHeight);
}


HRESULT STDMETHODCALLTYPE InlineImage::Draw(
    void* clientDrawingContext,
    IDWriteTextRenderer* renderer,
    FLOAT originX,
    FLOAT originY,
    BOOL isSideways,
    BOOL isRightToLeft,
    IUnknown* clientDrawingEffect
    )
{
    // Go from the text renderer interface back to the actual render target.
    RenderTarget* renderTarget = NULL;
    HRESULT hr = renderer->QueryInterface(__uuidof(RenderTarget), (IID_PPV_ARGS(&renderTarget)));
    if (FAILED(hr))
        return hr;

    float height    = rect_.bottom - rect_.top;
    float width     = rect_.right  - rect_.left;
    RectF destRect  = {originX, originY, originX + width, originY + height};

    renderTarget->DrawImage(image_, rect_, destRect);

    SafeRelease(&renderTarget);

    return S_OK;
}


HRESULT STDMETHODCALLTYPE InlineImage::GetMetrics(
    OUT DWRITE_INLINE_OBJECT_METRICS* metrics
    )
{
    DWRITE_INLINE_OBJECT_METRICS inlineMetrics = {};
    inlineMetrics.width     = rect_.right  - rect_.left;
    inlineMetrics.height    = rect_.bottom - rect_.top;
    inlineMetrics.baseline  = baseline_;
    *metrics = inlineMetrics;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE InlineImage::GetOverhangMetrics(
    OUT DWRITE_OVERHANG_METRICS* overhangs
    )
{
    overhangs->left      = 0;
    overhangs->top       = 0;
    overhangs->right     = 0;
    overhangs->bottom    = 0;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE InlineImage::GetBreakConditions(
    OUT DWRITE_BREAK_CONDITION* breakConditionBefore,
    OUT DWRITE_BREAK_CONDITION* breakConditionAfter
    )
{
    *breakConditionBefore = DWRITE_BREAK_CONDITION_NEUTRAL;
    *breakConditionAfter  = DWRITE_BREAK_CONDITION_NEUTRAL;
    return S_OK;
}


namespace
{
    HRESULT LoadAndLockResource(
        const wchar_t* resourceName,
        const wchar_t* resourceType,
        OUT UINT8** fileData,
        OUT DWORD* fileSize
        )
    {
        HRSRC resourceHandle = NULL;
        HGLOBAL resourceDataHandle = NULL;
        *fileData = NULL;
        *fileSize = 0;

        // Locate the resource handle in our DLL.
        resourceHandle = FindResourceW(
            HINST_THISCOMPONENT,
            resourceName,
            resourceType
            );
        if (resourceHandle == NULL)
            return E_FAIL;

        // Load the resource.
        resourceDataHandle = LoadResource(
            HINST_THISCOMPONENT,
            resourceHandle);
        if (resourceDataHandle == NULL)
            return E_FAIL;

        // Lock it to get a system memory pointer.
        *fileData = (BYTE*)LockResource(resourceDataHandle);
        if (*fileData == NULL)
            return E_FAIL;

        // Calculate the size.
        *fileSize = SizeofResource(HINST_THISCOMPONENT, resourceHandle);
        if (*fileSize == 0)
            return E_FAIL;

        return S_OK;
    }
}


HRESULT InlineImage::LoadImageFromResource(
    const wchar_t* resourceName,
    const wchar_t* resourceType,
    IWICImagingFactory* wicFactory,
    OUT IWICBitmapSource** bitmap
    )
{
    // Loads an image from a resource into the given bitmap.

    HRESULT hr = S_OK;

    DWORD fileSize;
    UINT8* fileData;    // [fileSize]

    IWICStream*             stream    = NULL;
    IWICBitmapDecoder*      decoder   = NULL;
    IWICBitmapFrameDecode*  source    = NULL;
    IWICFormatConverter*    converter = NULL;

    hr = LoadAndLockResource(resourceName, resourceType, &fileData, &fileSize);

    // Create a WIC stream to map onto the memory.
    if (SUCCEEDED(hr))
    {
        hr = wicFactory->CreateStream(&stream);
    }

    // Initialize the stream with the memory pointer and size.
    if (SUCCEEDED(hr))
    {
        hr = stream->InitializeFromMemory(reinterpret_cast<BYTE*>(fileData), fileSize);
    }

    // Create a decoder for the stream.
    if (SUCCEEDED(hr))
    {
        hr = wicFactory->CreateDecoderFromStream(
                stream,
                NULL,
                WICDecodeMetadataCacheOnLoad,
                &decoder
                );
    }

    // Create the initial frame.
    if (SUCCEEDED(hr))
    {
        hr = decoder->GetFrame(0, &source);
    }

    // Convert format to 32bppPBGRA - which D2D expects.
    if (SUCCEEDED(hr))
    {
        hr = wicFactory->CreateFormatConverter(&converter);
    }

    if (SUCCEEDED(hr))
    {
        hr = converter->Initialize(
                source,
                GUID_WICPixelFormat32bppPBGRA,
                WICBitmapDitherTypeNone,
                NULL,
                0.f,
                WICBitmapPaletteTypeMedianCut
                );
    }

    if (SUCCEEDED(hr))
    {
        *bitmap = SafeDetach(&converter);
    }

    SafeRelease(&converter);
    SafeRelease(&source);
    SafeRelease(&decoder);
    SafeRelease(&stream);

    return hr;
}


HRESULT InlineImage::LoadImageFromFile(
    const wchar_t* fileName,
    IWICImagingFactory* wicFactory,
    OUT IWICBitmapSource** bitmap
    )
{
    // Loads an image from a file into the given bitmap.

    HRESULT hr = S_OK;

    // create a decoder for the stream
    IWICBitmapDecoder*      decoder   = NULL;
    IWICBitmapFrameDecode*  source    = NULL;
    IWICFormatConverter*    converter = NULL;

    if (SUCCEEDED(hr))
    {
        hr = wicFactory->CreateDecoderFromFilename(
                fileName,
                NULL,
                GENERIC_READ,
                WICDecodeMetadataCacheOnLoad,
                &decoder
                );
    }

    // Create the initial frame.
    if (SUCCEEDED(hr))
    {
        hr = decoder->GetFrame(0, &source);
    }

    // Convert format to 32bppPBGRA - which D2D expects.
    if (SUCCEEDED(hr))
    {
        hr = wicFactory->CreateFormatConverter(&converter);
    }

    if (SUCCEEDED(hr))
    {
        hr = converter->Initialize(
                source,
                GUID_WICPixelFormat32bppPBGRA,
                WICBitmapDitherTypeNone,
                NULL,
                0.f,
                WICBitmapPaletteTypeMedianCut
                );
    }

    if (SUCCEEDED(hr))
    {
        *bitmap = SafeDetach(&converter);
    }

    SafeRelease(&converter);
    SafeRelease(&source);
    SafeRelease(&decoder);

    return hr;
}
