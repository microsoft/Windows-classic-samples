// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

//===============================================================
// CloudProviderSyncRootWatcher
//
//   This class watches for any changes that happen to files
//   and folders in the Sync Root on the client machine. This
//   allows for hydration to be signalled or other actions.
//
// Fakery Factor:
//
//   This class is pretty usable as-is. You will probably want to
//   get rid of that whole Ctrl-C shenanigans thing to stop the
//   watcher and replace it with code that's called by some UI
//   you do to uninstall.
//
//===============================================================

DirectoryWatcher CloudProviderSyncRootWatcher::s_directoryWatcher;
bool CloudProviderSyncRootWatcher::s_shutdownWatcher;

void CloudProviderSyncRootWatcher::WatchAndWait()
{
    //  Main loop - wait for Ctrl+C or our named event to be signaled
    SetConsoleCtrlHandler(Stop, TRUE);
    InitDirectoryWatcher();

    while (true)
    {
        try
        {
            auto task = s_directoryWatcher.ReadChangesAsync();

            while (!task.is_done())
            {
                Sleep(1000);

                if (s_shutdownWatcher)
                {
                    s_directoryWatcher.Cancel();
                    task.wait();
                }
            }

            if (s_shutdownWatcher)
            {
                break;
            }
        }
        catch (...)
        {
            wprintf(L"CloudProviderSyncRootWatcher watcher failed.\n");
            throw;
        }
    }
}

void CloudProviderSyncRootWatcher::OnSyncRootFileChanges(_In_ std::list<std::wstring>& changes)
{
    for (auto path : changes)
    {
        wprintf(L"Processing change for %s\n", path.c_str());

        DWORD attrib = GetFileAttributes(path.c_str());
        if (!(attrib & FILE_ATTRIBUTE_DIRECTORY))
        {
            winrt::handle placeholder(CreateFile(path.c_str(), 0, FILE_READ_DATA, nullptr, OPEN_EXISTING, 0, nullptr));

            LARGE_INTEGER offset = {};
            LARGE_INTEGER length;
            length.QuadPart = MAXLONGLONG;

            if (attrib & FILE_ATTRIBUTE_PINNED)
            {
                wprintf(L"Hydrating file %s\n", path.c_str());
                CfHydratePlaceholder(placeholder.get(), offset, length, CF_HYDRATE_FLAG_NONE, NULL);
            }
            else if (attrib & FILE_ATTRIBUTE_UNPINNED)
            {
                wprintf(L"Dehydrating file %s\n", path.c_str());
                CfDehydratePlaceholder(placeholder.get(), offset, length, CF_DEHYDRATE_FLAG_NONE, NULL);
            }
        }
    }
}

void CloudProviderSyncRootWatcher::InitDirectoryWatcher()
{
    // Set up a Directory Watcher on the client side to handle user's changing things there
    try
    {
        s_directoryWatcher.Initalize(ProviderFolderLocations::GetClientFolder(), OnSyncRootFileChanges);
    }
    catch (...)
    {
        wprintf(L"Could not init directory watcher.\n");
        throw;
    }
}

BOOL WINAPI
CloudProviderSyncRootWatcher::Stop(DWORD /*dwReason*/)
{
    s_shutdownWatcher = TRUE;
    return TRUE;
}

