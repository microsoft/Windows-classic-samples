//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "stdafx.h"
#include "WlanHostedNetworkWinRT.h"

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::Devices::Enumeration;
using namespace ABI::Windows::Devices::WiFiDirect;
using namespace ABI::Windows::Security::Credentials;
using namespace ABI::Windows::Networking;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;


typedef __FITypedEventHandler_2_Windows__CDevices__CWiFiDirect__CWiFiDirectConnectionListener_Windows__CDevices__CWiFiDirect__CWiFiDirectConnectionRequestedEventArgs ConnectionRequestedHandler;
typedef __FITypedEventHandler_2_Windows__CDevices__CWiFiDirect__CWiFiDirectAdvertisementPublisher_Windows__CDevices__CWiFiDirect__CWiFiDirectAdvertisementPublisherStatusChangedEventArgs StatusChangedHandler;
typedef __FITypedEventHandler_2_Windows__CDevices__CWiFiDirect__CWiFiDirectDevice_IInspectable ConnectionStatusChangedHandler;

typedef __FIAsyncOperationCompletedHandler_1_Windows__CDevices__CWiFiDirect__CWiFiDirectDevice FromIdAsyncHandler;

typedef __FIVectorView_1_Windows__CNetworking__CEndpointPair EndpointPairCollection;

WlanHostedNetworkHelper::WlanHostedNetworkHelper()
    : _ssidProvided(false),
      _passphraseProvided(false),
      _listener(nullptr),
      _autoAccept(true)
{
}

WlanHostedNetworkHelper::~WlanHostedNetworkHelper()
{
    if (_publisher.Get() != nullptr)
    {
        _publisher->Stop();
    }
    Reset();
}

void WlanHostedNetworkHelper::Start()
{
    HRESULT hr = S_OK;

    // Clean up old state
    Reset();

    // Create WiFiDirectAdvertisementPublisher
    hr = Windows::Foundation::ActivateInstance(HStringReference(RuntimeClass_Windows_Devices_WiFiDirect_WiFiDirectAdvertisementPublisher).Get(), &_publisher);
    if (FAILED(hr))
    {
        throw WlanHostedNetworkException("ActivateInstance for WiFiDirectAdvertisementPublisher failed", hr);
    }

    // Add event handler for advertisement StatusChanged
    hr = _publisher->add_StatusChanged(
        Callback<StatusChangedHandler>([this](IWiFiDirectAdvertisementPublisher* sender, IWiFiDirectAdvertisementPublisherStatusChangedEventArgs* args) -> HRESULT
    {
        HRESULT hr = S_OK;
        WiFiDirectAdvertisementPublisherStatus status;
        WiFiDirectError error;

        try
        {
            hr = args->get_Status(&status);
            if (FAILED(hr))
            {
                throw WlanHostedNetworkException("Get Status for AdvertisementPubliserStatusChangedEventArgs failed", hr);
            }

            switch (status)
            {
                case WiFiDirectAdvertisementPublisherStatus_Started:
                {
                    // Begin listening for connections and notify listener that the advertisement started
                    StartListener();

                    if (_listener != nullptr)
                    {
                        _listener->OnAdvertisementStarted();
                    }
                    break;
                }
                case WiFiDirectAdvertisementPublisherStatus_Aborted:
                {
                    // Check error and notify listener that the advertisement stopped
                    hr = args->get_Error(&error);
                    if (FAILED(hr))
                    {
                        throw WlanHostedNetworkException("Get Error for AdvertisementPubliserStatusChangedEventArgs failed", hr);
                    }

                    if (_listener != nullptr)
                    {
                        std::wstring message;

                        switch (error)
                        {
                        case WiFiDirectError_RadioNotAvailable:
                            message = L"Advertisement aborted, Wi-Fi radio is turned off";
                            break;

                        case WiFiDirectError_ResourceInUse:
                            message = L"Advertisement aborted, Resource In Use";
                            break;

                        default:
                            message = L"Advertisement aborted, unknown reason";
                            break;
                        }

                        _listener->OnAdvertisementAborted(message);
                    }
                    break;
                }
                case WiFiDirectAdvertisementPublisherStatus_Stopped:
                {
                    // Notify listener that the advertisement is stopped
                    if (_listener != nullptr)
                    {
                        _listener->OnAdvertisementStopped(L"Advertisement stopped");
                    }
                    break;
                }
            }
        }
        catch (WlanHostedNetworkException& e)
        {
            if (_listener != nullptr)
            {
                std::wostringstream ss;
                ss << e.what() << ": " << e.GetErrorCode();
                _listener->OnAsyncException(ss.str());
            }
            return e.GetErrorCode();
        }

        return hr;
    }).Get(), &_statusChangedToken);

    // Set Advertisement required settings

    hr = _publisher->get_Advertisement(_advertisement.GetAddressOf());
    if (FAILED(hr))
    {
        throw WlanHostedNetworkException("Get advertisement for WiFiDirectAdvertisementPublisher failed", hr);
    }

    // Must set the autonomous group owner (GO) enabled flag
    // Legacy Wi-Fi Direct advertisement uses a Wi-Fi Direct GO to act as an access point to legacy settings
    hr = _advertisement->put_IsAutonomousGroupOwnerEnabled(true);
    if (FAILED(hr))
    {
        throw WlanHostedNetworkException("Set is autonomous group owner for WiFiDirectAdvertisement failed", hr);
    }

    hr = _advertisement->get_LegacySettings(_legacySettings.GetAddressOf());
    if (FAILED(hr))
    {
        throw WlanHostedNetworkException("Get legacy settings for WiFiDirectAdvertisement failed", hr);
    }

    // Must enable legacy settings so that non-Wi-Fi Direct peers can connect in legacy mode
    hr = _legacySettings->put_IsEnabled(true);
    if (FAILED(hr))
    {
        throw WlanHostedNetworkException("Set is enabled for WiFiDirectLegacySettings failed", hr);
    }

    HString hstrSSID;
    HString hstrPassphrase;

    // Either specify an SSID, or read the randomly generated one
    if (_ssidProvided)
    {
        hr = hstrSSID.Set(_ssid.c_str());
        if (FAILED(hr))
        {
            throw WlanHostedNetworkException("Failed to create HSTRING representation for SSID", hr);
        }

        hr = _legacySettings->put_Ssid(hstrSSID.Get());
        if (FAILED(hr))
        {
            throw WlanHostedNetworkException("Set SSID for WiFiDirectLegacySettings failed", hr);
        }
    }
    else
    {
        hr = _legacySettings->get_Ssid(hstrSSID.GetAddressOf());
        if (FAILED(hr))
        {
            throw WlanHostedNetworkException("Get SSID for WiFiDirectLegacySettings failed", hr);
        }

        _ssid = hstrSSID.GetRawBuffer(nullptr);
    }

    // Either specify a passphrase, or read the randomly generated one
    ComPtr<IPasswordCredential> passwordCredential;

    hr = _legacySettings->get_Passphrase(passwordCredential.GetAddressOf());
    if (FAILED(hr))
    {
        throw WlanHostedNetworkException("Get Passphrase for WiFiDirectLegacySettings failed", hr);
    }

    if (_passphraseProvided)
    {
        hr = hstrPassphrase.Set(_passphrase.c_str());
        if (FAILED(hr))
        {
            throw WlanHostedNetworkException("Failed to create HSTRING representation for Passphrase", hr);
        }

        hr = passwordCredential->put_Password(hstrPassphrase.Get());
        if (FAILED(hr))
        {
            throw WlanHostedNetworkException("Set Passphrase for WiFiDirectLegacySettings failed", hr);
        }
    }
    else
    {
        hr = passwordCredential->get_Password(hstrPassphrase.GetAddressOf());
        if (FAILED(hr))
        {
            throw WlanHostedNetworkException("Get Passphrase for WiFiDirectLegacySettings failed", hr);
        }

        _passphrase = hstrPassphrase.GetRawBuffer(nullptr);
    }

    // Start the advertisement, which will create an access point that other peers can connect to
    hr = _publisher->Start();
    if (FAILED(hr))
    {
        throw WlanHostedNetworkException("Start WiFiDirectAdvertisementPublisher failed", hr);
    }
}

void WlanHostedNetworkHelper::Stop()
{
    HRESULT hr = S_OK;

    // Call stop on the publisher and expect the status changed callback
    if (_publisher.Get() != nullptr)
    {
        hr = _publisher->Stop();
        if (FAILED(hr))
        {
            throw WlanHostedNetworkException("Stop WiFiDirectAdvertisementPublisher failed", hr);
        }
    }
    else
    {
        throw WlanHostedNetworkException("WiFiDirectAdvertisementPublisher is not running", hr);
    }
}

void WlanHostedNetworkHelper::StartListener()
{
    HRESULT hr = S_OK;

    // Create WiFiDirectConnectionListener
    hr = Windows::Foundation::ActivateInstance(HStringReference(RuntimeClass_Windows_Devices_WiFiDirect_WiFiDirectConnectionListener).Get(), &_connectionListener);
    if (FAILED(hr))
    {
        throw WlanHostedNetworkException("ActivateInstance for WiFiDirectConnectionListener failed", hr);
    }

    hr = _connectionListener->add_ConnectionRequested(
        Callback<ConnectionRequestedHandler>([this](IWiFiDirectConnectionListener* sender, IWiFiDirectConnectionRequestedEventArgs* args) -> HRESULT
    {
        HRESULT hr = S_OK;
        ComPtr<IWiFiDirectConnectionRequest> request;
        ComPtr<IDeviceInformation> deviceInformation;
        HString deviceId;
        ComPtr<IWiFiDirectDeviceStatics> wfdStatics;

        if (_listener != nullptr)
        {
            _listener->LogMessage(L"Connection Requested...");
        }

        bool acceptConnection = true;
        if (!_autoAccept && _prompt != nullptr)
        {
            acceptConnection = _prompt->AcceptIncommingConnection();
        }

        try
        {
            hr = args->GetConnectionRequest(request.GetAddressOf());
            if (FAILED(hr))
            {
                throw WlanHostedNetworkException("Get connection request for ConnectionRequestedEventArgs failed", hr);
            }
            
            if (acceptConnection)
            {
                hr = request->get_DeviceInformation(deviceInformation.GetAddressOf());
                if (FAILED(hr))
                {
                    throw WlanHostedNetworkException("Get device information for ConnectionRequest failed", hr);
                }

                hr = GetActivationFactory(HStringReference(RuntimeClass_Windows_Devices_WiFiDirect_WiFiDirectDevice).Get(), &wfdStatics);
                if (FAILED(hr))
                {
                    throw WlanHostedNetworkException("GetActivationFactory for WiFiDirectDevice failed", hr);
                }

                hr = deviceInformation->get_Id(deviceId.GetAddressOf());
                if (FAILED(hr))
                {
                    throw WlanHostedNetworkException("Get ID for DeviceInformation failed", hr);
                }

                ComPtr<IAsyncOperation<WiFiDirectDevice*>> asyncAction;
                hr = wfdStatics->FromIdAsync(deviceId.Get(), &asyncAction);
                if (FAILED(hr))
                {
                    throw WlanHostedNetworkException("From ID Async for WiFiDirectDevice failed", hr);
                }

                hr = asyncAction->put_Completed(Callback<FromIdAsyncHandler>([this](IAsyncOperation<WiFiDirectDevice*>* pHandler, AsyncStatus status) -> HRESULT
                {
                    HRESULT hr = S_OK;
                    ComPtr<IWiFiDirectDevice> wfdDevice;
                    ComPtr<EndpointPairCollection> endpointPairs;
                    ComPtr<IEndpointPair> endpointPair;
                    ComPtr<IHostName> remoteHostName;
                    HString remoteHostNameDisplay;
                    HString deviceId;

                    try
                    {
                        if (status == AsyncStatus::Completed)
                        {
                            // Get the WiFiDirectDevice object
                            hr = pHandler->GetResults(wfdDevice.GetAddressOf());
                            if (FAILED(hr))
                            {
                                throw WlanHostedNetworkException("Put Completed for FromIDAsync operation failed", hr);
                            }

                            // Now retrieve the endpoint pairs, which includes the IP address assigned to the peer
                            hr = wfdDevice->GetConnectionEndpointPairs(endpointPairs.GetAddressOf());
                            if (FAILED(hr))
                            {
                                throw WlanHostedNetworkException("Get EndpointPairs for WiFiDirectDevice failed", hr);
                            }

                            hr = endpointPairs->GetAt(0, endpointPair.GetAddressOf());
                            if (FAILED(hr))
                            {
                                throw WlanHostedNetworkException("Get first EndpointPair in collection failed", hr);
                            }

                            hr = endpointPair->get_RemoteHostName(remoteHostName.GetAddressOf());
                            if (FAILED(hr))
                            {
                                throw WlanHostedNetworkException("Get Remote HostName for EndpointPair failed", hr);
                            }

                            hr = remoteHostName->get_DisplayName(remoteHostNameDisplay.GetAddressOf());
                            if (FAILED(hr))
                            {
                                throw WlanHostedNetworkException("Get Display Name for Remote HostName failed", hr);
                            }

                            // Add handler for connection status changed
                            EventRegistrationToken statusChangedToken;
                            hr = wfdDevice->add_ConnectionStatusChanged(Callback<ConnectionStatusChangedHandler>([this](IWiFiDirectDevice* sender, IInspectable*) -> HRESULT
                            {
                                WiFiDirectConnectionStatus status;
                                HString deviceId;
                                HRESULT hr = S_OK;

                                try
                                {
                                    hr = sender->get_ConnectionStatus(&status);
                                    if (FAILED(hr))
                                    {
                                        throw WlanHostedNetworkException("Get connection status for peer failed", hr);
                                    }

                                    switch (status)
                                    {
                                    case WiFiDirectConnectionStatus_Connected:
                                        // NO-OP
                                        break;
                                    case WiFiDirectConnectionStatus_Disconnected:
                                        // Clean-up state
                                        hr = sender->get_DeviceId(deviceId.GetAddressOf());
                                        if (FAILED(hr))
                                        {
                                            throw WlanHostedNetworkException("Get Device ID failed", hr);
                                        }

                                        auto itDevice = _connectedDevices.find(deviceId.GetRawBuffer(nullptr));
                                        auto itToken = _connectedDeviceStatusChangedTokens.find(deviceId.GetRawBuffer(nullptr));

                                        if (itToken != _connectedDeviceStatusChangedTokens.end())
                                        {
                                            if (itDevice != _connectedDevices.end())
                                            {
                                                itDevice->second->remove_ConnectionStatusChanged(itToken->second);
                                            }
                                            _connectedDeviceStatusChangedTokens.erase(itToken);
                                        }

                                        if (itDevice != _connectedDevices.end())
                                        {
                                            _connectedDevices.erase(itDevice);
                                        }

                                        // Notify listener of disconnect
                                        if (_listener != nullptr)
                                        {
                                            _listener->OnDeviceDisconnected(deviceId.GetRawBuffer(nullptr));
                                        }

                                        break;
                                    }
                                }
                                catch (WlanHostedNetworkException& e)
                                {
                                    if (_listener != nullptr)
                                    {
                                        std::wostringstream ss;
                                        ss << e.what() << ": " << e.GetErrorCode();
                                        _listener->OnAsyncException(ss.str());
                                    }
                                    return e.GetErrorCode();
                                }

                                return hr;
                            }).Get(), &statusChangedToken);

                            // Store the connected peer
                            hr = wfdDevice->get_DeviceId(deviceId.GetAddressOf());
                            if (FAILED(hr))
                            {
                                throw WlanHostedNetworkException("Get Device ID failed", hr);
                            }

                            _connectedDevices.insert(std::make_pair(deviceId.GetRawBuffer(nullptr), wfdDevice));
                            _connectedDeviceStatusChangedTokens.insert(std::make_pair(deviceId.GetRawBuffer(nullptr), statusChangedToken));

                            // Notify Listener
                            if (_listener != nullptr)
                            {
                                _listener->OnDeviceConnected(remoteHostNameDisplay.GetRawBuffer(nullptr));
                            }
                        }
                        else
                        {
                            if (_listener != nullptr)
                            {
                                switch (status)
                                {
                                case AsyncStatus::Started:
                                    _listener->LogMessage(L"Device connected, status=Started");
                                    break;
                                case AsyncStatus::Canceled:
                                    _listener->LogMessage(L"Device connected, status=Canceled");
                                    break;
                                case AsyncStatus::Error:
                                    _listener->LogMessage(L"Device connected, status=Error");
                                    break;
                                }
                            }
                        }
                    }
                    catch (WlanHostedNetworkException& e)
                    {
                        if (_listener != nullptr)
                        {
                            std::wostringstream ss;
                            ss << e.what() << ": " << e.GetErrorCode();
                            _listener->OnAsyncException(ss.str());
                        }
                        return e.GetErrorCode();
                    }

                    return hr;
                }).Get());
                if (FAILED(hr))
                {
                    throw WlanHostedNetworkException("Put Completed for FromIDAsync operation failed", hr);
                }
            }
            else
            {
                if (_listener != nullptr)
                {
                    _listener->LogMessage(L"Declined");
                }
            }
        }
        catch (WlanHostedNetworkException& e)
        {
            if (_listener != nullptr)
            {
                std::wostringstream ss;
                ss << e.what() << ": " << e.GetErrorCode();
                _listener->OnAsyncException(ss.str());
            }
            return e.GetErrorCode();
        }

        return hr;
    }).Get(), &_connectionRequestedToken);
    if (FAILED(hr))
    {
        throw WlanHostedNetworkException("Add ConnectionRequested handler for WiFiDirectConnectionListener failed", hr);
    }

    if (_listener != nullptr)
    {
        _listener->LogMessage(L"Connection Listener is ready");
    }
}

void WlanHostedNetworkHelper::Reset()
{
    if (_connectionListener.Get() != nullptr)
    {
        _connectionListener->remove_ConnectionRequested(_connectionRequestedToken);
    }

    if (_publisher.Get() != nullptr)
    {
        _publisher->remove_StatusChanged(_statusChangedToken);
    }

    _legacySettings.Reset();
    _advertisement.Reset();
    _publisher.Reset();
    _connectionListener.Reset();

    _connectedDevices.clear();
}
