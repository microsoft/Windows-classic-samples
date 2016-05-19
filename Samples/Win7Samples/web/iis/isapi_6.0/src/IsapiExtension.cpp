/*++

Copyright (c) 2003  Microsoft Corporation

Module Name: IsapiExtension.cpp

Abstract:

    A class to provide basic services to an
    ISAPI extension

Author:

    ISAPI developer (Microsoft employee), October 2002

--*/

#ifndef _WIN32_WINNT
#define _WIN32_WINNT    0x0500
#endif

#include <IsapiTools.h>

ISAPI_EXTENSION::ISAPI_EXTENSION()
    : _fInitialized( FALSE ),
      _fFirstHitInitDone( FALSE ),
      _pfnInit( NULL )
/*++

Purpose:

    Constructor for the ISAPI_EXTENSION object

Arguments:

    None

Returns:

    None

--*/
{
}

ISAPI_EXTENSION::~ISAPI_EXTENSION()
/*++

Purpose:

    Destructor for the ISAPI_EXTENSION object

Arguments:

    None

Returns:

    None

--*/
{
    //
    // If we successfully initialized, then we
    // need to delete the critical section
    //

    if ( _fInitialized )
    {
        DeleteCriticalSection( &_csInit );
        _fInitialized = FALSE;
    }
}

BOOL
ISAPI_EXTENSION::Initialize(
    LPSTR   szModuleName
    )
/*++

Purpose:

    Initializes the ISAPI_EXTENSION object

Arguments:

    szModuleName - The name of the module

Returns:

    TRUE on success, FALSE on failure

--*/
{
    LPSTR   pCursor;
    BOOL    fResult;
    DWORD   cchModuleFileName;

    //
    // Store the module name
    //

    if ( !_strModuleName.Copy( szModuleName ) )
    {
        return FALSE;
    }

    //
    // Derive the full path to this module.  We'll assume
    // that the full file name for this module does not
    // exceed MAX_PATH characters.  If it does, we'll
    // detect the condition and fail.
    //

    if ( !_strModulePath.ResizeBuffer( MAX_PATH+1 ) )
    {
        return FALSE;
    }

    cchModuleFileName = GetModuleFileName( GetModuleHandle(szModuleName),
                                           _strModulePath.QueryStr(),
                                           MAX_PATH );
    if ( cchModuleFileName == 0 ||
         cchModuleFileName == MAX_PATH )
    {
        if ( cchModuleFileName == MAX_PATH )
        {
            SetLastError( ERROR_INSUFFICIENT_BUFFER );
        }

        return FALSE;
    }

    _strModulePath.CalcLen();

    //
    // At this point, _strModulePath contains the full file
    // name to this module, including the ".dll".  We'll
    // strip off the module name to get the final result.
    //

    pCursor = strrchr( _strModulePath.QueryStr(), '\\' );

    if ( !pCursor )
    {
        //
        // Hmmm.  We really expected a backslash...
        //

        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    *pCursor = '\0';

    _strModulePath.CalcLen();

    //
    // Initialize the first hit init critical section.
    //
    // Note that the InitializeCriticalSection function
    // does not necessarily leave the critical section in
    // a useable state.  It's possible, in low memory
    // conditions, that a call to EnterCriticalSection
    // could cause an access violation.  To avoid this
    // situation, we'll use InitializeCriticalSectionAndSpinCount,
    // which detects this condition and returns FALSE
    // if it cannot fully initialize.
    //

    fResult = InitializeCriticalSectionAndSpinCount( &_csInit,
                                                     0 );

    if ( fResult )
    {
        _fInitialized = TRUE;
    }

    return fResult;
}

BOOL
ISAPI_EXTENSION::InitOnce(
    PFN_FIRST_HIT_INIT          pfnInit,
    VOID *                      pContext
    )
/*++

Purpose:

    Performs a guaranteed one-time call of the specified
    function.

Arguments:

    pfnInit  - The function to call exactly once
    pContext - Context to be passed to the specified function

Returns:

    TRUE on success, FALSE on failure

--*/
{
    BOOL    fResult;

    //
    // If we are already initialized, just return
    // TRUE.
    //
    // Note that we expect that most calls to this
    // function will return here.  It is important
    // that we do not enter a critical section on this
    // code path, as this would create contention as
    // multiple threads enter the ISAPI extension
    // concurrently.
    //

    if ( _fFirstHitInitDone )
    {
        return TRUE;
    }

    //
    // If we get here, then we need to call the
    // initialization function.  This needs to
    // happen exactly once, even if multiple threads
    // get past the above check.  We'll use a
    // critical section to ensure this.
    //

    EnterCriticalSection( &_csInit );

    //
    // Now that we're in the critical section, the current
    // thread has exclusive access to this code.  We'll
    // check to see if we are still uninitialized.  If so,
    // we'll call the init function and update the state
    // of this object so that other threads that get here
    // won't also call it.
    //

    if ( !_fFirstHitInitDone )
    {
        fResult = pfnInit( pContext );

        if ( fResult )
        {
            _fFirstHitInitDone = TRUE;
        }
    }

    LeaveCriticalSection( &_csInit );

    return fResult;
}


CHAR *
ISAPI_EXTENSION::QueryModulePath(
    VOID
    )
/*++

Purpose:

    Returns the path of the module of the ISAPI_EXTENSION object

Arguments:

    None

Returns:

    Pointer to the module name

--*/
{
    return _strModulePath.QueryStr();
}


