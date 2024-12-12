#pragma once
#include "BitwiseFlagToVisibilityConverter.g.h"

namespace winrt::PasskeyManager::implementation
{
    struct BitwiseFlagToVisibilityConverter : BitwiseFlagToVisibilityConverterT<BitwiseFlagToVisibilityConverter>
    {
        BitwiseFlagToVisibilityConverter()
        {
        }

        winrt::Windows::Foundation::IInspectable Convert(winrt::Windows::Foundation::IInspectable const& value, winrt::Windows::UI::Xaml::Interop::TypeName const& targetType, winrt::Windows::Foundation::IInspectable const& parameter, hstring const& language);
        winrt::Windows::Foundation::IInspectable ConvertBack(winrt::Windows::Foundation::IInspectable const& value, winrt::Windows::UI::Xaml::Interop::TypeName const& targetType, winrt::Windows::Foundation::IInspectable const& parameter, hstring const& language);
    };
}

namespace winrt::PasskeyManager::factory_implementation
{
    struct BitwiseFlagToVisibilityConverter : BitwiseFlagToVisibilityConverterT<BitwiseFlagToVisibilityConverter, implementation::BitwiseFlagToVisibilityConverter>
    {
    };
}
