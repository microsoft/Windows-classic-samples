// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once

#include "MainWindow.g.h"

namespace winrt::AppTabsIntegration::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow(AppTabsIntegration::TabViewModel const& initialTab = nullptr);

        void TabView_AddTabClicked(Microsoft::UI::Xaml::Controls::TabView const& sender, Windows::Foundation::IInspectable const& args);
        void TabView_CloseRequested(Microsoft::UI::Xaml::Controls::TabView const& sender, Microsoft::UI::Xaml::Controls::TabViewTabCloseRequestedEventArgs const& args);
        void TabView_SelectionChanged(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& args);
        void TabView_TabDroppedOutside(Microsoft::UI::Xaml::Controls::TabView const& sender, Microsoft::UI::Xaml::Controls::TabViewTabDroppedOutsideEventArgs const& args);
    private:
        winrt::fire_and_forget TabSwitchRequested(winrt::Windows::UI::Shell::WindowTabManager const&, winrt::Windows::UI::Shell::WindowTabSwitchRequestedEventArgs args);
        winrt::fire_and_forget TabCloseRequested(winrt::Windows::UI::Shell::WindowTabManager const&, winrt::Windows::UI::Shell::WindowTabCloseRequestedEventArgs args);
        winrt::fire_and_forget TabThumbnailRequested(winrt::Windows::UI::Shell::WindowTabManager const&, winrt::Windows::UI::Shell::WindowTabThumbnailRequestedEventArgs args);
        winrt::fire_and_forget TabTearOutRequested(winrt::Windows::UI::Shell::WindowTabManager const&, winrt::Windows::UI::Shell::WindowTabTearOutRequestedEventArgs args);

        void TabCollection_VectorChanged(winrt::Windows::Foundation::Collections::IObservableVector<AppTabsIntegration::TabViewModel> const& sender, Windows::Foundation::Collections::IVectorChangedEventArgs const& args);

        void RemoveTabByItem(AppTabsIntegration::TabViewModel const& tabViewModel);
        AppTabsIntegration::TabViewModel CreateTabViewModel();

        winrt::Windows::Foundation::Collections::IObservableVector<AppTabsIntegration::TabViewModel> m_tabs = winrt::single_threaded_observable_vector<winrt::AppTabsIntegration::TabViewModel>();
        winrt::Microsoft::UI::Dispatching::DispatcherQueue m_dispatcherQueue = this->DispatcherQueue();

        winrt::Windows::UI::Shell::WindowTabManager m_tabManager{ nullptr };
        winrt::Windows::UI::Shell::WindowTabManager::TabSwitchRequested_revoker m_tabSwitchRevoker{};
        winrt::Windows::UI::Shell::WindowTabManager::TabCloseRequested_revoker m_tabCloseRevoker{};
        winrt::Windows::UI::Shell::WindowTabManager::TabThumbnailRequested_revoker m_tabThumbnailRequestedRevoker{};
        winrt::Windows::UI::Shell::WindowTabManager::TabTearOutRequested_revoker m_tabTearOutRequestedRevoker{};
    };
}

namespace winrt::AppTabsIntegration::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
