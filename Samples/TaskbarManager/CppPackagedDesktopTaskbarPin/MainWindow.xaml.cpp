// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

namespace winrt
{
    using namespace Windows::ApplicationModel;
    using namespace Windows::Foundation;
    using namespace Windows::UI::Shell;
    using namespace Windows::UI::StartScreen;
    using namespace Microsoft::UI::Xaml;
}

namespace
{
    // Tries to unlock the necessary taskbar pinning LAF and returns true if successful.
    bool TryUnlockPinningLaf()
    {
        // Ensure the appropriate LAF is unlocked.
        // Appropriate LAF values must be specified in "LafData.h" based on your LAF token.
        auto unlockStatus = winrt::LimitedAccessFeatures::TryUnlockFeature(
            c_lafFeature.data(), c_lafToken.data(), c_lafAttestation.data()).Status();
        return ((unlockStatus == winrt::LimitedAccessFeatureStatus::Available) || (unlockStatus == winrt::LimitedAccessFeatureStatus::AvailableWithoutToken));
    }

    // Returns true if TaskbarManager supports desktop clients.
    bool IsDesktopAppPinningSupported()
    {
        // Check whether the available TaskbarManager supports desktop app clients by checking for the associated marker interface.
        return !!winrt::try_get_activation_factory<winrt::TaskbarManager, winrt::ITaskbarManagerDesktopAppSupportStatics>();
    }
}

namespace winrt::CppPackagedDesktopTaskbarPin::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();

        // Usage of this sample requires that you have an assigned LAF token for this functionality and have filled it in in LafData.h.
        if (!c_lafToken.empty())
        {
            this->samplePanel().Visibility(Visibility::Visible);
            this->lafErrorText().Visibility(Visibility::Collapsed);
        }
    }

    // Checks whether pinning the current app is possible and updates the pin button and status text accordingly.
    // NOTE: Your app should not give the user an option to pin if requesting the pin won't do anything, so the ability to pin should be checked first.
    //       There is no guarantee that an app will be allowed to pin at any given time.
    winrt::fire_and_forget MainWindow::PinCurrentScenarioButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
    {
        auto strongThis{ get_strong() };

        // First, check whether pinning from desktop apps is supported at all.
        if (TryUnlockPinningLaf() && IsDesktopAppPinningSupported())
        {
            // Then, check whether the current app is already pinned and, if not, whether pinning is currently allowed from this app.
            auto taskbarManager = winrt::TaskbarManager::GetDefault();
            if (!co_await taskbarManager.IsCurrentAppPinnedAsync())
            {
                if (taskbarManager.IsPinningAllowed())
                {
                    // Pinning is supported and allowed, and the app is not already pinned. The app can offer the user the option to pin it.
                    this->pinCurrentScenarioPinButton().IsEnabled(true);
                    this->pinCurrentScenarioStatusText().Text({});
                }
                else
                {
                    // Pinning is supported, but pinning is not currently allowed (e.g., the system may not be able to show the prompt
                    // at this time). The app should not offer the usre an option to pin now, but it may check again at a later time
                    // (such as when showing the UI with the pin option on it again).
                    this->pinCurrentScenarioPinButton().IsEnabled(false);
                    this->pinCurrentScenarioStatusText().Text(L"Pinning is not allowed");
                }
            }
            else
            {
                // Pinning is supported, but the app is already pinned. The app should not offer ther user an option to pin again.
                this->pinCurrentScenarioPinButton().IsEnabled(false);
                this->pinCurrentScenarioStatusText().Text(L"App is already pinned");
            }

            this->pinCurrentScenarioPinButton().Visibility(Visibility::Visible);
        }
        else
        {
            // Pinning is not supported on this system, either because the limited access feature couldn't be unlocked
            // or the API indicated desktop callers aren't supported. The app should not offer the user an option to pin.
            this->pinCurrentScenarioStatusText().Text(L"Pinning is not supported");
            this->pinCurrentScenarioPinButton().Visibility(Visibility::Collapsed);
        }
    }

    // Attempts to pin the current app and update the status text with the result.
    winrt::fire_and_forget MainWindow::PinCurrentScenarioPinButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
    {
        auto strongThis{ get_strong() };

        // Make the pin request. Additional pin requests for the current app may not be made
        // while one is still outstanding, so disable the pin request UI until the operation is complete.
        this->pinCurrentScenarioButton().IsEnabled(false);
        this->pinCurrentScenarioPinButton().IsEnabled(false);
        auto isPinned = co_await winrt::TaskbarManager::GetDefault().RequestPinCurrentAppAsync();

        // Update the UI to the appropriate state based on the results of the pin request.
        this->pinCurrentScenarioStatusText().Text(isPinned ? L"Pinned" : L"Not pinned");
        this->pinCurrentScenarioPinButton().IsEnabled(!isPinned);
        this->pinCurrentScenarioButton().IsEnabled(true);
    }

    // Checks whether pinning a secondary tile for the current app is possible and updates the pin/unpin button and status text accordingly.
    winrt::fire_and_forget MainWindow::PinSecondaryScenarioButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
    {
        // NOTE: Your app should not give the user an option to pin if requesting the pin won't do anything, so the ability to pin should be checked first.
        //       There is no guarantee that an app will be allowed to pin at any given time.
        co_await UpdateSecondaryPinScenarioUI();
    }

    // Attempts to pin a secondary tile for the current app and update the UI with the result.
    winrt::fire_and_forget MainWindow::PinSecondaryScenarioPinButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
    {
        auto strongThis{ get_strong() };

        // Create the secondary tile for the current app that will be pinned. It can specify additional arguments that will be passed to the app when it's launched via this tile.
        auto tile = winrt::SecondaryTile(
            L"TestSecondaryId",
            L"Test secondary tile",
            L"--testArgument",
            winrt::Uri(L"ms-appx:///Assets/Square150x150Secondary.png"),
            winrt::TileSize::Default);
        tile.VisualElements().Square44x44Logo(winrt::Uri(L"ms-appx:///Assets/Square44x44Secondary.png"));

        // Request that the secondary tile be pinned. Additional pin requests for the same tile may not
        // be made while one is still outstanding, so disable the pin request UI until the operation is complete.
        this->pinSecondaryScenarioButton().IsEnabled(false);
        this->pinSecondaryScenarioPinButton().IsEnabled(false);
        auto wasPinned = co_await winrt::TaskbarManager::GetDefault().RequestPinSecondaryTileAsync(tile);

        // Update the UI to the appropriate state based on the results of the pin request.
        co_await UpdateSecondaryPinScenarioUI();
        this->pinSecondaryScenarioStatusText().Text(
            (wasPinned ? L"Pinned  " : L"Not pinned  ") + this->pinSecondaryScenarioStatusText().Text());
        this->pinSecondaryScenarioButton().IsEnabled(true);
    }

    // Attempts to unpin the previously pinned secondary tile and update the UI with the result.
    winrt::fire_and_forget MainWindow::PinSecondaryScenarioUnpinButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
    {
        auto strongThis{ get_strong() };

        // Try to unpin the secondary tile that was pinned by this app earlier by providing its ID.
        // Disable the unpin UI while the unpin request is ongoing to prevent multiple simultaneous requests.
        this->pinSecondaryScenarioButton().IsEnabled(false);
        this->pinSecondaryScenarioUnpinButton().IsEnabled(false);
        auto wasUnpinned = co_await winrt::TaskbarManager::GetDefault().TryUnpinSecondaryTileAsync(L"TestSecondaryId");

        // Update the UI to the appropriate state based on the results of the unpin request.
        co_await UpdateSecondaryPinScenarioUI();
        this->pinSecondaryScenarioStatusText().Text(
            (wasUnpinned ? L"Unpinned  " : L"Not unpinned  ") + this->pinSecondaryScenarioStatusText().Text());
        this->pinSecondaryScenarioButton().IsEnabled(true);
    }

    // Checks the current pinning capabilities and status and updates the secondary tile pin scenario UI accordingly.
    winrt::IAsyncAction MainWindow::UpdateSecondaryPinScenarioUI()
    {
        auto thisStrong{ get_strong() };

        // First, check whether pinning from desktop apps is supported at all.
        if (TryUnlockPinningLaf() && IsDesktopAppPinningSupported())
        {
            // Then, check whether the tile is already pinned and, if not, whether pinning is currently allowed from this app.
            auto taskbarManager = winrt::TaskbarManager::GetDefault();
            if (!co_await taskbarManager.IsSecondaryTilePinnedAsync(L"TestSecondaryId"))
            {
                this->pinSecondaryScenarioPinButton().Visibility(winrt::Visibility::Visible);
                this->pinSecondaryScenarioUnpinButton().Visibility(winrt::Visibility::Collapsed);

                if (taskbarManager.IsPinningAllowed())
                {
                    // Pinning is supported and allowed, and the tile is not already pinned. The app can offer the user the option to pin it.
                    this->pinSecondaryScenarioPinButton().IsEnabled(true);
                    this->pinSecondaryScenarioStatusText().Text({});
                }
                else
                {
                    // Pinning is supported, but pinning is not currently allowed (e.g., the system may not be able to show the prompt
                    // at this time). The app should not offer the usre an option to pin now, but it may check again at a later time
                    // (such as when showing the UI with the pin option on it again).
                    this->pinSecondaryScenarioPinButton().IsEnabled(false);
                    this->pinSecondaryScenarioStatusText().Text(L"Pinning is not allowed");
                }
            }
            else
            {
                // Pinning is supported, but the tile is already pinned. The app should not offer the user the option to pin it again.
                this->pinSecondaryScenarioPinButton().Visibility(winrt::Visibility::Collapsed);
                this->pinSecondaryScenarioUnpinButton().Visibility(winrt::Visibility::Visible);
                this->pinSecondaryScenarioUnpinButton().IsEnabled(true);
                this->pinSecondaryScenarioStatusText().Text({});
            }
        }
        else
        {
            // Pinning is not supported on this system, either because the limited access feature couldn't be unlocked
            // or the API indicated desktop callers aren't supported. The app should not offer the user an option to pin.
            this->pinSecondaryScenarioStatusText().Text(L"Pinning is not supported");
            this->pinSecondaryScenarioPinButton().Visibility(Visibility::Collapsed);
        }
    }
}
