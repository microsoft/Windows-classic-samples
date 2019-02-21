// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once
class CloudProviderRegistrar
{
public:
    static void RegisterWithShell();
    static void Unregister();

private:
    static std::unique_ptr<TOKEN_USER> GetTokenInformation();
    static std::wstring GetSyncRootId();
    static void AddCustomState(
        _In_ winrt::IVector<winrt::StorageProviderItemPropertyDefinition>& customStates,
        _In_ LPCWSTR displayNameResource,
        _In_ int id);
};

