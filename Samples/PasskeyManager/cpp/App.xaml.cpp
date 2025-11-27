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
#include <winrt/Microsoft.UI.Xaml.Media.Animation.h>

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

void App::RegisterPluginClassFactory()
{
    winrt::check_hresult(::CoRegisterClassObject(
        contosoplugin_guid,
        make<ContosoPluginFactory>(m_hPluginOpCompletedEvent, m_hAppReadyForPluginOpEvent, m_hPluginCancelOperationEvent).get(),
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
    m_hPluginCancelOperationEvent(nullptr),
    m_hPluginWindowDisplayInfoReadyEvent(nullptr),
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

void App::ResetPluginOperationState()
{
    // Reset all events to their initial state
    ResetEvent(m_hPluginProceedButtonEvent.get());
    ResetEvent(m_hPluginCancelOperationEvent.get());
    ResetEvent(m_hPluginWindowDisplayInfoReadyEvent.get());
    ResetEvent(m_hPluginCredentialSelected.get());
    ResetEvent(m_hVaultConsentFailed.get());
    ResetEvent(m_hVaultConsentComplete.get());
    ResetEvent(m_hWindowReady.get());
    ResetEvent(m_hPluginOpCompletedEvent.get());

    // Reset operation options
    {
        std::lock_guard<std::mutex> lock(m_pluginOperationOptionsMutex);
        m_pluginOperationOptions = PluginOpertaionOptions{};

    }
    // Reset operation status
    m_pluginOperationStatus = PluginOperationStatus{};

    CloseOrHideWindow();
}

void App::CloseOrHideWindow()
{
    if (m_window)
    {
        // get native window handle
        HWND windowNative = m_window.as<MainWindow>()->GetNativeWindowHandle();
        // reset the owner window
        SetWindowLongPtr(windowNative, GWLP_HWNDPARENT, 0);
        // Hide the window instead of closing it to keep the process alive
        auto appWindow = m_window.as<MainWindow>()->GetAppWindow();
        if (appWindow)
        {
            appWindow.Hide();
        }
    }
}

void App::OnLaunched(LaunchActivatedEventArgs const&)
{
    std::wstring argsString{ m_args };
    if (argsString.find(L"-PluginActivated") != std::wstring::npos)
    {
        // Background Mode: The app is being activated by the OS to handle a passkey operation. It runs in the background.
        RegisterPluginClassFactory();
        // Start the plugin operation handling loop
        HandlePluginOperations();
    }
    else
    {
        // Interactive Mode: The user is launching the app directly.
        PluginCredentialManager::getInstance();
        m_window = make<MainWindow>();
        InitializeAppWindTitleBar();
        Frame rootFrame = CreateRootFrame();
        rootFrame.Navigate(xaml_typename<PasskeyManager::MainPage>(), box_value(m_args));
        m_window.Activate();
    }
}

void App::SetupPluginWindow()
{
    bool windowCreated = false;
    if (m_pluginOperationOptions.silentMode)
    {
        SetEvent(m_hWindowReady.get());
        SetEvent(m_hPluginCredentialSelected.get());
        SetEvent(m_hPluginProceedButtonEvent.get());
        return;
    }
    if (!m_window)
    {
        m_window = make<MainWindow>();
        InitializeAppWindTitleBar();
        windowCreated = true;
    }

    if (m_window.Visible())
    {
        return;
    }

    winrt::Microsoft::UI::Xaml::Controls::Frame rootFrame = CreateRootFrame();
    if (!windowCreated)
    {   rootFrame.Navigated([this](IInspectable const&, NavigationEventArgs const&)
        {
            this->m_window.Content().as<Frame>().Content().as<Page>().Loaded([this](IInspectable const&, RoutedEventArgs const&)
                {
                    this->m_window.AppWindow().Show();
                });
        });
    }
    if (m_pluginOperationOptions.operationType == PluginOperationType::MakeCredential)
    {
        rootFrame.Navigate(xaml_typename<PasskeyManager::MakeCredentialPage>(), box_value(m_args), Media::Animation::SuppressNavigationTransitionInfo{});
    }
    else
    {
        rootFrame.Navigate(xaml_typename<PasskeyManager::GetAssertion>(), box_value(m_args), Media::Animation::SuppressNavigationTransitionInfo{});
    }

    HWND windowNative = m_window.as<MainWindow>()->GetNativeWindowHandle();
    HWND& clientWindowHandle = m_pluginOperationOptions.hWnd;

    SetWindowLongPtr(windowNative, GWLP_HWNDPARENT, reinterpret_cast<LONG_PTR>(clientWindowHandle));

    RECT rc, rcClient, rcWindow;
    GetWindowRect(clientWindowHandle, &rcClient);
    GetWindowRect(windowNative, &rcWindow);
    CopyRect(&rc, &rcClient);

    // Fix the size of the window
    int width = 550;
    int height = 450;

    // Align the center of the window.
    int clientCenterX = (rcClient.right + rcClient.left) / 2;
    int clientCenterY = (rcClient.bottom + rcClient.top) / 2;
    int newX = max(clientCenterX - width / 2, 0);
    int newY = max(clientCenterY - height / 2, 0);

    SetWindowPos(windowNative,
        HWND_TOP,
        newX,
        newY,
        width,
        height,
        SWP_ASYNCWINDOWPOS);

    if (windowCreated)
    {
        m_window.Activate();
    }
    SetEvent(m_hWindowReady.get());
}

void App::HandlePluginOperations()
{
    // This method handles plugin operations in a loop as long as the app is running in background.
    while (true)
    {
        SetEvent(m_hAppReadyForPluginOpEvent.get());
        HANDLE rghWait[] = {
            m_hPluginWindowDisplayInfoReadyEvent.get(),
            m_hPluginOpCompletedEvent.get(),
            m_hPluginCancelOperationEvent.get()
        };

        DWORD dwSleepMilliseconds = INFINITE;
        DWORD hIndex = 0;
        if (FAILED(CoWaitForMultipleHandles(COWAIT_DISPATCH_WINDOW_MESSAGES | COWAIT_DISPATCH_CALLS, dwSleepMilliseconds, ARRAYSIZE(rghWait), rghWait, &hIndex)))
        {
            break; // Exit the loop on error
        }

        if (hIndex == 2) // user canceled before window could be setup
        {
            break;
        }
        else if (hIndex == 1) // Plugin operation completed without parsing display information. No UI to be displayed.
        {
            PluginCancelAction();
            continue; // Continue to next operation
        }
        else if (m_pluginOperationOptions.silentMode)
        {
            SetEvent(m_hWindowReady.get());
            SetEvent(m_hPluginProceedButtonEvent.get());
            LOG_IF_FAILED(CoWaitForMultipleHandles(COWAIT_DISPATCH_WINDOW_MESSAGES | COWAIT_DISPATCH_CALLS, dwSleepMilliseconds, 1, m_hPluginOpCompletedEvent.addressof(), &hIndex));
            ResetPluginOperationState();
            continue; // Continue to next operation
        }
        else
        {
            LOG_IF_FAILED(CoWaitForMultipleHandles(COWAIT_DISPATCH_WINDOW_MESSAGES | COWAIT_DISPATCH_CALLS, dwSleepMilliseconds, 1, m_hWindowReady.addressof(), &hIndex));
            break;
        }
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
    auto& credMgr = PluginCredentialManager::getInstance();
    credMgr.ReloadRegistryValues();
    {
        std::lock_guard<std::mutex> lock(m_pluginOperationOptionsMutex);
        m_pluginOperationOptions.hWnd = hWnd;
        m_pluginOperationOptions.operationType = operationType;
        m_pluginOperationOptions.rpName = rpName;
        m_pluginOperationOptions.userName = userName;
        m_pluginOperationOptions.silentMode = credMgr.GetSilentOperation();
        if (m_pluginOperationOptions.matchingCredentials.size() > 1 || credMgr.GetVaultLock())
        {
            m_pluginOperationOptions.silentMode = false;
        }
    }

    m_pluginOperationStatus.performOperationStatus = S_OK;

    SetEvent(this->m_hPluginWindowDisplayInfoReadyEvent.get());

    SetupPluginWindow();
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
    auto& credMgr = PluginCredentialManager::getInstance();
    credMgr.ReloadRegistryValues();
    {
        std::lock_guard<std::mutex> lock(m_pluginOperationOptionsMutex);
        m_pluginOperationOptions.rpName = rpName;
        m_pluginOperationOptions.operationType = PluginOperationType::GetAssertion;
        m_pluginOperationOptions.matchingCredentials = matchedCreds;
        m_pluginOperationOptions.hWnd = clientHwnd;
        m_pluginOperationOptions.silentMode = credMgr.GetSilentOperation();
        if (m_pluginOperationOptions.matchingCredentials.size() > 1 || credMgr.GetVaultLock())
        {
            m_pluginOperationOptions.silentMode = false;
        }
        else // if there is only one matching credential, select it by default
        {
            m_pluginOperationOptions.selectedCredential = m_pluginOperationOptions.matchingCredentials[0];
        }
    }

    SetEvent(this->m_hPluginWindowDisplayInfoReadyEvent.get());
    SetupPluginWindow();
    return true;
}

winrt::fire_and_forget App::SimulateUnLockVault()
{
    auto mainWindow = m_window.as<MainWindow>();
    auto reenableUI = wil::scope_exit([&]() { mainWindow->EnableUI(); });
    if (GetSilentMode())
    {
        reenableUI.release();
    }
    else
    {
        mainWindow->DisableUI();
    }

    winrt::hresult consentResult;
    auto vaultUnlockMethod = PluginCredentialManager::getInstance().GetVaultUnlockMethod();
    if (vaultUnlockMethod == VaultUnlockMethod::Consent)
    {
        consentResult = co_await mainWindow->RequestConsent(L"Unlock Contoso Passkey Manager Vault");
    }
    else if (vaultUnlockMethod == VaultUnlockMethod::Passkey)
    {
        winrt::apartment_context ui_thread;
        co_await winrt::resume_background();
        consentResult = PluginCredentialManager::getInstance().UnlockCredentialVaultWithPasskey(mainWindow->GetNativeWindowHandle());
        co_await ui_thread;
    }
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
    LOG_IF_FAILED(CoWaitForMultipleHandles(COWAIT_DISPATCH_WINDOW_MESSAGES | COWAIT_DISPATCH_CALLS, INFINITE, 1, m_hPluginOpCompletedEvent.addressof(), &hIndex));

    if (m_window)
    {
        m_window.as<MainWindow>()->UpdatePasskeyOperationStatus(m_pluginOperationStatus.performOperationStatus);
    }

    // Operation completed, continue to next operation
    ResetPluginOperationState();
    
    return true;
}

bool App::PluginCancelAction()
{
    SetEvent(m_hPluginCredentialSelected.get());

    if (m_window)
    {
        m_window.as<MainWindow>()->UpdatePasskeyOperationStatus(NTE_USER_CANCELLED);
    }

    // If cancel was done externally, we will close this instance to reset
    if (!m_window.Visible())
    {
        m_window.Close();
    }

    // Operation completed, continue to next operation
    ResetPluginOperationState();

    return true;
}
}


int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ PWSTR args, _In_ int)
{
    winrt::init_apartment(winrt::apartment_type::single_threaded);

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
