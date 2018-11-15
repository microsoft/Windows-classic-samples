/*++

Copyright (c) Microsoft Corporation

Module Name:

    main.cpp

Abstract:

    This implements the wmain() function for the RegFS provider.  It sets up the RegfsProvider object
    and starts the provider.

--*/

#include "stdafx.h"

using namespace regfs;

int __cdecl wmain(int argc, const WCHAR **argv)
{
    if (argc <= 1)
    {
        wprintf(L"Usage: \n");
        wprintf(L"> regfs.exe <Virtualization Root Path> \n");

        return -1;
    }

    // argv[1] should be the path to the virtualization root.
    std::wstring rootPath = argv[1];

    // Specify the notifications that we want ProjFS to send to us.  Everywhere under the virtualization
    // root we want ProjFS to tell us when files have been opened, when they're about to be renamed,
    // and when they're about to be deleted.
    PRJ_NOTIFICATION_MAPPING notificationMappings[1] = {};
    notificationMappings[0].NotificationRoot = L"";
    notificationMappings[0].NotificationBitMask = PRJ_NOTIFY_FILE_OPENED |
                                                  PRJ_NOTIFY_PRE_RENAME |
                                                  PRJ_NOTIFY_PRE_DELETE;

    // Store the notification mapping we set up into a start options structure.  We leave all the
    // other options at their defaults.  
    PRJ_STARTVIRTUALIZING_OPTIONS opts = {};
    opts.NotificationMappings = notificationMappings;
    opts.NotificationMappingsCount = 1;

    // Start the provider using the options we set up.
    RegfsProvider provider;
    auto hr = provider.Start(rootPath.c_str(), &opts);
    if (FAILED(hr))
    {
        wprintf(L"Failed to start virtualization instance: 0x%08x\n", hr);
        return -1;
    }

    wprintf(L"RegFS is running at virtualization root [%s]\n", rootPath.c_str());
    wprintf(L"Press Enter to stop the provider...");

    getchar();

    provider.Stop();

    return 0;
};
