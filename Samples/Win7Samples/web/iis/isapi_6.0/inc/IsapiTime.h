/*++

Copyright (c) 2003  Microsoft Corporation

Module Name: IsapiTime.h

Abstract:

    Some helper functions to work with time

Author:

    ISAPI developer (Microsoft employee), October 2002

--*/

#ifndef _isapi_time_h
#define _isapi_time_h

#include <IsapiTools.h>

BOOL
InitializeIsapiTime(
    VOID
    );

DWORD
GetCurrentTimeInSeconds(
    VOID
    );

DWORD64
GetCurrentTimeInMilliseconds(
    VOID
    );

BOOL
GetCurrentTimeAsFileTime(
    FILETIME *  pft
    );

BOOL
GetFileTimeFromString(
    FILETIME *  pft,
    CHAR *      szTimeString
    );

BOOL
GetFileTimeAsString(
    FILETIME *      pft,
    ISAPI_STRING *  pString
    );

BOOL
GetCurrentTimeAsString(
    ISAPI_STRING *  pString
    );

DWORD
DiffFileTimeInSeconds(
    FILETIME *  pft1,
    FILETIME *  pft2
    );

DWORD64
DiffFileTimeInMilliseconds(
    FILETIME *  pft1,
    FILETIME *  pft2
    );

#endif  // _isapi_time_h
