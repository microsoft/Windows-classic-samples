// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

class DirectoryWatcher
{
public:
    void Initialize(_In_ PCWSTR path, _In_ std::function<void(std::list<std::wstring>&)> callback);
    winrt::Windows::Foundation::IAsyncAction ReadChangesAsync();
    void Cancel();

private:
    winrt::Windows::Foundation::IAsyncAction ReadChangesInternalAsync();

    winrt::handle _dir;
    std::wstring _path;
    std::unique_ptr<FILE_NOTIFY_INFORMATION> _notify;
    OVERLAPPED _overlapped{};
    winrt::Windows::Foundation::IAsyncAction _readTask;
    std::function<void(std::list<std::wstring>&)> _callback;
};

