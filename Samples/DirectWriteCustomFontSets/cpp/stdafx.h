//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************


// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

// Define used to determine Win32 rather than WinRT implementation. This is used within DXHelper.h.
// Don't define in a WinRT project.
#ifndef THROW_SYSTEM_ERROR
#define THROW_SYSTEM_ERROR
#endif // !THROW_SYSTEM_ERROR


// Define used to force code paths that would be available on Windows 10 versions prior to the 
// Creators Update (preview build 15021). Only define if you want to force those code paths.
//#ifndef FORCE_TH1_IMPLEMENTATION
//#define FORCE_TH1_IMPLEMENTATION
//#endif // !FORCE_TH1_IMPLEMENTATION


// This will get added directly into the Windows 10 Creators Update SDK closer to release, but it 
// needs to be included here until then.
#undef NTDDI_VERSION
#define NTDDI_VERSION NTDDI_WIN10_RS2


#include "targetver.h"

#include <dwrite_3.h>
#include <filesystem>
#include <iostream>
#include <map> // std::map, used in CommandLineArgs
#include <stdio.h>
#include <string>
#include <vector>
#include <wrl/client.h> // Microsoft::WRL, for ComPtr; could also use ATL CComPtr or raw pointers, but not std::shared_ptr


// The project linker settings have been modified to list dwrite.lib as an additional dependency. (Note: this needs to be
// done for all configurations and platforms.) Alternatively, the following pragma can be used to declare the dependency:
// #pragma comment(lib, "dwrite")