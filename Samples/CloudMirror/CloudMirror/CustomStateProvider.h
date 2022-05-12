// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "CustomStateProvider.g.h"
#include <windows.storage.provider.h>

namespace winrt::CloudMirror::implementation
{
    struct __declspec(uuid("f0c9de6c-6c76-44d7-a58e-579cdf7af263"))
    CustomStateProvider : CustomStateProviderT<CustomStateProvider>
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
