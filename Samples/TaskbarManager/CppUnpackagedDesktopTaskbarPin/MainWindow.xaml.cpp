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
    using namespace Microsoft::UI::Xaml;
}

namespace
{
    // Returns the explicitly set app user model ID for the current process.
    wil::unique_cotaskmem_string GetCurrentProcessExplicitAumid()
    {
        wil::unique_cotaskmem_string aumid;
        winrt::check_win32(GetCurrentProcessExplicitAppUserModelID(&aumid));
        return aumid;
    }

    // Returns the path to the app list shortcut for this sample app.
    std::wstring GetAppListShortcutPath()
    {
        wil::unique_cotaskmem_string appListPath;
        winrt::check_hresult(SHGetKnownFolderPath(FOLDERID_Programs, KF_FLAG_DEFAULT, nullptr, &appListPath));
        return (std::wstring(appListPath.get()) + L"\\CppUnpackagedDesktopTaskbarPin.lnk");
    }

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

namespace winrt::CppUnpackagedDesktopTaskbarPin::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();

        // Usage of this sample requires that you have an assigned LAF token for this functionality and have
        // updated both LafData.h and CppUnpackagedDesktopTaskbarPin.rc. See README.md for details.
        if (!c_lafToken.empty() && ::FindResourceEx(nullptr, L"LimitedAccessFeature", L"Identity", 0))
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

    // Create a shortcut in the appropriate location to give this app an entry in the Start app list. This is
    // required for the app to be allowed to request pinning of itself. Update the status text with the result.
    // NOTE: Typically, this entry would be created by your apps installer.
    void MainWindow::AddAppListEntryButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&) try
    {
        // https://learn.microsoft.com/en-us/windows/win32/shell/how-to-add-shortcuts-to-the-start-menu
        // https://learn.microsoft.com/en-us/windows/win32/shell/appids

        auto link = wil::CoCreateInstance<ShellLink, IShellLink>();
        winrt::check_hresult(link->SetPath(wil::QueryFullProcessImageNameW().get()));
        winrt::check_hresult(link->SetDescription(L"C++ Unpackaged Desktop Taskbar Pinning App"));

        wil::unique_prop_variant appIdPropVar;
        winrt::check_hresult(InitPropVariantFromString(GetCurrentProcessExplicitAumid().get(), &appIdPropVar));

        auto propertyStore = link.query<IPropertyStore>();
        winrt::check_hresult(propertyStore->SetValue(INIT_PKEY_AppUserModel_ID, appIdPropVar));
        winrt::check_hresult(propertyStore->Commit());

        auto persistFile = link.query<IPersistFile>();
        winrt::check_hresult(persistFile->Save(GetAppListShortcutPath().c_str(), TRUE));

        this->appListEntryStatusText().Text(L"Entry added");
    }
    catch (const winrt::hresult_error& e)
    {
        this->appListEntryStatusText().Text(e.message());
    }

    // Removes the app list shortcut for this sample app.
    void MainWindow::RemoveAppListEntryButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&) try
    {
        winrt::check_bool(DeleteFileW(GetAppListShortcutPath().c_str()));
        this->appListEntryStatusText().Text(L"Entry removed");
    }
    catch (const winrt::hresult_error& e)
    {
        this->appListEntryStatusText().Text(e.message());
    }
}
