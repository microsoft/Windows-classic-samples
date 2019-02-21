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
    void Initalize(_In_ PCWSTR path, _In_ std::function<void(std::list<std::wstring>&)> callback);
    concurrency::task<void> ReadChangesAsync();
    void Cancel();

private:
    winrt::handle _dir;
    std::wstring _path;
    std::unique_ptr<FILE_NOTIFY_INFORMATION> _notify;
    OVERLAPPED _overlapped;
    concurrency::cancellation_token_source _cancellationTokenSource;
    std::function<void(std::list<std::wstring>&)> _callback;
};

