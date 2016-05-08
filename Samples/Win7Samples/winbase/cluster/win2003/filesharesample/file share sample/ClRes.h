/////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2003 <company name>
//
//  Module Name:
//      ClRes.h
//
//  Implementation File:
//      ClRes.cpp
//
//  Description:
//      Main header file for the resource DLL for File Share Sample (File Share Sample).
//
//  Author:
//      <name> (<e-mail name>) Mmmm DD, 2003
//
//  Revision History:
//
//  Notes:
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

/////////////////////////////////////////////////////////////////////////////
// Include Files
/////////////////////////////////////////////////////////////////////////////

#define UNICODE 1
#define _UNICODE 1
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0502
#endif

#pragma warning( disable : 4127 )   // conditional expression is constant (BEGIN_COM_MAP)
#pragma warning( disable : 4505 )   // unreferenced local function has been removed
#pragma warning( disable : 4511 )   // copy constructor could not be generated
#pragma warning( disable : 4514 )   // unreferenced inline function has been removed
#pragma warning( disable : 4710 )   // function not expanded / function '<x>' not inlined

#pragma warning( push )
#include <windows.h>
#include <clusapi.h>
#include <resapi.h>
#include <stdio.h>
#include <strsafe.h>
#include <assert.h>
#pragma warning( pop )

/////////////////////////////////////////////////////////////////////////////
// File Share Sample Definitions
/////////////////////////////////////////////////////////////////////////////

//
// This is the name of the cluster resource type.
//

#define RESTYPE_NAME        L"File Share Sample"

//
// This is the dll name.
//

#define RESTYPE_DLL_NAME    L"File Share Sample.dll"

//
// This is the name of the service we're dependent upon.
//

#define FILESHARESAMPLE_SVCNAME   L"LanmanServer"

BOOL WINAPI FileShareSampleDllMain(
      HINSTANCE hDllHandleIn
    , DWORD     nReasonIn
    , LPVOID    ReservedIn
    );

DWORD WINAPI FileShareSampleStartup(
      LPCWSTR                       pwszResourceTypeIn
    , DWORD                         nMinVersionSupportedIn
    , DWORD                         nMaxVersionSupportedIn
    , PSET_RESOURCE_STATUS_ROUTINE  pfnSetResourceStatusIn
    , PLOG_EVENT_ROUTINE            pfnLogEventIn
    , PCLRES_FUNCTION_TABLE *       pFunctionTableOut
    );

/////////////////////////////////////////////////////////////////////////////
// General Definitions
/////////////////////////////////////////////////////////////////////////////

#define DBG_PRINT printf

/////////////////////////////////////////////////////////////////////////////
// Global Variables and Prototypes
/////////////////////////////////////////////////////////////////////////////

extern HINSTANCE g_hInstance;

//
// Event Logging routine.
//

extern PLOG_EVENT_ROUTINE g_pfnLogEvent;

//
// Resource Status routine for pending Online and Offline calls.
//

extern PSET_RESOURCE_STATUS_ROUTINE g_pfnSetResourceStatus;

