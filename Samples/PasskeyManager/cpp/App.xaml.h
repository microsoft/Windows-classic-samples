// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once

#include "App.xaml.g.h"
#include "MainWindow.xaml.h"
#include "MakeCredentialPage.xaml.h"
#include <synchapi.h>
#include <pluginauthenticator.h>
#include <PluginAuthenticator/PluginAuthenticatorImpl.h>

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

    struct PluginOpertaionOptions
    {
        // define default constructor
        PluginOpertaionOptions() :
            hWnd(nullptr),
            operationType(PluginOperationType::MakeCredential),
            selectedCredential(nullptr),
            transactionId(GUID_NULL),
            silentMode(false)
        {
        };
        HWND hWnd;
        std::wstring rpName;
        std::wstring userName;
        PluginOperationType operationType;
        GUID transactionId;
        std::vector<const WEBAUTHN_CREDENTIAL_DETAILS*> matchingCredentials;
        const WEBAUTHN_CREDENTIAL_DETAILS* selectedCredential;
        bool silentMode;
    };

    struct App : AppT<App>
    {
        App();
        App(PWSTR args);

        wil::shared_event m_hAppReadyForPluginOpEvent { wil::EventOptions::ManualReset };
        wil::shared_event m_hPluginOpCompletedEvent{ wil::EventOptions::ManualReset };
        wil::unique_com_class_object_cookie m_registration;

        Microsoft::UI::Xaml::Controls::Frame CreateRootFrame();
        void InitializeAppWindTitleBar();
        void OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);
        void OnNavigationFailed(IInspectable const&, Microsoft::UI::Xaml::Navigation::NavigationFailedEventArgs const&);
        HWND GetNativeWindowHandle();
        bool SetPluginPerformOperationOptions(HWND hWnd, PluginOperationType operationType, std::wstring rpName, std::wstring userName);

        bool SetMatchingCredentials(std::wstring_view rpName, const std::vector<const WEBAUTHN_CREDENTIAL_DETAILS*>& matchedCreds, HWND hwnd);
        bool SetSelectedCredentialId(Windows::Storage::Streams::IBuffer credentialId);

        void SetPluginTransactionId(GUID requestTransactionId)
        {
            std::lock_guard<std::mutex> lock(m_pluginOperationOptionsMutex);
            m_pluginOperationOptions.transactionId = requestTransactionId;
        }

        auto GetDispatcherQueue() { return m_dispatcherQueue; }
        Microsoft::UI::Dispatching::DispatcherQueue m_dispatcherQueue = Microsoft::UI::Dispatching::DispatcherQueue::GetForCurrentThread();

        std::atomic<bool> m_isOperationInProgress{ false };
        wil::unique_event m_hPluginProceedButtonEvent{ wil::EventOptions::ManualReset };
        wil::shared_event m_hPluginCancelOperationEvent{ wil::EventOptions::ManualReset };
        wil::unique_event m_hPluginWindowDisplayInfoReadyEvent{ wil::EventOptions::ManualReset };
        wil::unique_event m_hWindowReady{ wil::EventOptions::ManualReset };
        wil::unique_event m_hPluginCredentialSelected{ wil::EventOptions::ManualReset };
        wil::unique_event m_hVaultConsentComplete{ wil::EventOptions::ManualReset };
        wil::unique_event m_hVaultConsentFailed{ wil::EventOptions::ManualReset };
        winrt::fire_and_forget SimulateUnLockVault();

        bool GetSilentMode()
        {
            return m_pluginOperationOptions.silentMode;
        }

        GUID GetPluginTransactionId()
        {
            return m_pluginOperationOptions.transactionId;
        }

        void RegisterPluginClassFactory();
        bool PluginCompleteAction();
        bool PluginCancelAction();

        void ResetPluginOperationState();
        void CloseOrHideWindow();
        void SetupPluginWindow();
        void HandlePluginOperations();

        PluginOperationStatus m_pluginOperationStatus;
        winrt::Microsoft::UI::Xaml::Window m_window{ nullptr };
        hstring m_args;
        std::mutex m_pluginOperationOptionsMutex;
        _Guarded_by_(m_pluginOperationOptionsMutex) PluginOpertaionOptions m_pluginOperationOptions;
    };
}
