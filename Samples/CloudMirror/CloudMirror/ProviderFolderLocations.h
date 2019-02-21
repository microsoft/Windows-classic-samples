// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once
class ProviderFolderLocations
{
public:
    static bool Init(
        _In_opt_ LPCWSTR serverFolder = nullptr,
        _In_opt_ LPCWSTR clientFolder = nullptr);

    static LPCWSTR GetServerFolder() { return s_serverFolder.data(); }
    static LPCWSTR GetClientFolder() { return s_clientFolder.data(); }

private:
    static std::wstring PromptForFolderPath(_In_ PCWSTR title);

    static std::wstring s_serverFolder;
    static std::wstring s_clientFolder;

};

