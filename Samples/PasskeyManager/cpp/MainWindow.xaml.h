// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once

#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include "MainWindow.g.h"

namespace winrt::PasskeyManager::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();
        HWND GetNativeWindowHandle();
        Microsoft::UI::WindowId GetWindowId();
        Microsoft::UI::Windowing::AppWindow GetAppWindow()
        {
            return Microsoft::UI::Windowing::AppWindow::GetFromWindowId(GetWindowId());
        }

        void UpdatePasskeyOperationStatus(HRESULT hr);
        void DisableUI();
        void EnableUI();

        void OnClosed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::WindowEventArgs const& args);

        winrt::Windows::Foundation::IAsyncOperation<winrt::hresult> RequestConsent(hstring userMessage);

    private:

        winrt::Microsoft::UI::Dispatching::DispatcherQueue m_dispatcherQueue = this->DispatcherQueue();
    };
}

namespace winrt::PasskeyManager::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
