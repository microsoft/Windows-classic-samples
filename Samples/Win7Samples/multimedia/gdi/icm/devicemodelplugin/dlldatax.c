//+--------------------------------------------------------------------------
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Abstract:
//      Wrapper for dlldata.c setting REGISTER_PROXY_DLL to build
//      interface proxies
//
//----------------------------------------------------------------------------

//
// merge proxy stub DLL
//
#ifdef _MERGE_PROXYSTUB 
// 
// DllRegisterServer, etc.
//
#define REGISTER_PROXY_DLL

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600    // Windows Vista or higher.
#endif

#ifndef NTDDI_VERSION
#define NTDDI_VERSION NTDDI_LONGHORN    // Windows Vista or higher.
#endif

#define USE_STUBLESS_PROXY

#pragma comment(lib, "rpcns4.lib")
#pragma comment(lib, "rpcrt4.lib")

#define ENTRY_PREFIX	Prx

#include "dlldata.c"
#include "DeviceModelPluginSample_p.c"

#endif
