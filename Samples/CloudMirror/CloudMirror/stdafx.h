// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <Unknwn.h>
#include <winrt/base.h>
#include <shlwapi.h>
#include <pathcch.h>
#include <ShlGuid.h>
#include <ShObjIdl_core.h>
#include <ShlObj_core.h>
#include <cfapi.h>
#include <ntstatus.h>
#include <sddl.h>
#include <winrt\windows.storage.provider.h>
#include <winrt\Windows.Security.Cryptography.h>
#include <ppltasks.h>
#include <strsafe.h>

namespace winrt {
    using namespace Windows::Foundation;
    using namespace Windows::Storage;
    using namespace Windows::Storage::Streams;
    using namespace Windows::Storage::Provider;
    using namespace Windows::Foundation::Collections;
    using namespace Windows::Security::Cryptography;
}

#include "Utilities.h"

#include "DirectoryWatcher.h"
#include "ProviderFolderLocations.h"
#include "FileCopierWithProgress.h"
#include "CloudProviderRegistrar.h"
#include "CloudProviderSyncRootWatcher.h"
#include "Placeholders.h"
#include "ShellServices.h"
#include "FakeCloudProvider.h"
