// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


// wrapper for dlldata.c

#ifdef _MERGE_PROXYSTUB // merge proxy stub DLL

#define REGISTER_PROXY_DLL //DllRegisterServer, etc.

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500	//for Win2000, change it to 0x0400 for NT4 or Win95 with DCOM
#endif
#define USE_STUBLESS_PROXY	//defined only with MIDL switch /Oicf

#pragma comment(lib, "rpcns4.lib")
#pragma comment(lib, "rpcrt4.lib")

#define ENTRY_PREFIX	Prx

#pragma warning(disable: 4100) // warning C4100: 'lpvReserved' : unreferenced formal parameter
#pragma warning(disable: 4152) // warning C4152: nonstandard extension, function/data pointer conversion in expression
#include "dlldata.c"
#include "RdcSdkTestServer_p.c"

#endif //_MERGE_PROXYSTUB
