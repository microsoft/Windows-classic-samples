//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************


#include "stdafx.h"
#include "PackedFontFileLoader.h"

using Microsoft::WRL::ComPtr;


namespace DWriteCustomFontSets
{
    // PackedFontFileLoader implementation:

    // constructors, destructors
    PackedFontFileLoader::PackedFontFileLoader(_In_ IDWriteFactory5* factory) :
        m_dwriteFactory5{ factory }
    {
    }


    // IDWriteFontFileLoader implementations

    HRESULT PackedFontFileLoader::CreateStreamFromKey(_In_reads_bytes_(fontFileReferenceKeySize) void const* fontFileReferenceKey, uint32_t fontFileReferenceKeySize, _COM_Outptr_ IDWriteFontFileStream** fontFileStream)
    {
        // This is a callback that DirectWrite calls to obtain the font data stream for a 
        // specific font.

        *fontFileStream = nullptr;

        // The key that's passed is the same key we provide when CreateCustomFontFileReference
        // is called. It's for our use as an ID for particular font file data. What we're 
        // expecting is a 32-bit index: make sure it's the right size.
        if (fontFileReferenceKeySize != sizeof(uint32_t))
            return E_INVALIDARG;

        // The key is an index into the g_packedFonts array; make sure it's in range.
        uint32_t fontIndex = *static_cast<uint32_t const*>(fontFileReferenceKey);
        if (fontIndex >= g_packedFontsCount)
            return E_INVALIDARG;

        PackedFontInfo const& fontInfo = g_packedFonts[fontIndex];

        // For this sample, it's assumed that the font data being processed is in packed format 
        // (WOFF or WOFF2). If this loader is going to be used for packed and raw / unpacked font 
        // data, then the container type should be tested (if not known, this can be done using
        // IDWriteFactory5::AnalyzeContainerType), and then the code should branch for the two
        // cases. For raw font data, the app would need to implement the IDWriteFontFileStream
        // interface to wrap the data, since DirectWrite uses that interface to read in the data.
        //
        // For unpacked font files, whether in local storage or in a memory buffer, there are 
        // easier ways to load the font, using APIs illustrated in other scenarios within this
        // sample. Instead of using the custom loader for both packed and unpacked font files,
        // the branching could happen at a higher level, using a custom loader for loading packed 
        // font files, but a different implementation for unpacked font files.

        // data is packed, WOFF or WOFF2 format.
        return m_dwriteFactory5->UnpackFontFile(
            fontInfo.fileContainer,
            fontInfo.fontData,
            fontInfo.fontDataSize,
            fontFileStream
        );

    }



    // IUnknown implementations

    ULONG PackedFontFileLoader::AddRef()
    {
        return InterlockedIncrement(&m_refCount);
    }

    HRESULT PackedFontFileLoader::QueryInterface(REFIID riid, void** object)
    {
        *object = nullptr;
        if (riid == __uuidof(IUnknown)
            || riid == __uuidof(IDWriteFontFileLoader))
        {
            AddRef();
            *object = this;
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    ULONG PackedFontFileLoader::Release()
    {
        ULONG newCount = InterlockedDecrement(&m_refCount);
        if (newCount == 0)
            delete this;
        return newCount;
    }

} // namespace DWriteCustomFontSets