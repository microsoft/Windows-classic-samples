#pragma once
#include "Credential.g.h"

namespace winrt::PasskeyManager::implementation
{
    struct Credential : CredentialT<Credential, winrt::non_agile>
    {
        Credential(hstring const& userName, hstring const& rpName, const Windows::Storage::Streams::IBuffer& credentialId);
        Credential(hstring const& userName, hstring const& rpName, const Windows::Storage::Streams::IBuffer& credentialId,
            CredentialOptionFlags options);

        hstring UserName();
        hstring RpName();
        Windows::Storage::Streams::IBuffer CredentialId();
        void CredentialOptions(CredentialOptionFlags value);
        CredentialOptionFlags CredentialOptions();

    private:
        Credential() = default;
        hstring m_userName;
        hstring m_rpName;
        Windows::Storage::Streams::IBuffer m_credentialId;
        CredentialOptionFlags m_options = CredentialOptionFlags::None;
    };
}
namespace winrt::PasskeyManager::factory_implementation
{
    struct Credential : CredentialT<Credential, implementation::Credential>
    {
    };
}
