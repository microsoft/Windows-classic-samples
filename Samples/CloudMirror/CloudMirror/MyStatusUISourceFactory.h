// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "MyStatusUISourceFactory.g.h"

namespace winrt::CloudMirror::implementation
{
    struct __declspec(uuid("b1d8ef74-822d-401a-a14a-25f45b1f70b7"))
        MyStatusUISourceFactory : MyStatusUISourceFactoryT<MyStatusUISourceFactory>
    {
        MyStatusUISourceFactory() = default;

        winrt::Windows::Storage::Provider::IStorageProviderStatusUISource GetStatusUISource(hstring const& syncRootId);

    private:
        winrt::Windows::Storage::Provider::IStorageProviderStatusUISource m_statusUISource{ nullptr };
    };
}

namespace winrt::CloudMirror::factory_implementation
{
    struct MyStatusUISourceFactory : MyStatusUISourceFactoryT<MyStatusUISourceFactory, implementation::MyStatusUISourceFactory>
    {
    };
}
