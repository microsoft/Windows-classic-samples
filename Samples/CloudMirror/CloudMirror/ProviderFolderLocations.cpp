// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

//===============================================================
// ProviderFolderLocations
//
//   Manages the locations of the folders where the syncroot
//   and "cloud" live.
//
// Fakery Factor:
//
//   You will likely rewrite all of this. But, look on the bright
//   side: This is a tiny class that does barely anything.
//
//===============================================================

std::wstring ProviderFolderLocations::s_serverFolder;
std::wstring ProviderFolderLocations::s_clientFolder;

bool ProviderFolderLocations::Init(_In_opt_ LPCWSTR serverFolder, _In_opt_ LPCWSTR clientFolder)
{
    if (serverFolder)
    {
        s_serverFolder = serverFolder;
    }
    if (clientFolder)
    {
        s_clientFolder = clientFolder;
    }
    if (s_serverFolder.empty())
    {
        s_serverFolder = PromptForFolderPath(L"\"Server in the Fluffy Cloud\" Location");
    }

    if (!s_serverFolder.empty() && s_clientFolder.empty())
    {
        s_clientFolder = PromptForFolderPath(L"\"Syncroot (Client)\" Location");
    }

    auto result{ false };
    if (!s_serverFolder.empty() && !s_clientFolder.empty())
    {
        // In case they were passed in params we may need to create the folder.
        // If the folder is already there then these are benign calls.
        CreateDirectory(s_serverFolder.c_str(), NULL);
        CreateDirectory(s_clientFolder.c_str(), NULL);
        result = true;
    }
    return result;
}

std::wstring ProviderFolderLocations::PromptForFolderPath(_In_ PCWSTR title)
{
    winrt::com_ptr<IFileOpenDialog> fileOpen;
    winrt::check_hresult(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, __uuidof(&fileOpen), fileOpen.put_void()));
    winrt::check_hresult(fileOpen->SetOptions(FOS_PICKFOLDERS));
    winrt::check_hresult(fileOpen->SetTitle(title));

    // Restore last location used
    auto settings = winrt::Windows::Storage::ApplicationData::Current().LocalSettings().Values();
    if (settings.HasKey(title))
    {
        auto lastLocation = winrt::unbox_value<winrt::hstring>(settings.Lookup(title));
        winrt::com_ptr<IShellItem> lastItem;
        winrt::check_hresult(SHCreateItemFromParsingName(lastLocation.c_str(), nullptr, __uuidof(lastItem), lastItem.put_void()));
        winrt::check_hresult(fileOpen->SetFolder(lastItem.get()));
    }

    try
    {
        winrt::check_hresult(fileOpen->Show(nullptr));
    }
    catch (winrt::hresult_canceled)
    {
        return std::wstring();
    }

    winrt::com_ptr<IShellItem> item;
    winrt::check_hresult(fileOpen->GetResult(item.put()));

    winrt::com_array<wchar_t> path;
    winrt::check_hresult(item->GetDisplayName(SIGDN_FILESYSPATH, winrt::put_abi(path)));

    // Save the last location
    settings.Insert(title, winrt::box_value(path.data()));

    return std::wstring(path.data());
}
