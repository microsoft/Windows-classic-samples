// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once
class FakeCloudProvider
{
public:
    static bool Start(
        _In_opt_ LPCWSTR serverFolder = nullptr, 
        _In_opt_ LPCWSTR clientFolder = nullptr);

private:
    static void ConnectSyncRootTransferCallbacks();
    static void DisconnectSyncRootTransferCallbacks();

    static void CALLBACK OnFetchData(
        _In_ CONST CF_CALLBACK_INFO* callbackInfo,
        _In_ CONST CF_CALLBACK_PARAMETERS* callbackParameters);

    static void CALLBACK OnCancelFetchData(
        _In_ CONST CF_CALLBACK_INFO* callbackInfo,
        _In_ CONST CF_CALLBACK_PARAMETERS* callbackParameters);

private:
    static CF_CONNECTION_KEY s_transferCallbackConnectionKey;
    static CF_CALLBACK_REGISTRATION s_MirrorCallbackTable[];
};

