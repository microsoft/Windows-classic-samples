#include "pch.h"
#include "MainWindow.xaml.h"
#include "MainWindow.g.cpp"


namespace winrt
{
    using namespace winrt::Microsoft::UI::Dispatching;
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Controls;
    using namespace winrt::Microsoft::UI::Xaml::Media;
    using namespace winrt::Microsoft::UI::Xaml::Media::Imaging;
    using namespace winrt::Windows::ApplicationModel::DataTransfer;
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Collections;
    using namespace winrt::Windows::Foundation::Metadata;
    using namespace winrt::Windows::Storage;
    using namespace winrt::Windows::Storage::Pickers;
    using namespace winrt::Windows::Storage::Streams;
    using namespace winrt::Windows::UI::Core;
}

#pragma region Checking for Windows SDK version 10.0.26100.7175 or higher
namespace winrt::Windows::ApplicationModel::DataTransfer
{
    struct TransferTarget;
}

template<typename, typename = void> constexpr bool is_type_complete_v = false;
template<typename T> constexpr bool is_type_complete_v<T, std::void_t<decltype(sizeof(T))>> = true;

static_assert(is_type_complete_v<struct winrt::TransferTarget>, "This sample requires Windows SDK version 10.0.26100.7175 or higher");
#pragma endregion

namespace winrt::Sample::implementation
{
    winrt::Windows::UI::WindowId WindowIdFromElement(winrt::UIElement const& element)
    {
        return { element.XamlRoot().ContentIslandEnvironment().AppWindowId().Value };
    }

    winrt::ImageSource TransferTargetItem::Logo()
    {
        if (m_forceFallback)
        {
            return nullptr;
        }

        if (!m_logo)
        {
            StartLoadLogo();
        }
        return m_logo;
    }

    winrt::fire_and_forget TransferTargetItem::ReportLogoFailed()
    {
        // When the bitmap logo file fails to load, remember that it failed
        // and raise the logo changed events to update the UI with the fallback logo.
        auto strongThis = get_strong();

        m_forceFallback = true;

        // Defer raising the event because we don't want to raise it while in the middle of a bind operation.
        co_await wil::resume_foreground(winrt::DispatcherQueue::GetForCurrentThread());
        RaisePropertyChanged(L"Logo");
    }

    winrt::fire_and_forget TransferTargetItem::StartLoadLogo() try
    {
        auto strongThis = get_strong();

        m_logo = winrt::BitmapImage();

        m_logo.ImageFailed({ get_weak(), [this](auto&&, auto&&) {
            ReportLogoFailed();
            }
            });

        auto reportFailure = wil::scope_exit([&] {
            ReportLogoFailed();
            });

        auto icon = m_target.DisplayIcon();
        if (!icon)
        {
            co_return;
        }

        m_logo.SetSource(co_await icon.OpenReadAsync());

        // We succeeded, so cancel the failure report.
        reportFailure.release();
    }
    catch (...)
    {
    }

    winrt::fire_and_forget TransferTargetItem::Invoke(winrt::IInspectable const& sender, winrt::RoutedEventArgs const& /* e */)
    {
        auto result = co_await m_watcher.TransferToAsync(m_target, WindowIdFromElement(sender.as<winrt::UIElement>()));
    }

    void TransferTargetItem::Update(winrt::TransferTarget const& target)
    {
        m_forceFallback = false;
        m_target = target;
        RaisePropertyChanged(L"Label");
        RaisePropertyChanged(L"Logo");
        RaisePropertyChanged(L"IsEnabled");
    }

    void MainWindow::InitializeComponent()
    {
        MainWindowT::InitializeComponent();

        // Disable the feature if the TransferTargetWatcher is not available.
        if (!winrt::ApiInformation::IsTypePresent(winrt::name_of<winrt::TransferTargetWatcher>()))
        {
            NotSupportedPanel().Visibility(Visibility::Visible);
            ConfigurationPanel().Visibility(Visibility::Collapsed);
        }
    }

    winrt::fire_and_forget MainWindow::OnTargetAdded(winrt::TransferTargetWatcher sender, winrt::TransferTargetChangedEventArgs const& args)
    {
        // Create a strong reference to keep ourselves alive across the coroutine suspension.
        auto strongThis = get_strong();

        auto target = args.Target();

        co_await wil::resume_foreground(DispatcherQueue());

        // Check that the event came from the watcher we are using.
        // This avoids a race condition where the event is in flight when the watcher is stopped.
        if (sender == m_watcher)
        {
            m_discoveredTargets.Append(make<TransferTargetItem>(sender, target));
        }
    }

    auto MainWindow::FindTargetItemById(winrt::hstring const& id)
    {
        return std::find_if(m_discoveredTargets.begin(), m_discoveredTargets.end(), [&](auto const& item)
            {
                return winrt::get_self<TransferTargetItem>(item)->Id() == id;
            });
    }

    winrt::fire_and_forget MainWindow::OnTargetRemoved(winrt::TransferTargetWatcher sender, winrt::TransferTargetChangedEventArgs const& args)
    {
        // Create a strong reference to keep ourselves alive across the coroutine suspension.
        auto strongThis = get_strong();

        auto target = args.Target();

        co_await wil::resume_foreground(DispatcherQueue());

        // Check that the event came from the watcher we are using.
        // This avoids a race condition where the event is in flight when the watcher is stopped.
        if (sender == m_watcher)
        {
            // If the target is in the list, remove it.
            // Note that the "target" is not necessarily the same object as the one passed to the Added event.
            // You have to match it up by the ID.
            auto it = FindTargetItemById(target.Id());
            if (it != m_discoveredTargets.end())
            {
                m_discoveredTargets.RemoveAt(static_cast<uint32_t>(it - m_discoveredTargets.begin()));
            }
        }
    }

    winrt::fire_and_forget MainWindow::OnTargetUpdated(winrt::TransferTargetWatcher sender, winrt::TransferTargetChangedEventArgs const& args)
    {
        // Create a strong reference to keep ourselves alive across the coroutine suspension.
        auto strongThis = get_strong();

        auto target = args.Target();

        co_await wil::resume_foreground(DispatcherQueue());

        // Check that the event came from the watcher we are using.
        // This avoids a race condition where the event is in flight when the watcher is stopped.
        if (sender == m_watcher)
        {
            // If the target is in the list, update it.
            // Note that the "target" is not necessarily the same object as the one passed to the Added event.
            // You have to match it up by the ID.
            auto it = FindTargetItemById(target.Id());
            if (it != m_discoveredTargets.end())
            {
                winrt::get_self<TransferTargetItem>(*it)->Update(target);
            }
        }
    }

    winrt::fire_and_forget MainWindow::SelectFilesButton_Click(winrt::Windows::Foundation::IInspectable const& /* sender */, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& /* e */)
    {
        // Create a strong reference to keep ourselves alive across the coroutine suspension.
        auto strongThis = get_strong();

        // Create and configure a file picker.
        FileOpenPicker picker;
        picker.ViewMode(PickerViewMode::List);
        picker.SuggestedStartLocation(PickerLocationId::DocumentsLibrary);
        picker.FileTypeFilter().Append(L"*");

        // Initialize the picker with our window handle.
        HWND hwnd;
        try_as<IWindowNative>()->get_WindowHandle(&hwnd);
        picker.as<IInitializeWithWindow>()->Initialize(hwnd);

        // Show the picker and remember the selected files.
        auto files = co_await picker.PickMultipleFilesAsync();

        m_selectedFiles.assign(files.begin(), files.end());

        // Update the label that summarizes how many files were selected.
        if (!m_selectedFiles.empty())
        {
            std::wstring text = std::to_wstring(m_selectedFiles.size()) + L" file(s) selected";
            FilesCountLabel().Text(text);
        }
        else
        {
            FilesCountLabel().Text(L"No files selected");
        }
    }

    void MainWindow::DiscoverTargetsButton_Click(winrt::Windows::Foundation::IInspectable const& /* sender */, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& /* e */)
    {
        // Create a new data package
        winrt::DataPackage dataPackage;

        // Fill the DataPackage with requested data.

        // StorageItems
        if (!m_selectedFiles.empty())
        {
            dataPackage.SetStorageItems(m_selectedFiles);
        }

        // Link
        if (LinkComboBox().SelectedIndex() > 0)
        {
            auto uri = winrt::Windows::Foundation::Uri(LinkComboBox().SelectedItem().as<winrt::hstring>());
            dataPackage.SetApplicationLink(uri);
            dataPackage.SetWebLink(uri);
        }

        // Text
        if (TextCheckBox().IsChecked().Value())
        {
            dataPackage.SetText(TextInput().Text());
        }

        // HTML
        if (HtmlCheckBox().IsChecked().Value())
        {
            dataPackage.SetHtmlFormat(winrt::HtmlFormatHelper::CreateHtmlFormat(HtmlInput().Text()));
        }

        // Custom Data Format
        if (CustomDataFormatCheckBox().IsChecked().Value())
        {
            dataPackage.SetData(CustomDataFormatNameInput().Text(), winrt::box_value(CustomDataContentInput().Text()));
        }

        // Set additional data package properties for illustration.
        // For demonstration purposes, these are hard-coded.
        auto properties = dataPackage.Properties();
        properties.Title(L"Sample Transfer Target Test");
        properties.Description(L"Testing TransferTargetWatcher API with multiple data formats");

        properties.ContentSourceWebLink(winrt::Windows::Foundation::Uri(L"https://www.microsoft.com"));
        properties.ContentSourceApplicationLink(winrt::Windows::Foundation::Uri(L"sampleapp://test"));
        properties.ApplicationListingUri(winrt::Windows::Foundation::Uri(L"https://www.microsoft.com/store"));

        // Add custom properties for additional metadata.
        properties.Insert(L"Author", winrt::box_value(L"Test User"));
        properties.Insert(L"Version", winrt::box_value(L"1.0.0"));
        properties.Insert(L"Category", winrt::box_value(L"Testing"));

        // Create TransferTargetDiscoveryOptions with the data package view
        auto discoveryOptions = TransferTargetDiscoveryOptions(dataPackage.GetView());

        // Maximum number of app targets.
        // It is important to set this because the default is zero, which produces no results.
        discoveryOptions.MaxAppTargets(NumberOfTargetsComboBox().SelectedItem().as<int32_t>());

        // Set AllowedTargetAppIds from the app filter input (comma-separated list) only if checkbox is checked
        if (AppFilterCheckBox().IsChecked().Value())
        {
            // Parse comma-separated AppIds into a vector
            auto appIds = std::vector<winrt::hstring>();
            std::wstringstream ss{ std::wstring(AppFilterInput().Text()) };
            std::wstring appId;

            while (ss >> std::ws, std::getline(ss, appId, L','))
            {
                appIds.emplace_back(appId);
            }

            discoveryOptions.AllowedTargetAppIds(appIds);
        }

        // Create the watcher
        m_watcher = TransferTarget::CreateWatcher(discoveryOptions);

        // Register for watcher events
        m_watcher.Added({ get_weak(), &MainWindow::OnTargetAdded });
        m_watcher.Removed({ get_weak(), &MainWindow::OnTargetRemoved });
        m_watcher.Updated({ get_weak(), &MainWindow::OnTargetUpdated });

        // Start the watcher
        m_watcher.Start();
        m_watcherRunning = true;

        // Go to Discovery view
        ConfigurationPanel().Visibility(Visibility::Collapsed);
        DiscoveryPanel().Visibility(Visibility::Visible);
        StopButton().IsEnabled(true);
    }

    void MainWindow::StopDiscoveryButton_Click(winrt::Windows::Foundation::IInspectable const& /* sender */, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& /* e */)
    {
        // Stop the watcher but stay in Discovery view so the user can still interact
        // with the discovered targets.
        m_watcher.Stop();
        m_watcherRunning = false;

        StopButton().IsEnabled(false);
    }

    void MainWindow::ReconfigureButton_Click(winrt::Windows::Foundation::IInspectable const& /* sender */, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& /* e */)
    {
        // Stop the watcher if we haven't already
        if (m_watcherRunning)
        {
            m_watcher.Stop();
            m_watcherRunning = false;
        }
        m_watcher = nullptr;

        // Clear targets from UI
        m_discoveredTargets.Clear();

        // Go back to Watcher Configuration view.
        ConfigurationPanel().Visibility(Visibility::Visible);
        DiscoveryPanel().Visibility(Visibility::Collapsed);
    }
}
