#pragma once

#include "MainWindow.g.h"
#include "TransferTargetItem.g.h"

namespace winrt::Sample::implementation
{
    struct TransferTargetItem : TransferTargetItemT<TransferTargetItem>
    {
    public:
        TransferTargetItem(
            winrt::Windows::ApplicationModel::DataTransfer::TransferTargetWatcher const& watcher,
            winrt::Windows::ApplicationModel::DataTransfer::TransferTarget const& target)
            : m_watcher(watcher), m_target(target), m_id(target.Id())
        {
        }

        auto Label() { return m_target.Label(); }
        winrt::Microsoft::UI::Xaml::Media::ImageSource Logo();
        bool IsEnabled() { return m_target.IsEnabled(); }

        winrt::fire_and_forget Invoke(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& /* e */);

        auto PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& value)
        {
            return m_propertyChanged.add(value);
        }

        void PropertyChanged(winrt::event_token const& token)
        {
            m_propertyChanged.remove(token);
        }

        auto const& Target() { return m_target; }
        auto const& Id() { return m_id; }
        void Update(winrt::Windows::ApplicationModel::DataTransfer::TransferTarget const& target);

        auto RaisePropertyChanged(std::wstring_view name)
        {
            return m_propertyChanged(*this, winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{ name });
        }

    private:
        winrt::Windows::ApplicationModel::DataTransfer::TransferTargetWatcher m_watcher;
        winrt::Windows::ApplicationModel::DataTransfer::TransferTarget m_target;
        winrt::hstring const m_id;
        winrt::event<winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
        winrt::Microsoft::UI::Xaml::Media::Imaging::BitmapImage m_logo{ nullptr };
        bool m_forceFallback{ false };

        winrt::fire_and_forget StartLoadLogo();
        winrt::fire_and_forget ReportLogoFailed();
    };

    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow() = default;
        void InitializeComponent();

        // Members used by binding
        auto DiscoveredTargets()
        {
            return m_discoveredTargets.as<winrt::Windows::Foundation::Collections::IObservableVector<winrt::Windows::Foundation::IInspectable>>();
        }

        static double EnabledOpacity(bool isEnabled) { return isEnabled ? 1.0 : 0.50; }
        static bool IsNull(winrt::Windows::Foundation::IInspectable const&value) { return value == nullptr; }
        static bool IsNotNull(winrt::Windows::Foundation::IInspectable const& value) { return value != nullptr; }

        // Event handlers
        winrt::fire_and_forget SelectFilesButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void DiscoverTargetsButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void StopDiscoveryButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ReconfigureButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        // Store selected files for later use with dataPackage
        std::vector<winrt::Windows::Storage::IStorageItem> m_selectedFiles;

        // Observable vector of found targets (in the form of TransferTargetItem objects)
        // for data binding to the ListView
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::Sample::TransferTargetItem> m_discoveredTargets =
            winrt::single_threaded_observable_vector<winrt::Sample::TransferTargetItem>();

        // TransferTargetWatcher members
        winrt::Windows::ApplicationModel::DataTransfer::TransferTargetWatcher m_watcher{ nullptr };
        bool m_watcherRunning{ false };

        // TransferTargetWatcher event handlers
        winrt::fire_and_forget OnTargetAdded(winrt::Windows::ApplicationModel::DataTransfer::TransferTargetWatcher sender, winrt::Windows::ApplicationModel::DataTransfer::TransferTargetChangedEventArgs const& args);
        winrt::fire_and_forget OnTargetRemoved(winrt::Windows::ApplicationModel::DataTransfer::TransferTargetWatcher sender, winrt::Windows::ApplicationModel::DataTransfer::TransferTargetChangedEventArgs const& args);
        winrt::fire_and_forget OnTargetUpdated(winrt::Windows::ApplicationModel::DataTransfer::TransferTargetWatcher sender, winrt::Windows::ApplicationModel::DataTransfer::TransferTargetChangedEventArgs const& args);

        // Helper methods
        auto FindTargetItemById(winrt::hstring const& id);
    };
}

namespace winrt::Sample::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
