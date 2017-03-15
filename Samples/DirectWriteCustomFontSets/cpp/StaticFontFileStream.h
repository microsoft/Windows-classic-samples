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


namespace DWriteCustomFontSets
{
    // Custom implementation of IDWriteFontFileStream, used for handling unpacked font
    // data in a custom IDWriteFontFileLoader implementation. The IDWriteFontFileStream
    // interface has callback methods that DirectWrite uses to access font file data for
    // some particular font.
    //
    // This sample makes a limiting assumption that the data will be static and defined
    // in the app. For scenarios involving non-static font data from external sources, 
    // ownership and lifetime of data should be taken into consideration. For example,
    // an implementation could accept an R-value reference to std::vector<uint8_t> in 
    // the ctor and then move ownership of the underlying data into this class.
    class StaticFontFileStream : public IDWriteFontFileStream
    {
    public:
        StaticFontFileStream(uint8_t const* data, uint64_t dataSize) :
            m_data{ data },
            m_dataSize{ dataSize }
        {
        }

        // IDWriteFontFileStream member methods
        virtual HRESULT STDMETHODCALLTYPE ReadFileFragment(
            _Outptr_result_bytebuffer_(fragmentSize) void const** fragmentStart,
            UINT64 fileOffset,
            UINT64 fragmentSize,
            _Out_ void** fragmentContext
        ) override;

        virtual void STDMETHODCALLTYPE ReleaseFileFragment(
            _In_ void* fragmentContext
        ) override
        {
            // We never allocate memory for a fragment context, so this is a no-op.
        }

        virtual HRESULT STDMETHODCALLTYPE GetFileSize(
            _Out_ UINT64* fileSize
        ) override;

        virtual HRESULT STDMETHODCALLTYPE GetLastWriteTime(
            _Out_ UINT64* lastWriteTime
        ) override;

        // IDWriteFontFileStream inherits from IUknown
        ULONG STDMETHODCALLTYPE AddRef() override;
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **object) override;
        ULONG STDMETHODCALLTYPE Release() override;

    private:
        ULONG m_refCount = 0;
        uint8_t const* m_data;
        uint64_t const m_dataSize;

    }; // StaticFontFileStream class

} // namespace DWriteCustomFontSets