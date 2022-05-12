// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"
#include "MyStatusUISource.h"
#include "MyStorageProviderUICommand.h"
#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.UI.ViewManagement.h>

namespace winrt
{
    using namespace ::winrt::Windows::Foundation;
    using namespace ::winrt::Windows::Foundation::Collections;
    using namespace ::winrt::Windows::UI::Core;
    using namespace ::winrt::Windows::UI::ViewManagement;
    using namespace ::winrt::Windows::Storage::Provider;
}


namespace winrt::CloudMirror::implementation
{
    StorageProviderStatusUI MyStatusUISource::GetStatusUI()
    {
        auto theme = L"";

        // GetStatusUI will be invoked on a theme & contrast changes from the client side
        // We expect providers to provider alternative paths to the icons in this scenario

        auto settings = winrt::UISettings();
        auto foreground = settings.GetColorValue(UIColorType::Foreground);
        bool isLightMode = !IsColorLight(foreground);
        if (isLightMode)
        {
            theme = L"\\lightTheme";
        }
        else
        {
            theme = L"\\darkTheme";
        }

        auto package = Windows::ApplicationModel::Package::Current();
        auto imagesPath = package.InstalledLocation().Path() + L"\\Images" + theme;

        auto ui = StorageProviderStatusUI();

        // Required: Report the provider state.
        StorageProviderState state = CloudProviderSyncRootWatcher::State();
        ui.ProviderState(state);
        if (state == StorageProviderState::InSync)
        {
            // Brief description of sync state.
            ui.ProviderStateLabel(L"Synced");
            // Optional icon.
            ui.ProviderStateIcon(Uri{ imagesPath + L"\\CloudIconSynced.svg" });
        }
        else
        {
            // Brief description of sync state.
            ui.ProviderStateLabel(L"Syncing");
            ui.ProviderStateIcon(Uri{ imagesPath + L"\\CloudIconSyncing.svg" });
        }

        // Optional: Sync status command.
        // If you do not set a SyncStatusCommand, or the command is Hidden,
        // then the sync status banner is not shown.
        IStorageProviderUICommand syncStatusCommand;
        if (state == StorageProviderState::InSync)
        {
            syncStatusCommand = winrt::make<MyStorageProviderUICommand>(
                L"Your files are in sync",
                L"Your files are in sync with CloudMirror.",
                Uri{ imagesPath + L"\\CloudIconSynced.svg" },
                StorageProviderUICommandState::Enabled);
        }
        else
        {
            syncStatusCommand = winrt::make<MyStorageProviderUICommand>(
                L"Your files are syncing",
                L"Your files are syncing with CloudMirror.",
                Uri{ imagesPath + L"\\CloudIconSyncing.svg" },
                StorageProviderUICommandState::Enabled);
        }
        ui.SyncStatusCommand(syncStatusCommand);

        // Optional: Quota information.
        // If you do not set a QuotaUI, then the quota section is not shown.
        // This sample generates random quota information.
        auto quotaUI = StorageProviderQuotaUI();
        auto percent = 100 * rand() / RAND_MAX;
        int64_t gigabyte = 1073741824;
        auto totalQuota = 1024 * gigabyte; // 1 terabyte
        auto quotaUsed = percent * totalQuota / 100;
        quotaUI.QuotaTotalInBytes(totalQuota);
        quotaUI.QuotaUsedInBytes(quotaUsed);
        quotaUI.QuotaUsedLabel(to_hstring(quotaUsed / gigabyte) + L" GB of 1 TB");
        // Optional: Set a custom color for the quota bar.
        if (percent > 90)
        {
            quotaUI.QuotaUsedColor(Windows::UI::Colors::Red());
        }

        ui.QuotaUI(quotaUI);

        // Optional: More info
        // We display more info if the user is running out of storage.
        if (percent > 80)
        {
            auto moreInfoUI = StorageProviderMoreInfoUI();
            moreInfoUI.Message(L"Your CloudMirror is getting full. Get more storage so you won't run out.");
            moreInfoUI.Command(winrt::make<MyStorageProviderUICommand>(
                L"Explore storage plans",
                L"Explore CloudMirror storage plans.",
                Uri{ imagesPath + L"\\CloudIconSynced.svg" },
                StorageProviderUICommandState::Enabled));
            ui.MoreInfoUI(moreInfoUI);
        }

        // Optional: The primary command is shown as a textual button.
        auto primaryCommand = winrt::make<MyStorageProviderUICommand>(
            L"View online",
            L"View CloudMirror online.",
            Uri{ imagesPath + L"\\CloudIconSynced.svg" },
            StorageProviderUICommandState::Enabled);
        ui.ProviderPrimaryCommand(primaryCommand);

        // Optional: Secondary commands are shown as icons.
        auto secondaryCommand1 = winrt::make<MyStorageProviderUICommand>(
            L"View profile",
            L"View your CloudMirror user profile",
            Uri{ imagesPath + L"\\ProfileIcon.svg" },
            StorageProviderUICommandState::Enabled);
        auto secondaryCommand2 = winrt::make<MyStorageProviderUICommand>(
            L"Settings",
            L"View your CloudMirror settings",
            Uri{ imagesPath + L"\\SettingsIcon.svg" },
            StorageProviderUICommandState::Enabled);
        ui.ProviderSecondaryCommands({ secondaryCommand1, secondaryCommand2 });

        return ui;
    }

    // The provider raises the StatusUIChanged event when the status has changed.
    // Upon learning that the status has changed, the system will call GetStatusUI to
    // get the new status.
    winrt::event_token MyStatusUISource::StatusUIChanged(winrt::Windows::Foundation::TypedEventHandler<IStorageProviderStatusUISource, IInspectable> const& handler)
    {
        return CloudProviderSyncRootWatcher::StatusChanged([weak = get_weak(), agile_handler = agile_ref(handler)](auto&&, auto&&)
        {
            if (auto strong = weak.get())
            {
                agile_handler.get()(*strong, nullptr);
            }
        });
    };

    void MyStatusUISource::StatusUIChanged(winrt::event_token const& token) noexcept
    {
        return CloudProviderSyncRootWatcher::StatusChanged(token);
    }
}
