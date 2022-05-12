// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "UriSource.g.h"

namespace winrt::CloudMirror::implementation
{
    struct __declspec(uuid("97961bcb-601c-4950-927c-43b9319c7217"))
        UriSource : UriSourceT<UriSource>
    {
        UriSource() = default;

        void GetPathForContentUri(_In_ hstring const& contentUri, _Out_ Windows::Storage::Provider::StorageProviderGetPathForContentUriResult const& result);
        void GetContentInfoForPath(_In_ hstring const& path, _Out_ Windows::Storage::Provider::StorageProviderGetContentInfoForPathResult const& result);
    };
}

namespace winrt::CloudMirror::factory_implementation
{
    struct UriSource : UriSourceT<UriSource, implementation::UriSource>
    {
    };
}
