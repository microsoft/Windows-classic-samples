// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once
class CloudProviderSyncRootWatcher
{
public:
    static void WatchAndWait();
    static BOOL WINAPI Stop(DWORD reason);

private:
    static void InitDirectoryWatcher();
    static void OnSyncRootFileChanges(_In_ std::list<std::wstring>& changes);

    static DirectoryWatcher s_directoryWatcher;
    static bool s_shutdownWatcher;
};

