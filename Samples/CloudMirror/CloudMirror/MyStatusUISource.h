// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

namespace winrt::CloudMirror::implementation
{
    struct MyStatusUISource : implements<MyStatusUISource, Windows::Storage::Provider::IStorageProviderStatusUISource>
    {
        MyStatusUISource() = default;

        Windows::Storage::Provider::StorageProviderStatusUI GetStatusUI();

        // event StatusUIChanged
        winrt::event_token StatusUIChanged(winrt::Windows::Foundation::TypedEventHandler<winrt::Windows::Storage::Provider::IStorageProviderStatusUISource, winrt::Windows::Foundation::IInspectable> const& handler);
        void StatusUIChanged(winrt::event_token const& token) noexcept;

    private:

        // This is a simple weighting of R, G, and B to account for perceptual brightness. 
        // It is more than enough to determine lightmode -vs- darkmode (because the colors
        // are nearly black or nearly white), but it should not be taken 
        // as a canonical formula for perceptual brightness

        // This is a hacky solution that we do not encourage providers to use for detecting theme changes
        inline bool IsColorLight(Windows::UI::Color& clr)
        {
            return (((5 * clr.G) + (2 * clr.R) + clr.B) > (8 * 128));
        }
    };
}
