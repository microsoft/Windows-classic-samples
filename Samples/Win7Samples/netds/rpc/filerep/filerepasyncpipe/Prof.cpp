// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*
   

    Server Profiling Routines

    FILE: Prof.cpp
    
    PURPOSE: Routines for profiling server system services.
    
    FUNCTIONS:
        ProfOpenLog - Opens the profile log file and prepares to
    take timings.
        ProfRecordTime - Records the time of an event.
        ProfCloseLog - Writes the profiling data to the file
    and closes it.

    COMMENTS: See the header file for function info.
        These routines depend on the existence of a high-resolution
    performance counter.

*/

#include "common.h"

#include <process.h>

#include "Prof.h"
#include "Service.h"

// Handle to the profile log file.
HANDLE hProfLog = NULL;

// Counters for the timings.
LARGE_INTEGER lpStartTime, lpCurrentTime, lpFrequency;

// The profile log text data.
// Profile log gets acumulated in this string first, and then
// gets written out when the log gets closed.
TCHAR PROF_LOG[PROF_LOG_MAX_SIZE];

// Current size of the profile log.
UINT nProfLogSize = 0;

// This is the maximum size of the log entry without the
// user-specified message.  It is basically the size of a string
// with an integer and a float.
#define MAX_ENTRY_SIZE_NOMSG (32)

CRITICAL_SECTION ProfCriticalSection;

VOID ProfOpenLog(LPTSTR LogFileName) {

    // Init the critsec.
    if (InitializeCriticalSectionAndSpinCount(&ProfCriticalSection, 0) == 0) {
        AddToMessageLogProcFailure(TEXT("ProfOpenLog: InitializeCriticalSectionAndSpinCount"), GetLastError());
        return;
    }

    // Open the profiling log.
    if ((hProfLog = CreateFile(LogFileName,
                               GENERIC_WRITE,
                               0,
                               NULL,
                               CREATE_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL)) == INVALID_HANDLE_VALUE) {
        AddToMessageLogProcFailure(TEXT("ProfOpenLog: CreateFile"), GetLastError());
        return;
    }

    // Get the current time.
    if (QueryPerformanceCounter(&lpStartTime) == FALSE) {
        AddToMessageLogProcFailure(TEXT("ProfOpenLog: QueryPerformanceCounter"), GetLastError());
        return;        
    }

    // Get the frequency.
    if (QueryPerformanceFrequency(&lpFrequency) == FALSE) {
        AddToMessageLogProcFailure(TEXT("ProfOpenLog: QueryPerformanceCounter"), GetLastError());
        return;        
    }
}

VOID ProfRecordTime(UINT id, LPTSTR msg) {
    FLOAT dt;

    EnterCriticalSection(&ProfCriticalSection);

    // Write the entry only if there is enough space left in
    // the log.
    if (nProfLogSize < PROF_LOG_MAX_SIZE - (MAX_ENTRY_SIZE_NOMSG + _tcslen(msg))) {
        if (QueryPerformanceCounter(&lpCurrentTime) == FALSE) {
            AddToMessageLogProcFailure(TEXT("ProfRecordTime: QueryPerformanceCounter"), GetLastError());
            LeaveCriticalSection(&ProfCriticalSection);
            return;
        }
        
        // Get the time stamp for the event.
        dt = (FLOAT)(lpCurrentTime.QuadPart - lpStartTime.QuadPart)/(FLOAT)(lpFrequency.QuadPart);
        
        // Append the entry to the log.
        nProfLogSize+=_stprintf_s(&PROF_LOG[nProfLogSize], nProfLogSize, TEXT("%d %s %f\n"), id, msg, dt);
    }

    LeaveCriticalSection(&ProfCriticalSection);
}

VOID ProfCloseLog(VOID) {
    ULONG cbWritten;
    DWORD status;

    // Delete the critsec.
    // Here we make the assumption that no outside functions will use
    // the critsec anymore.  This should be valid, since all the profiling data
    // has been written.
    // Also we make an assumption that only one thread will attempt
    // to close the profile log.  This should be valid as well, or the
    // user can inmplement a semaphore to take care of that.
    DeleteCriticalSection(&ProfCriticalSection);

    // Write out the profile log.
    if(!WriteFile(hProfLog,
                  (LPVOID)PROF_LOG,
                  nProfLogSize*sizeof(TCHAR),
                  &cbWritten,
                  NULL)) {

        AddToMessageLogProcFailure(TEXT("ProfCloseLog: WriteFile"), GetLastError());
        return;
    }

    // Close the profile log.
    if (hProfLog != NULL && hProfLog != INVALID_HANDLE_VALUE) {
        status = CloseHandle(hProfLog);
        ASSERT(status);
    }
}

// end Prof.cpp
