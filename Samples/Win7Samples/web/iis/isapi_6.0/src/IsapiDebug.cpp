/*++

Copyright (c) 2003  Microsoft Corporation

Module Name: IsapiDebug.cpp

Abstract:

    Debug functions for ISAPI tools library

Author:

    ISAPI developer (Microsoft employee), October 2002

--*/

#include <IsapiTools.h>
#include <stdio.h>

//
// Local prototypes
//

VOID
WriteDebugArgs(
    CHAR *  szFormat,
    va_list args
    );

//
// Function implementations
//

VOID
IsapiAssert(
    CHAR *  szExpr,
    CHAR *  szFile,
    DWORD   dwLine
    )
/*++

Purpose:

    Calls a DebugBreak and reports the expression, source
    filename, and line number to the debugger.

    This function is called by the ISAPI_ASSERT macro, which
    provides the expression, source file, and line information.

Arguments:

    szExpr - The expression that failed the assertion
    szFile - The name of the source file that contains the expression
    dwLine - The line number of the expression

Returns:

    None

--*/
{
    va_list args;

    args = (va_list)&szExpr;

    WriteDebugArgs( "ISAPIASSERT Failed:  \"%s\", File=\"%s\", Line=%d.\r\n",
                args );

    va_end( args );

    DebugBreak();
}

VOID
WriteDebug(
    CHAR *  szFormat,
    ...
    )
/*++

Purpose:

    Writes a line to an attached debugger using printf-style
    formatting.

Arguments:

    szFormat - The format string
    ...      - Zero or more additional arguments

Returns:

    None

--*/
{
    va_list args;

    va_start( args, szFormat );

    WriteDebugArgs( szFormat, args );

    va_end( args );
}

VOID
WriteDebugArgs(
    CHAR *  szFormat,
    va_list args
    )
/*++

Purpose:

    Writes a line to an attached debugger using printf-style
    formatting and a va_list of args.

Arguments:

    szFormat - The format string
    args     - The va_list of additional arguments

Returns:

    None

--*/
{
    CHAR    szOutput[MAX_DEBUG_OUTPUT];
    INT     nWritten = 0;

    //
    // Put the module name into the output buffer.
    //

    nWritten = _snprintf_s( szOutput, sizeof(szOutput),
                          MAX_DEBUG_OUTPUT,
                          "[%s.dll] ",
                          g_szModuleName );

    if ( nWritten == -1 )
    {
        //
        // Ouch.  Buffer was too small for even the module name
        //

        return;
    }

    //
    // Now write the rest of the data into the buffer
    //

    nWritten = _vsnprintf_s( szOutput + nWritten,
							sizeof(szOutput) - nWritten,
                           MAX_DEBUG_OUTPUT - nWritten,
                           szFormat,
                           args );

    //
    // If we overran the buffer, truncate the data
    // with \r\n\0
    //

    if ( nWritten == -1 )
    {
        //
        // If, for some reason, we have a pathologically
        // small MAX_DEBUG_OUTPUT, don't overrun the buffer
        //

        if ( MAX_DEBUG_OUTPUT < 3 )
        {
            return;
        }

        szOutput[MAX_DEBUG_OUTPUT - 3] = '\r';
        szOutput[MAX_DEBUG_OUTPUT - 2] = '\n';
        szOutput[MAX_DEBUG_OUTPUT - 1] = '\0';
    }

    //
    // Write it
    //

    OutputDebugString( szOutput );
}


