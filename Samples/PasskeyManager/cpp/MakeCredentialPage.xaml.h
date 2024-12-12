#pragma once
#include<winrt/Microsoft.UI.Xaml.Controls.h>
#include<winrt/Microsoft.UI.Xaml.Documents.h>
#include<winrt/Microsoft.UI.Xaml.Navigation.h>
#include "MakeCredentialPage.g.h"

namespace winrt::PasskeyManager::implementation
{
    struct MakeCredentialPage : MakeCredentialPageT<MakeCredentialPage>
    {
        MakeCredentialPage()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }

        void Create_Credential(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void Cancel_Plugin_Action(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);

        Windows::Foundation::IAsyncAction OnNavigatedTo(Microsoft::UI::Xaml::Navigation::NavigationEventArgs);

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
    };
}

namespace winrt::PasskeyManager::factory_implementation
{
    struct MakeCredentialPage : MakeCredentialPageT<MakeCredentialPage, implementation::MakeCredentialPage>
    {
    };
}
