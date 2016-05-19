// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


/*
  
    Server Profiling Routines

    FILE: Prof.h
    
    PURPOSE: Routines for profiling server system services.
    
    FUNCTIONS:
        ProfOpenLog - Opens the profile log file and prepares to
    take timings.
        ProfRecordTime - Records the time of an event.
        ProfCloseLog - Writes the profiling data to the file
    and closes it.

    COMMENTS: These routines depend on the existence of a high-resolution
        performance counter.
*/
#pragma once

// Maximum size for the profile log.
// After the log reaches this size new entries "fall off" (threy
// do not get reorded).
#define PROF_LOG_MAX_SIZE (10*1024*1024)

// This critsec is used for mutial exclusion when generating and 
// writing the profiling information.  It is also accessible to
// the server routines that may want to use it for syncronising
// their internal operation and creation of profiling info.
// The critsec can only be used between the calls to ProfOpenLog and
// ProfCloseLog.
extern CRITICAL_SECTION ProfCriticalSection;

/*
    FUNCTION: ProfOpenLog

    PURPOSE: Opens the profile log file and prepares to
    take timings.  Also inits the mutex.

    PARAMETERS:
        LogFileName - Name of the profile log file.

    RETURN VALUE:
        none

    COMMENTS:

*/
VOID ProfOpenLog(LPTSTR LogFileName);

/*
    FUNCTION: ProfRecordTime

    PURPOSE: Creates an entry for the even with identifier id, and
        with message msg with a timestamp corresponding to the number of
        secods since the start of the program.

    PARAMETERS:
        id - Id of the event.
        msg - Message for the event.

    RETURN VALUE:
        none

    COMMENTS:  If the maximum log size has been exceeded, no action is taken.

*/
VOID ProfRecordTime(UINT id, LPTSTR msg);

/*
    FUNCTION: ProfCloseLog

    PURPOSE: Writes out the acumulated profile data and closes the log file.
        Closes the mutex.

    PARAMETERS:
        none

    RETURN VALUE:
        none

    COMMENTS:

*/
VOID ProfCloseLog(VOID);

// end Prof.h
