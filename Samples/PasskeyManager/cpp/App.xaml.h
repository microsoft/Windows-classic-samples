// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once

#include "App.xaml.g.h"
#include "MainWindow.xaml.h"
#include "MakeCredentialPage.xaml.h"
#include <synchapi.h>
#include <include/webauthn/pluginauthenticator.h>

namespace winrt::PasskeyManager::implementation
{
    struct PluginOperationStatus
    {
        PluginOperationStatus() :
            performOperationStatus(S_OK),
            uvSignatureVerificationStatus(S_OK),
            requestSignatureVerificationStatus(S_OK)
        {};
        HRESULT performOperationStatus;
        HRESULT uvSignatureVerificationStatus;
        HRESULT requestSignatureVerificationStatus;
    };

    // define a typedefed enum of operation type with two value for make credential and get assertion
    typedef enum _PluginOperationType
    {
        PLUGIN_OPERATION_TYPE_MAKE_CREDENTIAL = 0,
        PLUGIN_OPERATION_TYPE_GET_ASSERTION = 1
    } PluginOperationType;

    struct PluginOpertaionOptions
    {
        // define default constructor
        PluginOpertaionOptions() :
            hWnd(nullptr),
            operationType(PluginOperationType::PLUGIN_OPERATION_TYPE_MAKE_CREDENTIAL),
            selectedCredential(nullptr)
        {};
        HWND hWnd;
        std::wstring rpName;
        std::wstring userName;
        PluginOperationType operationType;
        std::vector<const WEBAUTHN_CREDENTIAL_DETAILS*> matchingCredentials;
        const WEBAUTHN_CREDENTIAL_DETAILS* selectedCredential;
    };

    struct App : AppT<App>
    {
        App();
        App(PWSTR args);

        inline static wil::unique_event s_pluginOpRequestRecievedEvent { wil::EventOptions::ManualReset };
        inline static wil::unique_event s_hAppReadyForPluginOpEvent { wil::EventOptions::ManualReset };
        inline static wil::unique_event s_hPluginOpCompletedEvent{ wil::EventOptions::ManualReset };
        inline static wil::unique_com_class_object_cookie m_registration;

        Microsoft::UI::Xaml::Controls::Frame CreateRootFrame();
        void InitializeAppWindTitleBar();
        void OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);
        void OnNavigationFailed(IInspectable const&, Microsoft::UI::Xaml::Navigation::NavigationFailedEventArgs const&);
        HWND GetNativeWindowHandle();
        bool SetPluginPerformOperationOptions(HWND hWnd, PluginOperationType operationType, std::wstring rpName, std::wstring userName);

        bool SetMatchingCredentials(std::wstring_view rpName, const std::vector<const WEBAUTHN_CREDENTIAL_DETAILS*>& matchedCreds, HWND hwnd);
        bool SetSelectedCredentialId(Windows::Storage::Streams::IBuffer credentialId);

        auto GetDispatcherQueue() { return m_dispatcherQueue; }
        Microsoft::UI::Dispatching::DispatcherQueue m_dispatcherQueue = Microsoft::UI::Dispatching::DispatcherQueue::GetForCurrentThread();

        wil::unique_event m_hPluginProceedButtonEvent{ wil::EventOptions::ManualReset };
        wil::unique_event m_hPluginUserCancelEvent{ wil::EventOptions::ManualReset };
        wil::unique_event m_hPluginOpReqDisplayInfoParsedEvent{ wil::EventOptions::ManualReset };
        wil::unique_event m_hWindowReady{ wil::EventOptions::ManualReset };
        wil::unique_event m_hPluginCredentialSelected{ wil::EventOptions::ManualReset };
        wil::unique_event m_hVaultConsentComplete{ wil::EventOptions::ManualReset };
        wil::unique_event m_hVaultConsentFailed{ wil::EventOptions::ManualReset };
        winrt::fire_and_forget SimulateUnLockVaultUsingConsentVerifier();

        bool m_silentMode = false;
        bool GetSilentMode() const
        {
            return m_silentMode;
        }

        static void register_plugin_class_factory();
        bool PluginCompleteAction();
        bool PluginCancelAction();
    public:
        PluginOperationStatus m_pluginOperationStatus;
        winrt::Microsoft::UI::Xaml::Window m_window{ nullptr };
        hstring m_args;
        std::mutex m_pluginOperationOptionsMutex;
        _Guarded_by_(m_pluginOperationOptionsMutex) PluginOpertaionOptions m_pluginOperationOptions;
    };
}
