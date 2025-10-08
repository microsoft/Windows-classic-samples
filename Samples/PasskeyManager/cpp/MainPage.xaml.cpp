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
#include <future>
#include <coroutine>
#include <DispatcherQueue.h>

namespace winrt {
    using namespace winrt::Microsoft::UI::Xaml;
}

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

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
        co_await ui_thread;

        if (FAILED(hr))
        {
            pluginStateRun().Text(L"Not Registered");
            auto resources = Application::Current().Resources();
            auto neutralBrush = resources.Lookup(winrt::box_value(L"SystemFillColorNeutralBrush")).as<winrt::Microsoft::UI::Xaml::Media::SolidColorBrush>();
            pluginStateRun().Foreground(neutralBrush);
            registerPluginButton().IsEnabled(true);
            unregisterPluginButton().IsEnabled(false);
        }
        else
        {
            registerPluginButton().IsEnabled(false);
            unregisterPluginButton().IsEnabled(true);
            UpdatePluginStateTextBlock(pluginState);
            vaultLockSwitch().IsOn(vaultLocked);
            silentOperationSwitch().IsOn(silentOperation);
        }
        co_return;
    }

    winrt::IAsyncAction MainPage::vaultLockSwitch_Toggled(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        auto toggleSwitch = sender.as<Microsoft::UI::Xaml::Controls::ToggleSwitch>();
        bool toggleSwitchState = toggleSwitch.IsOn();
        winrt::apartment_context ui_thread;
        co_await winrt::resume_background();
        auto hr = PluginCredentialManager::getInstance().SetVaultLock(toggleSwitchState);
        if (FAILED(hr))
        {
            co_await ui_thread;
            LogFailure(L"Failed to change 'Simulate Vault Unlock'", hr);
        }
        co_return;
    }

    winrt::IAsyncAction MainPage::silentOperationSwitch_Toggled(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        winrt::apartment_context ui_thread;
        auto toggleSwitch = sender.as<Microsoft::UI::Xaml::Controls::ToggleSwitch>();
        auto toggleSwitchState = toggleSwitch.IsOn();
        co_await winrt::resume_background();
        auto hr = PluginCredentialManager::getInstance().SetSilentOperation(toggleSwitchState);
        if (FAILED(hr))
        {
            co_await ui_thread;
            LogFailure(L"Failed to change 'Minimize UI'", hr);
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
            [context = winrt::apartment_context{}, weakThis](wil::RegistryChangeKind changeKind) -> winrt::fire_and_forget {
                auto weakThisCopy = weakThis;
                auto contextCopy = context;
                co_await contextCopy;
                if (changeKind == wil::RegistryChangeKind::Modify)
                {
                    PluginCredentialManager::getInstance().ReloadRegistryValues();
                }
                if (auto self{ weakThisCopy.get() })
                {
                    self->UpdatePluginEnableState();
                }
            });
        std::wstring mockDBfilePath;
        PluginCredentialManager::getInstance().GetCredentialStorageFolderPath(mockDBfilePath);
        THROW_IF_FAILED(m_mockCredentialsDBWatcher.create(mockDBfilePath.c_str(),
            true,
            wil::FolderChangeEvents::All,
            [context = winrt::apartment_context{}, weakThis](wil::FolderChangeEvent, PCWSTR) -> winrt::fire_and_forget {
                auto weakThisCopy = weakThis;
                auto contextCopy = context;
                co_await contextCopy;
                PluginCredentialManager::getInstance().ReloadRegistryValues();
                if (auto self{ weakThisCopy.get() })
                {
                    self->UpdatePluginEnableState();
                    self->UpdateCredentialList();
                }
            }));
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

        winrt::apartment_context ui_thread;
        co_await winrt::resume_background();

        PluginCredentialManager& pluginCredentialManager = PluginCredentialManager::getInstance();
        pluginCredentialManager.ReloadCredentialManager();

        co_await ui_thread;
        auto credentialViewList = pluginCredentialManager.GetCredentialListViewModel();

        if (pluginCredentialManager.IsLocalCredentialMetadataLoaded())
        {
            std::wstring countOfLocalCreds = std::to_wstring(pluginCredentialManager.GetLocalCredentialCount()) + L" passkeys in Local DB";
            credsStatsRun1().Text(countOfLocalCreds);
        }
        else
        {
            credsStatsRun1().Text(L"Local DB not loaded");
        }

        if (pluginCredentialManager.IsCachedCredentialsMetadataLoaded())
        {
            std::wstring countOfPluginCreds = std::to_wstring(pluginCredentialManager.GetCachedCredentialCount()) + L" passkeys in system Cache";
            credsStatsRun2().Text(countOfPluginCreds);
        }
        else
        {
            credsStatsRun2().Text(L"Windows Cache Data not loaded");
        }

        m_credentialListViewModel.credentials().Clear();
        for (auto credListItem : credentialViewList)
        {
            m_credentialListViewModel.credentials().Append(*credListItem.detach());
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

        winrt::apartment_context ui_thread;
        co_await winrt::resume_background();
        HRESULT hr = PluginRegistrationManager::getInstance().UnregisterPlugin();

        co_await ui_thread;

        UpdatePluginEnableState();
        if (FAILED(hr))
        {
            LogFailure(L"Failed to Unregister plugin: ", hr);
            co_return;
        }
        LogSuccess(L"Plugin unregistered");
    }

    winrt::IAsyncAction MainPage::registerPluginButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        LogInProgress(L"Registering plugin...");
        winrt::apartment_context ui_thread;
        co_await winrt::resume_background();
        HRESULT hr = PluginRegistrationManager::getInstance().RegisterPlugin();

        co_await ui_thread;

        UpdatePluginEnableState();

        if (FAILED(hr))
        {
            LogFailure(L"WebAuthNPluginAddAuthenticator", hr);
            co_return;
        }
        LogSuccess(L"Plugin registered");
    }

    winrt::IAsyncAction MainPage::addAllPluginCredentials_Click(IInspectable const&, RoutedEventArgs const&)
    {
        LogInProgress(L"Adding All credentials to windows...");

        winrt::apartment_context ui_thread;
        co_await winrt::resume_background();
        HRESULT hr = PluginCredentialManager::getInstance().AddAllPluginCredentials();

        co_await ui_thread;

        UpdateCredentialList();
        if (FAILED(hr))
        {
            LogFailure(L"Failed to add credential to system cache: ", hr);
            co_return;
        }
        LogSuccess(L"Credentials synced");
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

        winrt::apartment_context ui_thread;
        co_await winrt::resume_background();
        HRESULT hr = PluginCredentialManager::getInstance().AddPluginCredentialById(credentialIdList);

        co_await ui_thread;

        UpdateCredentialList();
        if (FAILED(hr))
        {
            LogFailure(L"Failed to add credentials to system cache", hr);
            co_return;
        }
        LogSuccess(L"Selected credentials are added to system cache");
        co_return;
    }

    winrt::IAsyncAction MainPage::deleteAllPluginCredentials_Click(IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        LogInProgress(L"Deleting all credentials stored on this device...");

        winrt::apartment_context ui_thread;
        co_await winrt::resume_background();
        HRESULT hr = PluginCredentialManager::getInstance().DeleteAllPluginCredentials();

        co_await ui_thread;

        UpdateCredentialList();
        if (FAILED(hr))
        {
            LogFailure(L"Failed to delete credential from system cache", hr);
            co_return;
        }
        LogSuccess(L"All credentials deleted from system cache");
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

        winrt::apartment_context ui_thread;
        co_await winrt::resume_background();
        HRESULT hr = PluginCredentialManager::getInstance().DeletePluginCredentialById(credentialIdList, false);

        co_await ui_thread;

        UpdateCredentialList();
        if (FAILED(hr))
        {
            LogFailure(L"Failed to delete credentials from system cache", hr);
            co_return;
        }
        LogSuccess(L"Selected credentials deleted from system cache");
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

        winrt::apartment_context ui_thread;
        co_await winrt::resume_background();
        HRESULT hr = PluginCredentialManager::getInstance().DeletePluginCredentialById(credentialIdList, true);

        co_await ui_thread;

        UpdateCredentialList();
        if (FAILED(hr))
        {
            LogFailure(L"Failed to delete credentials", hr);
            co_return;
        }
        LogSuccess(L"Selected credentials deleted everywhere");
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

        winrt::apartment_context ui_thread;
        co_await winrt::resume_background();

        bool resetResult = PluginCredentialManager::getInstance().ResetLocalCredentialsStore();

        co_await ui_thread;
        UpdateCredentialList();
        if (resetResult)
        {
            LogFailure(L"Failed to delete all local credentials", E_FAIL);
            co_return;
        }
        LogSuccess(L"All local credentials deleted");
        co_return;
    }

    winrt::IAsyncAction MainPage::deleteAllCredentials_Click(IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        LogInProgress(L"Deleting all credentials stored on this device and cache...");
        winrt::apartment_context ui_thread;
        co_await winrt::resume_background();
        auto& credManager = PluginCredentialManager::getInstance();
        HRESULT hr = credManager.DeleteAllPluginCredentials();
        bool resetResult = credManager.ResetLocalCredentialsStore();
        co_await ui_thread;

        UpdateCredentialList();
        if (FAILED(hr) || !resetResult)
        {
            LogFailure(L"Failed to delete all credentials", hr);
            co_return;
        }
        LogSuccess(L"All credentials deleted");
    }

    void MainPage::UpdatePluginStateTextBlock(PLUGIN_AUTHENTICATOR_STATE state)
    {
        auto resources = Application::Current().Resources();
        auto successBrush = resources.Lookup(winrt::box_value(L"SystemFillColorSuccessBrush")).as<winrt::Microsoft::UI::Xaml::Media::SolidColorBrush>();
        auto criticalBrush = resources.Lookup(winrt::box_value(L"SystemFillColorCriticalBrush")).as<winrt::Microsoft::UI::Xaml::Media::SolidColorBrush>();
        auto cautionBrush = resources.Lookup(winrt::box_value(L"SystemFillColorCautionBrush")).as<winrt::Microsoft::UI::Xaml::Media::SolidColorBrush>();

        switch (state)
        {
        case PluginAuthenticatorState_Enabled:
            pluginStateRun().Text(L"Enabled");
            pluginStateRun().Foreground(successBrush);
            break;
        case PluginAuthenticatorState_Disabled:
            pluginStateRun().Text(L"Disabled");
            pluginStateRun().Foreground(criticalBrush);
            break;
        case PluginAuthenticatorState_Unknown:
            [[fallthrough]];
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
}
