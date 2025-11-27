#pragma once

#include <pluginauthenticator.h>
#include "MainPage.g.h"
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Documents.h>
#include "CredentialListViewModel.h"
#include <winrt/Windows.Foundation.h>
#include "Converter/BitwiseFlagToVisibilityConverter.h"
#include <wil\filesystem.h>

namespace winrt {
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Collections;
    using namespace Windows::Storage::Streams;
}

namespace winrt::PasskeyManager::implementation
{
    struct MainPage : MainPageT<MainPage>
    {
        MainPage();
        ~MainPage();

        std::optional<DWORD> m_cookie{};

        PasskeyManager::CredentialListViewModel CredentialList()
        {
            return m_credentialListViewModel;
        }

        winrt::IAsyncAction refreshButton_Click(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
        winrt::IAsyncAction registerPluginButton_Click(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
        winrt::IAsyncAction updatePluginButton_Click(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
        winrt::IAsyncAction unregisterPluginButton_Click(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
        winrt::IAsyncAction addAllPluginCredentials_Click(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
        winrt::IAsyncAction addSelectedCredentials_Click(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::IAsyncAction deleteAllPluginCredentials_Click(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
        winrt::IAsyncAction deleteSelectedPluginCredentials_Click(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
        winrt::IAsyncAction deleteSelectedPluginCredentialsEverywhere_Click(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
        winrt::IAsyncAction clearLogsButton_Click(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
        winrt::IAsyncAction deleteAllLocalCredentials_Click(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
        winrt::IAsyncAction deleteAllCredentials_Click(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
        winrt::IAsyncAction activatePluginButton_Click(IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::IAsyncAction VaultUnlockControl_IsCheckedChanged(winrt::Microsoft::UI::Xaml::Controls::ToggleSplitButton const& sender, winrt::Microsoft::UI::Xaml::Controls::ToggleSplitButtonIsCheckedChangedEventArgs const& args);
        winrt::IAsyncAction TestPasskeyVaultUnlock_Click(IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

        winrt::fire_and_forget UpdateCredentialList();

        winrt::IAsyncAction OnNavigatedTo(Microsoft::UI::Xaml::Navigation::NavigationEventArgs);

        void UpdatePasskeyOperationStatusText(hstring const& statusText)
        {
            textContent().Inlines().InsertAt(0, Microsoft::UI::Xaml::Documents::LineBreak{});
            Microsoft::UI::Xaml::Documents::Run statusTextBlock;
            statusTextBlock.Text(statusText);
            textContent().Inlines().InsertAt(0, statusTextBlock);
        }
        void LogSuccess(const winrt::hstring& input) {
            UpdatePasskeyOperationStatusText(winrt::hstring{ L"SUCCESS: " + input + L"\U00002705"});
        }
        void LogFailure(const winrt::hstring& input, HRESULT hr) {
            std::wstring result = L"FAILED: " + std::wstring(input.c_str()) + L": " + winrt::to_hstring(static_cast<int>(hr)).c_str() + L"\U0000274C";
            UpdatePasskeyOperationStatusText(winrt::hstring{ result });
        }
        void LogInProgress(const winrt::hstring& input) {
            UpdatePasskeyOperationStatusText(winrt::hstring{ input + L"\U000023F3"});
        }
        void LogWarning(const winrt::hstring& input, HRESULT hr = S_OK) {
            if (hr == S_OK)
            {
                UpdatePasskeyOperationStatusText(winrt::hstring{ L"WARNING: " + input + L"\U000026A0"});
                return;
            }
            std::wstring result = L"WARNING: " + std::wstring(input.c_str()) + L": " + winrt::to_hstring(static_cast<int>(hr)).c_str() + L"\U000026A0";
            UpdatePasskeyOperationStatusText(winrt::hstring{ result });
        }
        void UpdatePluginStateTextBlock(AUTHENTICATOR_STATE state);
        winrt::IAsyncAction SelectionChanged(IInspectable const& sender, Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const&);
        winrt::fire_and_forget UpdatePluginEnableState();

        winrt::IAsyncAction vaultLockSwitch_Toggled(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::IAsyncAction silentOperationSwitch_Toggled(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
    private:
        PasskeyManager::CredentialListViewModel m_credentialListViewModel{ nullptr };
        winrt::IMap<winrt::IBuffer, IInspectable> m_selectedCredentialsSet = winrt::single_threaded_map<winrt::IBuffer, IInspectable>();
        wil::unique_registry_watcher m_registryWatcher;
        wil::unique_folder_change_reader_nothrow m_mockCredentialsDBWatcher;

        void UpdateVaultUnlockControlText(bool isLocked);
    };
}

namespace winrt::PasskeyManager::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
