#include "pch.h"
#include "MainPage.xaml.h"
#if __has_include("MainPage.g.cpp")
#include "MainPage.g.cpp"
#include "App.xaml.h"
#include <ncrypt.h>
#include "Credential.h"
#endif
#include "PluginManagement/PluginRegistrationManager.h"
#include "PluginManagement/PluginCredentialManager.h"
#include "PluginAuthenticator/PluginAuthenticatorImpl.h"
#include <future>
#include <coroutine>
#include <DispatcherQueue.h>
#include <winrt/Microsoft.ui.interop.h>
#include <winrt/Microsoft.UI.Content.h>

namespace winrt {
    using namespace winrt::Microsoft::UI::Xaml;
}

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace {

    void CALLBACK WebAuthNStatusChangeCallback(void* context)
    {
        auto mainPage = static_cast<winrt::PasskeyManager::implementation::MainPage*>(context);
        if (mainPage)
        {
            mainPage->DispatcherQueue().TryEnqueue([mainPage]()
            {
                mainPage->UpdatePluginEnableState();
            });
        }
    }

    DWORD RegisterWebAuthNStatusChangeCallback(void* context)
    {
        auto app = winrt::Microsoft::UI::Xaml::Application::Current().as<winrt::PasskeyManager::implementation::App>();

        DWORD cookie{};
        THROW_IF_FAILED(WebAuthNPluginRegisterStatusChangeCallback(
            &WebAuthNStatusChangeCallback,
            context,
            contosoplugin_guid,
            &cookie));
        return cookie;
    }

    DWORD UnregisterWebAuthNStatusChangeCallback()
    {
        auto app = winrt::Microsoft::UI::Xaml::Application::Current().as<winrt::PasskeyManager::implementation::App>();

        DWORD cookie{};
        THROW_IF_FAILED(WebAuthNPluginUnregisterStatusChangeCallback(&cookie));
        return cookie;
    }
}

namespace winrt::PasskeyManager::implementation
{
    winrt::fire_and_forget MainPage::UpdatePluginEnableState()
    {
        winrt::apartment_context ui_thread;

        co_await winrt::resume_background();
        auto hr = PluginRegistrationManager::getInstance().RefreshPluginState();
        auto pluginState = PluginRegistrationManager::getInstance().GetPluginState();
        bool vaultLocked = PluginCredentialManager::getInstance().GetVaultLock();
        bool silentOperation = PluginCredentialManager::getInstance().GetSilentOperation();
        VaultUnlockMethod vaultUnlockMethod = PluginCredentialManager::getInstance().GetVaultUnlockMethod();

        co_await ui_thread;
        VaultUnlockControl().IsChecked(vaultLocked);
        UpdateVaultUnlockControlText(vaultLocked);
        vaultLockSwitch().IsOn(vaultUnlockMethod == VaultUnlockMethod::Passkey);
        silentOperationSwitch().IsOn(silentOperation);
        if (FAILED(hr))
        {
            pluginStateRun().Text(L"Not Registered");
            auto resources = Application::Current().Resources();
            auto neutralBrush = resources.Lookup(winrt::box_value(L"SystemFillColorNeutralBrush")).as<winrt::Microsoft::UI::Xaml::Media::SolidColorBrush>();
            pluginStateRun().Foreground(neutralBrush);
            registerPluginButton().IsEnabled(true);
            updatePluginButton().IsEnabled(false);
            unregisterPluginButton().IsEnabled(false);
            activatePluginButton().IsEnabled(false);
        }
        else
        {
            registerPluginButton().IsEnabled(false);
            updatePluginButton().IsEnabled(true);
            unregisterPluginButton().IsEnabled(true);
            activatePluginButton().IsEnabled(pluginState != AuthenticatorState_Enabled);
            UpdatePluginStateTextBlock(pluginState);
        }
        co_return;
    }

    winrt::IAsyncAction MainPage::vaultLockSwitch_Toggled(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        auto toggleSwitch = sender.as<Microsoft::UI::Xaml::Controls::ToggleSwitch>();
        bool toggleSwitchState = toggleSwitch.IsOn();

        com_ptr<App> curApp = winrt::Microsoft::UI::Xaml::Application::Current().as<App>();
        HWND hwnd = curApp->GetNativeWindowHandle();

        auto weakThis = get_weak();
        co_await winrt::resume_background();
        auto unlockMethod = toggleSwitchState ? VaultUnlockMethod::Passkey : VaultUnlockMethod::Consent;
        auto hr = PluginCredentialManager::getInstance().SetVaultUnlockMethod(unlockMethod);

        co_await wil::resume_foreground(DispatcherQueue());
        auto self = weakThis.get();
        if (FAILED(hr))
        {
            toggleSwitch.IsOn(!toggleSwitchState);
            if (self)
            {
                self->LogFailure(L"Failed to change 'Vault Unlock Control'", hr);
            }
        }
        else if (self)
        {
            self->LogSuccess(L"Changed 'Vault Unlock Control Method'");
        }

        if (unlockMethod == VaultUnlockMethod::Passkey)
        {
            weakThis = get_weak();
            co_await winrt::resume_background();
            hr = PluginRegistrationManager::getInstance().CreateVaultPasskey(hwnd);

            co_await wil::resume_foreground(DispatcherQueue());
            self = weakThis.get();
            if (SUCCEEDED(hr) || hr == NTE_EXISTS)
            {
                if (self)
                {
                    if (hr == NTE_EXISTS)
                    {
                        self->LogSuccess(L"Vault Unlock passkey already exists");
                    }
                    else
                    {
                        self->LogSuccess(L"Created passkey for Vault Unlock");
                    }
                }
            }
            else
            {
                toggleSwitch.IsOn(false);
                if (self)
                {
                    if (hr == NTE_USER_CANCELLED || hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))
                    {
                        self->LogWarning(L"Passkey registration cancelled", hr);
                    }
                    else
                    {
                        self->LogFailure(L"Failed to register passkey", hr);
                    }

                    if (hr == NTE_NOT_SUPPORTED)
                    {
                        self->LogWarning(L"Likely the authenticator chosen does not suppport PRF. This means the passkey was created in the authenticator, but not registered in Contoso. Delete it to try again.");
                    }
                }
            }
        }
        co_return;
    }

    winrt::IAsyncAction MainPage::silentOperationSwitch_Toggled(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        auto toggleSwitch = sender.as<Microsoft::UI::Xaml::Controls::ToggleSwitch>();
        auto toggleSwitchState = toggleSwitch.IsOn();

        auto weakThis = get_weak();

        co_await winrt::resume_background();
        auto hr = PluginCredentialManager::getInstance().SetSilentOperation(toggleSwitchState);
        if (FAILED(hr))
        {
            co_await wil::resume_foreground(DispatcherQueue());
            if (auto self{ weakThis.get() })
            {
                self->LogFailure(L"Failed to change 'Silent Operation'", hr);
            }
        }
        co_return;
    }

    MainPage::MainPage()
    {
        m_credentialListViewModel = winrt::make<PasskeyManager::implementation::CredentialListViewModel>();
        DataContext(m_credentialListViewModel);

        auto weakThis = get_weak();
        m_registryWatcher = wil::make_registry_watcher(
            HKEY_CURRENT_USER,
            c_pluginRegistryPath,
            true,
            [weakThis](wil::RegistryChangeKind changeKind) -> winrt::fire_and_forget {
                bool shouldLogWarning = false;
                if (changeKind == wil::RegistryChangeKind::Modify)
                {
                    auto& credMgr = PluginCredentialManager::getInstance();
                    credMgr.ReloadRegistryValues();
                    if (credMgr.GetVaultLock() && credMgr.GetSilentOperation())
                    {
                        credMgr.SetSilentOperation(false);
                        shouldLogWarning = true;
                    }
                }
                if (auto self{ weakThis.get() })
                {
                    co_await wil::resume_foreground(self->DispatcherQueue());
                    if (shouldLogWarning)
                    {
                        self->LogWarning(L"Vault unlock requires UI", E_NOT_VALID_STATE);
                    }
                    self->UpdatePluginEnableState();
                }
            });
        std::wstring mockDBfilePath;
        PluginCredentialManager::getInstance().GetCredentialStorageFolderPath(mockDBfilePath);
        THROW_IF_FAILED(m_mockCredentialsDBWatcher.create(mockDBfilePath.c_str(),
            true,
            wil::FolderChangeEvents::All,
            [weakThis](wil::FolderChangeEvent, PCWSTR) -> winrt::fire_and_forget {
                PluginCredentialManager::getInstance().ReloadRegistryValues();
                if (auto self{ weakThis.get() })
                {
                    co_await wil::resume_foreground(self->DispatcherQueue());
                    self->UpdatePluginEnableState();
                    self->UpdateCredentialList();
                }
            }));

        m_cookie = RegisterWebAuthNStatusChangeCallback(static_cast<void*>(this));
    }

    MainPage::~MainPage()
    {
        if (m_cookie.has_value())
        {
            m_cookie = UnregisterWebAuthNStatusChangeCallback();
        }
    }

    winrt::IAsyncAction MainPage::refreshButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        UpdatePluginEnableState();
        UpdateCredentialList();
        co_return;
    }

    winrt::fire_and_forget MainPage::UpdateCredentialList()
    {
        m_credentialListViewModel.credentials().Clear();
        auto weakThis = get_weak();
        co_await winrt::resume_background();

        PluginCredentialManager& pluginCredentialManager = PluginCredentialManager::getInstance();
        pluginCredentialManager.ReloadCredentialManager();

        co_await wil::resume_foreground(DispatcherQueue());
        auto credentialViewList = pluginCredentialManager.GetCredentialListViewModel();

        auto self = weakThis.get();
        if (!self)
        {
            co_return;
        }

        if (pluginCredentialManager.IsLocalCredentialMetadataLoaded())
        {
            std::wstring countOfLocalCreds = std::to_wstring(pluginCredentialManager.GetLocalCredentialCount()) + L" passkeys in Local DB";
            self->credsStatsRun1().Text(countOfLocalCreds);
        }
        else
        {
            self->credsStatsRun1().Text(L"Local DB not loaded");
        }

        if (pluginCredentialManager.IsCachedCredentialsMetadataLoaded())
        {
            std::wstring countOfPluginCreds = std::to_wstring(pluginCredentialManager.GetCachedCredentialCount()) + L" passkeys in system Cache";
            self->credsStatsRun2().Text(countOfPluginCreds);
        }
        else
        {
            self->credsStatsRun2().Text(L"Windows Cache Data not loaded");
        }

        self->m_credentialListViewModel.credentials().Clear();
        for (auto& credListItem : credentialViewList)
        {
            self->m_credentialListViewModel.credentials().Append(*credListItem.detach());
        }
        co_return;
    }

    winrt::IAsyncAction MainPage::OnNavigatedTo(Navigation::NavigationEventArgs e)
    {
        UpdatePluginEnableState();
        UpdateCredentialList();
        co_return;
    }

    winrt::IAsyncAction MainPage::unregisterPluginButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        LogInProgress(L"Unregistering plugin...");
        auto weakThis = get_weak();

        if (m_cookie.has_value())
        {
            m_cookie = UnregisterWebAuthNStatusChangeCallback();
        }

        co_await winrt::resume_background();
        HRESULT hr = PluginRegistrationManager::getInstance().UnregisterPlugin();

        co_await wil::resume_foreground(DispatcherQueue());

        auto self = weakThis.get();
        if (!self)
        {
            co_return;
        }

        self->UpdatePluginEnableState();
        if (FAILED(hr))
        {
            self->LogFailure(L"Failed to Unregister plugin: ", hr);
            co_return;
        }
        self->LogSuccess(L"Plugin unregistered");
    }

    winrt::IAsyncAction MainPage::registerPluginButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        LogInProgress(L"Registering plugin...");
        auto weakThis = get_weak();
        co_await winrt::resume_background();
        HRESULT hr = PluginRegistrationManager::getInstance().RegisterPlugin();

        co_await wil::resume_foreground(DispatcherQueue());
        auto self = weakThis.get();
        if (!self)
        {
            co_return;
        }

        self->UpdatePluginEnableState();

        if (FAILED(hr))
        {
            self->LogFailure(L"WebAuthNPluginAddAuthenticator", hr);
            co_return;
        }
        self->LogSuccess(L"Plugin registered");

        m_cookie = RegisterWebAuthNStatusChangeCallback(static_cast<void*>(this));
    }

    winrt::IAsyncAction MainPage::updatePluginButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        LogInProgress(L"Updating plugin...");
        auto weakThis = get_weak();
        co_await winrt::resume_background();
        HRESULT hr = PluginRegistrationManager::getInstance().UpdatePlugin();

        co_await wil::resume_foreground(DispatcherQueue());

        auto self = weakThis.get();
        if (!self)
        {
            co_return;
        }

        self->UpdatePluginEnableState();

        if (FAILED(hr))
        {
            self->LogFailure(L"WebAuthNPluginUpdateAuthenticatorDetails", hr);
            co_return;
        }
        self->LogSuccess(L"Plugin updated");
    }

    winrt::IAsyncAction MainPage::addAllPluginCredentials_Click(IInspectable const&, RoutedEventArgs const&)
    {
        LogInProgress(L"Adding All credentials to windows...");

        auto weakThis = get_weak();
        co_await winrt::resume_background();
        HRESULT hr = PluginCredentialManager::getInstance().AddAllPluginCredentials();

        co_await wil::resume_foreground(DispatcherQueue());

        auto self = weakThis.get();
        if (!self)
        {
            co_return;
        }

        self->UpdateCredentialList();
        if (FAILED(hr))
        {
            self->LogFailure(L"Failed to add credential to system cache: ", hr);
            co_return;
        }
        self->LogSuccess(L"Credentials synced");
        co_return;
    }

    winrt::IAsyncAction MainPage::addSelectedCredentials_Click(IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        LogInProgress(L"Adding selected passkey metadata to system cache...");

        std::vector<std::vector<UINT8>> credentialIdList;
        auto selectedItems = credentialListView().SelectedItems();
        if (selectedItems.Size() == 0)
        {
            LogWarning(L"No credentials selected", E_NOT_SET);
            co_return;
        }

        for (auto item : selectedItems)
        {
            auto credential = item.as<PasskeyManager::implementation::Credential>();
            auto reader = winrt::Windows::Storage::Streams::DataReader::FromBuffer(credential->CredentialId());
            std::vector<UINT8> credentialIdToAdd(reader.UnconsumedBufferLength());
            reader.ReadBytes(credentialIdToAdd);
            credentialIdList.push_back(credentialIdToAdd);
        }

        hstring statusText = L"Adding " + winrt::to_hstring(credentialIdList.size()) + L" selected credentials...";
        UpdatePasskeyOperationStatusText(statusText);

        auto weakThis = get_weak();
        co_await winrt::resume_background();
        HRESULT hr = PluginCredentialManager::getInstance().AddPluginCredentialById(credentialIdList);

        co_await wil::resume_foreground(DispatcherQueue());

        auto self = weakThis.get();
        if (!self)
        {
            co_return;
        }

        self->UpdateCredentialList();
        if (FAILED(hr))
        {
            self->LogFailure(L"Failed to add credentials to system cache", hr);
            co_return;
        }
        self->LogSuccess(L"Selected credentials are added to system cache");
        co_return;
    }

    winrt::IAsyncAction MainPage::deleteAllPluginCredentials_Click(IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        LogInProgress(L"Deleting all credentials stored on this device...");

        auto weakThis = get_weak();
        co_await winrt::resume_background();
        HRESULT hr = PluginCredentialManager::getInstance().DeleteAllPluginCredentials();

        co_await wil::resume_foreground(DispatcherQueue());

        auto self = weakThis.get();
        if (!self)
        {
            co_return;
        }

        self->UpdateCredentialList();
        if (FAILED(hr))
        {
            self->LogFailure(L"Failed to delete credential from system cache", hr);
            co_return;
        }
        self->LogSuccess(L"All credentials deleted from system cache");
        co_return;
    }

    winrt::IAsyncAction MainPage::deleteSelectedPluginCredentials_Click(IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        LogInProgress(L"Deleting selected credentials...");

        // find the list of creds with checkbox checked
        std::vector<std::vector<UINT8>> credentialIdList;
        auto selectedItems = credentialListView().SelectedItems();
        if (selectedItems.Size() == 0)
        {
            LogWarning(L"No credentials selected", E_NOT_SET);
            co_return;
        }

        for (auto item : selectedItems)
        {
            auto credential = item.as<PasskeyManager::implementation::Credential>();
            auto reader = winrt::Windows::Storage::Streams::DataReader::FromBuffer(credential->CredentialId());
            std::vector<UINT8> credentialIdToDelete(reader.UnconsumedBufferLength());
            reader.ReadBytes(credentialIdToDelete);
            credentialIdList.push_back(credentialIdToDelete);
        }

        // update the status block with count of selected creds
        hstring statusText = L"Deleting " + winrt::to_hstring(credentialIdList.size()) + L" selected credentials...";
        UpdatePasskeyOperationStatusText(statusText);

        auto weakThis = get_weak();
        co_await winrt::resume_background();
        HRESULT hr = PluginCredentialManager::getInstance().DeletePluginCredentialById(credentialIdList, false);

        co_await wil::resume_foreground(DispatcherQueue());

        auto self = weakThis.get();
        if (!self)
        {
            co_return;
        }

        self->UpdateCredentialList();
        if (FAILED(hr))
        {
            self->LogFailure(L"Failed to delete credentials from system cache", hr);
            co_return;
        }
        self->LogSuccess(L"Selected credentials deleted from system cache");
        co_return;
    }

    winrt::IAsyncAction MainPage::deleteSelectedPluginCredentialsEverywhere_Click(IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        LogInProgress(L"Deleting selected credentials everywhere...");

        // find the list of creds with checkbox checked
        std::vector<std::vector<UINT8>> credentialIdList;
        auto selectedItems = credentialListView().SelectedItems();
        if (selectedItems.Size() == 0)
        {
            LogWarning(L"No credentials selected", E_NOT_SET);
            co_return;
        }

        for (auto item : selectedItems)
        {
            auto credential = item.as<PasskeyManager::implementation::Credential>();
            auto reader = winrt::Windows::Storage::Streams::DataReader::FromBuffer(credential->CredentialId());
            std::vector<UINT8> credentialIdToDelete(reader.UnconsumedBufferLength());
            reader.ReadBytes(credentialIdToDelete);
            credentialIdList.push_back(credentialIdToDelete);
        }

        // update the status block with count of selected creds
        hstring statusText = winrt::to_hstring(credentialIdList.size()) + L" credentials selected...";
        UpdatePasskeyOperationStatusText(statusText);

        auto weakThis = get_weak();
        co_await winrt::resume_background();
        HRESULT hr = PluginCredentialManager::getInstance().DeletePluginCredentialById(credentialIdList, true);

        co_await wil::resume_foreground(DispatcherQueue());

        auto self = weakThis.get();
        if (!self)
        {
            co_return;
        }

        self->UpdateCredentialList();
        if (FAILED(hr))
        {
            self->LogFailure(L"Failed to delete credentials", hr);
            co_return;
        }
        self->LogSuccess(L"Selected credentials deleted everywhere");
        co_return;
    }

    winrt::IAsyncAction MainPage::clearLogsButton_Click(IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        textContent().Inlines().Clear();
        co_return;
    }

    winrt::IAsyncAction MainPage::deleteAllLocalCredentials_Click(IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        LogInProgress(L"Deleting all local credentials...");

        auto weakThis = get_weak();
        co_await winrt::resume_background();

        bool resetResult = PluginCredentialManager::getInstance().ResetLocalCredentialsStore();

        co_await wil::resume_foreground(DispatcherQueue());

        auto self = weakThis.get();
        if (!self)
        {
            co_return;
        }

        self->UpdateCredentialList();
        if (resetResult)
        {
            self->LogFailure(L"Failed to delete all local credentials", E_FAIL);
            co_return;
        }
        self->LogSuccess(L"All local credentials deleted");
        co_return;
    }

    winrt::IAsyncAction MainPage::deleteAllCredentials_Click(IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        LogInProgress(L"Deleting all credentials stored on this device and cache...");
        auto weakThis = get_weak();
        co_await winrt::resume_background();
        auto& credManager = PluginCredentialManager::getInstance();
        HRESULT hr = credManager.DeleteAllPluginCredentials();
        bool resetResult = credManager.ResetLocalCredentialsStore();
        co_await wil::resume_foreground(DispatcherQueue());

        auto self = weakThis.get();
        if (!self)
        {
            co_return;
        }

        self->UpdateCredentialList();
        if (FAILED(hr) || !resetResult)
        {
            self->LogFailure(L"Failed to delete all credentials", hr);
            co_return;
        }
        self->LogSuccess(L"All credentials deleted");
    }

    void MainPage::UpdatePluginStateTextBlock(AUTHENTICATOR_STATE state)
    {
        auto resources = Application::Current().Resources();
        auto successBrush = resources.Lookup(winrt::box_value(L"SystemFillColorSuccessBrush")).as<winrt::Microsoft::UI::Xaml::Media::SolidColorBrush>();
        auto criticalBrush = resources.Lookup(winrt::box_value(L"SystemFillColorCriticalBrush")).as<winrt::Microsoft::UI::Xaml::Media::SolidColorBrush>();
        auto cautionBrush = resources.Lookup(winrt::box_value(L"SystemFillColorCautionBrush")).as<winrt::Microsoft::UI::Xaml::Media::SolidColorBrush>();

        switch (state)
        {
        case AuthenticatorState_Enabled:
            pluginStateRun().Text(L"Enabled");
            pluginStateRun().Foreground(successBrush);
            break;
        case AuthenticatorState_Disabled:
            pluginStateRun().Text(L"Disabled");
            pluginStateRun().Foreground(criticalBrush);
            break;
        default:
            pluginStateRun().Text(L"Unknown");
            pluginStateRun().Foreground(cautionBrush);
            break;
        }
    }

    winrt::IAsyncAction MainPage::SelectionChanged(IInspectable const& sender, Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const&)
    {
        Microsoft::UI::Xaml::Controls::ListView listView = sender.as<Microsoft::UI::Xaml::Controls::ListView>();
        auto selected = listView.SelectedItems().Size() > 0;
        selectedAddButton().IsEnabled(selected);
        deleteSelectedCacheButton().IsEnabled(selected);
        deleteSelectedLocalButton().IsEnabled(selected);
        co_return;
    }

    winrt::IAsyncAction MainPage::activatePluginButton_Click(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        // URI ms-settings:passkeys-advancedoptions to navigate to the page on Settings app where the users can enable the plugin
        auto uri = Windows::Foundation::Uri(L"ms-settings:passkeys-advancedoptions");
        co_await Windows::System::Launcher::LaunchUriAsync(uri);
        co_return;
    }

    void MainPage::UpdateVaultUnlockControlText(bool isLocked)
    {
        if (isLocked)
        {
            VaultUnlockControl().Content(box_value(L"Vault Locked"));
        }
        else
        {
            VaultUnlockControl().Content(box_value(L"Vault Unlocked"));
        }
    }

    winrt::IAsyncAction MainPage::VaultUnlockControl_IsCheckedChanged(winrt::Microsoft::UI::Xaml::Controls::ToggleSplitButton const& sender, winrt::Microsoft::UI::Xaml::Controls::ToggleSplitButtonIsCheckedChangedEventArgs const& args)
    {
        // Capture the value we need before switching context
        bool toggleSplitState = sender.IsChecked();

        auto hr = PluginCredentialManager::getInstance().SetVaultLock(toggleSplitState);

        if (FAILED(hr))
        {
            LogFailure(L"Failed to change 'Simulate Vault Unlock'", hr);
        }

        UpdateVaultUnlockControlText(toggleSplitState);

        co_return;
    }

}
