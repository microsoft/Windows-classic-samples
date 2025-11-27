// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "App.xaml.h"
#include "MainWindow.xaml.h"
#include "MainPage.xaml.h"
#include "MakeCredentialPage.xaml.h"
#include "GetAssertion.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#include "PluginManagement/PluginCredentialManager.h"
#endif

namespace winrt
{
    using namespace winrt::PasskeyManager;
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Collections;
    using namespace winrt::Windows::Foundation::Metadata;
    using namespace winrt::Windows::Graphics::Imaging;
    using namespace winrt::Windows::Storage::Streams;
    using namespace winrt::Windows::UI::Shell;
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Controls;
    using namespace winrt::Microsoft::UI::Xaml::Media::Imaging;
    using namespace winrt::Windows::Security::Credentials;
    using namespace winrt::Windows::Security::Credentials::UI;
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

namespace winrt::PasskeyManager::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();
        m_dispatcherQueue = this->DispatcherQueue();

        // Retrieve the window handle (HWND) of this WinUI 3 window.
        HWND hwnd;
        winrt::check_hresult(this->try_as<::IWindowNative>()->get_WindowHandle(&hwnd));

        // Apply right-to-left flow direction if this window has right-to-left layout.
        if (GetWindowLongW(hwnd, GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
        {
            this->Content().as<winrt::FrameworkElement>().FlowDirection(FlowDirection::RightToLeft);
        }
        // Set the window title to "Contoso Passkey Manager"
        this->Title(L"Contoso Passkey Manager");
        this->Closed({ this, &MainWindow::OnClosed });
    }

    void MainWindow::OnClosed(IInspectable const&, winrt::Microsoft::UI::Xaml::WindowEventArgs const&)
    {
        auto curApp = winrt::Microsoft::UI::Xaml::Application::Current().as<App>();
        // call PluginCancelAction if this is a plugin operation window
        std::wstring argsString{ curApp->m_args };
        if (argsString.find(L"-PluginActivated") != std::wstring::npos)
        {
            curApp->PluginCancelAction();
        }
    }

    HWND MainWindow::GetNativeWindowHandle()
    {
        HWND hwnd;
        winrt::check_hresult(this->try_as<::IWindowNative>()->get_WindowHandle(&hwnd));
        return hwnd;
    }
    Microsoft::UI::WindowId MainWindow::GetWindowId()
    {
        HWND hwnd = this->GetNativeWindowHandle();
        return Microsoft::UI::GetWindowIdFromWindow(hwnd);
    }
    void MainWindow::UpdatePasskeyOperationStatus(HRESULT hr)
    {
        try
        {
            if (!this || !this->Content() ||
                !this->Content().try_as<Frame>() ||
                !this->Content().try_as<Frame>().Content())
            {
                return;
            }
            auto pluginOperationPage = this->Content().as<Frame>().Content().try_as<IPluginOperationPage>();
            if (pluginOperationPage)
            {
                pluginOperationPage.UpdatePasskeyOperationStatus(hr);
            }
        }
        catch (...) // Best effort to log the result
        {
            return;
        }
    }

    void MainWindow::DisableUI()
    {
        if (!this->Content() || !this->Content().try_as<Frame>() || !this->Content().try_as<Frame>().Content())
        {
            return;
        }
        auto pluginOperationPage = this->Content().as<Frame>().Content().try_as<IPluginOperationPage>();
        if (pluginOperationPage)
        {
            pluginOperationPage.DisableUI();
        }
    }

    void MainWindow::EnableUI()
    {
        if (!this->Content() || !this->Content().try_as<Frame>() || !this->Content().try_as<Frame>().Content())
        {
            return;
        }

        auto pluginOperationPage = this->Content().as<Frame>().Content().try_as<IPluginOperationPage>();
        if (pluginOperationPage)
        {
            pluginOperationPage.EnableUI();
        }
    }


    IAsyncOperation<winrt::hresult> MainWindow::RequestConsent(hstring userMessage)
    {
        auto lifetime = get_strong();

        HWND hwnd = nullptr;

        com_ptr<App> curApp = winrt::Microsoft::UI::Xaml::Application::Current().as<App>();
        if (curApp->GetSilentMode())
        {
            std::lock_guard lock(curApp->m_pluginOperationOptionsMutex);
            hwnd = curApp->m_pluginOperationOptions.hWnd;
        }
        else
        {
            // Retrieve the window handle of the current WinUI 3 window.  
            hwnd = GetNativeWindowHandle();
        }

        // Use the interop interface to request the logged on user's consent via device authentication  
        auto interop = winrt::get_activation_factory<UserConsentVerifier, ::IUserConsentVerifierInterop>();
        auto consentResult =
            co_await winrt::capture<winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Security::Credentials::UI::UserConsentVerificationResult>>(
                interop, &::IUserConsentVerifierInterop::RequestVerificationForWindowAsync, hwnd, reinterpret_cast<HSTRING>(winrt::get_abi(userMessage)));

        HRESULT hr;
        switch (consentResult)
        {
        case winrt::Windows::Security::Credentials::UI::UserConsentVerificationResult::Verified:
            hr = S_OK;
            break;
        case winrt::Windows::Security::Credentials::UI::UserConsentVerificationResult::DeviceBusy:
            hr = HRESULT_FROM_WIN32(ERROR_BUSY);
            break;
        case winrt::Windows::Security::Credentials::UI::UserConsentVerificationResult::DeviceNotPresent:
            hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
            break;
        case winrt::Windows::Security::Credentials::UI::UserConsentVerificationResult::DisabledByPolicy:
            hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DISABLED_BY_POLICY);
            break;
        case winrt::Windows::Security::Credentials::UI::UserConsentVerificationResult::RetriesExhausted:
            hr = HRESULT_FROM_WIN32(ERROR_RETRY);
            break;
        case winrt::Windows::Security::Credentials::UI::UserConsentVerificationResult::Canceled:
            hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
            break;
        default:
            hr = E_FAIL;
            break;
        }

        co_return hr;
    }
}
