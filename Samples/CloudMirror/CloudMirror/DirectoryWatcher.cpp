// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

const size_t c_bufferSize = sizeof(FILE_NOTIFY_INFORMATION) * 100;

// Run of the mill directory watcher to signal when user causes things to
// happen in the client folder sync root

void DirectoryWatcher::Initalize(
    _In_ PCWSTR path, 
    _In_ std::function<void(std::list<std::wstring>&)> callback)
{
    _path = path;
    _notify.reset(reinterpret_cast<FILE_NOTIFY_INFORMATION*>(new char[c_bufferSize]));

    _callback = callback;

    _dir.attach(CreateFile(path,
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        nullptr));
    if (_dir.get() == INVALID_HANDLE_VALUE)
    {
        throw winrt::hresult_error(HRESULT_FROM_WIN32(GetLastError()));
    }
}

concurrency::task<void> DirectoryWatcher::ReadChangesAsync()
{
    auto token = _cancellationTokenSource.get_token();
    return concurrency::create_task([this, token]
    {
        while (true)
        {
            DWORD returned;
            winrt::check_bool(ReadDirectoryChangesW(
                _dir.get(),
                _notify.get(),
                c_bufferSize,
                TRUE,
                FILE_NOTIFY_CHANGE_ATTRIBUTES,
                &returned,
                &_overlapped,
                nullptr));

            DWORD transferred;
            if (GetOverlappedResultEx(_dir.get(), &_overlapped, &transferred, 1000, FALSE))
            {
                std::list<std::wstring> result;
                FILE_NOTIFY_INFORMATION* next = _notify.get();
                while (next != nullptr)
                {
                    std::wstring fullPath(_path);
                    fullPath.append(L"\\");
                    fullPath.append(std::wstring_view(next->FileName, next->FileNameLength / sizeof(wchar_t)));
                    result.push_back(fullPath);

                    if (next->NextEntryOffset)
                    {
                        next = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(reinterpret_cast<char*>(next) + next->NextEntryOffset);
                    }
                    else
                    {
                        next = nullptr;
                    }
                }
                _callback(result);
            }
            else if (GetLastError() != WAIT_TIMEOUT)
            {
                throw winrt::hresult_error(HRESULT_FROM_WIN32(GetLastError()));
            }
            else if (token.is_canceled())
            {
                wprintf(L"watcher cancel received\n");
                concurrency::cancel_current_task();
                return;
            }
        }
    }, token);
}

void DirectoryWatcher::Cancel()
{
    wprintf(L"Canceling watcher\n");
    _cancellationTokenSource.cancel();
}

