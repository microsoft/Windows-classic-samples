/*++

Copyright (c) 2003  Microsoft Corporation

Module Name: IsapiTime.cpp

Abstract:

    Some helper functions to work with time

Author:

    ISAPI developer (Microsoft employee), October 2002

--*/

#include <IsapiTools.h>

//
// Define sizes for time strings.
//
// Note that time strings are generally in the form
// used in an HTTP date header. For example:
//
//   "Tue, 16 Oct 2002 20:25:00 GMT"
//

#define SIZE_DATE           sizeof("ddd, dd MMM yyyy")
#define SIZE_TIME           sizeof("HH:mm:ss")
#define SIZE_TIME_STRING    sizeof("ddd, dd MMMM yyyy HH:mm:ss GMT")

FILETIME g_ftInitTime = {0};

BOOL
InitializeIsapiTime(
    VOID
    )
/*++

Purpose:

    Initializes IsapiTools time functions by initializing g_ftInitTime

Arguments:

    None

Returns:

    TRUE on success, FALSE on failure

--*/
{
    if ( ((LARGE_INTEGER*)&g_ftInitTime)->QuadPart == 0 )
    {
        if ( GetCurrentTimeAsFileTime( &g_ftInitTime ) == FALSE )
        {
            return FALSE;
        }
    }

    return TRUE;
}

DWORD
GetCurrentTimeInSeconds(
    VOID
    )
/*++

Purpose:

    Returns the number of seconds since InitializeIsapiTime was called

Arguments:

    None

Returns:

    The number of seconds

--*/
{
    return (DWORD)(GetCurrentTimeInMilliseconds() / 1000);
}

DWORD64
GetCurrentTimeInMilliseconds(
    VOID
    )
/*++

Purpose:

    Returns the number of milliseconds since InitializeIsapiTime was called.

Arguments:

    None

Returns:

    The number of milliseconds

--*/
{
    FILETIME    ftCurrent;

    //
    // If we haven't been initialized, just return 0
    //

    if ( ((LARGE_INTEGER*)&g_ftInitTime)->QuadPart == 0 )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return 0;
    }

    //
    // Get the current time
    //
    // Note that GetCurrentTimeAsFileTime calls GetSystemTime,
    // which is a system call.  This introduces a performance
    // cost due to a kernel transition.  It would be possible
    // to use something like GetTickCount to obtain higher
    // performance, but GetTickCount returns a DWORD.  This
    // would introduce a rollover problem after about 49 days.
    //
    // This code favors correctness over performance in this
    // case.
    //

    if ( GetCurrentTimeAsFileTime( &ftCurrent ) == FALSE )
    {
        return 0;
    }

    return DiffFileTimeInMilliseconds( &g_ftInitTime, &ftCurrent );
}

BOOL
GetCurrentTimeAsFileTime(
    FILETIME *  pft
    )
/*++

Purpose:

    Returns the current time in the provided FILETIME structure

Arguments:

    pft - Pointer to the target FILETIME

Returns:

    TRUE on success, FALSE on failure

--*/
{
    SYSTEMTIME  st;

    //
    // Get the current time in GMT
    //

    GetSystemTime( &st );

    return SystemTimeToFileTime( &st, pft );
}

BOOL
GetFileTimeFromString(
    FILETIME *  pft,
    CHAR *      szTimeString
    )
/*++

Purpose:

    Returns a filetime, corresponding to the provided string,
    in the format of:

      "Tue, 16 Oct 2002 20:25:00 GMT"

Arguments:

    pft          - On return, contains the FILETIME data
    szTimeString - The string to parse for time

Returns:

    TRUE on success, FALSE on failure

--*/
{
    SYSTEMTIME  st;
    CHAR *      pCursor;

    ZeroMemory( &st, sizeof(SYSTEMTIME) );

    //
    // Build the members of the systemtime structure from
    // the input string.
    //

    pCursor = szTimeString;

    //
    // Get the day of week
    //

    if ( strnicmp( pCursor, "sun", 3 ) == 0 )
    {
        st.wDayOfWeek = 0;
    }
    else if ( strnicmp( pCursor, "mon", 3 ) == 0 )
    {
        st.wDayOfWeek = 1;
    }
    else if ( strnicmp( pCursor, "tue", 3 ) == 0 )
    {
        st.wDayOfWeek = 2;
    }
    else if ( strnicmp( pCursor, "wed", 3 ) == 0 )
    {
        st.wDayOfWeek = 3;
    }
    else if ( strnicmp( pCursor, "thu", 3 ) == 0 )
    {
        st.wDayOfWeek = 4;
    }
    else if ( strnicmp( pCursor, "fri", 3 ) == 0 )
    {
        st.wDayOfWeek = 5;
    }
    else if ( strnicmp( pCursor, "sat", 3 ) == 0 )
    {
        st.wDayOfWeek = 6;
    }
    else
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto Failed;
    }

    //
    // Get the day
    //

    pCursor = strchr( pCursor, ' ' );

    if ( !pCursor )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto Failed;
    }

    pCursor++;

    st.wDay = atoi( pCursor );

    //
    // Get the month
    //

    pCursor = strchr( pCursor, ' ' );

    if ( !pCursor )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto Failed;
    }

    pCursor++;

    if ( strnicmp( pCursor, "jan", 3 ) == 0 )
    {
        st.wMonth = 1;
    }
    else if ( strnicmp( pCursor, "feb", 3 ) == 0 )
    {
        st.wMonth = 2;
    }
    else if ( strnicmp( pCursor, "mar", 3 ) == 0 )
    {
        st.wMonth = 3;
    }
    else if ( strnicmp( pCursor, "apr", 3 ) == 0 )
    {
        st.wMonth = 4;
    }
    else if ( strnicmp( pCursor, "may", 3 ) == 0 )
    {
        st.wMonth = 5;
    }
    else if ( strnicmp( pCursor, "jun", 3 ) == 0 )
    {
        st.wMonth = 6;
    }
    else if ( strnicmp( pCursor, "jul", 3 ) == 0 )
    {
        st.wMonth = 7;
    }
    else if ( strnicmp( pCursor, "aug", 3 ) == 0 )
    {
        st.wMonth = 8;
    }
    else if ( strnicmp( pCursor, "sep", 3 ) == 0 )
    {
        st.wMonth = 9;
    }
    else if ( strnicmp( pCursor, "oct", 3 ) == 0 )
    {
        st.wMonth = 10;
    }
    else if ( strnicmp( pCursor, "nov", 3 ) == 0 )
    {
        st.wMonth = 11;
    }
    else if ( strnicmp( pCursor, "dec", 3 ) == 0 )
    {
        st.wMonth = 12;
    }
    else
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto Failed;
    }

    //
    // Get the year
    //

    pCursor = strchr( pCursor, ' ' );

    if ( !pCursor )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto Failed;
    }

    pCursor++;

    st.wYear = atoi( pCursor );

    //
    // Get the Hour
    //

    pCursor = strchr( pCursor, ' ' );

    if ( !pCursor )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto Failed;
    }

    pCursor++;

    st.wHour = atoi( pCursor );

    //
    // Get the minutes
    //

    pCursor = strchr( pCursor, ':' );

    if ( !pCursor )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto Failed;
    }

    pCursor++;

    st.wMinute = atoi( pCursor );

    //
    // Get the seconds
    //

    pCursor = strchr( pCursor, ':' );

    if ( !pCursor )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto Failed;
    }

    pCursor++;

    st.wSecond = atoi( pCursor );

    //
    // Now convert it to a filetime
    //

    return SystemTimeToFileTime( &st, pft );

Failed:

    //
    // No special cleanup required
    //

    return FALSE;
}

BOOL
GetFileTimeAsString(
    FILETIME *      pft,
    ISAPI_STRING *  pString
    )
/*++

Purpose:

    Builds string in the following format from the provided FILETIME:

      "Tue, 16 Oct 2002 20:25:00 GMT"

Arguments:

    pft     - The FILETIME
    pString - ISAPI_STRING to store the result

Returns:

    TRUE on success, FALSE on failure

--*/
{
    SYSTEMTIME  st;
    CHAR        szDate[SIZE_DATE];
    CHAR        szTime[SIZE_TIME];
    DWORD       cbCopied;
    BOOL        fResult;

    //
    // Convert the supplied time to a SYSTEMTIME
    //

    fResult = FileTimeToSystemTime( pft, &st );

    if ( !fResult )
    {
        goto Failed;
    }

    //
    // Get the date and time as strings
    //

    cbCopied = GetDateFormat( LOCALE_SYSTEM_DEFAULT,
                              0,
                              &st,
                              "ddd, dd MMM yyyy",
                              szDate,
                              SIZE_DATE );

    if ( cbCopied == 0 )
    {
        goto Failed;
    }

    cbCopied = GetTimeFormat( LOCALE_SYSTEM_DEFAULT,
                              0,
                              &st,
                              "HH:mm:ss",
                              szTime,
                              SIZE_TIME );

    if ( cbCopied == 0 )
    {
        goto Failed;
    }

    //
    // Build the final result into the string
    //

    fResult = pString->Printf( "%s %s GMT",
                               szDate,
                               szTime );

    if ( !fResult )
    {
        goto Failed;
    }

    return TRUE;

Failed:

    //
    // No special cleanup required
    //

    return FALSE;
}

BOOL
GetCurrentTimeAsString(
    ISAPI_STRING *  pString
    )
/*++

Purpose:

    Builds string in the following format from the current time:

      "Tue, 16 Oct 2002 20:25:00 GMT"

Arguments:

    pString - ISAPI_STRING to store the result

Returns:

    TRUE on success, FALSE on failure

--*/
{
    FILETIME    ft;

    if ( GetCurrentTimeAsFileTime( &ft ) == FALSE )
    {
        goto Failed;
    }

    return GetFileTimeAsString( &ft, pString );

Failed:

    //
    // No special cleanup required
    //

    return FALSE;
}

DWORD
DiffFileTimeInSeconds(
    FILETIME *  pft1,
    FILETIME *  pft2
    )
/*++

Purpose:

    Returns the difference, in seconds, between the two
    provided FILETIMEs

Arguments:

    pft1 - The first FILETIME
    pft2 - The second FILETIME

Returns:

    The difference, in seconds

--*/
{
    return (DWORD)(DiffFileTimeInMilliseconds( pft1, pft2 ) / 1000);
}

DWORD64
DiffFileTimeInMilliseconds(
    FILETIME *  pft1,
    FILETIME *  pft2
    )
/*++

Purpose:

    Returns the difference, in milliseconds, between the two
    provided FILETIMEs

Arguments:

    pft1 - The first FILETIME
    pft2 - The second FILETIME

Returns:

    The difference, in milliseconds

--*/
{
    DWORD64 Time1;
    DWORD64 Time2;
    DWORD64 DiffTime;

    Time1 = ((LARGE_INTEGER*)pft1)->QuadPart;
    Time2 = ((LARGE_INTEGER*)pft2)->QuadPart;

    DiffTime = Time2 > Time1 ? Time2 - Time1 : Time1 - Time2;

    //
    // Return result, converted from 100 nanosecond intervals
    // to milliseconds.
    //

    return DiffTime / 10000;
}
