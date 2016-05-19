//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// Simple Wininet Cache APIs usage application (entries enumeration).
//

#ifndef _CACHESAMPLE_H_
#define _CACHESAMPLE_H_

#ifndef UNICODE
    #define UNICODE
#endif


#include    <windows.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <strsafe.h>
#include    <wininet.h>

#define ERR_MSG_LEN                         512

#define CACHESAMPLE_ACTION_DELETE           1
#define CACHESAMPLE_ACTION_ENUM_ALL         2
#define CACHESAMPLE_ACTION_DETAIL           4
#define CACHESAMPLE_ACTION_ENUM_COOKIE      8
#define CACHESAMPLE_ACTION_ENUM_HISTORY     16
#define CACHESAMPLE_ACTION_ENUM_CONTENT     32
#define CACHESAMPLE_ACTION_ENUM_MASK        (CACHESAMPLE_ACTION_ENUM_ALL \
                                            |CACHESAMPLE_ACTION_ENUM_COOKIE \
                                            |CACHESAMPLE_ACTION_ENUM_HISTORY \
                                            |CACHESAMPLE_ACTION_ENUM_CONTENT)


#define HISTORY                         L"visited:"         // Enumerate only in the HISTORY container    
#define COOKIE                          L"cookie:"          // Enumerate only in the COOKIE container
#define ALL                             NULL                // Enumerate in all FIXED container
#define CONTENT                         L""                 // Enumerate only in the CONTENT container



VOID ParseArguments(int argc, LPWSTR *);
VOID ClearCache(VOID);
VOID DeleteAllCacheGroups(VOID);
VOID EnumerateCache(VOID);
VOID GetEntryDetail(VOID);
VOID PrintUsageBlock(VOID);
VOID *Malloc(size_t);
VOID Free(VOID*);
VOID LogInetError(DWORD err, LPCWSTR);
VOID LogSysError(DWORD err, LPCWSTR);


#endif

