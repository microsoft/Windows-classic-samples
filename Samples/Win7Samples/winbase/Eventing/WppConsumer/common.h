/*++

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
    ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
    PARTICULAR PURPOSE.

    Copyright (c) Microsoft Corporation. All rights reserved

Module Name:

    common.h

Abstract:

   Definitions on the decoding context structure, and utility function for determining
   if the running operating system is prior Windows 7.

--*/

extern "C" {

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

#include <initguid.h>
#include <wmistr.h>
#include <evntcons.h>
#include <evntrace.h>
#include <Tdh.h>

}

#include <comutil.h>


CONST ULONG STRLEN_GUID                         = 39;
CONST ULONG STRLEN_UTC_DATETIME                 = 64;
CONST ULONG ONE_HUNDRED_NANOSECONDS_PER_SECOND  = 10000000;
CONST PWSTR FORMAT_STRING_DATE                  = L"yyyy'-'MM'-'dd";
CONST PWSTR FORMAT_STRING_TIME                  = L"HH':'mm':'ss";

CONST ULONG WIN7_MAJOR_VERSION                  = 6; 
CONST ULONG WIN7_MINOR_VERSION                  = 1;

BOOLEAN
IsOSPriorWin7(
    VOID
    );

//
//  Following is a user-defined structure that can be passed to the
//  ProcessTrace API. EventCallback() and BufferCallback() functions can
//  retrieve a pointer to this structure from EVENT_RECORD::UserContext
//  and EVENT_TRACE_LOGFILE::Context respectively. It is a way to pass
//  around logfile-specific information between Callbacks. Better than
//  using globals.
//

CONST ULONG INITIAL_FORMATBUFFER_SIZE = 65536;

typedef struct _PROCESSING_CONTEXT {

    PWSTR TMFFile;
    ULONG PointerSize;
    __field_bcount(BufferSize) PBYTE Buffer;
    ULONG BufferSize;
    ULONG BufferCount;
    ULONGLONG EventCount;
    TDH_CONTEXT TdhContexts[2];
    BOOLEAN OSPriorWin7;

    _PROCESSING_CONTEXT()
        :TMFFile(NULL)
        ,PointerSize(sizeof(PVOID))
        ,BufferSize(INITIAL_FORMATBUFFER_SIZE)
        ,BufferCount(0)
        ,EventCount(0)
    {
        Buffer = (PBYTE)malloc(BufferSize);
        if (Buffer == NULL) {
            throw (ERROR_OUTOFMEMORY);
        }
        OSPriorWin7 = IsOSPriorWin7();
    }

    ~_PROCESSING_CONTEXT()
    {
        free(Buffer);
    }

} PROCESSING_CONTEXT, *PPROCESSING_CONTEXT;

BOOLEAN
IsOSPriorWin7(
    VOID
    )

/*++

Routine Description:

    This routine determines if the version of the running operating system is
    prior Windows 7 or not. This information is needed for making the decision
    to use some Tdh API functions which are new in Windows 7.

Arguments:

    None.

Return Value:

    TRUE - The operating system is prior Windows 7.

    FALSE - The operating system is Windows 7 or later.

--*/

{
    BOOL VersionQuerySuccess; 
    OSVERSIONINFO OperatingSystemInfo;

    ZeroMemory(&OperatingSystemInfo, sizeof(OSVERSIONINFO));
    OperatingSystemInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    VersionQuerySuccess = GetVersionEx(&OperatingSystemInfo);

    if (VersionQuerySuccess == TRUE) {
        if (OperatingSystemInfo.dwMajorVersion < WIN7_MAJOR_VERSION) {
            return TRUE;
        }
        if ((OperatingSystemInfo.dwMajorVersion == WIN7_MAJOR_VERSION) &&
            (OperatingSystemInfo.dwMinorVersion < WIN7_MINOR_VERSION)) {

            return TRUE;
        }
    }
    return FALSE;
}

