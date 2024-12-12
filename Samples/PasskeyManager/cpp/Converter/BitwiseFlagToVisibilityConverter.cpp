#include "pch.h"
#include "BitwiseFlagToVisibilityConverter.h"
#include "BitwiseFlagToVisibilityConverter.g.cpp"
#include "Credential.h"

namespace winrt
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Microsoft::UI::Xaml;
}

namespace winrt::PasskeyManager::implementation
{
    winrt::Windows::Foundation::IInspectable BitwiseFlagToVisibilityConverter::Convert(
        winrt::Windows::Foundation::IInspectable const& value,
        winrt::Windows::UI::Xaml::Interop::TypeName const&,
        winrt::Windows::Foundation::IInspectable const& parameter,
        hstring const&)
    {
        auto flagName = unbox_value<hstring>(parameter);
        auto options = winrt::unbox_value<CredentialOptionFlags>(value);
        if (flagName == L"MetadataStale")
        {
            return winrt::box_value((options & CredentialOptionFlags::MetadataValid) == CredentialOptionFlags::MetadataValid ? Visibility::Collapsed : Visibility::Visible);
        }
        else if (flagName == L"Success")
        {
            return winrt::box_value((options & CredentialOptionFlags::OperationSucceeded) == CredentialOptionFlags::OperationSucceeded ? Visibility::Visible : Visibility::Collapsed);
        }
        else if (flagName == L"Failed")
        {
            return winrt::box_value((options & CredentialOptionFlags::OperationFailed) == CredentialOptionFlags::OperationFailed ? Visibility::Visible : Visibility::Collapsed);
        }
        else if (flagName == L"AutofillCapable")
        {
            return winrt::box_value((options & CredentialOptionFlags::AutofillCapable) == CredentialOptionFlags::AutofillCapable ? Visibility::Visible : Visibility::Collapsed);
        }
        else
        {
            return winrt::box_value(Visibility::Collapsed);
        }
    }

    winrt::Windows::Foundation::IInspectable BitwiseFlagToVisibilityConverter::ConvertBack(
        winrt::Windows::Foundation::IInspectable const&,
        winrt::Windows::UI::Xaml::Interop::TypeName const&,
        winrt::Windows::Foundation::IInspectable const&,
        hstring const&)
    {
        throw winrt::hresult_not_implemented();
    }
}
