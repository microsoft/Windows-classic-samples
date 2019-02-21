// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

//===============================================================
// CloudProviderRegistrar
//
//   This class registers the provider with the Shell so that 
//   the syncroot shows up.
//
// Fakery Factor:
//
//   You should be able to replace the strings with your real values
//   and then use this class as-is.
//
//===============================================================

#define STORAGE_PROVIDER_ID L"TestStorageProvider"
#define STORAGE_PROVIDER_ACCOUNT L"TestAccount1"

void CloudProviderRegistrar::RegisterWithShell()
{
    try
    {
        auto syncRootID = GetSyncRootId();

        winrt::StorageProviderSyncRootInfo info;
        info.Id(syncRootID);

        auto folder = winrt::StorageFolder::GetFolderFromPathAsync(ProviderFolderLocations::GetClientFolder()).get();
        info.Path(folder);

        info.DisplayNameResource(L"TestStorageProviderDisplayName");
        // This icon is just for the sample. You should provide your own branded icon here
        info.IconResource(L"%SystemRoot%\\system32\\charmap.exe,0");
        info.HydrationPolicy(winrt::StorageProviderHydrationPolicy::Full);
        info.HydrationPolicyModifier(winrt::StorageProviderHydrationPolicyModifier::None);
        info.PopulationPolicy(winrt::StorageProviderPopulationPolicy::AlwaysFull);
        info.InSyncPolicy(winrt::StorageProviderInSyncPolicy::FileCreationTime | winrt::StorageProviderInSyncPolicy::DirectoryCreationTime);
        info.Version(L"1.0.0");
        info.ShowSiblingsAsGroup(false);
        info.HardlinkPolicy(winrt::StorageProviderHardlinkPolicy::None);

        winrt::Uri uri(L"http://cloudmirror.example.com/recyclebin");
        info.RecycleBinUri(uri);

        // Context
        std::wstring syncRootIdentity(ProviderFolderLocations::GetServerFolder());
        syncRootIdentity.append(L"->");
        syncRootIdentity.append(ProviderFolderLocations::GetClientFolder());

        wchar_t const contextString[] = L"TestProviderContextString";
        winrt::IBuffer contextBuffer = winrt::CryptographicBuffer::ConvertStringToBinary(syncRootIdentity.data(), winrt::BinaryStringEncoding::Utf8);
        info.Context(contextBuffer);

        winrt::IVector<winrt::StorageProviderItemPropertyDefinition> customStates = info.StorageProviderItemPropertyDefinitions();
        AddCustomState(customStates, L"CustomStateName1", 1);
        AddCustomState(customStates, L"CustomStateName2", 2);
        AddCustomState(customStates, L"CustomStateName3", 3);

        winrt::StorageProviderSyncRootManager::Register(info);

        // Give the cache some time to invalidate
        Sleep(1000);
    }
    catch (...)
    {
        // winrt::to_hresult() will eat the exception if it is a result of winrt::check_hresult,
        // otherwise the exception will get rethrown and this method will crash out as it should
        wprintf(L"Could not register the sync root, hr %08x\n", static_cast<HRESULT>(winrt::to_hresult()));
    }
}

//  A real sync engine should NOT unregister the sync root upon exit.
//  This is just to demonstrate the use of StorageProviderSyncRootManager::Unregister.
void CloudProviderRegistrar::Unregister()
{
    try
    {
        winrt::StorageProviderSyncRootManager::Unregister(GetSyncRootId());
    }
    catch (...)
    {
        // winrt::to_hresult() will eat the exception if it is a result of winrt::check_hresult,
        // otherwise the exception will get rethrown and this method will crash out as it should
        wprintf(L"Could not unregister the sync root, hr %08x\n", static_cast<HRESULT>(winrt::to_hresult()));
    }
}

std::unique_ptr<TOKEN_USER> CloudProviderRegistrar::GetTokenInformation()
{
    std::unique_ptr<TOKEN_USER> tokenInfo;

    // get the tokenHandle from current thread/process if it's null
    auto tokenHandle{ GetCurrentThreadEffectiveToken() }; // Pseudo token, don't free.

    DWORD tokenInfoSize{ 0 };
    if (!::GetTokenInformation(tokenHandle, TokenUser, nullptr, 0, &tokenInfoSize))
    {
        if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            tokenInfo.reset(reinterpret_cast<TOKEN_USER*>(new char[tokenInfoSize]));
            if (!::GetTokenInformation(tokenHandle, TokenUser, tokenInfo.get(), tokenInfoSize, &tokenInfoSize))
            {
                throw std::exception("GetTokenInformation failed");
            }
        }
        else
        {
            throw std::exception("GetTokenInformation failed");
        }
    }
    return tokenInfo;
}

std::wstring CloudProviderRegistrar::GetSyncRootId()
{
    std::unique_ptr<TOKEN_USER> tokenInfo(GetTokenInformation());
    auto sidString = Utilities::ConvertSidToStringSid(tokenInfo->User.Sid);
    std::wstring syncRootID(STORAGE_PROVIDER_ID);
    syncRootID.append(L"!");
    syncRootID.append(sidString.data());
    syncRootID.append(L"!");
    syncRootID.append(STORAGE_PROVIDER_ACCOUNT);

    return syncRootID;
}

void CloudProviderRegistrar::AddCustomState(
    _In_ winrt::IVector<winrt::StorageProviderItemPropertyDefinition>& customStates,
    _In_ LPCWSTR displayNameResource,
    _In_ int id)
{
    winrt::StorageProviderItemPropertyDefinition customState;
    customState.DisplayNameResource(displayNameResource);
    customState.Id(id);
    customStates.Append(customState);
}

