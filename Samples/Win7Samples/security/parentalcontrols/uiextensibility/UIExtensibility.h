
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// UIExtensibility.h - types and definitions for UIExtensibility command line
//  sample.

#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <Wbemidl.h>
#include <strsafe.h>
#include <wpc.h>

#include "Utilities.h"

# pragma comment(lib, "wbemuuid.lib")


// Define constants
#define ARGS_MIN 2
#define WPCS_WMI_NAMESPACE L"\\\\.\\root\\CIMV2\\Applications\\WindowsParentalControls"
#define WPCS_WMI_UI_EXTENSION L"WpcExtension"
#define WPCS_WMI_UI_EXTENSION_FORMAT_KEYS L"%s.ID=\"%s\",Subsystem=%d"
#define WPCS_WMI_UI_EXTENSION_SUBST_REMOVE 6
#define WPCS_WMI_STRING_WQL L"WQL"


// Define application operational modes
typedef enum
{
    OPERATION_LIST,
    OPERATION_QUERY,
    OPERATION_ADD,
    OPERATION_MOD,
    OPERATION_DEL
} OPERATION;

// Define simple data structures for holding command line parsing results and 
//  enumeration and query results
typedef struct
{
    PWSTR pszGuid;
    UINT nSubsystem;
    PWSTR pszNamePath;
    PWSTR pszSubTitlePath;
    PWSTR pszImagePath;
	PWSTR pszDisabledImagePath;
    PWSTR pszExePath;
} UXENTRY;

// Define mask for modify operations
typedef enum
{
    UMASK_GUID = 0x1,
    UMASK_SUBSYSTEM = 0x2,
    UMASK_NAMEPATH = 0x4,
    UMASK_SUBTITLEPATH = 0x8,
    UMASK_IMAGEPATH = 0x10,
    UMASK_DISABLEDIMAGEPATH = 0x20,
    UMASK_EXEPATH = 0x40
} UMASK;

typedef struct
{
    OPERATION eOperation;
    DWORD dwMask;
    union {
        UXENTRY stEntry;
        PWSTR pszQuery;
    };
} PARSERESULT;
