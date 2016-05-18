
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// ComplianceAPI.h - types and definitions for ComplianceAPI command line
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

#pragma comment(lib, "wbemuuid.lib")

// Define constants
#define ARGS_MIN 2


// Define application operational modes
typedef enum
{
    OPERATION_USER,
    OPERATION_ISGAMEBLOCKED,
    OPERATION_REQUESTURLOVERRIDE
} OPERATION;

// Define simple data structures for holding command line parsing results and 
//  enumeration and query results
typedef struct
{
    PWSTR pszUrl;
    DWORD dwNumSubUrl;
    PCWSTR* ppcszSubUrl;
} REQUESTURLOVERRIDE_IN;

typedef struct
{
    OPERATION eOperation;
    PWSTR pszSID;
    union
    {
        GUID guidAppID;
        REQUESTURLOVERRIDE_IN stRequestUrlOverride;
    };
} PARSERESULT;




