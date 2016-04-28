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

#pragma once

#include "WlanHostedNetworkWinRT.h"

/// A simple console helper to take commands and start the "soft AP"
class SimpleConsole : public IWlanHostedNetworkListener, public IWlanHostedNetworkPrompt
{
public:
    SimpleConsole();
    virtual ~SimpleConsole();

    void RunConsole();

    // IWlanHostedNetworkListener Implementation

    virtual void OnDeviceConnected(std::wstring remoteHostName) override;
    virtual void OnDeviceDisconnected(std::wstring deviceId) override;

    virtual void OnAdvertisementStarted() override;
    virtual void OnAdvertisementStopped(std::wstring message) override;
    virtual void OnAdvertisementAborted(std::wstring message) override;

    virtual void OnAsyncException(std::wstring message) override;

    virtual void LogMessage(std::wstring message) override;

    // IWlanHostedNetworkPrompt Implementation

    virtual bool AcceptIncommingConnection() override;

private:
    void ShowPrompt();
    void ShowHelp();
    bool ExecuteCommand(std::wstring command);

    WlanHostedNetworkHelper _hostedNetwork;

    // Event helper to wait on async operations in console
    Microsoft::WRL::Wrappers::Event _apEvent;
};
