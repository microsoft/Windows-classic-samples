#pragma once
#include<winrt/Microsoft.UI.Xaml.Controls.h>
#include<winrt/Microsoft.UI.Xaml.Documents.h>
#include<winrt/Microsoft.UI.Xaml.Navigation.h>

#include "GetAssertion.g.h"
#include "CredentialListViewModel.h"

namespace winrt::PasskeyManager::implementation
{
    struct GetAssertion : GetAssertionT<GetAssertion>
    {
        GetAssertion()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
            m_credentialListViewModel = winrt::make<PasskeyManager::implementation::CredentialListViewModel>();
            DataContext(m_credentialListViewModel);
        }

        PasskeyManager::CredentialListViewModel CredentialList();
        Windows::Foundation::IAsyncAction OnNavigatedTo(Microsoft::UI::Xaml::Navigation::NavigationEventArgs);
        void ListView_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void SelectionChanged(IInspectable const& sender, Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& args);

        void Create_Assertion(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void Cancel_Plugin_Action(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);

        void UpdatePasskeyOperationStatus(HRESULT hr);

        void DisableUI()
        {
            createButton().IsEnabled(false);
            cancelButton().IsEnabled(false);
        };

        void EnableUI()
        {
            createButton().IsEnabled(true);
            cancelButton().IsEnabled(true);
        };

    private:
        PasskeyManager::CredentialListViewModel m_credentialListViewModel{ nullptr };
    };
}

namespace winrt::PasskeyManager::factory_implementation
{
    struct GetAssertion : GetAssertionT<GetAssertion, implementation::GetAssertion>
    {
    };
}
