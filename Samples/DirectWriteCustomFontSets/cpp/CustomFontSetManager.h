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
#include "DXHelper.h"


namespace DWriteCustomFontSets
{
    // Struct for defining a set of known fonts used by the app: app-internal font properties,
    // and a font location (local file path, or relative URL).
    struct AppFontInfo
    {
        _Field_z_ const wchar_t* familyName;
        _Field_z_ const wchar_t* fullName;
        _Field_z_ const wchar_t* fontWeight;
        _Field_z_ const wchar_t* fontRelativeLocation; // local file path, or relative URL
        const uint32_t   fontIndex;
    };


    // Struct for in-memory, raw font data.
    struct MemoryFontInfo
    {
        _Field_size_bytes_(fontDataSize) const void* fontData;
        uint32_t fontDataSize;
    };


    // Struct for in-memory, packed font data. In this sample we assume the format is
    // known. If not known in advance, the container format of a file can be checked
    // using IDWriteFactory5::AnalyzeContainerType.
    struct PackedFontInfo
    {
        _Field_size_bytes_(fontDataSize) const uint8_t* fontData;
        uint32_t fontDataSize;
        DWRITE_CONTAINER_TYPE fileContainer; // indicate WOFF, WOFF2; if raw font file, use UNKNOWN
    };



    class CustomFontSetManager
    {
    public:
        CustomFontSetManager();
        ~CustomFontSetManager();

        // Method for checking API availability
        bool IDWriteFactory5_IsAvailable();

        // Methods for creating font sets under the different scenarios.
        void CreateFontSetUsingLocalFontFiles(const std::vector<std::wstring>& selectedFilePathNames);
        void CreateFontSetUsingKnownAppFonts();
        void CreateFontSetUsingKnownRemoteFonts();
        void CreateFontSetUsingInMemoryFontData();
        void CreateFontSetUsingPackedFontData();

        // Methods for obtaining information about the fonts in the font set.
        uint32_t                                GetFontCount() const;
        std::vector<std::wstring>               GetFullFontNames() const;
        std::vector<std::wstring>               GetFontDataDetails(HANDLE cancellationHandle) const;
        bool                                    CustomFontSetHasRemoteFonts() const;


    private:
        void UnregisterFontFileLoader(IDWriteFontFileLoader* fontFileLoader);
        Microsoft::WRL::ComPtr<IDWriteStringList>   GetPropertyValuesFromFontSet(DWRITE_FONT_PROPERTY_ID propertyId) const;

        Microsoft::WRL::ComPtr<IDWriteFactory3>             m_dwriteFactory3; // Available on all Windows 10 versions (build 10240 and later).
        Microsoft::WRL::ComPtr<IDWriteFactory5>             m_dwriteFactory5; // Available on Windows 10 Creators Update (preview build 15021 or later).
        Microsoft::WRL::ComPtr<IDWriteFontSet>              m_customFontSet;

        // The following are held as class members since they will be needed when creating a font
        // set (in the relevant scenarios), but will also be needed for as long as the app may
        // need to access the font data. File file loaders must be registered with a DirectWrite
        // factory object, and must also be unregistered before this class goes out of scope. This
        // will be done in the CustomFontSetManager destructor.
        Microsoft::WRL::ComPtr<IDWriteRemoteFontFileLoader>     m_remoteFontFileLoader; // Holds a system-implemented IDWriteRemoteFontFileLoader.
        Microsoft::WRL::ComPtr<IDWriteInMemoryFontFileLoader>   m_inMemoryFontFileLoader; // Holds a system-implemented IDWriteInMemoryFontFileLoader.
        Microsoft::WRL::ComPtr<IDWriteFontFileLoader>           m_packedFontFileLoader; // Holds a custom IDWriteFontFileLoader implementation.

    }; // class CustomFontSetManager


       // The following are defined in Statics.cpp:
    extern AppFontInfo const    g_appFonts[];
    extern uint32_t const       g_appFontsCount;
    extern wchar_t const*       g_remoteFontBaseUrl;
    extern AppFontInfo const    g_remoteFonts[];
    extern uint32_t const       g_remoteFontsCount;
    extern PackedFontInfo const g_packedFonts[]; // can also include raw font data not in a WOFF/WOFF2 container
    extern uint32_t const       g_packedFontsCount;

} // namespace DWriteCustomFontSets