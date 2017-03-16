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


#pragma once

#include "stdafx.h"
#include "CustomFontSetManager.h"
#include "StaticFontFileStream.h"



namespace DWriteCustomFontSets
{
    // Custom implementation of IDWriteFontFileLoader, used for handling fonts
    // that may be in packed (WOFF, WOFF2) or unpacked formats. The
    // IDWriteFontFile interface has callback methods that DirectWrite uses
    // for on-demand loading of one or more fonts that have been included
    // in a custom font collection or font set.

    class PackedFontFileLoader : public IDWriteFontFileLoader
    {
    public:
        PackedFontFileLoader(_In_ IDWriteFactory5* factory);

        // IDWriteFontFileLoader member methods

        HRESULT STDMETHODCALLTYPE CreateStreamFromKey(
            _In_reads_bytes_(fontFileReferenceKeySize) void const* fontFileReferenceKey,
            uint32_t fontFileReferenceKeySize,
            _COM_Outptr_ IDWriteFontFileStream** fontFileStream
        );

        // IDWriteFontFileLoader inherits from IUknown
        ULONG STDMETHODCALLTYPE AddRef() override;
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **object) override;
        ULONG STDMETHODCALLTYPE Release() override;

    private:
        ULONG m_refCount = 0;
        IDWriteFactory5* m_dwriteFactory5;

    }; // class PackedFontFileLoader 

} // namespace DWriteCustomFontSets