////////////////////////////////////////////////////////////////////////////////
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
////////////////////////////////////////////////////////////////////////////////
#pragma once

EXTERN_C HINSTANCE g_hInstance;

// Define the unique COM id for the COM object
class __declspec(uuid("a05d3c0d-590b-49e5-92d7-053917a218b6")) DeviceContextMenu;

VOID DllIncLockCount();
VOID DllDecLockCount();
