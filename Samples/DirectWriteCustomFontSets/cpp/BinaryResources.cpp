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
#include "BinaryResources.h"


namespace DWriteCustomFontSets
{
    BinaryResources::BinaryResources()
    {
        // Create the vector with all of the fonts compiled as resources into the app binary.

        // Add the icon font to the vector.
        AddFontResourceToVector(L"ICONFONT");

        // Add the body text font to the vector.
        AddFontResourceToVector(L"BODYTEXTFONT");

    } // BinaryResources::BinaryResources()


    void BinaryResources::GetFonts(_Out_ std::vector<DWriteCustomFontSets::MemoryFontInfo>& fonts) const
    {
        fonts = m_appFontResources;
    }



    // IUnknown implementation:

    ULONG BinaryResources::AddRef()
    {
        return InterlockedIncrement(&m_refCount);
    }

    HRESULT BinaryResources::QueryInterface(REFIID riid, _COM_Outptr_ void** object)
    {
        *object = nullptr;
        if (riid == __uuidof(IUnknown))
        {
            AddRef();
            *object = this;
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    ULONG BinaryResources::Release()
    {
        ULONG newCount = InterlockedDecrement(&m_refCount);
        if (newCount == 0)
            delete this;
        return newCount;
    }



    // Private helper method implementation

    void BinaryResources::AddFontResourceToVector(wchar_t const* resourceName)
    {
        // Will search for the resource name using resource type FONTFILE. If found,
        // will create a MemoryFontInfo struct for the resource data and add it to
        // the m_appFontResources vector.

        // Use nullptr for hModule to search in the app binary.
        HRSRC hFontResource = FindResource(nullptr, resourceName, L"FONTFILE");
        if (hFontResource == nullptr)
        {
            // Not found; return without updating the vector.
            return;
        }

        uint32_t binaryFontDataSize = SizeofResource(nullptr, hFontResource);
        HGLOBAL hFontResource_Loaded = LoadResource(nullptr, hFontResource);
        if (hFontResource_Loaded == nullptr)
        {
            return;
        }

        void* binaryFontData = LockResource(hFontResource_Loaded);
        if (binaryFontData != nullptr)
        {
            MemoryFontInfo fontInfo = { binaryFontData, binaryFontDataSize };
            m_appFontResources.push_back(fontInfo);
        }
    } // BinaryResources::AddFontResourceToVector


} // namespace DWriteCustomFontSets