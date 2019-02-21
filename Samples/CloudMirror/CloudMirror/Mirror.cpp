// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

int __cdecl wmain( INT argc, PWSTR argv[] )
{
    wprintf(L"Press ctrl-C to stop gracefully\n");
    wprintf(L"-------------------------------\n");

    winrt::init_apartment();

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
