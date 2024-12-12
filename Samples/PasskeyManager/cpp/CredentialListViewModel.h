#pragma once
#include "CredentialListViewModel.g.h"
#include "Credential.h"

namespace winrt::PasskeyManager::implementation
{
    struct CredentialListViewModel : CredentialListViewModelT<CredentialListViewModel>
    {
        CredentialListViewModel();

        winrt::Windows::Foundation::Collections::IObservableVector<winrt::PasskeyManager::Credential> credentials();

    private:
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::PasskeyManager::Credential> m_credentials;
    };
}
namespace winrt::PasskeyManager::factory_implementation
{
    struct CredentialListViewModel : CredentialListViewModelT<CredentialListViewModel, implementation::CredentialListViewModel>
    {
    };
}
