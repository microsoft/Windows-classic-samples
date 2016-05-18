// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


/*
    

    Server Debugging Routines

    FILE: DbgMsg.cpp
    
    PURPOSE: Routines for debugging system services.

*/

#include "common.h"

#include "DbgMsg.h"
#include "Service.h"

// Handle to the debug log file.
HANDLE hDbgMsgLog = NULL;

// Current size of the debug log.
UINT nDbgMsgLogSize = 0;

CRITICAL_SECTION DbgMsgCriticalSection;

VOID DbgMsgOpenLog(LPTSTR LogFileName) {

    // Init the critsec.
    if (InitializeCriticalSectionAndSpinCount(&DbgMsgCriticalSection, 0) == 0) {
        AddToMessageLogProcFailure(TEXT("DbgMsgOpenLog: InitializeCriticalSectionAndSpinCount"), GetLastError());
        return;
    }

    // Open the profiling log.
    if ((hDbgMsgLog = CreateFile(LogFileName,
                               GENERIC_WRITE,
                               0,
                               NULL,
                               CREATE_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL)) == INVALID_HANDLE_VALUE) {
        AddToMessageLogProcFailure(TEXT("DbgMsgOpenLog: CreateFile"), GetLastError());
    }
}

VOID DbgMsgRecord(LPTSTR msg) {
    ULONG cbWritten;

    EnterCriticalSection(&DbgMsgCriticalSection);

    // Write the entry only if there is enough space left in
    // the log.
    if (nDbgMsgLogSize < DEBUG_LOG_MAX_SIZE + _tcslen(msg)) {

        // Append the entry to the log.
        nDbgMsgLogSize += _tcslen(msg);
        if(!WriteFile(hDbgMsgLog,
                      (LPVOID)msg,
                      _tcslen(msg)*sizeof(TCHAR),
                      &cbWritten,
                      NULL)) {
            AddToMessageLogProcFailure(TEXT("DbgMsgRecord: WriteFile"), GetLastError());
        }
    }

    LeaveCriticalSection(&DbgMsgCriticalSection);
    return;
}

VOID DbgMsgCloseLog(VOID) {
    DWORD status;

    // Delete and the critsec.
    DeleteCriticalSection(&DbgMsgCriticalSection);

    // Close the debug log.
    if (hDbgMsgLog != NULL && hDbgMsgLog != INVALID_HANDLE_VALUE) {
        status = CloseHandle(hDbgMsgLog);
        ASSERT(status);
    }
}

// end DbgMsg.cpp
