#pragma once
#include "TabViewModel.g.h"

namespace winrt::AppTabsIntegration::implementation
{
    struct TabViewModel : TabViewModelT<TabViewModel>
    {
        TabViewModel(winrt::hstring const& name, winrt::Windows::UI::Shell::WindowTab const& windowTab);

        winrt::hstring Name() const;
        void Name(winrt::hstring const& value);

        winrt::Windows::UI::Color Color() const;
        void Color(winrt::Windows::UI::Color const& value);

        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
        {
            return m_propertyChanged.add(handler);
        }

        void PropertyChanged(winrt::event_token token)
        {
            return m_propertyChanged.remove(token);
        }

        // Methods for converting between TabViewModel and WindowTab.
        static AppTabsIntegration::TabViewModel GetFromWindowTab(winrt::Windows::UI::Shell::WindowTab const& windowTab);
        static winrt::Windows::UI::Shell::WindowTab GetWindowTab(AppTabsIntegration::TabViewModel const& tabViewModel);

    private:
        winrt::event<winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
        winrt::Windows::UI::Shell::WindowTab const m_windowTab;
        winrt::hstring m_name{};
        winrt::Windows::UI::Color m_color;

        template<typename T>
        void NotifyChange(T&& name)
        {
            m_propertyChanged(*this, winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(std::forward<T>(name)));
        }
    };
}
