// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

namespace winrt::CloudMirror::implementation
{
    struct MyStorageProviderUICommand : implements<MyStorageProviderUICommand, Windows::Storage::Provider::IStorageProviderUICommand>
    {
        MyStorageProviderUICommand(hstring const& label, hstring const& description, Windows::Foundation::Uri const& iconUri, Windows::Storage::Provider::StorageProviderUICommandState const& state)
            : m_label(label), m_description(description), m_iconUri(iconUri), m_state(state)
        {}

        // Label: The text to show.
        auto Label() const { return m_label; }

        // Description: Detailed information.
        auto Description() const { return m_description; }

        // Icon: The icon to show.
        auto Icon() const { return m_iconUri; }

        // State:
        // * Enabled shows the button in an enabled state.
        // * Disabled shows the button in a disabled state.
        // * Hidden hides the button entirely.
        auto State() const { return m_state; }

        // Invoke: The system calls this method when the user executes the command.
        void Invoke();

    private:
        hstring m_label;
        hstring m_description;
        Windows::Foundation::Uri m_iconUri;
        Windows::Storage::Provider::StorageProviderUICommandState m_state = Windows::Storage::Provider::StorageProviderUICommandState::Enabled;
    };
}
