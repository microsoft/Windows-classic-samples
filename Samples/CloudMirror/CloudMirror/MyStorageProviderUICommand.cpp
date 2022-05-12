// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"
#include "MyStorageProviderUICommand.h"

namespace winrt
{
    using namespace ::winrt::Windows::Storage::Provider;
}

namespace winrt::CloudMirror::implementation
{
    void MyStorageProviderUICommand::Invoke()
    {
        // Your provider should perform the command the user requested.
        // This sample just prints a message.
        wprintf(L"Execute the command \"%ls\"\n", m_label.c_str());
    }
}

