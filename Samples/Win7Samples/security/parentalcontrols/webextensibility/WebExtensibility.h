
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
#include <objbase.h>
#include <strsafe.h>
#include <wpc.h>

#include "Utilities.h"

# pragma comment(lib, "wbemuuid.lib")

// Define constants
#define ARGS_MIN 2
#define WPCS_WMI_NAMESPACE L"\\\\.\\root\\CIMV2\\Applications\\WindowsParentalControls"
#define WPCS_WMI_SYSTEM_SETTINGS L"WpcSystemSettings=@"
#define WPCS_WMI_UI_EXTENSION_FORMAT_KEYS L"%s.ID=\"%s\",Silo=%d"
#define WPCS_WMI_UI_EXTENSION_SUBST_REMOVE 6
#define WPCS_WMI_STRING_WQL L"WQL"

// Define application operational modes
typedef enum
{
    OPERATION_HTTP,
    OPERATION_URL,
    OPERATION_FILTER
} OPERATION;

typedef enum
{
    EXEMPTION_LIST,
    EXEMPTION_ADD,
    EXEMPTION_DEL,
    ID_NAME_GET,
    ID_NAME_SET,
    ID_NAME_RESET
} SUBOPERATION;

