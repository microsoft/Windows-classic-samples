// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

int __cdecl wmain( INT argc, PWSTR argv[] )
{
    winrt::init_apartment();

    // Detect a common debugging error up front.
    try
    {
        // If the program was launched incorrectly, this will throw.
        (void)winrt::Windows::Storage::ApplicationData::Current();
    }
    catch (...)
    {
        wprintf(L"This program should be launched from the Start menu, not from Visual Studio.\n");
        return 1;
    }

    wprintf(L"Press ctrl-C to stop gracefully\n");
    wprintf(L"-------------------------------\n");

    auto returnCode{ 0 };

    try
    {
        if (FakeCloudProvider::Start())
        {
            returnCode = 1;
        }
    }
    catch (...)
    {
        CloudProviderSyncRootWatcher::Stop(0); // Param is unused
    }

    return returnCode;
}
