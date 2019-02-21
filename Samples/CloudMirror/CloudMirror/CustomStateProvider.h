// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "CustomStateProvider.g.h"
#include <windows.storage.provider.h>

/* f0c9de6c-6c76-44d7-a58e-579cdf7af263 */
constexpr CLSID CLSID_CustomStateProvider = { 0xf0c9de6c, 0x6c76, 0x44d7, {0xa5, 0x8e, 0x57, 0x9c, 0xdf, 0x7a, 0xf2, 0x63} };

namespace winrt::CloudMirror::implementation
{
    struct CustomStateProvider : CustomStateProviderT<CustomStateProvider>
    {
        CustomStateProvider() = default;

        Windows::Foundation::Collections::IIterable<Windows::Storage::Provider::StorageProviderItemProperty> GetItemProperties(_In_ hstring const& itemPath);
    };
}

namespace winrt::CloudMirror::factory_implementation
{
    struct CustomStateProvider : CustomStateProviderT<CustomStateProvider, implementation::CustomStateProvider>
    {
    };
}
