// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"

#include "App.xaml.h"
#include "MainWindow.xaml.h"
#include "MainPage.xaml.h"
#include "MakeCredentialPage.xaml.h"
#include "GetAssertion.xaml.h"
#include "PluginManagement/PluginRegistrationManager.h"
#include "PluginManagement/PluginCredentialManager.h"
#include "PluginAuthenticator/PluginAuthenticatorImpl.h"
#include <winrt/Microsoft.ui.interop.h>
#include <winrt/Windows.Foundation.h>

#include <include/cbor-lite/codec.h>
#include <string>
#include <iostream>
#include <fstream>
namespace winrt
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Microsoft::UI::Windowing;
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Controls;
    using namespace winrt::Microsoft::UI::Xaml::Navigation;
    using namespace CborLite;
}

namespace winrt::PasskeyManager::implementation
{

void App::register_plugin_class_factory()
{
    winrt::check_hresult(CoInitializeSecurity(
        nullptr,
        -1,
        nullptr,
        nullptr,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr,
        EOAC_NONE,
        nullptr));

    winrt::check_hresult(::CoRegisterClassObject(
        contosoplugin_guid,
        make<ContosoPluginFactory>().get(),
        CLSCTX_LOCAL_SERVER,
        REGCLS_MULTIPLEUSE,
        &m_registration));
}

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

/// <summary>
/// Initializes the singleton application object.  This is the first line of authored code
/// executed, and as such is the logical equivalent of main() or WinMain().
/// </summary>
App::App() :
    m_hPluginProceedButtonEvent(nullptr),
    m_hPluginUserCancelEvent(nullptr),
    m_hPluginOpReqDisplayInfoParsedEvent(nullptr),
    m_hPluginCredentialSelected(nullptr),
    m_hVaultConsentFailed(nullptr),
    m_hVaultConsentComplete(nullptr),
    m_hWindowReady(nullptr),
    m_pluginOperationStatus({})
{
    InitializeComponent();

#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
    UnhandledException([this](IInspectable const&, UnhandledExceptionEventArgs const& e)
        {
            if (IsDebuggerPresent())
            {
                auto errorMessage = e.Message();
                __debugbreak();
            }
        });
#endif
}

App::App(PWSTR args) : m_args(args)
{
    InitializeComponent();
#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
    UnhandledException([this](IInspectable const&, UnhandledExceptionEventArgs const& e)
        {
            if (IsDebuggerPresent())
            {
                auto errorMessage = e.Message();
                __debugbreak();
            }
        });
#endif
}

void App::InitializeAppWindTitleBar()
{
    AppWindow appWind = m_window.as<MainWindow>()->GetAppWindow();
    AppWindowTitleBar titleBar = appWind.TitleBar();
    titleBar.ButtonBackgroundColor(winrt::Microsoft::UI::Colors::Transparent());
    titleBar.ExtendsContentIntoTitleBar(true);
    titleBar.PreferredHeightOption(winrt::Microsoft::UI::Windowing::TitleBarHeightOption::Tall);
    appWind.Title(L"Contoso Passkey Manager");
    appWind.SetIcon(L"Assets\\icon.ico");
    appWind.Title(L"");
}

void App::OnLaunched(LaunchActivatedEventArgs const&)
{
    std::wstring argsString{ m_args };
    if (argsString.find(L"-PluginActivated") != std::wstring::npos)
    {
        SetEvent(App::s_hAppReadyForPluginOpEvent.get());
        HANDLE rghWait[] = {
            m_hPluginOpReqDisplayInfoParsedEvent.get(),
            s_hPluginOpCompletedEvent.get()
        };

        DWORD dwSleepMilliseconds = INFINITE;
        DWORD hIndex = 0;
        if (FAILED(CoWaitForMultipleHandles(COWAIT_DISPATCH_WINDOW_MESSAGES | COWAIT_DISPATCH_CALLS, dwSleepMilliseconds, ARRAYSIZE(rghWait), rghWait, &hIndex)))
        {
            return;
        }

        if (hIndex == 1) // Plugin operation completed without parsing display information. No UI to be displayed.
        {
            PluginCancelAction();
            this->Exit();
            return;
        }
        else if (m_silentMode)
        {
            SetEvent(m_hWindowReady.get());
            SetEvent(m_hPluginProceedButtonEvent.get());
            LOG_IF_FAILED(CoWaitForMultipleHandles(COWAIT_DISPATCH_WINDOW_MESSAGES | COWAIT_DISPATCH_CALLS, dwSleepMilliseconds, 1, App::s_hPluginOpCompletedEvent.addressof(), &hIndex));
            PluginCancelAction();
            this->Exit();
            return;
        }
        else
        {
            m_window = make<MainWindow>();
            Frame rootFrame = CreateRootFrame();
            if (m_pluginOperationOptions.operationType == PluginOperationType::PLUGIN_OPERATION_TYPE_MAKE_CREDENTIAL)
            {
                rootFrame.Navigate(xaml_typename<PasskeyManager::MakeCredentialPage>(), box_value(m_args));
            }
            else
            {
                rootFrame.Navigate(xaml_typename<PasskeyManager::GetAssertion>(), box_value(m_args));
            }
            InitializeAppWindTitleBar();
            m_window.Activate();

            HWND windowNative = m_window.as<MainWindow>()->GetNativeWindowHandle();
            HWND& clientWindowHandle = m_pluginOperationOptions.hWnd;
            RECT rc, rcClient, rcWindow;
            GetWindowRect(clientWindowHandle, &rcClient);
            GetWindowRect(windowNative, &rcWindow);
            CopyRect(&rc, &rcClient);

            int width = rcClient.right - rcClient.left;
            int height = rcClient.bottom - rcClient.top;
            int newWidth = static_cast<int>(width * 1.2);
            int newHeight = static_cast<int>(height * 1.2);
            int newX = rcClient.left - static_cast<int>((newWidth - width) / 2);
            int newY = rcClient.top - static_cast<int>((newHeight - height) / 2);

            SetWindowPos(windowNative,
                HWND_TOP,
                newX,
                newY,
                newWidth,
                newHeight,
                SWP_ASYNCWINDOWPOS);
            SetForegroundWindow(windowNative);
            SetEvent(m_hWindowReady.get());
        }
    }
    else
    {
        PluginCredentialManager::getInstance();
        m_window = make<MainWindow>();
        InitializeAppWindTitleBar();
        Frame rootFrame = CreateRootFrame();
        rootFrame.Navigate(xaml_typename<PasskeyManager::MainPage>(), box_value(m_args));
        m_window.Activate();
    }
}

Frame App::CreateRootFrame()
{
    Frame rootFrame{ nullptr };
    auto content = m_window.Content();
    if (content)
    {
        rootFrame = content.try_as<Frame>();
    }

    if (!rootFrame)
    {
        rootFrame = Frame();
        rootFrame.NavigationFailed({ this, &App::OnNavigationFailed });
        m_window.Content(rootFrame);
    }

    return rootFrame;
}

void App::OnNavigationFailed(IInspectable const&, NavigationFailedEventArgs const& e)
{
    throw hresult_error(E_FAIL, hstring(L"Failed to load Page ") + e.SourcePageType().Name);
}

HWND App::GetNativeWindowHandle()
{
    return this->m_window.try_as<MainWindow>()->GetNativeWindowHandle();
}

bool App::SetPluginPerformOperationOptions(HWND hWnd,
    PluginOperationType operationType,
    std::wstring rpName,
    std::wstring userName)
{
    m_pluginOperationOptions.hWnd = hWnd;
    m_pluginOperationOptions.operationType = operationType;
    m_pluginOperationOptions.rpName = rpName;
    m_pluginOperationOptions.userName = userName;

    m_pluginOperationStatus.performOperationStatus = S_OK;

    if (m_pluginOperationOptions.matchingCredentials.size() <= 1)
    {
        auto& credManager = PluginCredentialManager::getInstance();
        m_silentMode = credManager.GetSilentOperation();
    }

    SetEvent(this->m_hPluginOpReqDisplayInfoParsedEvent.get());
    return true;
}

bool App::SetSelectedCredentialId(Windows::Storage::Streams::IBuffer credentialId)
{
    std::lock_guard<std::mutex> lock(m_pluginOperationOptionsMutex);
    for (auto& cred : m_pluginOperationOptions.matchingCredentials)
    {
        const std::vector<UINT8> credId(cred->pbCredentialID, cred->pbCredentialID + cred->cbCredentialID);
        auto reader = winrt::Windows::Storage::Streams::DataReader::FromBuffer(credentialId);
        std::vector<UINT8> selectedCredentialIdVec(reader.UnconsumedBufferLength());
        reader.ReadBytes(selectedCredentialIdVec);
        if (credId.size() == selectedCredentialIdVec.size() && memcmp(credId.data(), selectedCredentialIdVec.data(), credId.size()) == 0)
        {
            m_pluginOperationOptions.selectedCredential = cred;
            SetEvent(m_hPluginCredentialSelected.get());
            return true;
        }
    }
    return false;
}

bool App::SetMatchingCredentials(
    std::wstring_view rpName,
    const std::vector<const WEBAUTHN_CREDENTIAL_DETAILS *>& matchedCreds,
    HWND clientHwnd)
{
    std::lock_guard<std::mutex> lock(m_pluginOperationOptionsMutex);
    m_pluginOperationOptions.rpName = rpName;
    m_pluginOperationOptions.operationType = PluginOperationType::PLUGIN_OPERATION_TYPE_GET_ASSERTION;
    m_pluginOperationOptions.matchingCredentials = matchedCreds;
    m_pluginOperationOptions.hWnd = clientHwnd;

    // If we have multiple creds to choose from them we need UI.
    m_silentMode = false;

    SetEvent(this->m_hPluginOpReqDisplayInfoParsedEvent.get());
    return true;
}

winrt::fire_and_forget App::SimulateUnLockVaultUsingConsentVerifier()
{
    auto mainWindow = m_window.as<MainWindow>();
    auto reenableUI = wil::scope_exit([&]() {mainWindow->EnableUI(); });
    if (m_silentMode)
    {
        reenableUI.release();
    }
    else
    {
        mainWindow->DisableUI();
    }

    auto consentTask = mainWindow->RequestConsent(L"Unlock Contoso Passkey Manager Vault");
    auto consentResult = co_await consentTask;
    if (SUCCEEDED(consentResult))
    {
        SetEvent(m_hVaultConsentComplete.get());
    }
    else
    {
        SetEvent(m_hVaultConsentFailed.get());
    }
}

bool App::PluginCompleteAction()
{
    SetEvent(m_hPluginProceedButtonEvent.get());
    DWORD hIndex = 0;
    LOG_IF_FAILED(CoWaitForMultipleHandles(COWAIT_DISPATCH_WINDOW_MESSAGES | COWAIT_DISPATCH_CALLS, INFINITE, 1, App::s_hPluginOpCompletedEvent.addressof(), &hIndex));
    m_registration.reset();
    DWORD dwProcessId;
    GetWindowThreadProcessId(m_pluginOperationOptions.hWnd, &dwProcessId);
    AllowSetForegroundWindow(dwProcessId);
    SetForegroundWindow(m_pluginOperationOptions.hWnd);
    m_window.as<MainWindow>()->UpdatePasskeyOperationStatus(m_pluginOperationStatus.performOperationStatus);
    return true;

}

bool App::PluginCancelAction()
{
    SetEvent(m_hPluginUserCancelEvent.get());
    SetEvent(m_hPluginCredentialSelected.get());

    if (m_pluginOperationOptions.hWnd)
    {
        DWORD dwProcessId;
        GetWindowThreadProcessId(m_pluginOperationOptions.hWnd, &dwProcessId);
        AllowSetForegroundWindow(dwProcessId);
        SetForegroundWindow(m_pluginOperationOptions.hWnd);
    }
    std::wstring message = L"Plugin Operation Canceled\n";
    m_window.as<MainWindow>()->UpdatePasskeyOperationStatus(NTE_USER_CANCELLED);
    m_registration.reset();
    return true;
}
}


int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ PWSTR args, _In_ int)
{
    winrt::init_apartment(winrt::apartment_type::single_threaded);

    std::wstring argsString{ args };
    if (argsString.find(L"-PluginActivated") != std::wstring::npos)
    {
        winrt::PasskeyManager::implementation::App::register_plugin_class_factory();
        DWORD hIndex = 0;
        THROW_IF_FAILED(CoWaitForMultipleHandles(
            COWAIT_DISPATCH_WINDOW_MESSAGES | COWAIT_DISPATCH_CALLS,
            INFINITE,
            1,
            winrt::PasskeyManager::implementation::App::s_pluginOpRequestRecievedEvent.addressof(),
            &hIndex));
    }

    ::winrt::Microsoft::UI::Xaml::Application::Start(
        [args](auto&&)
        {
            if (args[0] != '\0')
            {
                ::winrt::make<::winrt::PasskeyManager::implementation::App>(args);
            }
            else
            {
                ::winrt::make<::winrt::PasskeyManager::implementation::App>();
            }
        });

    return 0;
}
