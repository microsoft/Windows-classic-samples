//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include <vector>
#include "FontFileStream.h"

//  This class implements the font collection and font file loader interfaces
//  for DirectWrite to call to load document/application-specific font to the
//  system. The font loader's lifetime is tied to the factory it's registered to.
class FontLoader :  public IDWriteFontCollectionLoader,
                    public IDWriteFontFileEnumerator,
                    public IDWriteFontFileLoader
{
public:
    FontLoader(
        _In_ std::wstring path,
        _In_ IDWriteFactory* dwriteFactory
        );

    HRESULT Load();

    // IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
        REFIID uuid,
        _Outptr_ void** object
        ) override;

    virtual ULONG STDMETHODCALLTYPE AddRef() override;

    virtual ULONG STDMETHODCALLTYPE Release() override;

    // IDWriteFontCollectionLoader
    virtual HRESULT STDMETHODCALLTYPE CreateEnumeratorFromKey(
        _In_ IDWriteFactory* factory,
        _In_reads_bytes_(fontCollectionKeySize) void const* fontCollectionKey,
        UINT32 fontCollectionKeySize,
        _Outptr_ IDWriteFontFileEnumerator** fontFileEnumerator
        ) override;

    // IDWriteFontFileEnumerator
    virtual HRESULT STDMETHODCALLTYPE MoveNext(OUT BOOL* hasCurrentFile) override;

    virtual HRESULT STDMETHODCALLTYPE GetCurrentFontFile(OUT IDWriteFontFile** currentFontFile) override;

    // IDWriteFontFileLoader
    virtual HRESULT STDMETHODCALLTYPE CreateStreamFromKey(
        _In_reads_bytes_(fontFileReferenceKeySize) void const* fontFileReferenceKey,
        UINT32 fontFileReferenceKeySize,
        _Outptr_ IDWriteFontFileStream** fontFileStream
        ) override;

private:
    // Reference counter
    ULONG m_refCount;

    // Storage location of the font file(s) to load
    std::wstring m_location;

    // Number of font files in the storage location
    size_t m_fontFileCount;

    // A list of font file streams loaded
    std::vector<Microsoft::WRL::ComPtr<FontFileStream>> m_fontFileStreams;

    // Index to the current font file stream in the loaded list
    size_t m_fontFileStreamIndex;

    // Current DirectWrite font file being indexed
    Microsoft::WRL::ComPtr<IDWriteFontFile> m_currentFontFile;

    // DirectWrite factory
    Microsoft::WRL::ComPtr<IDWriteFactory> m_dwriteFactory;
};
