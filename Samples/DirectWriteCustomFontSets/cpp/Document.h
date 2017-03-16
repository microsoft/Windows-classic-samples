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


// Defines the Document class -- used in scenario 4 to simulate a document that
// contains embedded font data that gets loaded into an in-memory buffer and 
// then added into a custom font set using a system-implemented 
// IDWriteInMemoryFontFileLoader.
//
// In order to avoid DirectWrite making a copy of the font data when the font is
// added to the font set, there must be a data-owner object that implements the
// IUnknown interface. This object gets passed, along with a pointer to the font
// data, in the call to IDWriteFactory5::CreateInMemoryFontFileReference. By 
// specifying a non-null owner, that enables the loader to assume that the data
// pointer will remain valid as long as it holds a reference to the owner object.
// In this sample, the data will be static, so the owner object doesn't need to
// do anything except implement the IUnknown interface.


#pragma once

#include "stdafx.h"
#include "CustomFontSetManager.h"


namespace Documents
{
    class Document : public IUnknown {
    public:
        // Document data
        std::wstring GetText() const;
        void GetFonts(_Out_ std::vector<DWriteCustomFontSets::MemoryFontInfo>& fonts) const;


        // IUnknown interface
        ULONG STDMETHODCALLTYPE AddRef() override;
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void **object) override;
        ULONG STDMETHODCALLTYPE Release() override;

    private:
        ULONG m_refCount = 0;

    };
} // namespace Documents
