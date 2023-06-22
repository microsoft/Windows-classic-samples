// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif
#include "TabViewModel.h"

namespace winrt
{
    using namespace winrt::AppTabsIntegration;
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Collections;
    using namespace winrt::Windows::Foundation::Metadata;
    using namespace winrt::Windows::Graphics::Imaging;
    using namespace winrt::Windows::Storage::Streams;
    using namespace winrt::Windows::UI::Shell;
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Controls;
    using namespace winrt::Microsoft::UI::Xaml::Media::Imaging;
}

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace
{
    // Helper for making Windows.UI.WindowId objects from HWNDs.
    winrt::Windows::UI::WindowId WindowsWindowIdFromHwnd(HWND hwnd)
    {
        static_assert(sizeof(winrt::Windows::UI::WindowId) == sizeof(winrt::Microsoft::UI::WindowId));
        auto id = winrt::Microsoft::UI::GetWindowIdFromWindow(hwnd);
        return reinterpret_cast<winrt::Windows::UI::WindowId&>(id);
    }
}

namespace winrt::AppTabsIntegration::implementation
{
    MainWindow::MainWindow(winrt::TabViewModel const& initialTab)
    {
        InitializeComponent();
        m_dispatcherQueue = this->DispatcherQueue();
        MainWindowTabView().TabItemsSource(m_tabs);

        // Retrieve the window handle (HWND) of this WinUI 3 window.
        HWND hwnd;
        winrt::check_hresult(this->try_as<::IWindowNative>()->get_WindowHandle(&hwnd));

        // Apply right-to-left flow direction if this window has right-to-left layout.
        if (GetWindowLongW(hwnd, GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
        {
            this->Content().as<winrt::FrameworkElement>().FlowDirection(FlowDirection::RightToLeft);
        }

        // Add WindowTabManager support if it is available.
        if (ApiInformation::IsApiContractPresent(winrt::name_of<WindowTabManagerContract>(), 1) && WindowTabManager::IsSupported())
        {
            // Get the WindowTabManager for this window.
            m_tabManager = WindowTabManager::GetForWindow(WindowsWindowIdFromHwnd(hwnd));

            // Register WindowTabManager events so we can respond to requests from the system.
            m_tabSwitchRevoker = m_tabManager.TabSwitchRequested(winrt::auto_revoke, { get_weak(), &MainWindow::TabSwitchRequested});
            m_tabCloseRevoker = m_tabManager.TabCloseRequested(winrt::auto_revoke, { get_weak(), &MainWindow::TabCloseRequested });
            m_tabThumbnailRequestedRevoker = m_tabManager.TabThumbnailRequested(winrt::auto_revoke, { get_weak(), &MainWindow::TabThumbnailRequested });

            // You can use WindowTabManager::IsTabTearOutSupported() to detect whether the system supports tab tear-out.
            // If the feature is not supported, then the TabTearOutRequested event is never raised, but it's still okay
            // to register a handler for it.
            m_tabTearOutRequestedRevoker = m_tabManager.TabTearOutRequested(winrt::auto_revoke, { get_weak(), &MainWindow::TabTearOutRequested });
        }

        // Subscribe to vector changes so we can keep the WindowTabManager in sync.
        // This is also how we learn about XAML tab rearrangement.
        m_tabs.VectorChanged({ get_weak(), &MainWindow::TabCollection_VectorChanged });

        // Insert the initial app tab.
        // The VectorChanged event handler will update the WindowTabManager.
        if (initialTab)
        {
            // Caller provided an initial tab.
            m_tabs.Append(initialTab);
        }
        else
        {
            // Create a default initial tab.
            m_tabs.Append(CreateTabViewModel());
        }
    }

    winrt::TabViewModel MainWindow::CreateTabViewModel()
    {
        static int s_counter = 0;

        winrt::hstring tabName = L"Tab #" + winrt::to_hstring(++s_counter);
        WindowTab windowTab{ nullptr };

        if (m_tabManager)
        {
            // Make a WindowTab that is the counterpart to our app tab.
            windowTab = WindowTab();

            // The Icon of the WindowTab should match the icon of the tab in the app (if any).
            RandomAccessStreamReference icoStream = RandomAccessStreamReference::CreateFromUri(Uri(L"ms-appx:///Assets/icon.ico"));
            windowTab.Icon(WindowTabIcon::CreateFromImage(icoStream));
        }

        // Create a tab to add to our TabView.
        auto const tabViewModel = winrt::make<TabViewModel>(tabName, windowTab);

        return tabViewModel;
    }

    // Handle the XAML event that tells us that the user wants to close a tab.
    // This event is raised on the UI thread.
    void MainWindow::TabView_CloseRequested(TabView const&, TabViewTabCloseRequestedEventArgs const& args)
    {
        auto const tabViewModel = args.Item().as<winrt::TabViewModel>();
        RemoveTabByItem(tabViewModel);
    }

    // Handle the XAML event that tells us that the user wants to create a new tab.
    // This event is raised on the UI thread.
    void MainWindow::TabView_AddTabClicked(TabView const&, IInspectable const&)
    {
        // Add a new TabViewModel to our app's TabView.
        winrt::TabViewModel tabViewModel = CreateTabViewModel();

        // The VectorChanged event handler will update the WindowTabManager.
        m_tabs.Append(tabViewModel);

        // Auto-select the newly-added tab.
        MainWindowTabView().SelectedItem(tabViewModel);
    }

    // Handle the XAML event that tells us that the user switched tabs.
    // This event is raised on the UI thread.
    void MainWindow::TabView_SelectionChanged(IInspectable const&, SelectionChangedEventArgs const&)
    {
        auto const tabModel = MainWindowTabView().SelectedItem().as<winrt::TabViewModel>();
        if (m_tabManager && tabModel)
        {
            // Tell the WindowTabManager that we have changed active tabs.
            m_tabManager.SetActiveTab(TabViewModel::GetWindowTab(tabModel));
        }
    }

    // Handle the XAML event that tells us that the user tore out a tab.
    // This event is raised on the UI thread.
    void MainWindow::TabView_TabDroppedOutside(TabView const&, TabViewTabDroppedOutsideEventArgs const& args)
    {
        // Don't let the user tear out the last tab.
        if (m_tabs.Size() < 2)
        {
            return;
        }

        // Remove the tab from this window
        winrt::TabViewModel tabViewModel = args.Item().as<winrt::TabViewModel>();
        RemoveTabByItem(tabViewModel);

        // Create another MainWindow with that tab as its only tab.
        auto tornOut = winrt::make<MainWindow>(tabViewModel);

        // Position the window under the cursor.
        POINT pos;
        if (GetCursorPos(&pos))
        {
            HWND hwnd;
            winrt::check_hresult(tornOut.try_as<::IWindowNative>()->get_WindowHandle(&hwnd));
            RECT rect;
            GetWindowRect(hwnd, &rect);
            MapWindowPoints(HWND_DESKTOP, hwnd, reinterpret_cast<LPPOINT>(&rect), 2);
            if (this->Content().as<winrt::FrameworkElement>().FlowDirection() == FlowDirection::RightToLeft)
            {
                RECT clientRect;
                GetClientRect(hwnd, &clientRect);
                pos.x -= clientRect.right;
            }
            SetWindowPos(hwnd, nullptr, pos.x + rect.left, pos.y + rect.top,
                0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
        }

        // Show the new window.
        tornOut.Activate();
    }

    // Handle the IObservableVector event that tells us that the list of TabDataModel objects changed.
    // This event is raised on the UI thread.
    void MainWindow::TabCollection_VectorChanged(IObservableVector<winrt::TabViewModel> const&, IVectorChangedEventArgs const& args)
    {
        // Keep the WindowTabManager's tab list in sync with our app's TabView tab list.
        if (m_tabManager)
        {
            auto index = args.Index();
            switch (args.CollectionChange())
            {
            case CollectionChange::ItemInserted:
                m_tabManager.Tabs().InsertAt(index, TabViewModel::GetWindowTab(m_tabs.GetAt(index)));
                break;
            case CollectionChange::ItemRemoved:
                m_tabManager.Tabs().RemoveAt(index);
                break;
            }
        }

        // Update the window title to reflect the number of tabs.
        auto const tabCount = m_tabs.Size();
        if (tabCount == 1)
        {
            this->Title(L"AppTabsIntegration Sample (1 tab)");
        }
        else
        {
            this->Title(L"AppTabsIntegration Sample (" + winrt::to_hstring(tabCount) + L" tabs)");
        }
    }

    // Handle the system event that tells us that the user used the system to select a tab in our app.
    // This event can be raised on any thread, so we explicitly switch to the UI thread before doing work.
    fire_and_forget MainWindow::TabSwitchRequested(WindowTabManager const&, WindowTabSwitchRequestedEventArgs args)
    {
        // Take a strong reference to the MainWindow so it doesn't get destructed while we co_await.
        auto strongThis = get_strong();
        co_await wil::resume_foreground(m_dispatcherQueue);

        // Switch to that tab in our UI. When we get the TabView.SelectionChanged event,
        // we will update the WindowTabManager to let it know that the switch has occurred.
        winrt::TabViewModel tabViewModel = TabViewModel::GetFromWindowTab(args.Tab());
        MainWindowTabView().SelectedItem(tabViewModel);
    }

    // Handle the system event that tells us that the user used the system to close a tab in our app.
    // This event can be raised on any thread, so we explicitly switch to the UI thread before doing work.
    fire_and_forget MainWindow::TabCloseRequested(WindowTabManager const&, WindowTabCloseRequestedEventArgs args)
    {
        // Take a strong reference to the MainWindow so it doesn't get destructed while we co_await.
        auto strongThis = get_strong();
        co_await wil::resume_foreground(m_dispatcherQueue);

        winrt::TabViewModel tabViewModel = TabViewModel::GetFromWindowTab(args.Tab());
        RemoveTabByItem(tabViewModel);
    }

    // Handle the system event that tells us that the system wants us to produce a thumbnail of a tab.
    // This event can be raised on any thread, so we explicitly switch to the UI thread before doing work.
    fire_and_forget MainWindow::TabThumbnailRequested(WindowTabManager const&, WindowTabThumbnailRequestedEventArgs args)
    {
        // Take a strong reference to the MainWindow so it doesn't get destructed while we co_await.
        auto strongThis = get_strong();

        // Take a deferral before we co_await so the system knows that we are still working on the thumbnail.
        auto deferral = args.GetDeferral();
        co_await wil::resume_foreground(m_dispatcherQueue);

        // For demonstration purposes, the thumbnail for a tab is the tab's solid color.
        // A real program would probably generate a thumbnail image of the tab contents.
        winrt::TabViewModel tabViewModel = TabViewModel::GetFromWindowTab(args.Tab());
        auto color = tabViewModel.Color();
        uint8_t pixel[4] = { color.B, color.G, color.R, 0xFF };
        auto inMemoryStream = InMemoryRandomAccessStream();
        auto encoder = co_await BitmapEncoder::CreateAsync(BitmapEncoder::PngEncoderId(), inMemoryStream);
        encoder.SetPixelData(
            BitmapPixelFormat::Bgra8, BitmapAlphaMode::Ignore,
            1, 1, 96, 96, pixel);

        auto size = args.RequestedSize();
        encoder.BitmapTransform().ScaledWidth(size.Width);
        encoder.BitmapTransform().ScaledHeight(size.Height);

        co_await encoder.FlushAsync();
        inMemoryStream.Seek(0);

        auto streamRef = RandomAccessStreamReference::CreateFromStream(inMemoryStream);
        args.Image(streamRef);

        // Complete the deferral to tell the system that the Image is ready.
        deferral.Complete();
    }

    // Handle the system event that tells us that the user has asked for a tab to be torn out.
    // This event can be raised on any thread, so we explicitly switch to the UI thread before doing work.
    winrt::fire_and_forget MainWindow::TabTearOutRequested(WindowTabManager const&, const WindowTabTearOutRequestedEventArgs args)
    {
        // Take a strong reference to the MainWindow so it doesn't get destructed while we co_await.
        auto strongThis = get_strong();

        // Take a deferral before we co_await so the system knows that we are still working on the tear-out.
        auto deferral = args.GetDeferral();
        co_await wil::resume_foreground(m_dispatcherQueue);

        // Remove the tab from this window
        winrt::TabViewModel tabViewModel = TabViewModel::GetFromWindowTab(args.Tab());
        RemoveTabByItem(tabViewModel);

        // Create another MainWindow with that tab as its only tab.
        auto tornOut = winrt::make<MainWindow>(tabViewModel);

        HWND hwnd;
        winrt::check_hresult(tornOut.try_as<::IWindowNative>()->get_WindowHandle(&hwnd));
        args.WindowId(reinterpret_cast<uint64_t>(hwnd));

        // Complete the deferral to tell the system the WindowId is ready.
        deferral.Complete();

        // Show the window after completing the deferral.
        tornOut.Activate();
    }

    void MainWindow::RemoveTabByItem(winrt::TabViewModel const& model)
    {
        uint32_t index;
        if (m_tabs.IndexOf(model, index))
        {
            // Note that this RemoveAt may trigger a SelectionChanged if the current selection is being removed.
            // Our VectorChanged event handler will update the WindowTabManager.
            m_tabs.RemoveAt(index);
        }

        // If that was the last tab, then exit this window.
        if (m_tabs.Size() == 0)
        {
            this->Close();
        }
    }
}
