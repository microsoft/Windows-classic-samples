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

#pragma once

#include <windows.h>
#include <strsafe.h>
#include <mmdeviceapi.h>

#if !defined(NTDDI_WIN10_NI) || (NTDDI_VERSION < NTDDI_WIN10_NI)
#error This sample uses IAudioViewManagerService which requires SDK version 10.0.22470.0 or higher.
#endif

#include <new>
#include <forward_list>
#include <memory>
