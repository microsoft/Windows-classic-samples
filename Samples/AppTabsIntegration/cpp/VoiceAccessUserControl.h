#pragma once
#include "VoiceAccessUserControl.g.h"
#include <winrt/Microsoft.UI.Xaml.Data.h>
#include <winrt/Windows.UI.Xaml.Controls.h>

namespace winrt::AppTabsIntegration::implementation
{
    struct VoiceAccessUserControl : VoiceAccessUserControlT<VoiceAccessUserControl>
    {
        VoiceAccessUserControl();

        // Property declarations using correct WinRT/C++/CX syntax
        winrt::hstring Mode() const;
        void Mode(winrt::hstring const& value);

        bool IsVoiceEnabled() const;
        void IsVoiceEnabled(bool value);

        // INotifyPropertyChanged implementation
        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
        {
            return m_propertyChanged.add(handler);
        }

        void PropertyChanged(winrt::event_token token)
        {
            return m_propertyChanged.remove(token);
        }

        // Method declaration for UpdateModeDisplay
        void UpdateModeDisplay();

    private:
        winrt::event<winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
        winrt::hstring m_mode{};
        bool m_isVoiceEnabled{ false };

        template<typename T>
        void NotifyChange(T&& name)
        {
            m_propertyChanged(*this, winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(std::forward<T>(name)));
        }
    };
}

namespace winrt::AppTabsIntegration::factory_implementation
{
    struct VoiceAccessUserControl : VoiceAccessUserControlT<VoiceAccessUserControl, implementation::VoiceAccessUserControl>
    {
    };
}