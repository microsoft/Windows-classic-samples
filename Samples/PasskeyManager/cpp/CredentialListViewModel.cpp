#include "pch.h"
#include "CredentialListViewModel.h"
#include "CredentialListViewModel.g.cpp"

namespace winrt::PasskeyManager::implementation
{
    CredentialListViewModel::CredentialListViewModel()
    {
        m_credentials = winrt::single_threaded_observable_vector<winrt::PasskeyManager::Credential>();
    }
    winrt::Windows::Foundation::Collections::IObservableVector<winrt::PasskeyManager::Credential> CredentialListViewModel::credentials()
    {
        return m_credentials;
    }
}
