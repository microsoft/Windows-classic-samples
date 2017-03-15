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

// Defines the BinaryResources class -- used in scenario 4 for handling
// font data that is embedded in the app binary as a resource and then
// loaded into memory.
//
// In order to avoid DirectWrite making a copy of the font data when the font is
// added to the font set, there must be a data-owner object that implements the
// IUnknown interface. This object serves as that data owner. It gets passed, 
// in the call to IDWriteFactory5::CreateInMemoryFontFileReference along with a 
// pointer to the font data. By specifying a non-null owner, that enables the 
// loader to assume that the data pointer will remain valid as long as it holds
// a reference to the owner object.


#pragma once

#include "stdafx.h"
#include "CustomFontSetManager.h"


namespace DWriteCustomFontSets
{
    class BinaryResources : public IUnknown
    {
    public:
        BinaryResources();

        void GetFonts(_Out_ std::vector<DWriteCustomFontSets::MemoryFontInfo>& fonts) const;

        // IUnknown interface
        ULONG STDMETHODCALLTYPE AddRef() override;
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void **object) override;
        ULONG STDMETHODCALLTYPE Release() override;

    private:
        void AddFontResourceToVector(wchar_t const* resourceName);

        std::vector<DWriteCustomFontSets::MemoryFontInfo> m_appFontResources;
        ULONG m_refCount = 0;

    };
} // namespace DWriteCustomFontSets