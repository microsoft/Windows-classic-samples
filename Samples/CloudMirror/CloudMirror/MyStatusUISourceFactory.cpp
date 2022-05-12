// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"
#include "MyStatusUISourceFactory.h"
#include "MyStatusUISource.h"

using namespace winrt::Windows::Storage::Provider;

namespace winrt::CloudMirror::implementation
{
    winrt::IStorageProviderStatusUISource MyStatusUISourceFactory::GetStatusUISource(hstring const& /*syncRootId*/)
    {
        // A real sync engine should use the syncRootId to verify the correct user account to create the StatusUISource for.
        return winrt::make<MyStatusUISource>();
    }
}
