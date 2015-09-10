// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// dllmain.h : Declaration of module class.

static const wchar_t SDK_SHA_progId[] = L"SdkShaInfo.ComponentInfo";
static const wchar_t SDK_SHA_friendlyName[] = L"ComponentInfo Class";

static const wchar_t SDK_SHA_appID[] = L"{71EB2D90-9A3F-4e5b-99A5-5A03E18F0F92}";
static const wchar_t SDK_SHA_typeLib[] = L"{74CCF207-06E3-4424-9D72-6F43A8D02FB7}";

class CSdkShaInfoModule
{
public:
    /// Add entries to registry
    HRESULT RegisterServer();

    /// Remove entries from registry
    HRESULT UnregisterServer();

};

extern CSdkShaInfoModule sdkModule;
