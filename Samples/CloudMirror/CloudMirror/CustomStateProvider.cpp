// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"
#include "CustomStateProvider.h"

namespace winrt
{
    using namespace winrt::Windows::Storage::Provider;
}

namespace winrt::CloudMirror::implementation
{
    Windows::Foundation::Collections::IIterable<Windows::Storage::Provider::StorageProviderItemProperty> CustomStateProvider::GetItemProperties(hstring const& itemPath)
    {
        std::hash<std::wstring> hashFunc;
        auto hash = hashFunc(itemPath.c_str());

        std::vector<winrt::StorageProviderItemProperty> properties;

        if ((hash & 0x1) != 0)
        {
            winrt::StorageProviderItemProperty itemProperty;
            itemProperty.Id(2);
            itemProperty.Value(L"Value2");
            // This icon is just for the sample. You should provide your own branded icon here
            itemProperty.IconResource(L"shell32.dll,-14");
            properties.push_back(std::move(itemProperty));
        }

        return winrt::single_threaded_vector(std::move(properties));
    }
}
